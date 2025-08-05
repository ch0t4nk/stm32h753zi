# Quick Start Guide

## Overview
Get started quickly with the STM32H753ZI motor control system. This guide covers essential setup and basic operations.

## Prerequisites

### Hardware Required
- STM32H753ZI Nucleo-144 development board
- X-NUCLEO-IHM02A1 stepper motor expansion shield
- 2x Stepper motors (NEMA 17 recommended)
- 2x AS5600 magnetic encoders
- Power supply (12-24V, 2A minimum)
- ST-LINK programmer (integrated on Nucleo board)

### Software Required
- ARM GCC toolchain
- CMake (3.16 or later)
- OpenOCD (for programming/debugging)
- Git

## Hardware Setup

### 1. Assembly
```
1. Mount X-NUCLEO-IHM02A1 shield on STM32H753ZI Nucleo board
2. Connect stepper motors to M1 and M2 terminals
3. Install AS5600 encoders on motor shafts
4. Connect power supply to shield power input
5. Connect ST-LINK USB cable to PC
```

### 2. Connections
```
Emergency Stop Button → PA9 (ESTOP_PIN)
Motor 1 Encoder SDA  → PB7 (I2C1_SDA)  
Motor 1 Encoder SCL  → PB6 (I2C1_SCL)
Motor 2 Encoder SDA  → PB11 (I2C2_SDA)
Motor 2 Encoder SCL  → PB10 (I2C2_SCL)
User LED            → PB0, PB7, PB14 (Green, Blue, Red)
```

### 3. Safety Setup
- Install emergency stop button (normally closed)
- Verify power supply voltage and current ratings
- Check motor wiring polarity
- Test emergency stop functionality before operation

## Software Setup

### 1. Clone Repository
```bash
git clone <repository-url>
cd stm32h753-motor-control
```

### 2. Build Firmware
```bash
# Configure build
cmake -S . -B build

# Compile firmware
cmake --build build

# Verify build output
ls build/stm32h753_ihm02a1.elf
```

### 3. Run Tests
```bash
# Run unit tests (host-based)
cd build && ctest

# Validate SSOT configuration
python3 scripts/validate_ssot.py

# Expected output: All tests pass
```

### 4. Program Device
```bash
# Flash firmware to STM32H753ZI
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg -c "program build/stm32h753_ihm02a1.elf verify reset exit"
```

## Basic Operation

### 1. System Initialization
```c
#include "main.h"
#include "controllers/motor_control.h"
#include "safety/emergency_stop.h"

int main(void) {
    // Initialize HAL
    HAL_Init();
    SystemClock_Config();
    
    // Initialize safety systems FIRST
    emergency_stop_init();
    safety_systems_init();
    
    // Initialize motor control
    motor_control_init();
    
    // Initialize communication
    uart_init();
    can_init();
    
    // Enter main control loop
    while (1) {
        motor_control_task();
        safety_monitor_task();
        communication_task();
        HAL_Delay(1); // 1ms control loop
    }
}
```

### 2. Basic Motor Control
```c
// Move Motor 1 to position 1000 steps
motor_move_to_position(MOTOR_1, 1000);

// Run Motor 2 at constant speed
motor_run(MOTOR_2, DIRECTION_FORWARD, 500); // 500 steps/sec

// Stop motors
motor_soft_stop(MOTOR_1);
motor_soft_stop(MOTOR_2);
```

### 3. Safety Operations
```c
// Check system status
SystemState_t state = system_get_state();

// Trigger emergency stop if needed
if (fault_detected) {
    software_emergency_stop(ESTOP_REASON_FAULT_DETECTED);
}

// Monitor motor faults
MotorStatus_t status;
motor_get_status(MOTOR_1, &status);
if (status.faults.overcurrent) {
    handle_motor_fault(MOTOR_1, FAULT_OVERCURRENT);
}
```

## Configuration

### 1. Motor Parameters (SSOT)
Edit `src/config/motor_config.h`:
```c
// Motor acceleration/deceleration
#define MOTOR_ACCELERATION              0x1000    // Adjust for your motor
#define MOTOR_MAX_SPEED                 0x1000    // Maximum safe speed

// Current settings
#define MOTOR_KVAL_RUN                  0x50      // Running current
#define MOTOR_OCD_THRESHOLD             0x08      // Overcurrent limit
```

### 2. Safety Parameters (SSOT)
Edit `src/config/safety_config.h`:
```c
// Emergency stop timing
#define EMERGENCY_STOP_MAX_RESPONSE_TIME_MS    100

// Fault detection intervals
#define FAULT_DETECTION_INTERVAL_MS            50
#define WATCHDOG_TIMEOUT_MS                    5000
```

### 3. Hardware Configuration (SSOT)
Edit `src/config/hardware_config.h`:
```c
// Emergency stop pin
#define ESTOP_PIN                       GPIO_PIN_9
#define ESTOP_GPIO_PORT                GPIOA

// Motor control pins (verify for your setup)
#define MOTOR1_CS_PIN                   GPIO_PIN_5
#define MOTOR1_CS_PORT                 GPIOA
```

## Testing and Validation

### 1. Safety System Test
```bash
# Run comprehensive safety tests
python3 scripts/run_safety_validation.py

# Expected output: All safety tests pass
```

### 2. Motor Function Test
```c
// Test motor movement
void test_basic_motor_operation(void) {
    // Initialize motors
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_init(MOTOR_1));
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_init(MOTOR_2));
    
    // Test position move
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_move_to_position(MOTOR_1, 100));
    
    // Wait for completion
    while (motor_is_moving(MOTOR_1)) {
        HAL_Delay(10);
    }
    
    // Verify position
    int32_t position = motor_get_position(MOTOR_1);
    TEST_ASSERT_INT32_WITHIN(5, 100, position);
}
```

### 3. Emergency Stop Test
```c
// Test emergency stop response
void test_emergency_stop_basic(void) {
    // Start motor
    motor_run(MOTOR_1, DIRECTION_FORWARD, 500);
    HAL_Delay(100);
    
    // Trigger emergency stop
    uint32_t start_time = HAL_GetTick();
    emergency_stop_trigger();
    
    // Wait for stop
    while (motor_is_moving(MOTOR_1)) {
        // Monitor
    }
    uint32_t stop_time = HAL_GetTick();
    
    // Verify response time
    TEST_ASSERT_LESS_THAN(100, stop_time - start_time);
    TEST_ASSERT_EQUAL(SYSTEM_STATE_EMERGENCY, system_get_state());
}
```

## Common Issues and Solutions

### Build Issues
```bash
# Issue: CMake not found
sudo apt install cmake

# Issue: ARM toolchain not found
sudo apt install gcc-arm-none-eabi

# Issue: Missing dependencies
sudo apt install openocd gdb-multiarch
```

### Hardware Issues
```bash
# Issue: Motors not responding
1. Check power supply connections
2. Verify shield mounting
3. Test with multimeter
4. Check motor wiring

# Issue: Encoder not reading
1. Verify I2C connections
2. Check magnet alignment
3. Test I2C communication
4. Verify pull-up resistors
```

### Programming Issues
```bash
# Issue: ST-LINK not detected
1. Check USB cable connection
2. Install ST-LINK drivers
3. Try different USB port
4. Reset board with RESET button

# Issue: Flash programming failed
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg -c "init; reset halt; flash erase_sector 0 0 last; reset; exit"
```

## Next Steps

### 1. Advanced Features
- Study [Motor Control Guide](motor_control.md) for advanced motion control
- Review [Safety Systems Guide](safety_systems.md) for comprehensive safety
- Explore [Communication Guide](communication.md) for networking

### 2. Customization
- Modify SSOT configuration files for your specific motors
- Implement custom motion profiles
- Add application-specific safety checks

### 3. Integration
- Review [API Documentation](../html/html/index.html) for detailed function reference
- Study [Safety Validation](../validation/safety_validation.md) for testing procedures
- Follow [Development Guidelines](../design/development_guidelines.md)

## Support Resources

### Documentation
- [Complete API Reference](../html/html/index.html)
- [Safety Validation Procedures](../validation/safety_validation.md)
- [Enhanced Documentation Search](../README.md#enhanced-documentation-search)

### Tools
```bash
# Search documentation
python3 scripts/search_enhanced_docs.py function HAL_GPIO_Init --scope STM32H7

# Validate configuration
python3 scripts/validate_ssot.py

# Generate documentation
doxygen docs/Doxyfile
```

### Debugging
```bash
# Start GDB debugging session
arm-none-eabi-gdb build/stm32h753_ihm02a1.elf
(gdb) target extended-remote :3333
(gdb) monitor reset halt
(gdb) load
(gdb) continue
```

Remember: **Always test safety systems before operating motors!**
