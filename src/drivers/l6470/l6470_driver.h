/**
 * @file l6470_driver.h
 * @brief L6470 Stepper Driver Header - X-CUBE-SPN2 Based Implementation
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * @note L6470 stepper motor driver implementation following X-CUBE-SPN2
 * framework with MCSDK selective enhancements
 *
 * TODO: See .github/instructions/stm32h7-spi-l6470.instructions.md for
 * complete SPI implementation details
 * TODO: See .github/instructions/l6470-registers.instructions.md for
 * register configuration and motor characterization
 */

#ifndef L6470_DRIVER_H
#define L6470_DRIVER_H

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
/* L6470 Driver Configuration (SSOT Integration)                             */
/* ==========================================================================
 */

// Use SSOT motor configuration
#define L6470_MAX_DEVICES MAX_MOTORS
#define L6470_SPI_INSTANCE MOTOR_SPI_INSTANCE
#define L6470_CS_PORT MOTOR_SPI_CS_PORT
#define L6470_CS_PIN MOTOR_SPI_CS_PIN

// X-CUBE-SPN2 Framework Integration
#if SPN2_FRAMEWORK_ENABLED
// #include "spn2_l6470_config.h"  // TODO: Create SPN2 integration layer -
// disabled for build
#endif

// MCSDK Enhancement Layer Integration
#if MCSDK_POSITION_CONTROL
// #include "mcsdk_position_control.h"  // TODO: Create MCSDK enhancement layer
// - disabled for build
#endif

/* ==========================================================================
 */
/* L6470 Command Definitions (from X-CUBE-SPN2)                              */
/* ==========================================================================
 */

// L6470 Command Set (following SPN2 implementation)
#define L6470_CMD_NOP 0x00
#define L6470_CMD_SET_PARAM 0x00
#define L6470_CMD_GET_PARAM 0x20
#define L6470_CMD_RUN 0x50
#define L6470_CMD_STEP_CLOCK 0x58
#define L6470_CMD_MOVE 0x40
#define L6470_CMD_GOTO 0x60
#define L6470_CMD_GOTO_DIR 0x68
#define L6470_CMD_GO_UNTIL 0x82
#define L6470_CMD_RELEASE_SW 0x92
#define L6470_CMD_GO_HOME 0x70
#define L6470_CMD_GO_MARK 0x78
#define L6470_CMD_RESET_POS 0xD8
#define L6470_CMD_RESET_DEVICE 0xC0
#define L6470_CMD_SOFT_STOP 0xB0
#define L6470_CMD_HARD_STOP 0xB8
#define L6470_CMD_SOFT_HIZ 0xA0
#define L6470_CMD_HARD_HIZ 0xA8
#define L6470_CMD_GET_STATUS 0xD0

/* ==========================================================================
 */
/* L6470 Register Definitions (from X-CUBE-SPN2)                             */
/* ==========================================================================
 */

// L6470 Parameter Registers (following SPN2 definitions)
#define L6470_REG_ABS_POS 0x01
#define L6470_REG_EL_POS 0x02
#define L6470_REG_MARK 0x03
#define L6470_REG_SPEED 0x04
#define L6470_REG_ACC 0x05
#define L6470_REG_DEC 0x06
#define L6470_REG_MAX_SPEED 0x07
#define L6470_REG_MIN_SPEED 0x08
#define L6470_REG_FS_SPD 0x15
#define L6470_REG_KVAL_HOLD 0x09
#define L6470_REG_KVAL_RUN 0x0A
#define L6470_REG_KVAL_ACC 0x0B
#define L6470_REG_KVAL_DEC 0x0C
#define L6470_REG_INT_SPD 0x0D
#define L6470_REG_ST_SLP 0x0E
#define L6470_REG_FN_SLP_ACC 0x0F
#define L6470_REG_FN_SLP_DEC 0x10
#define L6470_REG_K_THERM 0x11
#define L6470_REG_ADC_OUT 0x12
#define L6470_REG_OCD_TH 0x13
#define L6470_REG_STALL_TH 0x14
#define L6470_REG_STEP_MODE 0x16
#define L6470_REG_ALARM_EN 0x17
#define L6470_REG_CONFIG 0x18
#define L6470_REG_STATUS 0x19

/* ==========================================================================
 */
/* L6470 Status Register Bit Definitions (from datasheet)                   */
/* ==========================================================================
 */

// Status register bit masks
#define L6470_STATUS_HIZ (1 << 0)          // High impedance state
#define L6470_STATUS_BUSY (1 << 1)         // Device busy
#define L6470_STATUS_SW_F (1 << 2)         // Switch turn-on event
#define L6470_STATUS_SW_EVN (1 << 3)       // Switch input status
#define L6470_STATUS_DIR (1 << 4)          // Direction (1=forward, 0=reverse)
#define L6470_STATUS_MOT_STATUS (0x3 << 5) // Motor status bits (5-6)
#define L6470_STATUS_NOTPERF_CMD (1 << 7)  // Command not performed
#define L6470_STATUS_WRONG_CMD (1 << 8)    // Wrong command
#define L6470_STATUS_UVLO (1 << 9)         // Undervoltage lockout
#define L6470_STATUS_TH_WRN (1 << 10)      // Thermal warning
#define L6470_STATUS_TH_SD (1 << 11)       // Thermal shutdown
#define L6470_STATUS_OCD (1 << 12)         // Overcurrent detection
#define L6470_STATUS_STEP_LOSS_A (1 << 13) // Stall detection bridge A
#define L6470_STATUS_STEP_LOSS_B (1 << 14) // Stall detection bridge B
#define L6470_STATUS_SCK_MOD (1 << 15)     // Step clock mode

// Motor status values (bits 5-6)
#define L6470_MOT_STATUS_STOPPED 0x00
#define L6470_MOT_STATUS_ACCEL 0x01
#define L6470_MOT_STATUS_DECEL 0x02
#define L6470_MOT_STATUS_CONST_SPD 0x03

/* ==========================================================================
 */
/* Simulation Compatibility Macros                                          */
/* ==========================================================================
 */

// Status register compatibility for simulation
#define L6470_STATUS_MOT_STATUS_Msk L6470_STATUS_MOT_STATUS

/* ==========================================================================
 */
/* Driver Data Structures                                                    */
/* ==========================================================================
 */

/**
 * @brief L6470 device handle structure
 * @note Integrates SPN2 framework with MCSDK enhancements
 */
typedef struct {
    uint8_t device_id;       // Device ID (0 or 1)
    SPI_HandleTypeDef *hspi; // SPI handle from SSOT config
    GPIO_TypeDef *cs_port;   // CS port from SSOT config
    uint16_t cs_pin;         // CS pin from SSOT config

    // SPN2 Framework Integration
#if SPN2_FRAMEWORK_ENABLED
    void *spn2_handle; // SPN2 device handle
#endif

    // MCSDK Enhancement Integration
#if MCSDK_POSITION_CONTROL
    void *mcsdk_position_handle; // MCSDK position controller
#endif

    // Driver state
    uint32_t position; // Current position (steps)
    uint16_t speed;    // Current speed
    uint8_t status;    // Device status
    bool initialized;  // Initialization flag

} L6470_HandleTypeDef;

/* ==========================================================================
 */
/* Public Function Declarations                                              */
/* ==========================================================================
 */

/**
 * @brief Initialize L6470 driver system
 * @param hspi SPI handle from SSOT hardware configuration
 * @return HAL_StatusTypeDef HAL_OK if successful
 *
 * @note Follows stm32h7-spi-l6470.instructions.md for SPI setup
 */
HAL_StatusTypeDef L6470_Init(SPI_HandleTypeDef *hspi);

/**
 * @brief Initialize a specific L6470 device
 * @param handle Pointer to L6470 device handle
 * @param device_id Device ID (0 or 1)
 * @param hspi SPI handle
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef L6470_DeviceInit(L6470_HandleTypeDef *handle,
                                   uint8_t device_id, SPI_HandleTypeDef *hspi);

/**
 * @brief Send command to L6470 device
 * @param handle Pointer to L6470 device handle
 * @param command Command byte
 * @param param Command parameter (if applicable)
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef L6470_SendCommand(L6470_HandleTypeDef *handle,
                                    uint8_t command, uint32_t param);

/**
 * @brief Read register from L6470 device
 * @param handle Pointer to L6470 device handle
 * @param reg Register address
 * @param value Pointer to store read value
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef L6470_ReadRegister(L6470_HandleTypeDef *handle, uint8_t reg,
                                     uint32_t *value);

/**
 * @brief Write register to L6470 device
 * @param handle Pointer to L6470 device handle
 * @param reg Register address
 * @param value Value to write
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef L6470_WriteRegister(L6470_HandleTypeDef *handle, uint8_t reg,
                                      uint32_t value);

/**
 * @brief Move motor to absolute position
 * @param handle Pointer to L6470 device handle
 * @param position Target position (steps)
 * @return HAL_StatusTypeDef HAL_OK if successful
 *
 * @note Enhanced with MCSDK position control if enabled
 */
HAL_StatusTypeDef L6470_MoveToPosition(L6470_HandleTypeDef *handle,
                                       uint32_t position);

/**
 * @brief Move motor relative steps
 * @param handle Pointer to L6470 device handle
 * @param steps Number of steps (signed)
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef L6470_MoveSteps(L6470_HandleTypeDef *handle, int32_t steps);

/**
 * @brief Run motor at specified speed
 * @param handle Pointer to L6470 device handle
 * @param speed Speed value (L6470 format)
 * @param direction Direction (0=forward, 1=reverse)
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef L6470_Run(L6470_HandleTypeDef *handle, uint32_t speed,
                            uint8_t direction);

/**
 * @brief Stop motor (soft stop)
 * @param handle Pointer to L6470 device handle
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef L6470_Stop(L6470_HandleTypeDef *handle);

/**
 * @brief Emergency stop motor (hard stop)
 * @param handle Pointer to L6470 device handle
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef L6470_EmergencyStop(L6470_HandleTypeDef *handle);

/**
 * @brief Get current motor position
 * @param handle Pointer to L6470 device handle
 * @param position Pointer to store current position
 * @return HAL_StatusTypeDef HAL_OK if successful
 */
HAL_StatusTypeDef L6470_GetPosition(L6470_HandleTypeDef *handle,
                                    uint32_t *position);

/* ==========================================================================
 */
/* Enhanced API Functions (SSOT-based Implementation)                        */
/* ==========================================================================
 */

/**
 * @brief Initialize L6470 driver system
 * @param spi_handle Pointer to configured SPI handle
 * @return SystemError_t System error code
 */
SystemError_t l6470_init(void);

/**
 * @brief Initialize individual motor configuration
 * @param motor_id Motor identifier (0-based)
 * @return SystemError_t System error code
 */
SystemError_t l6470_init_motor(uint8_t motor_id);

/**
 * @brief Reset L6470 device
 * @param motor_id Motor identifier
 * @return SystemError_t System error code
 */
SystemError_t l6470_reset_device(uint8_t motor_id);

/**
 * @brief Set L6470 register parameter
 * @param motor_id Motor identifier
 * @param register_addr Register address
 * @param value Parameter value
 * @return SystemError_t System error code
 */
SystemError_t l6470_set_parameter(uint8_t motor_id, uint8_t register_addr,
                                  uint32_t value);

/**
 * @brief Get L6470 register parameter
 * @param motor_id Motor identifier
 * @param register_addr Register address
 * @param value Pointer to store parameter value
 * @return SystemError_t System error code
 */
SystemError_t l6470_get_parameter(uint8_t motor_id, uint8_t register_addr,
                                  uint32_t *value);

/**
 * @brief Get L6470 status register
 * @param motor_id Motor identifier
 * @param status Pointer to store status value
 * @return SystemError_t System error code
 */
SystemError_t l6470_get_status(uint8_t motor_id, uint16_t *status);

/**
 * @brief Move motor to absolute position
 * @param motor_id Motor identifier
 * @param position Target position in steps
 * @return SystemError_t System error code
 */
SystemError_t l6470_move_to_position(uint8_t motor_id, int32_t position);

/**
 * @brief Stop motor with controlled deceleration
 * @param motor_id Motor identifier
 * @return SystemError_t System error code
 */
SystemError_t l6470_soft_stop(uint8_t motor_id);

/**
 * @brief Stop motor immediately (emergency stop)
 * @param motor_id Motor identifier
 * @return SystemError_t System error code
 */
SystemError_t l6470_hard_stop(uint8_t motor_id);

/**
 * @brief Set motor to high impedance state
 * @param motor_id Motor identifier
 * @return SystemError_t System error code
 */
SystemError_t l6470_hard_hiz(uint8_t motor_id);

/**
 * @brief Check if L6470 driver system is initialized
 * @return bool true if initialized, false otherwise
 */
bool l6470_is_initialized(void);

/**
 * @brief Run motor at constant velocity
 * @param motor_id Motor identifier (0 or 1)
 * @param direction true for forward, false for reverse
 * @param speed Speed in steps/second
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 */
SystemError_t l6470_run(uint8_t motor_id, bool direction, float speed);

/**
 * @brief Reset motor position to zero
 * @param motor_id Motor identifier (0 or 1)
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 */
SystemError_t l6470_reset_position(uint8_t motor_id);

#endif /* L6470_DRIVER_H */
