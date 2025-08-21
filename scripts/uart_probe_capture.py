"""
Host-side UART probe capture script
- Opens the configured COM port at 115200 8N1 and listens for probe banners
- Writes captured lines to a timestamped file in scripts/logs/
- Usage: python scripts/uart_probe_capture.py --port COM5 --baud 115200 --timeout 10
"""

import argparse
import serial
import time
import os
from datetime import datetime

LOG_DIR = os.path.join(os.path.dirname(__file__), 'logs')
if not os.path.isdir(LOG_DIR):
    os.makedirs(LOG_DIR, exist_ok=True)


def capture(port, baud, timeout):
    fname = os.path.join(LOG_DIR, f"uart_probe_{port}_{datetime.utcnow().strftime('%Y%m%dT%H%M%SZ')}.log")
    print(f"Opening {port} @ {baud} baud for {timeout}s, saving to {fname}")
    ser = serial.Serial(port, baud, timeout=1)
    t0 = time.time()
    with open(fname, 'w', encoding='utf-8') as f:
        while time.time() - t0 < timeout:
            line = ser.readline()
            if line:
                try:
                    text = line.decode('utf-8', errors='replace').rstrip('\r\n')
                except Exception:
                    text = repr(line)
                ts = datetime.utcnow().isoformat() + 'Z'
                out = f"{ts} {text}"
                print(out)
                f.write(out + '\n')
    ser.close()
    print(f"Capture complete: {fname}")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Capture UART probe banners via serial port')
    parser.add_argument('--port', required=True, help='Serial port (e.g. COM5 or /dev/ttyACM0)')
    parser.add_argument('--baud', type=int, default=115200, help='Baud rate (default 115200)')
    parser.add_argument('--timeout', type=int, default=10, help='Capture duration in seconds (default 10)')
    args = parser.parse_args()
    capture(args.port, args.baud, args.timeout)
