/**
 * @file hardware_config.h
 * @brief Hardware Configuration - Single Source of Truth (SSOT)
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 * 
 * @note This file contains ALL hardware pin assignments and peripheral configurations.
 *       Never hardcode these values elsewhere - always reference this SSOT.
 * 
 * TODO: See .instructions/hardware-config.md for complete pin mapping documentation
 * TODO: See .instructions/stm32h753zi.md for MCU-specific configuration details
 */

#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include "stm32h7xx_hal.h"

/* ========================================================================== */
/* MCU Configuration (SSOT)                                                  */
/* ========================================================================== */
#define MCU_FAMILY              "STM32H7"
#define MCU_PART_NUMBER         "STM32H753ZI"
#define MCU_CORE_FREQUENCY_HZ   480000000UL
#define MCU_PACKAGE             "LQFP144"

/* ========================================================================== */
/* SPI Configuration for L6470 Stepper Drivers (SSOT)                       */
/* ========================================================================== */
// TODO: See .instructions/l6470-spi.md for daisy-chain protocol implementation
#define MOTOR_SPI_INSTANCE      SPI2
#define MOTOR_SPI_SCK_PIN       GPIO_PIN_13
#define MOTOR_SPI_SCK_PORT      GPIOB
#define MOTOR_SPI_SCK_AF        GPIO_AF5_SPI2
#define MOTOR_SPI_MISO_PIN      GPIO_PIN_14
#define MOTOR_SPI_MISO_PORT     GPIOB
#define MOTOR_SPI_MISO_AF       GPIO_AF5_SPI2
#define MOTOR_SPI_MOSI_PIN      GPIO_PIN_15
#define MOTOR_SPI_MOSI_PORT     GPIOB
#define MOTOR_SPI_MOSI_AF       GPIO_AF5_SPI2

// Chip Select (CS) for daisy-chained L6470 drivers
#define MOTOR_SPI_CS_PIN        GPIO_PIN_9
#define MOTOR_SPI_CS_PORT       GPIOA

// L6470 Control Signals
#define MOTOR_FLAG_PIN          GPIO_PIN_10   // Fault indication from drivers
#define MOTOR_FLAG_PORT         GPIOA
#define MOTOR_BUSY_PIN          GPIO_PIN_11   // Busy indication from drivers
#define MOTOR_BUSY_PORT         GPIOA

/* ========================================================================== */
/* I2C Configuration for AS5600 Magnetic Encoders (SSOT)                    */
/* ========================================================================== */
// TODO: See .instructions/as5600-encoder.md for encoder calibration procedures

// Encoder 1: Motor 1 Position Feedback
#define ENCODER1_I2C_INSTANCE   I2C1
#define ENCODER1_I2C_SCL_PIN    GPIO_PIN_6
#define ENCODER1_I2C_SCL_PORT   GPIOB
#define ENCODER1_I2C_SCL_AF     GPIO_AF4_I2C1
#define ENCODER1_I2C_SDA_PIN    GPIO_PIN_7
#define ENCODER1_I2C_SDA_PORT   GPIOB
#define ENCODER1_I2C_SDA_AF     GPIO_AF4_I2C1
#define ENCODER1_I2C_ADDRESS    0x36

// Encoder 2: Motor 2 Position Feedback  
#define ENCODER2_I2C_INSTANCE   I2C2
#define ENCODER2_I2C_SCL_PIN    GPIO_PIN_10
#define ENCODER2_I2C_SCL_PORT   GPIOB
#define ENCODER2_I2C_SCL_AF     GPIO_AF4_I2C2
#define ENCODER2_I2C_SDA_PIN    GPIO_PIN_11
#define ENCODER2_I2C_SDA_PORT   GPIOB
#define ENCODER2_I2C_SDA_AF     GPIO_AF4_I2C2
#define ENCODER2_I2C_ADDRESS    0x36

/* ========================================================================== */
/* UART Configuration for Virtual COM Port (SSOT)                           */
/* ========================================================================== */
// TODO: See .instructions/uart-protocol.md for command protocol specification
#define UART_INSTANCE           USART3
#define UART_TX_PIN             GPIO_PIN_8
#define UART_TX_PORT            GPIOD
#define UART_TX_AF              GPIO_AF7_USART3
#define UART_RX_PIN             GPIO_PIN_9
#define UART_RX_PORT            GPIOD
#define UART_RX_AF              GPIO_AF7_USART3

/* ========================================================================== */
/* CAN-FD Configuration for MCU-to-MCU Communication (SSOT)                 */
/* ========================================================================== */
// TODO: See .instructions/can-fd.md for message ID allocation and protocol
#define CAN_INSTANCE            FDCAN1
#define CAN_TX_PIN              GPIO_PIN_1
#define CAN_TX_PORT             GPIOD
#define CAN_TX_AF               GPIO_AF9_FDCAN1
#define CAN_RX_PIN              GPIO_PIN_0
#define CAN_RX_PORT             GPIOD
#define CAN_RX_AF               GPIO_AF9_FDCAN1

/* ========================================================================== */
/* Ethernet Configuration for Network Connectivity (SSOT)                   */
/* ========================================================================== */
// TODO: See .instructions/ethernet-lwip.md for LwIP stack configuration
// Note: STM32H753ZI Nucleo has on-board LAN8742 PHY connected via RMII

// RMII Interface Pins
#define ETH_RMII_REF_CLK_PIN    GPIO_PIN_1
#define ETH_RMII_REF_CLK_PORT   GPIOA
#define ETH_RMII_REF_CLK_AF     GPIO_AF11_ETH
#define ETH_RMII_MDIO_PIN       GPIO_PIN_2
#define ETH_RMII_MDIO_PORT      GPIOA
#define ETH_RMII_MDIO_AF        GPIO_AF11_ETH
#define ETH_RMII_MDC_PIN        GPIO_PIN_1
#define ETH_RMII_MDC_PORT       GPIOC
#define ETH_RMII_MDC_AF         GPIO_AF11_ETH

#define ETH_RMII_CRS_DV_PIN     GPIO_PIN_7
#define ETH_RMII_CRS_DV_PORT    GPIOA
#define ETH_RMII_CRS_DV_AF      GPIO_AF11_ETH
#define ETH_RMII_RXD0_PIN       GPIO_PIN_4
#define ETH_RMII_RXD0_PORT      GPIOC
#define ETH_RMII_RXD0_AF        GPIO_AF11_ETH
#define ETH_RMII_RXD1_PIN       GPIO_PIN_5
#define ETH_RMII_RXD1_PORT      GPIOC
#define ETH_RMII_RXD1_AF        GPIO_AF11_ETH

#define ETH_RMII_TX_EN_PIN      GPIO_PIN_11
#define ETH_RMII_TX_EN_PORT     GPIOG
#define ETH_RMII_TX_EN_AF       GPIO_AF11_ETH
#define ETH_RMII_TXD0_PIN       GPIO_PIN_13
#define ETH_RMII_TXD0_PORT      GPIOG
#define ETH_RMII_TXD0_AF        GPIO_AF11_ETH
#define ETH_RMII_TXD1_PIN       GPIO_PIN_12
#define ETH_RMII_TXD1_PORT      GPIOB
#define ETH_RMII_TXD1_AF        GPIO_AF11_ETH

// PHY Configuration
#define ETH_PHY_ADDRESS         0x00        // LAN8742 PHY address
#define ETH_PHY_RESET_PIN       GPIO_PIN_3  // Optional reset pin
#define ETH_PHY_RESET_PORT      GPIOG

/* ========================================================================== */
/* GPIO Configuration for Status LEDs and User Interface (SSOT)             */
/* ========================================================================== */
// TODO: See .instructions/user-interface.md for LED indication patterns

// Nucleo-144 on-board LEDs
#define LED_GREEN_PIN           GPIO_PIN_0   // LD1 - System status
#define LED_GREEN_PORT          GPIOB
#define LED_YELLOW_PIN          GPIO_PIN_1   // LD2 - CAN activity  
#define LED_YELLOW_PORT         GPIOE
#define LED_RED_PIN             GPIO_PIN_14  // LD3 - Error/fault
#define LED_RED_PORT            GPIOB

// User button
#define USER_BUTTON_PIN         GPIO_PIN_13  // B1 - Emergency stop
#define USER_BUTTON_PORT        GPIOC

/* ========================================================================== */
/* Timer Configuration for Control Loop and PWM (SSOT)                      */
/* ========================================================================== */
// TODO: See .instructions/control-timing.md for real-time control requirements

// Control loop timer (1kHz control frequency)
#define CONTROL_TIMER_INSTANCE  TIM2
#define CONTROL_TIMER_FREQ_HZ   1000

// General purpose timer for timeouts and delays
#define GENERAL_TIMER_INSTANCE  TIM3

/* ========================================================================== */
/* DMA Configuration for High-Performance Transfers (SSOT)                  */
/* ========================================================================== */
// TODO: See .instructions/dma-config.md for efficient peripheral data transfer

// SPI DMA channels for L6470 communication
#define MOTOR_SPI_DMA_TX_STREAM DMA1_Stream4
#define MOTOR_SPI_DMA_RX_STREAM DMA1_Stream3

// UART DMA for command processing
#define UART_DMA_TX_STREAM      DMA1_Stream2
#define UART_DMA_RX_STREAM      DMA1_Stream1

/* ========================================================================== */
/* Clock Configuration Parameters (SSOT)                                     */
/* ========================================================================== */
// TODO: See .instructions/clock-config.md for system clock tree setup

#define HSE_VALUE               25000000UL   // External crystal frequency
#define LSE_VALUE               32768UL      // LSE crystal for RTC
#define SYSTEM_CLOCK_FREQ       480000000UL  // System clock frequency
#define AHB_CLOCK_FREQ          240000000UL  // AHB bus frequency
#define APB1_CLOCK_FREQ         120000000UL  // APB1 bus frequency  
#define APB2_CLOCK_FREQ         120000000UL  // APB2 bus frequency

/* ========================================================================== */
/* Memory Map Configuration (SSOT)                                           */
/* ========================================================================== */
// TODO: See .instructions/memory-layout.md for STM32H7 memory organization

// STM32H753ZI Memory Layout
#define FLASH_BASE_ADDR         0x08000000UL
#define FLASH_SIZE_BYTES        (2048 * 1024)  // 2MB Flash

#define DTCM_RAM_BASE_ADDR      0x20000000UL   // 128KB DTCM
#define DTCM_RAM_SIZE_BYTES     (128 * 1024)

#define AXI_SRAM_BASE_ADDR      0x24000000UL   // 512KB AXI SRAM
#define AXI_SRAM_SIZE_BYTES     (512 * 1024)

#define SRAM1_BASE_ADDR         0x30000000UL   // 128KB SRAM1
#define SRAM1_SIZE_BYTES        (128 * 1024)

#define SRAM2_BASE_ADDR         0x30020000UL   // 128KB SRAM2  
#define SRAM2_SIZE_BYTES        (128 * 1024)

#define SRAM3_BASE_ADDR         0x30040000UL   // 32KB SRAM3
#define SRAM3_SIZE_BYTES        (32 * 1024)

#define SRAM4_BASE_ADDR         0x38000000UL   // 64KB SRAM4
#define SRAM4_SIZE_BYTES        (64 * 1024)

// Ethernet DMA buffers placement (must be in D2 domain)
#define ETH_DMA_BUFFER_BASE     SRAM2_BASE_ADDR

#endif /* HARDWARE_CONFIG_H */

/**
 * @note Hardware Configuration SSOT Rules:
 * 1. All pin assignments MUST be defined here
 * 2. Never hardcode GPIO ports/pins in source files  
 * 3. Always include this header when accessing hardware
 * 4. Document the source (schematic/datasheet) for each assignment
 * 5. Use meaningful names that indicate function, not just location
 */
