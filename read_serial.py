#!/usr/bin/env python3
"""
Simple serial monitor for STM32H753ZI FTR-019 domain integration testing
"""
import serial
import time
import sys

def read_serial_output(port='COM5', baudrate=115200, timeout=5):
    """Read serial output from STM32H753ZI for specified duration"""
    try:
        # Open serial port
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"Connected to {port} at {baudrate} baud")
        print("Reading serial output for FTR-019 domain integration analysis...")
        print("=" * 50)
        
        start_time = time.time()
        data_received = False
        
        while (time.time() - start_time) < timeout:
            if ser.in_waiting > 0:
                data = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
                print(data, end='')
                data_received = True
            time.sleep(0.1)
        
        if not data_received:
            print(f"No data received in {timeout} seconds")
            print("Firmware may not be outputting data or COM port issue")
            
        ser.close()
        return data_received
        
    except serial.SerialException as e:
        print(f"Serial port error: {e}")
        return False
    except Exception as e:
        print(f"Unexpected error: {e}")
        return False

if __name__ == "__main__":
    port = sys.argv[1] if len(sys.argv) > 1 else 'COM5'
    success = read_serial_output(port)
    sys.exit(0 if success else 1)
