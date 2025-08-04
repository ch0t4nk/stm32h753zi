# FreeRTOS Real-Time Operating System

## Overview
This directory contains FreeRTOS integration and task management for the STM32H753ZI stepper motor control project.

## Enhanced RTOS Development
**Reference Assets**: Leverage comprehensive STM32H7 and FreeRTOS documentation:
- **STM32H7 FreeRTOS**: Use `python3 scripts/documentation/search_docs.py concept RTOS` for STM32H7 FreeRTOS integration
- **FreeRTOS Examples**: `00_reference/STM32Cube_FW_H7_V1.12.0/Middlewares/Third_Party/FreeRTOS/` contains STM32H7-optimized FreeRTOS
- **Timing Requirements**: See `.github/instructions/time-handling.instructions.md` for real-time constraint management
- **Safety Integration**: Follow `.github/instructions/safety-systems.instructions.md` for safety-critical task design

## FreeRTOS Task Architecture
- **Motor Control Task**: High-priority stepper motor control with deterministic timing
- **Communication Task**: Protocol handling for UART, CAN, Ethernet interfaces
- **Safety Monitor Task**: Continuous safety system monitoring and fault detection
- **Encoder Task**: AS5600 position feedback processing with precise timing
- **Watchdog Task**: Independent watchdog management and system health monitoring

## Development Tasks
- TODO: Add FreeRTOS tasks for motor control, communications, and safety using enhanced STM32H7 guidance
- TODO: Implement real-time constraints following enhanced timing instruction guidance
- TODO: Integrate safety-critical task management with enhanced safety system requirements
- TODO: Optimize FreeRTOS configuration for STM32H753ZI performance using comprehensive documentation

## Implementation Guidance
Follow enhanced documentation system for:
- STM32H7-specific FreeRTOS configuration and optimization
- Real-time task design patterns from comprehensive timing documentation
- Safety-critical RTOS implementation from enhanced safety instruction guidance
- Inter-task communication using STM32H7 HAL and FreeRTOS primitives
