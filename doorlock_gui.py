import tkinter as tk
from tkinter import ttk, messagebox
import pymongo
from datetime import datetime
import matplotlib.pyplot as plt
import pandas as pd
import threading
import serial

# MongoDB setup
client = pymongo.MongoClient("mongodb://localhost:27017/")
db = client["doorlock_system"]
collection = db["access_logs"]

# Serial setup (change COM port to your Arduino port)
SERIAL_PORT = "COM6"  # Change this to your actual port
BAUD_RATE = 9600

# Function to insert data
def insert_data(status):
    data = {
        "timestamp": datetime.now(),
        "status": status  # "Success" or "Failed"
    }
    collection.insert_one(data)

# Function to simulate/collect 10 entries (You can skip this when reading from Arduino)
def simulate_entries():
    for i in range(10):
        insert_data("Success" if i % 2 == 0 else "Failed")

# Function to visualize data
def show_graph():
    data = list(collection.find().sort("timestamp", -1).limit(20))
    if len(data) == 0:
        messagebox.showinfo("No Data", "No sensor data available.")
        return

    df = pd.DataFrame(data)
    #df['timestamp'] = pd.to_datetime(df['timestamp'])
    df['minute'] = df['timestamp'].dt.strftime('%H:%M')

    success = df[df['status'] == "Success"].groupby('minute').size()
    failed = df[df['status'] == "Failed"].groupby('minute').size()
    motion = df[df['status'] == "Motion Detected"].groupby('minute').size()

    plt.figure(figsize=(10, 6))
    plt.plot(success, label='Success', marker='o')
    plt.plot(failed, label='Failed', marker='x')
    plt.plot(motion, label='Motion Detected', marker='^')
    plt.title("Access Attempts by Minute")
    plt.xlabel("Time (HH:MM)")
    plt.ylabel("Number of Attempts")
    plt.xticks(rotation=45)
    plt.legend()
    plt.tight_layout()
    plt.show()

# Function to read data from serial and store in DB
def read_serial_data():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        while True:
            line = ser.readline().decode().strip()
            if line:
                log_console.insert(tk.END, f"{datetime.now().strftime('%H:%M:%S')} - {line}\n")
                log_console.see(tk.END)
                if "Welcome" in line or "Access granted" in line:
                    insert_data("Success")
                elif "Wrong Password" in line:
                    insert_data("Failed")
                elif "Motion Detected" in line:  # <--- Handle PIR-triggered alert
                    insert_data("Motion Detected")  # Optional if you want to log it too
    except Exception as e:
        log_console.insert(tk.END, f"[ERROR] {str(e)}\n")

# Tkinter UI
root = tk.Tk()
root.title("Password-Protected Door Lock Dashboard")
root.geometry("700x500")

frame = ttk.Frame(root, padding=10)
frame.pack(fill='both', expand=True)

ttk.Label(frame, text="Password Lock Monitoring System", font=("Arial", 16)).pack(pady=10)

ttk.Button(frame, text="📈 Show Graph", command=show_graph).pack(pady=5)
ttk.Button(frame, text="➕ Simulate 10 Entries", command=simulate_entries).pack(pady=5)

log_console = tk.Text(frame, height=15, bg='black', fg='lime', font=('Courier', 10))
log_console.pack(fill='both', expand=True, pady=10)

# Start serial reading in background thread
threading.Thread(target=read_serial_data, daemon=True).start()

root.mainloop()
