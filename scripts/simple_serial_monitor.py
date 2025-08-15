#!/usr/bin/env python3
"""
Simple Serial Monitor for STM32H753ZI
Monitors UART3 (USB VCP) for firmware output
"""

import serial
import serial.tools.list_ports
import sys
import time


def find_stm32_port():
    """Find STM32 Nucleo VCP port"""
    ports = list(serial.tools.list_ports.comports())

    # Look for STM32 VCP descriptors
    stm32_keywords = ["STM32", "STLink", "ST-Link", "Virtual", "VCP"]

    for port in ports:
        port_desc = str(port.description).upper()
        if any(keyword.upper() in port_desc for keyword in stm32_keywords):
            return port.device

    # If not found, list all ports for user selection
    print("STM32 port not auto-detected. Available ports:")
    for i, port in enumerate(ports):
        print(f"  {i}: {port.device} - {port.description}")

    if ports:
        try:
            choice = int(input(f"Select port (0-{len(ports)-1}): "))
            if 0 <= choice < len(ports):
                return ports[choice].device
        except (ValueError, IndexError):
            pass

    return None


def main():
    print("🔍 STM32H753ZI Serial Monitor")
    print("============================")

    # Find serial port
    port = find_stm32_port()
    if not port:
        print("❌ No suitable serial port found")
        return 1

    print(f"📡 Connecting to {port} at 115200 baud...")

    try:
        with serial.Serial(port, 115200, timeout=1) as ser:
            print("✅ Connected! Press Ctrl+C to exit")
            print("📋 Waiting for firmware output...")
            print("-" * 60)

            line_buffer = ""

            while True:
                if ser.in_waiting > 0:
                    try:
                        data = ser.read(ser.in_waiting).decode(
                            "utf-8", errors="ignore"
                        )
                        line_buffer += data

                        # Process complete lines
                        while "\n" in line_buffer:
                            line, line_buffer = line_buffer.split("\n", 1)
                            line = line.rstrip("\r")
                            if line:
                                timestamp = time.strftime("%H:%M:%S")
                                print(f"[{timestamp}] {line}")

                    except UnicodeDecodeError:
                        # Skip non-UTF8 data
                        pass

                time.sleep(0.01)  # Small delay to prevent CPU spinning

    except serial.SerialException as e:
        print(f"❌ Serial error: {e}")
        return 1
    except KeyboardInterrupt:
        print("\n🛑 Monitor stopped by user")
        return 0


if __name__ == "__main__":
    sys.exit(main())
