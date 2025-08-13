/**
 * @file comm_config.h
 * @brief Communication Configuration - Single Source of Truth (SSOT)
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * @note This file contains ALL communication protocol parameters, addresses,
 * and settings. Never hardcode these values elsewhere - always reference this
 * SSOT.
 *
 * TODO: See .github/instructions/comm-protocols.instructions.md for protocol
 * specification details
 * TODO: See .github/instructions/stm32h7-uart-protocol.instructions.md for
 * UART command format
 * TODO: See .github/instructions/comm-protocols.instructions.md for CAN
 * message definitions
 */

#ifndef COMM_CONFIG_H
#define COMM_CONFIG_H

#include <stdint.h>

/* ==========================================================================
 */
/* Protocol Message Configuration (SSOT)                                     */
/* ==========================================================================
 */
// Message payload sizes
#define MAX_MESSAGE_PAYLOAD 128 // Maximum message payload size
// Note: MAX_ETH_PAYLOAD may be defined by HAL Legacy - use different name
#define COMM_MAX_ETH_PAYLOAD 1500 // Maximum Ethernet payload for our protocol

/* ==========================================================================
 */
/* UART Communication Configuration (SSOT)                                   */
/* ==========================================================================
 */
// TODO: See .github/instructions/stm32h7-uart-protocol.instructions.md for
// command protocol implementation

// UART Parameters
#define UART_BAUDRATE 115200
#define UART_WORD_LENGTH UART_WORDLENGTH_8B
#define UART_STOP_BITS UART_STOPBITS_1
#define UART_PARITY UART_PARITY_NONE
#define UART_HW_FLOW_CONTROL UART_HWCONTROL_NONE
#define UART_MODE UART_MODE_TX_RX

// UART Buffer Sizes
#define UART_TX_BUFFER_SIZE 512
#define UART_RX_BUFFER_SIZE 256
#define UART_CMD_MAX_LENGTH 64

// UART Timeouts (milliseconds)
#define UART_TX_TIMEOUT_MS 100
#define UART_RX_TIMEOUT_MS 1000
#define UART_CMD_TIMEOUT_MS 5000
#define UART_TIMEOUT_MS 1000       // Generic UART timeout
#define DEBUG_UART_BAUDRATE 115200 // Debug UART baudrate

// Protocol Timeouts
#define COMM_DEFAULT_TIMEOUT_MS 5000 // Default communication timeout

// UART Command Delimiters
#define UART_CMD_START_CHAR '{'
#define UART_CMD_END_CHAR '}'
#define UART_CMD_SEPARATOR ','
#define UART_RESPONSE_OK "OK"
#define UART_RESPONSE_ERROR "ERROR"

/* ==========================================================================
 */
/* CAN-FD Communication Configuration (SSOT)                                 */
/* ==========================================================================
 */
// TODO: See .github/instructions/comm-protocols.instructions.md for message
// protocol and error handling

// CAN Bit Timing Parameters (500 kbps nominal)
#define CAN_BITRATE_KBPS 500
#define CAN_PRESCALER 12
#define CAN_TIME_SEG1 13
#define CAN_TIME_SEG2 2
#define CAN_SYNC_JUMP_WIDTH 1

// CAN-FD Data Phase Timing (2 Mbps data phase)
#define CAN_FD_DATA_BITRATE_KBPS 2000
#define CAN_FD_DATA_PRESCALER 3
#define CAN_FD_DATA_TIME_SEG1 13
#define CAN_FD_DATA_TIME_SEG2 2
#define CAN_FD_DATA_SYNC_JUMP_WIDTH 1

// CAN Message Configuration
#define CAN_MAX_MESSAGE_SIZE 64 // CAN-FD max payload
#define CAN_MOTOR_BASE_ID 0x100 // Base CAN ID for motor commands
#define CAN_TIMEOUT_MS 1000     // CAN transmission timeout
#define CAN_TX_FIFO_SIZE 16
#define CAN_RX_FIFO_SIZE 32

// CAN Node Configuration
#define CAN_NODE_ID 0x01      // This controller's node ID
#define CAN_BROADCAST_ID 0x00 // Broadcast messages
#define CAN_MAX_NODES 16      // Maximum nodes on bus

/* ==========================================================================
 */
/* CAN Message ID Allocation (SSOT)                                          */
/* ==========================================================================
 */
// TODO: See .github/instructions/comm-protocols.instructions.md for message
// format specification

// Base IDs for different message types
#define CAN_ID_BASE_STATUS 0x100    // Node status messages
#define CAN_ID_BASE_COMMAND 0x200   // Command messages
#define CAN_ID_BASE_DATA 0x300      // Sensor data messages
#define CAN_ID_BASE_ERROR 0x400     // Error/fault messages
#define CAN_ID_BASE_HEARTBEAT 0x000 // Heartbeat/sync messages

// Specific Message IDs (Node ID is added to base)
#define CAN_ID_MOTOR1_STATUS (CAN_ID_BASE_STATUS + 0x01) // 0x101
#define CAN_ID_MOTOR2_STATUS (CAN_ID_BASE_STATUS + 0x02) // 0x102
#define CAN_ID_ENCODER1_DATA (CAN_ID_BASE_DATA + 0x01)   // 0x301
#define CAN_ID_ENCODER2_DATA (CAN_ID_BASE_DATA + 0x02)   // 0x302

#define CAN_ID_MOVE_COMMAND (CAN_ID_BASE_COMMAND + 0x01)  // 0x201
#define CAN_ID_STOP_COMMAND (CAN_ID_BASE_COMMAND + 0x02)  // 0x202
#define CAN_ID_CALIBRATE_CMD (CAN_ID_BASE_COMMAND + 0x03) // 0x203

#define CAN_ID_MOTOR_FAULT (CAN_ID_BASE_ERROR + 0x01)   // 0x401
#define CAN_ID_ENCODER_FAULT (CAN_ID_BASE_ERROR + 0x02) // 0x402
#define CAN_ID_SYSTEM_FAULT (CAN_ID_BASE_ERROR + 0x03)  // 0x403

#define CAN_ID_HEARTBEAT (CAN_ID_BASE_HEARTBEAT + CAN_NODE_ID) // 0x001
#define CAN_ID_TIME_SYNC (CAN_ID_BASE_HEARTBEAT + 0xFF)        // 0x0FF

// CAN Filter Configuration
#define CAN_FILTER_COUNT 8
#define CAN_FILTER_FIFO 0

/* ==========================================================================
 */
/* Ethernet/TCP Configuration (SSOT)                                         */
/* ==========================================================================
 */
// TODO: See .github/instructions/comm-protocols.instructions.md for LwIP stack
// parameters

// Network Configuration
#define ETH_STATIC_IP "192.168.1.50"
#define ETH_NETMASK "255.255.255.0"
#define ETH_GATEWAY "192.168.1.1"
#define ETH_DNS_SERVER "8.8.8.8"
#define ETH_MAC_ADDR {0x02, 0x00, 0x00, 0x12, 0x34, 0x56}

// TCP Server Configuration
#define TCP_SERVER_PORT 8000
#define TCP_MAX_CONNECTIONS 4
#define TCP_KEEPALIVE_IDLE 7200   // seconds
#define TCP_KEEPALIVE_INTERVAL 75 // seconds
#define TCP_KEEPALIVE_COUNT 9

// TCP Buffer Sizes
#define TCP_TX_BUFFER_SIZE 2048
#define TCP_RX_BUFFER_SIZE 1024
#define TCP_LISTEN_BACKLOG 5

// HTTP Server Configuration (optional)
#define HTTP_SERVER_PORT 80
#define HTTP_MAX_URI_LENGTH 256
#define HTTP_MAX_HEADER_LENGTH 512

/* ==========================================================================
 */
/* I2C Communication Configuration (SSOT)                                    */
/* ==========================================================================
 */
// TODO: See .github/instructions/stm32h7-i2c-as5600.instructions.md for timing
// and error handling

// I2C Timing Configuration (400 kHz fast mode)
#define I2C_TIMING_400KHZ 0x40912732 // Calculated for 120MHz APB1
#define I2C_TIMING_100KHZ 0x40C03E5B // Standard mode fallback

// I2C Operation Parameters
#define I2C_MAX_TRANSFER_SIZE 32
#define I2C_TIMEOUT_MS 1000
#define I2C_RETRY_COUNT 3

// I2C Device Addresses (7-bit addresses)
#define I2C_ADDR_AS5600 0x36 // Magnetic encoder
#define I2C_ADDR_EEPROM 0x50 // Optional calibration storage

/* ==========================================================================
 */
/* SPI Communication Configuration (SSOT)                                    */
/* ==========================================================================
 */
// TODO: See .github/instructions/stm32h7-spi-l6470.instructions.md for
// daisy-chain protocol timing

// SPI Parameters for L6470 Communication
#define SPI_BAUDRATE_PRESCALER SPI_BAUDRATEPRESCALER_32 // ~3.75 MHz
#define SPI_DIRECTION SPI_DIRECTION_2LINES
#define SPI_DATASIZE SPI_DATASIZE_8BIT
#define SPI_CPOL SPI_POLARITY_HIGH // CPOL = 1
#define SPI_CPHA SPI_PHASE_2EDGE   // CPHA = 1
#define SPI_NSS SPI_NSS_SOFT
#define SPI_FIRST_BIT SPI_FIRSTBIT_MSB

// SPI Timing and Buffers
#define SPI_TIMEOUT_MS 100
#define SPI_MAX_TRANSFER_SIZE 8 // Maximum bytes per transaction
#define SPI_CS_SETUP_TIME_US 1  // CS setup time before clock
#define SPI_CS_HOLD_TIME_US 1   // CS hold time after clock

// L6470 Daisy Chain Configuration
#define L6470_CHAIN_LENGTH 2   // Two motors in daisy chain
#define L6470_NOP_COMMAND 0x00 // No-operation command

/* ==========================================================================
 */
/* Communication Timing and Synchronization (SSOT)                          */
/* ==========================================================================
 */
// TODO: See .github/instructions/comm-protocols.instructions.md for real-time
// requirements

// Message Rates (milliseconds between transmissions)
#define HEARTBEAT_INTERVAL_MS 1000 // CAN heartbeat rate
#define STATUS_UPDATE_RATE_MS 100  // Motor status updates
#define SENSOR_SAMPLE_RATE_MS 10   // Encoder reading rate
#define WATCHDOG_TIMEOUT_MS 5000   // Communication watchdog

// Protocol Timeouts
#define CMD_RESPONSE_TIMEOUT_MS 500 // Maximum command processing time
#define LINK_TIMEOUT_MS 10000       // Link failure detection
#define RECONNECT_INTERVAL_MS 30000 // Auto-reconnect attempt rate

/* ==========================================================================
 */
/* Message Format Definitions (SSOT)                                         */
/* ==========================================================================
 */
// TODO: See .github/instructions/comm-protocols.instructions.md for protocol
// details

// JSON Message Templates (for UART/TCP)
#define JSON_MOTOR_STATUS_FMT                                                 \
    "{\"motor\":%d,\"angle\":%.2f,\"speed\":%.2f,\"status\":%d}"
#define JSON_COMMAND_FMT "{\"cmd\":\"%s\",\"motor\":%d,\"value\":%.2f}"
#define JSON_ERROR_FMT "{\"error\":%d,\"msg\":\"%s\",\"timestamp\":%lu}"

// Binary Message Headers (for CAN)
#define MSG_HEADER_SYNC 0xAA
#define MSG_HEADER_VERSION 0x01

// CRC16 Checksum Constants (SSOT)
#define CRC16_INIT_VALUE 0xFFFF // Initial CRC value
#define CRC16_POLYNOMIAL 0xA001 // CRC-16-ANSI polynomial
#define INVALID_DEVICE_ID 0xFF  // Invalid/broadcast device ID

#define MSG_CHECKSUM_POLY 0x1D // CRC polynomial

// Message Priority Levels
#define MSG_PRIORITY_EMERGENCY 0 // Emergency stop, faults
#define MSG_PRIORITY_HIGH 1      // Real-time control
#define MSG_PRIORITY_NORMAL 2    // Status updates
#define MSG_PRIORITY_LOW 3       // Configuration, diagnostics

// ============================================================================
// Communication IRQ Numbers
// ============================================================================

// System Communication IRQs
#define SYSTEM_UART_IRQ USART3_IRQn    // Debug/System UART interrupt
#define SYSTEM_CAN_IRQ FDCAN1_IT0_IRQn // System CAN bus interrupt

#endif /* COMM_CONFIG_H */

/**
 * @note Communication Configuration SSOT Rules:
 * 1. All protocol parameters MUST be defined here
 * 2. Never hardcode baud rates, timeouts, or addresses in source files
 * 3. Message IDs must be allocated from defined ranges
 * 4. All timing parameters centralized for easy tuning
 * 5. Protocol versions and formats defined once
 */
