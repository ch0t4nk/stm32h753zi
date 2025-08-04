# STM32H753ZI Stepper Motor Control Project

![Version](https://img.shields.io/badge/version-auto-blue)

## Overview
Advanced stepper motor control firmware for **STM32H753ZI Nucleo-144** with **X-NUCLEO-IHM02A1** shield, featuring:
- **Dual L6470 stepper drivers** with daisy-chain SPI communication
- **AS5600 magnetic encoders** for closed-loop position feedback  
- **Multi-protocol communication**: UART, CAN-FD, Ethernet REST API
- **Comprehensive safety systems** with watchdog and fault recovery
- **SSOT architecture** with centralized configuration management
- **Enhanced documentation system** with 86MB STM32H7 reference materials

## Enhanced Development Experience
This project features a **comprehensive documentation integration system**:
- **31,772 searchable keywords** across STM32H7 peripherals
- **Modular Copilot instructions** for architecture and HAL implementation
- **Real-time documentation search** for efficient development

### Documentation Search Tools
```bash
# Search STM32H7 peripheral documentation
python3 scripts/documentation/search_docs.py peripheral SPI

# Find specific HAL functions
python3 scripts/documentation/search_docs.py function HAL_SPI_Init

# Explore programming concepts  
python3 scripts/documentation/search_docs.py concept DMA
```

## Quickstart
1. Clone repository and open in VS Code dev container
2. Build: `cmake -S . -B build && cmake --build build`
3. Flash to NUCLEO-H753ZI: Use "Flash STM32H753ZI" task
4. Monitor via UART: 115200 baud on ST-Link Virtual COM Port

## Architecture
- **MCU**: STM32H753ZI (ARM Cortex-M7 @ 480MHz, 2MB Flash, 1MB RAM)
- **Motor Drivers**: 2× L6470 dSPIN (up to 7A, 1/128 microstepping)
- **Position Feedback**: 2× AS5600 magnetic encoders (12-bit resolution)
- **Communication**: UART (debug), CAN-FD, Ethernet (LAN8742 PHY)
- **Safety**: Independent watchdog, L6470 fault monitoring, thermal protection

## API References
- REST: `api/openapi.yaml`
- CAN: `api/can_api.yaml`

## Development Environment
- **Dev Container**: Ubuntu 22.04.5 LTS with complete embedded toolchain
- **Toolchain**: ARM GCC, OpenOCD, CMake, Doxygen
- **Documentation**: 86MB STM32H7 reference + 2.1MB L6470 documentation  
- **Testing**: Unity, GoogleTest frameworks
- **Validation**: SSOT consistency checking, pre-commit hooks

## Build System & Tasks
- See `CMakeLists.txt` or `Makefile`
- Example targets: stepper_control, encoder_read, api_demo, can_api_demo

## Build System & Tasks
Available VS Code tasks:
- **Build (CMake)**: `cmake -S . -B build && cmake --build build`
- **Flash STM32H753ZI**: Program firmware via OpenOCD + ST-Link
- **Run Tests (CTest)**: Execute unit and integration tests
- **Generate Docs (Doxygen)**: Create API documentation
- **Validate SSOT**: Check configuration consistency

### Example Build Commands
```bash
# Build project
cmake -S . -B build && cmake --build build

# Run tests
cd build && ctest

# Flash to hardware
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg 
        -c "program build/stm32h753_ihm02a1.elf verify reset exit"
```

## Project Structure
```
src/
├── config/              # SSOT configuration headers
├── common/              # SSOT common definitions  
├── drivers/             # Hardware abstraction layer
├── controllers/         # Control algorithms and logic
├── communication/       # Protocol implementations
├── safety/              # Safety monitoring and response
└── application/         # Main application logic

.github/instructions/    # Modular Copilot instruction system
00_reference/           # STM32H7 + L6470 documentation (READ-ONLY)
scripts/documentation/  # Documentation search and indexing tools
```

## Version System
- **Semantic Versioning**: MAJOR.MINOR.PATCH-PRERELEASE+BUILD
- **Auto-generation**: Git commit SHA and build metadata
- **Build Integration**: Version embedded in firmware binary

## Documentation System
This project features an **enhanced documentation integration system**:
- **Comprehensive Coverage**: 86MB STM32H7 + 2.1MB L6470 documentation
- **Searchable Indexes**: 31,772 keywords, 11 peripherals, 12,704 functions
- **Two-Tier Instructions**: Architecture principles + HAL implementation guidance
- **Real-time Search**: Instant peripheral/function/concept lookup

See `docs/ENHANCED_INSTRUCTION_SYSTEM.md` for complete documentation system overview.

---

**Safety First, SSOT Always, Modular Design Throughout.**
