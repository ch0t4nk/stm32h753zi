/**
 * @file l6470_registers_generated.h
 * @brief L6470 Register Definitions - AUTO-GENERATED
 * 
 * THIS FILE IS AUTO-GENERATED FROM SCHEMA: l6470_registers.yaml
 * DO NOT EDIT MANUALLY - CHANGES WILL BE OVERWRITTEN
 * 
 * Generated: 2025-08-07 00:18:38 UTC
 * Chip: L6470
 * Datasheet: Rev 9 - February 2012
 * Framework: X-CUBE-SPN2
 * 
 * @note This file provides SSOT register definitions for embedded driver
 * @note development. All register addresses, bit fields, and validation
 * @note constants are generated from the schema to ensure consistency.
 * 
 * @warning SAFETY CRITICAL - Validate all register configurations
 * @warning before deployment. Incorrect settings may damage hardware.
 */
#ifndef L6470_REGISTERS_GENERATED_H
#define L6470_REGISTERS_GENERATED_H

#include <stdint.h>
#include <stdbool.h>

/* SSOT Configuration Dependencies */
#include "hardware_config.h"
#include "safety_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* L6470 Register Addresses                                       */
/* ========================================================================== */

#define L6470_REG_ABS_POS              0x01    /* Current motor absolute position */
#define L6470_REG_EL_POS               0x02    /* Current motor electrical position */
#define L6470_REG_MARK                 0x03    /* Mark position register */
#define L6470_REG_SPEED                0x04    /* Current motor speed */
#define L6470_REG_ACC                  0x05    /* Acceleration rate */
#define L6470_REG_DEC                  0x06    /* Deceleration rate */
#define L6470_REG_MAX_SPEED            0x07    /* Maximum speed */
#define L6470_REG_MIN_SPEED            0x08    /* Minimum speed */
#define L6470_REG_KVAL_HOLD            0x09    /* Holding KVAL */
#define L6470_REG_KVAL_RUN             0x0A    /* Running KVAL */
#define L6470_REG_KVAL_ACC             0x0B    /* Acceleration KVAL */
#define L6470_REG_KVAL_DEC             0x0C    /* Deceleration KVAL */
#define L6470_REG_INT_SPD              0x0D    /* Intersect speed */
#define L6470_REG_FS_SPD               0x15    /* Full step speed */
#define L6470_REG_OCD_TH               0x13    /* Overcurrent detection threshold */
#define L6470_REG_STALL_TH             0x14    /* Stall detection threshold */
#define L6470_REG_STEP_MODE            0x16    /* Step mode and sync configuration */
#define L6470_REG_ALARM_EN             0x17    /* Alarm enable register */
#define L6470_REG_CONFIG               0x18    /* Device configuration register */
#define L6470_REG_STATUS               0x19    /* Device status register */

/* ========================================================================== */
/* L6470 Register Bit Field Definitions                           */
/* ========================================================================== */

/* STEP_MODE Register Bit Fields */
#define L6470_STEP_MODE_STEP_SEL_Pos             0U
#define L6470_STEP_MODE_STEP_SEL_Msk             (0x7U)
#define L6470_STEP_MODE_STEP_SEL_0               0U    /* Full step */
#define L6470_STEP_MODE_STEP_SEL_1               1U    /* Half step */
#define L6470_STEP_MODE_STEP_SEL_2               2U    /* 1/4 step */
#define L6470_STEP_MODE_STEP_SEL_3               3U    /* 1/8 step */
#define L6470_STEP_MODE_STEP_SEL_4               4U    /* 1/16 step */
#define L6470_STEP_MODE_STEP_SEL_5               5U    /* 1/32 step */
#define L6470_STEP_MODE_STEP_SEL_6               6U    /* 1/64 step */
#define L6470_STEP_MODE_STEP_SEL_7               7U    /* 1/128 step */
#define L6470_STEP_MODE_SYNC_SEL_Pos             4U
#define L6470_STEP_MODE_SYNC_SEL_Msk             (0x70U)
#define L6470_STEP_MODE_SYNC_SEL_0               0U    /* 1/2 FS_SPD */
#define L6470_STEP_MODE_SYNC_SEL_1               1U    /* FS_SPD */
#define L6470_STEP_MODE_SYNC_SEL_2               2U    /* 2*FS_SPD */
#define L6470_STEP_MODE_SYNC_SEL_3               3U    /* 4*FS_SPD */
#define L6470_STEP_MODE_SYNC_SEL_4               4U    /* 8*FS_SPD */
#define L6470_STEP_MODE_SYNC_SEL_5               5U    /* 16*FS_SPD */
#define L6470_STEP_MODE_SYNC_SEL_6               6U    /* 32*FS_SPD */
#define L6470_STEP_MODE_SYNC_SEL_7               7U    /* 64*FS_SPD */
#define L6470_STEP_MODE_SYNC_EN                  (1U << 7)
#define L6470_STEP_MODE_SYNC_EN_0                0U    /* Sync disabled */
#define L6470_STEP_MODE_SYNC_EN_1                1U    /* Sync enabled */

/* ALARM_EN Register Bit Fields */
#define L6470_ALARM_EN_OVERCURRENT               (1U << 0)
#define L6470_ALARM_EN_THERMAL_SHUTDOWN          (1U << 1)
#define L6470_ALARM_EN_THERMAL_WARNING           (1U << 2)
#define L6470_ALARM_EN_UVLO                      (1U << 3)
#define L6470_ALARM_EN_STALL_A                   (1U << 4)
#define L6470_ALARM_EN_STALL_B                   (1U << 5)
#define L6470_ALARM_EN_SWITCH                    (1U << 6)
#define L6470_ALARM_EN_WRONG_CMD                 (1U << 7)

/* CONFIG Register Bit Fields */
#define L6470_CONFIG_OSC_SEL_Pos                 0U
#define L6470_CONFIG_OSC_SEL_Msk                 (0x7U)
#define L6470_CONFIG_EXT_CLK                     (1U << 3)
#define L6470_CONFIG_SW_MODE                     (1U << 4)
#define L6470_CONFIG_EN_VSCOMP                   (1U << 5)
#define L6470_CONFIG_OC_SD                       (1U << 7)
#define L6470_CONFIG_POW_SR_Pos                  8U
#define L6470_CONFIG_POW_SR_Msk                  (0x300U)
#define L6470_CONFIG_F_PWM_DEC_Pos               10U
#define L6470_CONFIG_F_PWM_DEC_Msk               (0x1C00U)
#define L6470_CONFIG_F_PWM_INT_Pos               13U
#define L6470_CONFIG_F_PWM_INT_Msk               (0xE000U)

/* STATUS Register Bit Fields */
#define L6470_STATUS_HIZ                         (1U << 0)
#define L6470_STATUS_BUSY                        (1U << 1)
#define L6470_STATUS_SW_F                        (1U << 2)
#define L6470_STATUS_SW_EVN                      (1U << 3)
#define L6470_STATUS_DIR                         (1U << 4)
#define L6470_STATUS_MOT_STATUS_Pos              5U
#define L6470_STATUS_MOT_STATUS_Msk              (0x60U)
#define L6470_STATUS_MOT_STATUS_0                0U    /* Stopped */
#define L6470_STATUS_MOT_STATUS_1                1U    /* Acceleration */
#define L6470_STATUS_MOT_STATUS_2                2U    /* Deceleration */
#define L6470_STATUS_MOT_STATUS_3                3U    /* Constant speed */
#define L6470_STATUS_NOTPERF_CMD                 (1U << 7)
#define L6470_STATUS_WRONG_CMD                   (1U << 8)
#define L6470_STATUS_UVLO                        (1U << 9)
#define L6470_STATUS_TH_WRN                      (1U << 10)
#define L6470_STATUS_TH_SD                       (1U << 11)
#define L6470_STATUS_OCD                         (1U << 12)
#define L6470_STATUS_STEP_LOSS_A                 (1U << 13)
#define L6470_STATUS_STEP_LOSS_B                 (1U << 14)
#define L6470_STATUS_SCK_MOD                     (1U << 15)

/* ========================================================================== */
/* L6470 Command Definitions                                      */
/* ========================================================================== */

#define L6470_CMD_NOP                  0x00    /* No operation */
#define L6470_CMD_SET_PARAM            0x00    /* Set parameter */
#define L6470_CMD_GET_PARAM            0x20    /* Get parameter */
#define L6470_CMD_RUN                  0x50    /* Run at given speed */
#define L6470_CMD_STEP_CLOCK           0x58    /* Step clock mode */
#define L6470_CMD_MOVE                 0x40    /* Move N steps */
#define L6470_CMD_GOTO                 0x60    /* Go to absolute position */
#define L6470_CMD_GOTO_DIR             0x68    /* Go to position via direction */
#define L6470_CMD_GO_UNTIL             0x82    /* Go until switch */
#define L6470_CMD_RELEASE_SW           0x92    /* Release switch */
#define L6470_CMD_GO_HOME              0x70    /* Go to home position */
#define L6470_CMD_GO_MARK              0x78    /* Go to mark position */
#define L6470_CMD_RESET_POS            0xD8    /* Reset position to zero */
#define L6470_CMD_RESET_DEVICE         0xC0    /* Reset device */
#define L6470_CMD_SOFT_STOP            0xB0    /* Soft stop */
#define L6470_CMD_HARD_STOP            0xB8    /* Hard stop */
#define L6470_CMD_SOFT_HIZ             0xA0    /* Soft HiZ */
#define L6470_CMD_HARD_HIZ             0xA8    /* Hard HiZ */
#define L6470_CMD_GET_STATUS           0xD0    /* Get status and clear flags */

/* ========================================================================== */
/* L6470 Validation Constants                                     */
/* ========================================================================== */

/* Critical Registers for Safety Validation */
#define L6470_CRITICAL_REG_COUNT    4U
#define L6470_CRITICAL_REGS \
    { L6470_REG_OCD_TH, L6470_REG_STALL_TH, L6470_REG_MAX_SPEED, L6470_REG_CONFIG }

/* ========================================================================== */
/* L6470 Register Operation Functions                             */
/* ========================================================================== */

/**
 * @brief Validate L6470 register configuration
 * @param reg_addr Register address
 * @param value Register value
 * @return true if valid, false otherwise
 */
bool l6470_validate_register(uint8_t reg_addr, uint32_t value);

/**
 * @brief Get register size in bytes
 * @param reg_addr Register address
 * @return Size in bytes (1-3)
 */
uint8_t l6470_get_register_size(uint8_t reg_addr);

/**
 * @brief Check if register is read-only
 * @param reg_addr Register address
 * @return true if read-only, false if writable
 */
bool l6470_is_register_readonly(uint8_t reg_addr);

/**
 * @brief Get safe default value for register
 * @param reg_addr Register address
 * @return Default value or 0 if no default defined
 */
uint32_t l6470_get_safe_default(uint8_t reg_addr);

#ifdef __cplusplus
}
#endif

#endif /* L6470_REGISTERS_GENERATED_H */
