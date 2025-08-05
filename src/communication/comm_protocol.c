/**
 * @file comm_protocol.c
 * @brief Communication Protocol Implementation - STM32H753ZI Stepper Motor Project
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 * 
 * @note Communication implementation following:
 * - .github/instructions/comms.instructions.md
 * - config/comm_config.h (SSOT)
 * 
 * @warning SAFETY-CRITICAL: All communication must integrate with safety system
 */

#include "comm_protocol.h"
#include "controllers/motor_controller.h"
#include "safety/fault_monitor.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ========================================================================== */
/* Private Variables and State Management                                    */
/* ========================================================================== */

static bool comm_protocol_initialized = false;

// Communication channel configurations
static CommChannelConfig_t comm_channels[7] = {0}; // One for each protocol type

// UART communication state
static UART_HandleTypeDef* debug_uart_handle = NULL;
static uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE] = {0};
static uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE] = {0};
static volatile bool uart_rx_complete = false;
static volatile bool uart_tx_complete = true;
static uint32_t uart_rx_length = 0;

// CAN communication state
static CAN_HandleTypeDef* can_handle = NULL;
static CAN_TxHeaderTypeDef can_tx_header = {0};
static CAN_RxHeaderTypeDef can_rx_header = {0};
static uint8_t can_tx_data[8] = {0};
static uint8_t can_rx_data[8] = {0};
static uint32_t can_tx_mailbox = 0;

// Message processing
static char ascii_command_buffer[ASCII_COMMAND_MAX_LENGTH] = {0};
static uint32_t ascii_command_length = 0;

/* ========================================================================== */
/* Private Function Prototypes                                               */
/* ========================================================================== */

static SystemError_t process_motor_command(const MotorCommand_t* command);
static SystemError_t validate_motor_command(const MotorCommand_t* command);
static uint16_t calculate_message_checksum(const MessageHeader_t* header, const uint8_t* payload);
static SystemError_t send_uart_message(const uint8_t* data, uint32_t length);
static SystemError_t process_uart_received_data(void);

/* ========================================================================== */
/* Public API Implementation                                                 */
/* ========================================================================== */

/**
 * @brief Initialize communication protocol system
 */
SystemError_t comm_protocol_init(void) {
    if (comm_protocol_initialized) {
        return ERROR_ALREADY_INITIALIZED;
    }
    
    // SAFETY-CRITICAL: Check safety system before communication init
    COMM_SAFETY_CHECK();
    
    // Initialize communication channel configurations
    for (int i = 0; i < 7; i++) {
        comm_channels[i].protocol = (CommProtocol_t)i;
        comm_channels[i].enabled = false;
        comm_channels[i].baudrate = 0;
        comm_channels[i].timeout_ms = COMM_DEFAULT_TIMEOUT_MS;
        comm_channels[i].tx_count = 0;
        comm_channels[i].rx_count = 0;
        comm_channels[i].error_count = 0;
        comm_channels[i].last_activity = HAL_GetTick();
    }
    
    // Initialize UART buffers
    memset(uart_rx_buffer, 0, sizeof(uart_rx_buffer));
    memset(uart_tx_buffer, 0, sizeof(uart_tx_buffer));
    uart_rx_complete = false;
    uart_tx_complete = true;
    uart_rx_length = 0;
    
    // Initialize ASCII command processing
    memset(ascii_command_buffer, 0, sizeof(ascii_command_buffer));
    ascii_command_length = 0;
    
    comm_protocol_initialized = true;
    
    // Log initialization success
    safety_log_event(SAFETY_EVENT_SYSTEM_INIT, 0xFF, (uint32_t)SYSTEM_OK);
    
    return SYSTEM_OK;
}

/**
 * @brief Initialize UART communication channel
 */
SystemError_t comm_uart_init(UART_HandleTypeDef* huart, CommProtocol_t protocol) {
    if (!comm_protocol_initialized || huart == NULL) {
        return ERROR_NOT_INITIALIZED;
    }
    
    if (protocol != PROTOCOL_UART_ASCII && protocol != PROTOCOL_UART_BINARY) {
        return ERROR_INVALID_PARAMETER;
    }
    
    COMM_SAFETY_CHECK();
    
    // Store UART handle
    debug_uart_handle = huart;
    
    // Configure channel
    CommChannelConfig_t* channel = &comm_channels[protocol];
    channel->enabled = true;
    channel->baudrate = DEBUG_UART_BAUDRATE;
    channel->timeout_ms = UART_TIMEOUT_MS;
    channel->last_activity = HAL_GetTick();
    
    // Start UART DMA reception
    if (HAL_UART_Receive_DMA(huart, uart_rx_buffer, UART_RX_BUFFER_SIZE) != HAL_OK) {
        fault_monitor_record_system_fault(SYSTEM_FAULT_UART_FAULT, 
                                          FAULT_SEVERITY_ERROR, 0);
        return ERROR_COMM_DMA_FAILED;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Initialize debug UART for command interface
 */
SystemError_t debug_uart_init(UART_HandleTypeDef* huart) {
    if (huart == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    
    COMM_SAFETY_CHECK();
    
    // Configure UART parameters
    huart->Init.BaudRate = DEBUG_UART_BAUDRATE;
    huart->Init.WordLength = UART_WORDLENGTH_8B;
    huart->Init.StopBits = UART_STOPBITS_1;
    huart->Init.Parity = UART_PARITY_NONE;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;
    huart->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    
    if (HAL_UART_Init(huart) != HAL_OK) {
        fault_monitor_record_system_fault(SYSTEM_FAULT_UART_FAULT, 
                                          FAULT_SEVERITY_CRITICAL, 1);
        return ERROR_COMM_INIT_FAILED;
    }
    
    // Initialize UART communication channel
    return comm_uart_init(huart, PROTOCOL_UART_ASCII);
}

/**
 * @brief Initialize CAN communication channel
 */
SystemError_t comm_can_init(CAN_HandleTypeDef* hcan) {
    if (!comm_protocol_initialized || hcan == NULL) {
        return ERROR_NOT_INITIALIZED;
    }
    
    COMM_SAFETY_CHECK();
    
    // Store CAN handle
    can_handle = hcan;
    
    // Configure CAN TX header template
    can_tx_header.StdId = CAN_MOTOR_BASE_ID;
    can_tx_header.ExtId = 0;
    can_tx_header.RTR = CAN_RTR_DATA;
    can_tx_header.IDE = CAN_ID_STD;
    can_tx_header.DLC = 8;
    can_tx_header.TransmitGlobalTime = DISABLE;
    
    // Configure channel
    CommChannelConfig_t* channel = &comm_channels[PROTOCOL_CAN_MOTOR];
    channel->enabled = true;
    channel->baudrate = CAN_BITRATE_KBPS * 1000;
    channel->timeout_ms = CAN_TIMEOUT_MS;
    channel->last_activity = HAL_GetTick();
    
    // Start CAN
    if (HAL_CAN_Start(hcan) != HAL_OK) {
        fault_monitor_record_system_fault(SYSTEM_FAULT_CAN_FAULT, 
                                          FAULT_SEVERITY_CRITICAL, 0);
        return ERROR_COMM_INIT_FAILED;
    }
    
    // Activate CAN RX notification
    if (HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        fault_monitor_record_system_fault(SYSTEM_FAULT_CAN_FAULT, 
                                          FAULT_SEVERITY_ERROR, 1);
        return ERROR_COMM_INIT_FAILED;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Process received message
 */
SystemError_t comm_process_message(const Message_t* message) {
    if (!comm_protocol_initialized || message == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    
    COMM_SAFETY_CHECK();
    
    // Validate message header
    VALIDATE_MESSAGE_HEADER(&message->header);
    
    // Verify checksum
    uint16_t calculated_checksum = calculate_message_checksum(&message->header, message->payload);
    if (calculated_checksum != message->header.checksum) {
        fault_monitor_record_system_fault(SYSTEM_FAULT_COMM_ERROR, 
                                          FAULT_SEVERITY_WARNING, calculated_checksum);
        return ERROR_COMM_CHECKSUM_FAILED;
    }
    
    // Process based on protocol type
    switch (message->header.protocol_type) {
        case PROTOCOL_UART_BINARY: {
            if (message->header.payload_length >= sizeof(MotorCommand_t)) {
                MotorCommand_t* command = (MotorCommand_t*)message->payload;
                return process_motor_command(command);
            }
            break;
        }
        
        case PROTOCOL_CAN_MOTOR: {
            if (message->header.payload_length >= sizeof(MotorCommand_t)) {
                MotorCommand_t* command = (MotorCommand_t*)message->payload;
                return process_motor_command(command);
            }
            break;
        }
        
        default:
            return ERROR_COMM_UNSUPPORTED_PROTOCOL;
    }
    
    return ERROR_COMM_INVALID_MESSAGE;
}

/**
 * @brief Process UART ASCII command
 */
SystemError_t comm_process_ascii_command(const char* command_string) {
    if (!comm_protocol_initialized || command_string == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    
    COMM_SAFETY_CHECK();
    
    MotorCommand_t motor_command = {0};
    SystemError_t result = comm_parse_ascii_motor_command(command_string, &motor_command);
    
    if (result == SYSTEM_OK) {
        result = process_motor_command(&motor_command);
        
        // Send response
        char response[128];
        if (result == SYSTEM_OK) {
            snprintf(response, sizeof(response), "OK: Command executed\r\n");
        } else {
            snprintf(response, sizeof(response), "ERROR: %d\r\n", result);
        }
        
        comm_send_text_message(PROTOCOL_UART_ASCII, response);
    }
    
    return result;
}

/**
 * @brief Parse motor command from ASCII string
 */
SystemError_t comm_parse_ascii_motor_command(const char* ascii_command, MotorCommand_t* motor_command) {
    if (ascii_command == NULL || motor_command == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    
    memset(motor_command, 0, sizeof(MotorCommand_t));
    
    // Example commands:
    // "MOVE 0 1000 500" - Move motor 0 to position 1000 at speed 500
    // "STOP 1" - Stop motor 1
    // "HOME 0" - Home motor 0
    // "STATUS 1" - Get status of motor 1
    
    char command[16], param1[16], param2[16], param3[16];
    int params = sscanf(ascii_command, "%15s %15s %15s %15s", command, param1, param2, param3);
    
    if (params < 2) {
        return ERROR_COMM_INVALID_COMMAND;
    }
    
    // Parse motor ID
    motor_command->motor_id = (uint8_t)atoi(param1);
    if (motor_command->motor_id >= MAX_MOTORS) {
        return ERROR_MOTOR_INVALID_ID;
    }
    
    // Parse command type
    if (strcmp(command, "MOVE") == 0) {
        if (params >= 4) {
            motor_command->command = MOTOR_CMD_MOVE_ABSOLUTE;
            motor_command->data.move.position_steps = atoi(param2);
            motor_command->data.move.speed_steps_per_sec = atoi(param3);
            motor_command->data.move.acceleration = MOTOR1_ACCELERATION; // Default from SSOT
        } else {
            return ERROR_COMM_INVALID_COMMAND;
        }
    } else if (strcmp(command, "STOP") == 0) {
        motor_command->command = MOTOR_CMD_STOP;
    } else if (strcmp(command, "ESTOP") == 0) {
        motor_command->command = MOTOR_CMD_EMERGENCY_STOP;
    } else if (strcmp(command, "HOME") == 0) {
        motor_command->command = MOTOR_CMD_HOME;
    } else if (strcmp(command, "STATUS") == 0) {
        motor_command->command = MOTOR_CMD_GET_STATUS;
    } else {
        return ERROR_COMM_INVALID_COMMAND;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Send text message via UART
 */
SystemError_t comm_send_text_message(CommProtocol_t protocol, const char* message) {
    if (!comm_protocol_initialized || message == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    
    if (protocol != PROTOCOL_UART_ASCII || debug_uart_handle == NULL) {
        return ERROR_COMM_UNSUPPORTED_PROTOCOL;
    }
    
    uint32_t length = strlen(message);
    if (length > UART_TX_BUFFER_SIZE) {
        length = UART_TX_BUFFER_SIZE;
    }
    
    return send_uart_message((const uint8_t*)message, length);
}

/**
 * @brief Periodic communication task
 */
SystemError_t comm_protocol_task(void) {
    if (!comm_protocol_initialized) {
        return ERROR_NOT_INITIALIZED;
    }
    
    SystemError_t result = SYSTEM_OK;
    
    // Process UART received data
    if (uart_rx_complete) {
        result = process_uart_received_data();
        uart_rx_complete = false;
    }
    
    // Check communication timeouts
    uint32_t current_time = HAL_GetTick();
    for (int i = 0; i < 7; i++) {
        CommChannelConfig_t* channel = &comm_channels[i];
        if (channel->enabled && 
            (current_time - channel->last_activity) > channel->timeout_ms) {
            
            fault_monitor_record_system_fault(SYSTEM_FAULT_COMM_TIMEOUT, 
                                              FAULT_SEVERITY_WARNING, i);
            channel->error_count++;
        }
    }
    
    return result;
}

/**
 * @brief Check communication channel health
 */
bool comm_channel_is_healthy(CommProtocol_t protocol) {
    if (!comm_protocol_initialized || protocol >= 7) {
        return false;
    }
    
    CommChannelConfig_t* channel = &comm_channels[protocol];
    
    // Check if channel is enabled and has recent activity
    uint32_t current_time = HAL_GetTick();
    bool recent_activity = (current_time - channel->last_activity) < (channel->timeout_ms * 2);
    bool low_error_rate = (channel->error_count * 10) < (channel->tx_count + channel->rx_count);
    
    return channel->enabled && (recent_activity || (channel->tx_count + channel->rx_count) == 0) && low_error_rate;
}

/**
 * @brief Get communication channel statistics
 */
SystemError_t comm_get_channel_config(CommProtocol_t protocol, CommChannelConfig_t* config) {
    if (!comm_protocol_initialized || config == NULL || protocol >= 7) {
        return ERROR_INVALID_PARAMETER;
    }
    
    *config = comm_channels[protocol];
    return SYSTEM_OK;
}

/**
 * @brief Perform communication system self-test
 */
SystemError_t comm_protocol_self_test(void) {
    if (!comm_protocol_initialized) {
        return ERROR_NOT_INITIALIZED;
    }
    
    // Test UART loopback if available
    if (debug_uart_handle != NULL && comm_channels[PROTOCOL_UART_ASCII].enabled) {
        const char* test_message = "TEST\r\n";
        SystemError_t result = comm_send_text_message(PROTOCOL_UART_ASCII, test_message);
        if (result != SYSTEM_OK) {
            return ERROR_COMM_SELF_TEST_FAILED;
        }
    }
    
    // Test message processing
    MotorCommand_t test_command = {
        .motor_id = 0,
        .command = MOTOR_CMD_GET_STATUS,
        .data.raw_data = 0
    };
    
    SystemError_t validation_result = validate_motor_command(&test_command);
    if (validation_result != SYSTEM_OK) {
        return ERROR_COMM_SELF_TEST_FAILED;
    }
    
    return SYSTEM_OK;
}

/* ========================================================================== */
/* UART Callback Functions                                                   */
/* ========================================================================== */

/**
 * @brief Process UART RX complete callback
 */
void comm_uart_rx_complete_callback(UART_HandleTypeDef* huart) {
    if (huart == debug_uart_handle) {
        uart_rx_length = UART_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
        uart_rx_complete = true;
        comm_channels[PROTOCOL_UART_ASCII].rx_count++;
        comm_channels[PROTOCOL_UART_ASCII].last_activity = HAL_GetTick();
        
        // Restart DMA reception
        HAL_UART_Receive_DMA(huart, uart_rx_buffer, UART_RX_BUFFER_SIZE);
    }
}

/**
 * @brief Process UART TX complete callback
 */
void comm_uart_tx_complete_callback(UART_HandleTypeDef* huart) {
    if (huart == debug_uart_handle) {
        uart_tx_complete = true;
        comm_channels[PROTOCOL_UART_ASCII].tx_count++;
        comm_channels[PROTOCOL_UART_ASCII].last_activity = HAL_GetTick();
    }
}

/**
 * @brief Process UART error callback
 */
void comm_uart_error_callback(UART_HandleTypeDef* huart) {
    if (huart == debug_uart_handle) {
        comm_channels[PROTOCOL_UART_ASCII].error_count++;
        fault_monitor_record_system_fault(SYSTEM_FAULT_UART_FAULT, 
                                          FAULT_SEVERITY_WARNING, huart->ErrorCode);
        
        // Restart UART reception
        HAL_UART_Receive_DMA(huart, uart_rx_buffer, UART_RX_BUFFER_SIZE);
    }
}

/* ========================================================================== */
/* Private Function Implementation                                           */
/* ========================================================================== */

/**
 * @brief Process motor command
 */
static SystemError_t process_motor_command(const MotorCommand_t* command) {
    if (command == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    
    SystemError_t result = validate_motor_command(command);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Execute command based on type
    switch (command->command) {
        case MOTOR_CMD_STOP:
            result = motor_controller_stop_motor(command->motor_id);
            break;
            
        case MOTOR_CMD_EMERGENCY_STOP:
            result = motor_controller_emergency_stop(command->motor_id);
            break;
            
        case MOTOR_CMD_MOVE_ABSOLUTE:
            // Convert steps to degrees for motor controller
            float target_deg = MOTOR_CONTROLLER_STEPS_TO_DEGREES(command->data.move.position_steps);
            result = motor_controller_move_to_position(command->motor_id, target_deg);
            break;
            
        case MOTOR_CMD_HOME:
            result = motor_controller_home_motor(command->motor_id);
            break;
            
        case MOTOR_CMD_GET_STATUS:
            // Status will be sent in response
            result = SYSTEM_OK;
            break;
            
        default:
            result = ERROR_COMM_UNSUPPORTED_COMMAND;
            break;
    }
    
    return result;
}

/**
 * @brief Validate motor command
 */
static SystemError_t validate_motor_command(const MotorCommand_t* command) {
    if (command == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    
    // Validate motor ID
    if (command->motor_id >= MAX_MOTORS) {
        return ERROR_MOTOR_INVALID_ID;
    }
    
    // Validate command-specific parameters
    switch (command->command) {
        case MOTOR_CMD_MOVE_ABSOLUTE:
        case MOTOR_CMD_MOVE_RELATIVE:
            // Validate position limits
            if (abs(command->data.move.position_steps) > MOTOR_MAX_POSITION_STEPS) {
                return ERROR_MOTOR_POSITION_LIMIT;
            }
            // Validate speed limits
            if (command->data.move.speed_steps_per_sec > MOTOR_MAX_SPEED_STEPS_PER_SEC) {
                return ERROR_MOTOR_SPEED_LIMIT;
            }
            break;
            
        case MOTOR_CMD_SET_PARAMETERS:
            // Validate parameter limits
            if (command->data.parameters.max_speed > MOTOR_MAX_SPEED_STEPS_PER_SEC ||
                command->data.parameters.current_ma > MOTOR_MAX_CURRENT_MA) {
                return ERROR_MOTOR_PARAMETER_INVALID;
            }
            break;
            
        default:
            // Other commands don't require additional validation
            break;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Calculate message checksum
 */
static uint16_t calculate_message_checksum(const MessageHeader_t* header, const uint8_t* payload) {
    // Simple CRC16 implementation
    uint16_t crc = 0xFFFF;
    
    // Include header in checksum (excluding checksum field itself)
    const uint8_t* data = (const uint8_t*)header;
    for (size_t i = 0; i < offsetof(MessageHeader_t, checksum); i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    // Include payload in checksum
    for (uint16_t i = 0; i < header->payload_length; i++) {
        crc ^= payload[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

/**
 * @brief Send UART message
 */
static SystemError_t send_uart_message(const uint8_t* data, uint32_t length) {
    if (debug_uart_handle == NULL || data == NULL || !uart_tx_complete) {
        return ERROR_COMM_BUSY;
    }
    
    uart_tx_complete = false;
    
    if (HAL_UART_Transmit_DMA(debug_uart_handle, (uint8_t*)data, length) != HAL_OK) {
        uart_tx_complete = true;
        return ERROR_COMM_SEND_FAILED;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Process UART received data
 */
static SystemError_t process_uart_received_data(void) {
    if (uart_rx_length == 0) {
        return SYSTEM_OK;
    }
    
    // Process received bytes looking for complete commands
    for (uint32_t i = 0; i < uart_rx_length; i++) {
        char received_char = (char)uart_rx_buffer[i];
        
        if (received_char == ASCII_COMMAND_DELIMITER || received_char == ASCII_END_OF_LINE) {
            // Complete command received
            if (ascii_command_length > 0) {
                ascii_command_buffer[ascii_command_length] = '\0';
                comm_process_ascii_command(ascii_command_buffer);
                ascii_command_length = 0;
            }
        } else if (ascii_command_length < (ASCII_COMMAND_MAX_LENGTH - 1)) {
            // Add character to command buffer
            ascii_command_buffer[ascii_command_length++] = received_char;
        } else {
            // Command too long, reset buffer
            ascii_command_length = 0;
        }
    }
    
    // Clear receive buffer
    memset(uart_rx_buffer, 0, uart_rx_length);
    uart_rx_length = 0;
    
    return SYSTEM_OK;
}
