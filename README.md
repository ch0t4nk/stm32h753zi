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
- **32,200+ searchable keywords** across STM32H7 peripherals and Nucleo BSP
- **Modular Copilot instructions** for architecture and HAL implementation
- **Real-time documentation search** for efficient development

### Documentation Search Tools
```bash
# Search STM32H7 peripheral documentation
python3 scripts/search_enhanced_docs.py peripheral SPI --scope STM32H7

# Find specific HAL functions and L6470 registers
python3 scripts/search_enhanced_docs.py function HAL_SPI_Init --scope STM32H7
python3 scripts/search_enhanced_docs.py register ABS_POS --scope L6470

# Search Nucleo BSP board functions
python3 scripts/search_enhanced_docs.py function BSP_LED --scope NUCLEO_BSP

# Explore programming concepts across all documentation
python3 scripts/search_enhanced_docs.py concept stepper --scope all
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
- **Documentation**: 91.1MB comprehensive reference (STM32H7 + L6470 + Nucleo BSP)  
- **Testing**: Unity, GoogleTest frameworks
- **Validation**: SSOT consistency checking, pre-commit hooks

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
00_reference/           # STM32H7 + L6470 + Nucleo BSP documentation (READ-ONLY)
scripts/                # Consolidated development tools
docs/                   # Project documentation and summaries
tests/                  # Unit and integration tests
```

For complete file organization guidelines, see `.github/instructions/file-organization.instructions.md`.

## Version System
- **Semantic Versioning**: MAJOR.MINOR.PATCH-PRERELEASE+BUILD
- **Auto-generation**: Git commit SHA and build metadata
- **Build Integration**: Version embedded in firmware binary

## Documentation System
This project features an **enhanced documentation integration system**:
- **Comprehensive Coverage**: 91.1MB total (STM32H7 + L6470 + Nucleo BSP documentation)
- **Searchable Indexes**: 32,200+ keywords, 11 peripherals, 12,700+ functions
- **Two-Tier Instructions**: Architecture principles + HAL implementation guidance
- **Real-time Search**: Instant peripheral/function/concept lookup across all documentation

See `docs/ENHANCED_INSTRUCTION_SYSTEM.md` for complete documentation system overview.

---

**Safety First, SSOT Always, Modular Design Throughout.**
