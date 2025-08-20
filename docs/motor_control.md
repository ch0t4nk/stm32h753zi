Thinking...
We need to produce a markdown document for domain doc motor_control.md, with sections: Overview, Detailed Description, Algorithm Details, Best Practices, Reference Links. No commit, no push. Output only the markdown. Use anchors if needed? They provided anchors list. We can link to SSOT, STATUS, ARCHIVE, DOMAINS. We should reference these anchors. Use markdown format. The domain doc is for motor_control. Use content from context? Provide detailed explanation.

Let's craft:

- Overview: high-level of motor control, purpose, scope.
- Detailed Description: hardware, software architecture, interactions with safety, telemetry, build, SSOT integration.
- Algorithm Details: control loops, PID, state machine, fault handling, power sequencing.
- Best Practices: coding standards, unit tests, safety checks, hardware calibration, documentation.
- Reference Links: link to SSOT, STATUS, archive, domains list, other docs.

Let's incorporate anchors: SSOT, STATUS, ARCHIVE, DOMAINS. We'll link accordingly.

Make sure not to commit. Output only markdown.

Let's write.
...done thinking.

# Motor Control Domain Documentation

## Overview
The **Motor Control** domain governs the software responsible for driving the physical motors of the STM32H753ZI‑based system. It interfaces with the HAL, the safety stack, telemetry, and the host‑test framework. The domain ensures precise motion, reliable fault detection, and graceful shutdowns while satisfying the real‑time constraints of the FreeRTOS‑based firmware.

Key responsibilities:
- Drive motor current/velocity using closed‑loop control (PID/Feed‑forward).
- Monitor motor health (temperature, voltage, current limits).
- Interface with higher‑level navigation and mission planning modules.
- Provide status telemetry to the host and ground systems.
- Enforce safety interlocks and emergency stop behavior.

---

## Detailed Description
| Layer | Description | Key Components |
|-------|-------------|----------------|
| **HAL Interface** | Uses STM32Cube HAL to access PWM, ADC, and GPIO peripherals. | `HAL_TIM_PWM_Start`, `HAL_ADC_Start`, `HAL_GPIO_WritePin` |
| **Motor Driver** | PWM waveform generation, current sense, and dead‑time management. | `MotorDriver.c/h`, `motor_config.h` |
| **Control Loop** | Fixed‑point or floating‑point PID with optional feed‑forward. Runs on a 1 kHz FreeRTOS task. | `MotorControlTask()`, `motor_pid.h` |
| **State Machine** | Handles startup, normal operation, fault, and shutdown states. | `MotorStateMachine.c/h` |
| **Safety Interface** | Receives fault signals from safety module and triggers emergency stops. | `SafetyCheck()` |
| **Telemetry** | Publishes motor metrics (speed, torque, temperature) to telemetry bus. | `TelemetryPublish()` |
| **Host‑Test** | Unit and integration tests exercising the motor driver API. | `motor_host_tests/` |

The domain is fully SSOT‑aware: configuration constants (e.g., `MOTOR_MAX_CURRENT`, `MOTOR_PWM_FREQ`) are pulled from `src/config/motor_config.h` as defined in the **SSOT manifest** (`src/config/ssot.yaml`). This guarantees that all firmware builds and host tests use a single source of truth.

---

## Algorithm Details
### 1. Closed‑Loop Control
The control algorithm follows a standard PID structure with optional feed‑forward:

```c
/* Pseudo‑code for motor control tick */
int32_t error = desired_speed - measured_speed;
integral += error * dt;
derivative = (error - prev_error) / dt;

int32_t output = Kp * error + Ki * integral + Kd * derivative + Kff * desired_speed;

output = clamp(output, -MAX_PWM, MAX_PWM);
HAL_TIM_PWM_SetCompare(&htimX, CHN, output);
```

- `dt` is fixed at 1 ms (task period).
- Integral windup is prevented by clamping the integral term.
- Feed‑forward (`Kff`) accounts for known motor dynamics.

### 2. State Machine
```c
enum MotorState {
    MOTOR_STATE_IDLE,
    MOTOR_STATE_STARTUP,
    MOTOR_STATE_RUNNING,
    MOTOR_STATE_FAULT,
    MOTOR_STATE_SHUTDOWN
};

switch (state) {
    case MOTOR_STATE_IDLE:
        if (start_signal) state = MOTOR_STATE_STARTUP;
        break;
    case MOTOR_STATE_STARTUP:
        ramp_up_pwm();
        if (ready) state = MOTOR_STATE_RUNNING;
        break;
    case MOTOR_STATE_RUNNING:
        run_control_loop();
        if (fault_detected) state = MOTOR_STATE_FAULT;
        break;
    case MOTOR_STATE_FAULT:
        shutdown_motor();
        notify_safety();
        break;
    case MOTOR_STATE_SHUTDOWN:
        stop_pwm();
        state = MOTOR_STATE_IDLE;
        break;
}
```

### 3. Fault Detection
- **Over‑current**: ADC current sense > `MOTOR_MAX_CURRENT`.
- **Over‑temperature**: Thermistor reading > `MOTOR_MAX_TEMP`.
- **Under‑voltage**: Battery voltage < `MIN_BAT_VOLT`.

When any fault triggers, the state machine immediately transitions to `MOTOR_STATE_FAULT`, disables PWM, and raises an event to the safety stack.

### 4. Power Sequencing
Motor driver power‑up follows a strict sequence:
1. Enable power supply rails.
2. Initialize PWM outputs to zero.
3. Enable current sense ADC.
4. Confirm reference voltage stability before permitting startup.

The sequencing logic resides in `MotorPowerSequence.c/h`.

---

## Best Practices
| Practice | Why It Matters | Implementation |
|----------|----------------|----------------|
| **Use SSOT headers** | Eliminates configuration drift | `#include "motor_config.h"` |
| **Keep control loops deterministic** | Guarantees real‑time behavior | Run control in a dedicated FreeRTOS task with fixed priority. |
| **Avoid blocking calls in ISR** | Prevents priority inversion | Handle ADC conversions via DMA, not polling in interrupts. |
| **Validate firmware builds with host‑tests** | Early detection of API regressions | Run `build_host_tests/motor_host_tests` after every commit. |
| **Document all configuration constants** | Easier maintenance and tuning | Include descriptive comments in `motor_config.h`. |
| **Implement graceful degradation** | Safety first | On fault, reduce PWM to zero and enter `MOTOR_STATE_FAULT`. |
| **Use hardware timers for PWM** | Accurate duty‑cycle resolution | Configure `TIMx` in PWM mode with dead‑time insertion. |
| **Monitor and log telemetry** | Enables post‑flight analysis | Publish to telemetry bus via `TelemetryPublish()` at 10 Hz. |
| **Enforce code style** | Reduces bugs | Follow `docs/STATUS.md` guidelines and run clang‑format. |
| **Use unit tests for PID tuning** | Validates mathematical correctness | Mock sensor inputs and verify controller output. |

---

## Reference Links
- **SSOT Manifest** – [src/config/ssot.yaml](src/config/ssot.yaml) (Anchor: `SSOT`)
- **Project Status** – [docs/STATUS.md](docs/STATUS.md) (Anchor: `STATUS`)
- **Outdated Docs Archive** – [archive/outdated_docs/DOCS_BKUP_08-18-2025](archive/outdated_docs/DOCS_BKUP_08-18-2025) (Anchor: `ARCHIVE`)
- **Domain List** – [docs/DOMAINS.md](docs/DOMAINS.md) (Anchor: `DOMAINS`)
- **Motor Configuration Header** – [src/config/motor_config.h](src/config/motor_config.h)
- **Telemetry API** – [src/telemetry/telemetry.h](src/telemetry/telemetry.h)
- **Safety Stack** – [src/safety/safety.h](src/safety/safety.h)
- **FreeRTOS Task** – [src/tasks/motor_task.c](src/tasks/motor_task.c)

> **Note:** All links are relative to the repository root. Please ensure that the referenced files exist and are up‑to‑date before building the documentation.

