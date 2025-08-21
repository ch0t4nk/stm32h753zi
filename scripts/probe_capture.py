#!/usr/bin/env python3
"""Probe capture: scans COM ports and listens for short probe banners.
Usage: .venv\Scripts\python.exe scripts\probe_capture.py --baud 115200 --timeout 5
"""
import argparse
import serial
import serial.tools.list_ports
import time

parser = argparse.ArgumentParser()
parser.add_argument("--baud", type=int, default=115200)
parser.add_argument("--timeout", type=float, default=5.0, help="seconds to listen per port")
parser.add_argument("--prefix", type=str, default="PROBE:")
args = parser.parse_args()

ports = [p.device for p in serial.tools.list_ports.comports()]
if not ports:
    print("No serial ports found.")
    raise SystemExit(1)

print(f"Found ports: {ports}")
for port in ports:
    print(f"Listening on {port} at {args.baud} baud for {args.timeout}s...")
    try:
        with serial.Serial(port, args.baud, timeout=0.1) as ser:
            deadline = time.time() + args.timeout
            buffer = ""
            while time.time() < deadline:
                data = ser.read(256)
                if not data:
                    time.sleep(0.05)
                    continue
                try:
                    s = data.decode('utf-8', errors='replace')
                except Exception:
                    s = str(data)
                buffer += s
                if args.prefix in buffer:
                    # extract lines containing prefix
                    for line in buffer.splitlines():
                        if args.prefix in line:
                            print(f"{port} -> {line.strip()}")
                    break
            else:
                # timeout
                print(f"No probe banner detected on {port}.")
    except Exception as e:
        print(f"Error opening {port}: {e}")

print("Done.")
