---
applyTo: "src/communication/uart_protocol.c"
description: "STM32H7 UART HAL implementation for command interface and debugging with ST-Link VCP"
---

# STM32H7 UART HAL Implementation for Command Interface

## Overview
This instruction file provides STM32H753ZI-specific UART HAL implementation guidance for command interface and debugging through ST-Link Virtual COM Port, enabling robust communication with host systems.

## Enhanced Documentation Integration
**Reference Assets**: This instruction leverages comprehensive UART documentation:
- **STM32H7 UART Documentation**: Complete UART peripheral coverage with VCP configuration
- **Copilot-Optimized UART Search**:
  ```bash
  # Search UART peripheral with token efficiency
  python3 scripts/search_enhanced_docs.py peripheral UART --scope STM32H7
  
  # Find UART HAL functions for interrupt-based communication
  python3 scripts/search_enhanced_docs.py function HAL_UART_Transmit_IT --scope STM32H7
  
  # Search UART protocol implementation concepts
  python3 scripts/search_enhanced_docs.py concept "uart_protocol" --scope STM32H7
  
  # Find virtual COM port configuration
  python3 scripts/search_enhanced_docs.py concept "virtual_com_port" --scope STM32H7
  ```
- **Cross-Protocol Integration**: Links UART with communication protocols and command interface systems
- **Token-Efficient Reference**: Semantic categorization reduces search overhead for protocol implementation

## Integration with Existing Instructions
- **Extends**: `comm-protocols.instructions.md` (UART protocol specification)
- **Extends**: `hardware-pins.instructions.md` (UART pin assignments)
- **References**: `ssot-config.instructions.md` (SSOT principles)
- **References**: `error-handling.instructions.md` (error management)
- **Documentation**: Based on enhanced search system with 161 STM32H7 UART documentation files

## STM32H753ZI UART Configuration

### UART Handle Configuration (from structUART__InitTypeDef.md)
```c
#include "config/hardware_config.h"
#include "config/comm_config.h"

UART_HandleTypeDef huart3;

// Configure USART3 for ST-Link Virtual COM Port
SystemError_t uart_init(void) {
    huart3.Instance = USART3;                     // ST-Link VCP uses USART3
    huart3.Init.BaudRate = UART_BAUDRATE;         // From SSOT comm_config.h
    huart3.Init.WordLength = UART_WORDLENGTH_8B;  // 8 data bits
    huart3.Init.StopBits = UART_STOPBITS_1;       // 1 stop bit
    huart3.Init.Parity = UART_PARITY_NONE;        // No parity
    huart3.Init.Mode = UART_MODE_TX_RX;           // Full duplex
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;  // No hardware flow control
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    
    // STM32H7-specific advanced features
    huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    
    if (HAL_UART_Init(&huart3) != HAL_OK) {
        return ERROR_UART_INIT_FAILED;
    }
    
    return SYSTEM_OK;
}
```

### Circular Buffer Implementation for Robust Communication
```c
// Circular buffer for UART RX data
typedef struct {
    uint8_t buffer[UART_RX_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} CircularBuffer_t;

CircularBuffer_t uart_rx_buffer = {0};
CircularBuffer_t uart_tx_buffer = {0};

// Add data to circular buffer
SystemError_t cb_put(CircularBuffer_t* cb, uint8_t data) {
    if (cb->count >= UART_RX_BUFFER_SIZE) {
        return ERROR_BUFFER_FULL;
    }
    
    cb->buffer[cb->head] = data;
    cb->head = (cb->head + 1) % UART_RX_BUFFER_SIZE;
    cb->count++;
    
    return SYSTEM_OK;
}

// Get data from circular buffer
SystemError_t cb_get(CircularBuffer_t* cb, uint8_t* data) {
    if (cb->count == 0) {
        return ERROR_BUFFER_EMPTY;
    }
    
    *data = cb->buffer[cb->tail];
    cb->tail = (cb->tail + 1) % UART_RX_BUFFER_SIZE;
    cb->count--;
    
    return SYSTEM_OK;
}
```

### Interrupt-Based UART with DMA
```c
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart3_tx;

uint8_t uart_rx_dma_buffer[UART_RX_BUFFER_SIZE];
volatile uint16_t uart_rx_dma_head = 0;

// Configure DMA for UART
void uart_configure_dma(void) {
    __HAL_RCC_DMA1_CLK_ENABLE();
    
    // USART3 RX DMA configuration
    hdma_usart3_rx.Instance = DMA1_Stream1;
    hdma_usart3_rx.Init.Request = DMA_REQUEST_USART3_RX;
    hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_rx.Init.Mode = DMA_CIRCULAR;              // Circular mode for continuous RX
    hdma_usart3_rx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_usart3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    
    HAL_DMA_Init(&hdma_usart3_rx);
    __HAL_LINKDMA(&huart3, hdmarx, hdma_usart3_rx);
    
    // USART3 TX DMA configuration
    hdma_usart3_tx.Instance = DMA1_Stream3;
    hdma_usart3_tx.Init.Request = DMA_REQUEST_USART3_TX;
    hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_tx.Init.Mode = DMA_NORMAL;
    hdma_usart3_tx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_usart3_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    
    HAL_DMA_Init(&hdma_usart3_tx);
    __HAL_LINKDMA(&huart3, hdmatx, hdma_usart3_tx);
}

// Start DMA-based UART reception
SystemError_t uart_start_dma_reception(void) {
    if (HAL_UART_Receive_DMA(&huart3, uart_rx_dma_buffer, UART_RX_BUFFER_SIZE) != HAL_OK) {
        return ERROR_UART_DMA_START_FAILED;
    }
    
    return SYSTEM_OK;
}
```

## Command Processing Implementation

### Command Parser State Machine
```c
typedef enum {
    UART_PARSE_IDLE,
    UART_PARSE_COMMAND,
    UART_PARSE_PARAMETERS,
    UART_PARSE_COMPLETE
} UartParseState_t;

typedef struct {
    char command_buffer[UART_CMD_MAX_LENGTH];
    uint16_t buffer_index;
    UartParseState_t state;
    uint32_t last_char_time;
} UartParser_t;

UartParser_t uart_parser = {0};

// Process received characters
void uart_process_rx_data(void) {
    uint16_t dma_head = UART_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart3_rx);
    
    while (uart_rx_dma_head != dma_head) {
        uint8_t received_char = uart_rx_dma_buffer[uart_rx_dma_head];
        uart_rx_dma_head = (uart_rx_dma_head + 1) % UART_RX_BUFFER_SIZE;
        
        uart_parse_character(received_char);
    }
}

// Parse individual characters
void uart_parse_character(uint8_t c) {
    uint32_t current_time = HAL_GetTick();
    
    // Timeout check for incomplete commands
    if (current_time - uart_parser.last_char_time > UART_CMD_TIMEOUT_MS) {
        uart_parser.state = UART_PARSE_IDLE;
        uart_parser.buffer_index = 0;
    }
    
    uart_parser.last_char_time = current_time;
    
    switch (uart_parser.state) {
        case UART_PARSE_IDLE:
            if (c != '\r' && c != '\n' && c != ' ') {
                uart_parser.command_buffer[0] = c;
                uart_parser.buffer_index = 1;
                uart_parser.state = UART_PARSE_COMMAND;
            }
            break;
            
        case UART_PARSE_COMMAND:
        case UART_PARSE_PARAMETERS:
            if (c == '\r' || c == '\n') {
                uart_parser.command_buffer[uart_parser.buffer_index] = '\0';
                uart_parser.state = UART_PARSE_COMPLETE;
                uart_execute_command(uart_parser.command_buffer);
                uart_parser.buffer_index = 0;
                uart_parser.state = UART_PARSE_IDLE;
            } else if (uart_parser.buffer_index < UART_CMD_MAX_LENGTH - 1) {
                uart_parser.command_buffer[uart_parser.buffer_index] = c;
                uart_parser.buffer_index++;
                uart_parser.state = UART_PARSE_PARAMETERS;
            } else {
                // Command too long - reset
                uart_send_response("ERROR: Command too long\r\n");
                uart_parser.buffer_index = 0;
                uart_parser.state = UART_PARSE_IDLE;
            }
            break;
            
        default:
            uart_parser.state = UART_PARSE_IDLE;
            break;
    }
}
```

### Command Execution Engine
```c
// Command execution with comprehensive error handling
void uart_execute_command(const char* command) {
    char cmd_copy[UART_CMD_MAX_LENGTH];
    strncpy(cmd_copy, command, UART_CMD_MAX_LENGTH - 1);
    cmd_copy[UART_CMD_MAX_LENGTH - 1] = '\0';
    
    // Tokenize command
    char* token = strtok(cmd_copy, " ");
    if (token == NULL) {
        uart_send_response("ERROR: Empty command\r\n");
        return;
    }
    
    // Convert to uppercase for case-insensitive commands
    for (int i = 0; token[i]; i++) {
        token[i] = toupper(token[i]);
    }
    
    // Execute commands based on protocol specification
    if (strcmp(token, "MOVE") == 0) {
        uart_cmd_motor_move();
    } else if (strcmp(token, "STOP") == 0) {
        uart_cmd_motor_stop();
    } else if (strcmp(token, "HOME") == 0) {
        uart_cmd_motor_home();
    } else if (strcmp(token, "READ") == 0) {
        uart_cmd_encoder_read();
    } else if (strcmp(token, "STATUS") == 0) {
        uart_cmd_system_status();
    } else if (strcmp(token, "RESET") == 0) {
        uart_cmd_system_reset();
    } else {
        uart_send_response("ERROR: Unknown command\r\n");
    }
}

// Motor move command implementation
void uart_cmd_motor_move(void) {
    char* motor_str = strtok(NULL, " ");
    char* steps_str = strtok(NULL, " ");
    
    if (motor_str == NULL || steps_str == NULL) {
        uart_send_response("ERROR: MOVE requires motor_id and steps\r\n");
        return;
    }
    
    uint8_t motor_id = atoi(motor_str);
    int32_t steps = atol(steps_str);
    
    if (motor_id < 1 || motor_id > MAX_MOTORS) {
        uart_send_response("ERROR: Invalid motor ID\r\n");
        return;
    }
    
    SystemError_t result = motor_move_relative(motor_id, steps);
    if (result == SYSTEM_OK) {
        uart_send_response("OK\r\n");
    } else {
        char error_msg[64];
        snprintf(error_msg, sizeof(error_msg), "ERROR: Motor move failed (%d)\r\n", result);
        uart_send_response(error_msg);
    }
}
```

### Robust Transmission Functions
```c
// Safe UART transmission with timeout and error handling
SystemError_t uart_send_response(const char* response) {
    size_t length = strlen(response);
    
    if (length > UART_TX_BUFFER_SIZE) {
        return ERROR_UART_MESSAGE_TOO_LONG;
    }
    
    // Feed watchdog before transmission
    HAL_IWDG_Refresh(&hiwdg);
    
    HAL_StatusTypeDef result = HAL_UART_Transmit(&huart3, (uint8_t*)response, length, UART_TX_TIMEOUT_MS);
    
    if (result != HAL_OK) {
        return ERROR_UART_TRANSMISSION_FAILED;
    }
    
    return SYSTEM_OK;
}

// Formatted response transmission
SystemError_t uart_send_formatted(const char* format, ...) {
    char buffer[UART_TX_BUFFER_SIZE];
    va_list args;
    
    va_start(args, format);
    int length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (length < 0 || length >= sizeof(buffer)) {
        return ERROR_UART_FORMAT_ERROR;
    }
    
    return uart_send_response(buffer);
}

// System status reporting
void uart_cmd_system_status(void) {
    SystemStatus_t status;
    get_system_status(&status);  // From system management module
    
    uart_send_formatted("STATUS:\r\n");
    uart_send_formatted("  Uptime: %lu ms\r\n", HAL_GetTick());
    uart_send_formatted("  Motor1: %s\r\n", status.motor1_enabled ? "ENABLED" : "DISABLED");
    uart_send_formatted("  Motor2: %s\r\n", status.motor2_enabled ? "ENABLED" : "DISABLED");
    uart_send_formatted("  Encoder1: %d\r\n", status.encoder1_angle);
    uart_send_formatted("  Encoder2: %d\r\n", status.encoder2_angle);
    uart_send_formatted("  Errors: %d\r\n", status.error_count);
    uart_send_response("OK\r\n");
}
```

## Integration with Safety Systems

### UART Communication Monitoring
```c
#include "config/safety_config.h"

typedef struct {
    uint32_t commands_received;
    uint32_t commands_executed;
    uint32_t parse_errors;
    uint32_t last_command_time;
    uint8_t communication_active;
} UartHealth_t;

UartHealth_t uart_health = {0};

// Monitor UART communication health
void uart_health_monitor(void) {
    uint32_t current_time = HAL_GetTick();
    
    // Check for communication timeout
    if (current_time - uart_health.last_command_time > UART_COMM_TIMEOUT_MS) {
        uart_health.communication_active = 0;
    } else {
        uart_health.communication_active = 1;
    }
    
    // Check error rate
    if (uart_health.commands_received > 0) {
        uint32_t error_rate = (uart_health.parse_errors * 100) / uart_health.commands_received;
        if (error_rate > UART_MAX_ERROR_RATE_PERCENT) {
            // High error rate detected
            log_system_event(EVENT_UART_HIGH_ERROR_RATE, error_rate);
        }
    }
}

// Safe command execution with watchdog
void uart_safe_execute_command(const char* command) {
    HAL_IWDG_Refresh(&hiwdg);  // Feed watchdog
    
    uart_health.commands_received++;
    uart_health.last_command_time = HAL_GetTick();
    
    uart_execute_command(command);
    uart_health.commands_executed++;
}
```

### Error Recovery and Diagnostics
```c
// UART error recovery
SystemError_t uart_error_recovery(void) {
    uint32_t error_flags = huart3.ErrorCode;
    
    if (error_flags != HAL_UART_ERROR_NONE) {
        // Log errors
        log_uart_error(error_flags, HAL_GetTick());
        
        // Handle specific errors
        if (error_flags & HAL_UART_ERROR_PE) {
            // Parity error
            uart_health.parse_errors++;
        }
        
        if (error_flags & HAL_UART_ERROR_FE) {
            // Frame error - possibly baud rate mismatch
            uart_health.parse_errors++;
        }
        
        if (error_flags & HAL_UART_ERROR_ORE) {
            // Overrun error - data loss
            uart_health.parse_errors++;
            
            // Reset UART to clear overrun
            HAL_UART_DeInit(&huart3);
            HAL_Delay(10);
            uart_init();
            uart_start_dma_reception();
        }
        
        // Clear error flags
        __HAL_UART_CLEAR_FLAG(&huart3, UART_CLEAR_PEF | UART_CLEAR_FEF | UART_CLEAR_NEF | UART_CLEAR_OREF);
        
        return ERROR_UART_RECOVERY_ATTEMPTED;
    }
    
    return SYSTEM_OK;
}
```

## Performance Optimization

### High-Frequency Data Logging
```c
// Efficient data logging for real-time monitoring
typedef struct {
    uint32_t timestamp;
    uint16_t motor1_position;
    uint16_t motor2_position;
    uint16_t encoder1_angle;
    uint16_t encoder2_angle;
    uint8_t system_status;
} DataLogEntry_t;

#define LOG_BUFFER_SIZE 100
DataLogEntry_t log_buffer[LOG_BUFFER_SIZE];
uint16_t log_index = 0;
uint8_t log_enabled = 0;

// Fast binary data transmission
void uart_send_data_log(void) {
    if (log_index > 0) {
        // Send binary data with header
        uint8_t header[] = {'D', 'A', 'T', 'A', (uint8_t)(log_index & 0xFF), (uint8_t)(log_index >> 8)};
        HAL_UART_Transmit_DMA(&huart3, header, sizeof(header));
        
        // Wait for header transmission
        while (HAL_UART_GetState(&huart3) != HAL_UART_STATE_READY);
        
        // Send log data
        HAL_UART_Transmit_DMA(&huart3, (uint8_t*)log_buffer, log_index * sizeof(DataLogEntry_t));
        
        log_index = 0;  // Reset buffer
    }
}
```

## Documentation References
Based on STM32H753ZI documentation analysis (161 UART-related files):

**Key Reference Files:**
- `group__UART__Exported__Functions__Group1.md` (52.3KB) - Primary UART functions
- `structUART__InitTypeDef.md` (18.7KB) - UART initialization structure
- `group__UART__Exported__Functions__Group2.md` (89.4KB) - UART communication functions
- `group__IRDA__Exported__Functions__Group2.md` (98.0KB) - IrDA protocol functions
- `group__DMA__Exported__Functions__Group2.md` (60.8KB) - DMA integration

**Total Documentation Coverage**: 319.2KB of focused UART implementation guidance from STM32H753ZI-specific HAL documentation.

## STM32H753ZI UART Features
- **FIFO Support**: 8-byte TX/RX FIFOs for improved performance
- **Oversampling**: 16x or 8x oversampling for noise immunity
- **Auto Baud Rate**: Automatic baud rate detection (if needed)
- **DMA Support**: Full DMA integration for efficient transfers
- **Multi-Protocol**: UART, USART, IrDA, SmartCard support
- **Advanced Features**: Hardware flow control, LIN mode, RS485 support
