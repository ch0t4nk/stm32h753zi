# safety.md

## Overview
The **Safety** domain documents the design, configuration, and verification of safety‑critical features in the STM32H753ZI firmware.  
It covers:

- The safety‑critical configuration header `src/config/safety_config.h` (SSOT)  
- Runtime watchdog and fault‑handling mechanisms  
- Redundancy and fail‑safe motor‑control modes  
- Compliance with ISO 26262 (ASIL B) and MISRA‑C:2012 guidelines  

All safety‑related data are sourced from the single‑source‑of‑truth (SSOT) manifest (`src/config/ssot.yaml`) and are automatically refreshed during the CI pipeline.

> **Anchor**: `SSOT` → `src/config/ssot.yaml`  
> **Anchor**: `STATUS` → `docs/STATUS.md`  

## Detailed Description
| Feature | Implementation | SSOT Source |
|---------|----------------|-------------|
| **Core Watchdog** | Independent watchdog (IWDG) fed every 100 ms from the RTOS tick; triggers a system reset on fault | `src/config/safety_config.h` |
| **Motor Fault Detection** | Hall‑sensor fault flags, current over‑limit detection, and encoder slip monitoring trigger a safe‑stop routine | `src/config/motor_config.h` |
| **Redundant Communication** | Dual CAN bus (CAN FD) with message authentication codes (MAC) to detect and recover from bus‑level faults | `src/config/comm_config.h` |
| **Power‑On Self Test (POST)** | Boot‑time checks of RAM, flash, peripheral clocks, and safety configuration parameters; failures halt boot | `src/config/firmware_headers/` |
| **Software‑in‑the‑Loop (SIL) Safety Tests** | Host‑test harnesses in `build_host_tests` simulate fault conditions; all tests are run on every CI run | `build_host_tests/` |
| **Logging & Telemetry** | Safe‑mode telemetry packets include fault codes, timestamps, and stack traces; transmitted over CAN FD | `src/config/telemetry_config.h` |

The safety subsystem is tightly coupled with the **Motor Control** domain; motor fault handlers are invoked by the motor‑control scheduler and may initiate a safe‑stop sequence that sets a global fault flag accessible by the safety watchdog.

## Algorithm Details
### Watchdog Feeding Algorithm
```c
void feed_watchdog(void)
{
    static uint32_t last_feed = 0;
    uint32_t now = osKernelGetTickCount();

    /* Feed every 100 ms */
    if ((now - last_feed) >= 100)
    {
        /* IWDG kick */
        IWDG->KR = 0xAAAA;
        last_feed = now;
    }
}
```
*The watchdog is fed in the idle hook to guarantee execution even if RTOS tasks starve.*

### Safe‑Stop Procedure
```c
void trigger_safe_stop(void)
{
    /* Disable motor drivers */
    HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_RESET);

    /* Halt all motor control tasks */
    osTaskSuspendAll();

    /* Enter low‑power mode until manual reset */
    while (!HAL_GPIO_ReadPin(RESET_BUTTON_GPIO_Port, RESET_BUTTON_Pin))
    {
        __WFI();          // wait for interrupt
    }

    /* Restart system */
    NVIC_SystemReset();
}
```
*The procedure guarantees that the motors are de‑energised before the MCU resets, preventing any dangerous motion.*

### Fault Code Encoding
Fault codes are 16‑bit values:  
- Bits 15‑12: Fault class (0x1 – Motor, 0x2 – Communication, 0x3 – Power)  
- Bits 11‑8 : Fault subtype (e.g., 0x01 – Over‑current)  
- Bits 7‑0  : Instance number (0‑255)

The encoding allows for deterministic lookup in `safety_fault_lookup_table[]` and concise telemetry packets.

## Best Practices
1. **SSOT Consistency**  
   - All safety constants must be defined in `safety_config.h`.  
   - Changes to safety values trigger a CI build failure if not reflected in `ssot.yaml`.  

2. **Fail‑Safe Design**  
   - Motor drivers are disabled by default on boot.  
   - Any unhandled exception routes to `trigger_safe_stop()`.

3. **Unit & Integration Testing**  
   - Host‑tests in `build_host_tests/safety_tests/` exercise every fault path.  
   - Continuous Integration runs these tests on every push to `main`.

4. **Documentation**  
   - Keep this domain file updated with any new safety feature.  
   - Reference the **Telemetry** domain for details on fault code transmission.

5. **Hardware Verification**  
   - Validate watchdog timeout values on target hardware with a logic analyzer.  
   - Confirm that fault detection thresholds match the hardware spec sheet.

6. **Regulatory Compliance**  
   - Ensure all MISRA‑C:2012 rule violations are documented and justified.  
   - Maintain a safety case matrix linking each safety requirement to its implementation.

## Reference Links
- **SSOT Manifest** – [src/config/ssot.yaml](../src/config/ssot.yaml)  
- **Project Status** – [docs/STATUS.md](../docs/STATUS.md)  
- **Motor Control Domain** – [docs/motor_control.md](../docs/motor_control.md)  
- **Telemetry Domain** – [docs/telemetry.md](../docs/telemetry.md)  
- **Communications Domain** – [docs/communications.md](../docs/communications.md)  
- **Build & Testing** – [docs/build_testing.md](../docs/build_testing.md)  
- **Simulation** – [docs/simulation.md](../docs/simulation.md)  

---
