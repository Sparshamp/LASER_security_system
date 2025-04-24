import serial
import tkinter as tk
from datetime import datetime
import threading

# Replace COM3 with your port name (Linux: /dev/ttyUSB0 or ttyACM0)
arduino = serial.Serial('COM6', 9600)

data_log = []

def read_from_serial():
    while True:
        if arduino.in_waiting:
            line = arduino.readline().decode('utf-8').strip()
            if line:
                log_entry = f"{datetime.now().strftime('%H:%M:%S')} - {line}"
                data_log.append((datetime.now(), line))
                display.insert(tk.END, log_entry + "\n")
                display.yview(tk.END)

root = tk.Tk()
root.title("Laser Security System Monitor")
root.geometry("500x300")

title = tk.Label(root, text="Laser Security Monitoring", font=("Arial", 16))
title.pack()

display = tk.Text(root, height=12)
display.pack()

thread = threading.Thread(target=read_from_serial, daemon=True)
thread.start()

root.mainloop()
