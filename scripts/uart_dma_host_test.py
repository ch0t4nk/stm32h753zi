#!/usr/bin/env python3
"""
uart_dma_host_test.py

Simple host-side test script for UART DMA device test.

Usage: Adjust SERIAL_PORT and BAUD to match the device and run:
    python scripts/uart_dma_host_test.py --port COM5 --baud 115200

This script sends a block of data and expects an echo from the device.
It performs a few iterations and reports pass/fail.
"""
import argparse
import serial
import time


def make_pattern(length):
    return bytes((i & 0xFF) for i in range(length))


def run_test(port, baud, block_size, iterations, timeout):
    with serial.Serial(port, baud, timeout=timeout) as s:
        time.sleep(0.2)  # let device settle
        for i in range(iterations):
            payload = make_pattern(block_size)
            s.write(payload)
            # read expected number of bytes
            received = s.read(len(payload))
            if received != payload:
                print(f"Iteration {i}: MISMATCH (sent {len(payload)} bytes, got {len(received)})")
                return False
            else:
                print(f"Iteration {i}: OK ({len(payload)} bytes)")
        return True


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--port", required=True)
    p.add_argument("--baud", type=int, default=115200)
    p.add_argument("--size", type=int, default=512)
    p.add_argument("--iters", type=int, default=4)
    p.add_argument("--timeout", type=float, default=2.0)
    args = p.parse_args()

    ok = run_test(args.port, args.baud, args.size, args.iters, args.timeout)
    if ok:
        print("TEST PASSED")
        return 0
    else:
        print("TEST FAILED")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
