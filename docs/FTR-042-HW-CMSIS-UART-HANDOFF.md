# FTR-042 — HW-safe CMSIS UART Handoff (Manufacturer-verified)

Status: PLANNED — verification pass completed. Track ID: FTR-042.

## Purpose

Implement a safe boot-time UART handoff: use HAL for early boot telemetry, then
hand off to a CMSIS RTOS COM driver (or a CMSIS shim) without risking
peripheral/domain misconfiguration or hardware damage. All steps verified
against vendor docs in `00_reference/`.

Verified reference excerpts (sources)

- `00_reference/STM32H7/RM0433_Reference_Manual_RevX.md` — RCC, APB clock
  gating behavior for USART3.
- `00_reference/STM32H7/STM32H7_Nucleo_BSP.md` — COM1 -> USART3 mapping (PD8/PD9 AF7)
- `00_reference/STM32H7/STM32H7_HAL_Application_Notes.md` — HAL_UART_DeInit semantics
- `00_reference/X-NUCLEO-IHM02A1/X-NUCLEO-IHM02A1_Schematic_and_SBstrap.md` — shield routing
- `00_reference/STM32H7/Errata_STM32H7_Rev*.md` — device errata for peripheral gating

## Safe handoff recipe (manufacturer-verified)

1. Preconditions
   - Confirm SSOT: `src/config/comm_config.h` sets `DEBUG_UART_INSTANCE == USART3` and
     `DEBUG_UART_BAUDRATE == 115200`.
   - Confirm PD8/PD9 routing against X-NUCLEO straps and board photos.
2. Boot-time HAL initialize
   - Call MX_USART3_Init and emit boot banner via `HAL_UART_Transmit`.
3. Prepare for handoff
   - Abort DMA, disable NVIC IRQ for USART3, stop RTOS tasks using UART.
4. Handoff options
   - Safe (recommended): use CMSIS shim delegating to HAL (no HAL_DeInit).
   - Full: HAL_DeInit(&huart3) -> CMSIS Driver Initialize (only with full sign-off).
5. Post-handoff verification
   - Toggle GPIO before/after transmit and confirm host banner and toggle.
6. Rollback
   - Keep restore ELF and flashing script; verify checksum after restore.

## Sign-off template

- Hardware verifier: **\_\_\_** (initials + date)
- Firmware verifier: **\_\_\_** (initials + date)
- Test verifier: **\_\_\_** (initials + date)
- Safety officer: **\_\_\_** (initials + date)

## Artifacts

- `src/application/boot_uart_handoff.c` (template)
- `src/drivers/adaptation/driver_usart3_shim.{c,h}` (shim)
- `src/drivers/adaptation/cmsis_uart_adapter.c` (adapter changes)
- `features/feature_registry.json` (FTR-042 entry)

End of FTR-042.
