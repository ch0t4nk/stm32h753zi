/**
 * @file as5600_driver.h
 * @brief AS5600 Magnetic Encoder Driver Header
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * @note AS5600 12-bit magnetic rotary encoder driver for closed-loop
 * stepper motor feedback
 *
 * TODO: See .github/instructions/stm32h7-i2c-as5600.instructions.md for
 * complete I2C implementation details
 */

#ifndef AS5600_DRIVER_H
#define AS5600_DRIVER_H

#include "common/error_codes.h"
#include "config/hardware_config.h"
#include "config/motor_config.h"
#ifndef UNITY_TESTING
#include "stm32h7xx_hal.h"
#endif
#include <stdbool.h>
#include <stdint.h>

/* ==========================================================================
 */
/* AS5600 Configuration (SSOT Integration)                                   */
/* ==========================================================================
 */

// AS5600 I2C Address - Use SSOT definitions from hardware_config.h
// Hardware config defines both 7-bit (0x36) and 8-bit (0x6C) addresses

// Use SSOT hardware configuration for I2C
#define AS5600_I2C_INSTANCE ENCODER_I2C_INSTANCE
#define AS5600_I2C_TIMEOUT 1000 // ms

// Dual encoder configuration
#define AS5600_MAX_ENCODERS MAX_MOTORS

/* ==========================================================================
 */
/* AS5600 Register Definitions                                               */
/* ==========================================================================
 */

// Configuration Registers
#define AS5600_REG_ZMCO 0x00   // Zero position MSB
#define AS5600_REG_ZPOS_H 0x01 // Zero position MSB
#define AS5600_REG_ZPOS_L 0x02 // Zero position LSB
#define AS5600_REG_MPOS_H 0x03 // Maximum position MSB
#define AS5600_REG_MPOS_L 0x04 // Maximum position LSB
#define AS5600_REG_MANG_H 0x05 // Maximum angle MSB
#define AS5600_REG_MANG_L 0x06 // Maximum angle LSB
#define AS5600_REG_CONF_H 0x07 // Configuration MSB
#define AS5600_REG_CONF_L 0x08 // Configuration LSB

// Output Registers
#define AS5600_REG_RAW_ANGLE_H 0x0C // Raw angle MSB
#define AS5600_REG_RAW_ANGLE_L 0x0D // Raw angle LSB
#define AS5600_REG_ANGLE_H 0x0E     // Filtered angle MSB
#define AS5600_REG_ANGLE_L 0x0F     // Filtered angle LSB

// Status Registers
#define AS5600_REG_STATUS 0x0B      // Status register
#define AS5600_REG_AGC 0x1A         // AGC register
#define AS5600_REG_MAGNITUDE_H 0x1B // Magnitude MSB
#define AS5600_REG_MAGNITUDE_L 0x1C // Magnitude LSB

// Burn Commands
#define AS5600_REG_BURN 0xFF // Burn command

/* ==========================================================================
 */
/* AS5600 Configuration Values                                               */
/* ==========================================================================
 */

// Status register bits
#define AS5600_STATUS_MH (1 << 3) // Magnet too strong
#define AS5600_STATUS_ML (1 << 4) // Magnet too weak
#define AS5600_STATUS_MD (1 << 5) // Magnet detected

// Configuration register default values
#define AS5600_CONF_SF 0x00  // Slow filter: 16x
#define AS5600_CONF_FTH 0x00 // Fast filter threshold: 6 LSBs
#define AS5600_CONF_WD 0x00  // Watchdog: OFF

/* ==========================================================================
 */
/* Driver Data Structures                                                    */
/* ==========================================================================
 */

/**
 * @brief AS5600 device handle structure
 */
typedef struct {
  uint8_t encoder_id;      // Encoder ID (0 or 1)
  I2C_HandleTypeDef *hi2c; // I2C handle from SSOT config
  uint8_t i2c_address;     // Device I2C address

  // MCSDK Integration
#if CLOSED_LOOP_FEEDBACK
  void *feedback_handle; // Feedback controller handle
#endif

  // Current state
  uint16_t raw_angle;      // Raw angle reading (0-4095)
  uint16_t filtered_angle; // Filtered angle reading
  float angle_degrees;     // Angle in degrees (0-360)
  uint8_t status;          // Device status
  bool magnet_detected;    // Magnet detection flag
  bool initialized;        // Initialization flag

  // Calibration data
  uint16_t zero_position; // Zero position offset
  uint16_t max_position;  // Maximum position limit

} AS5600_HandleTypeDef;

/* ==========================================================================
 */
/* Public Function Declarations                                              */
/* ==========================================================================
 */

/**
 * @brief Initialize AS5600 encoder system
 * @param hi2c1 I2C handle for encoder 1
 * @param hi2c2 I2C handle for encoder 2 (can be same as hi2c1)
 * @return HAL_StatusTypeDef HAL_OK if successful
 *
 * @note Follows stm32h7-i2c-as5600.instructions.md for I2C setup
 */
HAL_StatusTypeDef AS5600_Init(I2C_HandleTypeDef *hi2c1,
                              I2C_HandleTypeDef *hi2c2);

/**
 * @brief Initialize a specific AS5600 encoder
 * @param handle Pointer to AS5600 device handle
 * @param encoder_id Encoder ID (0 or 1)
 * @param hi2c I2C handle
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef AS5600_DeviceInit(AS5600_HandleTypeDef *handle,
                                    uint8_t encoder_id,
                                    I2C_HandleTypeDef *hi2c);

/**
 * @brief Read register from AS5600
 * @param handle Pointer to AS5600 device handle
 * @param reg Register address
 * @param value Pointer to store read value
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef AS5600_ReadRegister(AS5600_HandleTypeDef *handle, uint8_t reg,
                                      uint8_t *value);

/**
 * @brief Write register to AS5600
 * @param handle Pointer to AS5600 device handle
 * @param reg Register address
 * @param value Value to write
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef AS5600_WriteRegister(AS5600_HandleTypeDef *handle,
                                       uint8_t reg, uint8_t value);

/**
 * @brief Read 16-bit register pair from AS5600
 * @param handle Pointer to AS5600 device handle
 * @param reg_h MSB register address
 * @param value Pointer to store 16-bit value
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef AS5600_ReadRegister16(AS5600_HandleTypeDef *handle,
                                        uint8_t reg_h, uint16_t *value);

/**
 * @brief Get raw angle reading
 * @param handle Pointer to AS5600 device handle
 * @param angle Pointer to store raw angle (0-4095)
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef AS5600_GetRawAngle(AS5600_HandleTypeDef *handle,
                                     uint16_t *angle);

/**
 * @brief Get filtered angle reading
 * @param handle Pointer to AS5600 device handle
 * @param angle Pointer to store filtered angle (0-4095)
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef AS5600_GetAngle(AS5600_HandleTypeDef *handle,
                                  uint16_t *angle);

/**
 * @brief Get angle in degrees
 * @param handle Pointer to AS5600 device handle
 * @param degrees Pointer to store angle in degrees (0.0-360.0)
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef AS5600_GetAngleDegrees(AS5600_HandleTypeDef *handle,
                                         float *degrees);

/**
 * @brief Check magnet status
 * @param handle Pointer to AS5600 device handle
 * @param detected Pointer to store magnet detection flag
 * @param too_strong Pointer to store magnet too strong flag
 * @param too_weak Pointer to store magnet too weak flag
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef AS5600_CheckMagnet(AS5600_HandleTypeDef *handle,
                                     bool *detected, bool *too_strong,
                                     bool *too_weak);

/**
 * @brief Set zero position
 * @param handle Pointer to AS5600 device handle
 * @param zero_pos Zero position value (0-4095)
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef AS5600_SetZeroPosition(AS5600_HandleTypeDef *handle,
                                         uint16_t zero_pos);

/**
 * @brief Get AGC value
 * @param handle Pointer to AS5600 device handle
 * @param agc Pointer to store AGC value
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef AS5600_GetAGC(AS5600_HandleTypeDef *handle, uint8_t *agc);

/**
 * @brief Get magnitude
 * @param handle Pointer to AS5600 device handle
 * @param magnitude Pointer to store magnitude value
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef AS5600_GetMagnitude(AS5600_HandleTypeDef *handle,
                                      uint16_t *magnitude);

/* ==========================================================================
 */
/* Enhanced API Functions (SSOT-based Implementation)                        */
/* ==========================================================================
 */

/**
 * @brief Initialize AS5600 encoder system
 * @param hi2c1 I2C handle for encoder 1
 * @param hi2c2 I2C handle for encoder 2
 * @return SystemError_t System error code
 */
SystemError_t as5600_init(void);

/**
 * @brief Initialize individual encoder
 * @param encoder_id Encoder identifier (0 or 1)
 * @return SystemError_t System error code
 */
SystemError_t as5600_init_encoder(uint8_t encoder_id);

/**
 * @brief Read raw angle from AS5600
 * @param encoder_id Encoder identifier
 * @param raw_angle Pointer to store raw angle value (0-4095)
 * @return SystemError_t System error code
 */
SystemError_t as5600_read_raw_angle(uint8_t encoder_id, uint16_t *raw_angle);

/**
 * @brief Read filtered angle from AS5600
 * @param encoder_id Encoder identifier
 * @param angle Pointer to store filtered angle value (0-4095)
 * @return SystemError_t System error code
 */
SystemError_t as5600_read_angle(uint8_t encoder_id, uint16_t *angle);

/**
 * @brief Read angle in degrees
 * @param encoder_id Encoder identifier
 * @param angle_degrees Pointer to store angle in degrees (0.0 to 360.0)
 * @return SystemError_t System error code
 */
SystemError_t as5600_read_angle_degrees(uint8_t encoder_id,
                                        float *angle_degrees);

/**
 * @brief Read magnet magnitude
 * @param encoder_id Encoder identifier
 * @param magnitude Pointer to store magnitude value
 * @return SystemError_t System error code
 */
SystemError_t as5600_read_magnitude(uint8_t encoder_id, uint16_t *magnitude);

/**
 * @brief Read status register
 * @param encoder_id Encoder identifier
 * @param status Pointer to store status flags
 * @return SystemError_t System error code
 */
SystemError_t as5600_read_status(uint8_t encoder_id, uint8_t *status);

/**
 * @brief Get encoder velocity in degrees per second
 * @param encoder_id Encoder identifier
 * @param velocity_dps Pointer to store velocity
 * @return SystemError_t System error code
 */
SystemError_t as5600_get_velocity(uint8_t encoder_id, float *velocity_dps);

/**
 * @brief Check if magnet is properly positioned
 * @param encoder_id Encoder identifier
 * @param magnet_ok Pointer to store magnet status
 * @return SystemError_t System error code
 */
SystemError_t as5600_check_magnet(uint8_t encoder_id, bool *magnet_ok);

/**
 * @brief Calibrate encoder zero position
 * @param encoder_id Encoder identifier
 * @param current_angle Current mechanical angle (degrees)
 * @return SystemError_t System error code
 */
SystemError_t as5600_calibrate_zero(uint8_t encoder_id, float current_angle);

/**
 * @brief Check if AS5600 encoder system is initialized
 * @return bool true if initialized, false otherwise
 */
bool as5600_is_initialized(void);

/**
 * @brief Get encoder error count
 * @param encoder_id Encoder identifier
 * @param error_count Pointer to store error count
 * @return SystemError_t System error code
 */
SystemError_t as5600_get_error_count(uint8_t encoder_id, uint32_t *error_count);

/**
 * @brief Set encoder zero position reference
 * @param encoder_id Encoder identifier
 * @param zero_position_deg Zero position in degrees
 * @return SystemError_t System error code
 */
SystemError_t as5600_set_zero_position(uint8_t encoder_id,
                                       float zero_position_deg);

/* ==========================================================================
 */
/* Simulation Compatibility Macros                                          */
/* ==========================================================================
 */

// Register address compatibility for simulation - removed duplicates
// Note: Register definitions are already defined above around lines 61-70
// No need to redefine them here

#endif /* AS5600_DRIVER_H */
