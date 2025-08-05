---
applyTo: "src/**"
description: "STM32H753ZI hardware pin assignments and peripheral configuration guidance"
---

# STM32H753ZI Hardware Configuration Instructions

## Overview
This instruction file provides comprehensive guidance for configuring the STM32H753ZI Nucleo-144 board hardware interfaces for the stepper motor control project.

## Enhanced Documentation Integration
**Reference Assets**: This instruction leverages the comprehensive STM32H7 documentation system:
- **STM32H7 HAL Documentation**: 86MB, 3,997 files with unified peripheral coverage
- **STM32H7xx Nucleo BSP Documentation**: 824KB, 42 files with board support package functions
- **Copilot-Optimized Search**: Use enhanced search with semantic categories
  ```bash
  # Search for GPIO peripheral documentation
  python3 scripts/search_enhanced_docs.py peripheral GPIO --scope STM32H7
  
  # Search Nucleo BSP board functions (LEDs, buttons, COM ports)
  python3 scripts/search_enhanced_docs.py function BSP_LED --scope NUCLEO_BSP
  python3 scripts/search_enhanced_docs.py function BSP_PB --scope NUCLEO_BSP
  
  # Search for hardware configuration concepts
  python3 scripts/search_enhanced_docs.py concept "pin assignment" --scope STM32H7
  
  # Find specific HAL functions
  python3 scripts/search_enhanced_docs.py function HAL_GPIO_Init --scope STM32H7
  ```
- **Unified Index**: 32,200+ keywords across STM32H7 + Nucleo BSP documentation for instant guidance
- **Reference Files**: Complete coverage from root-level docs (RM0433, OM0253) to HAL implementation details

## STM32H753ZI Specific Guidelines

### MCU Characteristics (from ST BSP)
- **CPU**: ARM Cortex-M7 @ 480MHz
- **Flash**: 2MB
- **RAM**: 1MB total (multiple domains)
- **Package**: LQFP144
- **Voltage**: 1.62V to 3.6V VDD

### Pin Assignment Strategy

#### SPI Configuration (Motor Drivers)
Based on STM32H7xx Nucleo BSP and X-NUCLEO-IHM02A1 shield:
**Documentation References**: 
- Use `python3 scripts/search_enhanced_docs.py peripheral SPI --scope STM32H7` for SPI implementation details
- Search concepts: `python3 scripts/search_enhanced_docs.py concept "SPI configuration" --scope STM32H7`
- Find HAL functions: `python3 scripts/search_enhanced_docs.py function HAL_SPI_Init --scope STM32H7`
```c
// L6470 Daisy-Chain SPI (SSOT Reference)
#define MOTOR_SPI_INSTANCE      SPI2          // Arduino D13/D12/D11 compatibility
#define MOTOR_SPI_SCK_PIN       GPIO_PIN_13   // PB13 - Arduino D13
#define MOTOR_SPI_MISO_PIN      GPIO_PIN_14   // PB14 - Arduino D12  
#define MOTOR_SPI_MOSI_PIN      GPIO_PIN_15   // PB15 - Arduino D11
#define MOTOR_SPI_CS_PIN        GPIO_PIN_9    // PA9  - Arduino D8
```

**Critical Requirements:**
- Use SPI Mode 3 (CPOL=1, CPHA=1) for L6470 compatibility
- Maximum SPI clock: 5MHz (L6470 datasheet specification)
- CS must be shared for daisy-chain operation (both L6470s)
- BUSY/FLAG pins are separate for each motor

#### I2C Configuration (Encoders)
AS5600 magnetic encoders require separate I2C buses due to fixed address (0x36):
**Documentation References**: 
- Use `python3 scripts/search_enhanced_docs.py peripheral I2C --scope STM32H7` for I2C implementation details
- Search concepts: `python3 scripts/search_enhanced_docs.py concept "I2C dual bus" --scope STM32H7`
- Find HAL functions: `python3 scripts/search_enhanced_docs.py function HAL_I2C_Init --scope STM32H7`
```c
// AS5600 Encoder 1 (Motor 1)
#define ENCODER1_I2C_INSTANCE   I2C1
#define ENCODER1_I2C_SCL_PIN    GPIO_PIN_6    // PB6 - Morpho CN7-58
#define ENCODER1_I2C_SDA_PIN    GPIO_PIN_7    // PB7 - Morpho CN7-59

// AS5600 Encoder 2 (Motor 2)  
#define ENCODER2_I2C_INSTANCE   I2C2
#define ENCODER2_I2C_SCL_PIN    GPIO_PIN_10   // PB10 - Morpho CN7-51
#define ENCODER2_I2C_SDA_PIN    GPIO_PIN_11   // PB11 - Morpho CN7-49
```

**Critical Requirements:**
- I2C Speed: Standard (100kHz) or Fast (400kHz) mode
- Pull-up resistors: 4.7kΩ to 3.3V (check if present on AS5600 breakout)
- Both encoders use same address (0x36), hence separate buses required

#### UART Configuration (Virtual COM Port)
STM32H753ZI Nucleo has USART3 connected to ST-Link for VCP:
**Documentation References**: 
- Use `python3 scripts/search_enhanced_docs.py peripheral UART --scope STM32H7` for UART implementation details
- Search concepts: `python3 scripts/search_enhanced_docs.py concept "virtual COM port" --scope STM32H7`
- Find HAL functions: `python3 scripts/search_enhanced_docs.py function HAL_UART_Init --scope STM32H7`
```c
// Virtual COM Port via ST-Link
#define UART_INSTANCE           USART3
#define UART_TX_PIN             GPIO_PIN_8    // PD8 - ST-Link VCP TX
#define UART_RX_PIN             GPIO_PIN_9    // PD9 - ST-Link VCP RX
```

#### Nucleo BSP Integration (Board Support Package)
**STM32H7xx Nucleo BSP Documentation**: Use comprehensive BSP function documentation for board-specific features:
**Documentation References**:
- Board functions: `python3 scripts/search_enhanced_docs.py function BSP_LED --scope NUCLEO_BSP`
- Button handling: `python3 scripts/search_enhanced_docs.py function BSP_PB --scope NUCLEO_BSP`
- COM port setup: `python3 scripts/search_enhanced_docs.py function BSP_COM --scope NUCLEO_BSP`
- Complete BSP reference: `00_reference/stm32h7xx_nucleo_bsp_md/`

```c
// Nucleo BSP LED Functions (from BSP documentation)
#define LED_GREEN              LED1           // LD1 - Green LED
#define LED_BLUE               LED2           // LD2 - Blue LED  
#define LED_RED                LED3           // LD3 - Red LED

// Nucleo BSP Button Functions
#define USER_BUTTON            BUTTON_USER    // Blue user button (PC13)

// BSP Function Integration Examples:
void nucleo_bsp_init(void) {
    // Initialize LEDs using BSP functions
    BSP_LED_Init(LED_GREEN);
    BSP_LED_Init(LED_BLUE);
    BSP_LED_Init(LED_RED);
    
    // Initialize user button
    BSP_PB_Init(USER_BUTTON, BUTTON_MODE_GPIO);
}
```

#### CAN Configuration (MCU-to-MCU Communication)
STM32H753ZI has FDCAN peripheral, requires external transceiver:
```c
// CAN-FD Interface (requires external transceiver)
#define CAN_INSTANCE            FDCAN1
#define CAN_TX_PIN              GPIO_PIN_1    // PD1 - Morpho CN7-44
#define CAN_RX_PIN              GPIO_PIN_0    // PD0 - Morpho CN7-46
```

**External Hardware Required:**
- CAN transceiver: MCP2551 (5V) or SN65HVD230 (3.3V)
- 120Ω termination resistors at bus ends
- Twisted pair cable for CAN_H/CAN_L

#### Ethernet Configuration (Built-in PHY)
STM32H753ZI Nucleo has built-in LAN8742 PHY:
```c
// Ethernet RMII Interface (built-in LAN8742)
#define ETH_RMII_TX_EN_PIN      GPIO_PIN_11   // PG11
#define ETH_RMII_TXD0_PIN       GPIO_PIN_13   // PG13  
#define ETH_RMII_TXD1_PIN       GPIO_PIN_14   // PG14
#define ETH_RMII_RX_ER_PIN      GPIO_PIN_2    // PI2
#define ETH_RMII_RXD0_PIN       GPIO_PIN_4    // PC4
#define ETH_RMII_RXD1_PIN       GPIO_PIN_5    // PC5
#define ETH_RMII_CRS_DV_PIN     GPIO_PIN_7    // PA7
```

### GPIO Configuration Guidelines

#### Output Pins (Motors, LEDs, Control)
```c
GPIO_InitTypeDef GPIO_InitStruct = {0};
GPIO_InitStruct.Pin = MOTOR1_CS_PIN;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;  // For SPI CS timing
HAL_GPIO_Init(MOTOR1_CS_PORT, &GPIO_InitStruct);
```

#### Input Pins (Buttons, Fault Signals)
```c
GPIO_InitStruct.Pin = MOTOR1_FAULT_PIN;
GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
GPIO_InitStruct.Pull = GPIO_PULLUP;  // L6470 FAULT is open-drain
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(MOTOR1_FAULT_PORT, &GPIO_InitStruct);
```

## Clock Configuration Requirements

### System Clock Setup
For optimal performance with 480MHz operation:
```c
// HSE = 25MHz (Nucleo-144 crystal)
// PLL1: 25MHz * 96 / 2 / 1 = 480MHz (max for STM32H753ZI)
// AHB = 480MHz / 2 = 240MHz
// APB1 = 240MHz / 2 = 120MHz  
// APB2 = 240MHz / 2 = 120MHz
```

### Peripheral Clock Requirements
- **SPI2**: APBx clock, configure for 5MHz max (L6470 limit)
- **I2C1/I2C2**: APB1 clock, configure for 400kHz (Fast mode)
- **USART3**: APB1 clock, 115200 baud standard
- **FDCAN1**: APB1 clock, supports up to 1Mbps CAN
- **Ethernet**: PLL provides 50MHz RMII reference

## Memory Configuration

### Memory Domains for DMA
STM32H753ZI has multiple memory domains. For DMA operations:
- **DTCM RAM**: Fast access, no cache issues
- **AXI SRAM**: Main application memory
- **SRAM1/2/3**: DMA accessible, consider cache coherency

### Ethernet Buffer Placement
```c
// Place in D2 domain for Ethernet DMA access
__attribute__((section(".EthBuffers"))) 
uint8_t eth_rx_buffer[ETH_RX_BUF_SIZE];
```

## Power Management Considerations

### Power Domains
- VDD: 3.3V main supply
- VDDA: 3.3V analog supply (critical for ADC if used)
- VDD33USB: 3.3V USB supply (for ST-Link VCP)

### Low Power Modes
If implementing power saving:
- Stop mode maintains SRAM
- Standby mode requires configuration save/restore
- RTC can wake from standby

## Safety and Fault Handling

### Critical Pin Monitoring
Monitor these pins for system safety:
```c
// L6470 fault pins (active low, open-drain)
#define MOTOR1_FAULT_PIN        GPIO_PIN_10   // PA10 - Arduino D2
#define MOTOR2_FAULT_PIN        GPIO_PIN_2    // PA2  - Arduino A1

// AS5600 magnetic field detection (optional)
#define ENCODER1_MAG_PIN        GPIO_PIN_3    // PA3  - Arduino A0
#define ENCODER2_MAG_PIN        GPIO_PIN_1    // PA1  - Arduino A4
```

### Watchdog Configuration
```c
// Independent Watchdog for safety
#define IWDG_TIMEOUT_MS         1000          // 1 second max
#define IWDG_PRESCALER          IWDG_PRESCALER_32
```

## Pin Conflict Resolution

### Arduino Shield Compatibility
The X-NUCLEO-IHM02A1 shield uses Arduino UNO R3 pinout:
- D13/D12/D11/D10: SPI interface
- D8: SPI CS (shared for daisy-chain)
- D4: Motor 1 FLAG/BUSY
- D7: Motor 2 FLAG/BUSY
- D2: Motor 1 SW (optional switch input)
- D6: Motor 2 SW (optional switch input)

### Morpho Connector Usage
Use morpho connectors for additional I2C buses and CAN:
- CN7/CN10: Extended pin access
- CN11/CN12: Power and additional GPIOs

## Initialization Order Requirements

### Proper Startup Sequence
```c
1. System Clock Configuration
2. GPIO Clock Enable (all used ports)
3. GPIO Configuration (CS pins HIGH before SPI init)
4. SPI Peripheral Configuration
5. I2C Peripheral Configuration  
6. UART Peripheral Configuration
7. CAN Peripheral Configuration
8. Ethernet MAC/PHY Configuration
9. DMA Configuration (if used)
10. NVIC/Interrupt Configuration
```

### Critical Timing Requirements
- CS pins must be HIGH before L6470 SPI initialization
- Allow 1ms minimum after L6470 power-up before first SPI command
- AS5600 requires 5ms startup time after power-on
- CAN transceiver may need enable signal setup time

Remember to always validate hardware configuration against the STM32H753ZI datasheet and board schematic before implementation.
