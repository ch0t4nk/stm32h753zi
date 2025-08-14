/**
 * @file l6470_registers_generated.h
 * @brief L6470 Register Definitions - Auto-Generated from Schema
 * @version 1.0
 * @date 2025-08-14 10:29:42
 *
 * ⚠️  WARNING: AUTO-GENERATED FILE - DO NOT EDIT MANUALLY
 * This file is generated from YAML schema definitions.
 * Modify the schema file and regenerate instead.
 *
 * Schema source: schemas/l6470_registers.yaml
 * Generator: scripts/schema_generator.py
 *
 * @note Generated for STM32H753ZI stepper motor control project
 * @note Based on L6470 datasheet Rev 9 - February 2012
 * @note Follows SSOT configuration management principles
 */
#ifndef L6470_REGISTERS_GENERATED_H
#define L6470_REGISTERS_GENERATED_H


/* Required includes for SSOT integration */
#include <stdint.h>
#include <stdbool.h>
#include "common/error_codes.h"
#include "hal_abstraction/hal_abstraction.h"

/* ========================================================================== */
/* Register Address Definitions                                              */
/* ========================================================================== */
#define CHIP_ABS_POS_ADDR    0x01    /* Current motor absolute position */
#define CHIP_EL_POS_ADDR    0x02    /* Current motor electrical position */
#define CHIP_MARK_ADDR    0x03    /* Mark position register */
#define CHIP_SPEED_ADDR    0x04    /* Current motor speed */
#define CHIP_ACC_ADDR    0x05    /* Acceleration rate */
#define CHIP_DEC_ADDR    0x06    /* Deceleration rate */
#define CHIP_MAX_SPEED_ADDR    0x07    /* Maximum speed */
#define CHIP_MIN_SPEED_ADDR    0x08    /* Minimum speed */
#define CHIP_KVAL_HOLD_ADDR    0x09    /* Holding KVAL */
#define CHIP_KVAL_RUN_ADDR    0x0A    /* Running KVAL */
#define CHIP_KVAL_ACC_ADDR    0x0B    /* Acceleration KVAL */
#define CHIP_KVAL_DEC_ADDR    0x0C    /* Deceleration KVAL */
#define CHIP_INT_SPD_ADDR    0x0D    /* Intersect speed */
#define CHIP_FS_SPD_ADDR    0x15    /* Full step speed */
#define CHIP_OCD_TH_ADDR    0x13    /* Overcurrent detection threshold */
#define CHIP_STALL_TH_ADDR    0x14    /* Stall detection threshold */
#define CHIP_STEP_MODE_ADDR    0x16    /* Step mode and sync configuration */
#define CHIP_ALARM_EN_ADDR    0x17    /* Alarm enable register */
#define CHIP_CONFIG_ADDR    0x18    /* Device configuration register */
#define CHIP_STATUS_ADDR    0x19    /* Device status register */

/* ========================================================================== */
/* Register Mask Definitions                                                 */
/* ========================================================================== */
#define ABS_POS_MASK        0x3FFFFF
#define EL_POS_MASK        0x01FF
#define MARK_MASK        0x3FFFFF
#define SPEED_MASK        0x0FFFFF
#define ACC_MASK        0x0FFF
#define DEC_MASK        0x0FFF
#define MAX_SPEED_MASK        0x03FF
#define MIN_SPEED_MASK        0x0FFF
#define KVAL_HOLD_MASK        0xFF
#define KVAL_RUN_MASK        0xFF
#define KVAL_ACC_MASK        0xFF
#define KVAL_DEC_MASK        0xFF
#define INT_SPD_MASK        0x3FFF
#define FS_SPD_MASK        0x03FF
#define OCD_TH_MASK        0x1F
#define STALL_TH_MASK        0x7F
#define STEP_MODE_MASK        0xFF
#define ALARM_EN_MASK        0xFF
#define CONFIG_MASK        0xFFFF
#define STATUS_MASK        0xFFFF

/* ========================================================================== */
/* Register Field Definitions                                                */
/* ========================================================================== */
/* STEP_MODE register fields */
#define STEP_MODE_STEP_SEL_POS    0
#define STEP_MODE_STEP_SEL_MSK    0x0007
#define STEP_MODE_STEP_SEL_FULL_STEP    0x0000
#define STEP_MODE_STEP_SEL_HALF_STEP    0x0001
#define STEP_MODE_STEP_SEL_1/4_STEP    0x0002
#define STEP_MODE_STEP_SEL_1/8_STEP    0x0003
#define STEP_MODE_STEP_SEL_1/16_STEP    0x0004
#define STEP_MODE_STEP_SEL_1/32_STEP    0x0005
#define STEP_MODE_STEP_SEL_1/64_STEP    0x0006
#define STEP_MODE_STEP_SEL_1/128_STEP    0x0007
#define STEP_MODE_SYNC_SEL_POS    4
#define STEP_MODE_SYNC_SEL_MSK    0x0070
#define STEP_MODE_SYNC_SEL_1/2_FS_SPD    0x0000
#define STEP_MODE_SYNC_SEL_FS_SPD    0x0010
#define STEP_MODE_SYNC_SEL_2*FS_SPD    0x0020
#define STEP_MODE_SYNC_SEL_4*FS_SPD    0x0030
#define STEP_MODE_SYNC_SEL_8*FS_SPD    0x0040
#define STEP_MODE_SYNC_SEL_16*FS_SPD    0x0050
#define STEP_MODE_SYNC_SEL_32*FS_SPD    0x0060
#define STEP_MODE_SYNC_SEL_64*FS_SPD    0x0070
#define STEP_MODE_SYNC_EN_POS    7
#define STEP_MODE_SYNC_EN_MSK    0x0080
#define STEP_MODE_SYNC_EN_SYNC_DISABLED    0x0000
#define STEP_MODE_SYNC_EN_SYNC_ENABLED    0x0080

/* ALARM_EN register fields */
#define ALARM_EN_OVERCURRENT_POS    0
#define ALARM_EN_OVERCURRENT_MSK    0x0001
#define ALARM_EN_THERMAL_SHUTDOWN_POS    1
#define ALARM_EN_THERMAL_SHUTDOWN_MSK    0x0002
#define ALARM_EN_THERMAL_WARNING_POS    2
#define ALARM_EN_THERMAL_WARNING_MSK    0x0004
#define ALARM_EN_UVLO_POS    3
#define ALARM_EN_UVLO_MSK    0x0008
#define ALARM_EN_STALL_A_POS    4
#define ALARM_EN_STALL_A_MSK    0x0010
#define ALARM_EN_STALL_B_POS    5
#define ALARM_EN_STALL_B_MSK    0x0020
#define ALARM_EN_SWITCH_POS    6
#define ALARM_EN_SWITCH_MSK    0x0040
#define ALARM_EN_WRONG_CMD_POS    7
#define ALARM_EN_WRONG_CMD_MSK    0x0080

/* CONFIG register fields */
#define CONFIG_OSC_SEL_POS    0
#define CONFIG_OSC_SEL_MSK    0x0007
#define CONFIG_EXT_CLK_POS    3
#define CONFIG_EXT_CLK_MSK    0x0008
#define CONFIG_SW_MODE_POS    4
#define CONFIG_SW_MODE_MSK    0x0010
#define CONFIG_EN_VSCOMP_POS    5
#define CONFIG_EN_VSCOMP_MSK    0x0020
#define CONFIG_OC_SD_POS    7
#define CONFIG_OC_SD_MSK    0x0080
#define CONFIG_POW_SR_POS    8
#define CONFIG_POW_SR_MSK    0x0300
#define CONFIG_F_PWM_DEC_POS    10
#define CONFIG_F_PWM_DEC_MSK    0x1C00
#define CONFIG_F_PWM_INT_POS    13
#define CONFIG_F_PWM_INT_MSK    0xE000

/* STATUS register fields */
#define STATUS_HIZ_POS    0
#define STATUS_HIZ_MSK    0x0001
#define STATUS_BUSY_POS    1
#define STATUS_BUSY_MSK    0x0002
#define STATUS_SW_F_POS    2
#define STATUS_SW_F_MSK    0x0004
#define STATUS_SW_EVN_POS    3
#define STATUS_SW_EVN_MSK    0x0008
#define STATUS_DIR_POS    4
#define STATUS_DIR_MSK    0x0010
#define STATUS_MOT_STATUS_POS    5
#define STATUS_MOT_STATUS_MSK    0x0060
#define STATUS_MOT_STATUS_STOPPED    0x0000
#define STATUS_MOT_STATUS_ACCELERATION    0x0020
#define STATUS_MOT_STATUS_DECELERATION    0x0040
#define STATUS_MOT_STATUS_CONSTANT_SPEED    0x0060
#define STATUS_NOTPERF_CMD_POS    7
#define STATUS_NOTPERF_CMD_MSK    0x0080
#define STATUS_WRONG_CMD_POS    8
#define STATUS_WRONG_CMD_MSK    0x0100
#define STATUS_UVLO_POS    9
#define STATUS_UVLO_MSK    0x0200
#define STATUS_TH_WRN_POS    10
#define STATUS_TH_WRN_MSK    0x0400
#define STATUS_TH_SD_POS    11
#define STATUS_TH_SD_MSK    0x0800
#define STATUS_OCD_POS    12
#define STATUS_OCD_MSK    0x1000
#define STATUS_STEP_LOSS_A_POS    13
#define STATUS_STEP_LOSS_A_MSK    0x2000
#define STATUS_STEP_LOSS_B_POS    14
#define STATUS_STEP_LOSS_B_MSK    0x4000
#define STATUS_SCK_MOD_POS    15
#define STATUS_SCK_MOD_MSK    0x8000

/* ========================================================================== */
/* Command Definitions                                                       */
/* ========================================================================== */
#define CMD_NOP    0x00    /* No operation */
#define CMD_SET_PARAM    0x00    /* Set parameter */
#define CMD_GET_PARAM    0x20    /* Get parameter */
#define CMD_RUN    0x50    /* Run at given speed */
#define CMD_STEP_CLOCK    0x58    /* Step clock mode */
#define CMD_MOVE    0x40    /* Move N steps */
#define CMD_GOTO    0x60    /* Go to absolute position */
#define CMD_GOTO_DIR    0x68    /* Go to position via direction */
#define CMD_GO_UNTIL    0x82    /* Go until switch */
#define CMD_RELEASE_SW    0x92    /* Release switch */
#define CMD_GO_HOME    0x70    /* Go to home position */
#define CMD_GO_MARK    0x78    /* Go to mark position */
#define CMD_RESET_POS    0xD8    /* Reset position to zero */
#define CMD_RESET_DEVICE    0xC0    /* Reset device */
#define CMD_SOFT_STOP    0xB0    /* Soft stop */
#define CMD_HARD_STOP    0xB8    /* Hard stop */
#define CMD_SOFT_HIZ    0xA0    /* Soft HiZ */
#define CMD_HARD_HIZ    0xA8    /* Hard HiZ */
#define CMD_GET_STATUS    0xD0    /* Get status and clear flags */

/* ========================================================================== */
/* Validation Constants                                                      */
/* ========================================================================== */
/* Safe default values */
#define OCD_TH_SAFE_DEFAULT    0x0008
#define STALL_TH_SAFE_DEFAULT    0x0040
#define MAX_SPEED_SAFE_DEFAULT    0x0041
#define KVAL_RUN_SAFE_DEFAULT    0x0029

/* ========================================================================== */
/* Register Access Function Declarations                                    */
/* ========================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Validate L6470 register value
 * @param reg_addr Register address
 * @param value Value to validate
 * @return true if valid, false otherwise
 */
bool l6470_validate_register_value(uint8_t reg_addr, uint32_t value);

/**
 * @brief Get safe default value for L6470 register
 * @param reg_addr Register address
 * @return Safe default value
 */
uint32_t l6470_get_safe_default(uint8_t reg_addr);

/**
 * @brief Check if L6470 register is critical for safety
 * @param reg_addr Register address
 * @return true if critical, false otherwise
 */
bool l6470_is_critical_register(uint8_t reg_addr);

#ifdef __cplusplus
}
#endif


#endif /* L6470_REGISTERS_GENERATED_H */
