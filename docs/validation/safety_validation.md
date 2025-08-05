# Safety System Validation Procedures

## Overview
This document defines comprehensive validation procedures for safety-critical systems in the STM32H753ZI motor control project. All safety systems must pass these validation tests before deployment.

## Emergency Stop Validation

### Test ES-001: Hardware Emergency Stop Response Time
**Objective**: Verify hardware emergency stop response time meets specification (< 100ms)

**Prerequisites**:
- System initialized and motors running
- Emergency stop button connected and functional
- Oscilloscope or timing measurement available

**Procedure**:
1. Start Motor 1 at 1000 steps/sec
2. Start Motor 2 at 1500 steps/sec
3. Trigger hardware emergency stop button
4. Measure time from button press to motor stop
5. Verify both motors stop within 100ms
6. Verify system enters EMERGENCY state
7. Verify all outputs disabled

**Expected Results**:
- Response time < 100ms for both motors
- System state = SYSTEM_STATE_EMERGENCY
- All motor outputs disabled
- Emergency indicators activated

**Test Code**:
```c
void test_hardware_emergency_stop_response(void) {
    uint32_t start_time, stop_time;
    
    // Start motors
    motor_run(MOTOR_1, DIRECTION_FORWARD, 1000);
    motor_run(MOTOR_2, DIRECTION_FORWARD, 1500);
    HAL_Delay(100); // Allow motors to reach speed
    
    // Trigger emergency stop and measure
    start_time = get_microsecond_timer();
    trigger_hardware_emergency_stop();
    
    // Wait for motors to stop
    while (motor_is_moving(MOTOR_1) || motor_is_moving(MOTOR_2)) {
        // Measure time
    }
    stop_time = get_microsecond_timer();
    
    uint32_t response_time_us = stop_time - start_time;
    
    // Validate response time
    TEST_ASSERT_LESS_THAN(100000, response_time_us); // < 100ms
    TEST_ASSERT_EQUAL(SYSTEM_STATE_EMERGENCY, system_get_state());
    TEST_ASSERT_FALSE(motor_is_moving(MOTOR_1));
    TEST_ASSERT_FALSE(motor_is_moving(MOTOR_2));
}
```

### Test ES-002: Software Emergency Stop
**Objective**: Verify software emergency stop functionality

**Procedure**:
1. Initialize system in RUNNING state
2. Start motor movements
3. Trigger software emergency stop with reason code
4. Verify immediate motor stop
5. Verify system state transition
6. Verify error logging

**Expected Results**:
- Motors stop immediately (hard stop)
- System transitions to EMERGENCY state
- Reason code logged correctly
- Safety callbacks executed

### Test ES-003: Emergency Stop Recovery
**Objective**: Verify proper recovery from emergency stop condition

**Procedure**:
1. Trigger emergency stop (hardware or software)
2. Attempt to clear emergency condition
3. Verify system requirements for recovery
4. Execute recovery sequence
5. Verify system returns to operational state

**Expected Results**:
- Emergency cannot be cleared while trigger active
- Recovery requires explicit acknowledgment
- System performs safety checks before returning to operation
- All safety systems re-enabled

## Fault Detection Validation

### Test FD-001: Motor Overcurrent Detection
**Objective**: Verify L6470 overcurrent detection and response

**Procedure**:
1. Configure L6470 with low overcurrent threshold (for testing)
2. Start motor with high load or stall condition
3. Monitor for overcurrent fault detection
4. Verify fault response and motor stop
5. Verify fault logging and system state

**Expected Results**:
- Overcurrent detected within configured time
- Motor stops immediately
- Fault logged with timestamp and motor ID
- System transitions to FAULT state

**Test Code**:
```c
void test_motor_overcurrent_detection(void) {
    // Set low OCD threshold for testing
    uint8_t original_threshold = l6470_get_ocd_threshold(MOTOR_1);
    l6470_set_ocd_threshold(MOTOR_1, 0x01); // Minimum threshold
    
    // Start motor and create overcurrent condition
    motor_run(MOTOR_1, DIRECTION_FORWARD, 500);
    simulate_motor_stall(MOTOR_1); // Test fixture function
    
    // Wait for fault detection
    uint32_t timeout = HAL_GetTick() + 1000; // 1 second timeout
    while (HAL_GetTick() < timeout) {
        uint16_t status = l6470_get_status(MOTOR_1);
        if (status & L6470_STATUS_OCD) {
            break;
        }
        HAL_Delay(10);
    }
    
    // Verify fault detected
    uint16_t status = l6470_get_status(MOTOR_1);
    TEST_ASSERT_TRUE(status & L6470_STATUS_OCD);
    TEST_ASSERT_FALSE(motor_is_moving(MOTOR_1));
    TEST_ASSERT_EQUAL(SYSTEM_STATE_FAULT, system_get_state());
    
    // Cleanup
    l6470_set_ocd_threshold(MOTOR_1, original_threshold);
    system_clear_fault();
}
```

### Test FD-002: Thermal Protection
**Objective**: Verify thermal protection mechanisms

**Procedure**:
1. Monitor thermal status registers
2. Simulate thermal warning condition
3. Verify warning response (reduce current)
4. Simulate thermal shutdown condition
5. Verify shutdown response (stop motor)

### Test FD-003: Encoder Fault Detection
**Objective**: Verify AS5600 encoder fault detection

**Procedure**:
1. Monitor encoder magnitude and status
2. Simulate weak magnetic field
3. Verify fault detection and response
4. Simulate encoder communication failure
5. Verify fallback to open-loop control

## Watchdog Validation

### Test WD-001: Hardware Watchdog Operation
**Objective**: Verify hardware watchdog reset functionality

**Procedure**:
1. Initialize hardware watchdog with short timeout
2. Stop watchdog refresh task
3. Verify system reset occurs within timeout period
4. Verify proper system restart after reset

**Safety Note**: This test should be run in controlled environment with recovery mechanisms.

### Test WD-002: Software Task Watchdog
**Objective**: Verify software task monitoring

**Procedure**:
1. Configure task watchdogs for critical tasks
2. Simulate task hang/deadlock condition
3. Verify watchdog timeout detection
4. Verify fault handling and recovery

**Test Code**:
```c
void test_software_task_watchdog(void) {
    // Configure test task with short timeout
    task_watchdog_configure(TEST_TASK_ID, 100); // 100ms timeout
    task_watchdog_enable(TEST_TASK_ID);
    
    // Initial checkin
    task_watchdog_checkin(TEST_TASK_ID);
    HAL_Delay(50); // Within timeout
    
    // Verify no timeout yet
    TEST_ASSERT_FALSE(task_watchdog_is_timeout(TEST_TASK_ID));
    
    // Wait for timeout
    HAL_Delay(150); // Exceed timeout
    
    // Check for timeout detection
    task_watchdog_monitor(); // Update timeout status
    TEST_ASSERT_TRUE(task_watchdog_is_timeout(TEST_TASK_ID));
    
    // Verify fault handling
    TEST_ASSERT_EQUAL(SYSTEM_STATE_FAULT, system_get_state());
}
```

## State Management Validation

### Test SM-001: State Transition Validation
**Objective**: Verify all valid and invalid state transitions

**Procedure**:
1. Test all valid state transitions
2. Attempt invalid state transitions
3. Verify state transition logging
4. Verify state-specific behaviors

**Test Matrix**:
```
| From\To   | INIT | READY | RUNNING | WARNING | FAULT | EMERGENCY |
| --------- | ---- | ----- | ------- | ------- | ----- | --------- |
| INIT      | -    | ✓     | ✗       | ✗       | ✓     | ✓         |
| READY     | ✗    | -     | ✓       | ✗       | ✓     | ✓         |
| RUNNING   | ✗    | ✓     | -       | ✓       | ✓     | ✓         |
| WARNING   | ✗    | ✓     | ✓       | -       | ✓     | ✓         |
| FAULT     | ✗    | ✓     | ✗       | ✗       | -     | ✓         |
| EMERGENCY | ✗    | ✓     | ✗       | ✗       | ✗     | -         |
```

### Test SM-002: Safety State Enforcement
**Objective**: Verify safety requirements enforced in each state

**Procedure**:
1. Verify FAULT state prevents motor operation
2. Verify EMERGENCY state disables all outputs
3. Verify WARNING state implements reduced functionality
4. Verify state-specific safety checks

## Integration Validation

### Test INT-001: Complete Safety Chain
**Objective**: Verify end-to-end safety system operation

**Procedure**:
1. Initialize complete system
2. Start normal operation
3. Inject various fault conditions
4. Verify proper fault detection and response
5. Test recovery procedures
6. Verify system returns to operational state

### Test INT-002: Multiple Fault Conditions
**Objective**: Verify handling of simultaneous faults

**Procedure**:
1. Create multiple fault conditions simultaneously
2. Verify priority handling of faults
3. Verify system remains in safe state
4. Verify all faults logged correctly

### Test INT-003: Safety System Under Load
**Objective**: Verify safety system performance under high load

**Procedure**:
1. Create high system load (multiple motors, fast control loops)
2. Inject fault conditions
3. Verify safety response times maintained
4. Verify no missed fault detections

## Performance Validation

### Test PERF-001: Safety Response Times
**Objective**: Verify all safety response times meet specifications

**Requirements**:
- Emergency stop response: < 100ms
- Fault detection cycle: < 50ms
- Watchdog refresh: < 1000ms
- State transition: < 10ms

### Test PERF-002: Safety System Resource Usage
**Objective**: Verify safety systems don't impact normal operation

**Procedure**:
1. Measure CPU usage with safety systems active
2. Measure memory usage
3. Verify real-time performance maintained
4. Check for timing impacts on control loops

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
