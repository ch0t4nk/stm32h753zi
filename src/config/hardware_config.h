/**
 * @file hardware_config.h
 * @brief Hardware Configuration - Single Source of Truth (SSOT)
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * @note This file contains ALL hardware pin assignments and peripheral
 * configurations. Never hardcode these values elsewhere - always reference
 * this SSOT.
 *
 * TODO: See .github/instructions/hardware-pins.instructions.md for complete
 * pin mapping documentation
 * TODO: See .github/instructions/hardware-pins.instructions.md for
 * MCU-specific configuration details
 */

#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#ifdef FIRMWARE_BUILD
// Firmware build: Include STM32 HAL headers
#include "stm32h7xx_hal.h"
#else
// Host test build: Include HAL abstraction headers for enums
#include "hal_abstraction/hal_abstraction.h"
#endif

/* ==========================================================================
 */
/* MCU Configuration (SSOT)                                                  */
/* ==========================================================================
 */
#define MCU_FAMILY "STM32H7"
#define MCU_PART_NUMBER "STM32H753ZI"
#define MCU_CORE_FREQUENCY_HZ 480000000UL
#define MCU_PACKAGE "LQFP144"

/* STM32H7 Clock Configuration (SSOT) - Validated against official docs */
#define SYSTEM_CLOCK_HSE_HZ                                                   \
    25000000UL                  // External crystal on Nucleo-144 (validated)
#define SYSTEM_CLOCK_PLL_M 5    // HSE prescaler: 25MHz/5 = 5MHz
#define SYSTEM_CLOCK_PLL_N 192  // PLL multiplier: 5MHz*192 = 960MHz
#define SYSTEM_CLOCK_PLL_P 2    // SYSCLK divider: 960MHz/2 = 480MHz
#define SYSTEM_CLOCK_AHB_DIV 2  // AHB prescaler: 480MHz/2 = 240MHz
#define SYSTEM_CLOCK_APB1_DIV 2 // APB1 prescaler: 240MHz/2 = 120MHz
#define SYSTEM_CLOCK_APB2_DIV 2 // APB2 prescaler: 240MHz/2 = 120MHz
#define SYSTEM_CLOCK_APB3_DIV 2 // APB3 prescaler: 240MHz/2 = 120MHz
#define SYSTEM_CLOCK_APB4_DIV 2 // APB4 prescaler: 240MHz/2 = 120MHz

/* X-CUBE-SPN2 + MCSDK Integration Framework (SSOT) */
#define MOTOR_CONTROL_FRAMEWORK_SPN2                                          \
    1 // Primary: X-CUBE-SPN2 stepper framework
#define MOTOR_CONTROL_FRAMEWORK_MCSDK                                         \
    1                               // Secondary: Selective MCSDK enhancement
#define MOTOR_CONTROL_HYBRID_MODE 1 // Enable hybrid SPN2+MCSDK integration

/* ==========================================================================
 */
/* SPI Configuration for L6470 Stepper Drivers (SSOT)                       */
/* ==========================================================================
 */
// TODO: See .github/instructions/stm32h7-spi-l6470.instructions.md for
// daisy-chain protocol implementation
#define MOTOR_SPI_INSTANCE SPI2
#define MOTOR_SPI_SCK_PIN GPIO_PIN_13
#define MOTOR_SPI_SCK_PORT GPIOB
#define MOTOR_SPI_SCK_AF GPIO_AF5_SPI2
#define MOTOR_SPI_MISO_PIN GPIO_PIN_14
#define MOTOR_SPI_MISO_PORT GPIOB
#define MOTOR_SPI_MISO_AF GPIO_AF5_SPI2
#define MOTOR_SPI_MOSI_PIN GPIO_PIN_15
#define MOTOR_SPI_MOSI_PORT GPIOB
#define MOTOR_SPI_MOSI_AF GPIO_AF5_SPI2

// Chip Select (CS) for daisy-chained L6470 drivers
#define MOTOR_SPI_CS_PIN GPIO_PIN_9
#define MOTOR_SPI_CS_PORT GPIOA

// L6470 Control Signals
#define MOTOR_FLAG_PIN GPIO_PIN_10 // Fault indication from drivers
#define MOTOR_FLAG_PORT GPIOA
#define MOTOR_BUSY_PIN GPIO_PIN_11 // Busy indication from drivers
#define MOTOR_BUSY_PORT GPIOA

/* ==========================================================================
 */
/* I2C Configuration for AS5600 Magnetic Encoders (SSOT)                    */
/* ==========================================================================
 */
// TODO: See .github/instructions/stm32h7-i2c-as5600.instructions.md for
// dual-bus I2C configuration

// Primary I2C bus for encoder operations
#define ENCODER_I2C_INSTANCE I2C1

// Encoder 1: Motor 1 Position Feedback
#define ENCODER1_I2C_INSTANCE I2C1
#define ENCODER1_I2C_SCL_PIN GPIO_PIN_6
#define ENCODER1_I2C_SCL_PORT GPIOB
#define ENCODER1_I2C_SCL_AF GPIO_AF4_I2C1
#define ENCODER1_I2C_SDA_PIN GPIO_PIN_7
#define ENCODER1_I2C_SDA_PORT GPIOB
#define ENCODER1_I2C_SDA_AF GPIO_AF4_I2C1
#define ENCODER1_I2C_ADDRESS 0x36

// Encoder 2: Motor 2 Position Feedback
#define ENCODER2_I2C_INSTANCE I2C2
#define ENCODER2_I2C_SCL_PIN GPIO_PIN_10
#define ENCODER2_I2C_SCL_PORT GPIOB
#define ENCODER2_I2C_SCL_AF GPIO_AF4_I2C2
#define ENCODER2_I2C_SDA_PIN GPIO_PIN_11
#define ENCODER2_I2C_SDA_PORT GPIOB
#define ENCODER2_I2C_SDA_AF GPIO_AF4_I2C2
#define ENCODER2_I2C_ADDRESS 0x36

/* ==========================================================================
 */
/* UART Configuration for Virtual COM Port (SSOT)                           */
/* ==========================================================================
 */
// TODO: See .github/instructions/stm32h7-uart-protocol.instructions.md for
// command protocol specification
#define UART_INSTANCE USART3
#define UART_TX_PIN GPIO_PIN_8
#define UART_TX_PORT GPIOD
#define UART_TX_AF GPIO_AF7_USART3
#define UART_RX_PIN GPIO_PIN_9
#define UART_RX_PORT GPIOD
#define UART_RX_AF GPIO_AF7_USART3

/* ==========================================================================
 */
/* CAN-FD Configuration for MCU-to-MCU Communication (SSOT)                 */
/* ==========================================================================
 */
// TODO: See .github/instructions/comm-protocols.instructions.md for message ID
// allocation and protocol
#define CAN_INSTANCE FDCAN1
#define CAN_TX_PIN GPIO_PIN_1
#define CAN_TX_PORT GPIOD
#define CAN_TX_AF GPIO_AF9_FDCAN1
#define CAN_RX_PIN GPIO_PIN_0
#define CAN_RX_PORT GPIOD
#define CAN_RX_AF GPIO_AF9_FDCAN1

/* ==========================================================================
 */
/* Ethernet Configuration for Network Connectivity (SSOT)                   */
/* ==========================================================================
 */
// TODO: See .github/instructions/comm-protocols.instructions.md for LwIP stack
// configuration Note: STM32H753ZI Nucleo has on-board LAN8742 PHY connected
// via RMII

// RMII Interface Pins
#define ETH_RMII_REF_CLK_PIN GPIO_PIN_1
#define ETH_RMII_REF_CLK_PORT GPIOA
#define ETH_RMII_REF_CLK_AF GPIO_AF11_ETH
#define ETH_RMII_MDIO_PIN GPIO_PIN_2
#define ETH_RMII_MDIO_PORT GPIOA
#define ETH_RMII_MDIO_AF GPIO_AF11_ETH
#define ETH_RMII_MDC_PIN GPIO_PIN_1
#define ETH_RMII_MDC_PORT GPIOC
#define ETH_RMII_MDC_AF GPIO_AF11_ETH

#define ETH_RMII_CRS_DV_PIN GPIO_PIN_7
#define ETH_RMII_CRS_DV_PORT GPIOA
#define ETH_RMII_CRS_DV_AF GPIO_AF11_ETH
#define ETH_RMII_RXD0_PIN GPIO_PIN_4
#define ETH_RMII_RXD0_PORT GPIOC
#define ETH_RMII_RXD0_AF GPIO_AF11_ETH
#define ETH_RMII_RXD1_PIN GPIO_PIN_5
#define ETH_RMII_RXD1_PORT GPIOC
#define ETH_RMII_RXD1_AF GPIO_AF11_ETH

#define ETH_RMII_TX_EN_PIN GPIO_PIN_11
#define ETH_RMII_TX_EN_PORT GPIOG
#define ETH_RMII_TX_EN_AF GPIO_AF11_ETH
#define ETH_RMII_TXD0_PIN GPIO_PIN_13
#define ETH_RMII_TXD0_PORT GPIOG
#define ETH_RMII_TXD0_AF GPIO_AF11_ETH
#define ETH_RMII_TXD1_PIN GPIO_PIN_12
#define ETH_RMII_TXD1_PORT GPIOB
#define ETH_RMII_TXD1_AF GPIO_AF11_ETH

// PHY Configuration
#define ETH_PHY_ADDRESS 0x00         // LAN8742 PHY address
#define ETH_PHY_RESET_PIN GPIO_PIN_3 // Optional reset pin
#define ETH_PHY_RESET_PORT GPIOG

/* ==========================================================================
 */
/* GPIO Configuration for Status LEDs and User Interface (SSOT)             */
/* ==========================================================================
 */
// TODO: See .instructions/user-interface.md for LED indication patterns

/* ==========================================================================
 */
/* GPIO Pin Configuration (SSOT) - Pin Number Based Architecture             */
/* ==========================================================================
 */

// STM32H753ZI Nucleo-144 Board Pin Assignments
// See UM1974 STM32 Nucleo-144 boards user manual for reference

/* ===== SSOT PIN NUMBER DEFINITIONS =====
 * Define only the pin NUMBER (0-15) and PORT enum - everything else computed
 * Pin remapping requires changes ONLY here - no duplicated definitions!
 */

// Status LEDs (Nucleo-144 onboard LEDs) - SSOT Base Definitions
#define LED_GREEN_PIN_NUM 0 // LD1 - System status (PB0)
#define LED_GREEN_PORT_ENUM HAL_GPIO_PORT_B
#define LED_YELLOW_PIN_NUM 1 // LD2 - CAN activity (PE1)
#define LED_YELLOW_PORT_ENUM HAL_GPIO_PORT_E
#define LED_RED_PIN_NUM 14 // LD3 - Error/fault (PB14)
#define LED_RED_PORT_ENUM HAL_GPIO_PORT_B

// User Button - SSOT Base Definition
#define USER_BUTTON_PIN_NUM 13 // B1 - Emergency stop (PC13)
#define USER_BUTTON_PORT_ENUM HAL_GPIO_PORT_C

// Safety Relays - SSOT Base Definitions
#define SAFETY_RELAY1_PIN_NUM 4 // Safety relay 1 control (PC4)
#define SAFETY_RELAY1_PORT_ENUM HAL_GPIO_PORT_C
#define SAFETY_RELAY2_PIN_NUM 5 // Safety relay 2 control (PC5)
#define SAFETY_RELAY2_PORT_ENUM HAL_GPIO_PORT_C

/* ===== COMPUTED DEFINITIONS - Auto-generated from SSOT =====
 * These are computed from the base pin numbers above
 * NEVER modify these directly - change the _PIN_NUM definitions above!
 */

// Macro to convert pin number to bit mask
#define PIN_TO_BITMASK(pin_num) (1U << (pin_num))

// Status LED computed definitions
#ifdef FIRMWARE_BUILD
// STM32 HAL constants for firmware
#define LED_GREEN_PIN (PIN_TO_BITMASK(LED_GREEN_PIN_NUM))   // GPIO_PIN_0
#define LED_GREEN_PORT GPIOB                                // STM32 HAL port
#define LED_YELLOW_PIN (PIN_TO_BITMASK(LED_YELLOW_PIN_NUM)) // GPIO_PIN_1
#define LED_YELLOW_PORT GPIOE                               // STM32 HAL port
#define LED_RED_PIN (PIN_TO_BITMASK(LED_RED_PIN_NUM))       // GPIO_PIN_14
#define LED_RED_PORT GPIOB                                  // STM32 HAL port
#else
// HAL abstraction for host tests
#define LED_GREEN_PIN (PIN_TO_BITMASK(LED_GREEN_PIN_NUM))   // Bit mask
#define LED_GREEN_PORT LED_GREEN_PORT_ENUM                  // HAL enum
#define LED_YELLOW_PIN (PIN_TO_BITMASK(LED_YELLOW_PIN_NUM)) // Bit mask
#define LED_YELLOW_PORT LED_YELLOW_PORT_ENUM                // HAL enum
#define LED_RED_PIN (PIN_TO_BITMASK(LED_RED_PIN_NUM))       // Bit mask
#define LED_RED_PORT LED_RED_PORT_ENUM                      // HAL enum
#endif

// Pin indices for array access (always the pin number!)
#define LED_GREEN_PIN_INDEX LED_GREEN_PIN_NUM   // 0
#define LED_YELLOW_PIN_INDEX LED_YELLOW_PIN_NUM // 1
#define LED_RED_PIN_INDEX LED_RED_PIN_NUM       // 14

// User button computed definitions
// NOTE: emergency_stop_abstracted.c uses HAL abstraction, so it needs HAL enum
// values NOTE: emergency_stop.c uses direct STM32 HAL, so it would need STM32
// HAL pointers For now, providing HAL abstraction enums for the abstracted
// modules
#define USER_BUTTON_PIN                                                       \
    (PIN_TO_BITMASK(USER_BUTTON_PIN_NUM))         // GPIO_PIN_13 bit mask
#define USER_BUTTON_PORT USER_BUTTON_PORT_ENUM    // HAL_GPIO_PORT_C enum
#define USER_BUTTON_PIN_INDEX USER_BUTTON_PIN_NUM // 13

// Direct STM32 HAL versions (for modules that use direct HAL)
#define USER_BUTTON_STM32_PORT GPIOC      // STM32 HAL port pointer
#define USER_BUTTON_STM32_PIN GPIO_PIN_13 // STM32 HAL pin mask

// Safety relay computed definitions
// HAL abstraction versions (for modules using HAL abstraction)
#define SAFETY_RELAY1_PIN                                                     \
    (PIN_TO_BITMASK(SAFETY_RELAY1_PIN_NUM))        // GPIO_PIN_4 bit mask
#define SAFETY_RELAY1_PORT SAFETY_RELAY1_PORT_ENUM // HAL_GPIO_PORT_C enum
#define SAFETY_RELAY2_PIN                                                     \
    (PIN_TO_BITMASK(SAFETY_RELAY2_PIN_NUM))        // GPIO_PIN_5 bit mask
#define SAFETY_RELAY2_PORT SAFETY_RELAY2_PORT_ENUM // HAL_GPIO_PORT_C enum

// Direct STM32 HAL versions (for modules that use direct HAL)
#define SAFETY_RELAY1_STM32_PIN GPIO_PIN_4            // STM32 HAL pin mask
#define SAFETY_RELAY1_STM32_PORT GPIOC                // STM32 HAL port pointer
#define SAFETY_RELAY2_STM32_PIN GPIO_PIN_5            // STM32 HAL pin mask
#define SAFETY_RELAY2_STM32_PORT GPIOC                // STM32 HAL port pointer
#define SAFETY_RELAY1_PIN_INDEX SAFETY_RELAY1_PIN_NUM // 4
#define SAFETY_RELAY2_PIN_INDEX SAFETY_RELAY2_PIN_NUM // 5

// Emergency Stop System Hardware (SSOT) - Safety Critical
// HAL abstraction versions (for modules using HAL abstraction)
#define ESTOP_BUTTON_PIN USER_BUTTON_PIN // Emergency stop button (bit mask)
#define ESTOP_BUTTON_PORT                                                     \
    USER_BUTTON_PORT // Emergency stop button port (HAL enum)
#define ESTOP_BUTTON_PIN_INDEX                                                \
    USER_BUTTON_PIN_INDEX // Pin index for tests (13)

// Direct STM32 HAL versions (for modules that use direct HAL)
#define ESTOP_BUTTON_STM32_PIN USER_BUTTON_STM32_PIN // STM32 HAL pin mask
#define ESTOP_BUTTON_STM32_PORT                                               \
    USER_BUTTON_STM32_PORT // STM32 HAL port pointer

// Emergency stop button trigger type for interrupts
#define ESTOP_BUTTON_TRIGGER_TYPE HAL_GPIO_MODE_IT_FALLING

// Emergency stop LED mapping for HAL abstraction compatibility
#ifdef USE_STM32_HAL_DIRECT
// For STM32 HAL direct usage - emergency stop uses HAL abstraction, so use HAL
// enum
#define ESTOP_LED_PIN LED_RED_PIN // Emergency stop LED pin mask
#define ESTOP_LED_PORT                                                        \
    LED_RED_PORT_ENUM // HAL abstraction enum for emergency stop
#else
// For HAL abstraction and host tests
#define ESTOP_LED_PIN LED_RED_PIN        // Emergency stop LED pin mask
#define ESTOP_LED_PORT LED_RED_PORT_ENUM // HAL abstraction enum
#endif
#define ESTOP_LED_PIN_INDEX LED_RED_PIN_INDEX // Pin index for tests (14)

/* ==========================================================================
 */
/* Motor Enable/Disable Pins (SSOT)                                          */
/* ==========================================================================
 */
#ifdef FIRMWARE_BUILD
// Firmware build: Use STM32 HAL constants
#define MOTOR1_ENABLE_PIN GPIO_PIN_6 // Motor 1 enable signal
#define MOTOR1_ENABLE_PORT GPIOC
#define MOTOR2_ENABLE_PIN GPIO_PIN_7 // Motor 2 enable signal
#define MOTOR2_ENABLE_PORT GPIOC
#else
// Host test build: Use HAL abstraction constants
#define MOTOR1_ENABLE_PIN (1U << 6)        // GPIO_PIN_6 equivalent
#define MOTOR1_ENABLE_PORT HAL_GPIO_PORT_C // HAL abstraction enum
#define MOTOR2_ENABLE_PIN (1U << 7)        // GPIO_PIN_7 equivalent
#define MOTOR2_ENABLE_PORT HAL_GPIO_PORT_C // HAL abstraction enum
#endif

/* ==========================================================================
 */
/* Timer Configuration for Control Loop and PWM (SSOT)                      */
/* ==========================================================================
 */
// TODO: See .instructions/control-timing.md for real-time control requirements

// Control loop timer (1kHz control frequency)
#define CONTROL_TIMER_INSTANCE TIM2
#define CONTROL_TIMER_FREQ_HZ 1000

// General purpose timer for timeouts and delays
#define GENERAL_TIMER_INSTANCE TIM3

/* ==========================================================================
 */
/* DMA Configuration for High-Performance Transfers (SSOT)                  */
/* ==========================================================================
 */
// TODO: See .github/instructions/hardware-pins.instructions.md for efficient
// peripheral data transfer

// SPI DMA channels for L6470 communication
#define MOTOR_SPI_DMA_TX_STREAM DMA1_Stream4
#define MOTOR_SPI_DMA_RX_STREAM DMA1_Stream3

// UART DMA for command processing
#define UART_DMA_TX_STREAM DMA1_Stream2
#define UART_DMA_RX_STREAM DMA1_Stream1

/* ==========================================================================
 */
/* Clock Configuration Parameters (SSOT)                                     */
/* ==========================================================================
 */
// TODO: See .github/instructions/hardware-pins.instructions.md for system
// clock tree setup

// Protect against redefinition warnings - only define if not already defined
#ifndef HSE_VALUE
#define HSE_VALUE 8000000UL // External crystal frequency (8MHz on Nucleo-144)
#endif

#ifndef LSE_VALUE
#define LSE_VALUE 32768UL // LSE crystal for RTC
#endif

#ifndef VDD_VALUE
#define VDD_VALUE 3300UL // VDD voltage in millivolts
#endif

// Power supply monitoring constants
#define SUPPLY_VOLTAGE_MIN_MV 10000 // Minimum supply voltage (10V)
#define SUPPLY_VOLTAGE_MAX_MV 15000 // Maximum supply voltage (15V)

// Temperature monitoring constants
#define SYSTEM_MAX_TEMP_C 85 // Maximum system temperature

// Position error constants
#define MAX_POSITION_ERROR_STEPS 100 // Maximum position error in steps

#define SYSTEM_CLOCK_FREQ 480000000UL // System clock frequency
#define AHB_CLOCK_FREQ 240000000UL    // AHB bus frequency
#define APB1_CLOCK_FREQ 120000000UL   // APB1 bus frequency
#define APB2_CLOCK_FREQ 120000000UL   // APB2 bus frequency

/* ==========================================================================
 */
/* Memory Map Configuration (SSOT)                                           */
/* ==========================================================================
 */
// TODO: See .github/instructions/hardware-pins.instructions.md for STM32H7
// memory organization

// STM32H753ZI Memory Layout
#define FLASH_BASE_ADDR 0x08000000UL
#define FLASH_SIZE_BYTES (2048 * 1024) // 2MB Flash

#define DTCM_RAM_BASE_ADDR 0x20000000UL // 128KB DTCM
#define DTCM_RAM_SIZE_BYTES (128 * 1024)

#define AXI_SRAM_BASE_ADDR 0x24000000UL // 512KB AXI SRAM
#define AXI_SRAM_SIZE_BYTES (512 * 1024)

#define SRAM1_BASE_ADDR 0x30000000UL // 128KB SRAM1
#define SRAM1_SIZE_BYTES (128 * 1024)

#define SRAM2_BASE_ADDR 0x30020000UL // 128KB SRAM2
#define SRAM2_SIZE_BYTES (128 * 1024)

#define SRAM3_BASE_ADDR 0x30040000UL // 32KB SRAM3
#define SRAM3_SIZE_BYTES (32 * 1024)

#define SRAM4_BASE_ADDR 0x38000000UL // 64KB SRAM4
#define SRAM4_SIZE_BYTES (64 * 1024)

// Ethernet DMA buffers placement (must be in D2 domain)
#define ETH_DMA_BUFFER_BASE SRAM2_BASE_ADDR

#endif /* HARDWARE_CONFIG_H */

/**
 * @note Hardware Configuration SSOT Rules:
 * 1. All pin assignments MUST be defined here
 * 2. Never hardcode GPIO ports/pins in source files
 * 3. Always include this header when accessing hardware
 * 4. Document the source (schematic/datasheet) for each assignment
 * 5. Use meaningful names that indicate function, not just location
 */
