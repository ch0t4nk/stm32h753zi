/**
 * @file comm_protocol.h
 * @brief Communication Protocol Interface - STM32H753ZI Stepper Motor Project
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @note Communication implementation following:
 * - .github/instructions/comms.instructions.md
 * - config/comm_config.h (SSOT)
 *
 * @warning SAFETY-CRITICAL: All communication must integrate with safety
 * system
 */

#ifndef COMM_PROTOCOL_H
#define COMM_PROTOCOL_H

#include "common/data_types.h"
#include "config/comm_config.h"
#include "config/safety_config.h"
#include "safety/safety_system.h"
#ifndef UNITY_TESTING
#include "stm32h7xx_hal.h"
#endif
#include <stdbool.h>
#include <stdint.h>
// Shared SSOT constants
#include "config/constants_shared.h"
/* Project-wide SSOT: small constants that may be materialized as hardcoded
 * values in lower-level code. See src/config/project_constants.h for
 * definitions and runtime/update notes.
 */
#include "config/project_constants.h"

/* Per-file mapping: message header magic value materialized from SSOT.
 * This makes the dependency explicit for validators and keeps the canonical
 * value in src/config/project_constants.h
 */
#ifndef MESSAGE_HEADER_MAGIC
#define MESSAGE_HEADER_MAGIC SSOT_SYSTEM_STATE_MAGIC
#endif

/* ==========================================================================
 */
/* Communication Protocol Types (following comms.instructions.md)           */
/* ==========================================================================
 */

/**
 * @brief Communication protocol layers
 */
typedef enum {
    COMM_LAYER_PHYSICAL,   ///< GPIO, hardware interfaces
    COMM_LAYER_DATALINK,   ///< SPI, I2C, UART framing
    COMM_LAYER_NETWORK,    ///< CAN, Ethernet protocols
    COMM_LAYER_TRANSPORT,  ///< Message segmentation, flow control
    COMM_LAYER_APPLICATION ///< Motor control commands, status
} CommLayer_t;

/**
 * @brief Protocol types supported
 */
typedef enum {
    PROTOCOL_UART_ASCII,   ///< Human-readable ASCII commands
    PROTOCOL_UART_BINARY,  ///< Binary protocol for efficiency
    PROTOCOL_CAN_MOTOR,    ///< CAN motor control protocol
    PROTOCOL_SPI_L6470,    ///< L6470 stepper driver protocol
    PROTOCOL_I2C_AS5600,   ///< AS5600 encoder protocol
    PROTOCOL_ETHERNET_UDP, ///< UDP for real-time data
    PROTOCOL_ETHERNET_TCP  ///< TCP for reliable commands
} CommProtocol_t;

/**
 * @brief Universal message header
 */
typedef struct {
    uint32_t magic;      ///< MESSAGE_HEADER_MAGIC (from SSOT) for validation
    uint16_t message_id; ///< Unique message identifier
    uint16_t payload_length; ///< Payload size in bytes
    uint8_t protocol_type;   ///< CommProtocol_t value
    uint8_t priority;        ///< Message priority (0=highest)
    uint16_t checksum;       ///< CRC16 checksum
    uint32_t timestamp_ms;   ///< Message timestamp
} __attribute__((packed)) MessageHeader_t;

/**
 * @brief Complete message structure
 */
typedef struct {
    MessageHeader_t header;
    uint8_t payload[MAX_MESSAGE_PAYLOAD];
} Message_t;

/**
 * @brief Communication channel configuration
 */
typedef struct {
    CommProtocol_t protocol; ///< Protocol type
    bool enabled;            ///< Channel enabled/disabled
    uint32_t baudrate;       ///< Baudrate for serial protocols
    uint32_t timeout_ms;     ///< Communication timeout
    uint32_t tx_count;       ///< Transmitted message count
    uint32_t rx_count;       ///< Received message count
    uint32_t error_count;    ///< Communication error count
    uint32_t last_activity;  ///< Last activity timestamp
} CommChannelConfig_t;

/* ==========================================================================
 */
/* Motor Control Command Types                                               */
/* ==========================================================================
 */

/**
 * @brief Motor control command types
 */
typedef enum {
    MOTOR_CMD_STOP = 0x01,            ///< Stop motor
    MOTOR_CMD_EMERGENCY_STOP = 0x02,  ///< Emergency stop
    MOTOR_CMD_MOVE_ABSOLUTE = 0x10,   ///< Move to absolute position
    MOTOR_CMD_MOVE_RELATIVE = 0x11,   ///< Move relative to current position
    MOTOR_CMD_MOVE_CONTINUOUS = 0x12, ///< Continuous motion
    MOTOR_CMD_HOME = 0x20,            ///< Home motor
    MOTOR_CMD_CALIBRATE = 0x21,       ///< Calibrate position
    MOTOR_CMD_SET_POSITION = 0x22,    ///< Set current position
    MOTOR_CMD_GET_STATUS = 0x30,      ///< Get motor status
    MOTOR_CMD_GET_POSITION = 0x31,    ///< Get current position
    MOTOR_CMD_SET_PARAMETERS = 0x40,  ///< Set motor parameters
    MOTOR_CMD_SELF_TEST = 0x50        ///< Perform self-test
} MotorCommandType_t;

/**
 * @brief Motor command structure
 */
typedef struct {
    uint8_t motor_id;           ///< Target motor ID (0-based)
    MotorCommandType_t command; ///< Command type
    union {
        struct {
            int32_t position_steps; ///< Target position for move commands
            uint32_t speed_steps_per_sec; ///< Speed for move commands
            uint32_t acceleration;        ///< Acceleration for move commands
        } move;
        struct {
            uint32_t max_speed;    ///< Maximum speed parameter
            uint32_t acceleration; ///< Acceleration parameter
            uint32_t current_ma;   ///< Motor current in mA
        } parameters;
        uint32_t raw_data; ///< Raw command data
    } data;
} MotorCommand_t;

/**
 * @brief Motor status response structure
 */
typedef struct {
    uint8_t motor_id;             ///< Motor ID
    uint8_t state;                ///< Current motor state
    int32_t position_steps;       ///< Current position in steps
    uint32_t speed_steps_per_sec; ///< Current speed
    uint32_t fault_flags;         ///< Active fault flags
    uint32_t timestamp;           ///< Status timestamp
} MotorStatusResponse_t;

/* ==========================================================================
 */
/* Public Function Prototypes                                                */
/* ==========================================================================
 */

/**
 * @brief Initialize communication protocol system
 * @return System error code
 */
SystemError_t comm_protocol_init(void);

/**
 * @brief Initialize UART communication channel
 * @param huart UART handle
 * @param protocol Protocol type (ASCII or binary)
 * @return System error code
 */
SystemError_t comm_uart_init(UART_HandleTypeDef *huart,
                             CommProtocol_t protocol);

/**
 * @brief Initialize CAN communication channel
 * @param hfdcan FDCAN handle (STM32H7 uses FDCAN not classic CAN)
 * @return System error code
 */
SystemError_t comm_can_init(FDCAN_HandleTypeDef *hfdcan);

/**
 * @brief Process received message
 * @param message Pointer to received message
 * @return System error code
 */
SystemError_t comm_process_message(const Message_t *message);

/**
 * @brief Send motor command via communication channel
 * @param protocol Protocol to use
 * @param command Pointer to motor command
 * @return System error code
 */
SystemError_t comm_send_motor_command(CommProtocol_t protocol,
                                      const MotorCommand_t *command);

/**
 * @brief Send motor status response
 * @param protocol Protocol to use
 * @param status Pointer to motor status
 * @return System error code
 */
SystemError_t comm_send_motor_status(CommProtocol_t protocol,
                                     const MotorStatusResponse_t *status);

/**
 * @brief Send text message via UART (for debugging)
 * @param protocol UART protocol to use
 * @param message Text message to send
 * @return System error code
 */
SystemError_t comm_send_text_message(CommProtocol_t protocol,
                                     const char *message);

/**
 * @brief Process UART ASCII command
 * @param command_string ASCII command string
 * @return System error code
 */
SystemError_t comm_process_ascii_command(const char *command_string);

/**
 * @brief Parse motor command from ASCII string
 * @param ascii_command ASCII command string
 * @param motor_command Pointer to motor command structure to fill
 * @return System error code
 */
SystemError_t comm_parse_ascii_motor_command(const char *ascii_command,
                                             MotorCommand_t *motor_command);

/**
 * @brief Format motor status as ASCII string
 * @param status Pointer to motor status
 * @param ascii_buffer Buffer to store ASCII string
 * @param buffer_size Size of ASCII buffer
 * @return System error code
 */
SystemError_t
comm_format_ascii_motor_status(const MotorStatusResponse_t *status,
                               char *ascii_buffer, size_t buffer_size);

/**
 * @brief Periodic communication task (call from main loop)
 * @return System error code
 */
SystemError_t comm_protocol_task(void);

/**
 * @brief Check communication channel health
 * @param protocol Protocol to check
 * @return True if healthy, false otherwise
 */
bool comm_channel_is_healthy(CommProtocol_t protocol);

/**
 * @brief Get communication channel statistics
 * @param protocol Protocol to query
 * @param config Pointer to configuration structure to fill
 * @return System error code
 */
SystemError_t comm_get_channel_config(CommProtocol_t protocol,
                                      CommChannelConfig_t *config);

/**
 * @brief Enable/disable communication channel
 * @param protocol Protocol to control
 * @param enabled True to enable, false to disable
 * @return System error code
 */
SystemError_t comm_set_channel_enabled(CommProtocol_t protocol, bool enabled);

/**
 * @brief Perform communication system self-test
 * @return System error code
 */
SystemError_t comm_protocol_self_test(void);

/* ==========================================================================
 */
/* UART Command Processing Functions                                         */
/* ==========================================================================
 */

/**
 * @brief Initialize debug UART for command interface
 * @param huart UART handle (typically USART3 for ST-Link VCP)
 * @return System error code
 */
SystemError_t debug_uart_init(UART_HandleTypeDef *huart);

/**
 * @brief Process UART RX complete callback
 * @param huart UART handle
 */
void comm_uart_rx_complete_callback(UART_HandleTypeDef *huart);

/**
 * @brief Process UART TX complete callback
 * @param huart UART handle
 */
void comm_uart_tx_complete_callback(UART_HandleTypeDef *huart);

/**
 * @brief Process UART error callback
 * @param huart UART handle
 */
void comm_uart_error_callback(UART_HandleTypeDef *huart);

/* ==========================================================================
 */
/* CAN Protocol Functions                                                    */
/* ==========================================================================
 */

/**
 * @brief Send CAN motor command
 * @param motor_id Target motor ID
 * @param command Motor command
 * @return System error code
 */
SystemError_t comm_can_send_motor_command(uint8_t motor_id,
                                          const MotorCommand_t *command);

/**
 * @brief Process received CAN message
 * @param hcan CAN handle
 */
void comm_can_rx_callback(FDCAN_HandleTypeDef *hfdcan);

/* ==========================================================================
 */
/* Safety Integration Macros                                                 */
/* ==========================================================================
 */

/**
 * @brief Safety check before processing commands
 */
#define COMM_SAFETY_CHECK()                                                   \
    do {                                                                      \
        if (!safety_system_is_operational()) {                                \
            return ERROR_SAFETY_SYSTEM_NOT_READY;                             \
        }                                                                     \
        if (safety_get_emergency_stop_state()) {                              \
            return ERROR_SAFETY_EMERGENCY_STOP;                               \
        }                                                                     \
    } while (0)

/**
 * @brief Validate message header
 */
#define VALIDATE_MESSAGE_HEADER(header)                                       \
    do {                                                                      \
        if ((header)->magic != MESSAGE_MAGIC) {                               \
            return ERROR_COMM_INVALID_MESSAGE;                                \
        }                                                                     \
        if ((header)->payload_length > MAX_MESSAGE_PAYLOAD) {                 \
            return ERROR_COMM_MESSAGE_TOO_LARGE;                              \
        }                                                                     \
    } while (0)

#/ *
#* Prefer SSOT usage instead of hardcoded numbers.If a consumer requires a
#* hard - coded literal(for example for static buffer allocation), use a
#* local conversion macro to make the decision explicit.Example below shows
#* a conversion from SSOT to local buffer sizes.
#*
#* Notes:
#* - `SSOT_MAX_MOTORS` is a build -                                           \
    time SSOT that requires a rebuild / reflash
#* to change.Consumer code must document whether a runtime reconfiguration
#* is supported.See top - level SSOT header comment.
#* /

/* Local buffer sizing derived from SSOT values. These are safe compile-time
 * calculations and document whether they are runtime-changeable.
 * - UART_TX_BUFFER_SIZE: static buffer -> requires rebuild to change size
 * - ASCII_COMMAND_MAX_LENGTH: small command buffer -> can be overridden
 *   at runtime if the receiver supports dynamic allocation (not used here).
 */
#define UART_TX_BUFFER_SIZE ((size_t)512U)
#define ASCII_COMMAND_MAX_LENGTH ((size_t)128U)

// ASCII command delimiters
#define ASCII_COMMAND_DELIMITER '\r'
#define ASCII_PARAMETER_SEPARATOR ' '
#define ASCII_END_OF_LINE '\n'

#endif /* COMM_PROTOCOL_H */
