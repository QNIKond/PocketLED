import threading
import queue
import time
import tkinter as tk
from tkinter import scrolledtext, messagebox

import serial
from serial import SerialException


PORT = "COM9"
BAUDRATE = 9600
BYTESIZE = serial.EIGHTBITS
PARITY = serial.PARITY_NONE
STOPBITS = serial.STOPBITS_ONE
TIMEOUT = 0.1

#Должно быть меньше 64 (0x40)
COMMANDS = { 
    "var1": {"code": 0x11, "param": False},
    "var2": {"code": 0x12, "param": False},
    "var3": {"code": 0x13, "param": False},
    "var4": {"code": 0x14, "param": False},
    "svar1": {"code": 0x15, "param": True},
    "svar2": {"code": 0x16, "param": True},
    "svar3": {"code": 0x17, "param": True},
    "svar4": {"code": 0x18, "param": True},
}


RX_MESSAGES = {

    0x02: {"text": "Lazy response:", "param": True},
    0x03: {"text": "X:", "param": True},
    0x04: {"text": "Y:", "param": True},
    0x05: {"text": "inputRaw changed:", "param": True},
    0x11: {"text": "Test point 1", "param": False},
    0x12: {"text": "Test point 2", "param": False},
    0x13: {"text": "Test point 3", "param": False},
    0x14: {"text": "Test point 4", "param": False},
    0x15: {"text": "Param 1", "param": True},
    0x16: {"text": "Param 2", "param": True},
    0x17: {"text": "Param 3", "param": True},
    0x18: {"text": "Param 4", "param": True},
}


KEY_COMMANDS = {
    "Up":    {"press": 0xF0, "release": 0xF1},
    "Left":  {"press": 0xF2, "release": 0xF3},
    "Right":  {"press": 0xF4, "release": 0xF5},
    "Down": {"press": 0xF6, "release": 0xF7},
    "End":  {"press": 0xF8, "release": 0xF9},
    "Home":   {"press": 0xFA, "release": 0xFB},
    "Delete":{"press": 0xFE, "release": 0xFF},
}


class UartGuiApp:
    def __init__(self, root):
        self.root = root
        self.root.title("UART console")
        self.root.geometry("500x500")

        self.bg_color = "#1e1e1e"
        self.fg_color = "#d4d4d4"
        self.entry_bg = "#2d2d2d"
        self.button_bg = "#3a3a3a"
        self.highlight = "#569cd6"

        self.root.configure(bg=self.bg_color)

        self.connected = False

        self.rx_queue = queue.Queue()
        self.stop_event = threading.Event()
        self.ser = None

        self.pressed_keys = set()

        self._build_ui()
        self._open_serial()
        self._start_reader_thread()
        self._poll_rx_queue()

        self.root.bind("<KeyPress>", self.on_key_press)
        self.root.bind("<KeyRelease>", self.on_key_release)

        self.root.protocol("WM_DELETE_WINDOW", self.on_close)
        

    def _build_ui(self):
        self.text = scrolledtext.ScrolledText(
            self.root,
            wrap=tk.WORD,
            height=12,
            bg=self.bg_color,
            fg=self.fg_color,
            insertbackground=self.fg_color,
            selectbackground=self.highlight,
            font=("Consolas", 12)
        )
        self.text.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)
        self.text.configure(state=tk.DISABLED)

        bottom = tk.Frame(self.root, bg=self.bg_color)
        bottom.pack(fill=tk.X, padx=8, pady=(0, 8))

        self.entry = tk.Entry(
            bottom,
            bg=self.entry_bg,
            fg=self.fg_color,
            insertbackground=self.fg_color
        )
        self.entry.pack(side=tk.LEFT, fill=tk.X, expand=True)
        self.entry.bind("<Return>", self.on_enter)

        send_btn = tk.Button(
            bottom,
            text="Send",
            bg=self.button_bg,
            fg=self.fg_color,
            activebackground=self.highlight,
            activeforeground="white",
            relief=tk.FLAT
        )
        send_btn.pack(side=tk.LEFT, padx=(8, 0))
        send_btn.config(command=self.on_send_button)

        clear_btn = tk.Button(
            bottom,
            text="Clear",
            bg=self.button_bg,
            fg=self.fg_color,
            activebackground=self.highlight,
            activeforeground="white",
            relief=tk.FLAT
        )
        clear_btn.pack(side=tk.LEFT, padx=(8, 0))
        clear_btn.config(command=self._clear_console)

        self.status = tk.Label(
            self.root,
            text="Status: initializing...",
            anchor="w",
            bg=self.bg_color,
            fg=self.fg_color
        )
        self.status.pack(fill=tk.X, padx=8, pady=(0, 8))

        self._log(f"Available commands: {', '.join(sorted(COMMANDS.keys()))}")
        self._log(f"UART: {PORT}, {BAUDRATE} 8N1")

    def _open_serial(self):
        try:
            self.ser = serial.Serial(
                port=PORT,
                baudrate=BAUDRATE,
                bytesize=BYTESIZE,
                parity=PARITY,
                stopbits=STOPBITS,
                timeout=TIMEOUT,
            )
            self.connected = True
            self._set_status(f"Status: connected to {PORT}")
        except SerialException:
            self.ser = None
            self.connected = False
            self._set_status("Status: disconnected")

    def _set_status(self, text):
        self.root.after(0, lambda: self.status.config(text=text))

    def _start_reader_thread(self):
        if self.ser is None:
            return
        t = threading.Thread(target=self._reader_loop, daemon=True)
        t.start()

    def _reader_loop(self):
        expect_param = False
        last_cmd = None
        last_text = ""

        reconnect_delay = 1.0  # секунды

        while not self.stop_event.is_set():

            # =========================
            # ЕСЛИ НЕТ СОЕДИНЕНИЯ → ПЫТАЕМСЯ ПОДКЛЮЧИТЬСЯ
            # =========================
            if not self.connected:
                try:
                    self.ser = serial.Serial(
                        port=PORT,
                        baudrate=BAUDRATE,
                        bytesize=BYTESIZE,
                        parity=PARITY,
                        stopbits=STOPBITS,
                        timeout=TIMEOUT,
                    )
                    self.connected = True
                    self._set_status(f"Status: connected to {PORT}")
                    time.sleep(0.5)
                    continue
                except SerialException:
                    self._set_status("Status: waiting for device...")
                    time.sleep(reconnect_delay)
                    continue

            # =========================
            # ЧТЕНИЕ ДАННЫХ
            # =========================
            try:
                data = self.ser.read(self.ser.in_waiting or 1)

                if data:
                    for b in data:

                        if expect_param:
                            self.rx_queue.put(
                                f"  RX: {last_text} 0x{b:02X} ({b}) (0x{last_cmd:02X})"
                            )
                            expect_param = False
                            last_cmd = None
                            continue

                        if b in RX_MESSAGES:
                            entry = RX_MESSAGES[b]

                            if entry["param"]:
                                expect_param = True
                                last_cmd = b
                                last_text = entry["text"]
                            else:
                                self.rx_queue.put(
                                    f"  RX: {entry['text']} (0x{b:02X})"
                                )
                        else:
                            self.rx_queue.put(f"  RX: 0x{b:02X}")

            # =========================
            # ЕСЛИ УСТРОЙСТВО ОТВАЛИЛОСЬ
            # =========================
            except (SerialException, OSError, PermissionError):
                try:
                    if self.ser:
                        self.ser.close()
                except Exception:
                    pass

                self.ser = None
                self.connected = False
                self._set_status("Status: disconnected")

                time.sleep(reconnect_delay)

    def _poll_rx_queue(self):
        try:
            while True:
                msg = self.rx_queue.get_nowait()
                self._log(msg)
        except queue.Empty:
            pass

        self.root.after(50, self._poll_rx_queue)

    def _log(self, message):
        # Check if scrollbar is already at the bottom
        at_bottom = self.text.yview()[1] >= 0.99

        self.text.configure(state=tk.NORMAL)
        self.text.insert(tk.END, message + "\n")

        # Only scroll if user was already at bottom
        if at_bottom:
            self.text.see(tk.END)

        self.text.configure(state=tk.DISABLED)

    def _clear_console(self):
        self.text.configure(state=tk.NORMAL)
        self.text.delete(1.0, tk.END)
        self.text.configure(state=tk.DISABLED)

    def on_key_press(self, event):
        key = event.keysym
        if key in KEY_COMMANDS:
            if key in self.pressed_keys:
                return "break"

            self.pressed_keys.add(key)
            self._send_byte(KEY_COMMANDS[key]["press"])
            return "break"

    def on_key_release(self, event):
        key = event.keysym
        if key in KEY_COMMANDS:
            if key in self.pressed_keys:
                self.pressed_keys.remove(key)
                self._send_byte(KEY_COMMANDS[key]["release"])
                return "break"

    def _send_byte(self, value):
        try:
            if self.connected and self.ser and self.ser.is_open:
                self.ser.write(bytes([value]))
                self.ser.flush()
        except Exception:
            self.connected = False

    def on_enter(self, event=None):
        self.on_send_button()
        return "break"

    def on_send_button(self):
        raw = self.entry.get().strip()
        self.entry.delete(0, tk.END)

        if not raw:
            return
        if not self.connected:
            self._log("ERROR: device not connected")
            return
        ok, result = self.parse_command(raw)
        if not ok:
            self._log(f"CMD: {raw}")
            self._log(f"ERROR: {result}")
            return

        try:
            for pkt in result:
                self.ser.write(pkt)
                self.ser.flush()

            #новый лог
            hex_codes = " ".join(f"0x{b:02X}" for pkt in result for b in pkt)
            self._log(f"CMD: {raw} ({hex_codes})")

        except Exception as e:
            self._log(f"ERROR: send failed: {e}")

    def parse_command(self, raw_text):
        parts = raw_text.split()
        if not parts:
            return False, "empty command"

        cmd = parts[0].lower()

        if cmd not in COMMANDS:
            return False, f'unknown command "{cmd}"'

        spec = COMMANDS[cmd]

        if spec["param"]:
            if len(parts) != 2:
                return False, "parameter required"

            param_text = parts[1].lower()

            
            if param_text == "on":
                param = 1
            elif param_text == "off":
                param = 0
            else:
                try:
                    param = int(param_text, 0)
                except ValueError:
                    return False, "invalid parameter"

            if not (0 <= param <= 255):
                return False, "parameter must be 0..255"

            return True, [bytes([spec["code"]]), bytes([param])]

        else:
            return True, [bytes([spec["code"]]), bytes([1])]

    def on_close(self):
        self.stop_event.set()
        try:
            if self.ser and self.ser.is_open:
                self.ser.close()
        except Exception:
            pass
        self.root.destroy()


def main():
    root = tk.Tk()
    app = UartGuiApp(root)
    root.mainloop()


if __name__ == "__main__":
    main()
