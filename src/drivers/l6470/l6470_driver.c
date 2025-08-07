/**
 * @file l6470_driver.c
 * @brief L6470 Stepper Driver Implementation - STM32H7 SPI HAL
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @note L6470 stepper motor driver implementation following instruction files:
 * - .github/instructions/stm32h7-spi-l6470.instructions.md
 * - .github/instructions/l6470-registers.instructions.md
 * - .github/instructions/safety-systems.instructions.md
 */

#include "l6470_driver.h"
#include "common/error_codes.h"
#include "common/system_state.h"
#include "config/comm_config.h"
#include "config/hardware_config.h"
#include "config/l6470_registers_generated.h"
#include "config/motor_config.h"
#include "hal_abstraction/hal_abstraction.h"
#include "simulation/hardware_simulation.h"
#include <string.h>

/* ==========================================================================
 */
/* Private Variables and State Management                                    */
/* ==========================================================================
 */

static bool l6470_initialized = false;

// Driver state for each motor
typedef struct {
    bool is_initialized;
    uint16_t last_status;
    uint32_t last_command_time;
    uint32_t fault_count;
    MotorState_t current_state;
    bool simulation_mode; // Added for simulation framework integration
} L6470_DriverState_t;

static L6470_DriverState_t driver_state[L6470_MAX_DEVICES] = {0};

// SPI Communication Health Monitoring (per safety-systems.instructions.md)
typedef struct {
    uint32_t transaction_count;
    uint32_t error_count;
    uint32_t last_error_time;
    SystemError_t last_error_code;
} SPI_HealthMonitor_t;

static SPI_HealthMonitor_t spi_health = {0};

/* ==========================================================================
 */
/* Private Function Declarations                                             */
/* ==========================================================================
 */

static SystemError_t l6470_spi_transaction(uint8_t *tx_data, uint8_t *rx_data,
                                           uint16_t size);
static SystemError_t l6470_send_daisy_command(uint8_t motor1_cmd,
                                              uint32_t motor1_data,
                                              uint8_t motor2_cmd,
                                              uint32_t motor2_data);
static SystemError_t l6470_send_single_command(uint8_t motor_id,
                                               uint8_t command, uint32_t data);
static uint32_t l6470_pack_parameter(uint8_t register_addr, uint32_t value,
                                     uint8_t num_bytes);
static void l6470_unpack_parameter(uint8_t *rx_data, uint32_t *value,
                                   uint8_t num_bytes);
static SystemError_t l6470_validate_motor_id(uint8_t motor_id);

/* ==========================================================================
 */
/* Public API Implementation                                                 */
/* ==========================================================================
 */

/**
 * @brief Initialize L6470 driver system
 * @return System error code
 */
SystemError_t l6470_init(void) {
    // Initialize simulation framework if enabled
#if SIMULATION_ENABLED
    simulation_error_t sim_result = simulation_init(
        "schemas/l6470_registers.yaml", "schemas/as5600_registers.yaml");
    if (sim_result == SIM_OK) {
        // Enable simulation mode for all motors
        for (uint8_t i = 0; i < L6470_MAX_DEVICES; i++) {
            driver_state[i].simulation_mode = true;
        }
        l6470_initialized = true;
        return SYSTEM_OK; // Skip hardware initialization in simulation mode
    }
#endif

    // Initialize SPI instance for motor control
    SystemError_t spi_result = HAL_Abstraction_SPI_Init(HAL_SPI_INSTANCE_2);
    if (spi_result != SYSTEM_OK) {
        return spi_result;
    }

    // Initialize CS pin (ensure HIGH before first communication)
    HAL_Abstraction_GPIO_Write(HAL_GPIO_PORT_A, MOTOR_SPI_CS_PIN,
                               HAL_GPIO_STATE_SET);

    // Wait for L6470 power-up time (per datasheet: minimum 1ms)
    HAL_Abstraction_Delay(2);

    // Initialize each motor in the daisy chain
    for (uint8_t motor_id = 0; motor_id < L6470_MAX_DEVICES; motor_id++) {
        SystemError_t result = l6470_init_motor(motor_id);
        if (result != SYSTEM_OK) {
            return result;
        }
        driver_state[motor_id].simulation_mode = false; // Real hardware mode
    }

    l6470_initialized = true;

    return SYSTEM_OK;
}

/**
 * @brief Initialize individual motor configuration
 * @param motor_id Motor identifier (0-based)
 * @return System error code
 */
SystemError_t l6470_init_motor(uint8_t motor_id) {
    SystemError_t result = l6470_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }

    L6470_DriverState_t *state = &driver_state[motor_id];

    // Reset motor driver
    result = l6470_reset_device(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Configure motor parameters (from motor_config.h SSOT)
    result = l6470_set_parameter(motor_id, L6470_REG_ACC, L6470_ACC);
    if (result != SYSTEM_OK)
        return result;

    result = l6470_set_parameter(motor_id, L6470_REG_DEC, L6470_DEC);
    if (result != SYSTEM_OK)
        return result;

    result =
        l6470_set_parameter(motor_id, L6470_REG_MAX_SPEED, L6470_MAX_SPEED);
    if (result != SYSTEM_OK)
        return result;

    result =
        l6470_set_parameter(motor_id, L6470_REG_MIN_SPEED, L6470_MIN_SPEED);
    if (result != SYSTEM_OK)
        return result;

    result = l6470_set_parameter(motor_id, L6470_REG_FS_SPD, L6470_FS_SPD);
    if (result != SYSTEM_OK)
        return result;

    // Configure current control (KVAL registers)
    result =
        l6470_set_parameter(motor_id, L6470_REG_KVAL_HOLD, L6470_KVAL_HOLD);
    if (result != SYSTEM_OK)
        return result;

    result = l6470_set_parameter(motor_id, L6470_REG_KVAL_RUN, L6470_KVAL_RUN);
    if (result != SYSTEM_OK)
        return result;

    result = l6470_set_parameter(motor_id, L6470_REG_KVAL_ACC, L6470_KVAL_ACC);
    if (result != SYSTEM_OK)
        return result;

    result = l6470_set_parameter(motor_id, L6470_REG_KVAL_DEC, L6470_KVAL_DEC);
    if (result != SYSTEM_OK)
        return result;

    // Configure overcurrent protection
    result = l6470_set_parameter(motor_id, L6470_REG_OCD_TH, L6470_OCD_TH);
    if (result != SYSTEM_OK)
        return result;

    // Configure step mode
    result =
        l6470_set_parameter(motor_id, L6470_REG_STEP_MODE, L6470_STEP_MODE);
    if (result != SYSTEM_OK)
        return result;

    // Read initial status
    uint16_t status;
    result = l6470_get_status(motor_id, &status);
    if (result != SYSTEM_OK)
        return result;

    state->last_status = status;
    state->is_initialized = true;
    state->current_state = MOTOR_STATE_IDLE;
    state->last_command_time = HAL_Abstraction_GetTick();

    return SYSTEM_OK;
}

/**
 * @brief Reset L6470 device
 * @param motor_id Motor identifier
 * @return System error code
 */
SystemError_t l6470_reset_device(uint8_t motor_id) {
    SystemError_t result = l6470_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }

#if SIMULATION_ENABLED
    // Use simulation for device reset if enabled
    if (driver_state[motor_id].simulation_mode) {
        simulation_error_t sim_result =
            l6470_sim_send_command(L6470_CMD_RESET_DEVICE, 0);
        if (sim_result != SIM_OK) {
            return ERROR_MOTOR_COMMUNICATION_FAILED;
        }

        // Wait for reset completion (per datasheet)
        HAL_Abstraction_Delay(1);

        // Clear driver state
        driver_state[motor_id].is_initialized = false;
        driver_state[motor_id].fault_count = 0;

        return SYSTEM_OK;
    }
#endif

    // Send RESET_DEVICE command
    result = l6470_send_single_command(motor_id, L6470_CMD_RESET_DEVICE, 0);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Wait for reset completion (per datasheet)
    HAL_Abstraction_Delay(1);

    // Clear driver state
    driver_state[motor_id].is_initialized = false;
    driver_state[motor_id].fault_count = 0;

    return SYSTEM_OK;
}

/**
 * @brief Set L6470 register parameter
 * @param motor_id Motor identifier
 * @param register_addr Register address
 * @param value Parameter value
 * @return System error code
 */
SystemError_t l6470_set_parameter(uint8_t motor_id, uint8_t register_addr,
                                  uint32_t value) {
    SystemError_t result = l6470_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }

#if SIMULATION_ENABLED
    // Use simulation for parameter write if enabled
    if (driver_state[motor_id].simulation_mode) {
        // Convert SystemError_t to simulation_error_t if needed
        simulation_error_t sim_result =
            l6470_sim_write_register(register_addr, value);
        return (sim_result == SIM_OK) ? SYSTEM_OK
                                      : ERROR_MOTOR_COMMUNICATION_FAILED;
    }
#endif

    // Determine parameter size based on register
    uint8_t param_bytes = 3; // Default to 3 bytes, adjust per register

    switch (register_addr) {
    case L6470_REG_STEP_MODE:
    case L6470_REG_ALARM_EN:
    case L6470_REG_CONFIG:
    case L6470_REG_STATUS:
        param_bytes = 2;
        break;
    case L6470_REG_KVAL_HOLD:
    case L6470_REG_KVAL_RUN:
    case L6470_REG_KVAL_ACC:
    case L6470_REG_KVAL_DEC:
    case L6470_REG_OCD_TH:
    case L6470_REG_STALL_TH:
        param_bytes = 1;
        break;
    default:
        param_bytes = 3;
        break;
    }

    uint8_t command = L6470_CMD_SET_PARAM | register_addr;
    uint32_t packed_data =
        l6470_pack_parameter(register_addr, value, param_bytes);

    return l6470_send_single_command(motor_id, command, packed_data);
}

/**
 * @brief Get L6470 register parameter
 * @param motor_id Motor identifier
 * @param register_addr Register address
 * @param value Pointer to store parameter value
 * @return System error code
 */
SystemError_t l6470_get_parameter(uint8_t motor_id, uint8_t register_addr,
                                  uint32_t *value) {
    if (value == NULL) {
        return ERROR_MOTOR_CONFIG_INVALID;
    }

    SystemError_t result = l6470_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }

#if SIMULATION_ENABLED
    // Use simulation for parameter read if enabled
    if (driver_state[motor_id].simulation_mode) {
        simulation_error_t sim_result =
            l6470_sim_read_register(register_addr, value);
        return (sim_result == SIM_OK) ? SYSTEM_OK
                                      : ERROR_MOTOR_COMMUNICATION_FAILED;
    }
#endif

    uint8_t command = L6470_CMD_GET_PARAM | register_addr;

    // Prepare daisy-chain command
    uint8_t tx_buffer[8] = {0};
    uint8_t rx_buffer[8] = {0};

    if (motor_id == 0) {
        // Command motor 1, NOP for motor 2
        tx_buffer[0] = command;
        tx_buffer[1] = 0x00;
        tx_buffer[2] = 0x00;
        tx_buffer[3] = 0x00;
        tx_buffer[4] = L6470_CMD_NOP;
        tx_buffer[5] = 0x00;
        tx_buffer[6] = 0x00;
        tx_buffer[7] = 0x00;
    } else {
        // NOP for motor 1, command motor 2
        tx_buffer[0] = L6470_CMD_NOP;
        tx_buffer[1] = 0x00;
        tx_buffer[2] = 0x00;
        tx_buffer[3] = 0x00;
        tx_buffer[4] = command;
        tx_buffer[5] = 0x00;
        tx_buffer[6] = 0x00;
        tx_buffer[7] = 0x00;
    }

    result = l6470_spi_transaction(tx_buffer, rx_buffer, 8);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Extract parameter value from response
    uint8_t param_bytes = 3; // Adjust based on register
    uint8_t start_byte = (motor_id == 0) ? 0 : 4;

    l6470_unpack_parameter(&rx_buffer[start_byte], value, param_bytes);

    return SYSTEM_OK;
}

/**
 * @brief Get L6470 status register
 * @param motor_id Motor identifier
 * @param status Pointer to store status value
 * @return System error code
 */
SystemError_t l6470_get_status(uint8_t motor_id, uint16_t *status) {
    if (status == NULL) {
        return ERROR_MOTOR_CONFIG_INVALID;
    }

    SystemError_t result = l6470_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }

#if SIMULATION_ENABLED
    // Use simulation for status read if enabled
    if (driver_state[motor_id].simulation_mode) {
        uint32_t status_value = 0;
        simulation_error_t sim_result =
            l6470_sim_read_register(L6470_REG_STATUS, &status_value);
        if (sim_result == SIM_OK) {
            *status = (uint16_t)(status_value & 0xFFFF);
            return SYSTEM_OK;
        }
        return ERROR_MOTOR_COMMUNICATION_FAILED;
    }
#endif

    uint8_t tx_data[2] = {L6470_CMD_GET_STATUS, 0x00};
    uint8_t rx_data[2] = {0};

    // Assert CS
    HAL_Abstraction_GPIO_Write(HAL_GPIO_PORT_A, 9, HAL_GPIO_STATE_RESET);

    // Small delay for L6470 setup time (from datasheet)
    HAL_Abstraction_Delay(1);

    // SPI transaction using HAL abstraction
    HAL_SPI_Transaction_t transaction = {.tx_data = tx_data,
                                         .rx_data = rx_data,
                                         .data_size = 2,
                                         .timeout_ms = SPI_TIMEOUT_MS};
    SystemError_t spi_result =
        HAL_Abstraction_SPI_TransmitReceive(HAL_SPI_INSTANCE_2, &transaction);

    // Deassert CS
    HAL_Abstraction_GPIO_Write(HAL_GPIO_PORT_A, 9, HAL_GPIO_STATE_SET);

    if (spi_result != ERROR_CODE_SUCCESS) {
        spi_health.error_count++;
        spi_health.last_error_time = HAL_Abstraction_GetTick();
        spi_health.last_error_code = ERROR_SPI_TRANSMISSION_FAILED;
        return ERROR_SPI_TRANSMISSION_FAILED;
    }

    *status = (rx_data[0] << 8) | rx_data[1];

    // Update driver state
    driver_state[motor_id].last_status = *status;

    // Check for L6470 error flags (per safety-systems.instructions.md)
    if (*status & L6470_STATUS_UVLO) {
        driver_state[motor_id].fault_count++;
        return ERROR_MOTOR_UNDERVOLTAGE;
    }
    if (*status & L6470_STATUS_OCD) {
        driver_state[motor_id].fault_count++;
        return ERROR_MOTOR_OVERCURRENT;
    }
    if (*status & L6470_STATUS_TH_SD) {
        driver_state[motor_id].fault_count++;
        return ERROR_MOTOR_OVERHEAT;
    }
    if (*status & (L6470_STATUS_STEP_LOSS_A | L6470_STATUS_STEP_LOSS_B)) {
        driver_state[motor_id].fault_count++;
        return ERROR_MOTOR_STALL;
    }

    return SYSTEM_OK;
}

/**
 * @brief Move motor to absolute position
 * @param motor_id Motor identifier
 * @param position Target position in steps
 * @return System error code
 */
SystemError_t l6470_move_to_position(uint8_t motor_id, int32_t position) {
    SystemError_t result = l6470_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Validate position limits
    if (position < 0 || position > (int32_t)MOTOR_TOTAL_STEPS) {
        return ERROR_MOTOR_POSITION_OUT_OF_RANGE;
    }

#if SIMULATION_ENABLED
    // Use simulation for movement if enabled
    if (driver_state[motor_id].simulation_mode) {
        simulation_error_t sim_result =
            l6470_sim_send_command(L6470_CMD_GOTO, position);
        if (sim_result == SIM_OK) {
            driver_state[motor_id].current_state = MOTOR_STATE_RUNNING;
            driver_state[motor_id].last_command_time =
                HAL_Abstraction_GetTick();
            return SYSTEM_OK;
        }
        return ERROR_MOTOR_COMMUNICATION_FAILED;
    }
#endif

    uint8_t command = L6470_CMD_GOTO;
    uint32_t abs_position = (uint32_t)position & 0x3FFFFF; // 22-bit position

    result = l6470_send_single_command(motor_id, command, abs_position);
    if (result == SYSTEM_OK) {
        driver_state[motor_id].current_state = MOTOR_STATE_RUNNING;
        driver_state[motor_id].last_command_time = HAL_Abstraction_GetTick();
    }

    return result;
}

/**
 * @brief Stop motor with controlled deceleration
 * @param motor_id Motor identifier
 * @return System error code
 */
SystemError_t l6470_soft_stop(uint8_t motor_id) {
    SystemError_t result = l6470_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }

#if SIMULATION_ENABLED
    // Use simulation for stop command if enabled
    if (driver_state[motor_id].simulation_mode) {
        simulation_error_t sim_result =
            l6470_sim_send_command(L6470_CMD_SOFT_STOP, 0);
        if (sim_result == SIM_OK) {
            driver_state[motor_id].current_state = MOTOR_STATE_DECELERATING;
            driver_state[motor_id].last_command_time =
                HAL_Abstraction_GetTick();
            return SYSTEM_OK;
        }
        return ERROR_MOTOR_COMMUNICATION_FAILED;
    }
#endif

    result = l6470_send_single_command(motor_id, L6470_CMD_SOFT_STOP, 0);
    if (result == SYSTEM_OK) {
        driver_state[motor_id].current_state = MOTOR_STATE_DECELERATING;
        driver_state[motor_id].last_command_time = HAL_Abstraction_GetTick();
    }

    return result;
}

/**
 * @brief Stop motor immediately (emergency stop)
 * @param motor_id Motor identifier
 * @return System error code
 */
SystemError_t l6470_hard_stop(uint8_t motor_id) {
    SystemError_t result = l6470_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }

#if SIMULATION_ENABLED
    // Use simulation for emergency stop if enabled
    if (driver_state[motor_id].simulation_mode) {
        simulation_error_t sim_result =
            l6470_sim_send_command(L6470_CMD_HARD_STOP, 0);
        if (sim_result == SIM_OK) {
            driver_state[motor_id].current_state = MOTOR_STATE_EMERGENCY_STOP;
            driver_state[motor_id].last_command_time =
                HAL_Abstraction_GetTick();
            return SYSTEM_OK;
        }
        return ERROR_MOTOR_COMMUNICATION_FAILED;
    }
#endif

    result = l6470_send_single_command(motor_id, L6470_CMD_HARD_STOP, 0);
    if (result == SYSTEM_OK) {
        driver_state[motor_id].current_state = MOTOR_STATE_EMERGENCY_STOP;
        driver_state[motor_id].last_command_time = HAL_Abstraction_GetTick();
    }

    return result;
}

/**
 * @brief Set motor to high impedance state
 * @param motor_id Motor identifier
 * @return System error code
 */
SystemError_t l6470_hard_hiz(uint8_t motor_id) {
    SystemError_t result = l6470_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }

#if SIMULATION_ENABLED
    // Use simulation for high impedance command if enabled
    if (driver_state[motor_id].simulation_mode) {
        simulation_error_t sim_result =
            l6470_sim_send_command(L6470_CMD_HARD_HIZ, 0);
        if (sim_result == SIM_OK) {
            driver_state[motor_id].current_state = MOTOR_STATE_IDLE;
            driver_state[motor_id].last_command_time =
                HAL_Abstraction_GetTick();
            return SYSTEM_OK;
        }
        return ERROR_MOTOR_COMMUNICATION_FAILED;
    }
#endif

    result = l6470_send_single_command(motor_id, L6470_CMD_HARD_HIZ, 0);
    if (result == SYSTEM_OK) {
        driver_state[motor_id].current_state = MOTOR_STATE_IDLE;
        driver_state[motor_id].last_command_time = HAL_Abstraction_GetTick();
    }

    return result;
}

/* ==========================================================================
 */
/* Private Function Implementations                                          */
/* ==========================================================================
 */

/**
 * @brief Safe SPI transaction with health monitoring
 * @param tx_data Transmit buffer
 * @param rx_data Receive buffer
 * @param size Data size in bytes
 * @return System error code
 */
static SystemError_t l6470_spi_transaction(uint8_t *tx_data, uint8_t *rx_data,
                                           uint16_t size) {
    if (tx_data == NULL || rx_data == NULL) {
        return ERROR_MOTOR_CONFIG_INVALID;
    }

    spi_health.transaction_count++;

    // Assert CS (active low) - using HAL abstraction
    SystemError_t gpio_result =
        HAL_Abstraction_GPIO_Write(HAL_GPIO_PORT_A, 9, HAL_GPIO_STATE_RESET);
    if (gpio_result != SYSTEM_OK) {
        spi_health.error_count++;
        spi_health.last_error_time = HAL_Abstraction_GetTick();
        spi_health.last_error_code = ERROR_GPIO_WRITE_FAILED;
        return ERROR_GPIO_WRITE_FAILED;
    }

    // Small setup delay (per L6470 datasheet)
    HAL_Abstraction_Delay(1);

    // Prepare SPI transaction using HAL abstraction
    HAL_SPI_Transaction_t transaction = {.tx_data = tx_data,
                                         .rx_data = rx_data,
                                         .data_size = size,
                                         .timeout_ms = SPI_TIMEOUT_MS};

    SystemError_t spi_result =
        HAL_Abstraction_SPI_TransmitReceive(HAL_SPI_INSTANCE_2, &transaction);

    // Deassert CS (active low, so set high)
    gpio_result =
        HAL_Abstraction_GPIO_Write(HAL_GPIO_PORT_A, 9, HAL_GPIO_STATE_SET);
    if (gpio_result != SYSTEM_OK) {
        spi_health.error_count++;
        spi_health.last_error_time = HAL_Abstraction_GetTick();
        spi_health.last_error_code = ERROR_GPIO_WRITE_FAILED;
        return ERROR_GPIO_WRITE_FAILED;
    }

    if (spi_result != SYSTEM_OK) {
        spi_health.error_count++;
        spi_health.last_error_time = HAL_Abstraction_GetTick();
        spi_health.last_error_code = ERROR_SPI_TRANSMISSION_FAILED;
        return ERROR_SPI_TRANSMISSION_FAILED;
    }

    return SYSTEM_OK;
}

/**
 * @brief Send command to motors in daisy-chain configuration
 * @param motor1_cmd Command for motor 1
 * @param motor1_data Data for motor 1
 * @param motor2_cmd Command for motor 2
 * @param motor2_data Data for motor 2
 * @return System error code
 */
static SystemError_t l6470_send_daisy_command(uint8_t motor1_cmd,
                                              uint32_t motor1_data,
                                              uint8_t motor2_cmd,
                                              uint32_t motor2_data) {
    uint8_t tx_buffer[8];
    uint8_t rx_buffer[8];

    // Pack daisy-chain data (motor 1 first, motor 2 second)
    tx_buffer[0] = motor1_cmd;
    tx_buffer[1] = (motor1_data >> 16) & 0xFF;
    tx_buffer[2] = (motor1_data >> 8) & 0xFF;
    tx_buffer[3] = motor1_data & 0xFF;
    tx_buffer[4] = motor2_cmd;
    tx_buffer[5] = (motor2_data >> 16) & 0xFF;
    tx_buffer[6] = (motor2_data >> 8) & 0xFF;
    tx_buffer[7] = motor2_data & 0xFF;

    return l6470_spi_transaction(tx_buffer, rx_buffer, 8);
}

/**
 * @brief Send command to single motor while maintaining daisy-chain
 * @param motor_id Motor identifier (0 or 1)
 * @param command Command byte
 * @param data Command data
 * @return System error code
 */
static SystemError_t
l6470_send_single_command(uint8_t motor_id, uint8_t command, uint32_t data) {
    if (motor_id == 0) {
        // Command motor 1, send NOP to motor 2
        return l6470_send_daisy_command(command, data, L6470_CMD_NOP, 0);
    } else if (motor_id == 1) {
        // Command motor 2, send NOP to motor 1
        return l6470_send_daisy_command(L6470_CMD_NOP, 0, command, data);
    } else {
        return ERROR_MOTOR_INVALID_ID;
    }
}

/**
 * @brief Pack parameter value for transmission
 * @param register_addr Register address
 * @param value Parameter value
 * @param num_bytes Number of bytes for parameter
 * @return Packed parameter value
 */
static uint32_t l6470_pack_parameter(uint8_t register_addr, uint32_t value,
                                     uint8_t num_bytes) {
    (void)register_addr; // Unused for now

    switch (num_bytes) {
    case 1:
        return value & 0xFF;
    case 2:
        return value & 0xFFFF;
    case 3:
    default:
        return value & 0xFFFFFF;
    }
}

/**
 * @brief Unpack parameter value from received data
 * @param rx_data Received data buffer
 * @param value Pointer to store unpacked value
 * @param num_bytes Number of bytes to unpack
 */
static void l6470_unpack_parameter(uint8_t *rx_data, uint32_t *value,
                                   uint8_t num_bytes) {
    *value = 0;

    for (uint8_t i = 0; i < num_bytes; i++) {
        *value |= ((uint32_t)rx_data[i]) << (8 * (num_bytes - 1 - i));
    }
}

/**
 * @brief Validate motor ID parameter
 * @param motor_id Motor identifier to validate
 * @return System error code
 */
static SystemError_t l6470_validate_motor_id(uint8_t motor_id) {
    if (motor_id >= L6470_MAX_DEVICES) {
        return ERROR_MOTOR_INVALID_ID;
    }

    if (!l6470_initialized) {
        return ERROR_MOTOR_INIT_FAILED;
    }

    return SYSTEM_OK;
}

/* ==========================================================================
 */
/* Diagnostic and Status Functions                                           */
/* ==========================================================================
 */

/**
 * @brief Get SPI communication health statistics
 * @param health Pointer to health structure
 * @return System error code
 */
SystemError_t l6470_get_spi_health(SPI_HealthMonitor_t *health) {
    if (health == NULL) {
        return ERROR_MOTOR_CONFIG_INVALID;
    }

    memcpy(health, &spi_health, sizeof(SPI_HealthMonitor_t));
    return SYSTEM_OK;
}

/**
 * @brief Get driver state for specified motor
 * @param motor_id Motor identifier
 * @param state Pointer to state structure
 * @return System error code
 */
SystemError_t l6470_get_driver_state(uint8_t motor_id,
                                     L6470_DriverState_t *state) {
    if (state == NULL) {
        return ERROR_MOTOR_CONFIG_INVALID;
    }

    SystemError_t result = l6470_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }

    memcpy(state, &driver_state[motor_id], sizeof(L6470_DriverState_t));
    return SYSTEM_OK;
}

/**
 * @brief Check if L6470 driver system is initialized
 * @return true if initialized, false otherwise
 */
bool l6470_is_initialized(void) {
    return l6470_initialized;
}

/**
 * @brief Run motor at constant velocity
 * @param motor_id Motor identifier (0 or 1)
 * @param direction true for forward, false for reverse
 * @param speed Speed in steps/second
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 */
SystemError_t l6470_run(uint8_t motor_id, bool direction, float speed) {
    if (motor_id >= L6470_MAX_DEVICES) {
        return ERROR_MOTOR_INVALID_ID;
    }

    if (!l6470_initialized) {
        return ERROR_MOTOR_INIT_FAILED;
    }

#if SIMULATION_ENABLED
    // Use simulation for run command if enabled
    if (driver_state[motor_id].simulation_mode) {
        // Convert speed to L6470 format
        uint32_t l6470_speed = (uint32_t)(speed / 4000000.0f * 0x10000000UL);
        if (l6470_speed > 0x3FF) {
            l6470_speed = 0x3FF;
        }

        uint8_t command = L6470_CMD_RUN;
        if (direction) {
            command |= 0x01; // Forward direction
        }

        simulation_error_t sim_result =
            l6470_sim_send_command(command, l6470_speed);
        return (sim_result == SIM_OK) ? SYSTEM_OK
                                      : ERROR_MOTOR_COMMUNICATION_FAILED;
    }
#endif

    // Convert speed to L6470 format (step/tick)
    // L6470 uses 250ns ticks, so steps/second = steps/tick * 4000000
    uint32_t l6470_speed = (uint32_t)(speed / 4000000.0f * 0x10000000UL);

    // Clamp to valid range (0 to 0x3FF)
    if (l6470_speed > 0x3FF) {
        l6470_speed = 0x3FF;
    }

    // Build RUN command with direction
    uint8_t command = L6470_CMD_RUN;
    if (direction) {
        command |= 0x01; // Forward direction
    }

    // Send command via daisy chain
    if (motor_id == 0) {
        // Motor 0: send command, Motor 1: NOP
        return l6470_send_daisy_command(command, l6470_speed, L6470_CMD_NOP,
                                        0);
    } else {
        // Motor 0: NOP, Motor 1: send command
        return l6470_send_daisy_command(L6470_CMD_NOP, 0, command,
                                        l6470_speed);
    }
}

/**
 * @brief Reset motor position to zero
 * @param motor_id Motor identifier (0 or 1)
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 */
SystemError_t l6470_reset_position(uint8_t motor_id) {
    if (motor_id >= L6470_MAX_DEVICES) {
        return ERROR_MOTOR_INVALID_ID;
    }

    if (!l6470_initialized) {
        return ERROR_MOTOR_INIT_FAILED;
    }

#if SIMULATION_ENABLED
    // Use simulation for position reset if enabled
    if (driver_state[motor_id].simulation_mode) {
        simulation_error_t sim_result =
            l6470_sim_send_command(L6470_CMD_RESET_POS, 0);
        return (sim_result == SIM_OK) ? SYSTEM_OK
                                      : ERROR_MOTOR_COMMUNICATION_FAILED;
    }
#endif

    // Send RESET_POS command
    if (motor_id == 0) {
        // Motor 0: send command, Motor 1: NOP
        return l6470_send_daisy_command(L6470_CMD_RESET_POS, 0x00,
                                        L6470_CMD_NOP, 0x00);
    } else {
        // Motor 0: NOP, Motor 1: send command
        return l6470_send_daisy_command(L6470_CMD_NOP, 0x00,
                                        L6470_CMD_RESET_POS, 0x00);
    }
}

/* ==========================================================================
 */
/* Simulation Function Stubs (ARM Build Compatibility)                      */
/* ==========================================================================
 */

#if SIMULATION_ENABLED
/**
 * @brief Simulation stub for L6470 command sending
 * @param command L6470 command to simulate
 * @param parameter Command parameter
 * @return simulation_error_t Always returns SIM_OK for ARM builds
 */
simulation_error_t l6470_sim_send_command(uint8_t command,
                                          uint32_t parameter) {
    // Stub implementation for ARM firmware builds
    (void)command; // Suppress unused parameter warnings
    (void)parameter;
    return SIM_OK;
}

/**
 * @brief Simulation stub for L6470 register writing
 * @param reg_addr Register address
 * @param value Value to write
 * @return simulation_error_t Always returns SIM_OK for ARM builds
 */
simulation_error_t l6470_sim_write_register(uint8_t reg_addr, uint32_t value) {
    // Stub implementation for ARM firmware builds
    (void)reg_addr; // Suppress unused parameter warnings
    (void)value;
    return SIM_OK;
}

/**
 * @brief Simulation stub for L6470 register reading
 * @param reg_addr Register address
 * @param value Pointer to store read value
 * @return simulation_error_t Always returns SIM_OK for ARM builds
 */
simulation_error_t l6470_sim_read_register(uint8_t reg_addr, uint32_t *value) {
    // Stub implementation for ARM firmware builds
    (void)reg_addr; // Suppress unused parameter warnings
    if (value != NULL) {
        *value = 0x00; // Return default value for ARM builds
    }
    return SIM_OK;
}
#endif /* SIMULATION_ENABLED */
