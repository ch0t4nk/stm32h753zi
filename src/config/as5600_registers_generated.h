/**
 * @file as5600_registers_generated.h
 * @brief AS5600 Register Definitions - Auto-Generated from Schema
 * @version 1.0
 * @date 2025-08-15 14:30:36
 *
 * ⚠️  WARNING: AUTO-GENERATED FILE - DO NOT EDIT MANUALLY
 * This file is generated from YAML schema definitions.
 * Modify the schema file and regenerate instead.
 *
 * Schema source: schemas/as5600_registers.yaml
 * Generator: scripts/schema_generator.py
 *
 * @note Generated for STM32H753ZI stepper motor control project
 * @note Based on AS5600 datasheet Rev 1.21
 * @note Follows SSOT configuration management principles
 */
#ifndef AS5600_REGISTERS_GENERATED_H
#define AS5600_REGISTERS_GENERATED_H


/* Required includes for SSOT integration */
#include <stdint.h>
#include <stdbool.h>
#include "common/error_codes.h"
#include "hal_abstraction/hal_abstraction.h"

/* ========================================================================== */
/* Register Address Definitions                                              */
/* ========================================================================== */
#define CHIP_ZMCO_ADDR    0x00    /* Programming count register (OTP) */
#define CHIP_ZPOS_H_ADDR    0x01    /* Zero position MSB (OTP) */
#define CHIP_ZPOS_L_ADDR    0x02    /* Zero position LSB (OTP) */
#define CHIP_MPOS_H_ADDR    0x03    /* Maximum position MSB (OTP) */
#define CHIP_MPOS_L_ADDR    0x04    /* Maximum position LSB (OTP) */
#define CHIP_MANG_H_ADDR    0x05    /* Maximum angle MSB (OTP) */
#define CHIP_MANG_L_ADDR    0x06    /* Maximum angle LSB (OTP) */
#define CHIP_CONF_H_ADDR    0x07    /* Configuration MSB (OTP) */
#define CHIP_CONF_L_ADDR    0x08    /* Configuration LSB (OTP) */
#define CHIP_RAW_ANGLE_H_ADDR    0x0C    /* Raw angle MSB (unfiltered) */
#define CHIP_RAW_ANGLE_L_ADDR    0x0D    /* Raw angle LSB (unfiltered) */
#define CHIP_ANGLE_H_ADDR    0x0E    /* Filtered angle MSB */
#define CHIP_ANGLE_L_ADDR    0x0F    /* Filtered angle LSB */
#define CHIP_STATUS_ADDR    0x0B    /* Status register */
#define CHIP_AGC_ADDR    0x1A    /* Automatic Gain Control */
#define CHIP_MAGNITUDE_H_ADDR    0x1B    /* Magnitude MSB */
#define CHIP_MAGNITUDE_L_ADDR    0x1C    /* Magnitude LSB */
#define CHIP_BURN_ADDR    0xFF    /* Burn command register */

/* ========================================================================== */
/* Register Mask Definitions                                                 */
/* ========================================================================== */
#define ZMCO_MASK        0x03
#define ZPOS_H_MASK        0x0F
#define ZPOS_L_MASK        0xFF
#define MPOS_H_MASK        0x0F
#define MPOS_L_MASK        0xFF
#define MANG_H_MASK        0x0F
#define MANG_L_MASK        0xFF
#define CONF_H_MASK        0xFF
#define CONF_L_MASK        0xFF
#define RAW_ANGLE_H_MASK        0x0F
#define RAW_ANGLE_L_MASK        0xFF
#define ANGLE_H_MASK        0x0F
#define ANGLE_L_MASK        0xFF
#define STATUS_MASK        0x38
#define AGC_MASK        0xFF
#define MAGNITUDE_H_MASK        0x0F
#define MAGNITUDE_L_MASK        0xFF
#define BURN_MASK        0xFF

/* ========================================================================== */
/* Register Field Definitions                                                */
/* ========================================================================== */
/* CONF_H register fields */
#define CONF_H_PM_POS    0
#define CONF_H_PM_MSK    0x0003
#define CONF_H_PM_NOM_(NORMAL)    0x0000
#define CONF_H_PM_LPM1_(LOW_POWER_1)    0x0001
#define CONF_H_PM_LPM2_(LOW_POWER_2)    0x0002
#define CONF_H_PM_LPM3_(LOW_POWER_3)    0x0003
#define CONF_H_HYST_POS    2
#define CONF_H_HYST_MSK    0x000C
#define CONF_H_HYST_OFF    0x0000
#define CONF_H_HYST_1_LSB    0x0004
#define CONF_H_HYST_2_LSBS    0x0008
#define CONF_H_HYST_3_LSBS    0x000C
#define CONF_H_OUTS_POS    4
#define CONF_H_OUTS_MSK    0x0030
#define CONF_H_OUTS_ANALOG_(FULL_RANGE)    0x0000
#define CONF_H_OUTS_ANALOG_(REDUCED_RANGE)    0x0010
#define CONF_H_OUTS_DIGITAL_PWM    0x0020
#define CONF_H_OUTS_RESERVED    0x0030
#define CONF_H_PWMF_POS    6
#define CONF_H_PWMF_MSK    0x00C0
#define CONF_H_PWMF_115_HZ    0x0000
#define CONF_H_PWMF_230_HZ    0x0040
#define CONF_H_PWMF_460_HZ    0x0080
#define CONF_H_PWMF_920_HZ    0x00C0

/* CONF_L register fields */
#define CONF_L_SF_POS    0
#define CONF_L_SF_MSK    0x0003
#define CONF_L_SF_16X    0x0000
#define CONF_L_SF_8X    0x0001
#define CONF_L_SF_4X    0x0002
#define CONF_L_SF_2X    0x0003
#define CONF_L_FTH_POS    2
#define CONF_L_FTH_MSK    0x001C
#define CONF_L_FTH_SLOW_FILTER_ONLY    0x0000
#define CONF_L_FTH_6_LSBS    0x0004
#define CONF_L_FTH_7_LSBS    0x0008
#define CONF_L_FTH_9_LSBS    0x000C
#define CONF_L_FTH_18_LSBS    0x0010
#define CONF_L_FTH_21_LSBS    0x0014
#define CONF_L_FTH_24_LSBS    0x0018
#define CONF_L_FTH_10_LSBS    0x001C
#define CONF_L_WD_POS    5
#define CONF_L_WD_MSK    0x0020
#define CONF_L_WD_OFF    0x0000
#define CONF_L_WD_ON    0x0020

/* STATUS register fields */
#define STATUS_MH_POS    3
#define STATUS_MH_MSK    0x0008
#define STATUS_ML_POS    4
#define STATUS_ML_MSK    0x0010
#define STATUS_MD_POS    5
#define STATUS_MD_MSK    0x0020

/* ========================================================================== */
/* Combined Register Macros (Multi-byte registers)                          */
/* ========================================================================== */
/* ZPOS: Zero position (0-4095) */
#define ZPOS_COMBINE(high, low)    (((high) << 8) | (low))
#define ZPOS_HIGH(val)            (((val) >> 8) & 0x0F)
#define ZPOS_LOW(val)             ((val) & 0xFF)

/* MPOS: Maximum position (0-4095) */
#define MPOS_COMBINE(high, low)    (((high) << 8) | (low))
#define MPOS_HIGH(val)            (((val) >> 8) & 0x0F)
#define MPOS_LOW(val)             ((val) & 0xFF)

/* MANG: Maximum angle (0-4095) */
#define MANG_COMBINE(high, low)    (((high) << 8) | (low))
#define MANG_HIGH(val)            (((val) >> 8) & 0x0F)
#define MANG_LOW(val)             ((val) & 0xFF)

/* RAW_ANGLE: Raw angle reading (0-4095) */
#define RAW_ANGLE_COMBINE(high, low)    (((high) << 8) | (low))
#define RAW_ANGLE_HIGH(val)            (((val) >> 8) & 0x0F)
#define RAW_ANGLE_LOW(val)             ((val) & 0xFF)

/* ANGLE: Filtered angle reading (0-4095) */
#define ANGLE_COMBINE(high, low)    (((high) << 8) | (low))
#define ANGLE_HIGH(val)            (((val) >> 8) & 0x0F)
#define ANGLE_LOW(val)             ((val) & 0xFF)

/* MAGNITUDE: Magnitude reading (0-4095) */
#define MAGNITUDE_COMBINE(high, low)    (((high) << 8) | (low))
#define MAGNITUDE_HIGH(val)            (((val) >> 8) & 0x0F)
#define MAGNITUDE_LOW(val)             ((val) & 0xFF)

/* ========================================================================== */
/* Validation Constants                                                      */
/* ========================================================================== */
/* Safe default values */
#define CONF_H_SAFE_DEFAULT    0x0000
#define CONF_L_SAFE_DEFAULT    0x0000
#define ZPOS_H_SAFE_DEFAULT    0x0000
#define ZPOS_L_SAFE_DEFAULT    0x0000
#define MPOS_H_SAFE_DEFAULT    0x000F
#define MPOS_L_SAFE_DEFAULT    0x00FF
#define MANG_H_SAFE_DEFAULT    0x000F
#define MANG_L_SAFE_DEFAULT    0x00FF

/* ========================================================================== */
/* Register Access Function Declarations                                    */
/* ========================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Validate AS5600 register value
 * @param reg_addr Register address
 * @param value Value to validate
 * @return true if valid, false otherwise
 */
bool as5600_validate_register_value(uint8_t reg_addr, uint32_t value);

/**
 * @brief Get safe default value for AS5600 register
 * @param reg_addr Register address
 * @return Safe default value
 */
uint32_t as5600_get_safe_default(uint8_t reg_addr);

/**
 * @brief Check if AS5600 register is critical for safety
 * @param reg_addr Register address
 * @return true if critical, false otherwise
 */
bool as5600_is_critical_register(uint8_t reg_addr);

#ifdef __cplusplus
}
#endif


#endif /* AS5600_REGISTERS_GENERATED_H */
