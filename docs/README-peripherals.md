# Peripheral Map (living document)

This file lists the intended peripherals for the STM32H753ZI firmware and their current initialization / traceability status. Keep this file updated as features and hardware bring-up progress.

| Peripheral         | Init location (file)                                               | ISR owner / callback                        | DMA used                           | Notes / Next steps                                                                            |
| ------------------ | ------------------------------------------------------------------ | ------------------------------------------- | ---------------------------------- | --------------------------------------------------------------------------------------------- |
| UART3 (debug)      | `src/application/uart_test.c`, `src/communication/comm_protocol.c` | HAL IRQ -> `comm_uart_rx_complete_callback` | Yes (RX/TX DMA used by comm stack) | Used for debug/status print and command interface. Confirm hdma mapping and document in code. |
| FDCAN1             | `src/communication/comm_protocol.c` (handles)                      | HAL FDCAN callbacks                         | No (currently)                     | FDCAN used for motor CAN messages; add DMA if high throughput required.                       |
| TIM2 / TIM3 / TIM6 | `Core/Src/main.c` / `Core/Src/stm32h7xx_it.c`                      | HAL TIM callbacks (control loop, safety)    | No                                 | TIM2 used for control loop handler; TIM3 for safety monitor; TIM6 for HAL tick.               |
| ADC1 / ADC2        | (not fully integrated)                                             | (TBD)                                       | Potentially (DMA for streaming)    | Add ADC + DMA plan if sensors need streaming.                                                 |
| SPI (L6470)        | `src/drivers/` (L6470 driver)                                      | Polling / DMA optional                      | Possibly (future)                  | L6470 SPI currently uses HAL SPI; consider DMA for bulk transfers.                            |
| GPIO / LEDs        | `src/application/uart_test.c`, `src/config/hardware_config.h`      | N/A                                         | No                                 | Visual debug signals.                                                                         |

## How to update

- When adding or changing a peripheral, update this table with the file path(s), ISR ownership, and whether DMA is used.
- Use inline comments at the top of peripheral source files to reference this table.

## Phase 5: Runtime validation pointers

- Record simple pass/fail checks for each peripheral here (boot, loopback, DMA transfer) and link to test scripts in `scripts/`.

## Concrete file references (where to find MX/HAL DMA bindings)

For quick traceability, here are repository locations that show the USART3 DMA bindings used as the canonical example for this project:

- `archive/development_artifacts/instruction_archive/granular_instructions/stm32h7-uart-protocol.instructions.md`

  - Example lines (search inside file):
    - `hdma_usart3_rx.Instance = DMA1_Stream1` (RX stream) — `archive/development_artifacts/instruction_archive/granular_instructions/stm32h7-uart-protocol.instructions.md:122`
    - `hdma_usart3_tx.Instance = DMA1_Stream3` (TX stream) — `archive/development_artifacts/instruction_archive/granular_instructions/stm32h7-uart-protocol.instructions.md:130`
    - `__HAL_LINKDMA(&huart3, hdmarx, hdma_usart3_rx)` — `archive/development_artifacts/instruction_archive/granular_instructions/stm32h7-uart-protocol.instructions.md:134`
    - `__HAL_LINKDMA(&huart3, hdmatx, hdma_usart3_tx)` — `archive/development_artifacts/instruction_archive/granular_instructions/stm32h7-uart-protocol.instructions.md:134`

- `src/application/uart_verification_test.c`

  - Contains runtime checks for `huart3.hdmatx` / `huart3.hdmarx` to validate DMA presence during tests.

- MX/STM32Cube generated hooks: Search for `HAL_UART_MspInit` implementations in `src/` and `Core/` to find exact board-level DMA initialization code; HAL samples are under `stm32h753zi_spn2_mcsdk_rtos/` and may contain weak `HAL_UART_MspInit` definitions.

Use these file references when adding or validating DMA annotations in source files.

## Authoritative file:line anchors (verified)

The following file:line anchors were located in the repository and can be used as authoritative references when documenting or validating DMA bindings for USART3:

- `src/application/uart_test_fixed.c` — `HAL_UART_MspInit` implementation (USART3 GPIO + clock) — see function start at approximately line 284 in that file.
- `archive/old_hal_rtos/Core/Src/stm32h7xx_hal_msp.c` — `HAL_UART_MspInit` (USART3 MSP hooks used in archived board code) — see function at approximately line 360 in that file.
- `archive/development_artifacts/instruction_archive/granular_instructions/stm32h7-uart-protocol.instructions.md` — concrete DMA bindings and examples:
  - `hdma_usart3_rx.Instance = DMA1_Stream1` (RX stream) — around line 122.
  - `hdma_usart3_tx.Instance = DMA1_Stream3` (TX stream) — around line 130.
  - `__HAL_LINKDMA(&huart3, hdmarx, hdma_usart3_rx)` — around line 134.
  - DMA head calculation example using `__HAL_DMA_GET_COUNTER(&hdma_usart3_rx)` — around line 184.

### Precise anchors added during scan

- `src/application/uart_test_fixed.c:284` — `void HAL_UART_MspInit(UART_HandleTypeDef *huart) {` (USART3 GPIO + clock)
- `archive/old_hal_rtos/Core/Src/stm32h7xx_hal_msp.c:360` — `void HAL_UART_MspInit(UART_HandleTypeDef *huart) {` (archived MSP)
- `archive/development_artifacts/instruction_archive/granular_instructions/stm32h7-uart-protocol.instructions.md:122` — `hdma_usart3_rx.Instance = DMA1_Stream1;`
- `archive/development_artifacts/instruction_archive/granular_instructions/stm32h7-uart-protocol.instructions.md:137` — `hdma_usart3_tx.Instance = DMA1_Stream3;`
- `archive/development_artifacts/instruction_archive/granular_instructions/stm32h7-uart-protocol.instructions.md:134` — `__HAL_LINKDMA(&huart3, hdmarx, hdma_usart3_rx);`
- `archive/development_artifacts/instruction_archive/granular_instructions/stm32h7-uart-protocol.instructions.md:149` — `__HAL_LINKDMA(&huart3, hdmatx, hdma_usart3_tx);`

Note: These anchors were read from the repository at scan time. If files are edited later, run a token search (e.g., `HAL_UART_MspInit`, `hdma_usart3_rx.Instance`, or `__HAL_LINKDMA`) to relocate the current lines.

Notes:

- Line numbers are the locations discovered during repository analysis and are provided as approximate anchors (they may shift with edits). Use the file paths and token searches (function name `HAL_UART_MspInit`, identifier `hdma_usart3_rx.Instance`, or token `__HAL_LINKDMA`) to find the exact current line if files have been edited.

## Additional authoritative init locations (peripheral quick map)

Below are other authoritative MSP / MX init locations discovered in the repository. These are good anchors when documenting each peripheral's init and ISR ownership:

- FDCAN (MSP):

  - `archive/old_hal_rtos/Core/Src/stm32h7xx_hal_msp.c` — `HAL_FDCAN_MspInit` implementation (see ~line 93).
  - `Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_fdcan.c` — weak `HAL_FDCAN_MspInit` declaration (HAL core) (~line 646).

- TIM (Base MSP):

  - `Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c` — weak `HAL_TIM_Base_MspInit` implementation (HAL core) (~line 372).
  - Archive HAL MSP overrides (look in `archive/old_hal_rtos/.../stm32h7xx_hal_tim.c`).

- SPI (MSP):

  - `archive/old_hal_rtos/Core/Src/stm32h7xx_hal_msp.c` — `HAL_SPI_MspInit` example (see ~line 289).
  - `Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.c` — weak `HAL_SPI_MspInit` declaration (~line 533).

- ADC (MSP):

  - `Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.c` — weak `HAL_ADC_MspInit` declaration (~line 1095); HAL calls `HAL_ADC_MspInit` where appropriate (~line 475).

- MX / Generated stubs:
  - `archive/development_scripts/project_bootstrap/create_basic_project.py` — contains a generated `MX_USART3_UART_Init` stub and call sites (useful when re-generating MX code or understanding intended MX hooks).

Use these references when adding file:line anchors for peripherals in `docs/README-peripherals.md` or when annotating source files.
