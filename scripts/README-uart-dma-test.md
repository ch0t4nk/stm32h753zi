# UART DMA Host/Device Test

This folder contains a simple host-side test script and guidance to validate the device's UART DMA circular RX + TX echo behavior.

Files

- `uart_dma_host_test.py` : Host script to send data blocks and verify echoes.

Quick usage (Windows PowerShell):

```powershell
# Activate repo venv if needed
& .\.venv\Scripts\Activate.ps1
python .\scripts\uart_dma_host_test.py --port COM5 --baud 115200 --size 512 --iters 4
```

Device-side notes

- Build a test firmware with `TEST_UART_DMA` defined and include `src/tests/uart_dma_device_test.c`.
- Call `uart_dma_device_test_start()` from `main()` after `HAL_Init()` and UART init.

The host script requires `pyserial` (`pip install pyserial`).
