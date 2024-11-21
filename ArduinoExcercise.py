import serial
import time
import sys
import tkinter as tk
from tkinter import messagebox
import win32com.client

# Configure the serial port (change COMx or /dev/ttyUSBx to the correct port)
SERIAL_PORT = 'COM5'  # Replace with your Arduino's serial port
BAUD_RATE = 9600
EXIT_COMMAND = "EXIT"

def show_warning(message, timeout=3000):
    """Display a warning message in a popup that closes automatically."""
    def close_popup():
        root.destroy()  # Close the tkinter window

    root = tk.Tk()
    root.withdraw()  # Hide the root window
    root.after(timeout, close_popup)  # Schedule the popup to close after the timeout
    messagebox.showwarning("Warning", message)
    root.mainloop()

def close_chrome():
    """Close all instances of Google Chrome using WMI."""
    try:
        wmi = win32com.client.GetObject("winmgmts:")
        for process in wmi.InstancesOf("Win32_Process"):
            if process.Name == "chrome.exe":
                print(f"Terminating process {process.Name} (PID: {process.ProcessId})")
                process.Terminate()
        print("All Chrome processes terminated.")
    except Exception as e:
        print(f"Failed to terminate Chrome processes: {e}")

def monitor_arduino():
    # Open the serial connection
    with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
        print("Listening to Arduino...")
        
        zero_rpm_start_time = None
        warning_active = False
        warning_start_time = None
        
        while True:
            line = ser.readline().decode('utf-8').strip()
            print(f"Received line: {line}")
            
            if line.startswith("RPM:"):
                try:
                    rpm = float(line.split(":")[1].strip())
                    print(f"RPM: {rpm}")
                    
                    if rpm == 0:
                        if zero_rpm_start_time is None:
                            zero_rpm_start_time = time.time()
                        
                        # Check if RPM has been 0 for 3 seconds
                        if time.time() - zero_rpm_start_time >= 3 and not warning_active:
                            print("WARNING: RPM is 0! Rest or resume cycling.")
                            show_warning("WARNING: RPM is 0! Rest or resume cycling.")
                            warning_active = True
                            warning_start_time = time.time()
                    
                    else:
                        # Reset zero RPM detection if RPM rises
                        zero_rpm_start_time = None
                        warning_active = False

                    # Handle the warning and exit sequence
                    if warning_active and (time.time() - warning_start_time >= 5):
                        print("EXIT: RPM still 0 after 5 seconds, closing the program.")
                        ser.write(EXIT_COMMAND.encode())  # Send exit signal to Arduino
                        close_chrome()  # Close Google Chrome
                        sys.exit()  # Exit the Python program
                        
                except ValueError:
                    pass

            # Check for the exit signal from Arduino
            if line == EXIT_COMMAND:
                print("Arduino requested an EXIT.")
                close_chrome()  # Ensure Chrome is closed on EXIT signal
                break

            time.sleep(0.1)  # To avoid excessive CPU usage

if __name__ == "__main__":
    monitor_arduino()
