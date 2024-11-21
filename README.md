To run the updated script for monitoring the Arduino and closing Google Chrome, you will need the following Python libraries:

Required Libraries:
pyserial: For communicating with the Arduino via the serial port.

bash
Copy code
pip install pyserial
tkinter: For showing warning messages in popups (built-in with Python, no need to install separately).

win32com: For interacting with WMI to terminate Chrome processes.

This is part of the pywin32 package.
bash
Copy code
pip install pywin32
psutil (if you want to use an alternative method for closing processes instead of WMI):

bash
Copy code
pip install psutil
Summary of Libraries:
pyserial: For serial communication with Arduino.
tkinter: For displaying GUI popups (default with Python).
pywin32: For accessing WMI to kill Chrome processes.
psutil (optional): For alternative process management.
