# STM32H753ZI Project - Consolidated Copilot Instructions

This directory contains domain-based instruction files that provide comprehensive guidance for the STM32H753ZI stepper motor control project. The instruction system has been consolidated from 39 granular files to 6 domain-based files (85% reduction) with robust VS Code Copilot scoping.

## 🤖 Enhanced Development Automation (NEW!)

**STATUS.md Automation Phase 3 Complete**: The project now features comprehensive automation for enhanced development productivity:

- **✅ Git Hooks**: Automatic documentation updates via post-commit hooks with intelligent loop prevention
- **✅ Real-time Monitoring**: Live project status tracking with optimized git process management
- **✅ VS Code Integration**: Enhanced tasks for automated build+update, preview, and manual triggers
- **✅ Performance Optimization**: <1s response times through filesystem-first checking and strict timeouts
- **✅ Smart Detection**: Merge commit handling, build context analysis, session continuity tracking

The automation system ensures all documentation (including these instructions) reflects the current project state automatically.

## 📁 Domain-Based Instruction System

### Core Instructions (6 Files)
- [`project-setup.instructions.md`](./project-setup.instructions.md) - Workspace setup, build configuration, testing infrastructure, optimization
  - **Scope**: `**/*.{c,h,md,cmake,txt,json,yml,yaml}`
- [`hardware.instructions.md`](./hardware.instructions.md) - Hardware configuration, GPIO control, memory management, STM32H7 HAL
  - **Scope**: `src/drivers/**/*.{c,h}`
- [`comms.instructions.md`](./comms.instructions.md) - Communication protocols, UART/CAN/SPI/I2C interfaces, networking
  - **Scope**: `src/communication/**/*.{c,h}`
- [`core-software.instructions.md`](./core-software.instructions.md) - Data types, error handling, state management, motor control algorithms
  - **Scope**: `src/{common,controllers,drivers}/**/*.{c,h}`
- [`safety-rt.instructions.md`](./safety-rt.instructions.md) - Safety systems, real-time control, timing management, fail-safe mechanisms
  - **Scope**: `src/{safety,controllers}/**/*.{c,h}`
- [`api-ui.instructions.md`](./api-ui.instructions.md) - API design, user interfaces, documentation generation, external integration
  - **Scope**: `src/{communication,application}/**/*.{c,h}`

### Supporting Instructions
- [`ssot-config.instructions.md`](./ssot-config.instructions.md) - Single Source of Truth configuration management principles
- [`file-organization.instructions.md`](./file-organization.instructions.md) - File structure and asset placement guidelines
- [`cmake-build-system.instructions.md`](./cmake-build-system.instructions.md) - CMake build system configuration and troubleshooting

### Architecture Overview
The consolidated system provides:
- **Domain expertise** for specific code areas
- **VS Code Copilot scoping** with reliable brace-expansion patterns
- **Comprehensive coverage** of all original instruction content
- **Reduced complexity** while maintaining full functionality

## 🎯 Usage Guidelines

## 🎯 Usage Guidelines

### For Developers
- Reference these domain-based instructions for comprehensive guidance
- Follow the SSOT principle - all configuration should be centralized
- Use appropriate instruction file based on the domain you're working in

### For GitHub Copilot
- Each instruction file provides domain-specific context for code generation
- Follow safety-first principles outlined in safety-rt.instructions.md
- Always reference SSOT configuration headers as defined in ssot-config.instructions.md
- Use robust brace-expansion patterns for reliable VS Code scoping

### SSOT Configuration System
All configuration follows Single Source of Truth principles:
```c
// Hardware configurations
#include "config/hardware_config.h"
#include "config/comm_config.h"  
#include "config/motor_config.h"
#include "config/safety_config.h"
#include "config/build_config.h"
#include "config/documentation_config.h"
```

## 🔄 Maintenance

When adding new functionality:
1. Determine appropriate domain-based instruction file
2. Update the relevant instruction file with new guidance  
3. Ensure SSOT principles are maintained
4. Test VS Code Copilot scoping with brace-expansion patterns

## 📚 Related Documentation

- **MCSDK Integration**: [`docs/MCSDK_INTEGRATION_PLAN.md`](../../docs/MCSDK_INTEGRATION_PLAN.md)
- **Copilot Instructions**: [`../../.github/copilot-instructions.md`](../../.github/copilot-instructions.md)
- **Project Documentation**: [`docs/`](../../docs/)

---

**Key Improvements**: Reduced from 39 granular files to 6 domain-based files (85% reduction) with robust brace-expansion `applyTo` patterns for reliable VS Code Copilot scoping. All original content consolidated and enhanced with better organization.
