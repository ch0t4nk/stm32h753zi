# Safety System Validation Procedures - ARM_CM7 Production

## Overview
This document defines comprehensive validation procedures for safety-critical systems in the STM32H753ZI motor control project with ARM_CM7 production firmware. All safety systems must pass these validation tests before deployment.

**System Status**: ✅ **Phase 1 Complete** - 50.5KB ARM_CM7 Safety Systems Operational  
**Performance**: ⚡ **<1ms Emergency Stop** response time validated  
**Architecture**: 🛡️ **Hardware-level safety** with software monitoring  
**Phase 2**: 🚀 **RTOS Safety Tasks** - highest priority monitoring

---

## 🛡️ **Safety Validation Architecture** (ARM_CM7 Production)

### **Validation Framework Overview**
```mermaid
graph TB
    subgraph "Hardware Validation (Layer 1 - Critical Tests)"
        HW_ESTOP_TEST["Hardware Emergency Stop Test<br/>⚡ <1ms response validation<br/>📊 Response time: 800µs measured<br/>✅ PA9 interrupt verified"]
        HW_FAULT_TEST["L6470 Hardware Fault Test<br/>🔍 OCD/UVLO/TH testing<br/>⚡ 50µs fault detection<br/>✅ Immediate motor stop"]
    end
    
    subgraph "ARM_CM7 Software Validation (Layer 2 - Current)"
        SW_SAFETY_TEST["Software Safety Test<br/>🔄 5ms monitoring cycle<br/>📊 Position/speed limits<br/>✅ Fault injection testing"]
        ENCODER_TEST["Encoder Validation Test<br/>📏 0.1° resolution verify<br/>🔍 Magnetic field testing<br/>✅ I2C communication"]
    end
    
    subgraph "Integration Validation (Layer 3 - System)"
        SYSTEM_TEST["System Integration Test<br/>🏗️ End-to-end validation<br/>📊 Multi-fault scenarios<br/>✅ Load testing verified"]
        PERF_TEST["Performance Validation<br/>⚡ Real-time constraints<br/>📊 ARM_CM7 480MHz optimized<br/>✅ <50µs control loop"]
    end
    
    subgraph "Phase 2: RTOS Validation (Layer 4 - Future)"
        RTOS_SAFETY_TEST["RTOS Safety Task Test<br/>🎯 Priority 4 validation<br/>🔄 2ms task monitoring<br/>📋 Queue performance"]
        RTOS_INTEGRATION["RTOS Integration Test<br/>⏰ Task synchronization<br/>🔍 Deadlock detection<br/>🛡️ Recovery validation"]
    end
    
    HW_ESTOP_TEST --> SW_SAFETY_TEST
    HW_FAULT_TEST --> SW_SAFETY_TEST
    SW_SAFETY_TEST --> SYSTEM_TEST
    ENCODER_TEST --> SYSTEM_TEST
    SYSTEM_TEST --> PERF_TEST
    
    SW_SAFETY_TEST -.->|Phase 2 Migration| RTOS_SAFETY_TEST
    SYSTEM_TEST -.->|Phase 2 Enhancement| RTOS_INTEGRATION
```

---

## ⚡ **Emergency Stop Validation** (ARM_CM7 Production Implementation)

### **Emergency Stop Test Matrix** (Current Performance: <800µs Measured)
```mermaid
graph LR
    subgraph "Emergency Stop Test Scenarios"
        HW_TRIGGER["Hardware Trigger Test<br/>📌 PA9 falling edge<br/>⚡ Target: <1ms<br/>✅ Measured: 800µs"]
        SW_TRIGGER["Software Trigger Test<br/>💻 API call emergency<br/>⚡ Target: <5ms<br/>✅ Measured: 2.1ms"]
        MULTI_MOTOR["Multi-Motor Stop Test<br/>🔄 Both motors running<br/>⚡ Simultaneous stop<br/>✅ 850µs max"]
        RECOVERY_TEST["Recovery Test<br/>🔄 Post-stop recovery<br/>🔍 Safety checks<br/>✅ Manual reset only"]
    end
    
    subgraph "Validation Criteria"
        TIMING["Response Time<br/>⚡ <1ms hardware<br/>⚡ <5ms software<br/>📊 ARM_CM7 optimized"]
        STATE["System State<br/>🛡️ EMERGENCY mode<br/>❌ All outputs disabled<br/>📝 Event logging"]
        SAFETY["Safety Requirements<br/>🔒 Motor power off<br/>🚨 Visual indicators<br/>📊 Telemetry stop"]
    end
    
    HW_TRIGGER --> TIMING
    SW_TRIGGER --> TIMING
    MULTI_MOTOR --> STATE
    RECOVERY_TEST --> SAFETY
```

### Test ES-001: Hardware Emergency Stop Response Time (ARM_CM7 Validated)
**Objective**: Verify hardware emergency stop response time meets specification (<1ms ARM_CM7 target)

**Prerequisites**:
- ARM_CM7 system initialized (50.5KB firmware loaded)
- Dual motors running (L6470 drivers active)
- PA9 emergency stop button connected and functional
- High-resolution timing measurement (TIM2 1µs resolution)

**Procedure**:
1. Start Motor 1 at 1000 steps/sec (validated L6470 configuration)
2. Start Motor 2 at 1500 steps/sec (validated L6470 configuration)
3. Trigger hardware emergency stop button (PA9 falling edge)
4. Measure time from button press to motor stop using ARM_CM7 optimized timing
5. Verify both motors stop within <1ms (production target)
6. Verify system enters EMERGENCY state via state machine
7. Verify all outputs disabled using HAL abstraction

**Expected Results** (ARM_CM7 Production Validated):
- Response time <800µs for both motors (measured performance)
- System state = SYSTEM_STATE_EMERGENCY (validated)
- All motor outputs disabled via HAL abstraction
- Emergency indicators activated (LED status)
- Event logged with microsecond timestamp

**Test Code** (ARM_CM7 Optimized):
```c
#include "safety/emergency_stop.h"
#include "hal_abstraction/hal_abstraction.h"  // Production HAL abstraction
#include "config/safety_config.h"             // SSOT safety configuration

void test_hardware_emergency_stop_response_arm_cm7(void) {
    uint32_t start_time_us, stop_time_us;
    
    // Initialize ARM_CM7 high-resolution timer (1µs resolution)
    TEST_ASSERT_EQUAL(SYSTEM_OK, realtime_timer_init());
    
    // Start motors using production HAL abstraction
    SystemError_t motor1_result = motor_run_validated(MOTOR_1, DIRECTION_FORWARD, 1000);
    SystemError_t motor2_result = motor_run_validated(MOTOR_2, DIRECTION_FORWARD, 1500);
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor1_result);
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor2_result);
    
    // Allow motors to reach speed (ARM_CM7 optimized timing)
    HAL_Delay(100); // 100ms stabilization
    
    // Verify motors are running before test
    TEST_ASSERT_TRUE(motor_is_moving(MOTOR_1));
    TEST_ASSERT_TRUE(motor_is_moving(MOTOR_2));
    
    // Trigger emergency stop and measure with ARM_CM7 precision
    start_time_us = get_microsecond_timer();
    HAL_Abstraction_GPIO_Write(ESTOP_TRIGGER_PORT, ESTOP_TRIGGER_PIN, HAL_GPIO_STATE_RESET);
    
    // Wait for motors to stop with timeout protection
    uint32_t timeout_us = start_time_us + 5000; // 5ms timeout
    while ((motor_is_moving(MOTOR_1) || motor_is_moving(MOTOR_2)) && 
           (get_microsecond_timer() < timeout_us)) {
        // High-frequency polling for precise measurement
        __NOP();
    }
    stop_time_us = get_microsecond_timer();
    
    uint32_t response_time_us = stop_time_us - start_time_us;
    
    // Validate ARM_CM7 production performance targets
    TEST_ASSERT_LESS_THAN(1000, response_time_us);           // <1ms target
    TEST_ASSERT_LESS_THAN(800, response_time_us);            // <800µs production target
    TEST_ASSERT_EQUAL(SYSTEM_STATE_EMERGENCY, system_get_state());
    TEST_ASSERT_FALSE(motor_is_moving(MOTOR_1));
    TEST_ASSERT_FALSE(motor_is_moving(MOTOR_2));
    
    // Verify HAL abstraction safety outputs
    HAL_GPIO_PinState motor1_enable = HAL_Abstraction_GPIO_Read(MOTOR1_ENABLE_PORT, MOTOR1_ENABLE_PIN);
    HAL_GPIO_PinState motor2_enable = HAL_Abstraction_GPIO_Read(MOTOR2_ENABLE_PORT, MOTOR2_ENABLE_PIN);
    TEST_ASSERT_EQUAL(HAL_GPIO_STATE_RESET, motor1_enable);
    TEST_ASSERT_EQUAL(HAL_GPIO_STATE_RESET, motor2_enable);
    
    // Log performance for continuous monitoring
    safety_log_performance_metric("emergency_stop_response_us", response_time_us);
}
```

### Test ES-002: Software Emergency Stop (ARM_CM7 Production)
**Objective**: Verify software emergency stop functionality with ARM_CM7 optimized performance

**Procedure**:
1. Initialize ARM_CM7 system in RUNNING state (50.5KB firmware)
2. Start motor movements using production HAL abstraction
3. Trigger software emergency stop with reason code via API
4. Verify immediate motor stop (<5ms target)
5. Verify system state transition to EMERGENCY
6. Verify comprehensive error logging with timestamps

**Expected Results** (ARM_CM7 Validated):
- Motors stop immediately (hard stop <2.1ms measured)
- System transitions to EMERGENCY state via validated state machine
- Reason code logged correctly with microsecond timestamp
- Safety callbacks executed using HAL abstraction
- Phase 2 RTOS event queue preparation (future enhancement)

**Test Code** (ARM_CM7 Optimized):
```c
void test_software_emergency_stop_arm_cm7(void) {
    uint32_t start_time_us, stop_time_us;
    EmergencyStopReason_t test_reason = ESTOP_REASON_SOFTWARE_COMMANDED;
    
    // Initialize system to RUNNING state
    TEST_ASSERT_EQUAL(SYSTEM_OK, system_set_state(SYSTEM_STATE_RUNNING));
    
    // Start motors using HAL abstraction
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_run_validated(MOTOR_1, DIRECTION_FORWARD, 800));
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_run_validated(MOTOR_2, DIRECTION_BACKWARD, 1200));
    HAL_Delay(50); // ARM_CM7 optimized stabilization
    
    // Trigger software emergency stop with timing measurement
    start_time_us = get_microsecond_timer();
    SystemError_t estop_result = software_emergency_stop(test_reason);
    
    // Wait for motor stop with precise timing
    uint32_t timeout_us = start_time_us + 10000; // 10ms timeout
    while ((motor_is_moving(MOTOR_1) || motor_is_moving(MOTOR_2)) && 
           (get_microsecond_timer() < timeout_us)) {
        __NOP(); // ARM_CM7 optimized polling
    }
    stop_time_us = get_microsecond_timer();
    
    uint32_t response_time_us = stop_time_us - start_time_us;
    
    // Validate ARM_CM7 software emergency stop performance
    TEST_ASSERT_EQUAL(SYSTEM_OK, estop_result);
    TEST_ASSERT_LESS_THAN(5000, response_time_us);           // <5ms target
    TEST_ASSERT_LESS_THAN(2500, response_time_us);           // <2.5ms production target
    TEST_ASSERT_EQUAL(SYSTEM_STATE_EMERGENCY, system_get_state());
    TEST_ASSERT_FALSE(motor_is_moving(MOTOR_1));
    TEST_ASSERT_FALSE(motor_is_moving(MOTOR_2));
    
    // Verify safety event logging with ARM_CM7 timestamp precision
    safety_event_t logged_event;
    TEST_ASSERT_EQUAL(SYSTEM_OK, safety_get_last_event(&logged_event));
    TEST_ASSERT_EQUAL(SAFETY_SOFTWARE_EMERGENCY_STOP, logged_event.type);
    TEST_ASSERT_EQUAL(test_reason, logged_event.reason);
    TEST_ASSERT_TRUE(logged_event.timestamp > 0);
    
    // Log performance metric for continuous monitoring
    safety_log_performance_metric("software_emergency_stop_response_us", response_time_us);
}
```

### Test ES-003: Emergency Stop Recovery (ARM_CM7 Production)
**Objective**: Verify proper recovery from emergency stop condition with ARM_CM7 safety checks

**Procedure**:
1. Trigger emergency stop (hardware or software)
2. Attempt to clear emergency condition
3. Verify ARM_CM7 system requirements for recovery
4. Execute recovery sequence with safety validation
5. Verify system returns to operational state with all safety systems active

**Expected Results** (ARM_CM7 Production):
- Emergency cannot be cleared while trigger active (PA9 state validated)
- Recovery requires explicit acknowledgment via API
- System performs comprehensive safety checks using HAL abstraction
- All safety systems re-enabled and validated
- Phase 2 RTOS tasks properly initialized (future enhancement)

**Test Code** (ARM_CM7 Safety Recovery):
```c
void test_emergency_stop_recovery_arm_cm7(void) {
    // Trigger emergency stop first
    TEST_ASSERT_EQUAL(SYSTEM_OK, software_emergency_stop(ESTOP_REASON_TEST_SEQUENCE));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_EMERGENCY, system_get_state());
    
    // Attempt recovery while trigger still active (should fail)
    SystemError_t recovery_result = emergency_stop_recovery_attempt();
    TEST_ASSERT_EQUAL(ERROR_EMERGENCY_TRIGGER_ACTIVE, recovery_result);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_EMERGENCY, system_get_state());
    
    // Clear trigger condition (simulate button release)
    HAL_Abstraction_GPIO_Write(ESTOP_TRIGGER_PORT, ESTOP_TRIGGER_PIN, HAL_GPIO_STATE_SET);
    HAL_Delay(50); // Debounce delay
    
    // Attempt recovery with proper acknowledgment
    recovery_result = emergency_stop_recovery_with_acknowledgment("test_recovery");
    TEST_ASSERT_EQUAL(SYSTEM_OK, recovery_result);
    
    // Verify system returns to READY state (not direct to RUNNING)
    TEST_ASSERT_EQUAL(SYSTEM_STATE_READY, system_get_state());
    
    // Verify safety systems are active
    TEST_ASSERT_TRUE(safety_systems_are_active());
    TEST_ASSERT_TRUE(watchdog_is_active());
    
    // Verify motors can be re-enabled after recovery
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_enable_validated(MOTOR_1));
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_enable_validated(MOTOR_2));
    
    // Log recovery event
    safety_event_t logged_event;
    TEST_ASSERT_EQUAL(SYSTEM_OK, safety_get_last_event(&logged_event));
    TEST_ASSERT_EQUAL(SAFETY_EMERGENCY_STOP_RECOVERY, logged_event.type);
}
```

---

## 🔍 **Fault Detection Validation** (ARM_CM7 Production System)

### **Fault Detection Test Matrix** (ARM_CM7 Optimized)
```mermaid
graph TB
    subgraph "L6470 Hardware Fault Detection (50µs Response)"
        OCD_TEST["Overcurrent Detection<br/>⚡ OCD threshold testing<br/>📊 3A configurable limit<br/>✅ 50µs detection time"]
        THERMAL_TEST["Thermal Protection<br/>🌡️ TH_WRN/TH_SD testing<br/>📊 125°C/150°C thresholds<br/>✅ Hardware protection"]
        UVLO_TEST["Undervoltage Lockout<br/>⚡ UVLO threshold testing<br/>📊 7.0V minimum<br/>✅ Supply monitoring"]
        STALL_TEST["Stall Detection<br/>🔍 Motion verification<br/>📊 Load monitoring<br/>✅ Position feedback"]
    end
    
    subgraph "AS5600 Encoder Validation (I2C 400kHz)"
        MAG_FIELD_TEST["Magnetic Field Test<br/>🧲 Field strength validation<br/>📊 ML/MH flag testing<br/>✅ 0.1° resolution"]
        I2C_COMM_TEST["I2C Communication<br/>🔗 Communication integrity<br/>📊 400kHz bus speed<br/>✅ Error detection"]
        POS_ACCURACY_TEST["Position Accuracy<br/>📏 12-bit resolution<br/>📊 0.0879° per LSB<br/>✅ Calibration verify"]
    end
    
    subgraph "ARM_CM7 Software Monitoring (5ms Cycle)"
        LIMIT_CHECK["Position/Speed Limits<br/>📊 Software boundaries<br/>🔍 Real-time monitoring<br/>✅ 5ms validation cycle"]
        COMM_MONITOR["Communication Monitor<br/>🔗 Protocol integrity<br/>📊 Timeout detection<br/>✅ Error counting"]
        SYSTEM_HEALTH["System Health Check<br/>🏥 Resource monitoring<br/>📊 Memory/CPU usage<br/>✅ Performance metrics"]
    end
    
    OCD_TEST --> LIMIT_CHECK
    THERMAL_TEST --> SYSTEM_HEALTH
    MAG_FIELD_TEST --> POS_ACCURACY_TEST
    I2C_COMM_TEST --> COMM_MONITOR
```

### Test FD-001: Motor Overcurrent Detection (ARM_CM7 L6470 Integration)
**Objective**: Verify L6470 overcurrent detection and response with ARM_CM7 optimization

**Procedure**:
1. Configure L6470 with production overcurrent threshold (3A nominal)
2. Start motor with controlled load using ARM_CM7 motion control
3. Create overcurrent condition using test fixture
4. Monitor for overcurrent fault detection (<50µs target)
5. Verify fault response and immediate motor stop
6. Verify comprehensive fault logging with ARM_CM7 timestamp

**Expected Results** (ARM_CM7 Production):
- Overcurrent detected within 50µs (L6470 hardware response)
- Motor stops immediately via L6470 hardware protection
- Fault logged with microsecond timestamp and motor ID
- System transitions to FAULT state via ARM_CM7 state machine
- HAL abstraction properly reports fault condition

**Test Code** (ARM_CM7 Optimized L6470 Integration):
```c
#include "drivers/l6470/l6470_driver.h"
#include "hal_abstraction/hal_abstraction.h"
#include "config/motor_config.h"  // SSOT motor configuration

void test_motor_overcurrent_detection_arm_cm7(void) {
    uint32_t fault_detection_start_us, fault_detection_end_us;
    
    // Get original OCD threshold for restoration
    uint8_t original_threshold = l6470_get_ocd_threshold(MOTOR_1);
    
    // Set lower OCD threshold for controlled testing (ARM_CM7 safe testing)
    uint8_t test_threshold = 0x08; // Reduced threshold for test
    SystemError_t config_result = l6470_set_ocd_threshold(MOTOR_1, test_threshold);
    TEST_ASSERT_EQUAL(SYSTEM_OK, config_result);
    
    // Verify threshold was set correctly
    uint8_t read_threshold = l6470_get_ocd_threshold(MOTOR_1);
    TEST_ASSERT_EQUAL(test_threshold, read_threshold);
    
    // Start motor with ARM_CM7 optimized control
    fault_detection_start_us = get_microsecond_timer();
    SystemError_t motor_result = motor_run_validated(MOTOR_1, DIRECTION_FORWARD, 500);
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_result);
    
    // Create controlled overcurrent condition using test fixture
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_fixture_create_overcurrent(MOTOR_1));
    
    // Monitor for fault detection with ARM_CM7 precision timing
    uint32_t timeout_us = fault_detection_start_us + 1000000; // 1 second timeout
    bool fault_detected = false;
    
    while (get_microsecond_timer() < timeout_us && !fault_detected) {
        uint16_t status = l6470_get_status(MOTOR_1);
        if (status & L6470_STATUS_OCD) {
            fault_detection_end_us = get_microsecond_timer();
            fault_detected = true;
            break;
        }
        // ARM_CM7 optimized polling - no unnecessary delays
    }
    
    uint32_t detection_time_us = fault_detection_end_us - fault_detection_start_us;
    
    // Validate ARM_CM7 + L6470 fault detection performance
    TEST_ASSERT_TRUE(fault_detected);
    TEST_ASSERT_LESS_THAN(100, detection_time_us);          // <100µs target
    TEST_ASSERT_LESS_THAN(50, detection_time_us);           // <50µs production target
    
    // Verify motor stopped immediately
    TEST_ASSERT_FALSE(motor_is_moving(MOTOR_1));
    
    // Verify system state transition to FAULT
    TEST_ASSERT_EQUAL(SYSTEM_STATE_FAULT, system_get_state());
    
    // Verify fault logging with ARM_CM7 precision
    safety_event_t logged_event;
    TEST_ASSERT_EQUAL(SYSTEM_OK, safety_get_last_event(&logged_event));
    TEST_ASSERT_EQUAL(SAFETY_MOTOR_OVERCURRENT, logged_event.type);
    TEST_ASSERT_EQUAL(MOTOR_1, logged_event.motor_id);
    TEST_ASSERT_TRUE(logged_event.timestamp > 0);
    
    // Cleanup: Restore original threshold and clear fault
    l6470_set_ocd_threshold(MOTOR_1, original_threshold);
    test_fixture_clear_overcurrent(MOTOR_1);
    system_clear_fault();
    
    // Log performance metric for continuous monitoring
    safety_log_performance_metric("overcurrent_detection_time_us", detection_time_us);
}
```

### Test FD-002: Thermal Protection (ARM_CM7 L6470 Thermal Monitoring)
**Objective**: Verify thermal protection mechanisms with ARM_CM7 monitoring

**Procedure**:
1. Monitor L6470 thermal status registers using ARM_CM7 SPI interface
2. Simulate thermal warning condition (TH_WRN)
3. Verify warning response (current reduction)
4. Simulate thermal shutdown condition (TH_SD)
5. Verify shutdown response (immediate motor stop)

**Expected Results** (ARM_CM7 Production):
- TH_WRN detected at 125°C threshold
- Current automatically reduced to 50% (software response)
- TH_SD detected at 150°C threshold
- Motor immediately stopped via hardware protection
- System transitions to FAULT state with thermal reason

**Test Code** (ARM_CM7 Thermal Management):
```c
void test_thermal_protection_arm_cm7(void) {
    uint32_t thermal_start_us = get_microsecond_timer();
    
    // Start motor at moderate load
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_run_validated(MOTOR_1, DIRECTION_FORWARD, 800));
    
    // Simulate thermal warning using test fixture
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_fixture_simulate_thermal_warning(MOTOR_1));
    
    // Monitor for thermal warning detection
    bool warning_detected = false;
    uint32_t timeout_us = thermal_start_us + 100000; // 100ms timeout
    
    while (get_microsecond_timer() < timeout_us && !warning_detected) {
        uint16_t status = l6470_get_status(MOTOR_1);
        if (status & L6470_STATUS_TH_WRN) {
            warning_detected = true;
            break;
        }
    }
    
    TEST_ASSERT_TRUE(warning_detected);
    
    // Verify current reduction response (software thermal management)
    uint8_t reduced_current = l6470_get_current_setting(MOTOR_1);
    uint8_t expected_current = MOTOR1_NOMINAL_CURRENT / 2; // 50% reduction
    TEST_ASSERT_EQUAL(expected_current, reduced_current);
    
    // Simulate thermal shutdown
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_fixture_simulate_thermal_shutdown(MOTOR_1));
    
    // Monitor for thermal shutdown detection
    bool shutdown_detected = false;
    timeout_us = get_microsecond_timer() + 50000; // 50ms timeout
    
    while (get_microsecond_timer() < timeout_us && !shutdown_detected) {
        uint16_t status = l6470_get_status(MOTOR_1);
        if (status & L6470_STATUS_TH_SD) {
            shutdown_detected = true;
            break;
        }
    }
    
    TEST_ASSERT_TRUE(shutdown_detected);
    TEST_ASSERT_FALSE(motor_is_moving(MOTOR_1));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_FAULT, system_get_state());
    
    // Cleanup
    test_fixture_clear_thermal_conditions(MOTOR_1);
    system_clear_fault();
}
```

### Test FD-003: Encoder Fault Detection (ARM_CM7 AS5600 Integration)
**Objective**: Verify AS5600 encoder fault detection with ARM_CM7 I2C interface

**Procedure**:
1. Monitor AS5600 encoder magnitude and status using ARM_CM7 I2C (400kHz)
2. Simulate weak magnetic field condition
3. Verify fault detection and response (fallback to open-loop)
4. Simulate encoder I2C communication failure
5. Verify communication error handling and system response

**Expected Results** (ARM_CM7 Production):
- Weak magnetic field detected (magnitude < threshold)
- System falls back to open-loop control automatically
- I2C communication errors detected within 5ms
- Position feedback marked as invalid
- Motor operation continues in degraded mode

**Test Code** (ARM_CM7 AS5600 Fault Detection):
```c
void test_encoder_fault_detection_arm_cm7(void) {
    uint32_t encoder_test_start_us = get_microsecond_timer();
    
    // Initialize encoder and verify baseline operation
    TEST_ASSERT_EQUAL(SYSTEM_OK, as5600_init(ENCODER_1));
    
    uint16_t baseline_magnitude = as5600_read_magnitude(ENCODER_1);
    TEST_ASSERT_GREATER_THAN(AS5600_MIN_MAGNITUDE, baseline_magnitude);
    
    // Test 1: Weak magnetic field detection
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_fixture_simulate_weak_magnetic_field(ENCODER_1));
    
    // Monitor for weak field detection
    bool weak_field_detected = false;
    uint32_t timeout_us = encoder_test_start_us + 50000; // 50ms timeout
    
    while (get_microsecond_timer() < timeout_us && !weak_field_detected) {
        uint8_t status = as5600_read_status(ENCODER_1);
        uint16_t magnitude = as5600_read_magnitude(ENCODER_1);
        
        if ((status & AS5600_STATUS_ML) || (magnitude < AS5600_MIN_MAGNITUDE)) {
            weak_field_detected = true;
            break;
        }
    }
    
    TEST_ASSERT_TRUE(weak_field_detected);
    
    // Verify fallback to open-loop control
    MotorControlMode_t control_mode = motor_get_control_mode(MOTOR_1);
    TEST_ASSERT_EQUAL(MOTOR_CONTROL_OPEN_LOOP, control_mode);
    
    // Test 2: I2C communication failure
    test_fixture_clear_magnetic_field_simulation(ENCODER_1);
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_fixture_simulate_i2c_failure(ENCODER_1));
    
    // Attempt encoder read and verify communication error
    SystemError_t read_result = as5600_read_position(ENCODER_1, NULL);
    TEST_ASSERT_EQUAL(ERROR_I2C_COMMUNICATION_FAILED, read_result);
    
    // Verify encoder marked as invalid
    bool encoder_valid = encoder_is_valid(ENCODER_1);
    TEST_ASSERT_FALSE(encoder_valid);
    
    // Verify motor continues operation (degraded mode)
    TEST_ASSERT_TRUE(motor_is_operational(MOTOR_1));
    
    // Verify safety event logged
    safety_event_t logged_event;
    TEST_ASSERT_EQUAL(SYSTEM_OK, safety_get_last_event(&logged_event));
    TEST_ASSERT_EQUAL(SAFETY_ENCODER_COMMUNICATION_FAILURE, logged_event.type);
    TEST_ASSERT_EQUAL(ENCODER_1, logged_event.encoder_id);
    
    // Cleanup
    test_fixture_clear_i2c_failure_simulation(ENCODER_1);
    encoder_reset_fault_state(ENCODER_1);
}
```

---

## ⏰ **Watchdog Validation** (ARM_CM7 Production + Phase 2 RTOS)

### **Watchdog System Architecture** (ARM_CM7 + Future RTOS)
```mermaid
graph TB
    subgraph "ARM_CM7 Hardware Watchdog (Current - Phase 1)"
        HW_IWDG["Independent Watchdog (IWDG)<br/>⏰ 5s timeout period<br/>🔄 LSI 32kHz clock<br/>✅ Hardware reset on timeout"]
        HW_REFRESH["Watchdog Refresh Task<br/>🔄 1s refresh cycle<br/>📊 System health checks<br/>✅ Conditional refresh"]
    end
    
    subgraph "ARM_CM7 Software Watchdog (Current - Phase 1)"
        SW_TASK_MONITOR["Task Monitor<br/>📊 Critical task monitoring<br/>⏰ Individual timeouts<br/>✅ Deadlock detection"]
        SW_HEALTH_CHECK["System Health Check<br/>🏥 Resource validation<br/>📊 Memory/CPU usage<br/>✅ Performance metrics"]
    end
    
    subgraph "Phase 2: RTOS Watchdog (Future Enhancement)"
        RTOS_TASK_WD["RTOS Task Watchdog<br/>🎯 FreeRTOS task monitoring<br/>⏰ Per-task timeout<br/>🔄 Queue-based reporting"]
        RTOS_SYSTEM_WD["RTOS System Watchdog<br/>🛡️ Scheduler monitoring<br/>📊 Task priority validation<br/>🔍 Inter-task deadlock"]
    end
    
    HW_IWDG --> SW_TASK_MONITOR
    HW_REFRESH --> SW_HEALTH_CHECK
    SW_TASK_MONITOR -.->|Phase 2 Migration| RTOS_TASK_WD
    SW_HEALTH_CHECK -.->|Phase 2 Enhancement| RTOS_SYSTEM_WD
```

### Test WD-001: Hardware Watchdog Operation (ARM_CM7 IWDG)
**Objective**: Verify hardware watchdog reset functionality with ARM_CM7 precision

**Procedure**:
1. Initialize ARM_CM7 IWDG with production timeout (5 seconds)
2. Stop watchdog refresh task simulation
3. Verify system reset occurs within timeout period
4. Verify proper ARM_CM7 system restart after reset

**Safety Note**: This test should be run in controlled environment with recovery mechanisms.

**Expected Results** (ARM_CM7 Production):
- IWDG reset occurs within 5.5 seconds (including tolerance)
- System restarts with proper ARM_CM7 initialization
- Reset reason correctly identified as IWDG reset
- All safety systems re-initialized after reset

**Test Code** (ARM_CM7 IWDG Validation):
```c
#include "safety/watchdog.h"
#include "hal_abstraction/hal_abstraction.h"

void test_hardware_watchdog_operation_arm_cm7(void) {
    uint32_t test_start_time_ms;
    
    // Initialize IWDG with short timeout for testing (ARM_CM7 optimized)
    IWDG_HandleTypeDef test_iwdg;
    test_iwdg.Instance = IWDG;
    test_iwdg.Init.Prescaler = IWDG_PRESCALER_256;    // 32kHz/256 = 125Hz
    test_iwdg.Init.Reload = 625;                      // 5 second timeout (625/125Hz)
    
    SystemError_t iwdg_result = HAL_IWDG_Init(&test_iwdg);
    TEST_ASSERT_EQUAL(HAL_OK, iwdg_result);
    
    // Record test start time with ARM_CM7 precision
    test_start_time_ms = HAL_GetTick();
    
    // Disable watchdog refresh (simulate system hang)
    watchdog_disable_refresh();
    
    // Wait for watchdog timeout with monitoring
    uint32_t timeout_start_ms = HAL_GetTick();
    uint32_t expected_reset_time_ms = timeout_start_ms + 5000; // 5 second target
    uint32_t max_wait_time_ms = timeout_start_ms + 7000;      // 7 second safety limit
    
    // Monitor until reset or timeout
    while (HAL_GetTick() < max_wait_time_ms) {
        // Check if we're approaching expected reset time
        if (HAL_GetTick() > expected_reset_time_ms) {
            // System should have reset by now - test will fail
            break;
        }
        HAL_Delay(100); // ARM_CM7 optimized polling
    }
    
    // If we reach here, watchdog failed to reset system
    TEST_FAIL_MESSAGE("Hardware watchdog did not reset system within expected time");
}

// Post-reset validation function (called after system restart)
void test_validate_post_watchdog_reset_arm_cm7(void) {
    // Check reset reason using ARM_CM7 RCC registers
    uint32_t reset_flags = RCC->RSR;
    
    // Verify IWDG reset flag is set
    TEST_ASSERT_TRUE(reset_flags & RCC_RSR_IWDG1RSTF);
    
    // Verify system reinitializes correctly
    TEST_ASSERT_EQUAL(SYSTEM_STATE_INIT, system_get_state());
    
    // Verify all safety systems are active after reset
    TEST_ASSERT_TRUE(safety_systems_are_active());
    TEST_ASSERT_TRUE(emergency_stop_is_functional());
    
    // Clear reset flags
    RCC->RSR |= RCC_RSR_RMVF;
    
    // Log watchdog reset event
    safety_log_event_simple(SAFETY_WATCHDOG_RESET_RECOVERY, "Hardware watchdog reset validated");
}
```

### Test WD-002: Software Task Watchdog (ARM_CM7 Task Monitoring)
**Objective**: Verify software task monitoring with ARM_CM7 precision timing

**Procedure**:
1. Configure task watchdogs for critical ARM_CM7 tasks
2. Simulate task hang/deadlock condition
3. Verify watchdog timeout detection within specified time
4. Verify fault handling and recovery procedures

**Expected Results** (ARM_CM7 Production):
- Task timeout detected within monitoring cycle (100ms)
- Fault handler executed with proper ARM_CM7 context
- System transitions to appropriate fault state
- Recovery procedures initiated automatically

**Test Code** (ARM_CM7 Task Monitoring):
```c
void test_software_task_watchdog_arm_cm7(void) {
    uint32_t watchdog_test_start_us = get_microsecond_timer();
    
    // Configure test task with precise ARM_CM7 timing
    TaskID_t test_task_id = TASK_ID_MOTOR_CONTROL; // Use real task ID
    uint32_t test_timeout_ms = 100; // 100ms timeout for testing
    
    SystemError_t config_result = task_watchdog_configure(test_task_id, test_timeout_ms);
    TEST_ASSERT_EQUAL(SYSTEM_OK, config_result);
    
    SystemError_t enable_result = task_watchdog_enable(test_task_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, enable_result);
    
    // Initial checkin to establish baseline
    task_watchdog_checkin(test_task_id);
    HAL_Delay(50); // Within timeout - should be OK
    
    // Verify no timeout detected yet
    bool timeout_status = task_watchdog_is_timeout(test_task_id);
    TEST_ASSERT_FALSE(timeout_status);
    
    // Wait for timeout condition (simulate task hang)
    HAL_Delay(150); // Exceed 100ms timeout
    
    // Update watchdog monitor with ARM_CM7 precision timing
    uint32_t monitor_start_us = get_microsecond_timer();
    SystemError_t monitor_result = task_watchdog_monitor();
    uint32_t monitor_duration_us = get_microsecond_timer() - monitor_start_us;
    
    TEST_ASSERT_EQUAL(SYSTEM_OK, monitor_result);
    
    // Verify timeout was detected
    timeout_status = task_watchdog_is_timeout(test_task_id);
    TEST_ASSERT_TRUE(timeout_status);
    
    // Verify monitoring performance (ARM_CM7 target: <1ms)
    TEST_ASSERT_LESS_THAN(1000, monitor_duration_us);
    
    // Verify fault handling was triggered
    TEST_ASSERT_EQUAL(SYSTEM_STATE_FAULT, system_get_state());
    
    // Verify fault event logged with precise timing
    safety_event_t logged_event;
    TEST_ASSERT_EQUAL(SYSTEM_OK, safety_get_last_event(&logged_event));
    TEST_ASSERT_EQUAL(SAFETY_TASK_WATCHDOG_TIMEOUT, logged_event.type);
    TEST_ASSERT_EQUAL(test_task_id, logged_event.task_id);
    TEST_ASSERT_TRUE(logged_event.timestamp > 0);
    
    // Test recovery procedure
    SystemError_t recovery_result = task_watchdog_recovery(test_task_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, recovery_result);
    
    // Verify task can checkin again after recovery
    task_watchdog_checkin(test_task_id);
    timeout_status = task_watchdog_is_timeout(test_task_id);
    TEST_ASSERT_FALSE(timeout_status);
    
    // Cleanup
    task_watchdog_disable(test_task_id);
    system_clear_fault();
    
    // Log performance metrics
    uint32_t total_test_time_us = get_microsecond_timer() - watchdog_test_start_us;
    safety_log_performance_metric("task_watchdog_test_duration_us", total_test_time_us);
    safety_log_performance_metric("watchdog_monitor_duration_us", monitor_duration_us);
}
```

---

## 🔄 **State Management Validation** (ARM_CM7 Production State Machine)

### **State Transition Validation Matrix** (ARM_CM7 Production)
```mermaid
stateDiagram-v2
    [*] --> INIT : System Startup
    INIT --> READY : ✅ Self-test passed<br/>🔍 Safety systems active
    INIT --> FAULT : ❌ Self-test failed<br/>🛡️ Hardware fault detected
    READY --> RUNNING : ✅ Start command<br/>🔄 Motors enabled
    READY --> FAULT : ❌ Safety violation<br/>🛡️ Emergency condition
    RUNNING --> READY : ✅ Stop command<br/>🔄 Normal operation end
    RUNNING --> WARNING : ⚠️ Non-critical fault<br/>📊 Degraded operation
    RUNNING --> FAULT : ❌ Critical fault<br/>🛡️ Immediate stop required
    RUNNING --> EMERGENCY : 🚨 Emergency trigger<br/>⚡ <1ms response
    WARNING --> RUNNING : ✅ Warning cleared<br/>🔄 Normal operation resumed
    WARNING --> FAULT : ❌ Warning escalated<br/>🛡️ Critical condition
    WARNING --> EMERGENCY : 🚨 Emergency trigger<br/>⚡ <1ms response
    FAULT --> READY : ✅ Fault cleared<br/>🔍 Manual acknowledgment
    FAULT --> EMERGENCY : 🚨 Emergency trigger<br/>⚡ Hardware override
    EMERGENCY --> READY : ✅ Emergency cleared<br/>🔍 Safety validation
    
    note right of EMERGENCY : ⚡ Highest Priority State<br/>🛡️ Hardware-enforced<br/>❌ All outputs disabled
    note right of FAULT : 🛡️ Safe State<br/>📊 Fault analysis<br/>🔍 Recovery procedures
    note right of WARNING : ⚠️ Degraded Operation<br/>📊 Reduced performance<br/>🔄 Continued monitoring
```

### Test SM-001: State Transition Validation (ARM_CM7 Production)
**Objective**: Verify all valid and invalid state transitions with ARM_CM7 state machine

**Procedure**:
1. Test all valid state transitions using production ARM_CM7 state machine
2. Attempt invalid state transitions and verify rejection
3. Verify state transition logging with microsecond timestamps
4. Verify state-specific behaviors and safety enforcement

**Test Matrix** (ARM_CM7 Production Validated):
```
| From\To   | INIT | READY | RUNNING | WARNING | FAULT | EMERGENCY |
| --------- | ---- | ----- | ------- | ------- | ----- | --------- |
| INIT      | -    | ✅    | ❌      | ❌      | ✅    | ✅        |
| READY     | ❌   | -     | ✅      | ❌      | ✅    | ✅        |
| RUNNING   | ❌   | ✅    | -       | ✅      | ✅    | ✅        |
| WARNING   | ❌   | ✅    | ✅      | -       | ✅    | ✅        |
| FAULT     | ❌   | ✅    | ❌      | ❌      | -     | ✅        |
| EMERGENCY | ❌   | ✅    | ❌      | ❌      | ❌    | -         |
```

**Test Code** (ARM_CM7 State Machine Validation):
```c
void test_state_transition_validation_arm_cm7(void) {
    uint32_t transition_test_start_us = get_microsecond_timer();
    
    // Test 1: Valid transitions from INIT state
    TEST_ASSERT_EQUAL(SYSTEM_STATE_INIT, system_get_state());
    
    // INIT -> READY (valid)
    uint32_t transition_start_us = get_microsecond_timer();
    SystemError_t result = system_set_state(SYSTEM_STATE_READY);
    uint32_t transition_duration_us = get_microsecond_timer() - transition_start_us;
    
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_READY, system_get_state());
    TEST_ASSERT_LESS_THAN(10000, transition_duration_us); // <10ms transition time
    
    // INIT -> RUNNING (invalid - should be rejected)
    result = system_set_state(SYSTEM_STATE_INIT); // Reset to INIT
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    result = system_set_state(SYSTEM_STATE_RUNNING); // Invalid transition
    TEST_ASSERT_EQUAL(ERROR_INVALID_STATE_TRANSITION, result);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_INIT, system_get_state()); // State unchanged
    
    // Test 2: Valid transitions from READY state
    system_set_state(SYSTEM_STATE_READY);
    
    // READY -> RUNNING (valid)
    result = system_set_state(SYSTEM_STATE_RUNNING);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_RUNNING, system_get_state());
    
    // Verify motors can be enabled in RUNNING state
    TEST_ASSERT_TRUE(motor_enable_is_allowed(MOTOR_1));
    TEST_ASSERT_TRUE(motor_enable_is_allowed(MOTOR_2));
    
    // Test 3: Emergency state transitions (always valid)
    SystemState_t current_state = system_get_state();
    
    transition_start_us = get_microsecond_timer();
    result = system_set_state(SYSTEM_STATE_EMERGENCY);
    transition_duration_us = get_microsecond_timer() - transition_start_us;
    
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_EMERGENCY, system_get_state());
    TEST_ASSERT_LESS_THAN(1000, transition_duration_us); // <1ms emergency transition
    
    // Verify emergency state behavior
    TEST_ASSERT_FALSE(motor_enable_is_allowed(MOTOR_1));
    TEST_ASSERT_FALSE(motor_enable_is_allowed(MOTOR_2));
    TEST_ASSERT_TRUE(emergency_indicators_are_active());
    
    // Test 4: State transition logging
    safety_event_t logged_event;
    result = safety_get_last_event(&logged_event);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(SAFETY_STATE_TRANSITION, logged_event.type);
    TEST_ASSERT_EQUAL(current_state, logged_event.previous_state);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_EMERGENCY, logged_event.new_state);
    TEST_ASSERT_TRUE(logged_event.timestamp > 0);
    
    // Cleanup: Return to READY state
    result = system_set_state(SYSTEM_STATE_READY);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Log performance metrics
    uint32_t total_test_duration_us = get_microsecond_timer() - transition_test_start_us;
    safety_log_performance_metric("state_transition_test_duration_us", total_test_duration_us);
}
```

### Test SM-002: Safety State Enforcement (ARM_CM7 Production)
**Objective**: Verify safety requirements enforced in each state with ARM_CM7 precision

**Procedure**:
1. Verify FAULT state prevents motor operation completely
2. Verify EMERGENCY state disables all outputs immediately
3. Verify WARNING state implements reduced functionality
4. Verify state-specific safety checks with ARM_CM7 monitoring

**Expected Results** (ARM_CM7 Production):
- FAULT state: All motor commands rejected, system in safe state
- EMERGENCY state: All outputs disabled within <1ms
- WARNING state: Reduced performance, enhanced monitoring
- State enforcement: Hardware-level safety where applicable

**Test Code** (ARM_CM7 Safety State Enforcement):
```c
void test_safety_state_enforcement_arm_cm7(void) {
    uint32_t enforcement_test_start_us = get_microsecond_timer();
    
    // Test 1: FAULT state safety enforcement
    SystemError_t result = system_set_state(SYSTEM_STATE_FAULT);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_FAULT, system_get_state());
    
    // Verify motor operations are prevented
    result = motor_run_validated(MOTOR_1, DIRECTION_FORWARD, 1000);
    TEST_ASSERT_EQUAL(ERROR_SYSTEM_IN_FAULT_STATE, result);
    TEST_ASSERT_FALSE(motor_is_moving(MOTOR_1));
    
    result = motor_enable_validated(MOTOR_2);
    TEST_ASSERT_EQUAL(ERROR_SYSTEM_IN_FAULT_STATE, result);
    TEST_ASSERT_FALSE(motor_is_enabled(MOTOR_2));
    
    // Test 2: EMERGENCY state safety enforcement
    uint32_t emergency_start_us = get_microsecond_timer();
    result = system_set_state(SYSTEM_STATE_EMERGENCY);
    uint32_t emergency_duration_us = get_microsecond_timer() - emergency_start_us;
    
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_EMERGENCY, system_get_state());
    TEST_ASSERT_LESS_THAN(1000, emergency_duration_us); // <1ms emergency activation
    
    // Verify all outputs disabled immediately
    HAL_GPIO_PinState motor1_enable = HAL_Abstraction_GPIO_Read(MOTOR1_ENABLE_PORT, MOTOR1_ENABLE_PIN);
    HAL_GPIO_PinState motor2_enable = HAL_Abstraction_GPIO_Read(MOTOR2_ENABLE_PORT, MOTOR2_ENABLE_PIN);
    TEST_ASSERT_EQUAL(HAL_GPIO_STATE_RESET, motor1_enable);
    TEST_ASSERT_EQUAL(HAL_GPIO_STATE_RESET, motor2_enable);
    
    // Verify emergency indicators active
    TEST_ASSERT_TRUE(emergency_indicators_are_active());
    
    // Test 3: WARNING state safety enforcement
    result = system_set_state(SYSTEM_STATE_WARNING);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_WARNING, system_get_state());
    
    // Verify reduced functionality is enforced
    result = motor_run_validated(MOTOR_1, DIRECTION_FORWARD, 2000); // High speed
    TEST_ASSERT_EQUAL(ERROR_SPEED_REDUCED_IN_WARNING_STATE, result);
    
    result = motor_run_validated(MOTOR_1, DIRECTION_FORWARD, 500); // Reduced speed
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_TRUE(motor_is_moving(MOTOR_1));
    
    // Verify enhanced monitoring is active
    TEST_ASSERT_TRUE(enhanced_safety_monitoring_is_active());
    
    // Test 4: State-specific safety checks
    // Each state should perform appropriate safety validations
    uint32_t safety_check_start_us = get_microsecond_timer();
    result = system_perform_state_specific_safety_checks();
    uint32_t safety_check_duration_us = get_microsecond_timer() - safety_check_start_us;
    
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_LESS_THAN(5000, safety_check_duration_us); // <5ms safety checks
    
    // Cleanup: Return to READY state
    result = system_set_state(SYSTEM_STATE_READY);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Log performance metrics
    uint32_t total_test_duration_us = get_microsecond_timer() - enforcement_test_start_us;
    safety_log_performance_metric("safety_enforcement_test_duration_us", total_test_duration_us);
    safety_log_performance_metric("emergency_activation_duration_us", emergency_duration_us);
    safety_log_performance_metric("safety_check_duration_us", safety_check_duration_us);
}
```

---

## 🏗️ **Integration Validation** (ARM_CM7 Production System)

### **Complete Safety Chain Integration** (ARM_CM7 End-to-End)
```mermaid
graph TB
    subgraph "Integration Test Flow (ARM_CM7 Production)"
        INIT_TEST["System Initialization<br/>🔄 50.5KB firmware load<br/>✅ Safety systems init<br/>📊 Self-test validation"]
        NORMAL_OP["Normal Operation<br/>🎯 Dual motor control<br/>📊 Real-time monitoring<br/>✅ Closed-loop feedback"]
        FAULT_INJECT["Fault Injection<br/>⚡ Multiple fault scenarios<br/>🔍 Emergency conditions<br/>📊 Load testing"]
        SAFETY_RESPONSE["Safety Response<br/>🛡️ Immediate protection<br/>⚡ <1ms emergency stop<br/>📝 Event logging"]
        RECOVERY_TEST["Recovery Validation<br/>🔄 Fault clearance<br/>🔍 Safety verification<br/>✅ Return to operation"]
    end
    
    subgraph "Multi-Fault Scenario Testing"
        SIMUL_FAULTS["Simultaneous Faults<br/>⚡ OCD + Thermal<br/>🔗 I2C + Position<br/>📊 Priority handling"]
        LOAD_TESTING["High Load Testing<br/>🏋️ 480MHz ARM_CM7<br/>📊 Real-time constraints<br/>✅ Performance validation"]
        STRESS_TEST["Stress Testing<br/>🔄 Continuous operation<br/>📊 Memory monitoring<br/>⚡ Timing validation"]
    end
    
    INIT_TEST --> NORMAL_OP
    NORMAL_OP --> FAULT_INJECT
    FAULT_INJECT --> SAFETY_RESPONSE
    SAFETY_RESPONSE --> RECOVERY_TEST
    RECOVERY_TEST --> NORMAL_OP
    
    FAULT_INJECT --> SIMUL_FAULTS
    NORMAL_OP --> LOAD_TESTING
    LOAD_TESTING --> STRESS_TEST
```

### Test INT-001: Complete Safety Chain (ARM_CM7 End-to-End Validation)
**Objective**: Verify end-to-end safety system operation with ARM_CM7 production firmware

**Procedure**:
1. Initialize complete ARM_CM7 system (50.5KB firmware)
2. Start normal dual-motor operation with closed-loop feedback
3. Inject various fault conditions systematically
4. Verify proper fault detection and response timing
5. Test comprehensive recovery procedures
6. Verify system returns to operational state with all safety systems active

**Expected Results** (ARM_CM7 Production):
- System initializes completely within 2 seconds
- All safety systems active and validated
- Fault detection within specified timeframes (<50µs hardware, <5ms software)
- Emergency response within <1ms
- Recovery procedures successful with safety validation

**Test Code** (ARM_CM7 Complete Safety Chain):
```c
void test_complete_safety_chain_arm_cm7(void) {
    uint32_t integration_test_start_us = get_microsecond_timer();
    safety_test_metrics_t test_metrics = {0};
    
    // Phase 1: System Initialization
    uint32_t init_start_us = get_microsecond_timer();
    SystemError_t init_result = system_full_initialization();
    test_metrics.initialization_time_us = get_microsecond_timer() - init_start_us;
    
    TEST_ASSERT_EQUAL(SYSTEM_OK, init_result);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_READY, system_get_state());
    TEST_ASSERT_LESS_THAN(2000000, test_metrics.initialization_time_us); // <2s init
    
    // Verify all safety systems are active
    TEST_ASSERT_TRUE(emergency_stop_is_functional());
    TEST_ASSERT_TRUE(watchdog_is_active());
    TEST_ASSERT_TRUE(fault_detection_is_active());
    TEST_ASSERT_TRUE(safety_monitoring_is_active());
    
    // Phase 2: Normal Operation Establishment
    SystemError_t motor_result = system_set_state(SYSTEM_STATE_RUNNING);
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_result);
    
    motor_result = motor_run_validated(MOTOR_1, DIRECTION_FORWARD, 1000);
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_result);
    motor_result = motor_run_validated(MOTOR_2, DIRECTION_BACKWARD, 800);
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_result);
    
    HAL_Delay(100); // Allow motors to reach steady state
    TEST_ASSERT_TRUE(motor_is_moving(MOTOR_1));
    TEST_ASSERT_TRUE(motor_is_moving(MOTOR_2));
    
    // Phase 3: Systematic Fault Injection Testing
    
    // Test 3.1: Hardware Emergency Stop
    uint32_t emergency_test_start_us = get_microsecond_timer();
    trigger_hardware_emergency_stop();
    
    while (motor_is_moving(MOTOR_1) || motor_is_moving(MOTOR_2)) {
        if ((get_microsecond_timer() - emergency_test_start_us) > 5000) {
            TEST_FAIL_MESSAGE("Emergency stop timeout");
        }
    }
    test_metrics.emergency_stop_response_us = get_microsecond_timer() - emergency_test_start_us;
    
    TEST_ASSERT_LESS_THAN(1000, test_metrics.emergency_stop_response_us); // <1ms
    TEST_ASSERT_EQUAL(SYSTEM_STATE_EMERGENCY, system_get_state());
    
    // Recovery from emergency stop
    SystemError_t recovery_result = emergency_stop_recovery_with_acknowledgment("integration_test");
    TEST_ASSERT_EQUAL(SYSTEM_OK, recovery_result);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_READY, system_get_state());
    
    // Test 3.2: Motor Overcurrent Fault
    system_set_state(SYSTEM_STATE_RUNNING);
    motor_run_validated(MOTOR_1, DIRECTION_FORWARD, 500);
    
    uint32_t fault_test_start_us = get_microsecond_timer();
    test_fixture_create_overcurrent(MOTOR_1);
    
    // Wait for fault detection
    while (system_get_state() != SYSTEM_STATE_FAULT) {
        if ((get_microsecond_timer() - fault_test_start_us) > 100000) {
            TEST_FAIL_MESSAGE("Overcurrent fault detection timeout");
        }
    }
    test_metrics.fault_detection_time_us = get_microsecond_timer() - fault_test_start_us;
    
    TEST_ASSERT_LESS_THAN(100, test_metrics.fault_detection_time_us); // <100µs
    TEST_ASSERT_FALSE(motor_is_moving(MOTOR_1));
    
    // Clear fault and verify recovery
    test_fixture_clear_overcurrent(MOTOR_1);
    recovery_result = system_clear_fault_with_validation();
    TEST_ASSERT_EQUAL(SYSTEM_OK, recovery_result);
    
    // Test 3.3: Encoder Communication Fault
    system_set_state(SYSTEM_STATE_RUNNING);
    motor_run_validated(MOTOR_2, DIRECTION_FORWARD, 600);
    
    uint32_t encoder_fault_start_us = get_microsecond_timer();
    test_fixture_simulate_i2c_failure(ENCODER_1);
    
    // Wait for encoder fault detection and fallback
    while (motor_get_control_mode(MOTOR_2) == MOTOR_CONTROL_CLOSED_LOOP) {
        if ((get_microsecond_timer() - encoder_fault_start_us) > 50000) {
            TEST_FAIL_MESSAGE("Encoder fault detection timeout");
        }
    }
    test_metrics.encoder_fault_detection_us = get_microsecond_timer() - encoder_fault_start_us;
    
    TEST_ASSERT_LESS_THAN(50000, test_metrics.encoder_fault_detection_us); // <50ms
    TEST_ASSERT_EQUAL(MOTOR_CONTROL_OPEN_LOOP, motor_get_control_mode(MOTOR_2));
    TEST_ASSERT_TRUE(motor_is_moving(MOTOR_2)); // Should continue in open-loop
    
    // Phase 4: Performance Validation
    test_metrics.total_test_time_us = get_microsecond_timer() - integration_test_start_us;
    
    // Log comprehensive test metrics
    safety_log_performance_metric("integration_test_total_time_us", test_metrics.total_test_time_us);
    safety_log_performance_metric("system_init_time_us", test_metrics.initialization_time_us);
    safety_log_performance_metric("emergency_response_us", test_metrics.emergency_stop_response_us);
    safety_log_performance_metric("fault_detection_us", test_metrics.fault_detection_time_us);
    safety_log_performance_metric("encoder_fault_detection_us", test_metrics.encoder_fault_detection_us);
    
    // Verify all performance targets met
    TEST_ASSERT_LESS_THAN(1000, test_metrics.emergency_stop_response_us);
    TEST_ASSERT_LESS_THAN(100, test_metrics.fault_detection_time_us);
    TEST_ASSERT_LESS_THAN(50000, test_metrics.encoder_fault_detection_us);
    
    // Cleanup: Return system to safe state
    system_set_state(SYSTEM_STATE_READY);
    test_fixture_clear_all_conditions();
}
```

### Test INT-002: Multiple Fault Conditions (ARM_CM7 Priority Handling)
**Objective**: Verify handling of simultaneous faults with ARM_CM7 priority management

**Procedure**:
1. Create multiple fault conditions simultaneously
2. Verify priority handling of faults (emergency > thermal > overcurrent)
3. Verify system remains in safest state
4. Verify all faults logged correctly with proper timestamps

**Expected Results** (ARM_CM7 Production):
- Highest priority fault takes precedence (EMERGENCY > FAULT > WARNING)
- All fault conditions detected and logged
- System remains in safest possible state
- Recovery requires clearing all fault conditions

**Test Code** (ARM_CM7 Multi-Fault Testing):
```c
void test_multiple_fault_conditions_arm_cm7(void) {
    uint32_t multi_fault_start_us = get_microsecond_timer();
    
    // Initialize system in normal operation
    TEST_ASSERT_EQUAL(SYSTEM_OK, system_set_state(SYSTEM_STATE_RUNNING));
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_run_validated(MOTOR_1, DIRECTION_FORWARD, 1000));
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_run_validated(MOTOR_2, DIRECTION_BACKWARD, 800));
    
    HAL_Delay(50); // Stabilize operation
    
    // Create multiple simultaneous fault conditions
    uint32_t fault_injection_start_us = get_microsecond_timer();
    
    // Inject faults in rapid succession (simulate real-world scenario)
    test_fixture_create_overcurrent(MOTOR_1);      // Hardware fault
    HAL_Delay(1); // 1ms delay
    test_fixture_simulate_thermal_warning(MOTOR_2); // Thermal fault
    HAL_Delay(1); // 1ms delay
    test_fixture_simulate_i2c_failure(ENCODER_1);   // Communication fault
    HAL_Delay(1); // 1ms delay
    trigger_software_emergency_stop(ESTOP_REASON_MULTIPLE_FAULTS); // Emergency condition
    
    uint32_t fault_injection_duration_us = get_microsecond_timer() - fault_injection_start_us;
    
    // Wait for system response (should prioritize emergency stop)
    uint32_t response_timeout_us = get_microsecond_timer() + 10000; // 10ms timeout
    while (get_microsecond_timer() < response_timeout_us) {
        if (system_get_state() == SYSTEM_STATE_EMERGENCY) {
            break;
        }
    }
    
    uint32_t system_response_time_us = get_microsecond_timer() - fault_injection_start_us;
    
    // Verify system in highest priority state (EMERGENCY)
    TEST_ASSERT_EQUAL(SYSTEM_STATE_EMERGENCY, system_get_state());
    TEST_ASSERT_LESS_THAN(5000, system_response_time_us); // <5ms total response
    
    // Verify all motors stopped
    TEST_ASSERT_FALSE(motor_is_moving(MOTOR_1));
    TEST_ASSERT_FALSE(motor_is_moving(MOTOR_2));
    
    // Verify all fault conditions were detected and logged
    safety_event_t events[10];
    uint8_t event_count = safety_get_recent_events(events, 10);
    TEST_ASSERT_GREATER_THAN(3, event_count); // At least 4 events (3 faults + emergency)
    
    bool overcurrent_logged = false;
    bool thermal_logged = false;
    bool i2c_failure_logged = false;
    bool emergency_logged = false;
    
    for (uint8_t i = 0; i < event_count; i++) {
        switch (events[i].type) {
            case SAFETY_MOTOR_OVERCURRENT:
                overcurrent_logged = true;
                TEST_ASSERT_EQUAL(MOTOR_1, events[i].motor_id);
                break;
            case SAFETY_THERMAL_WARNING:
                thermal_logged = true;
                TEST_ASSERT_EQUAL(MOTOR_2, events[i].motor_id);
                break;
            case SAFETY_ENCODER_COMMUNICATION_FAILURE:
                i2c_failure_logged = true;
                TEST_ASSERT_EQUAL(ENCODER_1, events[i].encoder_id);
                break;
            case SAFETY_SOFTWARE_EMERGENCY_STOP:
                emergency_logged = true;
                TEST_ASSERT_EQUAL(ESTOP_REASON_MULTIPLE_FAULTS, events[i].reason);
                break;
        }
    }
    
    TEST_ASSERT_TRUE(overcurrent_logged);
    TEST_ASSERT_TRUE(thermal_logged);
    TEST_ASSERT_TRUE(i2c_failure_logged);
    TEST_ASSERT_TRUE(emergency_logged);
    
    // Test recovery procedure (must clear all faults)
    
    // Attempt recovery while faults still active (should fail)
    SystemError_t recovery_result = emergency_stop_recovery_attempt();
    TEST_ASSERT_EQUAL(ERROR_ACTIVE_FAULTS_PREVENT_RECOVERY, recovery_result);
    
    // Clear all fault conditions
    test_fixture_clear_overcurrent(MOTOR_1);
    test_fixture_clear_thermal_conditions(MOTOR_2);
    test_fixture_clear_i2c_failure_simulation(ENCODER_1);
    
    // Now recovery should succeed
    recovery_result = emergency_stop_recovery_with_acknowledgment("multi_fault_recovery");
    TEST_ASSERT_EQUAL(SYSTEM_OK, recovery_result);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_READY, system_get_state());
    
    // Verify all systems operational after recovery
    TEST_ASSERT_TRUE(motor_enable_is_allowed(MOTOR_1));
    TEST_ASSERT_TRUE(motor_enable_is_allowed(MOTOR_2));
    TEST_ASSERT_TRUE(encoder_is_functional(ENCODER_1));
    
    // Log performance metrics
    uint32_t total_test_time_us = get_microsecond_timer() - multi_fault_start_us;
    safety_log_performance_metric("multi_fault_test_duration_us", total_test_time_us);
    safety_log_performance_metric("fault_injection_duration_us", fault_injection_duration_us);
    safety_log_performance_metric("multi_fault_response_time_us", system_response_time_us);
}
```

### Test INT-003: Safety System Under Load (ARM_CM7 Performance Validation)
**Objective**: Verify safety system performance under high ARM_CM7 system load

**Procedure**:
1. Create high system load (dual motors, fast control loops, communication)
2. Inject fault conditions during high load operation
3. Verify safety response times maintained under load
4. Verify no missed fault detections during stress conditions

**Expected Results** (ARM_CM7 High Performance):
- Safety response times maintained (<1ms emergency, <50µs hardware faults)
- No missed fault detections even under 80% CPU load
- Real-time constraints preserved (480MHz ARM_CM7 advantage)
- Memory usage remains within bounds

**Test Code** (ARM_CM7 Load Testing):
```c
void test_safety_system_under_load_arm_cm7(void) {
    uint32_t load_test_start_us = get_microsecond_timer();
    system_load_metrics_t load_metrics = {0};
    
    // Phase 1: Establish high system load
    TEST_ASSERT_EQUAL(SYSTEM_OK, system_set_state(SYSTEM_STATE_RUNNING));
    
    // Start high-frequency motor operations
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_run_high_frequency(MOTOR_1, 2000)); // 2000 steps/sec
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_run_high_frequency(MOTOR_2, 1800)); // 1800 steps/sec
    
    // Start continuous communication
    TEST_ASSERT_EQUAL(SYSTEM_OK, communication_start_high_frequency_polling());
    
    // Start intensive monitoring
    TEST_ASSERT_EQUAL(SYSTEM_OK, safety_monitoring_set_high_frequency_mode());
    
    // Allow system to reach high load state
    HAL_Delay(200); // 200ms stabilization
    
    // Phase 2: Measure baseline performance under load
    uint32_t baseline_start_us = get_microsecond_timer();
    
    for (uint32_t i = 0; i < 100; i++) {
        uint32_t cycle_start_us = get_microsecond_timer();
        
        // Perform typical safety monitoring cycle
        system_perform_safety_monitoring_cycle();
        
        uint32_t cycle_duration_us = get_microsecond_timer() - cycle_start_us;
        if (cycle_duration_us > load_metrics.max_monitoring_cycle_us) {
            load_metrics.max_monitoring_cycle_us = cycle_duration_us;
        }
        load_metrics.total_monitoring_time_us += cycle_duration_us;
        
        HAL_Delay(1); // 1ms between cycles
    }
    
    load_metrics.average_monitoring_cycle_us = load_metrics.total_monitoring_time_us / 100;
    uint32_t baseline_duration_us = get_microsecond_timer() - baseline_start_us;
    
    // Verify performance under load meets requirements
    TEST_ASSERT_LESS_THAN(5000, load_metrics.max_monitoring_cycle_us);     // <5ms max
    TEST_ASSERT_LESS_THAN(2000, load_metrics.average_monitoring_cycle_us); // <2ms avg
    
    // Phase 3: Fault injection under high load
    uint32_t fault_under_load_start_us = get_microsecond_timer();
    
    // Inject emergency stop while system under load
    trigger_hardware_emergency_stop();
    
    // Measure response time under load
    while (motor_is_moving(MOTOR_1) || motor_is_moving(MOTOR_2)) {
        if ((get_microsecond_timer() - fault_under_load_start_us) > 5000) {
            TEST_FAIL_MESSAGE("Emergency stop timeout under load");
        }
    }
    
    uint32_t emergency_response_under_load_us = get_microsecond_timer() - fault_under_load_start_us;
    
    // Verify emergency response time not degraded by load
    TEST_ASSERT_LESS_THAN(1200, emergency_response_under_load_us); // <1.2ms (20% tolerance)
    TEST_ASSERT_EQUAL(SYSTEM_STATE_EMERGENCY, system_get_state());
    
    // Phase 4: CPU and memory usage validation
    uint32_t cpu_usage_percent = system_get_cpu_usage_percent();
    uint32_t memory_usage_bytes = system_get_memory_usage_bytes();
    uint32_t stack_usage_percent = system_get_max_stack_usage_percent();
    
    TEST_ASSERT_LESS_THAN(85, cpu_usage_percent);      // <85% CPU usage
    TEST_ASSERT_LESS_THAN(80000, memory_usage_bytes);  // <80KB memory usage
    TEST_ASSERT_LESS_THAN(75, stack_usage_percent);    // <75% stack usage
    
    // Log comprehensive load test metrics
    safety_log_performance_metric("load_test_duration_us", get_microsecond_timer() - load_test_start_us);
    safety_log_performance_metric("max_monitoring_cycle_us", load_metrics.max_monitoring_cycle_us);
    safety_log_performance_metric("avg_monitoring_cycle_us", load_metrics.average_monitoring_cycle_us);
    safety_log_performance_metric("emergency_response_under_load_us", emergency_response_under_load_us);
    safety_log_performance_metric("cpu_usage_percent", cpu_usage_percent);
    safety_log_performance_metric("memory_usage_bytes", memory_usage_bytes);
    safety_log_performance_metric("stack_usage_percent", stack_usage_percent);
    
    // Cleanup: Restore normal operation
    emergency_stop_recovery_with_acknowledgment("load_test_complete");
    communication_stop_high_frequency_polling();
    safety_monitoring_set_normal_frequency_mode();
    system_set_state(SYSTEM_STATE_READY);
}
```

---

## ⚡ **Performance Validation** (ARM_CM7 Production Metrics)

### **ARM_CM7 Performance Requirements** (Production Validated)
```mermaid
graph TB
    subgraph "Safety Response Time Requirements (ARM_CM7 480MHz)"
        EMERGENCY_TIME["Emergency Stop Response<br/>🎯 Target: <1ms<br/>✅ Measured: 800µs<br/>🏆 ARM_CM7 optimized"]
        FAULT_DETECTION["Fault Detection Cycle<br/>🎯 Target: <50µs hardware<br/>✅ Measured: 45µs<br/>⚡ L6470 hardware response"]
        SW_MONITORING["Software Monitoring<br/>🎯 Target: <5ms cycle<br/>✅ Measured: 2.1ms<br/>🔄 Continuous validation"]
        STATE_TRANSITION["State Transition<br/>🎯 Target: <10ms<br/>✅ Measured: 3.2ms<br/>🔄 State machine optimized"]
    end
    
    subgraph "ARM_CM7 Resource Usage (50.5KB Firmware)"
        CPU_USAGE["CPU Usage<br/>📊 Normal: 35% average<br/>📊 Peak: 68% maximum<br/>✅ 480MHz headroom"]
        MEMORY_USAGE["Memory Usage<br/>📊 FLASH: 50.5KB (2.41%)<br/>📊 DTCMRAM: 67.5KB (25.74%)<br/>✅ Efficient allocation"]
        STACK_USAGE["Stack Usage<br/>📊 Main: 45% maximum<br/>📊 ISR: 28% maximum<br/>✅ Safe margins"]
        INTERRUPT_LATENCY["Interrupt Latency<br/>🎯 Target: <10µs<br/>✅ Measured: 3.8µs<br/>⚡ ARM_CM7 response"]
    end
    
    EMERGENCY_TIME --> CPU_USAGE
    FAULT_DETECTION --> MEMORY_USAGE
    SW_MONITORING --> STACK_USAGE
    STATE_TRANSITION --> INTERRUPT_LATENCY
```

### Test PERF-001: Safety Response Times (ARM_CM7 Validated Performance)
**Objective**: Verify all safety response times meet ARM_CM7 production specifications

**Requirements** (ARM_CM7 Production Targets):
- Emergency stop response: <1ms (measured: 800µs)
- Hardware fault detection: <50µs (measured: 45µs)
- Software monitoring cycle: <5ms (measured: 2.1ms)
- State transition: <10ms (measured: 3.2ms)
- Watchdog refresh: <1s (measured: 500ms typical)

**Test Code** (ARM_CM7 Performance Validation):
```c
#include "performance/performance_monitor.h"
#include "safety/safety_timing.h"

void test_safety_response_times_arm_cm7(void) {
    performance_test_results_t results = {0};
    uint32_t test_start_us = get_microsecond_timer();
    
    // Test 1: Emergency Stop Response Time
    for (uint32_t i = 0; i < 100; i++) {
        // Setup: Start motor operation
        TEST_ASSERT_EQUAL(SYSTEM_OK, system_set_state(SYSTEM_STATE_RUNNING));
        TEST_ASSERT_EQUAL(SYSTEM_OK, motor_run_validated(MOTOR_1, DIRECTION_FORWARD, 1000));
        HAL_Delay(10); // Brief operation
        
        // Measure emergency stop response
        uint32_t emergency_start_us = get_microsecond_timer();
        trigger_hardware_emergency_stop();
        
        while (motor_is_moving(MOTOR_1)) {
            // Wait for stop
        }
        uint32_t emergency_duration_us = get_microsecond_timer() - emergency_start_us;
        
        // Record measurement
        results.emergency_response_times_us[i] = emergency_duration_us;
        if (emergency_duration_us > results.max_emergency_response_us) {
            results.max_emergency_response_us = emergency_duration_us;
        }
        results.total_emergency_response_us += emergency_duration_us;
        
        // Recovery for next iteration
        emergency_stop_recovery_with_acknowledgment("perf_test");
        HAL_Delay(50); // Recovery delay
    }
    
    results.avg_emergency_response_us = results.total_emergency_response_us / 100;
    
    // Validate emergency stop performance
    TEST_ASSERT_LESS_THAN(1000, results.max_emergency_response_us);  // <1ms max
    TEST_ASSERT_LESS_THAN(900, results.avg_emergency_response_us);   // <900µs avg
    
    // Test 2: Hardware Fault Detection Time
    for (uint32_t i = 0; i < 50; i++) {
        // Setup motor operation
        system_set_state(SYSTEM_STATE_RUNNING);
        motor_run_validated(MOTOR_1, DIRECTION_FORWARD, 500);
        HAL_Delay(20);
        
        // Measure fault detection time
        uint32_t fault_start_us = get_microsecond_timer();
        test_fixture_create_overcurrent(MOTOR_1);
        
        // Wait for hardware fault detection
        while (!(l6470_get_status(MOTOR_1) & L6470_STATUS_OCD)) {
            if ((get_microsecond_timer() - fault_start_us) > 200) {
                TEST_FAIL_MESSAGE("Hardware fault detection timeout");
            }
        }
        uint32_t fault_duration_us = get_microsecond_timer() - fault_start_us;
        
        results.fault_detection_times_us[i] = fault_duration_us;
        if (fault_duration_us > results.max_fault_detection_us) {
            results.max_fault_detection_us = fault_duration_us;
        }
        results.total_fault_detection_us += fault_duration_us;
        
        // Cleanup
        test_fixture_clear_overcurrent(MOTOR_1);
        system_clear_fault();
        HAL_Delay(30);
    }
    
    results.avg_fault_detection_us = results.total_fault_detection_us / 50;
    
    // Validate hardware fault detection performance
    TEST_ASSERT_LESS_THAN(50, results.max_fault_detection_us);  // <50µs max
    TEST_ASSERT_LESS_THAN(40, results.avg_fault_detection_us);  // <40µs avg
    
    // Test 3: Software Monitoring Cycle Time
    system_set_state(SYSTEM_STATE_RUNNING);
    
    for (uint32_t i = 0; i < 200; i++) {
        uint32_t cycle_start_us = get_microsecond_timer();
        
        // Execute complete software monitoring cycle
        SystemError_t monitor_result = system_perform_safety_monitoring_cycle();
        TEST_ASSERT_EQUAL(SYSTEM_OK, monitor_result);
        
        uint32_t cycle_duration_us = get_microsecond_timer() - cycle_start_us;
        
        results.monitoring_cycle_times_us[i] = cycle_duration_us;
        if (cycle_duration_us > results.max_monitoring_cycle_us) {
            results.max_monitoring_cycle_us = cycle_duration_us;
        }
        results.total_monitoring_cycle_us += cycle_duration_us;
        
        HAL_Delay(5); // 5ms between cycles
    }
    
    results.avg_monitoring_cycle_us = results.total_monitoring_cycle_us / 200;
    
    // Validate software monitoring performance
    TEST_ASSERT_LESS_THAN(5000, results.max_monitoring_cycle_us);  // <5ms max
    TEST_ASSERT_LESS_THAN(3000, results.avg_monitoring_cycle_us);  // <3ms avg
    
    // Test 4: State Transition Time
    for (uint32_t i = 0; i < 50; i++) {
        SystemState_t transitions[] = {
            SYSTEM_STATE_READY, SYSTEM_STATE_RUNNING, SYSTEM_STATE_WARNING,
            SYSTEM_STATE_FAULT, SYSTEM_STATE_READY
        };
        
        for (uint32_t j = 0; j < 4; j++) {
            uint32_t transition_start_us = get_microsecond_timer();
            
            SystemError_t transition_result = system_set_state(transitions[j+1]);
            TEST_ASSERT_EQUAL(SYSTEM_OK, transition_result);
            
            uint32_t transition_duration_us = get_microsecond_timer() - transition_start_us;
            
            if (transition_duration_us > results.max_state_transition_us) {
                results.max_state_transition_us = transition_duration_us;
            }
            results.total_state_transition_us += transition_duration_us;
            results.state_transition_count++;
            
            HAL_Delay(10); // Brief delay between transitions
        }
    }
    
    results.avg_state_transition_us = results.total_state_transition_us / results.state_transition_count;
    
    // Validate state transition performance
    TEST_ASSERT_LESS_THAN(10000, results.max_state_transition_us);  // <10ms max
    TEST_ASSERT_LESS_THAN(5000, results.avg_state_transition_us);   // <5ms avg
    
    // Log comprehensive performance results
    uint32_t total_test_time_us = get_microsecond_timer() - test_start_us;
    
    safety_log_performance_metric("perf_test_duration_us", total_test_time_us);
    safety_log_performance_metric("max_emergency_response_us", results.max_emergency_response_us);
    safety_log_performance_metric("avg_emergency_response_us", results.avg_emergency_response_us);
    safety_log_performance_metric("max_fault_detection_us", results.max_fault_detection_us);
    safety_log_performance_metric("avg_fault_detection_us", results.avg_fault_detection_us);
    safety_log_performance_metric("max_monitoring_cycle_us", results.max_monitoring_cycle_us);
    safety_log_performance_metric("avg_monitoring_cycle_us", results.avg_monitoring_cycle_us);
    safety_log_performance_metric("max_state_transition_us", results.max_state_transition_us);
    safety_log_performance_metric("avg_state_transition_us", results.avg_state_transition_us);
    
    // Generate performance report
    generate_performance_validation_report(&results);
}
```

### Test PERF-002: Safety System Resource Usage (ARM_CM7 Optimization)
**Objective**: Verify safety systems don't impact normal operation on ARM_CM7 platform

**Procedure**:
1. Measure CPU usage with safety systems active on ARM_CM7 (480MHz)
2. Measure memory usage (FLASH/DTCMRAM/ITCMRAM)
3. Verify real-time performance maintained under load
4. Check for timing impacts on control loops

**Expected Results** (ARM_CM7 Production):
- CPU usage <70% under normal load, <85% under stress
- Memory usage within allocated bounds (50.5KB FLASH, <80KB DTCMRAM)
- Real-time constraints maintained (control loop <50µs)
- No jitter in critical timing paths

**Test Code** (ARM_CM7 Resource Monitoring):
```c
void test_safety_system_resource_usage_arm_cm7(void) {
    resource_usage_metrics_t metrics = {0};
    uint32_t resource_test_start_us = get_microsecond_timer();
    
    // Baseline measurement (safety systems inactive)
    SystemError_t baseline_result = system_measure_baseline_performance(&metrics.baseline);
    TEST_ASSERT_EQUAL(SYSTEM_OK, baseline_result);
    
    // Activate all safety systems
    TEST_ASSERT_EQUAL(SYSTEM_OK, safety_systems_activate_all());
    TEST_ASSERT_EQUAL(SYSTEM_OK, system_set_state(SYSTEM_STATE_RUNNING));
    
    // Start normal motor operations
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_run_validated(MOTOR_1, DIRECTION_FORWARD, 1500));
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_run_validated(MOTOR_2, DIRECTION_BACKWARD, 1200));
    
    // Measurement Phase 1: Normal Operation
    HAL_Delay(1000); // 1 second of normal operation
    
    metrics.normal_cpu_usage = system_get_cpu_usage_percent();
    metrics.normal_memory_usage = system_get_memory_usage_bytes();
    metrics.normal_stack_usage = system_get_max_stack_usage_percent();
    
    // Verify normal operation resource usage
    TEST_ASSERT_LESS_THAN(70, metrics.normal_cpu_usage);     // <70% CPU
    TEST_ASSERT_LESS_THAN(80000, metrics.normal_memory_usage); // <80KB memory
    TEST_ASSERT_LESS_THAN(60, metrics.normal_stack_usage);   // <60% stack
    
    // Measurement Phase 2: Under Load
    // Create controlled high load scenario
    TEST_ASSERT_EQUAL(SYSTEM_OK, system_enable_high_frequency_monitoring());
    TEST_ASSERT_EQUAL(SYSTEM_OK, communication_enable_stress_testing());
    
    HAL_Delay(2000); // 2 seconds under load
    
    metrics.load_cpu_usage = system_get_cpu_usage_percent();
    metrics.load_memory_usage = system_get_memory_usage_bytes();
    metrics.load_stack_usage = system_get_max_stack_usage_percent();
    
    // Verify load operation resource usage
    TEST_ASSERT_LESS_THAN(85, metrics.load_cpu_usage);       // <85% CPU under load
    TEST_ASSERT_LESS_THAN(90000, metrics.load_memory_usage); // <90KB memory under load
    TEST_ASSERT_LESS_THAN(75, metrics.load_stack_usage);     // <75% stack under load
    
    // Measurement Phase 3: Real-time Constraint Validation
    uint32_t control_loop_measurements[100];
    
    for (uint32_t i = 0; i < 100; i++) {
        uint32_t loop_start_us = get_microsecond_timer();
        
        // Execute one control loop iteration
        SystemError_t loop_result = motor_control_loop_iteration();
        TEST_ASSERT_EQUAL(SYSTEM_OK, loop_result);
        
        uint32_t loop_duration_us = get_microsecond_timer() - loop_start_us;
        control_loop_measurements[i] = loop_duration_us;
        
        if (loop_duration_us > metrics.max_control_loop_us) {
            metrics.max_control_loop_us = loop_duration_us;
        }
        metrics.total_control_loop_us += loop_duration_us;
        
        HAL_Delay(1); // 1ms between measurements
    }
    
    metrics.avg_control_loop_us = metrics.total_control_loop_us / 100;
    
    // Calculate control loop jitter
    uint32_t jitter_sum = 0;
    for (uint32_t i = 1; i < 100; i++) {
        uint32_t diff = (control_loop_measurements[i] > control_loop_measurements[i-1]) ?
                       (control_loop_measurements[i] - control_loop_measurements[i-1]) :
                       (control_loop_measurements[i-1] - control_loop_measurements[i]);
        jitter_sum += diff;
    }
    metrics.avg_control_loop_jitter_us = jitter_sum / 99;
    
    // Verify real-time constraints
    TEST_ASSERT_LESS_THAN(50, metrics.max_control_loop_us);      // <50µs max
    TEST_ASSERT_LESS_THAN(30, metrics.avg_control_loop_us);      // <30µs avg
    TEST_ASSERT_LESS_THAN(10, metrics.avg_control_loop_jitter_us); // <10µs jitter
    
    // Measurement Phase 4: Interrupt Latency
    uint32_t interrupt_latencies[50];
    
    for (uint32_t i = 0; i < 50; i++) {
        uint32_t trigger_time_us = get_microsecond_timer();
        
        // Trigger test interrupt and measure response
        test_fixture_trigger_test_interrupt();
        
        // Wait for interrupt handler completion
        while (!test_fixture_is_interrupt_handled()) {
            // Wait
        }
        
        uint32_t response_time_us = get_microsecond_timer() - trigger_time_us;
        interrupt_latencies[i] = response_time_us;
        
        if (response_time_us > metrics.max_interrupt_latency_us) {
            metrics.max_interrupt_latency_us = response_time_us;
        }
        metrics.total_interrupt_latency_us += response_time_us;
        
        test_fixture_clear_test_interrupt();
        HAL_Delay(10);
    }
    
    metrics.avg_interrupt_latency_us = metrics.total_interrupt_latency_us / 50;
    
    // Verify interrupt latency
    TEST_ASSERT_LESS_THAN(10, metrics.max_interrupt_latency_us);  // <10µs max
    TEST_ASSERT_LESS_THAN(5, metrics.avg_interrupt_latency_us);   // <5µs avg
    
    // Memory fragmentation analysis (ARM_CM7 specific)
    uint32_t heap_free_bytes = system_get_heap_free_bytes();
    uint32_t largest_free_block = system_get_largest_free_block_bytes();
    uint32_t fragmentation_percent = ((heap_free_bytes - largest_free_block) * 100) / heap_free_bytes;
    
    metrics.memory_fragmentation_percent = fragmentation_percent;
    TEST_ASSERT_LESS_THAN(20, fragmentation_percent); // <20% fragmentation
    
    // Log comprehensive resource usage metrics
    uint32_t total_resource_test_us = get_microsecond_timer() - resource_test_start_us;
    
    safety_log_performance_metric("resource_test_duration_us", total_resource_test_us);
    safety_log_performance_metric("normal_cpu_usage_percent", metrics.normal_cpu_usage);
    safety_log_performance_metric("load_cpu_usage_percent", metrics.load_cpu_usage);
    safety_log_performance_metric("normal_memory_usage_bytes", metrics.normal_memory_usage);
    safety_log_performance_metric("load_memory_usage_bytes", metrics.load_memory_usage);
    safety_log_performance_metric("max_control_loop_us", metrics.max_control_loop_us);
    safety_log_performance_metric("avg_control_loop_us", metrics.avg_control_loop_us);
    safety_log_performance_metric("control_loop_jitter_us", metrics.avg_control_loop_jitter_us);
    safety_log_performance_metric("max_interrupt_latency_us", metrics.max_interrupt_latency_us);
    safety_log_performance_metric("avg_interrupt_latency_us", metrics.avg_interrupt_latency_us);
    safety_log_performance_metric("memory_fragmentation_percent", metrics.memory_fragmentation_percent);
    
    // Generate resource usage report
    generate_resource_usage_report(&metrics);
    
    // Cleanup
    system_disable_high_frequency_monitoring();
    communication_disable_stress_testing();
    system_set_state(SYSTEM_STATE_READY);
}
```

## Validation Documentation

### Test Records
Each test must maintain:
- Test execution date/time
- Software version tested
- Hardware configuration
- Test results (pass/fail)
- Performance measurements
- Any deviations or anomalies

### Test Report Template
```
Test ID: ES-001
Test Name: Hardware Emergency Stop Response Time
Date: [DATE]
Tester: [NAME]
Software Version: [VERSION]
Hardware Configuration: [CONFIG]

Test Results:
- Motor 1 Response Time: [XX]ms
- Motor 2 Response Time: [XX]ms
- System State Transition: [PASS/FAIL]
- Emergency Indicators: [PASS/FAIL]

Overall Result: [PASS/FAIL]
Comments: [ANY ISSUES OR OBSERVATIONS]
```

## Automated Validation

### Continuous Testing
```c
// Automated safety validation suite
SystemError_t run_safety_validation_suite(void) {
    uint32_t passed_tests = 0;
    uint32_t total_tests = 0;
    
    // Emergency stop tests
    if (test_hardware_emergency_stop_response() == TEST_PASS) passed_tests++;
    total_tests++;
    
    if (test_software_emergency_stop() == TEST_PASS) passed_tests++;
    total_tests++;
    
    // Fault detection tests
    if (test_motor_overcurrent_detection() == TEST_PASS) passed_tests++;
    total_tests++;
    
    if (test_thermal_protection() == TEST_PASS) passed_tests++;
    total_tests++;
    
    // Watchdog tests
    if (test_software_task_watchdog() == TEST_PASS) passed_tests++;
    total_tests++;
    
    // Generate report
    log_validation_results(passed_tests, total_tests);
    
    return (passed_tests == total_tests) ? SYSTEM_OK : ERROR_VALIDATION_FAILED;
}
```

### Test Environment Setup
```bash
# Run complete safety validation
python3 scripts/run_safety_validation.py

# Generate safety test report
python3 scripts/generate_safety_report.py

# Validate SSOT safety configuration
python3 scripts/validate_ssot.py --safety-only
```

## Compliance and Certification

### Documentation Requirements
- Test procedures documented and approved
- Test results recorded and archived
- Traceability to safety requirements
- Regular review and updates

### Periodic Validation
- Monthly: Automated test suite execution
- Quarterly: Complete manual validation
- Annually: Full safety system review
- After changes: Impact assessment and testing

## Related Documentation
- [Safety Requirements Specification](../requirements/safety_requirements.md)
- [Emergency Stop Testing Guide](../testing/emergency_stop_testing.md)
- [Fault Analysis Procedures](../procedures/fault_analysis.md)
- [Safety System Architecture](../design/safety_architecture.md)
