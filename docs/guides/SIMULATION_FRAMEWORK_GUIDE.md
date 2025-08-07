# STM32H753ZI Simulation Framework Guide

## Overview
The simulation framework enables safe development and testing of stepper motor control algorithms without hardware risk. This guide covers usage, configuration, and testing with the simulation system.

## Quick Start

### Enable Simulation Mode
```bash
# Build with simulation enabled (default for current build)
cmake -S . -B build/Debug --preset Debug
cmake --build build/Debug
```

The firmware is built with `SIMULATION_MODE=1` by default, enabling simulation throughout the codebase.

### Verify Simulation is Active
```c
#include "simulation/motor_simulation.h"

if (motor_simulation_is_active()) {
    printf("Simulation mode active\n");
} else {
    printf("Hardware mode active\n");
}
```

## Simulation Architecture

### Core Components
- **`motor_simulation.h/c`**: AS5600 encoder simulation with motion physics
- **`hardware_simulation.h/c`**: L6470 stepper driver register simulation
- **Generated Headers**: `l6470_registers_generated.h` + `as5600_registers_generated.h`
- **HAL Integration**: Simulation hooks in HAL abstraction layer

### Simulation Modes
```c
typedef enum {
    SIM_MODE_DISABLED = 0,     // Real hardware operation
    SIM_MODE_BASIC,            // Basic register simulation
    SIM_MODE_REALISTIC,        // Realistic motion physics
    SIM_MODE_FAULT_INJECTION   // Include fault scenarios
} SimulationMode_t;
```

## L6470 Stepper Driver Simulation

### Basic Usage
```c
#include "drivers/l6470/l6470_driver.h"

// Standard driver calls work transparently in simulation
SystemError_t result = l6470_init(0);  // Motor 0
if (result == SYSTEM_OK) {
    l6470_move(0, 1000);  // Move 1000 steps - simulated
    
    uint16_t status = l6470_get_status(0);  // Simulated status
    printf("Motor status: 0x%04X\n", status);
}
```

### Simulation Features
- **Register Simulation**: All L6470 registers simulated with realistic values
- **Motion Physics**: Position updates based on commanded moves
- **Status Flags**: Realistic status register responses (BUSY, DIR, etc.)
- **Fault Injection**: Configurable fault conditions for testing

### Available L6470 Simulation APIs
```c
// Motor control simulation
simulation_error_t l6470_sim_move(uint8_t motor_id, int32_t steps);
simulation_error_t l6470_sim_run(uint8_t motor_id, float speed);
simulation_error_t l6470_sim_stop(uint8_t motor_id);

// Status and monitoring
int32_t l6470_sim_get_position(uint8_t motor_id);
float l6470_sim_get_speed(uint8_t motor_id);
uint16_t l6470_sim_get_status(uint8_t motor_id);

// Configuration and testing
simulation_error_t l6470_sim_set_fault(uint8_t motor_id, uint16_t fault_mask);
simulation_error_t l6470_sim_clear_faults(uint8_t motor_id);
```

## AS5600 Encoder Simulation

### Basic Usage
```c
#include "drivers/as5600/as5600_driver.h"

// Standard driver calls work transparently in simulation
SystemError_t result = as5600_init(0);  // Encoder 0
if (result == SYSTEM_OK) {
    uint16_t angle = as5600_read_angle(0);  // Simulated angle
    printf("Encoder angle: %d\n", angle);
}
```

### Simulation Features
- **Position Tracking**: Encoder follows simulated motor position
- **Motion Patterns**: Configurable rotation patterns for testing
- **Noise Simulation**: Realistic sensor noise characteristics
- **Fault Injection**: Magnetic field strength simulation

### Available AS5600 Simulation APIs
```c
// Position simulation
simulation_error_t as5600_sim_set_position(uint8_t encoder_id, uint16_t position);
uint16_t as5600_sim_get_position(uint8_t encoder_id);

// Motion patterns
simulation_error_t as5600_sim_set_pattern(uint8_t encoder_id, 
                                         SimulationPattern_t pattern,
                                         float frequency);

// Status simulation
uint8_t as5600_sim_get_status(uint8_t encoder_id);
uint8_t as5600_sim_get_magnitude(uint8_t encoder_id);
```

## Testing with Simulation

### Unit Testing Example
```c
#include "unity.h"
#include "simulation/motor_simulation.h"
#include "drivers/l6470/l6470_driver.h"

void test_motor_move_simulation(void) {
    // Initialize simulation
    TEST_ASSERT_TRUE(motor_simulation_is_active());
    
    // Test motor movement
    SystemError_t result = l6470_move(0, 1000);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Verify simulated position
    int32_t position = l6470_sim_get_position(0);
    TEST_ASSERT_EQUAL(1000, position);
}
```

### Integration Testing
```c
void test_closed_loop_control(void) {
    // Start motor movement
    l6470_move(0, 500);
    
    // Verify encoder follows
    for (int i = 0; i < 10; i++) {
        motor_simulation_update(100);  // 100ms simulation step
        
        uint16_t encoder_pos = as5600_read_angle(0);
        int32_t motor_pos = l6470_sim_get_position(0);
        
        // Verify encoder tracks motor
        TEST_ASSERT_INT_WITHIN(10, motor_pos, encoder_pos);
    }
}
```

## Configuration

### Simulation Parameters
```c
// Motor simulation configuration
#define SIM_MAX_MOTORS 2
#define SIM_MAX_ENCODERS 2
#define SIM_POSITION_RESOLUTION 4096
#define SIM_TICK_RATE_HZ 1000

// Physics simulation parameters
#define SIM_DEFAULT_ACCELERATION 1000  // steps/s²
#define SIM_DEFAULT_MAX_SPEED 2000     // steps/s
#define SIM_POSITION_NOISE_LEVEL 2     // ±2 counts noise
```

### Build Configuration
```cmake
# Enable simulation in CMakeLists.txt
add_compile_definitions(SIMULATION_MODE=1)
add_compile_definitions(SIMULATION_ENABLED=1)

# Link simulation libraries
target_link_libraries(stm32h753_ihm02a1 PRIVATE
    simulation_motor
    simulation_hardware
)
```

## Troubleshooting

### Common Issues

**1. Simulation Not Active**
```c
// Check if simulation is properly enabled
if (!motor_simulation_is_active()) {
    printf("ERROR: Simulation not active - check SIMULATION_MODE=1\n");
}
```

**2. Register Conflicts**
- **Issue**: Redefinition warnings for L6470/AS5600 constants
- **Solution**: Use generated headers `l6470_registers_generated.h` and `as5600_registers_generated.h`
- **Note**: Warnings during SSOT transition are normal

**3. HAL Abstraction Issues**
```c
// Verify HAL abstraction is working
SystemError_t hal_result = HAL_Abstraction_SPI_Init(HAL_SPI_INSTANCE_2);
if (hal_result != SYSTEM_OK) {
    printf("HAL abstraction initialization failed\n");
}
```

### Debug Features
```c
// Enable simulation debugging
#define SIM_DEBUG_ENABLED 1

// Get detailed simulation state
SimulationStatus_t status;
hardware_simulation_get_status(&status);
printf("Motor position: %d, speed: %.2f\n", 
       status.motor_position, status.motor_speed);
```

## Performance Considerations

### Simulation Overhead
- **Minimal Impact**: Simulation adds <5% overhead to normal operation
- **Memory Usage**: ~2KB additional RAM for simulation state
- **Real-time**: Simulation runs at 1kHz update rate

### Optimization Tips
- Use `SIM_MODE_BASIC` for simple register testing
- Use `SIM_MODE_REALISTIC` for motion algorithm validation
- Enable `SIM_MODE_FAULT_INJECTION` only when testing error handling

## Advanced Features

### Custom Motion Patterns
```c
// Define custom encoder patterns
typedef enum {
    SIM_PATTERN_STATIC = 0,
    SIM_PATTERN_CONTINUOUS_ROTATION,
    SIM_PATTERN_OSCILLATION,
    SIM_PATTERN_STEP_RESPONSE,
    SIM_PATTERN_CUSTOM
} SimulationPattern_t;

// Set custom pattern
as5600_sim_set_pattern(0, SIM_PATTERN_OSCILLATION, 0.5f);  // 0.5 Hz
```

### Fault Injection
```c
// Inject L6470 faults for testing
l6470_sim_set_fault(0, L6470_STATUS_UVLO);     // Undervoltage
l6470_sim_set_fault(0, L6470_STATUS_OCD);      // Overcurrent
l6470_sim_set_fault(0, L6470_STATUS_TH_SD);    // Thermal shutdown

// Clear all faults
l6470_sim_clear_faults(0);
```

### Linked Motor-Encoder Simulation
```c
// Motors and encoders automatically linked
l6470_move(0, 1000);              // Move motor 0
uint16_t pos = as5600_read_angle(0);  // Encoder 0 follows automatically
```

## Related Documentation
- [HAL Abstraction Guide](hal_abstraction_guide.md)
- [Dual Build System Instructions](../../.github/instructions/dual-build-system.instructions.md)
- [Testing Framework Guide](testing_framework_guide.md)
- [Core Software Instructions](../../.github/instructions/core-software.instructions.md)

## API Reference
- [Motor Simulation API](../api/motor_simulation_api.md)
- [Hardware Simulation API](../api/hardware_simulation_api.md)
- [L6470 Driver API](../api/l6470_driver_api.md)
- [AS5600 Driver API](../api/as5600_driver_api.md)
