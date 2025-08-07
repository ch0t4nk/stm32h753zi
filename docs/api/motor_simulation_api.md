# Motor Simulation API Reference

## Overview
The Motor Simulation API provides comprehensive stepper motor simulation capabilities for the STM32H753ZI project. This API enables hardware-free development and testing by simulating L6470 stepper motor drivers and their physical behavior.

## Core Simulation Functions

### Initialization and Control

#### `motor_simulation_init()`
```c
SimulationError_t motor_simulation_init(void);
```
**Description**: Initialize the motor simulation framework.
**Returns**: `SIM_ERROR_NONE` on success, error code on failure.
**Usage**:
```c
SimulationError_t result = motor_simulation_init();
if (result != SIM_ERROR_NONE) {
    // Handle initialization error
}
```

#### `motor_simulation_is_active()`
```c
bool motor_simulation_is_active(void);
```
**Description**: Check if motor simulation is currently active.
**Returns**: `true` if simulation is active, `false` otherwise.
**Usage**:
```c
if (motor_simulation_is_active()) {
    // Use simulation APIs
} else {
    // Use real hardware APIs
}
```

#### `motor_simulation_update()`
```c
void motor_simulation_update(uint32_t delta_time_ms);
```
**Description**: Update simulation state by specified time.
**Parameters**:
- `delta_time_ms`: Time step in milliseconds
**Usage**:
```c
// Update simulation by 10ms
motor_simulation_update(10);

// Simulation loop
while (motor_is_running) {
    motor_simulation_update(1);  // 1ms steps
    process_control_loop();
}
```

#### `simulation_reset()`
```c
void simulation_reset(void);
```
**Description**: Reset all simulation state to initial conditions.
**Usage**:
```c
void setUp(void) {
    simulation_reset();  // Clean state for each test
}
```

### Motor Instance Management

#### `l6470_sim_init()`
```c
SimulationError_t l6470_sim_init(uint8_t motor_id);
```
**Description**: Initialize simulation for specific L6470 motor.
**Parameters**:
- `motor_id`: Motor identifier (0 or 1)
**Returns**: `SIM_ERROR_NONE` on success
**Usage**:
```c
SimulationError_t result = l6470_sim_init(0);  // Initialize motor 0
```

#### `l6470_sim_deinit()`
```c
void l6470_sim_deinit(uint8_t motor_id);
```
**Description**: Deinitialize motor simulation instance.
**Parameters**:
- `motor_id`: Motor identifier
**Usage**:
```c
l6470_sim_deinit(0);  // Clean up motor 0 simulation
```

## Position and Movement Control

### Position Management

#### `l6470_sim_set_position()`
```c
void l6470_sim_set_position(uint8_t motor_id, int32_t position);
```
**Description**: Set absolute motor position in steps.
**Parameters**:
- `motor_id`: Motor identifier
- `position`: Position in steps (-2^21 to 2^21)
**Usage**:
```c
l6470_sim_set_position(0, 1000);  // Set to position 1000 steps
l6470_sim_set_position(1, -500);  // Set to position -500 steps
```

#### `l6470_sim_get_position()`
```c
int32_t l6470_sim_get_position(uint8_t motor_id);
```
**Description**: Get current motor position in steps.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: Current position in steps
**Usage**:
```c
int32_t current_pos = l6470_sim_get_position(0);
printf("Motor 0 position: %d steps\n", current_pos);
```

#### `l6470_sim_move()`
```c
void l6470_sim_move(uint8_t motor_id, int32_t steps);
```
**Description**: Command relative movement in steps.
**Parameters**:
- `motor_id`: Motor identifier
- `steps`: Relative movement (positive = clockwise)
**Usage**:
```c
l6470_sim_move(0, 1000);   // Move 1000 steps clockwise
l6470_sim_move(0, -500);   // Move 500 steps counter-clockwise
```

#### `l6470_sim_goto()`
```c
void l6470_sim_goto(uint8_t motor_id, int32_t position);
```
**Description**: Command absolute position movement.
**Parameters**:
- `motor_id`: Motor identifier
- `position`: Target absolute position
**Usage**:
```c
l6470_sim_goto(0, 5000);   // Go to absolute position 5000
```

### Speed Control

#### `l6470_sim_set_speed()`
```c
void l6470_sim_set_speed(uint8_t motor_id, float speed_steps_per_sec);
```
**Description**: Set motor speed in steps per second.
**Parameters**:
- `motor_id`: Motor identifier
- `speed_steps_per_sec`: Speed (0.0 to max speed)
**Usage**:
```c
l6470_sim_set_speed(0, 1000.0f);  // 1000 steps/sec
l6470_sim_set_speed(1, 500.5f);   // 500.5 steps/sec
```

#### `l6470_sim_get_speed()`
```c
float l6470_sim_get_speed(uint8_t motor_id);
```
**Description**: Get current motor speed.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: Current speed in steps per second
**Usage**:
```c
float current_speed = l6470_sim_get_speed(0);
if (current_speed > MAX_SAFE_SPEED) {
    l6470_sim_soft_stop(0);
}
```

#### `l6470_sim_run()`
```c
void l6470_sim_run(uint8_t motor_id, float speed_steps_per_sec);
```
**Description**: Start continuous rotation at specified speed.
**Parameters**:
- `motor_id`: Motor identifier
- `speed_steps_per_sec`: Target speed (positive = clockwise)
**Usage**:
```c
l6470_sim_run(0, 800.0f);   // Run clockwise at 800 steps/sec
l6470_sim_run(1, -600.0f);  // Run counter-clockwise at 600 steps/sec
```

### Motion Control

#### `l6470_sim_soft_stop()`
```c
void l6470_sim_soft_stop(uint8_t motor_id);
```
**Description**: Decelerate motor to stop using configured deceleration.
**Parameters**:
- `motor_id`: Motor identifier
**Usage**:
```c
l6470_sim_soft_stop(0);  // Gradual stop
```

#### `l6470_sim_hard_stop()`
```c
void l6470_sim_hard_stop(uint8_t motor_id);
```
**Description**: Immediately stop motor without deceleration.
**Parameters**:
- `motor_id`: Motor identifier
**Usage**:
```c
l6470_sim_hard_stop(0);  // Emergency stop
```

#### `l6470_sim_soft_hiz()`
```c
void l6470_sim_soft_hiz(uint8_t motor_id);
```
**Description**: Decelerate to stop and set high impedance state.
**Parameters**:
- `motor_id`: Motor identifier
**Usage**:
```c
l6470_sim_soft_hiz(0);  // Stop and release motor
```

#### `l6470_sim_hard_hiz()`
```c
void l6470_sim_hard_hiz(uint8_t motor_id);
```
**Description**: Immediately stop and set high impedance state.
**Parameters**:
- `motor_id`: Motor identifier
**Usage**:
```c
l6470_sim_hard_hiz(0);  // Emergency stop and release
```

## Status and State Management

### Status Reading

#### `l6470_sim_get_status()`
```c
uint16_t l6470_sim_get_status(uint8_t motor_id);
```
**Description**: Get motor status register value.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: 16-bit status register value
**Usage**:
```c
uint16_t status = l6470_sim_get_status(0);
if (status & L6470_STATUS_BUSY) {
    // Motor is busy
}
if (status & L6470_STATUS_DIR) {
    // Motor rotating clockwise
}
```

#### `l6470_sim_is_busy()`
```c
bool l6470_sim_is_busy(uint8_t motor_id);
```
**Description**: Check if motor is currently executing a command.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: `true` if busy, `false` if idle
**Usage**:
```c
// Wait for movement completion
while (l6470_sim_is_busy(0)) {
    motor_simulation_update(10);
}
```

#### `l6470_sim_get_direction()`
```c
bool l6470_sim_get_direction(uint8_t motor_id);
```
**Description**: Get current rotation direction.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: `true` for clockwise, `false` for counter-clockwise
**Usage**:
```c
bool is_clockwise = l6470_sim_get_direction(0);
printf("Motor direction: %s\n", is_clockwise ? "CW" : "CCW");
```

### Fault and Error Management

#### `l6470_sim_set_fault()`
```c
void l6470_sim_set_fault(uint8_t motor_id, uint16_t fault_flags);
```
**Description**: Inject fault conditions for testing.
**Parameters**:
- `motor_id`: Motor identifier
- `fault_flags`: Fault bits to set (L6470_STATUS_* flags)
**Usage**:
```c
// Inject undervoltage fault
l6470_sim_set_fault(0, L6470_STATUS_UVLO);

// Inject multiple faults
l6470_sim_set_fault(0, L6470_STATUS_UVLO | L6470_STATUS_TH_SD);
```

#### `l6470_sim_clear_faults()`
```c
void l6470_sim_clear_faults(uint8_t motor_id);
```
**Description**: Clear all fault conditions.
**Parameters**:
- `motor_id`: Motor identifier
**Usage**:
```c
l6470_sim_clear_faults(0);  // Clear all faults
```

#### `l6470_sim_get_faults()`
```c
uint16_t l6470_sim_get_faults(uint8_t motor_id);
```
**Description**: Get current fault status.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: Fault flags currently active
**Usage**:
```c
uint16_t faults = l6470_sim_get_faults(0);
if (faults & L6470_STATUS_UVLO) {
    handle_undervoltage_fault();
}
```

## Configuration and Parameters

### Motion Parameters

#### `l6470_sim_set_acceleration()`
```c
void l6470_sim_set_acceleration(uint8_t motor_id, float acc_steps_per_sec2);
```
**Description**: Set motor acceleration in steps/sec².
**Parameters**:
- `motor_id`: Motor identifier
- `acc_steps_per_sec2`: Acceleration value
**Usage**:
```c
l6470_sim_set_acceleration(0, 1000.0f);  // 1000 steps/sec²
```

#### `l6470_sim_set_deceleration()`
```c
void l6470_sim_set_deceleration(uint8_t motor_id, float dec_steps_per_sec2);
```
**Description**: Set motor deceleration in steps/sec².
**Parameters**:
- `motor_id`: Motor identifier
- `dec_steps_per_sec2`: Deceleration value
**Usage**:
```c
l6470_sim_set_deceleration(0, 2000.0f);  // 2000 steps/sec²
```

#### `l6470_sim_set_max_speed()`
```c
void l6470_sim_set_max_speed(uint8_t motor_id, float max_speed_steps_per_sec);
```
**Description**: Set maximum motor speed in steps/sec.
**Parameters**:
- `motor_id`: Motor identifier
- `max_speed_steps_per_sec`: Maximum speed value
**Usage**:
```c
l6470_sim_set_max_speed(0, 5000.0f);  // Max 5000 steps/sec
```

#### `l6470_sim_set_min_speed()`
```c
void l6470_sim_set_min_speed(uint8_t motor_id, float min_speed_steps_per_sec);
```
**Description**: Set minimum motor speed in steps/sec.
**Parameters**:
- `motor_id`: Motor identifier
- `min_speed_steps_per_sec`: Minimum speed value
**Usage**:
```c
l6470_sim_set_min_speed(0, 100.0f);  // Min 100 steps/sec
```

### Current and Power Settings

#### `l6470_sim_set_holding_current()`
```c
void l6470_sim_set_holding_current(uint8_t motor_id, float current_ma);
```
**Description**: Set holding current in milliamps.
**Parameters**:
- `motor_id`: Motor identifier
- `current_ma`: Holding current (0-3000 mA)
**Usage**:
```c
l6470_sim_set_holding_current(0, 500.0f);  // 500 mA holding
```

#### `l6470_sim_set_running_current()`
```c
void l6470_sim_set_running_current(uint8_t motor_id, float current_ma);
```
**Description**: Set running current in milliamps.
**Parameters**:
- `motor_id`: Motor identifier
- `current_ma`: Running current (0-3000 mA)
**Usage**:
```c
l6470_sim_set_running_current(0, 1000.0f);  // 1000 mA running
```

## Advanced Simulation Features

### Physics Simulation

#### `l6470_sim_set_load_torque()`
```c
void l6470_sim_set_load_torque(uint8_t motor_id, float torque_mNm);
```
**Description**: Set mechanical load torque for realistic simulation.
**Parameters**:
- `motor_id`: Motor identifier
- `torque_mNm`: Load torque in milliNewton-meters
**Usage**:
```c
l6470_sim_set_load_torque(0, 50.0f);  // 50 mNm load
```

#### `l6470_sim_set_inertia()`
```c
void l6470_sim_set_inertia(uint8_t motor_id, float inertia_kg_m2);
```
**Description**: Set rotational inertia for dynamic simulation.
**Parameters**:
- `motor_id`: Motor identifier
- `inertia_kg_m2`: Rotational inertia in kg⋅m²
**Usage**:
```c
l6470_sim_set_inertia(0, 1e-6f);  // 1 µkg⋅m²
```

#### `l6470_sim_set_friction()`
```c
void l6470_sim_set_friction(uint8_t motor_id, float friction_coefficient);
```
**Description**: Set friction coefficient for realistic motion.
**Parameters**:
- `motor_id`: Motor identifier
- `friction_coefficient`: Friction coefficient (0.0-1.0)
**Usage**:
```c
l6470_sim_set_friction(0, 0.1f);  // 10% friction
```

### Thermal Simulation

#### `l6470_sim_set_temperature()`
```c
void l6470_sim_set_temperature(uint8_t motor_id, float temp_celsius);
```
**Description**: Set driver temperature for thermal protection testing.
**Parameters**:
- `motor_id`: Motor identifier
- `temp_celsius`: Temperature in Celsius
**Usage**:
```c
l6470_sim_set_temperature(0, 85.0f);  // 85°C temperature
```

#### `l6470_sim_get_temperature()`
```c
float l6470_sim_get_temperature(uint8_t motor_id);
```
**Description**: Get current driver temperature.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: Temperature in Celsius
**Usage**:
```c
float temp = l6470_sim_get_temperature(0);
if (temp > THERMAL_WARNING_TEMP) {
    handle_thermal_warning();
}
```

### Power Supply Simulation

#### `l6470_sim_set_supply_voltage()`
```c
void l6470_sim_set_supply_voltage(uint8_t motor_id, float voltage_v);
```
**Description**: Set supply voltage for power-related testing.
**Parameters**:
- `motor_id`: Motor identifier
- `voltage_v`: Supply voltage in volts
**Usage**:
```c
l6470_sim_set_supply_voltage(0, 12.0f);  // 12V supply
l6470_sim_set_supply_voltage(0, 7.0f);   // Test undervoltage
```

#### `l6470_sim_get_power_consumption()`
```c
float l6470_sim_get_power_consumption(uint8_t motor_id);
```
**Description**: Get current power consumption in watts.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: Power consumption in watts
**Usage**:
```c
float power = l6470_sim_get_power_consumption(0);
printf("Motor power: %.2f W\n", power);
```

## Multi-Motor Coordination

### Synchronized Movement

#### `l6470_sim_sync_move()`
```c
void l6470_sim_sync_move(uint8_t motor_count, uint8_t *motor_ids, int32_t *steps);
```
**Description**: Coordinate simultaneous movement of multiple motors.
**Parameters**:
- `motor_count`: Number of motors to coordinate
- `motor_ids`: Array of motor identifiers
- `steps`: Array of step counts for each motor
**Usage**:
```c
uint8_t motors[] = {0, 1};
int32_t steps[] = {1000, -500};
l6470_sim_sync_move(2, motors, steps);
```

#### `l6470_sim_sync_run()`
```c
void l6470_sim_sync_run(uint8_t motor_count, uint8_t *motor_ids, float *speeds);
```
**Description**: Start coordinated continuous rotation.
**Parameters**:
- `motor_count`: Number of motors
- `motor_ids`: Array of motor identifiers  
- `speeds`: Array of speeds for each motor
**Usage**:
```c
uint8_t motors[] = {0, 1};
float speeds[] = {800.0f, 600.0f};
l6470_sim_sync_run(2, motors, speeds);
```

#### `l6470_sim_sync_stop()`
```c
void l6470_sim_sync_stop(uint8_t motor_count, uint8_t *motor_ids);
```
**Description**: Simultaneously stop multiple motors.
**Parameters**:
- `motor_count`: Number of motors
- `motor_ids`: Array of motor identifiers
**Usage**:
```c
uint8_t motors[] = {0, 1};
l6470_sim_sync_stop(2, motors);
```

## Debug and Diagnostics

### State Inspection

#### `l6470_sim_dump_state()`
```c
void l6470_sim_dump_state(uint8_t motor_id);
```
**Description**: Print comprehensive motor state information.
**Parameters**:
- `motor_id`: Motor identifier
**Usage**:
```c
l6470_sim_dump_state(0);  // Debug output for motor 0
```

#### `l6470_sim_get_step_count()`
```c
uint32_t l6470_sim_get_step_count(uint8_t motor_id);
```
**Description**: Get total steps executed since initialization.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: Cumulative step count
**Usage**:
```c
uint32_t total_steps = l6470_sim_get_step_count(0);
printf("Total steps: %u\n", total_steps);
```

#### `l6470_sim_get_runtime()`
```c
uint32_t l6470_sim_get_runtime(uint8_t motor_id);
```
**Description**: Get motor runtime in milliseconds.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: Runtime in milliseconds
**Usage**:
```c
uint32_t runtime = l6470_sim_get_runtime(0);
printf("Motor runtime: %u ms\n", runtime);
```

### Performance Monitoring

#### `l6470_sim_get_command_count()`
```c
uint32_t l6470_sim_get_command_count(uint8_t motor_id);
```
**Description**: Get number of commands executed.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: Command count
**Usage**:
```c
uint32_t commands = l6470_sim_get_command_count(0);
```

#### `l6470_sim_reset_statistics()`
```c
void l6470_sim_reset_statistics(uint8_t motor_id);
```
**Description**: Reset performance statistics.
**Parameters**:
- `motor_id`: Motor identifier
**Usage**:
```c
l6470_sim_reset_statistics(0);  // Reset counters
```

## Error Handling

### Error Codes
```c
typedef enum {
    SIM_ERROR_NONE = 0,
    SIM_ERROR_INVALID_MOTOR_ID,
    SIM_ERROR_NOT_INITIALIZED,
    SIM_ERROR_INVALID_PARAMETER,
    SIM_ERROR_HARDWARE_FAULT,
    SIM_ERROR_OVERCURRENT,
    SIM_ERROR_UNDERVOLTAGE,
    SIM_ERROR_THERMAL_SHUTDOWN,
    SIM_ERROR_POSITION_LIMIT,
    SIM_ERROR_SPEED_LIMIT
} SimulationError_t;
```

### Error Handling Patterns
```c
// Check for valid motor ID
if (motor_id >= MAX_MOTORS) {
    return SIM_ERROR_INVALID_MOTOR_ID;
}

// Validate simulation is active
if (!motor_simulation_is_active()) {
    return SIM_ERROR_NOT_INITIALIZED;
}

// Parameter validation
if (speed_steps_per_sec < 0 || speed_steps_per_sec > MAX_SPEED) {
    return SIM_ERROR_INVALID_PARAMETER;
}
```

## Usage Examples

### Basic Movement Test
```c
void test_basic_movement(void) {
    // Initialize simulation
    TEST_ASSERT_EQUAL(SIM_ERROR_NONE, motor_simulation_init());
    TEST_ASSERT_EQUAL(SIM_ERROR_NONE, l6470_sim_init(0));
    
    // Set initial position
    l6470_sim_set_position(0, 0);
    
    // Command movement
    l6470_sim_move(0, 1000);
    
    // Simulate until completion
    while (l6470_sim_is_busy(0)) {
        motor_simulation_update(10);
    }
    
    // Verify final position
    TEST_ASSERT_EQUAL(1000, l6470_sim_get_position(0));
}
```

### Speed Ramp Test
```c
void test_speed_ramp(void) {
    l6470_sim_init(0);
    l6470_sim_set_acceleration(0, 500.0f);
    l6470_sim_set_max_speed(0, 2000.0f);
    
    // Start at low speed
    l6470_sim_run(0, 100.0f);
    
    // Gradually increase speed
    for (int i = 0; i < 20; i++) {
        motor_simulation_update(100);  // 100ms steps
        float current_speed = l6470_sim_get_speed(0);
        printf("Speed at %d ms: %.1f steps/sec\n", i * 100, current_speed);
    }
    
    // Check final speed
    float final_speed = l6470_sim_get_speed(0);
    TEST_ASSERT_FLOAT_WITHIN(10.0f, 100.0f, final_speed);
}
```

### Fault Injection Test
```c
void test_fault_handling(void) {
    l6470_sim_init(0);
    
    // Normal operation
    l6470_sim_move(0, 500);
    motor_simulation_update(100);
    TEST_ASSERT_FALSE(l6470_sim_get_faults(0));
    
    // Inject undervoltage fault
    l6470_sim_set_fault(0, L6470_STATUS_UVLO);
    
    // Verify fault detection
    uint16_t status = l6470_sim_get_status(0);
    TEST_ASSERT_TRUE(status & L6470_STATUS_UVLO);
    
    // Clear fault and verify recovery
    l6470_sim_clear_faults(0);
    status = l6470_sim_get_status(0);
    TEST_ASSERT_FALSE(status & L6470_STATUS_UVLO);
}
```

## Related Documentation
- [Hardware Simulation API](hardware_simulation_api.md)
- [AS5600 Driver API](as5600_driver_api.md)
- [L6470 Driver API](l6470_driver_api.md)
- [Testing Framework Guide](../guides/testing_framework_guide.md)
- [Simulation Framework Guide](../guides/SIMULATION_FRAMEWORK_GUIDE.md)
