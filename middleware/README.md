# Middleware & Simulation Hooks

## Overview
This directory contains middleware components and simulation interfaces for the STM32H753ZI stepper motor control project.

## Enhanced Middleware Development
**Reference Assets**: Leverage comprehensive STM32H7 middleware documentation:
- **STM32H7 Middleware**: Use `python3 scripts/search_enhanced_docs.py concept middleware stm32h7` for STM32H7 middleware patterns
- **Communication Middleware**: See `.github/instructions/comm-protocols.instructions.md` for protocol abstraction guidance
- **Simulation Support**: `00_reference/STM32Cube_FW_H7_V1.12.0/` contains middleware examples and patterns
- **Integration Examples**: STM32H7 HAL middleware integration with FreeRTOS, LwIP, and USB

## Middleware Components
- **Communication Abstraction**: Protocol-agnostic interfaces for UART, CAN, Ethernet, SPI, I2C
- **Simulation Hooks**: Hardware abstraction for virtual/real bus testing
- **Safety Middleware**: Watchdog and fault detection middleware following enhanced safety guidance
- **RTOS Integration**: FreeRTOS middleware with STM32H7 optimizations

## Development Tasks
- TODO: Add communication simulation hooks for real/virtual buses using enhanced protocol documentation
- TODO: Implement safety middleware following `.github/instructions/safety-systems.instructions.md`
- TODO: Create hardware abstraction middleware using STM32H7 HAL documentation references
- TODO: Integrate RTOS middleware with enhanced timing and safety requirements

## Implementation Guidance
Follow enhanced documentation system for:
- Protocol abstraction patterns from comprehensive communication documentation
- Safety middleware requirements from enhanced safety instruction guidance
- STM32H7-specific middleware optimization using comprehensive HAL documentation
