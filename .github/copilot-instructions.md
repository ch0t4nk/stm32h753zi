<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

# STM32H753ZI Stepper Motor Control Project - Copilot Instructions

This workspace develops firmware for an **STM32H753ZI Nucleo-144** board controlling stepper motors via **X-NUCLEO-IHM02A1** shield with closed-loop feedback using **AS5600 magnetic encoders**.

## Project Architecture

### Hardware Components
- **STM32H753ZI MCU**: Main controller (ARM Cortex-M7, 480MHz)
- **X-NUCLEO-IHM02A1**: Dual stepper driver shield (2x L6470 dSPIN ICs)
- **AS5600 Encoders (2x)**: 12-bit magnetic rotary encoders for feedback
- **Communication**: UART (USB VCP), CAN-FD, Ethernet (LAN8742 PHY)

### Design Principles
Follow **SOLID principles** and **Single Source of Truth (SSOT)** strictly:
- **SRP**: Separate classes for motor control, drivers, encoders, communication
- **OCP**: Use abstract interfaces for extensibility
- **LSP**: Ensure all implementations are substitutable
- **ISP**: Keep interfaces focused and specific
- **DIP**: Depend on abstractions, not concrete implementations
- **SSOT**: All configuration, state, and constants centralized in designated files

### Single Source of Truth Requirements
**CRITICAL**: All configuration must come from centralized sources:

#### Configuration Headers (SSOT)
```c
// TODO: See .instructions/ssot-structure.md for complete organization
src/config/
├── hardware_config.h      // Pin definitions, peripheral assignments
├── comm_config.h          // Protocol parameters, addresses, timeouts
├── motor_config.h         // L6470 parameters, motion limits
├── safety_config.h        // Watchdog, fault thresholds
└── build_config.h         // Version info, build metadata
```

#### State Management (SSOT)
```c
// TODO: See .instructions/system-state.md for centralized state design
// Single authoritative system state - never duplicate!
typedef struct {
    MotorState_t motors[2];
    EncoderState_t encoders[2];
    CommState_t communication;
    SafetyState_t safety;
    uint32_t system_uptime_ms;
} SystemState_t;

extern SystemState_t g_system_state;  // Global SSOT instance
```

#### Error Handling (SSOT)
```c
// TODO: See .instructions/error-codes.md for centralized error management
typedef enum {
    SYSTEM_OK = 0x0000,
    ERROR_MOTOR_BASE = 0x1000,
    ERROR_ENCODER_BASE = 0x2000,
    ERROR_COMM_BASE = 0x3000,
    ERROR_SAFETY_BASE = 0x4000
} SystemError_t;
```

### Safety Requirements
**CRITICAL**: Always implement fail-safe behavior:
- Monitor L6470 fault flags via SPI GetStatus commands
- Implement watchdog timer for software fault recovery
- Use controlled ramping (acceleration/deceleration profiles)
- Validate encoder readings for plausibility
- Stop motors on any communication timeout or fault

## Reference Assets Available
The `00_reference/` directory contains ST official assets (READ-ONLY):
- STM32H7 HAL drivers and examples
- X-NUCLEO-IHM02A1 board support package
- L6470 driver libraries and documentation
- Application examples and datasheets

**IMPORTANT**: Never modify files in `00_reference/`. Copy needed code to appropriate `src/` locations.

## Code Generation Guidelines

### 1. SSOT File Structure & Naming
```
src/
├── config/                  // SSOT configuration headers
│   ├── hardware_config.h    // Pin assignments, peripherals
│   ├── comm_config.h        // Communication parameters
│   ├── motor_config.h       // Motor/L6470 settings
│   ├── safety_config.h      // Safety thresholds
│   └── build_config.h       // Version, build info
├── common/                  // SSOT common definitions
│   ├── error_codes.h        // System error definitions
│   ├── system_state.h       // Central state structure
│   └── data_types.h         // Common type definitions
├── drivers/
│   ├── l6470_driver.h/c     // L6470 stepper driver SPI interface
│   ├── as5600_encoder.h/c   // AS5600 I2C encoder interface
│   └── interfaces/
│       ├── motor_driver_interface.h
│       └── encoder_interface.h
├── controllers/
│   └── stepper_motor_controller.h/c
├── communication/
│   ├── uart_handler.h/c     // USART3 for VCP
│   ├── can_handler.h/c      // FDCAN1 interface
│   └── ethernet_handler.h/c // LwIP TCP/IP
└── safety/
    └── watchdog_manager.h/c
```

### 2. SSOT Code Generation Rules

**Always reference SSOT - never hardcode values:**
```c
// CORRECT - Use SSOT configuration
#include "config/hardware_config.h"
HAL_GPIO_WritePin(MOTOR1_CS_PORT, MOTOR1_CS_PIN, GPIO_PIN_SET);

// INCORRECT - Hardcoded values
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
```

**Always validate SSOT data access:**
```c
// TODO: See .instructions/ssot-validation.md for data access patterns
SystemError_t system_state_set_motor_angle(uint8_t motor_id, float angle) {
    if (motor_id >= MAX_MOTORS) return ERROR_INVALID_PARAMETER;
    if (angle < MOTOR_MIN_ANGLE || angle > MOTOR_MAX_ANGLE) return ERROR_OUT_OF_RANGE;
    
    g_system_state.motors[motor_id].target_angle = angle;
    return SYSTEM_OK;
}
```

### 3. Hardware Configuration (SSOT)
All pin assignments in `src/config/hardware_config.h`:

```c
// TODO: See .instructions/hardware-config.md for complete pin definitions
// SPI Configuration (SSOT)
#define MOTOR_SPI_INSTANCE      SPI2
#define MOTOR_SPI_SCK_PIN       GPIO_PIN_13
#define MOTOR_SPI_SCK_PORT      GPIOB
#define MOTOR_SPI_MISO_PIN      GPIO_PIN_14
#define MOTOR_SPI_MISO_PORT     GPIOB
#define MOTOR_SPI_MOSI_PIN      GPIO_PIN_15
#define MOTOR_SPI_MOSI_PORT     GPIOB
#define MOTOR_SPI_CS_PIN        GPIO_PIN_9
#define MOTOR_SPI_CS_PORT       GPIOA

// I2C Configuration (SSOT)
#define ENCODER1_I2C_INSTANCE   I2C1
#define ENCODER1_I2C_SCL_PIN    GPIO_PIN_6
#define ENCODER1_I2C_SCL_PORT   GPIOB
#define ENCODER1_I2C_SDA_PIN    GPIO_PIN_7
#define ENCODER1_I2C_SDA_PORT   GPIOB
#define ENCODER1_I2C_ADDRESS    0x36

#define ENCODER2_I2C_INSTANCE   I2C2
#define ENCODER2_I2C_SCL_PIN    GPIO_PIN_10
#define ENCODER2_I2C_SCL_PORT   GPIOB
#define ENCODER2_I2C_SDA_PIN    GPIO_PIN_11
#define ENCODER2_I2C_SDA_PORT   GPIOB
#define ENCODER2_I2C_ADDRESS    0x36
```

### 4. Communication Configuration (SSOT)
All protocol parameters in `src/config/comm_config.h`:

```c
// TODO: See .instructions/comm-config.md for protocol definitions
// UART Configuration
#define UART_INSTANCE           USART3
#define UART_BAUDRATE           115200
#define UART_TX_PIN             GPIO_PIN_8
#define UART_TX_PORT            GPIOD
#define UART_RX_PIN             GPIO_PIN_9
#define UART_RX_PORT            GPIOD

// CAN Configuration
#define CAN_INSTANCE            FDCAN1
#define CAN_BITRATE_KBPS        500
#define CAN_TX_PIN              GPIO_PIN_1
#define CAN_TX_PORT             GPIOD
#define CAN_RX_PIN              GPIO_PIN_0
#define CAN_RX_PORT             GPIOD

// CAN Message IDs (SSOT)
#define CAN_ID_MOTOR1_STATUS    0x101
#define CAN_ID_MOTOR2_STATUS    0x102
#define CAN_ID_COMMAND          0x201
#define CAN_ID_HEARTBEAT        0x001

// Ethernet Configuration
#define ETH_STATIC_IP           "192.168.1.50"
#define ETH_NETMASK             "255.255.255.0"
#define ETH_GATEWAY             "192.168.1.1"
#define TCP_SERVER_PORT         8000
```

## When generating code:
1. **Always reference SSOT files** for all constants and configurations
2. **Create validation functions** for accessing shared state
3. **Use TODO comments** referencing appropriate instruction modules
4. **Document the SSOT source** for each configuration parameter
5. **Implement thread-safe access** to shared state if using RTOS
6. **Generate build-time validation** to catch SSOT inconsistencies

## Environment Context
This workspace runs in a dev container with:
- ARM GCC toolchain, OpenOCD, Doxygen with Graphviz
- Unity/GoogleTest frameworks
- Python tools: CAN, Sphinx, Breathe for docs
- SSOT validation tools: jq, pre-commit hooks
- All development tools for embedded C/C++

## Reference Material Usage
When referencing `00_reference/` assets:
- **Copy** needed header files to `src/` with proper attribution
- **Extract** relevant code snippets with modifications for project needs
- **Document** the source of copied/adapted code in SSOT headers
- **Never** directly include from `00_reference/` in build system

Remember: **Safety first, fail-safe always, SOLID architecture throughout, SSOT for all configuration.**
