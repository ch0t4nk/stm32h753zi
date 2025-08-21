# UART Probe Host Capture

This folder contains a small helper to capture the probe firmware's identification banners over the ST-Link VCP.

Requirements

- Python 3.8+
- pyserial (install with `python -m pip install pyserial`)

Files

- `uart_probe_capture.py` — simple script to capture serial output and write a timestamped log to `scripts/logs/`.

Quick usage (PowerShell):

```powershell
# Install dependency
python -m pip install pyserial

# Run capture (replace COM5 with your ST-Link VCP port)
python .\scripts\uart_probe_capture.py --port COM5 --baud 115200 --timeout 10
```

Troubleshooting

- If you don't see data, verify SB12/SB19 are closed (user photo confirmed they are ON), and confirm the board appears as a COM port in Windows Device Manager (ST-Link VCP).
- If the script fails to open the port, check for another program holding the COM port (e.g., an IDE serial monitor). Close it and retry.
- If encoding issues occur, the script will replace invalid bytes; check `scripts/logs/` for raw representations.

Safety

- The probe firmware leaves `SAFE_NO_MOTOR_POWER` enabled; motor outputs are disabled. The host script only listens and does not send commands.
