# motor_control.md

---

## Overview
The **Motor Control** domain governs the real‑time operation of the STM32H753ZI‑based motor drive stack. It encompasses high‑level control loops (position, velocity, torque), safety interfaces, power stage management, and the integration of sensor data into the RTOS task hierarchy. This domain is tightly coupled to the **Safety**, **Telemetry**, and **Communications** domains, and is exposed to host‑test frameworks for validation and regression testing.

---

## Detailed Description
| Sub‑domain | Responsibility | Key Artifacts |
|------------|----------------|---------------|
| **Closed‑loop Control** | Implements PID/IMU‑based controllers for velocity/position/torque | `src/control/pid.c`, `src/control/imu_filter.c` |
| **Power Stage Management** | Controls PWM generation, current sensing, and fault detection | `src/pwm/pwm_driver.c`, `src/current/current_sensor.c` |
| **Sensor Fusion** | Fuses encoder, Hall, and IMU data for robust state estimation | `src/sensor/fusion.c` |
| **State Machine** | Manages drive modes (idle, acceleration, coast, emergency stop) | `src/state/state_machine.c` |
| **Diagnostics & Logging** | Publishes diagnostic events to the telemetry domain | `src/diag/diag_logger.c` |

All header files referenced here are part of the **SSOT** (`src/config/motor_config.h`, `src/config/hardware_config.h`, `src/config/workspace_config.h`). The SSOT manifest (`src/config/ssot.yaml`) guarantees a single source of truth for configuration values such as `MOTOR_MAX_RPM`, `PWM_FREQUENCY`, and `CURRENT_LIMIT`.

The motor control domain operates under the FreeRTOS scheduler. Dedicated tasks (e.g., `vTaskMotorControl`, `vTaskPwmUpdate`) run at fixed priorities, with a 1 ms loop time for the main control loop.

---

## Algorithm Details

### 1. Velocity Controller (PID)
```c
float setpoint = desired_rpm / RPM_TO_REVS;
float measured = measured_rpm / RPM_TO_REVS;

error = setpoint - measured;
integral += error * dt;
derivative = (error - prev_error) / dt;

output = Kp * error + Ki * integral + Kd * derivative;
output = clamp(output, -PWM_LIMIT, PWM_LIMIT);

prev_error = error;
```
*Integral anti‑windup* is applied via clamping to the PWM limits. Tuning is performed in simulation before firmware deployment.

### 2. Current Regulation
A simple PI controller drives the H‑bridge PWM duty cycle to match the desired phase current. Current sensors feed back at 10 kHz, synchronized with the PWM update task.

### 3. Encoder Dead‑Band Compensation
To avoid jitter when the motor is near stand‑still, a dead‑band of 5 RPM is applied. Within this band, the controller output is smoothly ramped to zero to prevent oscillations.

### 4. Fault Handling
- **Over‑current**: immediate PWM shutdown and transition to *Emergency Stop* state.
- **Under‑voltage**: same transition with a log entry.
- **Over‑temperature**: throttles PWM duty cycle, then enters *Emergency Stop* if not resolved within 500 ms.

Fault flags are set via `GPIO_IRQ` callbacks and cleared only by the host when the fault condition is confirmed.

---

## Best Practices

| Topic | Recommendation | Why |
|-------|----------------|-----|
| **Configuration Management** | Keep all tunable parameters in `motor_config.h`. Do not hard‑code values in source files. | SSOT compliance ensures consistent builds across CI and local environments. |
| **Timing Consistency** | Pin the motor control task to a dedicated core if using an SMP MCU. | Avoids jitter from unrelated tasks. |
| **Code Modularity** | Separate the algorithm implementation from the hardware abstraction layer. | Easier to swap sensors or motor drivers without touching control logic. |
| **Unit Testing** | Use the host‑test framework (`tests/motor_control/*.c`) to validate each controller block with mock sensors. | Prevents regressions during firmware updates. |
| **Documentation** | Update the domain doc and SSOT comments after every parameter change. | Keeps the team aligned and satisfies the `docs/STATUS.md` audit. |
| **Safety Interlocks** | Implement double‑checked locks around shared fault flags. | Avoids race conditions that could lead to unsafe PWM outputs. |
| **Performance Profiling** | Measure CPU load of the motor task with `osTaskGetRunTimeStats()` during development. | Ensures the 1 ms loop stays within budget on the target hardware. |

---

## Reference Links

* **SSOT Manifest** – [`src/config/ssot.yaml`](src/config/ssot.yaml)  
* **Motor Configuration Header** – [`src/config/motor_config.h`](src/config/motor_config.h)  
* **HAL Driver for PWM** – [`src/pwm/pwm_driver.c`](src/pwm/pwm_driver.c)  
* **Current Sensor Abstraction** – [`src/current/current_sensor.c`](src/current/current_sensor.c)  
* **State Machine Implementation** – [`src/state/state_machine.c`](src/state/state_machine.c)  
* **Host‑Test Framework** – [`tests/motor_control/`](tests/motor_control/)  
* **FreeRTOS Documentation** – [https://www.freertos.org/](https://www.freertos.org/)  
* **STM32Cube HAL** – [https://www.st.com/en/embedded-software/stm32cube-h7.html](https://www.st.com/en/embedded-software/stm32cube-h7.html)  

---
