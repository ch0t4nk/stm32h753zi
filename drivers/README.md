# Hardware Abstraction Drivers

## Overview
This directory contains hardware abstraction layer (HAL) drivers for the STM32H753ZI stepper motor control project.

## 🚀 Current Status: Ready for Driver Implementation
**Peripheral Initialization Complete**: All STM32H753ZI peripherals are configured and ready for driver development.

### ✅ Initialized Peripherals
```c
extern SPI_HandleTypeDef hspi2;      // ✅ L6470 stepper drivers (PB13/14/15, PA9)
extern I2C_HandleTypeDef hi2c1;      // ✅ AS5600 encoder 1 (PB6/7)
extern I2C_HandleTypeDef hi2c2;      // ✅ AS5600 encoder 2 (PB10/11)
extern TIM_HandleTypeDef htim2;      // ✅ 1kHz control loop timer
extern FDCAN_HandleTypeDef hfdcan1;  // ✅ MCU communication (PD0/1)
```

## 🤖 Enhanced Documentation & Automation
**Semantic Search Integration**: Use AI-powered documentation search with real embeddings:
```bash
# NEW: Production semantic search with AI understanding
./scripts/stm32_search.sh function "HAL_SPI_Init" --scope STM32H7
./scripts/stm32_search.sh concept "L6470 configuration" --scope L6470  
./scripts/stm32_search.sh function "BSP_LED_Init" --scope NUCLEO_BSP
./scripts/stm32_search.sh peripheral "I2C" --scope all
```

**Automated Development**: STATUS.md automatically updates after every commit, providing real-time project context for development decisions.

## Enhanced Documentation Integration
**Reference Assets**: Leverage comprehensive STM32H7 and component documentation:
- **STM32H7 HAL Documentation**: Use `python3 scripts/search_enhanced_docs.py peripheral <PERIPHERAL> --scope STM32H7` for implementation details
- **Nucleo BSP Functions**: Use `python3 scripts/search_enhanced_docs.py function BSP_LED --scope NUCLEO_BSP` for board support
- **Component References**: 
  - L6470 drivers: See `00_reference/x_cube_spn2_markdown_docs/` and `.github/instructions/stm32h7-spi-l6470.instructions.md`
  - AS5600 encoders: See `.github/instructions/stm32h7-i2c-as5600.instructions.md`
  - GPIO/General: See `.github/instructions/stm32h7-gpio-hal.instructions.md`
- **HAL Examples**: `00_reference/STM32Cube_FW_H7_V1.12.0/Projects/` contains working driver implementations
- **BSP Functions**: `00_reference/stm32h7xx_nucleo_bsp_md/` contains Nucleo-144 board support documentation

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
