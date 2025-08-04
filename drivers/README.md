# Hardware Abstraction Drivers

## Overview
This directory contains hardware abstraction layer (HAL) drivers for the STM32H753ZI stepper motor control project.

## Enhanced Documentation Integration
**Reference Assets**: Leverage comprehensive STM32H7 and component documentation:
- **STM32H7 HAL Documentation**: Use `python3 scripts/documentation/search_docs.py peripheral <PERIPHERAL>` for implementation details
- **Component References**: 
  - L6470 drivers: See `00_reference/x_cube_spn2_markdown_docs/` and `.github/instructions/stm32h7-spi-l6470.instructions.md`
  - AS5600 encoders: See `.github/instructions/stm32h7-i2c-as5600.instructions.md`
  - GPIO/General: See `.github/instructions/stm32h7-gpio-hal.instructions.md`
- **HAL Examples**: `00_reference/STM32Cube_FW_H7_V1.12.0/Projects/` contains working driver implementations

## Development Tasks
- TODO: Add C++ interfaces for L6470, AS5600, CAN, Ethernet using enhanced documentation guidance
- TODO: Support mocking for simulation with STM32H7 HAL compatibility
- TODO: Implement drivers following modular instruction system patterns

## Implementation Guidance
Follow the comprehensive instruction files in `.github/instructions/` for:
- Hardware pin configurations
- Peripheral-specific HAL implementations
- Error handling and safety integration
- SSOT configuration management
