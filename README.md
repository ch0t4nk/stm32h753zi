# stm32h753_ihm02a1 Workspace

![Version](https://img.shields.io/badge/version-auto-blue)

## Overview
Stepper motor control firmware for STM32H753ZI (NUCLEO-H753ZI) + X-NUCLEO-IHM02A1, with CAN-FD, Ethernet REST API, and I2C encoder feedback. Safety, modularity, and CI/CD ready.

## Quickstart
1. Clone repo & checkout workspace branch
2. Build with CMake or Make
3. Flash to NUCLEO-H753ZI
4. Run host scripts for API/CAN demo

## API References
- REST: `api/openapi.yaml`
- CAN: `api/can_api.yaml`

## Build/Flash
- See `CMakeLists.txt` or `Makefile`
- Example targets: stepper_control, encoder_read, api_demo, can_api_demo

## Version Badge
- Auto-generated from git SHA

---

TODO: See `.instructions/README.md` for module-specific details.
