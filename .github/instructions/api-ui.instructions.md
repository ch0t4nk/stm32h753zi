---
applyTo: "src/{communication,application}/**/*.{c,h}"
description: "API design, user interfaces, documentation generation, and external integration for STM32H753ZI stepper motor project"
---

# API & User Interface Instructions

## Overview
This instruction file provides comprehensive guidance for API design, user interface implementation, documentation generation, and external system integration in the STM32H753ZI stepper motor control system.

## API Design Principles

### RESTful API Architecture
```c
// HTTP method mappings for motor control
typedef enum {
    HTTP_METHOD_GET,        // Read operations (status, configuration)
    HTTP_METHOD_POST,       // Create operations (new motion commands)
    HTTP_METHOD_PUT,        // Update operations (modify settings)
    HTTP_METHOD_DELETE,     // Stop/cancel operations
    HTTP_METHOD_PATCH       // Partial updates (single parameter changes)
} HTTPMethod_t;

// API endpoint structure
typedef struct {
    const char* path;                  // URL path pattern
    HTTPMethod_t method;               // HTTP method
    SystemError_t (*handler)(const char* body, char* response, size_t response_size);
    const char* description;           // API documentation
    bool requires_auth;                // Authentication required
    uint32_t rate_limit_per_minute;    // Rate limiting
} APIEndpoint_t;

// Motor control API endpoints
static const APIEndpoint_t motor_api_endpoints[] = {
    {"/api/v1/motors", HTTP_METHOD_GET, handle_get_all_motors, 
     "Get status of all motors", false, 60},
    {"/api/v1/motors/{id}", HTTP_METHOD_GET, handle_get_motor_status,
     "Get specific motor status", false, 120},
    {"/api/v1/motors/{id}/move", HTTP_METHOD_POST, handle_motor_move,
     "Command motor movement", true, 30},
    {"/api/v1/motors/{id}/stop", HTTP_METHOD_DELETE, handle_motor_stop,
     "Stop motor movement", true, 60},
    {"/api/v1/motors/{id}/home", HTTP_METHOD_POST, handle_motor_home,
     "Home motor to reference position", true, 10},
    {"/api/v1/system/status", HTTP_METHOD_GET, handle_get_system_status,
     "Get overall system status", false, 120},
    {"/api/v1/system/emergency-stop", HTTP_METHOD_POST, handle_emergency_stop,
     "Trigger emergency stop", true, 10},
    {"/api/v1/config/motors/{id}", HTTP_METHOD_PUT, handle_update_motor_config,
     "Update motor configuration", true, 10},
    {NULL, 0, NULL, NULL, false, 0}  // Terminator
};
```

### JSON API Data Structures
```c
// JSON response structure for motor status
typedef struct {
    uint8_t motor_id;                  // Motor identifier
    const char* state;                 // Motor state string
    float current_position;            // Position in degrees
    float target_position;             // Target position in degrees
    float current_speed;               // Speed in RPM
    bool homed;                        // Home position established
    const char* last_error;            // Last error description
    uint32_t error_count;             // Total error count
    uint32_t uptime_ms;               // Motor uptime
} MotorStatusJSON_t;

// JSON response structure for system status
typedef struct {
    const char* system_state;          // System state string
    uint32_t uptime_ms;               // System uptime
    uint8_t active_motors;            // Number of active motors
    uint32_t total_commands;          // Commands processed
    uint32_t total_errors;            // Total error count
    float cpu_usage_percent;          // CPU utilization
    uint32_t free_memory_bytes;       // Available memory
    bool emergency_stop_active;       // E-stop status
    MotorStatusJSON_t motors[MAX_MOTORS];  // Motor status array
} SystemStatusJSON_t;

/**
 * @brief Generate JSON response for motor status
 * @param motor_id Motor identifier
 * @param json_buffer Output buffer for JSON
 * @param buffer_size Size of output buffer
 * @return SystemError_t Success or error code
 */
SystemError_t generate_motor_status_json(uint8_t motor_id, char* json_buffer, size_t buffer_size) {
    if (motor_id >= MAX_MOTORS) {
        return ERROR_MOTOR_INVALID_ID;
    }
    
    MotorStateInfo_t motor_state;
    SystemError_t result = get_motor_state(motor_id, &motor_state);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Build JSON response
    int written = snprintf(json_buffer, buffer_size,
        "{\n"
        "  \"motor_id\": %d,\n"
        "  \"state\": \"%s\",\n"
        "  \"current_position\": %.2f,\n"
        "  \"target_position\": %.2f,\n"
        "  \"current_speed\": %.1f,\n"
        "  \"homed\": %s,\n"
        "  \"last_error\": \"%s\",\n"
        "  \"error_count\": %d,\n"
        "  \"timestamp\": %lu\n"
        "}",
        motor_id,
        get_motor_state_string(motor_state.state),
        motor_state.current_position,
        motor_state.target_position,
        motor_state.current_speed,
        motor_state.homed ? "true" : "false",
        get_error_description(motor_state.last_error),
        motor_state.error_count,
        motor_state.last_update_time
    );
    
    if (written >= buffer_size) {
        return ERROR_BUFFER_OVERFLOW;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Parse JSON motor movement command
 * @param json_body JSON request body
 * @param motor_id Output motor identifier
 * @param target_position Output target position
 * @param speed Output movement speed
 * @return SystemError_t Success or error code
 */
SystemError_t parse_motor_move_json(const char* json_body, uint8_t* motor_id, 
                                   float* target_position, float* speed) {
    // Simple JSON parsing for embedded system
    // In a full implementation, use a JSON library like cJSON
    
    const char* pos_key = "\"position\":";
    const char* speed_key = "\"speed\":";
    const char* motor_key = "\"motor_id\":";
    
    // Parse motor_id
    const char* motor_ptr = strstr(json_body, motor_key);
    if (motor_ptr != NULL) {
        motor_ptr += strlen(motor_key);
        *motor_id = (uint8_t)strtol(motor_ptr, NULL, 10);
    } else {
        return ERROR_COMM_PARSE_FAILED;
    }
    
    // Parse position
    const char* pos_ptr = strstr(json_body, pos_key);
    if (pos_ptr != NULL) {
        pos_ptr += strlen(pos_key);
        *target_position = strtof(pos_ptr, NULL);
    } else {
        return ERROR_COMM_PARSE_FAILED;
    }
    
    // Parse speed (optional)
    const char* speed_ptr = strstr(json_body, speed_key);
    if (speed_ptr != NULL) {
        speed_ptr += strlen(speed_key);
        *speed = strtof(speed_ptr, NULL);
    } else {
        *speed = DEFAULT_MOTOR_SPEED_RPM;  // Use default speed
    }
    
    return SYSTEM_OK;
}
```

### API Request Handlers
```c
/**
 * @brief Handle GET request for all motor status
 * @param body Request body (unused for GET)
 * @param response Response buffer
 * @param response_size Size of response buffer
 * @return SystemError_t Success or error code
 */
SystemError_t handle_get_all_motors(const char* body, char* response, size_t response_size) {
    (void)body;  // Unused parameter
    
    SystemStatusJSON_t system_status;
    SystemError_t result = get_system_status_json(&system_status);
    if (result != SYSTEM_OK) {
        snprintf(response, response_size, 
                "{\"error\": \"Failed to get system status\", \"code\": %d}", result);
        return result;
    }
    
    // Generate JSON response
    int written = snprintf(response, response_size,
        "{\n"
        "  \"system_state\": \"%s\",\n"
        "  \"uptime_ms\": %lu,\n"
        "  \"motors\": [\n",
        system_status.system_state,
        system_status.uptime_ms
    );
    
    // Add each motor status
    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        char motor_json[512];
        result = generate_motor_status_json(i, motor_json, sizeof(motor_json));
        if (result == SYSTEM_OK) {
            written += snprintf(response + written, response_size - written,
                              "%s%s", motor_json, (i < MAX_MOTORS - 1) ? ",\n" : "\n");
        }
    }
    
    written += snprintf(response + written, response_size - written, "  ]\n}");
    
    return SYSTEM_OK;
}

/**
 * @brief Handle POST request for motor movement
 * @param body JSON request body
 * @param response Response buffer
 * @param response_size Size of response buffer
 * @return SystemError_t Success or error code
 */
SystemError_t handle_motor_move(const char* body, char* response, size_t response_size) {
    uint8_t motor_id;
    float target_position;
    float speed;
    
    // Parse JSON request
    SystemError_t result = parse_motor_move_json(body, &motor_id, &target_position, &speed);
    if (result != SYSTEM_OK) {
        snprintf(response, response_size,
                "{\"error\": \"Invalid JSON format\", \"code\": %d}", result);
        return result;
    }
    
    // Validate parameters
    if (motor_id >= MAX_MOTORS) {
        snprintf(response, response_size,
                "{\"error\": \"Invalid motor ID\", \"motor_id\": %d}", motor_id);
        return ERROR_MOTOR_INVALID_ID;
    }
    
    // Execute motor movement
    result = motor_move_to_position(motor_id, target_position);
    if (result == SYSTEM_OK) {
        snprintf(response, response_size,
                "{\n"
                "  \"status\": \"success\",\n"
                "  \"motor_id\": %d,\n"
                "  \"target_position\": %.2f,\n"
                "  \"speed\": %.1f,\n"
                "  \"timestamp\": %lu\n"
                "}",
                motor_id, target_position, speed, HAL_GetTick());
    } else {
        snprintf(response, response_size,
                "{\n"
                "  \"status\": \"error\",\n"
                "  \"error\": \"%s\",\n"
                "  \"code\": %d,\n"
                "  \"motor_id\": %d\n"
                "}",
                get_error_description(result), result, motor_id);
    }
    
    return result;
}

/**
 * @brief Handle emergency stop request
 * @param body Request body (unused)
 * @param response Response buffer
 * @param response_size Size of response buffer
 * @return SystemError_t Success or error code
 */
SystemError_t handle_emergency_stop(const char* body, char* response, size_t response_size) {
    (void)body;  // Unused parameter
    
    SystemError_t result = execute_emergency_stop(ESTOP_SOURCE_API);
    
    if (result == SYSTEM_OK) {
        snprintf(response, response_size,
                "{\n"
                "  \"status\": \"success\",\n"
                "  \"message\": \"Emergency stop activated\",\n"
                "  \"timestamp\": %lu\n"
                "}", HAL_GetTick());
    } else {
        snprintf(response, response_size,
                "{\n"
                "  \"status\": \"error\",\n"
                "  \"error\": \"%s\",\n"
                "  \"code\": %d\n"
                "}", get_error_description(result), result);
    }
    
    return result;
}
```

## Command Line Interface

### ASCII Command Processing
```c
// Command line interface structure
typedef struct {
    char input_buffer[CLI_MAX_INPUT_LENGTH];
    uint16_t input_length;
    bool command_ready;
    uint32_t command_count;
    uint32_t prompt_count;
} CommandLineInterface_t;

static CommandLineInterface_t cli = {0};

/**
 * @brief Initialize command line interface
 * @return SystemError_t Success or error code
 */
SystemError_t cli_init(void) {
    memset(&cli, 0, sizeof(CommandLineInterface_t));
    
    // Send welcome message
    cli_print_welcome();
    cli_show_prompt();
    
    return SYSTEM_OK;
}

/**
 * @brief Process received character from UART
 * @param received_char Character received from UART
 * @return SystemError_t Success or error code
 */
SystemError_t cli_process_char(char received_char) {
    switch (received_char) {
        case '\r':  // Carriage return
        case '\n':  // Line feed
            if (cli.input_length > 0) {
                cli.input_buffer[cli.input_length] = '\0';
                cli.command_ready = true;
                cli_echo_char('\n');
            }
            break;
            
        case '\b':  // Backspace
        case 0x7F:  // Delete
            if (cli.input_length > 0) {
                cli.input_length--;
                cli_echo_string("\b \b");  // Erase character on terminal
            }
            break;
            
        case 0x03:  // Ctrl+C
            cli_cancel_command();
            break;
            
        default:
            if (received_char >= 32 && received_char <= 126) {  // Printable characters
                if (cli.input_length < (CLI_MAX_INPUT_LENGTH - 1)) {
                    cli.input_buffer[cli.input_length++] = received_char;
                    cli_echo_char(received_char);
                } else {
                    cli_echo_string("\nBuffer full!\n");
                    cli_show_prompt();
                }
            }
            break;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Execute command line command
 * @return SystemError_t Success or error code
 */
SystemError_t cli_execute_command(void) {
    if (!cli.command_ready) {
        return SYSTEM_OK;
    }
    
    ASCIICommand_t command;
    SystemError_t result = parse_ascii_command(cli.input_buffer, &command);
    
    if (result == SYSTEM_OK && command.valid) {
        char response[CLI_MAX_RESPONSE_LENGTH];
        
        // Find and execute command handler
        result = execute_ascii_command(&command, response, sizeof(response));
        
        // Send response
        cli_echo_string(response);
        cli.command_count++;
    } else {
        cli_echo_string("Invalid command. Type 'help' for available commands.\n");
    }
    
    // Reset for next command
    cli.input_length = 0;
    cli.command_ready = false;
    cli_show_prompt();
    
    return result;
}

/**
 * @brief Print welcome message
 */
void cli_print_welcome(void) {
    cli_echo_string("\n");
    cli_echo_string("=====================================\n");
    cli_echo_string("  STM32H753ZI Motor Control System  \n");
    cli_echo_string("         Version " FIRMWARE_VERSION "\n");
    cli_echo_string("=====================================\n");
    cli_echo_string("Type 'help' for available commands\n");
    cli_echo_string("\n");
}

/**
 * @brief Show command prompt
 */
void cli_show_prompt(void) {
    cli_echo_string("motor> ");
    cli.prompt_count++;
}

/**
 * @brief Echo string to UART
 * @param str String to echo
 */
void cli_echo_string(const char* str) {
    if (str != NULL) {
        HAL_UART_Transmit(&huart3, (uint8_t*)str, strlen(str), UART_TIMEOUT_MS);
    }
}

/**
 * @brief Echo single character to UART
 * @param ch Character to echo
 */
void cli_echo_char(char ch) {
    HAL_UART_Transmit(&huart3, (uint8_t*)&ch, 1, UART_TIMEOUT_MS);
}
```

### Advanced CLI Features
```c
/**
 * @brief Enhanced help command with detailed descriptions
 * @param cmd Command structure (unused)
 * @param response Response buffer
 * @param response_size Size of response buffer
 * @return SystemError_t Success or error code
 */
SystemError_t handle_help_command(const ASCIICommand_t* cmd, char* response, size_t response_size) {
    (void)cmd;  // Unused parameter
    
    int written = snprintf(response, response_size,
        "Available Commands:\n"
        "==================\n"
        "  MOVE <motor_id> <position> [speed]  - Move motor to position (degrees)\n"
        "  STOP [motor_id]                     - Stop motor(s) (all if no ID)\n"
        "  HOME <motor_id>                     - Home motor to reference position\n"
        "  STATUS [motor_id]                   - Get system or motor status\n"
        "  RESET                               - Reset system\n"
        "  CONFIG <parameter> <value>          - Set configuration parameter\n"
        "  INFO                                - Show system information\n"
        "  LOG [level]                         - Show or set log level\n"
        "  TEST <test_name>                    - Run system test\n"
        "  HELP                                - Show this help message\n"
        "\n"
        "Examples:\n"
        "  MOVE 0 90.5 100     - Move motor 0 to 90.5° at 100 RPM\n"
        "  STATUS 1            - Get status of motor 1\n"
        "  STOP                - Stop all motors\n"
        "  HOME 0              - Home motor 0\n"
        "\n");
    
    return SYSTEM_OK;
}

/**
 * @brief Configuration command handler
 * @param cmd Command structure
 * @param response Response buffer
 * @param response_size Size of response buffer
 * @return SystemError_t Success or error code
 */
SystemError_t handle_config_command(const ASCIICommand_t* cmd, char* response, size_t response_size) {
    if (cmd->param_count < 2) {
        snprintf(response, response_size, 
                "ERROR: CONFIG requires parameter name and value\n"
                "Usage: CONFIG <parameter> <value>\n"
                "Available parameters: max_speed, acceleration, kp, ki, kd\n");
        return ERROR_COMM_INVALID_PARAMS;
    }
    
    // Parse parameter name (simplified - use string comparison)
    const char* param_name = cmd->command + 7;  // Skip "CONFIG "
    float value = cmd->parameters[1];
    
    SystemError_t result = SYSTEM_OK;
    
    if (strstr(param_name, "max_speed") != NULL) {
        if (value > 0 && value <= MOTOR_MAX_SPEED_RPM) {
            // Update configuration
            motor_config.max_speed_rpm = value;
            snprintf(response, response_size, "OK: Max speed set to %.1f RPM\n", value);
        } else {
            snprintf(response, response_size, "ERROR: Speed must be 0-%.1f RPM\n", MOTOR_MAX_SPEED_RPM);
            result = ERROR_CONFIG_OUT_OF_RANGE;
        }
    } else if (strstr(param_name, "acceleration") != NULL) {
        if (value > 0 && value <= MOTOR_MAX_ACCELERATION) {
            motor_config.acceleration = value;
            snprintf(response, response_size, "OK: Acceleration set to %.1f deg/s²\n", value);
        } else {
            snprintf(response, response_size, "ERROR: Acceleration must be 0-%.1f deg/s²\n", MOTOR_MAX_ACCELERATION);
            result = ERROR_CONFIG_OUT_OF_RANGE;
        }
    } else {
        snprintf(response, response_size, "ERROR: Unknown parameter '%s'\n", param_name);
        result = ERROR_CONFIG_INVALID_VALUE;
    }
    
    return result;
}

/**
 * @brief System information command
 * @param cmd Command structure (unused)
 * @param response Response buffer
 * @param response_size Size of response buffer
 * @return SystemError_t Success or error code
 */
SystemError_t handle_info_command(const ASCIICommand_t* cmd, char* response, size_t response_size) {
    (void)cmd;  // Unused parameter
    
    SystemStatus_t status;
    get_system_status(&status);
    
    snprintf(response, response_size,
        "System Information:\n"
        "==================\n"
        "  Firmware Version: %s\n"
        "  Build Date: %s %s\n"
        "  MCU: STM32H753ZI @ %lu MHz\n"
        "  Uptime: %lu.%03lu seconds\n"
        "  Free Memory: %lu bytes\n"
        "  CPU Usage: %.1f%%\n"
        "  Active Motors: %d/%d\n"
        "  Commands Processed: %lu\n"
        "  Total Errors: %lu\n"
        "  Emergency Stop: %s\n"
        "\n",
        FIRMWARE_VERSION,
        __DATE__, __TIME__,
        SystemCoreClock / 1000000,
        status.uptime_ms / 1000, status.uptime_ms % 1000,
        status.free_heap_bytes,
        status.cpu_usage_percent,
        status.active_motors, MAX_MOTORS,
        status.command_count,
        status.error_count,
        status.safety_status.emergency_stop_active ? "ACTIVE" : "Normal"
    );
    
    return SYSTEM_OK;
}
```

## Documentation Generation

### API Documentation System
```c
/**
 * @brief Generate OpenAPI/Swagger documentation
 * @param output_buffer Buffer for generated documentation
 * @param buffer_size Size of output buffer
 * @return SystemError_t Success or error code
 */
SystemError_t generate_openapi_documentation(char* output_buffer, size_t buffer_size) {
    int written = snprintf(output_buffer, buffer_size,
        "{\n"
        "  \"openapi\": \"3.0.0\",\n"
        "  \"info\": {\n"
        "    \"title\": \"STM32H753ZI Motor Control API\",\n"
        "    \"version\": \"%s\",\n"
        "    \"description\": \"RESTful API for dual stepper motor control system\"\n"
        "  },\n"
        "  \"servers\": [\n"
        "    {\n"
        "      \"url\": \"http://%s/api/v1\",\n"
        "      \"description\": \"Motor control system\"\n"
        "    }\n"
        "  ],\n"
        "  \"paths\": {\n",
        FIRMWARE_VERSION,
        get_device_ip_address()
    );
    
    // Add API endpoint documentation
    for (int i = 0; motor_api_endpoints[i].path != NULL; i++) {
        const APIEndpoint_t* endpoint = &motor_api_endpoints[i];
        
        written += snprintf(output_buffer + written, buffer_size - written,
            "    \"%s\": {\n"
            "      \"%s\": {\n"
            "        \"summary\": \"%s\",\n"
            "        \"responses\": {\n"
            "          \"200\": {\n"
            "            \"description\": \"Success\"\n"
            "          },\n"
            "          \"400\": {\n"
            "            \"description\": \"Bad Request\"\n"
            "          },\n"
            "          \"500\": {\n"
            "            \"description\": \"Internal Server Error\"\n"
            "          }\n"
            "        }\n"
            "      }\n"
            "    }%s\n",
            endpoint->path,
            get_http_method_string(endpoint->method),
            endpoint->description,
            (motor_api_endpoints[i + 1].path != NULL) ? "," : ""
        );
    }
    
    written += snprintf(output_buffer + written, buffer_size - written,
        "  }\n"
        "}\n"
    );
    
    return (written < buffer_size) ? SYSTEM_OK : ERROR_BUFFER_OVERFLOW;
}

/**
 * @brief Generate system diagnostics report
 * @param output_buffer Buffer for generated report
 * @param buffer_size Size of output buffer
 * @return SystemError_t Success or error code
 */
SystemError_t generate_diagnostics_report(char* output_buffer, size_t buffer_size) {
    SystemStatus_t status;
    get_system_status(&status);
    
    int written = snprintf(output_buffer, buffer_size,
        "STM32H753ZI Motor Control System - Diagnostics Report\n"
        "====================================================\n"
        "Generated: %lu ms uptime\n"
        "\n"
        "System Status:\n"
        "--------------\n"
        "  State: %s\n"
        "  Uptime: %lu.%03lu seconds\n"
        "  Free Memory: %lu bytes\n"
        "  CPU Usage: %.1f%%\n"
        "  Emergency Stop: %s\n"
        "\n"
        "Motor Status:\n"
        "-------------\n",
        HAL_GetTick(),
        get_system_state_string(status.state),
        status.uptime_ms / 1000, status.uptime_ms % 1000,
        status.free_heap_bytes,
        status.cpu_usage_percent,
        status.safety_status.emergency_stop_active ? "ACTIVE" : "Normal"
    );
    
    // Add motor-specific diagnostics
    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        MotorStateInfo_t motor_state = status.motor_states[i];
        
        written += snprintf(output_buffer + written, buffer_size - written,
            "  Motor %d:\n"
            "    State: %s\n"
            "    Position: %.2f° (Target: %.2f°)\n"
            "    Speed: %.1f RPM\n"
            "    Homed: %s\n"
            "    Errors: %d (Last: %s)\n"
            "\n",
            i,
            get_motor_state_string(motor_state.state),
            motor_state.current_position,
            motor_state.target_position,
            motor_state.current_speed,
            motor_state.homed ? "Yes" : "No",
            motor_state.error_count,
            get_error_description(motor_state.last_error)
        );
    }
    
    // Add safety system status
    written += snprintf(output_buffer + written, buffer_size - written,
        "Safety Systems:\n"
        "---------------\n"
        "  Watchdog: %s\n"
        "  Over Temperature: %s\n"
        "  Over Current: %s\n"
        "  Position Limits: %s\n"
        "  Encoder Faults: %s\n"
        "  Communication Timeout: %s\n"
        "\n",
        status.safety_status.watchdog_active ? "Active" : "Inactive",
        status.safety_status.over_temperature ? "WARNING" : "Normal",
        status.safety_status.over_current ? "WARNING" : "Normal",
        status.safety_status.position_limit_exceeded ? "VIOLATED" : "Normal",
        status.safety_status.encoder_fault ? "FAULT" : "Normal",
        status.safety_status.communication_timeout ? "TIMEOUT" : "Normal"
    );
    
    return (written < buffer_size) ? SYSTEM_OK : ERROR_BUFFER_OVERFLOW;
}
```

## External System Integration

### CAN Bus Integration
```c
/**
 * @brief CAN message dispatcher for external systems
 * @param can_message Received CAN message
 * @return SystemError_t Success or error code
 */
SystemError_t process_external_can_message(const CANMessage_t* can_message) {
    switch (can_message->id) {
        case CAN_ID_EXTERNAL_MOVE_COMMAND:
            return process_external_move_command(can_message);
            
        case CAN_ID_EXTERNAL_STATUS_REQUEST:
            return process_external_status_request(can_message);
            
        case CAN_ID_EXTERNAL_EMERGENCY_STOP:
            return execute_emergency_stop(ESTOP_SOURCE_CAN);
            
        case CAN_ID_EXTERNAL_CONFIG_UPDATE:
            return process_external_config_update(can_message);
            
        default:
            return ERROR_COMM_UNKNOWN_MESSAGE;
    }
}

/**
 * @brief Send periodic status updates via CAN
 * @return SystemError_t Success or error code
 */
SystemError_t send_periodic_can_status(void) {
    static uint32_t last_status_time = 0;
    uint32_t current_time = HAL_GetTick();
    
    if ((current_time - last_status_time) >= CAN_STATUS_PERIOD_MS) {
        SystemStatus_t status;
        get_system_status(&status);
        
        CANStatusMessage_t can_status = {
            .system_state = status.state,
            .uptime_ms = status.uptime_ms,
            .active_motors = status.active_motors,
            .error_count = status.error_count,
            .emergency_stop = status.safety_status.emergency_stop_active
        };
        
        // Send motor positions
        for (uint8_t i = 0; i < MAX_MOTORS; i++) {
            can_status.motor_positions[i] = status.motor_states[i].current_position;
            can_status.motor_speeds[i] = status.motor_states[i].current_speed;
        }
        
        SystemError_t result = can_send_status_message(&can_status);
        if (result == SYSTEM_OK) {
            last_status_time = current_time;
        }
        
        return result;
    }
    
    return SYSTEM_OK;
}
```

### Modbus Integration
```c
/**
 * @brief Modbus register mapping for external access
 */
typedef enum {
    MODBUS_REG_SYSTEM_STATE = 0x0000,      // System state (read-only)
    MODBUS_REG_MOTOR1_POSITION = 0x0001,   // Motor 1 position (read-only)
    MODBUS_REG_MOTOR1_TARGET = 0x0002,     // Motor 1 target (read/write)
    MODBUS_REG_MOTOR1_SPEED = 0x0003,      // Motor 1 speed (read-only)
    MODBUS_REG_MOTOR1_STATE = 0x0004,      // Motor 1 state (read-only)
    MODBUS_REG_MOTOR2_POSITION = 0x0005,   // Motor 2 position (read-only)
    MODBUS_REG_MOTOR2_TARGET = 0x0006,     // Motor 2 target (read/write)
    MODBUS_REG_MOTOR2_SPEED = 0x0007,      // Motor 2 speed (read-only)
    MODBUS_REG_MOTOR2_STATE = 0x0008,      // Motor 2 state (read-only)
    MODBUS_REG_EMERGENCY_STOP = 0x0009,    // Emergency stop (read/write)
    MODBUS_REG_ERROR_COUNT = 0x000A,       // Total error count (read-only)
    MODBUS_REG_UPTIME = 0x000B             // System uptime (read-only)
} ModbusRegister_t;

/**
 * @brief Handle Modbus read register request
 * @param register_addr Register address
 * @param value Pointer to return value
 * @return SystemError_t Success or error code
 */
SystemError_t modbus_read_register(uint16_t register_addr, uint16_t* value) {
    SystemStatus_t status;
    get_system_status(&status);
    
    switch (register_addr) {
        case MODBUS_REG_SYSTEM_STATE:
            *value = (uint16_t)status.state;
            break;
            
        case MODBUS_REG_MOTOR1_POSITION:
            *value = (uint16_t)(status.motor_states[0].current_position * 10);  // 0.1° resolution
            break;
            
        case MODBUS_REG_MOTOR1_TARGET:
            *value = (uint16_t)(status.motor_states[0].target_position * 10);
            break;
            
        case MODBUS_REG_MOTOR1_SPEED:
            *value = (uint16_t)status.motor_states[0].current_speed;
            break;
            
        case MODBUS_REG_MOTOR1_STATE:
            *value = (uint16_t)status.motor_states[0].state;
            break;
            
        case MODBUS_REG_EMERGENCY_STOP:
            *value = status.safety_status.emergency_stop_active ? 1 : 0;
            break;
            
        case MODBUS_REG_ERROR_COUNT:
            *value = (uint16_t)status.error_count;
            break;
            
        case MODBUS_REG_UPTIME:
            *value = (uint16_t)(status.uptime_ms / 1000);  // Seconds
            break;
            
        default:
            return ERROR_COMM_INVALID_REGISTER;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Handle Modbus write register request
 * @param register_addr Register address
 * @param value Value to write
 * @return SystemError_t Success or error code
 */
SystemError_t modbus_write_register(uint16_t register_addr, uint16_t value) {
    switch (register_addr) {
        case MODBUS_REG_MOTOR1_TARGET:
            {
                float target_position = (float)value / 10.0f;  // 0.1° resolution
                return motor_move_to_position(0, target_position);
            }
            
        case MODBUS_REG_MOTOR2_TARGET:
            {
                float target_position = (float)value / 10.0f;
                return motor_move_to_position(1, target_position);
            }
            
        case MODBUS_REG_EMERGENCY_STOP:
            if (value == 1) {
                return execute_emergency_stop(ESTOP_SOURCE_MODBUS);
            } else if (value == 0) {
                return reset_emergency_stop();
            }
            return ERROR_COMM_INVALID_VALUE;
            
        default:
            return ERROR_COMM_INVALID_REGISTER;
    }
}
```

Remember: **Consistent API design enables easy integration, comprehensive documentation supports maintainability, robust error handling ensures reliable external communication, standardized interfaces promote interoperability.**
