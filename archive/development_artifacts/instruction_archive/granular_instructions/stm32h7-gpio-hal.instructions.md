---
applyTo: "src/drivers/**/*.c"
description: "STM32H7 GPIO HAL implementation guidance with comprehensive documentation references"
---

# STM32H7 GPIO Implementation Instructions

## Overview
This instruction file provides STM32H753ZI-specific GPIO HAL implementation guidance, leveraging comprehensive STM32H7 documentation for precise peripheral configuration.

## Enhanced Documentation Integration
**Reference Assets**: This instruction leverages the comprehensive STM32H7 GPIO documentation system:
- **GPIO Documentation Coverage**: 198+ STM32H7 GPIO files with unified semantic categorization
- **STM32H7xx Nucleo BSP Documentation**: 42 files with board-specific GPIO implementations
- **Copilot-Optimized GPIO Search**:
  ```bash
  # Search GPIO peripheral with token efficiency
  python3 scripts/search_enhanced_docs.py peripheral GPIO --scope STM32H7
  
  # Find Nucleo BSP GPIO functions (LEDs, buttons)
  python3 scripts/search_enhanced_docs.py function BSP_LED --scope NUCLEO_BSP
  python3 scripts/search_enhanced_docs.py function BSP_PB --scope NUCLEO_BSP
  
  # Find specific GPIO HAL functions
  python3 scripts/search_enhanced_docs.py function HAL_GPIO_Init --scope STM32H7
  
  # Search GPIO configuration concepts
  python3 scripts/search_enhanced_docs.py concept "gpio_configuration" --scope STM32H7
  
  # Find GPIO interrupt implementation
  python3 scripts/search_enhanced_docs.py concept "gpio_interrupt" --scope STM32H7
  ```
- **Cross-Reference Integration**: Links motor control, safety systems, and communication protocols
- **Token-Efficient Lookup**: 7x faster search with semantic categorization for Copilot assistance

## Integration with Existing Instructions
- **Extends**: `hardware-pins.instructions.md` (high-level pin assignments)
- **References**: `ssot-config.instructions.md` (SSOT principles)
- **BSP Integration**: Leverages STM32H7xx Nucleo BSP for board-specific GPIO functions
- **Documentation**: Based on 198 STM32H7 GPIO files + 42 Nucleo BSP files from enhanced search system

## Critical STM32H753ZI GPIO Implementation

### GPIO Initialization Pattern (from STM32H7 HAL Documentation)
```c
#include "config/hardware_config.h"  // SSOT reference

// GPIO Configuration Structure (from structGPIO__InitTypeDef.md)
void gpio_configure_motor_pins(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Enable GPIO clocks FIRST (critical requirement)
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    // Motor 1 CS Pin (Output, Push-Pull)
    GPIO_InitStruct.Pin = MOTOR1_CS_PIN;           // From SSOT
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;    // Push-pull output
    GPIO_InitStruct.Pull = GPIO_NOPULL;            // No pull resistor
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;  // High speed for SPI CS
    HAL_GPIO_Init(MOTOR1_CS_PORT, &GPIO_InitStruct);
    
    // Set CS high initially (inactive)
    HAL_GPIO_WritePin(MOTOR1_CS_PORT, MOTOR1_CS_PIN, GPIO_PIN_SET);
}
```

### SPI GPIO Configuration (for L6470 Interface)
```c
// SPI2 GPIO Configuration (from group__GPIO__Exported__Functions__Group1.md)
void gpio_configure_spi_pins(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // SPI2 GPIO Configuration
    GPIO_InitStruct.Pin = MOTOR_SPI_SCK_PIN | MOTOR_SPI_MISO_PIN | MOTOR_SPI_MOSI_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;        // Alternate function push-pull
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;     // STM32H753ZI specific AF
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
```

### I2C GPIO Configuration (for AS5600 Encoders)
```c
// I2C GPIO Configuration (from group__GPIO__mode__define.md)
void gpio_configure_i2c_pins(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // I2C1 for Encoder 1
    GPIO_InitStruct.Pin = ENCODER1_I2C_SCL_PIN | ENCODER1_I2C_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;        // Open-drain for I2C
    GPIO_InitStruct.Pull = GPIO_PULLUP;            // Internal pull-up
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;     // STM32H753ZI specific AF
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // I2C2 for Encoder 2  
    GPIO_InitStruct.Pin = ENCODER2_I2C_SCL_PIN | ENCODER2_I2C_SDA_PIN;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;     // Different I2C instance
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
```

## STM32H753ZI-Specific GPIO Features

### GPIO Speed Settings (from group__GPIO__speed__define.md)
```c
// GPIO Speed Recommendations for STM32H753ZI
#define GPIO_SPEED_FREQ_LOW         0x00000000U  // <= 8 MHz
#define GPIO_SPEED_FREQ_MEDIUM      0x00000001U  // <= 50 MHz  
#define GPIO_SPEED_FREQ_HIGH        0x00000002U  // <= 100 MHz
#define GPIO_SPEED_FREQ_VERY_HIGH   0x00000003U  // <= 180 MHz

// Use cases:
// - LEDs, slow control signals: GPIO_SPEED_FREQ_LOW
// - SPI CS, moderate signals: GPIO_SPEED_FREQ_HIGH  
// - High-speed SPI/UART: GPIO_SPEED_FREQ_VERY_HIGH
```

### GPIO Pull-up/Pull-down Configuration
```c
// From group__GPIO__pull__define.md
typedef enum {
    GPIO_NOPULL = 0x00000000U,     // No pull resistor
    GPIO_PULLUP = 0x00000001U,     // Pull-up resistor  
    GPIO_PULLDOWN = 0x00000002U    // Pull-down resistor
} GPIO_PullTypeDef;

// Guidelines:
// - SPI MOSI/MISO/SCK: GPIO_NOPULL (driven by peripherals)
// - I2C SDA/SCL: GPIO_PULLUP (required for open-drain)
// - Input buttons: GPIO_PULLUP or GPIO_PULLDOWN
// - Unused pins: GPIO_PULLDOWN (power saving)
```

## Critical STM32H753ZI Considerations

### Power Domain Awareness
```c
// STM32H753ZI has multiple power domains - ensure clocks are enabled
void gpio_system_init(void) {
    // Enable all GPIO clocks used in the project
    __HAL_RCC_GPIOA_CLK_ENABLE();  // CS pins, UART
    __HAL_RCC_GPIOB_CLK_ENABLE();  // SPI, I2C  
    __HAL_RCC_GPIOC_CLK_ENABLE();  // Ethernet
    __HAL_RCC_GPIOD_CLK_ENABLE();  // CAN, additional pins
    __HAL_RCC_GPIOG_CLK_ENABLE();  // Ethernet RMII
}
```

### Alternate Function Mapping (STM32H753ZI-Specific)
```c
// Critical: Use correct AF numbers for STM32H753ZI (from datasheet)
#define GPIO_AF4_I2C1          4    // I2C1 alternate function
#define GPIO_AF4_I2C2          4    // I2C2 alternate function  
#define GPIO_AF5_SPI2          5    // SPI2 alternate function
#define GPIO_AF7_USART3        7    // USART3 alternate function
#define GPIO_AF9_FDCAN1        9    // FDCAN1 alternate function
#define GPIO_AF11_ETH          11   // Ethernet alternate function
```

## Error Prevention Guidelines

### Common GPIO Configuration Errors
```c
// ❌ WRONG: Clock not enabled before GPIO configuration
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);  // Will fail silently

// ✅ CORRECT: Enable clock first
__HAL_RCC_GPIOA_CLK_ENABLE();
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

// ❌ WRONG: Incorrect alternate function number
GPIO_InitStruct.Alternate = GPIO_AF6_SPI2;  // Wrong AF for STM32H753ZI

// ✅ CORRECT: Use STM32H753ZI datasheet AF mapping  
GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;  // Correct AF
```

### Validation Functions
```c
#include "config/safety_config.h"

SystemError_t gpio_validate_configuration(void) {
    // Verify critical pins are configured correctly
    if (HAL_GPIO_ReadPin(MOTOR1_CS_PORT, MOTOR1_CS_PIN) != GPIO_PIN_SET) {
        return ERROR_GPIO_CS_INVALID_STATE;
    }
    
    // Verify GPIO clocks are enabled
    if (!(RCC->AHB4ENR & RCC_AHB4ENR_GPIOAEN)) {
        return ERROR_GPIO_CLOCK_NOT_ENABLED;
    }
    
    return SYSTEM_OK;
}
```

## Integration with Safety Systems

### Watchdog-Safe GPIO Operations
```c
#include "config/safety_config.h"

void gpio_safe_write_with_watchdog(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state) {
    // Feed watchdog before critical GPIO operation
    HAL_IWDG_Refresh(&hiwdg);
    
    HAL_GPIO_WritePin(port, pin, state);
    
    // Verify write operation
    if (HAL_GPIO_ReadPin(port, pin) != state) {
        system_trigger_safe_stop();  // From safety_config.h
    }
}
```

## Documentation References
Based on STM32H753ZI documentation analysis (198 GPIO-related files):

**Key Reference Files:**
- `group__GPIO__Exported__Functions__Group1.md` (8.7KB) - Primary GPIO functions
- `structGPIO__InitTypeDef.md` (6.3KB) - GPIO initialization structure
- `group__GPIO__mode__define.md` (8.5KB) - GPIO mode definitions
- `group__GPIO__pins__define.md` (7.9KB) - GPIO pin definitions
- `group__GPIO__speed__define.md` (2.0KB) - GPIO speed settings
- `group__GPIO__pull__define.md` (1.6KB) - Pull resistor configuration

**Total Documentation Coverage**: 26KB of focused GPIO implementation guidance from STM32H753ZI-specific HAL documentation.
