---
applyTo: "src/config/comm_config.h"
description: "Communication protocol specifications for UART, CAN, Ethernet, I2C, and SPI interfaces"
---

# Communication Protocol Configuration Instructions

## Overview
This instruction file provides comprehensive guidance for configuring all communication interfaces in the STM32H753ZI stepper motor control project.

## Enhanced Documentation Integration
**Reference Assets**: This instruction leverages comprehensive STM32H7 communication documentation:
- **STM32H7 HAL Documentation**: 86MB with complete peripheral coverage including UART, SPI, I2C, CAN, Ethernet
- **Search Tools**: Use `python3 scripts/documentation/search_docs.py peripheral <PROTOCOL>` for specific implementation details
- **Protocol Documentation**: See `00_reference/stm32h753xx_User_Manual_md/group__<PROTOCOL>__Exported__Functions.md` for HAL functions
- **Communication Examples**: `00_reference/STM32Cube_FW_H7_V1.12.0/Projects/` contains working communication examples

## UART Protocol Specification

### Virtual COM Port Configuration
Based on STM32H753ZI Nucleo ST-Link VCP capabilities:

```c
// UART Physical Configuration (SSOT Reference)
#define UART_INSTANCE           USART3
#define UART_BAUDRATE           115200          // Standard rate for reliability
#define UART_WORDLENGTH         UART_WORDLENGTH_8B
#define UART_STOPBITS           UART_STOPBITS_1
#define UART_PARITY             UART_PARITY_NONE
#define UART_HWCONTROL          UART_HWCONTROL_NONE
#define UART_MODE               UART_MODE_TX_RX

// Buffer Configuration
#define UART_TX_BUFFER_SIZE     256             // Transmit buffer size
#define UART_RX_BUFFER_SIZE     128             // Receive buffer size
#define UART_CMD_MAX_LENGTH     64              // Maximum command length
```

### Command Protocol Format
```c
// Command Structure: CMD PARAM1 PARAM2 ... PARAMN\r\n
// Response Structure: STATUS DATA\r\n or ERROR CODE\r\n

// Command Categories
typedef enum {
    CMD_MOTOR = 0,      // Motor control commands
    CMD_ENCODER,        // Encoder reading commands  
    CMD_SYSTEM,         // System status/config commands
    CMD_DEBUG           // Debug and diagnostic commands
} UartCommandCategory_t;

// Standard Commands
#define UART_CMD_MOTOR_MOVE     "MOVE"         // MOVE motor_id steps
#define UART_CMD_MOTOR_STOP     "STOP"         // STOP motor_id
#define UART_CMD_MOTOR_HOME     "HOME"         // HOME motor_id
#define UART_CMD_ENCODER_READ   "READ"         // READ encoder_id
#define UART_CMD_SYSTEM_STATUS  "STATUS"       // STATUS
#define UART_CMD_SYSTEM_RESET   "RESET"        // RESET
```

### Protocol Implementation Guidelines
```c
// Command Parser State Machine
typedef enum {
    UART_STATE_IDLE,
    UART_STATE_RECEIVING,
    UART_STATE_PROCESSING,
    UART_STATE_RESPONDING
} UartState_t;

// Response Codes
#define UART_RESP_OK            "OK"
#define UART_RESP_ERROR         "ERROR"
#define UART_RESP_INVALID       "INVALID"
#define UART_RESP_BUSY          "BUSY"

// Example Command Processing
void uart_process_command(char* command) {
    if (strncmp(command, UART_CMD_MOTOR_MOVE, strlen(UART_CMD_MOTOR_MOVE)) == 0) {
        // Parse: MOVE 1 1000 (move motor 1 by 1000 steps)
        uint8_t motor_id;
        int32_t steps;
        if (sscanf(command, "MOVE %hhu %ld", &motor_id, &steps) == 2) {
            if (motor_move(motor_id, steps) == SYSTEM_OK) {
                uart_send_response(UART_RESP_OK);
            } else {
                uart_send_response(UART_RESP_ERROR);
            }
        } else {
            uart_send_response(UART_RESP_INVALID);
        }
    }
}
```

## CAN-FD Protocol Specification

### Physical Layer Configuration
```c
// CAN-FD Configuration (SSOT Reference)
#define CAN_INSTANCE            FDCAN1
#define CAN_BITRATE_KBPS        500             // Nominal bit rate
#define CAN_DATA_BITRATE_KBPS   2000            // Data phase bit rate (CAN-FD)
#define CAN_SAMPLE_POINT        87              // Sample point percentage
#define CAN_SJW                 1               // Synchronization jump width

// Message Configuration
#define CAN_MAX_DATA_BYTES      64              // CAN-FD maximum data length
#define CAN_TIMEOUT_MS          100             // Transmission timeout
```

### Message ID Allocation Strategy
```c
// CAN Message ID Allocation (11-bit standard IDs)
// Format: [Priority:3][Node_ID:3][Message_Type:5]

// Priority Levels
#define CAN_PRIORITY_EMERGENCY  0x0             // Highest priority
#define CAN_PRIORITY_HIGH       0x1             // High priority  
#define CAN_PRIORITY_NORMAL     0x2             // Normal priority
#define CAN_PRIORITY_LOW        0x3             // Low priority

// Node IDs
#define CAN_NODE_MASTER         0x0             // Master controller
#define CAN_NODE_MOTOR_CTRL     0x1             // This motor controller
#define CAN_NODE_SENSOR_BOARD   0x2             // Sensor interface board
#define CAN_NODE_UI_BOARD       0x3             // User interface board

// Message Types
#define CAN_MSG_HEARTBEAT       0x00            // Periodic heartbeat
#define CAN_MSG_MOTOR_CMD       0x01            // Motor command
#define CAN_MSG_MOTOR_STATUS    0x02            // Motor status report
#define CAN_MSG_ENCODER_DATA    0x03            // Encoder data
#define CAN_MSG_FAULT_REPORT    0x04            // Fault notification
#define CAN_MSG_CONFIG_REQ      0x05            // Configuration request
#define CAN_MSG_CONFIG_RESP     0x06            // Configuration response

// Complete Message ID Calculation
#define CAN_ID(priority, node, type) \
    (((priority & 0x7) << 8) | ((node & 0x7) << 5) | (type & 0x1F))

// Specific Message IDs for This Node
#define CAN_ID_HEARTBEAT        CAN_ID(CAN_PRIORITY_NORMAL, CAN_NODE_MOTOR_CTRL, CAN_MSG_HEARTBEAT)
#define CAN_ID_MOTOR1_STATUS    CAN_ID(CAN_PRIORITY_NORMAL, CAN_NODE_MOTOR_CTRL, CAN_MSG_MOTOR_STATUS)
#define CAN_ID_MOTOR2_STATUS    (CAN_ID_MOTOR1_STATUS + 1)
#define CAN_ID_FAULT_REPORT     CAN_ID(CAN_PRIORITY_HIGH, CAN_NODE_MOTOR_CTRL, CAN_MSG_FAULT_REPORT)
```

### Message Format Specifications
```c
// Heartbeat Message (8 bytes)
typedef struct __attribute__((packed)) {
    uint32_t system_uptime_ms;      // System uptime in milliseconds
    uint16_t system_status;         // System status bitmap
    uint8_t motor_states;           // Motor states (4 bits each)
    uint8_t fault_count;            // Number of active faults
} CAN_HeartbeatMsg_t;

// Motor Status Message (32 bytes - CAN-FD)
typedef struct __attribute__((packed)) {
    uint8_t motor_id;               // Motor identifier
    uint8_t motor_state;            // Current motor state
    int32_t current_position;       // Current position (encoder counts)
    int32_t target_position;        // Target position
    float current_velocity;         // Current velocity (deg/s)
    float motor_current;            // Motor current (A)
    uint16_t fault_flags;           // Active fault flags
    uint32_t timestamp_ms;          // Message timestamp
    uint8_t reserved[6];            // Reserved for future expansion
} CAN_MotorStatusMsg_t;

// Motor Command Message (16 bytes)
typedef struct __attribute__((packed)) {
    uint8_t motor_id;               // Target motor ID
    uint8_t command_type;           // Command type
    int32_t position_target;        // Position target (if applicable)
    float velocity_target;          // Velocity target (if applicable)
    uint16_t command_flags;         // Command flags
    uint32_t command_id;            // Unique command identifier
} CAN_MotorCommandMsg_t;
```

## Ethernet/TCP Protocol Specification

### Network Layer Configuration
```c
// Ethernet Configuration (SSOT Reference)
#define ETH_STATIC_IP           "192.168.1.50"
#define ETH_NETMASK             "255.255.255.0"
#define ETH_GATEWAY             "192.168.1.1"
#define ETH_DNS_SERVER          "8.8.8.8"

// TCP Server Configuration
#define TCP_SERVER_PORT         8000            // Command server port
#define TCP_MAX_CONNECTIONS     2               // Maximum concurrent connections
#define TCP_BUFFER_SIZE         1024            // TCP buffer size
#define TCP_TIMEOUT_MS          5000            // Connection timeout
```

### Application Protocol (JSON-based)
```c
// JSON Command Format
// Request:  {"cmd": "motor_move", "motor": 1, "steps": 1000, "id": 12345}
// Response: {"status": "ok", "id": 12345} or {"status": "error", "error": "message", "id": 12345}

// Command Processing
typedef struct {
    char command[32];               // Command name
    uint8_t motor_id;               // Motor ID (if applicable)
    int32_t param1;                 // First parameter
    float param2;                   // Second parameter
    uint32_t command_id;            // Unique command ID
} TCPCommand_t;

// Response Structure
typedef struct {
    char status[16];                // "ok" or "error"
    char message[128];              // Response message or error description
    uint32_t command_id;            // Matching command ID
    float data[4];                  // Response data (if applicable)
} TCPResponse_t;
```

## I2C Protocol Specification (AS5600 Encoders)

### Physical Configuration
```c
// I2C Configuration for AS5600 Encoders (SSOT Reference)
#define I2C_CLOCK_SPEED         400000          // 400kHz Fast Mode
#define I2C_TIMEOUT_MS          100             // Transaction timeout
#define I2C_RETRY_COUNT         3               // Number of retry attempts

// AS5600 Register Addresses (from datasheet)
#define AS5600_REG_ZMCO         0x00            // Zero position MSB
#define AS5600_REG_ZPOS_H       0x01            // Zero position MSB
#define AS5600_REG_ZPOS_L       0x02            // Zero position LSB
#define AS5600_REG_MPOS_H       0x03            // Maximum position MSB
#define AS5600_REG_MPOS_L       0x04            // Maximum position LSB
#define AS5600_REG_MANG_H       0x05            // Maximum angle MSB
#define AS5600_REG_MANG_L       0x06            // Maximum angle LSB
#define AS5600_REG_CONF_H       0x07            // Configuration MSB
#define AS5600_REG_CONF_L       0x08            // Configuration LSB
#define AS5600_REG_RAW_ANGLE_H  0x0C            // Raw angle MSB
#define AS5600_REG_RAW_ANGLE_L  0x0D            // Raw angle LSB
#define AS5600_REG_ANGLE_H      0x0E            // Processed angle MSB
#define AS5600_REG_ANGLE_L      0x0F            // Processed angle LSB
#define AS5600_REG_STATUS       0x0B            // Status register
#define AS5600_REG_AGC          0x1A            // Automatic gain control
#define AS5600_REG_MAGNITUDE_H  0x1B            // Magnitude MSB
#define AS5600_REG_MAGNITUDE_L  0x1C            // Magnitude LSB
```

### Read Protocol Implementation
```c
// AS5600 Angle Reading Function
float as5600_read_angle_degrees(I2C_HandleTypeDef *hi2c, uint8_t device_addr) {
    uint8_t data[2];
    uint16_t raw_angle;
    
    // Read angle registers (0x0E, 0x0F)
    if (HAL_I2C_Mem_Read(hi2c, device_addr, AS5600_REG_ANGLE_H, 
                         I2C_MEMADD_SIZE_8BIT, data, 2, I2C_TIMEOUT_MS) == HAL_OK) {
        raw_angle = (data[0] << 8) | data[1];
        return (float)raw_angle * 360.0f / 4096.0f;  // Convert to degrees
    }
    return -1.0f;  // Error value
}

// Status Checking
uint8_t as5600_check_magnet_status(I2C_HandleTypeDef *hi2c, uint8_t device_addr) {
    uint8_t status;
    
    if (HAL_I2C_Mem_Read(hi2c, device_addr, AS5600_REG_STATUS, 
                         I2C_MEMADD_SIZE_8BIT, &status, 1, I2C_TIMEOUT_MS) == HAL_OK) {
        return status & 0x38;  // Extract magnet status bits (MD, ML, MH)
    }
    return 0;
}
```

## SPI Protocol Specification (L6470 Motors)

### Daisy-Chain Protocol Implementation
```c
// SPI Configuration for L6470 Daisy-Chain (SSOT Reference)
#define L6470_SPI_MODE          3               // CPOL=1, CPHA=1
#define L6470_SPI_SPEED         5000000         // 5MHz maximum
#define L6470_SPI_TIMEOUT_MS    10              // Transaction timeout

// Daisy-Chain Transaction Structure
typedef struct {
    uint8_t motor1_cmd;         // Command for first L6470
    uint8_t motor1_data[3];     // Data bytes for first L6470
    uint8_t motor2_cmd;         // Command for second L6470  
    uint8_t motor2_data[3];     // Data bytes for second L6470
} L6470_DaisyTransaction_t;

// Daisy-Chain SPI Communication
void l6470_daisy_send(L6470_DaisyTransaction_t *transaction) {
    uint8_t tx_buffer[8];       // Maximum transaction size
    uint8_t rx_buffer[8];       // Response buffer
    
    // Prepare transmission buffer (motor1 data sent first)
    tx_buffer[0] = transaction->motor1_cmd;
    memcpy(&tx_buffer[1], transaction->motor1_data, 3);
    tx_buffer[4] = transaction->motor2_cmd;
    memcpy(&tx_buffer[5], transaction->motor2_data, 3);
    
    // Execute SPI transaction
    HAL_GPIO_WritePin(MOTOR_SPI_CS_PORT, MOTOR_SPI_CS_PIN, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi2, tx_buffer, rx_buffer, 8, L6470_SPI_TIMEOUT_MS);
    HAL_GPIO_WritePin(MOTOR_SPI_CS_PORT, MOTOR_SPI_CS_PIN, GPIO_PIN_SET);
    
    // Process responses if needed
}
```

## Communication Timing Requirements

### Real-Time Constraints
```c
// Communication Timing Budgets
#define UART_MAX_RESPONSE_TIME_MS       50      // UART command response time
#define CAN_HEARTBEAT_INTERVAL_MS       100     // CAN heartbeat period
#define I2C_ENCODER_READ_INTERVAL_MS    10      // Encoder reading frequency
#define SPI_MOTOR_CMD_MAX_TIME_MS       5       // Motor command execution time
#define TCP_KEEPALIVE_INTERVAL_MS       1000    // TCP connection keepalive

// Priority Levels for Communication
#define COMM_PRIORITY_EMERGENCY         0       // Emergency stop, faults
#define COMM_PRIORITY_CONTROL           1       // Motor control commands
#define COMM_PRIORITY_FEEDBACK          2       // Sensor feedback
#define COMM_PRIORITY_STATUS            3       // Status reporting
#define COMM_PRIORITY_DEBUG             4       // Debug information
```

### Error Handling and Recovery
```c
// Communication Error Types
typedef enum {
    COMM_ERROR_NONE = 0,
    COMM_ERROR_TIMEOUT,
    COMM_ERROR_CHECKSUM,
    COMM_ERROR_OVERRUN,
    COMM_ERROR_PROTOCOL,
    COMM_ERROR_BUSY
} CommError_t;

// Error Recovery Strategies
void comm_handle_error(CommInterface_t interface, CommError_t error) {
    switch (error) {
        case COMM_ERROR_TIMEOUT:
            // Retry with exponential backoff
            break;
        case COMM_ERROR_CHECKSUM:
            // Request retransmission
            break;
        case COMM_ERROR_OVERRUN:
            // Clear buffers and restart
            break;
        default:
            // Log error and continue
            break;
    }
}
```

Remember to validate all communication parameters against the respective peripheral specifications and ensure proper error handling for robust operation.
