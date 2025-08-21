# Phase 5 — Runtime & Measurement Validation (Template)

Purpose: capture hardware-in-loop verification steps to confirm static analysis findings with measured behavior.

1. Boot verification

- Expectation: MCU reaches main() and scheduler starts.
- Test: flash image, open serial, observe boot messages printed by UART test.
- Pass criteria: startup banner printed and green LED on.

2. Clock measurement

- Expectation: System clock runs at configured frequency (example: 480 MHz PLL output feeding CPU and APB clocks as configured).
- Test: measure SYSCLK or confirm via debugger registers (HAL_RCC_GetSysClockFreq()) logged over UART.
- Pass criteria: measured or reported frequency matches config within tolerance.

3. UART/DMA loopback

- Expectation: DMA RX/TX transfers occur without overruns for a sustained period.
- Test: send a stream of packets and confirm echo and CRC checks.
- Pass criteria: no CRC errors, no UART DMA error callbacks for a defined test window.

4. ISR/callback mapping

- Expectation: IRQ handlers call HAL IRQ handlers and HAL invokes callbacks used by application.
- Test: instrument and log handler invocation counts during test sequences.
- Pass criteria: expected callback invocations observed.

5. Peripheral functional checks (per-peripheral)

- Fill entries as needed; link to `scripts/` host-tests where available.

How to record results

- Use `reports/runtime_results/<date>-<testname>.md` and link to commit.
- Attach oscilloscope/QC measurements if available.
