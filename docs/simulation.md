# Simulation

> **Anchor**: `simulation.md` (included in the `DOMAINS` list)

---

## Overview

The *Simulation* domain defines the architecture, tooling, and workflow for running firmware logic in a virtualized environment. It enables rapid unit‑and integration‑testing of the STM32H753ZI firmware without requiring physical hardware. The simulation stack is tightly coupled to the **Single Source Of Truth (SSOT)** manifest (`src/config/ssot.yaml`) and is continuously validated by the automated build and test pipeline described in `docs/STATUS.md`.

Key goals:

- **Deterministic execution** of state machines, timers, and peripheral drivers.
- **Low‑latency feedback** during development cycles.
- **Seamless integration** with CI/CD for regression detection.
- **High fidelity** emulation of the STM32H753ZI peripherals, including Cortex‑M7 core, RTOS, and peripheral IPs (GPIO, UART, SPI, I²C, CAN, ADC, etc.).

---

## Detailed Description

### Architecture

| Layer | Responsibility | Tools |
|-------|----------------|-------|
| **Hardware‑Abstraction Layer (HAL)** | Exposes peripheral APIs via the STM32Cube HAL. | `src/HAL/` |
| **Simulation Core** | Emulates the Cortex‑M7 processor and peripheral registers. | QEMU‑GDB + custom register maps |
| **Peripheral Model Library** | Implements individual peripheral behavior (e.g., `stm32h7_gpio.c`, `stm32h7_can.c`). | C files compiled into the simulation image |
| **FreeRTOS Scheduler** | Provides task scheduling, ISR handling, and real‑time guarantees. | `FreeRTOS/` |
| **Test Harness** | Drives application code, injects stimuli, and asserts expectations. | CUnit/Unity + Python bindings |
| **CI Pipeline** | Automates build, test, and artifact collection. | CMake + Ninja + GitHub Actions |

### Build Process

1. **CMake Configuration**  
   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -G Ninja -DFORCE_SIMULATION=ON
   ```
   *`FORCE_SIMULATION=ON`* switches HAL includes to simulation stubs.

2. **Compile Simulation Image**  
   The simulator image (`sim.bin`) contains the firmware ELF linked against simulation libraries.

3. **Launch QEMU**  
   ```bash
   qemu-system-arm -M stm32h7 -kernel sim.bin -semihosting -S -gdb tcp::1234
   ```

4. **Run Tests**  
   `ctest --output-on-failure` executes the full test matrix. Results are posted to `docs/STATUS.md`.

### Integration with SSOT

All header files used by the simulation are listed under `firmware_headers` in `ssot.yaml`. This guarantees that peripheral register maps and configuration constants are consistent between real firmware and simulation. Any change to these headers triggers a rebuild and re‑generation of the simulation environment.

---

## Algorithm Details

### Peripheral Emulation

Each peripheral is represented by a **State Machine** that processes register writes/reads and produces events. For example, the CAN controller operates as:

```c
switch (can_state) {
  case CAN_IDLE:
    if (tx_pending) can_state = CAN_TRANSMIT;
    break;
  case CAN_TRANSMIT:
    if (tx_complete) {
      tx_pending = false;
      can_state = CAN_IDLE;
    }
    break;
}
```

The simulator updates the state each *CPU cycle* using the QEMU event loop. Timers are implemented by incrementing a global cycle counter and checking against configured thresholds.

### Interrupt Scheduling

Interrupts are queued in a priority‑based list. When the CPU core reaches the end of an instruction sequence, the simulator dispatches the highest‑priority pending interrupt, sets the appropriate flag, and calls the ISR stub. This mimics the STM32 interrupt controller (NVIC) behavior.

### Test Harness Execution

```python
# Example Python wrapper
import simapi

simapi.set_gpio('LED', 1)
assert simapi.get_gpio('LED') == 1

simapi.send_can_frame(frame_id=0x123, data=[0xDE, 0xAD])
assert simapi.get_can_rx_queue() == [(0x123, [0xDE, 0xAD])]
```

The `simapi` module is auto‑generated from the SSOT headers and exposes a clean API for writing tests in Python or C.

---

## Best Practices

| Practice | Why It Matters | How to Implement |
|----------|----------------|------------------|
| **Keep Simulation and Firmware Build Flags In Sync** | Divergent build flags cause false positives/negatives. | Use CMake `-DFORCE_SIMULATION=ON` and maintain a single `CMakePresets.json`. |
| **Validate Peripheral Models Against Datasheets** | Inaccurate models lead to regressions when hardware is finally used. | Run automated sanity checks (e.g., timing diagrams) against the STM32H753ZI reference manual. |
| **Isolate Hardware‑Specific Code** | Reduces duplication between simulation and real firmware. | Wrap hardware accesses in macros that resolve to simulation stubs or real registers depending on `FORCE_SIMULATION`. |
| **Use Deterministic Random Seeds** | Enables repeatable test runs. | Pass `--seed` to the test harness or hard‑code seeds in the test suite. |
| **Document Test Cases in Markdown** | Improves readability and traceability. | Store test descriptions in `tests/README.md` and link to the corresponding test functions. |
| **Integrate Simulation Results into STATUS.md** | Provides a single source of truth for build health. | Auto‑generate a section in `STATUS.md` summarizing the latest simulation test counts and any failures. |
| **Keep Simulation Code in a Separate Git Branch** | Avoids accidental commits of simulation stubs into the firmware branch. | Merge simulation changes through pull requests that are reviewed by both firmware and simulation maintainers. |

---

## Reference Links

- **SSOT Manifest**: [`src/config/ssot.yaml`](src/config/ssot.yaml)
- **Status Overview**: [`docs/STATUS.md`](docs/STATUS.md)
- **Archived Domain Docs**: [`archive/outdated_docs/DOCS_BKUP_08-18-2025`](archive/outdated_docs/DOCS_BKUP_08-18-2025)
- **Simulation Source**: [`src/simulation/`](src/simulation/)
- **CMake Build Script**: [`CMakeLists.txt`](CMakeLists.txt)
- **QEMU Command Reference**: https://qemu.readthedocs.io/en/latest/system/arm/arm.html
- **FreeRTOS Scheduler Docs**: https://www.freertos.org/a00110.html
- **STM32H753ZI Reference Manual**: https://www.st.com/resource/en/reference_manual/dm00439884.pdf
- **Python Simulation API**: [`simapi/__init__.py`](simapi/__init__.py)
- **CI Workflow Configuration**: `.github/workflows/ci.yml`

--- 

*End of simulation domain documentation.*
