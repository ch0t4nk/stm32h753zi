---
applyTo: "src/communication/**/*.{c,h}"
description: "Communication protocols, UART/CAN/SPI/I2C interfaces, and networking implementation for STM32H753ZI stepper motor project"
---

# Communication & Protocols Instructions

## Overview
This instruction file provides comprehensive guidance for communication protocol implementation in the STM32H753ZI stepper motor control system, including UART command interface, CAN-FD networking, SPI motor control, I2C sensor communication, and Ethernet connectivity.

## Protocol Architecture

### Communication Layer Structure
```c
// Communication protocol layers
typedef enum {
    COMM_LAYER_PHYSICAL,     // GPIO, hardware interfaces
    COMM_LAYER_DATALINK,     // SPI, I2C, UART framing
    COMM_LAYER_NETWORK,      // CAN, Ethernet protocols
    COMM_LAYER_TRANSPORT,    // Message segmentation, flow control
    COMM_LAYER_APPLICATION   // Motor control commands, status
} CommLayer_t;

// Protocol types supported
typedef enum {
    PROTOCOL_UART_ASCII,     // Human-readable ASCII commands
    PROTOCOL_UART_BINARY,    // Binary protocol for efficiency
    PROTOCOL_CAN_MOTOR,      // CAN motor control protocol
    PROTOCOL_SPI_L6470,      // L6470 stepper driver protocol
    PROTOCOL_I2C_AS5600,     // AS5600 encoder protocol
    PROTOCOL_ETHERNET_UDP,   // UDP for real-time data
    PROTOCOL_ETHERNET_TCP    // TCP for reliable commands
} CommProtocol_t;
```

### Message Framework
```c
// Universal message header
typedef struct {
    uint32_t magic;          // 0xDEADBEEF for validation
    uint16_t message_id;     // Unique message identifier
    uint16_t payload_length; // Payload size in bytes
    uint8_t protocol_type;   // CommProtocol_t value
    uint8_t priority;        // Message priority (0=highest)
    uint16_t checksum;       // CRC16 checksum
    uint32_t timestamp_ms;   // Message timestamp
} __attribute__((packed)) MessageHeader_t;

// Complete message structure
typedef struct {
    MessageHeader_t header;
    uint8_t payload[MAX_MESSAGE_PAYLOAD];
} Message_t;

#define MAX_MESSAGE_PAYLOAD 256
#define MESSAGE_MAGIC 0xDEADBEEF
```

## UART Protocol Implementation

### STM32H7 UART HAL Configuration

**Debug UART Setup (ST-Link VCP):**
```c
/**
 * @brief Initialize debug UART for command interface
 * @return SystemError_t Success or error code
 */
SystemError_t debug_uart_init(void) {
    UART_HandleTypeDef* huart = &huart3;  // USART3 for ST-Link VCP
    
    // Configure UART parameters
    huart->Instance = USART3;
    huart->Init.BaudRate = DEBUG_UART_BAUDRATE;  // From SSOT config
    huart->Init.WordLength = UART_WORDLENGTH_8B;
    huart->Init.StopBits = UART_STOPBITS_1;
    huart->Init.Parity = UART_PARITY_NONE;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;
    huart->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    
    if (HAL_UART_Init(huart) != HAL_OK) {
        return ERROR_COMM_INIT_FAILED;
    }
    
    // Enable UART interrupts for RX
    HAL_NVIC_SetPriority(USART3_IRQn, UART_RX_INTERRUPT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
    
    // Start DMA reception
    if (HAL_UART_Receive_DMA(huart, uart_rx_buffer, UART_RX_BUFFER_SIZE) != HAL_OK) {
        return ERROR_COMM_DMA_FAILED;
    }
    
    return SYSTEM_OK;
}
```

**External UART Setup (UART4):**
```c
/**
 * @brief Initialize external UART for system communication
 * @return SystemError_t Success or error code
 */
SystemError_t external_uart_init(void) {
    UART_HandleTypeDef* huart = &huart4;
    
    huart->Instance = UART4;
    huart->Init.BaudRate = EXT_UART_BAUDRATE;
    huart->Init.WordLength = UART_WORDLENGTH_8B;
    huart->Init.StopBits = UART_STOPBITS_1;
    huart->Init.Parity = UART_PARITY_NONE;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;
    huart->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    
    if (HAL_UART_Init(huart) != HAL_OK) {
        return ERROR_COMM_INIT_FAILED;
    }
    
    return SYSTEM_OK;
}
```

### ASCII Command Protocol

**Command Structure:**
```c
// ASCII command format: "CMD PARAM1 PARAM2\r\n"
typedef struct {
    char command[16];        // Command name (e.g., "MOVE", "STATUS")
    float parameters[8];     // Up to 8 numeric parameters
    uint8_t param_count;     // Number of parameters provided
    bool valid;              // Command parsing success flag
} ASCIICommand_t;

// Command parsing function
SystemError_t parse_ascii_command(const char* input, ASCIICommand_t* cmd) {
    if (input == NULL || cmd == NULL) {
        return ERROR_NULL_POINTER;
    }
    
    memset(cmd, 0, sizeof(ASCIICommand_t));
    
    // Parse command name
    int chars_read = 0;
    if (sscanf(input, "%15s %n", cmd->command, &chars_read) != 1) {
        return ERROR_COMM_PARSE_FAILED;
    }
    
    // Convert command to uppercase
    for (int i = 0; cmd->command[i]; i++) {
        cmd->command[i] = toupper(cmd->command[i]);
    }
    
    // Parse parameters
    const char* param_start = input + chars_read;
    cmd->param_count = 0;
    
    while (cmd->param_count < 8) {
        float value;
        int param_chars;
        
        if (sscanf(param_start, "%f %n", &value, &param_chars) != 1) {
            break;  // No more parameters
        }
        
        cmd->parameters[cmd->param_count++] = value;
        param_start += param_chars;
    }
    
    cmd->valid = true;
    return SYSTEM_OK;
}
```

**Command Handlers:**
```c
// Command handler function pointer type
typedef SystemError_t (*CommandHandler_t)(const ASCIICommand_t* cmd, char* response, size_t response_size);

// Command registry entry
typedef struct {
    const char* command_name;
    CommandHandler_t handler;
    const char* help_text;
} CommandRegistryEntry_t;

// Motor movement command handler
SystemError_t handle_move_command(const ASCIICommand_t* cmd, char* response, size_t response_size) {
    if (cmd->param_count < 2) {
        snprintf(response, response_size, "ERROR: MOVE requires motor_id and position\r\n");
        return ERROR_COMM_INVALID_PARAMS;
    }
    
    uint8_t motor_id = (uint8_t)cmd->parameters[0];
    float target_position = cmd->parameters[1];
    
    if (motor_id >= MAX_MOTORS) {
        snprintf(response, response_size, "ERROR: Invalid motor ID %d\r\n", motor_id);
        return ERROR_MOTOR_INVALID_ID;
    }
    
    SystemError_t result = motor_move_to_position(motor_id, target_position);
    if (result == SYSTEM_OK) {
        snprintf(response, response_size, "OK: Motor %d moving to %.2f degrees\r\n", 
                motor_id, target_position);
    } else {
        snprintf(response, response_size, "ERROR: Move failed with code %d\r\n", result);
    }
    
    return result;
}

// Status query command handler
SystemError_t handle_status_command(const ASCIICommand_t* cmd, char* response, size_t response_size) {
    if (cmd->param_count == 0) {
        // System status
        SystemStatus_t status;
        get_system_status(&status);
        
        snprintf(response, response_size, 
                "STATUS: Motors=%d, Uptime=%lums, Errors=%d\r\n",
                status.active_motors, status.uptime_ms, status.error_count);
    } else {
        // Motor-specific status
        uint8_t motor_id = (uint8_t)cmd->parameters[0];
        if (motor_id >= MAX_MOTORS) {
            snprintf(response, response_size, "ERROR: Invalid motor ID %d\r\n", motor_id);
            return ERROR_MOTOR_INVALID_ID;
        }
        
        MotorStateInfo_t motor_state;
        SystemError_t result = get_motor_state(motor_id, &motor_state);
        if (result == SYSTEM_OK) {
            snprintf(response, response_size,
                    "MOTOR%d: Pos=%.2f°, Speed=%.1frpm, State=%d\r\n",
                    motor_id, motor_state.current_position, 
                    motor_state.current_speed, motor_state.state);
        } else {
            snprintf(response, response_size, "ERROR: Failed to get motor %d status\r\n", motor_id);
        }
    }
    
    return SYSTEM_OK;
}

// Command registry
static const CommandRegistryEntry_t command_registry[] = {
    {"MOVE", handle_move_command, "MOVE motor_id position - Move motor to position"},
    {"STATUS", handle_status_command, "STATUS [motor_id] - Get system or motor status"},
    {"STOP", handle_stop_command, "STOP [motor_id] - Stop motor(s)"},
    {"HOME", handle_home_command, "HOME motor_id - Home motor to reference position"},
    {"RESET", handle_reset_command, "RESET - Reset system"},
    {"HELP", handle_help_command, "HELP - Show available commands"},
    {NULL, NULL, NULL}  // Terminator
};
```

### Binary Protocol Implementation

**Binary Message Format:**
```c
// Binary command structure
typedef struct {
    uint8_t command_id;      // Command identifier
    uint8_t motor_id;        // Target motor (0xFF for all)
    uint16_t sequence;       // Message sequence number
    float data[4];           // Command-specific data
    uint32_t checksum;       // CRC32 checksum
} __attribute__((packed)) BinaryCommand_t;

// Binary response structure
typedef struct {
    uint8_t response_id;     // Response identifier
    uint8_t status;          // SystemError_t status code
    uint16_t sequence;       // Original message sequence
    float data[4];           // Response data
    uint32_t checksum;       // CRC32 checksum
} __attribute__((packed)) BinaryResponse_t;

// Command IDs
#define CMD_MOVE_TO_POSITION    0x01
#define CMD_SET_SPEED           0x02
#define CMD_STOP_MOTOR          0x03
#define CMD_GET_STATUS          0x04
#define CMD_HOME_MOTOR          0x05
#define CMD_RESET_SYSTEM        0x06
```

## CAN-FD Protocol Implementation

### CAN Configuration
```c
/**
 * @brief Initialize CAN-FD interface for motor network
 * @return SystemError_t Success or error code
 */
SystemError_t can_motor_init(void) {
    FDCAN_HandleTypeDef* hfdcan = &hfdcan1;
    
    // Basic CAN configuration
    hfdcan->Instance = FDCAN1;
    hfdcan->Init.FrameFormat = FDCAN_FRAME_FD_BRS;  // CAN-FD with bit rate switching
    hfdcan->Init.Mode = FDCAN_MODE_NORMAL;
    hfdcan->Init.AutoRetransmission = ENABLE;
    hfdcan->Init.TransmitPause = DISABLE;
    hfdcan->Init.ProtocolException = DISABLE;
    
    // Nominal bit timing (500 kbps)
    hfdcan->Init.NominalPrescaler = 16;
    hfdcan->Init.NominalSyncJumpWidth = 1;
    hfdcan->Init.NominalTimeSeg1 = 63;
    hfdcan->Init.NominalTimeSeg2 = 16;
    
    // Data bit timing (2 Mbps for CAN-FD)
    hfdcan->Init.DataPrescaler = 4;
    hfdcan->Init.DataSyncJumpWidth = 1;
    hfdcan->Init.DataTimeSeg1 = 15;
    hfdcan->Init.DataTimeSeg2 = 4;
    
    // Message RAM configuration
    hfdcan->Init.MessageRAMOffset = 0;
    hfdcan->Init.StdFiltersNbr = 1;
    hfdcan->Init.ExtFiltersNbr = 0;
    hfdcan->Init.RxFifo0ElmtsNbr = 8;
    hfdcan->Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_64;
    hfdcan->Init.RxFifo1ElmtsNbr = 0;
    hfdcan->Init.RxBuffersNbr = 0;
    hfdcan->Init.TxEventsNbr = 0;
    hfdcan->Init.TxBuffersNbr = 0;
    hfdcan->Init.TxFifoQueueElmtsNbr = 8;
    hfdcan->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
    hfdcan->Init.TxElmtSize = FDCAN_DATA_BYTES_64;
    
    if (HAL_FDCAN_Init(hfdcan) != HAL_OK) {
        return ERROR_COMM_INIT_FAILED;
    }
    
    // Configure message filters
    FDCAN_FilterTypeDef filter_config = {0};
    filter_config.IdType = FDCAN_STANDARD_ID;
    filter_config.FilterIndex = 0;
    filter_config.FilterType = FDCAN_FILTER_MASK;
    filter_config.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter_config.FilterID1 = CAN_MOTOR_BASE_ID;     // Base motor command ID
    filter_config.FilterID2 = 0x7F0;                 // Mask for motor IDs
    
    if (HAL_FDCAN_ConfigFilter(hfdcan, &filter_config) != HAL_OK) {
        return ERROR_COMM_CONFIG_FAILED;
    }
    
    // Start CAN
    if (HAL_FDCAN_Start(hfdcan) != HAL_OK) {
        return ERROR_COMM_START_FAILED;
    }
    
    // Enable RX interrupt
    if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
        return ERROR_COMM_INTERRUPT_FAILED;
    }
    
    return SYSTEM_OK;
}
```

### CAN Motor Control Protocol
```c
// CAN message IDs
#define CAN_MOTOR_BASE_ID       0x100   // Base motor command ID
#define CAN_STATUS_BASE_ID      0x200   // Base status response ID
#define CAN_ERROR_ID            0x7FF   // Error/emergency ID

// CAN motor command structure
typedef struct {
    uint8_t motor_id;        // Motor identifier (0-15)
    uint8_t command;         // Command type
    uint16_t sequence;       // Message sequence
    float parameter1;        // First parameter
    float parameter2;        // Second parameter
} __attribute__((packed)) CANMotorCommand_t;

/**
 * @brief Send motor command via CAN
 * @param motor_id Motor identifier
 * @param command Command type
 * @param param1 First parameter
 * @param param2 Second parameter
 * @return SystemError_t Success or error code
 */
SystemError_t can_send_motor_command(uint8_t motor_id, uint8_t command, 
                                    float param1, float param2) {
    FDCAN_TxHeaderTypeDef tx_header = {0};
    CANMotorCommand_t can_cmd = {0};
    
    // Prepare CAN header
    tx_header.Identifier = CAN_MOTOR_BASE_ID + motor_id;
    tx_header.IdType = FDCAN_STANDARD_ID;
    tx_header.TxFrameType = FDCAN_DATA_FRAME;
    tx_header.DataLength = FDCAN_DLC_BYTES_12;  // sizeof(CANMotorCommand_t)
    tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_header.BitRateSwitch = FDCAN_BRS_ON;     // Use CAN-FD high speed
    tx_header.FDFormat = FDCAN_FD_CAN;
    tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    tx_header.MessageMarker = 0;
    
    // Prepare command data
    can_cmd.motor_id = motor_id;
    can_cmd.command = command;
    can_cmd.sequence = get_next_can_sequence();
    can_cmd.parameter1 = param1;
    can_cmd.parameter2 = param2;
    
    // Send message
    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &tx_header, (uint8_t*)&can_cmd) != HAL_OK) {
        return ERROR_COMM_TX_FAILED;
    }
    
    return SYSTEM_OK;
}
```

## SPI L6470 Communication

### L6470 SPI Protocol
```c
/**
 * @brief Initialize SPI for L6470 daisy-chain communication
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_spi_init(void) {
    SPI_HandleTypeDef* hspi = &hspi2;
    
    hspi->Instance = SPI2;
    hspi->Init.Mode = SPI_MODE_MASTER;
    hspi->Init.Direction = SPI_DIRECTION_2LINES;
    hspi->Init.DataSize = SPI_DATASIZE_8BIT;
    hspi->Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi->Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi->Init.NSS = SPI_NSS_SOFT;
    hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;  // ~3.75 MHz at 120 MHz APB
    hspi->Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi->Init.TIMode = SPI_TIMODE_DISABLE;
    hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi->Init.CRCPolynomial = 0x0;
    hspi->Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    hspi->Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    hspi->Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    hspi->Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi->Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi->Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    hspi->Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    hspi->Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    hspi->Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    hspi->Init.IOSwap = SPI_IO_SWAP_DISABLE;
    
    if (HAL_SPI_Init(hspi) != HAL_OK) {
        return ERROR_COMM_INIT_FAILED;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Send command to L6470 via SPI daisy-chain
 * @param motor_id Motor identifier (0 or 1)
 * @param command L6470 command byte
 * @param data Command data (if applicable)
 * @param data_length Number of data bytes
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_spi_send_command(uint8_t motor_id, uint8_t command, 
                                    const uint8_t* data, uint8_t data_length) {
    if (motor_id >= MAX_MOTORS) {
        return ERROR_MOTOR_INVALID_ID;
    }
    
    // Prepare daisy-chain buffer (2 motors maximum)
    uint8_t tx_buffer[8] = {0};  // NOP commands for non-target motors
    uint8_t rx_buffer[8] = {0};
    uint8_t buffer_size = MAX_MOTORS * (1 + data_length);  // Command + data per motor
    
    // Fill buffer with NOP commands
    for (int i = 0; i < buffer_size; i++) {
        tx_buffer[i] = L6470_NOP;
    }
    
    // Insert actual command for target motor
    int cmd_index = (MAX_MOTORS - 1 - motor_id) * (1 + data_length);
    tx_buffer[cmd_index] = command;
    
    // Insert data bytes if present
    for (int i = 0; i < data_length; i++) {
        tx_buffer[cmd_index + 1 + i] = data[i];
    }
    
    // Assert CS (active low)
    HAL_GPIO_WritePin(L6470_SPI_CS_PORT, L6470_SPI_CS_PIN, GPIO_PIN_RESET);
    
    // Transmit command
    HAL_StatusTypeDef spi_result = HAL_SPI_TransmitReceive(&hspi2, tx_buffer, 
                                                          rx_buffer, buffer_size, 
                                                          SPI_TIMEOUT_MS);
    
    // Deassert CS
    HAL_GPIO_WritePin(L6470_SPI_CS_PORT, L6470_SPI_CS_PIN, GPIO_PIN_SET);
    
    if (spi_result != HAL_OK) {
        return ERROR_COMM_SPI_FAILED;
    }
    
    // Process response data if needed
    if (command & 0x80) {  // Read command
        // Extract response for target motor
        // Response processing depends on specific command
    }
    
    return SYSTEM_OK;
}
```

## I2C AS5600 Communication

### AS5600 I2C Protocol
```c
/**
 * @brief Initialize I2C for AS5600 encoder communication
 * @param i2c_instance I2C instance (I2C1 or I2C2)
 * @return SystemError_t Success or error code
 */
SystemError_t as5600_i2c_init(I2C_TypeDef* i2c_instance) {
    I2C_HandleTypeDef* hi2c = (i2c_instance == I2C1) ? &hi2c1 : &hi2c2;
    
    hi2c->Instance = i2c_instance;
    hi2c->Init.Timing = I2C_TIMING_400KHZ;           // 400 kHz fast mode
    hi2c->Init.OwnAddress1 = 0;
    hi2c->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c->Init.OwnAddress2 = 0;
    hi2c->Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    if (HAL_I2C_Init(hi2c) != HAL_OK) {
        return ERROR_COMM_INIT_FAILED;
    }
    
    // Configure analog filter
    if (HAL_I2CEx_ConfigAnalogFilter(hi2c, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        return ERROR_COMM_CONFIG_FAILED;
    }
    
    // Configure digital filter
    if (HAL_I2CEx_ConfigDigitalFilter(hi2c, 0) != HAL_OK) {
        return ERROR_COMM_CONFIG_FAILED;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Read AS5600 register via I2C
 * @param encoder_id Encoder identifier (0 or 1)
 * @param reg_addr Register address
 * @param data Pointer to data buffer
 * @param data_length Number of bytes to read
 * @return SystemError_t Success or error code
 */
SystemError_t as5600_i2c_read_register(uint8_t encoder_id, uint8_t reg_addr, 
                                      uint8_t* data, uint8_t data_length) {
    if (encoder_id >= MAX_ENCODERS) {
        return ERROR_ENCODER_INVALID_ID;
    }
    
    if (data == NULL) {
        return ERROR_NULL_POINTER;
    }
    
    I2C_HandleTypeDef* hi2c = (encoder_id == 0) ? &hi2c1 : &hi2c2;
    
    // Write register address, then read data
    HAL_StatusTypeDef result = HAL_I2C_Mem_Read(hi2c, AS5600_I2C_ADDRESS << 1, 
                                               reg_addr, I2C_MEMADD_SIZE_8BIT,
                                               data, data_length, I2C_TIMEOUT_MS);
    
    if (result == HAL_TIMEOUT) {
        return ERROR_COMM_TIMEOUT;
    } else if (result != HAL_OK) {
        return ERROR_COMM_I2C_FAILED;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Write AS5600 register via I2C
 * @param encoder_id Encoder identifier (0 or 1)
 * @param reg_addr Register address
 * @param data Pointer to data buffer
 * @param data_length Number of bytes to write
 * @return SystemError_t Success or error code
 */
SystemError_t as5600_i2c_write_register(uint8_t encoder_id, uint8_t reg_addr,
                                       const uint8_t* data, uint8_t data_length) {
    if (encoder_id >= MAX_ENCODERS) {
        return ERROR_ENCODER_INVALID_ID;
    }
    
    if (data == NULL) {
        return ERROR_NULL_POINTER;
    }
    
    I2C_HandleTypeDef* hi2c = (encoder_id == 0) ? &hi2c1 : &hi2c2;
    
    HAL_StatusTypeDef result = HAL_I2C_Mem_Write(hi2c, AS5600_I2C_ADDRESS << 1,
                                                reg_addr, I2C_MEMADD_SIZE_8BIT,
                                                (uint8_t*)data, data_length, 
                                                I2C_TIMEOUT_MS);
    
    if (result == HAL_TIMEOUT) {
        return ERROR_COMM_TIMEOUT;
    } else if (result != HAL_OK) {
        return ERROR_COMM_I2C_FAILED;
    }
    
    return SYSTEM_OK;
}
```

## Ethernet Implementation

### Ethernet Configuration
```c
/**
 * @brief Initialize Ethernet interface with LAN8742 PHY
 * @return SystemError_t Success or error code
 */
SystemError_t ethernet_init(void) {
    ETH_HandleTypeDef* heth = &heth;
    
    // ETH configuration
    heth->Instance = ETH;
    heth->Init.MACAddr[0] = ETH_MAC_ADDR0;
    heth->Init.MACAddr[1] = ETH_MAC_ADDR1;
    heth->Init.MACAddr[2] = ETH_MAC_ADDR2;
    heth->Init.MACAddr[3] = ETH_MAC_ADDR3;
    heth->Init.MACAddr[4] = ETH_MAC_ADDR4;
    heth->Init.MACAddr[5] = ETH_MAC_ADDR5;
    heth->Init.MediaInterface = HAL_ETH_RMII_MODE;
    heth->Init.TxDesc = DMATxDscrTab;
    heth->Init.RxDesc = DMARxDscrTab;
    heth->Init.RxBuffLen = ETH_RX_BUFFER_SIZE;
    
    if (HAL_ETH_Init(heth) != HAL_OK) {
        return ERROR_COMM_INIT_FAILED;
    }
    
    // Initialize LwIP stack
    lwip_init();
    
    // Configure network interface
    netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);
    netif_set_default(&gnetif);
    
    if (netif_is_link_up(&gnetif)) {
        netif_set_up(&gnetif);
    } else {
        netif_set_down(&gnetif);
    }
    
    return SYSTEM_OK;
}
```

### UDP Communication
```c
/**
 * @brief Send motor status via UDP
 * @param motor_id Motor identifier
 * @return SystemError_t Success or error code
 */
SystemError_t udp_send_motor_status(uint8_t motor_id) {
    struct udp_pcb* pcb;
    struct pbuf* p;
    MotorStatusPacket_t status_packet;
    err_t err;
    
    // Create UDP PCB
    pcb = udp_new();
    if (pcb == NULL) {
        return ERROR_COMM_UDP_FAILED;
    }
    
    // Prepare status packet
    status_packet.header.magic = UDP_PACKET_MAGIC;
    status_packet.header.packet_type = UDP_PACKET_MOTOR_STATUS;
    status_packet.header.motor_id = motor_id;
    status_packet.header.timestamp = HAL_GetTick();
    
    get_motor_state(motor_id, &status_packet.motor_state);
    
    status_packet.header.checksum = calculate_checksum(&status_packet, 
                                                      sizeof(status_packet) - sizeof(uint16_t));
    
    // Allocate pbuf and copy data
    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(status_packet), PBUF_RAM);
    if (p == NULL) {
        udp_remove(pcb);
        return ERROR_COMM_MEMORY_FAILED;
    }
    
    memcpy(p->payload, &status_packet, sizeof(status_packet));
    
    // Send packet
    err = udp_sendto(pcb, p, &remote_ip, UDP_REMOTE_PORT);
    
    // Cleanup
    pbuf_free(p);
    udp_remove(pcb);
    
    if (err != ERR_OK) {
        return ERROR_COMM_UDP_FAILED;
    }
    
    return SYSTEM_OK;
}
```

## Protocol Error Handling

### Communication Error Recovery
```c
/**
 * @brief Handle communication errors with retry logic
 * @param protocol Protocol type that failed
 * @param error_code Specific error code
 * @return SystemError_t Recovery result
 */
SystemError_t handle_comm_error(CommProtocol_t protocol, SystemError_t error_code) {
    static uint32_t error_counts[8] = {0};  // Error count per protocol
    static uint32_t last_error_time[8] = {0};
    
    uint32_t current_time = HAL_GetTick();
    uint8_t protocol_index = (uint8_t)protocol;
    
    // Increment error count
    error_counts[protocol_index]++;
    last_error_time[protocol_index] = current_time;
    
    // Log error
    log_comm_error(protocol, error_code, error_counts[protocol_index]);
    
    // Protocol-specific recovery actions
    switch (protocol) {
        case PROTOCOL_SPI_L6470:
            if (error_counts[protocol_index] > 3) {
                // Reset L6470 drivers
                l6470_hard_reset();
                HAL_Delay(10);
                l6470_spi_init();
                error_counts[protocol_index] = 0;
            }
            break;
            
        case PROTOCOL_I2C_AS5600:
            if (error_counts[protocol_index] > 5) {
                // Reset I2C peripheral
                as5600_i2c_reset();
                as5600_i2c_init(I2C1);
                as5600_i2c_init(I2C2);
                error_counts[protocol_index] = 0;
            }
            break;
            
        case PROTOCOL_UART_ASCII:
        case PROTOCOL_UART_BINARY:
            if (error_counts[protocol_index] > 10) {
                // Reset UART
                uart_reset_and_reinit();
                error_counts[protocol_index] = 0;
            }
            break;
            
        case PROTOCOL_CAN_MOTOR:
            if (error_counts[protocol_index] > 5) {
                // Reset CAN peripheral
                can_reset_and_reinit();
                error_counts[protocol_index] = 0;
            }
            break;
            
        default:
            break;
    }
    
    // If too many errors, disable protocol temporarily
    if (error_counts[protocol_index] > 20) {
        disable_protocol_temporarily(protocol, 5000);  // 5 second timeout
        error_counts[protocol_index] = 0;
        return ERROR_COMM_PROTOCOL_DISABLED;
    }
    
    return SYSTEM_OK;
}
```

Remember: **Robust communication ensures reliable motor control, proper error handling prevents system failures, efficient protocols enable real-time performance.**
