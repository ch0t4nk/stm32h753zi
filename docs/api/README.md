# STM32H753ZI Motor Control API Reference

## Overview
This API reference provides comprehensive documentation for the STM32H753ZI stepper motor control system with X-NUCLEO-IHM02A1 shield.

## Core Modules

### Safety Systems (`src/safety/`)
- **Emergency Stop API**: Hardware and software emergency stop implementations
- **Fault Detection**: Motor, encoder, and communication fault monitoring
- **System Monitoring**: Watchdog, thermal, and overcurrent protection

### Motor Control (`src/controllers/`)
- **Stepper Motor Control**: L6470 driver interface and motion control
- **Position Control**: Closed-loop position control with AS5600 encoders
- **Motion Planning**: Trajectory generation and velocity profiling

### Communication (`src/communication/`)
- **UART API**: Configuration, transmission, and error handling
- **CAN API**: Protocol implementation and message management
- **SPI Interface**: L6470 and AS5600 communication protocols

### Hardware Drivers (`src/drivers/`)
- **GPIO Control**: Pin configuration and state management
- **Peripheral Drivers**: Timer, ADC, and DMA interfaces
- **Board Support**: STM32H753ZI Nucleo-144 specific functions

### Common Utilities (`src/common/`)
- **Error Handling**: System error codes and recovery procedures
- **Data Types**: Common data structures and type definitions
- **Utilities**: Mathematical functions and helper routines

## Configuration (SSOT)
All system parameters are defined in single source of truth headers:
- `src/config/hardware_config.h` - Hardware pin assignments
- `src/config/motor_config.h` - Motor control parameters
- `src/config/comm_config.h` - Communication settings
- `src/config/safety_config.h` - Safety thresholds and limits

## Safety Considerations
⚠️ **CRITICAL**: This system controls stepper motors. Always:
- Implement proper emergency stop procedures
- Monitor fault conditions continuously
- Validate all input parameters
- Use fail-safe defaults
- Test safety systems regularly

## Quick Start
See the [Developer Guide](../guides/quick_start.md) for getting started with the API.

## Generated Documentation
- [HTML API Reference](../html/index.html) - Complete API documentation
- [Module Index](../html/modules.html) - Organized by functional modules
- [File Index](../html/files.html) - Source file documentation
