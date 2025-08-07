# Simulation Testing Workflows

## Overview
This guide provides step-by-step workflows for testing stepper motor control algorithms using the simulation framework. These workflows enable comprehensive validation without hardware risk.

## Quick Test Workflows

### 1. Basic Driver Validation
Test that drivers initialize and respond correctly in simulation mode.

```bash
# Build with simulation
cmake --build build/Debug

# Run basic driver tests
./build/Debug/tests/test_driver_simulation.elf
```

**Test Checklist:**
- [ ] L6470 initialization succeeds
- [ ] AS5600 initialization succeeds  
- [ ] Basic register read/write operations work
- [ ] Status registers return expected values

### 2. Motion Control Testing
Validate motor movement and position feedback.

```c
// Example test sequence
void test_basic_motion(void) {
    // Initialize drivers
    TEST_ASSERT_EQUAL(SYSTEM_OK, l6470_init(0));
    TEST_ASSERT_EQUAL(SYSTEM_OK, as5600_init(0));
    
    // Test relative movement
    l6470_move(0, 1000);  // Move 1000 steps
    
    // Wait for movement completion (simulated)
    while (l6470_is_busy(0)) {
        motor_simulation_update(10);  // 10ms simulation steps
    }
    
    // Verify final position
    int32_t motor_pos = l6470_sim_get_position(0);
    uint16_t encoder_pos = as5600_read_angle(0);
    
    TEST_ASSERT_EQUAL(1000, motor_pos);
    TEST_ASSERT_INT_WITHIN(10, motor_pos, encoder_pos);
}
```

### 3. Closed-Loop Control Testing
Test position control algorithms with encoder feedback.

```c
void test_position_control_loop(void) {
    int32_t target_position = 2000;
    int32_t current_position = 0;
    
    // Simple position control loop
    while (abs(target_position - current_position) > 5) {
        int32_t error = target_position - current_position;
        
        // Simple proportional control
        int32_t move_steps = error / 4;  // P-gain = 0.25
        
        if (abs(move_steps) > 100) {
            move_steps = (move_steps > 0) ? 100 : -100;  // Limit step size
        }
        
        l6470_move(0, move_steps);
        
        // Wait for movement
        while (l6470_is_busy(0)) {
            motor_simulation_update(10);
        }
        
        // Read new position
        current_position = l6470_sim_get_position(0);
    }
    
    TEST_ASSERT_INT_WITHIN(5, target_position, current_position);
}
```

## Advanced Testing Workflows

### 4. Fault Injection Testing
Test error handling and recovery procedures.

```c
void test_fault_recovery(void) {
    // Initialize normally
    TEST_ASSERT_EQUAL(SYSTEM_OK, l6470_init(0));
    
    // Inject overcurrent fault
    l6470_sim_set_fault(0, L6470_STATUS_OCD);
    
    // Verify fault detection
    uint16_t status = l6470_get_status(0);
    TEST_ASSERT_TRUE(status & L6470_STATUS_OCD);
    
    // Test recovery procedure
    l6470_reset_device(0);
    l6470_sim_clear_faults(0);
    
    // Verify recovery
    status = l6470_get_status(0);
    TEST_ASSERT_FALSE(status & L6470_STATUS_OCD);
}
```

### 5. Performance Benchmarking
Measure control loop performance and timing.

```c
void test_control_loop_timing(void) {
    uint32_t start_time = HAL_GetTick();
    int iterations = 1000;
    
    for (int i = 0; i < iterations; i++) {
        // Simulate typical control loop operations
        uint16_t encoder_pos = as5600_read_angle(0);
        int32_t motor_pos = l6470_sim_get_position(0);
        
        // Simple control calculation
        int32_t error = encoder_pos - motor_pos;
        if (abs(error) > 10) {
            l6470_move(0, error / 2);
        }
        
        motor_simulation_update(1);  // 1ms simulation step
    }
    
    uint32_t elapsed = HAL_GetTick() - start_time;
    float avg_time_per_loop = (float)elapsed / iterations;
    
    // Verify real-time performance (should be <1ms per loop)
    TEST_ASSERT_LESS_THAN(1.0f, avg_time_per_loop);
}
```

## Multi-Motor Testing

### 6. Dual Motor Coordination
Test synchronized movement of multiple motors.

```c
void test_dual_motor_sync(void) {
    // Initialize both motors
    TEST_ASSERT_EQUAL(SYSTEM_OK, l6470_init(0));
    TEST_ASSERT_EQUAL(SYSTEM_OK, l6470_init(1));
    
    // Start synchronized movement
    l6470_move(0, 1000);  // Motor 0: 1000 steps
    l6470_move(1, 500);   // Motor 1: 500 steps
    
    // Monitor until both complete
    bool motor0_done = false, motor1_done = false;
    while (!motor0_done || !motor1_done) {
        motor_simulation_update(10);
        
        if (!motor0_done && !l6470_is_busy(0)) {
            motor0_done = true;
        }
        if (!motor1_done && !l6470_is_busy(1)) {
            motor1_done = true;
        }
    }
    
    // Verify final positions
    TEST_ASSERT_EQUAL(1000, l6470_sim_get_position(0));
    TEST_ASSERT_EQUAL(500, l6470_sim_get_position(1));
}
```

## Safety System Testing

### 7. Emergency Stop Testing
Test emergency stop procedures and recovery.

```c
void test_emergency_stop(void) {
    // Start motor movement
    l6470_run(0, 1000.0f);  // Run at 1000 steps/sec
    
    // Let it run for a bit
    for (int i = 0; i < 50; i++) {
        motor_simulation_update(10);  // 500ms total
    }
    
    // Trigger emergency stop
    emergency_stop_all_motors();
    
    // Verify immediate stop
    TEST_ASSERT_FALSE(l6470_is_busy(0));
    
    // Verify motor is in high-impedance state
    uint16_t status = l6470_get_status(0);
    TEST_ASSERT_TRUE(status & L6470_STATUS_HIZ);
}
```

### 8. Watchdog Testing
Test watchdog timer and automatic safety shutdown.

```c
void test_watchdog_timeout(void) {
    // Start motor with watchdog enabled
    watchdog_enable(1000);  // 1 second timeout
    l6470_run(0, 500.0f);
    
    // Simulate normal operation with watchdog refresh
    for (int i = 0; i < 50; i++) {
        motor_simulation_update(10);
        if (i % 10 == 0) {
            watchdog_refresh();  // Refresh every 100ms
        }
    }
    
    // Stop refreshing watchdog (simulate system hang)
    for (int i = 0; i < 150; i++) {
        motor_simulation_update(10);  // 1.5 seconds without refresh
    }
    
    // Verify watchdog triggered safety stop
    TEST_ASSERT_FALSE(l6470_is_busy(0));
    TEST_ASSERT_TRUE(is_safety_stop_active());
}
```

## Automated Test Suites

### Running Full Test Suite
```bash
# Run all simulation tests
cmake --build build/Debug --target test_simulation

# Run specific test categories
cmake --build build/Debug --target test_drivers
cmake --build build/Debug --target test_motion_control
cmake --build build/Debug --target test_safety_systems
```

### Continuous Integration Testing
```yaml
# Example CI workflow for simulation testing
name: Simulation Tests
on: [push, pull_request]

jobs:
  simulation-tests:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Setup ARM Toolchain
        run: sudo apt-get install gcc-arm-none-eabi
      - name: Build Simulation
        run: |
          cmake -S . -B build --preset Debug
          cmake --build build
      - name: Run Tests
        run: |
          cmake --build build --target test_simulation
          ./build/tests/test_simulation_suite.elf
```

## Test Results Analysis

### Performance Metrics
```c
// Example metrics collection
typedef struct {
    float avg_control_loop_time;
    float max_control_loop_time;
    int total_position_errors;
    int max_position_error;
    int fault_injection_tests_passed;
} TestMetrics_t;

void collect_test_metrics(TestMetrics_t *metrics) {
    // Collect timing data
    metrics->avg_control_loop_time = simulation_get_avg_loop_time();
    metrics->max_control_loop_time = simulation_get_max_loop_time();
    
    // Collect error data
    metrics->total_position_errors = simulation_get_position_error_count();
    metrics->max_position_error = simulation_get_max_position_error();
    
    // Report results
    printf("Average loop time: %.3f ms\n", metrics->avg_control_loop_time);
    printf("Maximum position error: %d steps\n", metrics->max_position_error);
}
```

### Test Coverage Report
```bash
# Generate test coverage report
gcov -r build/CMakeFiles/stm32h753_ihm02a1.dir/src/drivers/*.gcno
lcov --capture --directory build --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

## Debugging Simulation Issues

### Enable Debug Output
```c
// Enable detailed simulation logging
#define SIM_DEBUG_ENABLED 1
#define SIM_LOG_LEVEL SIM_LOG_VERBOSE

// Check simulation state
if (motor_simulation_is_active()) {
    SimulationStatus_t status;
    hardware_simulation_get_status(&status);
    printf("Motor 0 - Pos: %d, Speed: %.2f, State: %d\n",
           status.motor_position, status.motor_speed, status.motor_state);
}
```

### Common Debug Patterns
```c
// Verify register simulation
uint16_t expected = 0x1234;
l6470_sim_set_register(0, L6470_REG_ABS_POS, expected);
uint16_t actual = l6470_read_register(0, L6470_REG_ABS_POS);
assert(expected == actual);

// Check motion physics
int32_t start_pos = l6470_sim_get_position(0);
l6470_move(0, 100);
motor_simulation_update(1000);  // 1 second
int32_t end_pos = l6470_sim_get_position(0);
assert(end_pos == start_pos + 100);
```

## Best Practices

1. **Start Simple**: Begin with basic driver tests before complex motion control
2. **Use Small Steps**: Update simulation frequently (10ms steps) for realistic behavior
3. **Test Edge Cases**: Include fault injection and error conditions
4. **Verify Timing**: Ensure real-time performance requirements are met
5. **Document Results**: Keep records of test metrics and performance data

## Related Documentation
- [Simulation Framework Guide](SIMULATION_FRAMEWORK_GUIDE.md)
- [HAL Abstraction Guide](hal_abstraction_guide.md)
- [Testing Framework Guide](testing_framework_guide.md)
