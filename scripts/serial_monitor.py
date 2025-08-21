r"""
Simple serial monitor for the Nucleo board.
Usage: run the venv python to ensure pyserial from .venv is used:
r"C:/repos/Nucleo-H753ZI Project/code/.venv/Scripts/python.exe" scripts/serial_monitor.py --port COM5 --baud 115200

It prints lines as they arrive and exits on Ctrl-C or after --duration seconds.
"""
import argparse
import sys

try:
    import serial
except Exception as e:
    print("pyserial not installed or failed to import:", e)
    sys.exit(2)

parser = argparse.ArgumentParser()
parser.add_argument('--port', default='COM5')
parser.add_argument('--baud', type=int, default=115200)
parser.add_argument('--timeout', type=float, default=1.0)
parser.add_argument('--duration', type=float, default=None, help='Seconds to run before exiting')
args = parser.parse_args()

try:
    ser = serial.Serial(args.port, args.baud, timeout=args.timeout)
    print(f"Opened serial port: {ser.name} @ {args.baud}")
except Exception as e:
    print("Failed to open serial port:", e)
    sys.exit(1)

import time
start = time.time()
try:
    while True:
        if args.duration is not None and (time.time() - start) >= args.duration:
            break
        line = ser.readline()
        if not line:
            continue
        try:
            sys.stdout.buffer.write(line)
            sys.stdout.flush()
        except Exception:
            print(line.decode(errors='replace'))
except KeyboardInterrupt:
    print('\nSerial monitor stopped by user')
finally:
    try:
        ser.close()
    except Exception:
        pass
