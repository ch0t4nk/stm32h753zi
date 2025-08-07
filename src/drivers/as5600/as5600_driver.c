/**
 * @file as5600_driver.c
 * @brief AS5600 Magnetic Encoder Driver Implementation using HAL abstraction
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @note AS5600 12-bit magnetic encoder driver implementation following:
 * - .github/instructions/stm32h7-i2c-as5600.instructions.md
 * - .github/instructions/error-handling.instructions.md
 * - .github/instructions/units-measurements.instructions.md
 */

#include "as5600_driver.h"
#include "common/error_codes.h"
#include "common/system_state.h"
#include "config/as5600_registers_generated.h"
#include "config/comm_config.h"
#include "config/hardware_config.h"
#include "config/motor_config.h"
#include "hal_abstraction/hal_abstraction.h"
#include "simulation/motor_simulation.h"
#include <math.h>
#include <string.h>

/* ==========================================================================
 */
/* Private Variables and State Management                                    */
/* ==========================================================================
 */

static bool as5600_initialized = false;

// Encoder state for each device (dual I2C bus configuration)
typedef struct {
    bool is_initialized;
    HAL_I2C_Instance_t i2c_instance; // HAL abstraction I2C instance
    uint8_t i2c_address;
    uint16_t raw_angle;
    uint16_t filtered_angle;
    float angle_degrees;
    float previous_angle;
    float velocity_dps;
    float zero_position_deg; // Zero reference position
    uint32_t last_read_time;
    uint32_t last_update_time; // Last calibration/update time
    uint16_t magnitude;
    uint8_t status_flags;
    uint32_t read_count;
    uint32_t error_count;
    bool magnet_detected;
    bool calibrated;
    uint16_t zero_offset; // Zero offset for calibration
    bool simulation_mode; // Added for simulation framework integration
} AS5600_EncoderState_t;

static AS5600_EncoderState_t encoder_state[AS5600_MAX_ENCODERS] = {0};

/* ==========================================================================
 */
/* Private Function Declarations                                             */
/* ==========================================================================
 */

static SystemError_t as5600_i2c_read_register(uint8_t encoder_id,
                                              uint8_t reg_addr, uint8_t *data);
static SystemError_t as5600_i2c_write_register(uint8_t encoder_id,
                                               uint8_t reg_addr, uint8_t data);
static SystemError_t as5600_i2c_read_16bit(uint8_t encoder_id,
                                           uint8_t reg_addr_high,
                                           uint16_t *value);
static SystemError_t as5600_validate_encoder_id(uint8_t encoder_id);
static float as5600_raw_to_degrees(uint16_t raw_value);
static SystemError_t as5600_check_magnet_status(uint8_t encoder_id);
static void as5600_calculate_velocity(uint8_t encoder_id);

/* ==========================================================================
 */
/* Public API Implementation                                                 */
/* ==========================================================================
 */

/**
 * @brief Initialize AS5600 encoders using HAL abstraction
 * @return System error code
 */
SystemError_t as5600_init(void) {
    // Initialize I2C peripherals through HAL abstraction
    SystemError_t result = HAL_Abstraction_I2C_Init(HAL_I2C_INSTANCE_1);
    if (result != SYSTEM_OK) {
        return result;
    }

    result = HAL_Abstraction_I2C_Init(HAL_I2C_INSTANCE_2);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Initialize each encoder
    for (uint8_t encoder_id = 0; encoder_id < AS5600_MAX_ENCODERS;
         encoder_id++) {
        SystemError_t result = as5600_init_encoder(encoder_id);
        if (result != SYSTEM_OK) {
            return result;
        }
    }

    as5600_initialized = true;

    return SYSTEM_OK;
}

/**
 * @brief Initialize individual encoder
 * @param encoder_id Encoder identifier (0 or 1)
 * @return System error code
 */
SystemError_t as5600_init_encoder(uint8_t encoder_id) {
    SystemError_t result = as5600_validate_encoder_id(encoder_id);
    if (result != SYSTEM_OK) {
        return result;
    }

    AS5600_EncoderState_t *state = &encoder_state[encoder_id];

    // Configure encoder state using HAL abstraction
    state->i2c_instance =
        (encoder_id == 0) ? HAL_I2C_INSTANCE_1 : HAL_I2C_INSTANCE_2;
    state->i2c_address = AS5600_I2C_ADDRESS_8BIT;
    state->last_read_time = HAL_Abstraction_GetTick();

#if SIMULATION_ENABLED
    // Check if we're in simulation mode
    state->simulation_mode = motor_simulation_is_active();
#else
    state->simulation_mode = false;
#endif

    // Test I2C communication by reading status register
    uint8_t status;
    result = as5600_i2c_read_register(encoder_id, AS5600_REG_STATUS, &status);
    if (result != SYSTEM_OK) {
        return ERROR_ENCODER_COMMUNICATION;
    }

    state->status_flags = status;

    // Check magnet detection
    result = as5600_check_magnet_status(encoder_id);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Read initial angle values
    result = as5600_read_raw_angle(encoder_id, &state->raw_angle);
    if (result != SYSTEM_OK) {
        return result;
    }

    result = as5600_read_angle(encoder_id, &state->filtered_angle);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Convert to degrees and initialize previous angle
    state->angle_degrees = as5600_raw_to_degrees(state->filtered_angle);
    state->previous_angle = state->angle_degrees;

    // Read magnitude for magnet strength validation
    result = as5600_read_magnitude(encoder_id, &state->magnitude);
    if (result != SYSTEM_OK) {
        return result;
    }

    state->is_initialized = true;
    state->read_count = 1;

    return SYSTEM_OK;
}

/**
 * @brief Read raw angle from AS5600
 * @param encoder_id Encoder identifier
 * @param raw_angle Pointer to store raw angle value (0-4095)
 * @return System error code
 */
SystemError_t as5600_read_raw_angle(uint8_t encoder_id, uint16_t *raw_angle) {
    if (raw_angle == NULL) {
        return ERROR_ENCODER_CONFIG_INVALID;
    }

    SystemError_t result = as5600_validate_encoder_id(encoder_id);
    if (result != SYSTEM_OK) {
        return result;
    }

    result =
        as5600_i2c_read_16bit(encoder_id, AS5600_REG_RAW_ANGLE_H, raw_angle);
    if (result == SYSTEM_OK) {
        // Mask to 12-bit value
        *raw_angle &= ENCODER_VALUE_MASK;
        encoder_state[encoder_id].raw_angle = *raw_angle;
        encoder_state[encoder_id].read_count++;
        encoder_state[encoder_id].last_read_time = HAL_Abstraction_GetTick();
    } else {
        encoder_state[encoder_id].error_count++;
    }

    return result;
}

/**
 * @brief Read filtered angle from AS5600
 * @param encoder_id Encoder identifier
 * @param angle Pointer to store filtered angle value (0-4095)
 * @return System error code
 */
SystemError_t as5600_read_angle(uint8_t encoder_id, uint16_t *angle) {
    if (angle == NULL) {
        return ERROR_ENCODER_CONFIG_INVALID;
    }

    SystemError_t result = as5600_validate_encoder_id(encoder_id);
    if (result != SYSTEM_OK) {
        return result;
    }

    result = as5600_i2c_read_16bit(encoder_id, AS5600_REG_ANGLE_H, angle);
    if (result == SYSTEM_OK) {
        // Mask to 12-bit value
        *angle &= ENCODER_VALUE_MASK;
        encoder_state[encoder_id].filtered_angle = *angle;
        encoder_state[encoder_id].read_count++;
        encoder_state[encoder_id].last_read_time = HAL_Abstraction_GetTick();
    } else {
        encoder_state[encoder_id].error_count++;
    }

    return result;
}

/**
 * @brief Read angle in degrees
 * @param encoder_id Encoder identifier
 * @param angle_degrees Pointer to store angle in degrees (0.0 to 360.0)
 * @return System error code
 */
SystemError_t as5600_read_angle_degrees(uint8_t encoder_id,
                                        float *angle_degrees) {
    if (angle_degrees == NULL) {
        return ERROR_ENCODER_CONFIG_INVALID;
    }

    uint16_t raw_angle;
    SystemError_t result = as5600_read_angle(encoder_id, &raw_angle);
    if (result != SYSTEM_OK) {
        return result;
    }

    *angle_degrees = as5600_raw_to_degrees(raw_angle);

    // Update encoder state
    AS5600_EncoderState_t *state = &encoder_state[encoder_id];
    state->previous_angle = state->angle_degrees;
    state->angle_degrees = *angle_degrees;

    // Calculate velocity
    as5600_calculate_velocity(encoder_id);

    return SYSTEM_OK;
}

/**
 * @brief Read magnet magnitude
 * @param encoder_id Encoder identifier
 * @param magnitude Pointer to store magnitude value
 * @return System error code
 */
SystemError_t as5600_read_magnitude(uint8_t encoder_id, uint16_t *magnitude) {
    if (magnitude == NULL) {
        return ERROR_ENCODER_CONFIG_INVALID;
    }

    SystemError_t result = as5600_validate_encoder_id(encoder_id);
    if (result != SYSTEM_OK) {
        return result;
    }

    result =
        as5600_i2c_read_16bit(encoder_id, AS5600_REG_MAGNITUDE_H, magnitude);
    if (result == SYSTEM_OK) {
        encoder_state[encoder_id].magnitude = *magnitude;
    }

    return result;
}

/**
 * @brief Read status register
 * @param encoder_id Encoder identifier
 * @param status Pointer to store status flags
 * @return System error code
 */
SystemError_t as5600_read_status(uint8_t encoder_id, uint8_t *status) {
    if (status == NULL) {
        return ERROR_ENCODER_CONFIG_INVALID;
    }

    SystemError_t result = as5600_validate_encoder_id(encoder_id);
    if (result != SYSTEM_OK) {
        return result;
    }

    result = as5600_i2c_read_register(encoder_id, AS5600_REG_STATUS, status);
    if (result == SYSTEM_OK) {
        encoder_state[encoder_id].status_flags = *status;

        // Update magnet detection status
        encoder_state[encoder_id].magnet_detected =
            (*status & AS5600_STATUS_MD) != 0;
    }

    return result;
}

/**
 * @brief Get encoder velocity in degrees per second
 * @param encoder_id Encoder identifier
 * @param velocity_dps Pointer to store velocity
 * @return System error code
 */
SystemError_t as5600_get_velocity(uint8_t encoder_id, float *velocity_dps) {
    if (velocity_dps == NULL) {
        return ERROR_ENCODER_CONFIG_INVALID;
    }

    SystemError_t result = as5600_validate_encoder_id(encoder_id);
    if (result != SYSTEM_OK) {
        return result;
    }

    *velocity_dps = encoder_state[encoder_id].velocity_dps;
    return SYSTEM_OK;
}

/**
 * @brief Check if magnet is properly positioned
 * @param encoder_id Encoder identifier
 * @param magnet_ok Pointer to store magnet status
 * @return System error code
 */
SystemError_t as5600_check_magnet(uint8_t encoder_id, bool *magnet_ok) {
    if (magnet_ok == NULL) {
        return ERROR_ENCODER_CONFIG_INVALID;
    }

    uint8_t status;
    SystemError_t result = as5600_read_status(encoder_id, &status);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Check magnet status flags
    bool magnet_too_strong = (status & AS5600_STATUS_MH) != 0;
    bool magnet_too_weak = (status & AS5600_STATUS_ML) != 0;
    bool magnet_detected = (status & AS5600_STATUS_MD) != 0;

    *magnet_ok = magnet_detected && !magnet_too_strong && !magnet_too_weak;

    return SYSTEM_OK;
}

/**
 * @brief Calibrate encoder zero position
 * @param encoder_id Encoder identifier
 * @param current_angle Current mechanical angle (degrees)
 * @return System error code
 */
SystemError_t as5600_calibrate_zero(uint8_t encoder_id, float current_angle) {
    SystemError_t result = as5600_validate_encoder_id(encoder_id);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Read current raw angle
    uint16_t raw_angle;
    result = as5600_read_raw_angle(encoder_id, &raw_angle);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Calculate zero offset
    float raw_degrees = as5600_raw_to_degrees(raw_angle);
    float offset_degrees = raw_degrees - current_angle;

    // Normalize offset to 0-360 range
    while (offset_degrees < 0.0f)
        offset_degrees += 360.0f;
    while (offset_degrees >= 360.0f)
        offset_degrees -= 360.0f;

    // Convert offset to raw value
    uint16_t zero_position =
        (uint16_t)((offset_degrees / 360.0f) * 4095.0f + 0.5f);

    // Write zero position to AS5600 (requires PROG pin low - not implemented)
    // For now, just store in state for software compensation
    encoder_state[encoder_id].calibrated = true;
    encoder_state[encoder_id].zero_offset =
        zero_position; // Use the calculated value

    return SYSTEM_OK;
}

/**
 * @brief Set encoder zero position reference
 * @param encoder_id Encoder identifier
 * @param zero_position_deg Zero position in degrees
 * @return SystemError_t System error code
 */
SystemError_t as5600_set_zero_position(uint8_t encoder_id,
                                       float zero_position_deg) {
    SystemError_t result = as5600_validate_encoder_id(encoder_id);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Validate zero position range
    if (zero_position_deg < 0.0f || zero_position_deg >= 360.0f) {
        return ERROR_ENCODER_OUT_OF_RANGE;
    }

    // Store zero reference position for this encoder
    AS5600_EncoderState_t *state = &encoder_state[encoder_id];
    state->zero_position_deg = zero_position_deg;

    // Update calibration timestamp
    state->last_update_time = HAL_Abstraction_GetTick();

    return SYSTEM_OK;
}

/* ==========================================================================
 */
/* Private Function Implementations                                          */
/* ==========================================================================
 */

/**
 * @brief Convert simulation bool result to system error
 * @param sim_success Simulation function success (true/false)
 * @return Corresponding system error code
 */
#if SIMULATION_ENABLED
static SystemError_t as5600_simulation_bool_to_system_error(bool sim_success) {
    return sim_success ? SYSTEM_OK : ERROR_ENCODER_COMMUNICATION;
}
#endif

/**
 * @brief Read single register from AS5600
 * @param encoder_id Encoder identifier
 * @param reg_addr Register address
 * @param data Pointer to store read data
 * @return System error code
 */
static SystemError_t
as5600_i2c_read_register(uint8_t encoder_id, uint8_t reg_addr, uint8_t *data) {
    AS5600_EncoderState_t *state = &encoder_state[encoder_id];

#if SIMULATION_ENABLED
    if (state->simulation_mode) {
        // Use simulation backend
        bool sim_result = as5600_sim_read_register(encoder_id, reg_addr, data);
        return as5600_simulation_bool_to_system_error(sim_result);
    }
#endif

    HAL_I2C_Transaction_t transaction = {.device_address = state->i2c_address,
                                         .register_address = reg_addr,
                                         .data = data,
                                         .data_size = 1,
                                         .timeout_ms = AS5600_I2C_TIMEOUT,
                                         .use_register_address = true};

    SystemError_t result = HAL_Abstraction_I2C_MemRead(
        encoder_id == 0 ? HAL_I2C_INSTANCE_1 : HAL_I2C_INSTANCE_2,
        &transaction);

    if (result != SYSTEM_OK) {
        return ERROR_ENCODER_COMMUNICATION;
    }

    return SYSTEM_OK;
}

/**
 * @brief Write single register to AS5600
 * @param encoder_id Encoder identifier
 * @param reg_addr Register address
 * @param data Data to write
 * @return System error code
 */
/**
 * @brief Write register to AS5600 encoder (Future use - requires PROG pin)
 * @param encoder_id Encoder identifier (0 or 1)
 * @param reg_addr Register address
 * @param data Data to write
 * @return System error code
 * @note Currently unused - reserved for future OTP programming features
 */
__attribute__((unused)) static SystemError_t
as5600_i2c_write_register(uint8_t encoder_id, uint8_t reg_addr, uint8_t data) {
    AS5600_EncoderState_t *state = &encoder_state[encoder_id];

#if SIMULATION_ENABLED
    if (state->simulation_mode) {
        // AS5600 write operations are rarely used in simulation
        // For now, just return success for simulation compatibility
        // TODO: Implement proper AS5600 write simulation if needed
        return SYSTEM_OK;
    }
#endif

    HAL_I2C_Transaction_t transaction = {.device_address = state->i2c_address,
                                         .register_address = reg_addr,
                                         .data = &data,
                                         .data_size = 1,
                                         .timeout_ms = AS5600_I2C_TIMEOUT,
                                         .use_register_address = true};

    SystemError_t result = HAL_Abstraction_I2C_MemWrite(
        encoder_id == 0 ? HAL_I2C_INSTANCE_1 : HAL_I2C_INSTANCE_2,
        &transaction);

    if (result != SYSTEM_OK) {
        return ERROR_ENCODER_COMMUNICATION;
    }

    return SYSTEM_OK;
}

/**
 * @brief Read 16-bit value from AS5600 (MSB first)
 * @param encoder_id Encoder identifier
 * @param reg_addr_high High byte register address
 * @param value Pointer to store 16-bit value
 * @return System error code
 */
static SystemError_t as5600_i2c_read_16bit(uint8_t encoder_id,
                                           uint8_t reg_addr_high,
                                           uint16_t *value) {
    AS5600_EncoderState_t *state = &encoder_state[encoder_id];

#if SIMULATION_ENABLED
    if (state->simulation_mode) {
        // Use simulation backend for 16-bit reads
        uint8_t high_byte, low_byte;
        bool sim_result;

        // Read high byte
        sim_result =
            as5600_sim_read_register(encoder_id, reg_addr_high, &high_byte);
        if (!sim_result) {
            return ERROR_ENCODER_COMMUNICATION;
        }

        // Read low byte
        sim_result =
            as5600_sim_read_register(encoder_id, reg_addr_high + 1, &low_byte);
        if (!sim_result) {
            return ERROR_ENCODER_COMMUNICATION;
        }

        // Combine high and low bytes (MSB first)
        *value = ((uint16_t)high_byte << 8) | low_byte;
        return SYSTEM_OK;
    }
#endif

    uint8_t data[2];

    HAL_I2C_Transaction_t transaction = {.device_address = state->i2c_address,
                                         .register_address = reg_addr_high,
                                         .data = data,
                                         .data_size = 2,
                                         .timeout_ms = AS5600_I2C_TIMEOUT,
                                         .use_register_address = true};

    // Read 2 bytes starting from high byte register
    SystemError_t result = HAL_Abstraction_I2C_MemRead(
        encoder_id == 0 ? HAL_I2C_INSTANCE_1 : HAL_I2C_INSTANCE_2,
        &transaction);

    if (result != SYSTEM_OK) {
        return ERROR_ENCODER_COMMUNICATION;
    }

    // Combine high and low bytes (MSB first)
    *value = ((uint16_t)data[0] << 8) | data[1];

    return SYSTEM_OK;
}

/**
 * @brief Validate encoder ID parameter
 * @param encoder_id Encoder identifier to validate
 * @return System error code
 */
static SystemError_t as5600_validate_encoder_id(uint8_t encoder_id) {
    if (encoder_id >= AS5600_MAX_ENCODERS) {
        return ERROR_ENCODER_INVALID_ID;
    }

    if (!as5600_initialized) {
        return ERROR_ENCODER_INIT_FAILED;
    }

    if (!encoder_state[encoder_id].is_initialized) {
        return ERROR_ENCODER_CONFIG_INVALID;
    }

    return SYSTEM_OK;
}

/**
 * @brief Convert raw AS5600 value to degrees
 * @param raw_value Raw 12-bit value (0-4095)
 * @return Angle in degrees (0.0 to 360.0)
 */
static float as5600_raw_to_degrees(uint16_t raw_value) {
    // AS5600 is 12-bit: 4096 counts = 360 degrees
    return ((float)raw_value / 4096.0f) * 360.0f;
}

/**
 * @brief Check magnet status and update state
 * @param encoder_id Encoder identifier
 * @return System error code
 */
static SystemError_t as5600_check_magnet_status(uint8_t encoder_id) {
    uint8_t status;
    SystemError_t result = as5600_read_status(encoder_id, &status);
    if (result != SYSTEM_OK) {
        return result;
    }

    AS5600_EncoderState_t *state = &encoder_state[encoder_id];

    // Check magnet detection
    if (!(status & AS5600_STATUS_MD)) {
        state->magnet_detected = false;
        return ERROR_ENCODER_MAGNET_NOT_DETECTED;
    }

    // Check magnet strength
    if (status & AS5600_STATUS_MH) {
        return ERROR_ENCODER_MAGNET_TOO_STRONG;
    }

    if (status & AS5600_STATUS_ML) {
        return ERROR_ENCODER_MAGNET_TOO_WEAK;
    }

    state->magnet_detected = true;
    return SYSTEM_OK;
}

/**
 * @brief Calculate angular velocity from position difference
 * @param encoder_id Encoder identifier
 */
static void as5600_calculate_velocity(uint8_t encoder_id) {
    AS5600_EncoderState_t *state = &encoder_state[encoder_id];
    uint32_t current_time = HAL_Abstraction_GetTick();

    if (state->read_count <= 1) {
        state->velocity_dps = 0.0f;
        return;
    }

    // Calculate time difference
    uint32_t time_diff = current_time - state->last_read_time;
    if (time_diff == 0) {
        return; // Avoid division by zero
    }

    // Calculate angle difference (handle 360/0 degree wraparound)
    float angle_diff = state->angle_degrees - state->previous_angle;

    // Handle wraparound
    if (angle_diff > 180.0f) {
        angle_diff -= 360.0f;
    } else if (angle_diff < -180.0f) {
        angle_diff += 360.0f;
    }

    // Calculate velocity in degrees per second
    float time_diff_s = (float)time_diff / 1000.0f;
    state->velocity_dps = angle_diff / time_diff_s;
}

/* ==========================================================================
 */
/* Diagnostic and Status Functions                                           */
/* ==========================================================================
 */

/**
 * @brief Get encoder state information
 * @param encoder_id Encoder identifier
 * @param state Pointer to state structure
 * @return System error code
 */
SystemError_t as5600_get_encoder_state(uint8_t encoder_id,
                                       AS5600_EncoderState_t *state) {
    if (state == NULL) {
        return ERROR_ENCODER_CONFIG_INVALID;
    }

    SystemError_t result = as5600_validate_encoder_id(encoder_id);
    if (result != SYSTEM_OK) {
        return result;
    }

    memcpy(state, &encoder_state[encoder_id], sizeof(AS5600_EncoderState_t));
    return SYSTEM_OK;
}

/**
 * @brief Check if AS5600 encoder system is initialized
 * @return true if initialized, false otherwise
 */
bool as5600_is_initialized(void) {
    return as5600_initialized;
}

/**
 * @brief Get encoder error count
 * @param encoder_id Encoder identifier
 * @param error_count Pointer to store error count
 * @return System error code
 */
SystemError_t as5600_get_error_count(uint8_t encoder_id,
                                     uint32_t *error_count) {
    if (error_count == NULL) {
        return ERROR_ENCODER_CONFIG_INVALID;
    }

    SystemError_t result = as5600_validate_encoder_id(encoder_id);
    if (result != SYSTEM_OK) {
        return result;
    }

    *error_count = encoder_state[encoder_id].error_count;
    return SYSTEM_OK;
}
