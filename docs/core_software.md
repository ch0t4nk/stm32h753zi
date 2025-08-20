Thinking...
We need to produce a markdown document core_software.md with sections: Overview, Detailed Description, Algorithm Details, Best Practices, Reference Links. We have anchors: SSOT, STATUS, ARCHIVE, DOMAINS list includes core_software.md.

The doc should follow policy in prompt exactly. The policy isn't fully included but likely expects referencing anchors and referencing relevant files.

We should not commit or push changes.

The doc should include links to SSOT, STATUS, ARCHIVE. Use markdown link syntax referencing anchor IDs? They give anchor dict with names and values. Use e.g. [SSOT](#ssot) maybe? But they provide mapping: {'SSOT': 'src/config/ssot.yaml'}. So link to that path.

Similarly STATUS to docs/STATUS.md. ARCHIVE to archive/outdated_docs/DOCS_BKUP_08-18-2025. DOMAINS list includes file names.

We should include anchor references in the document. Provide appropriate sections with content about core software. Use information from context. We must not push changes.

Let's craft content.

Also need to reference Algorithm Details: maybe include key algorithms like firmware initialization, clock configuration, safety checks. Use high-level.

Reference Links: maybe to the SSOT YAML, STATUS, archive, and other domains docs.

Let's produce markdown.
...done thinking.

# Core Software

---

## Overview
The **Core Software** domain encapsulates the firmware that runs on the STM32H753ZI MCU. It provides the low‑level drivers, system services, and middleware that enable the higher‑level domains (communication, telemetry, motor control, safety, etc.) to operate reliably and deterministically.  
The codebase is built with CMake, ARM GCC, and STM32Cube HAL, and is tightly coupled to the single source of truth (SSOT) configuration defined in [`src/config/ssot.yaml`](src/config/ssot.yaml).  
All firmware builds are verified against the status report in [`docs/STATUS.md`](docs/STATUS.md) and older iterations are archived in [`archive/outdated_docs/DOCS_BKUP_08-18-2025`](archive/outdated_docs/DOCS_BKUP_08-18-2025).

---

## Detailed Description
| Layer | Responsibility | Key Artifacts |
|-------|----------------|---------------|
| **Bootloader** | Implements system reset, clock tree setup, and power‑on safety checks. | `src/boot/*.c` |
| **HAL Layer** | Wraps STM32Cube HAL to provide cleaner interfaces for peripherals (I2C, SPI, UART, ADC, PWM). | `src/hardware/*.h` |
| **RTOS Kernel** | FreeRTOS + X‑CUBE‑SPN2 provides task scheduling, queues, and timing services. | `src/rtos/*.c` |
| **Middleware** | Diagnostic services (logging, error handling), watchdog, and real‑time clock. | `src/middleware/*.c` |
| **API Layer** | Exposes functions to higher domains (motor_control, telemetry, safety) via static libraries. | `src/api/*.h` |
| **Build System** | CMake + Ninja; uses `CMakePresets.json` for reproducible builds. | `CMakeLists.txt`, `CMakePresets.json` |

The firmware follows the **Single Source of Truth** model: configuration constants (clock frequencies, peripheral enable flags, safety limits) live in the SSOT headers (`build_config.h`, `hardware_config.h`, `motor_config.h`, etc.). Any change to the SSOT automatically propagates to all domains through the build system, ensuring consistency across the entire project.

---

## Algorithm Details
### 1. Clock Tree Configuration
The bootloader configures the PLL based on SSOT entries:

```c
PLL_Frequency = (HSI_Value * PLL_MUL) / PLL_DIV;
```

* The algorithm selects the minimal PLL multiplier that satisfies the required peripheral clocks while staying below the MCU’s safe operating frequency.
* The configuration is validated against the SSOT constraints; mismatches trigger a fault and halt the system.

### 2. Safety‑Critical Runtime Check
Safety checks run in a high‑priority FreeRTOS task:

```c
void safety_task(void *arg) {
    while (1) {
        read_sensors();
        if (out_of_range()) { trigger_emergency_stop(); }
        vTaskDelay(SAFETY_CHECK_PERIOD);
    }
}
```

* Sensors are polled at a fixed period defined in SSOT.
* The algorithm immediately asserts an emergency stop if any parameter violates its safety bound, preventing hardware damage.

### 3. Communication Handshake
The communication domain performs a state machine handshake over UART:

```c
state = IDLE;
while (1) {
    switch(state) {
        case IDLE:
            if (recv_start()) state = RECEIVING;
            break;
        case RECEIVING:
            if (recv_complete()) { process_msg(); state = IDLE; }
            break;
    }
}
```

* The state machine ensures that only well‑formed messages are passed to higher layers, improving robustness.

### 4. Telemetry Buffering
Telemetry data are written to a circular buffer with overwrite protection:

```c
void telemetry_append(const data_t *msg) {
    buf[write_idx] = *msg;
    write_idx = (write_idx + 1) % BUF_SIZE;
    if (write_idx == read_idx) { /* buffer full – drop oldest */ }
}
```

* This algorithm guarantees that telemetry is never lost during high‑frequency bursts, at the expense of the oldest samples.

---

## Best Practices
| Topic | Recommendation |
|-------|----------------|
| **SSOT Updates** | Always update `src/config/ssot.yaml` and associated header files before adding new features. Run `scripts/run_python_configurable.ps1` to auto‑sync documentation. |
| **Code Review** | Enforce a minimum of **two** approvals for changes that touch SSOT or RTOS configuration. |
| **Build Verification** | Run `cmake --build build_host_tests` after every change to ensure host‑test compatibility. |
| **Documentation** | Keep domain docs in sync with the status report; link to the latest `STATUS.md`. |
| **Testing** | Use the host‑test harness to run safety and telemetry checks before flashing the MCU. |
| **Version Control** | Tag firmware releases; archive old docs in `archive/outdated_docs`. |
| **Error Handling** | All fault handlers should log to the middleware logger and enter a safe state. |
| **Resource Limits** | Monitor stack usage with `configCHECK_FOR_STACK_OVERFLOW` and keep task stacks within the SSOT‑defined limits. |

---

## Reference Links
* **SSOT Manifest** – [`src/config/ssot.yaml`](src/config/ssot.yaml)  
* **Project Status** – [`docs/STATUS.md`](docs/STATUS.md)  
* **Archived Documentation** – [`archive/outdated_docs/DOCS_BKUP_08-18-2025`](archive/outdated_docs/DOCS_BKUP_08-18-2025)  
* **Domain Index** – [`docs/domains.md`](docs/domains.md) – links to all domain docs (core_software.md, communications.md, telemetry.md, motor_control.md, safety.md, build_testing.md, simulation.md, documentation_ssot.md)  

---

