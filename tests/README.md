# Unit & Integration Tests

## Overview
This directory contains unit and integration tests for the STM32H753ZI stepper motor control project.

## Enhanced Testing Framework Integration
**Reference Assets**: Leverage comprehensive testing documentation and examples:
- **STM32H7 Testing**: Use `python3 scripts/documentation/search_docs.py concept testing` for STM32H7-specific testing patterns
- **Test Examples**: `00_reference/STM32Cube_FW_H7_V1.12.0/Projects/` contains test implementations for STM32H7
- **HAL Testing**: Test STM32H7 HAL functions with proper error handling and safety validation
- **Integration Guidance**: Follow `.github/instructions/` files for component integration testing

## Testing Framework Setup
- **Unity (C)**: Embedded unit testing framework for C drivers
- **GoogleTest (C++)**: Unit testing framework for C++ interfaces
- **CTest/Ceedling**: Build system integration for automated testing
- **Hardware-in-Loop**: STM32H753ZI Nucleo board testing with enhanced documentation references

## Development Tasks
- TODO: Add Unity (C) and GoogleTest (C++) tests with STM32H7 HAL validation
- TODO: Integrate Ceedling/CTest with enhanced build configuration
- TODO: Implement safety system tests following enhanced instruction guidance
- TODO: Create hardware abstraction tests for L6470 and AS5600 drivers

## Testing Strategy
Follow comprehensive testing patterns from:
- `.github/instructions/safety-systems.instructions.md` for safety-critical test requirements
- `.github/instructions/error-handling.instructions.md` for fault injection testing
- Enhanced documentation system for STM32H7-specific testing best practices
