# Bootloader Template

## Overview
Bootloader implementation for STM32H753ZI with support for CAN-FD and Ethernet Device Firmware Update (DFU).

## Development Tasks
- TODO: Implement CAN-FD DFU logic following `.github/instructions/comm-protocols.instructions.md`
- TODO: Implement Ethernet DFU logic with proper error handling
- TODO: Embed git SHA in binaries using `.github/instructions/build-config.instructions.md`
- TODO: Follow `.github/instructions/safety-systems.instructions.md` for secure boot validation

## Enhanced Development Guidance
**Reference Assets**: Leverage comprehensive STM32H7 bootloader documentation:
- **STM32H7 Bootloader**: Use `python3 scripts/search_enhanced_docs.py concept bootloader stm32h7` for STM32H7 bootloader patterns
- **Communication Protocols**: See `.github/instructions/comm-protocols.instructions.md` for CAN-FD and Ethernet implementation
- **Build Integration**: Follow `.github/instructions/build-config.instructions.md` for version embedding
- **Safety Systems**: Implement secure boot following `.github/instructions/safety-systems.instructions.md`
