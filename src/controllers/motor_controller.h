/**
 * @file motor_controller.h
 * @brief Motor Controller API - Integrates L6470 + AS5600 for Closed-Loop Control
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 * 
 * @note Motor control integration API following instruction files:
 * - .github/instructions/stm32h7-spi-l6470.instructions.md
 * - .github/instructions/stm32h7-i2c-as5600.instructions.md
 * - .github/instructions/safety-systems.instructions.md
 * - .github/instructions/data-types.instructions.md
 */

#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Includes                                                                   */
/* ========================================================================== */

#ifndef UNITY_TESTING
#include "stm32h7xx_hal.h"
#endif
#include "common/error_codes.h"
#include "common/data_types.h"
#include "config/motor_config.h"
#include <stdbool.h>

/* ========================================================================== */
/* Motor Control Types and Constants                                         */
/* ========================================================================== */

// Note: MotorState_t is defined in motor_config.h as SSOT
// Including motor_config.h provides the unified MotorState_t definition

/**
 * @brief Motor controller configuration
 * @note SSOT configuration from motor_config.h
 */
typedef struct {
    float position_tolerance_deg;       /**< Position tolerance in degrees */
    float max_velocity_dps;            /**< Maximum velocity in deg/s */
    float max_acceleration_dps2;       /**< Maximum acceleration in deg/s² */
    uint32_t control_loop_period_ms;   /**< Control loop update period */
    bool enable_closed_loop;           /**< Enable closed-loop control */
} MotorControllerConfig_t;

/**
 * @brief Motor status information
 */
typedef struct {
    bool enabled;                      /**< Motor enabled flag */
    MotorState_t state;               /**< Current motor state */
    float current_position_deg;       /**< Current position from encoder */
    float target_position_deg;        /**< Target position */
    float position_error_deg;         /**< Position error (target - current) */
    float current_velocity_dps;       /**< Current velocity from encoder */
    uint32_t fault_count;             /**< Number of faults detected */
    uint32_t last_update_time_ms;     /**< Last update timestamp */
} MotorStatus_t;

/* ========================================================================== */
/* Public API Function Declarations                                          */
/* ========================================================================== */

/**
 * @brief Initialize motor controller system
 * @param hspi SPI handle for L6470 communication
 * @param hi2c1 I2C handle for encoder 1 (motor 0)
 * @param hi2c2 I2C handle for encoder 2 (motor 1)
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 * 
 * @note This function initializes:
 *       - L6470 stepper drivers via SPI daisy-chain
 *       - AS5600 encoders on separate I2C buses
 *       - Motor control state variables
 *       - Safety monitoring systems
 * 
 * @warning Must be called before any other motor controller functions
 */
SystemError_t motor_controller_init(SPI_HandleTypeDef* hspi, 
                                   I2C_HandleTypeDef* hi2c1, 
                                   I2C_HandleTypeDef* hi2c2);

/**
 * @brief Enable motor control for specified motor
 * @param motor_id Motor identifier (0 or 1)
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 * 
 * @note Performs safety checks before enabling:
 *       - Encoder magnet detection
 *       - L6470 status verification
 *       - Position reading validation
 */
SystemError_t motor_controller_enable_motor(uint8_t motor_id);

/**
 * @brief Disable motor control for specified motor
 * @param motor_id Motor identifier (0 or 1)
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 * 
 * @note Sets motor to high impedance state for safety
 */
SystemError_t motor_controller_disable_motor(uint8_t motor_id);

/**
 * @brief Move motor to absolute position
 * @param motor_id Motor identifier (0 or 1)
 * @param target_position_deg Target position in degrees (0.0 to 360.0)
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 * 
 * @note Function performs:
 *       - Position limit checking
 *       - Current position update from encoder
 *       - L6470 move command with proper step conversion
 *       - State tracking update
 */
SystemError_t motor_controller_move_to_position(uint8_t motor_id, float target_position_deg);

/**
 * @brief Stop motor with controlled deceleration
 * @param motor_id Motor identifier (0 or 1)
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 */
SystemError_t motor_controller_stop_motor(uint8_t motor_id);

/**
 * @brief Emergency stop motor immediately
 * @param motor_id Motor identifier (0 or 1)
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 * 
 * @warning This function performs immediate stop without deceleration
 */
SystemError_t motor_controller_emergency_stop(uint8_t motor_id);

/**
 * @brief Update motor control loop (call periodically)
 * @return SystemError_t - Overall system status
 * 
 * @note This function should be called regularly (e.g., 10-50Hz) to:
 *       - Update position from encoders
 *       - Calculate position errors
 *       - Check for movement completion
 *       - Perform safety monitoring
 *       - Handle fault conditions
 */
SystemError_t motor_controller_update(void);

/**
 * @brief Get current motor position in degrees
 * @param motor_id Motor identifier (0 or 1)
 * @param position_deg Pointer to store position value
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 * 
 * @note Reads directly from AS5600 encoder for real-time position
 */
SystemError_t motor_controller_get_position(uint8_t motor_id, float* position_deg);

/**
 * @brief Get current motor velocity in degrees per second
 * @param motor_id Motor identifier (0 or 1)
 * @param velocity_dps Pointer to store velocity value
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 * 
 * @note Calculated from AS5600 encoder position changes over time
 */
SystemError_t motor_controller_get_velocity(uint8_t motor_id, float* velocity_dps);

/**
 * @brief Get motor control state
 * @param motor_id Motor identifier (0 or 1)
 * @param state Pointer to store state value
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 */
SystemError_t motor_controller_get_state(uint8_t motor_id, MotorState_t* state);

/**
 * @brief Get comprehensive motor status information
 * @param motor_id Motor identifier (0 or 1)
 * @param status Pointer to store status structure
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 * 
 * @note Provides complete motor status including:
 *       - Position and velocity
 *       - State and error information
 *       - Fault counts and timestamps
 */
SystemError_t motor_controller_get_status(uint8_t motor_id, MotorStatus_t* status);

/**
 * @brief Check if motor controller system is initialized
 * @return true if initialized, false otherwise
 */
bool motor_controller_is_initialized(void);

/**
 * @brief Home motor to zero position
 * @param motor_id Motor identifier (0 or 1)
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 * 
 * @note Sets current encoder position as zero reference
 */
SystemError_t motor_controller_home_motor(uint8_t motor_id);

/**
 * @brief Set motor velocity for continuous motion
 * @param motor_id Motor identifier (0 or 1)
 * @param velocity_dps Target velocity in degrees per second
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 * 
 * @note For continuous motion applications
 */
SystemError_t motor_controller_set_velocity(uint8_t motor_id, float velocity_dps);

/**
 * @brief Calibrate encoder zero position
 * @param motor_id Motor identifier (0 or 1)
 * @param zero_position_deg Zero position in degrees
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 */
SystemError_t motor_controller_calibrate_encoder(uint8_t motor_id, float zero_position_deg);

/* ========================================================================== */
/* Utility Macros                                                            */
/* ========================================================================== */

/**
 * @brief Check if motor ID is valid
 * @param motor_id Motor ID to check
 * @return true if valid, false otherwise
 */
#define MOTOR_CONTROLLER_IS_VALID_MOTOR_ID(motor_id) ((motor_id) < MAX_MOTORS)

/**
 * @brief Convert degrees to steps using SSOT configuration
 * @param degrees Angle in degrees
 * @return Number of steps
 */
#define MOTOR_CONTROLLER_DEGREES_TO_STEPS(degrees) \
    ((int32_t)((degrees) * MOTOR_TOTAL_STEPS / 360.0f + 0.5f))

/**
 * @brief Convert steps to degrees using SSOT configuration
 * @param steps Number of steps
 * @return Angle in degrees
 */
#define MOTOR_CONTROLLER_STEPS_TO_DEGREES(steps) \
    ((float)(steps) * 360.0f / MOTOR_TOTAL_STEPS)

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_CONTROLLER_H */
