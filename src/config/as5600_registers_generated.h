/**
 * @file as5600_registers_generated.h
 * @brief AS5600 Register Definitions - Auto-Generated from Schema
 * @version 1.0
 * @date 2025-08-20 17:00:50
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
#define AS5600_ZMCO_ADDR    0x00    /* Programming count register (OTP) */
#define AS5600_ZPOS_H_ADDR    0x01    /* Zero position MSB (OTP) */
#define AS5600_ZPOS_L_ADDR    0x02    /* Zero position LSB (OTP) */
#define AS5600_MPOS_H_ADDR    0x03    /* Maximum position MSB (OTP) */
#define AS5600_MPOS_L_ADDR    0x04    /* Maximum position LSB (OTP) */
#define AS5600_MANG_H_ADDR    0x05    /* Maximum angle MSB (OTP) */
#define AS5600_MANG_L_ADDR    0x06    /* Maximum angle LSB (OTP) */
#define AS5600_CONF_H_ADDR    0x07    /* Configuration MSB (OTP) */
#define AS5600_CONF_L_ADDR    0x08    /* Configuration LSB (OTP) */
#define AS5600_RAW_ANGLE_H_ADDR    0x0C    /* Raw angle MSB (unfiltered) */
#define AS5600_RAW_ANGLE_L_ADDR    0x0D    /* Raw angle LSB (unfiltered) */
#define AS5600_ANGLE_H_ADDR    0x0E    /* Filtered angle MSB */
#define AS5600_ANGLE_L_ADDR    0x0F    /* Filtered angle LSB */
#define AS5600_STATUS_ADDR    0x0B    /* Status register */
#define AS5600_AGC_ADDR    0x1A    /* Automatic Gain Control */
#define AS5600_MAGNITUDE_H_ADDR    0x1B    /* Magnitude MSB */
#define AS5600_MAGNITUDE_L_ADDR    0x1C    /* Magnitude LSB */
#define AS5600_BURN_ADDR    0xFF    /* Burn command register */

/* ========================================================================== */
/* Register Mask Definitions                                                 */
/* ========================================================================== */
#define AS5600_ZMCO_MASK        0x03
#define AS5600_ZPOS_H_MASK        0x0F
#define AS5600_ZPOS_L_MASK        0xFF
#define AS5600_MPOS_H_MASK        0x0F
#define AS5600_MPOS_L_MASK        0xFF
#define AS5600_MANG_H_MASK        0x0F
#define AS5600_MANG_L_MASK        0xFF
#define AS5600_CONF_H_MASK        0xFF
#define AS5600_CONF_L_MASK        0xFF
#define AS5600_RAW_ANGLE_H_MASK        0x0F
#define AS5600_RAW_ANGLE_L_MASK        0xFF
#define AS5600_ANGLE_H_MASK        0x0F
#define AS5600_ANGLE_L_MASK        0xFF
#define AS5600_STATUS_MASK        0x38
#define AS5600_AGC_MASK        0xFF
#define AS5600_MAGNITUDE_H_MASK        0x0F
#define AS5600_MAGNITUDE_L_MASK        0xFF
#define AS5600_BURN_MASK        0xFF

/* ========================================================================== */
/* Register Field Definitions                                                */
/* ========================================================================== */
/* CONF_H register fields */
#define AS5600_CONF_H_PM_POS    0
#define AS5600_CONF_H_PM_MSK    0x0003
#define AS5600_CONF_H_PM_NOM_NORMAL_    0x0000
#define AS5600_CONF_H_PM_LPM1_LOW_POWER_1_    0x0001
#define AS5600_CONF_H_PM_LPM2_LOW_POWER_2_    0x0002
#define AS5600_CONF_H_PM_LPM3_LOW_POWER_3_    0x0003
#define AS5600_CONF_H_HYST_POS    2
#define AS5600_CONF_H_HYST_MSK    0x000C
#define AS5600_CONF_H_HYST_OFF    0x0000
#define AS5600_CONF_H_HYST__1_LSB    0x0004
#define AS5600_CONF_H_HYST__2_LSBS    0x0008
#define AS5600_CONF_H_HYST__3_LSBS    0x000C
#define AS5600_CONF_H_OUTS_POS    4
#define AS5600_CONF_H_OUTS_MSK    0x0030
#define AS5600_CONF_H_OUTS_ANALOG_FULL_RANGE_    0x0000
#define AS5600_CONF_H_OUTS_ANALOG_REDUCED_RANGE_    0x0010
#define AS5600_CONF_H_OUTS_DIGITAL_PWM    0x0020
#define AS5600_CONF_H_OUTS_RESERVED    0x0030
#define AS5600_CONF_H_PWMF_POS    6
#define AS5600_CONF_H_PWMF_MSK    0x00C0
#define AS5600_CONF_H_PWMF__115_HZ    0x0000
#define AS5600_CONF_H_PWMF__230_HZ    0x0040
#define AS5600_CONF_H_PWMF__460_HZ    0x0080
#define AS5600_CONF_H_PWMF__920_HZ    0x00C0

/* CONF_L register fields */
#define AS5600_CONF_L_SF_POS    0
#define AS5600_CONF_L_SF_MSK    0x0003
#define AS5600_CONF_L_SF__16X    0x0000
#define AS5600_CONF_L_SF__8X    0x0001
#define AS5600_CONF_L_SF__4X    0x0002
#define AS5600_CONF_L_SF__2X    0x0003
#define AS5600_CONF_L_FTH_POS    2
#define AS5600_CONF_L_FTH_MSK    0x001C
#define AS5600_CONF_L_FTH_SLOW_FILTER_ONLY    0x0000
#define AS5600_CONF_L_FTH__6_LSBS    0x0004
#define AS5600_CONF_L_FTH__7_LSBS    0x0008
#define AS5600_CONF_L_FTH__9_LSBS    0x000C
#define AS5600_CONF_L_FTH__18_LSBS    0x0010
#define AS5600_CONF_L_FTH__21_LSBS    0x0014
#define AS5600_CONF_L_FTH__24_LSBS    0x0018
#define AS5600_CONF_L_FTH__10_LSBS    0x001C
#define AS5600_CONF_L_WD_POS    5
#define AS5600_CONF_L_WD_MSK    0x0020
#define AS5600_CONF_L_WD_OFF    0x0000
#define AS5600_CONF_L_WD_ON    0x0020

/* STATUS register fields */
#define AS5600_STATUS_MH_POS    3
#define AS5600_STATUS_MH_MSK    0x0008
#define AS5600_STATUS_ML_POS    4
#define AS5600_STATUS_ML_MSK    0x0010
#define AS5600_STATUS_MD_POS    5
#define AS5600_STATUS_MD_MSK    0x0020

/* ========================================================================== */
/* Combined Register Macros (Multi-byte registers)                          */
/* ========================================================================== */
/* ZPOS: Zero position (0-4095) */
#define AS5600_ZPOS_COMBINE(high, low)    (((high) << 8) | (low))
#define AS5600_ZPOS_HIGH(val)            (((val) >> 8) & 0x0F)
#define AS5600_ZPOS_LOW(val)             ((val) & 0xFF)

/* MPOS: Maximum position (0-4095) */
#define AS5600_MPOS_COMBINE(high, low)    (((high) << 8) | (low))
#define AS5600_MPOS_HIGH(val)            (((val) >> 8) & 0x0F)
#define AS5600_MPOS_LOW(val)             ((val) & 0xFF)

/* MANG: Maximum angle (0-4095) */
#define AS5600_MANG_COMBINE(high, low)    (((high) << 8) | (low))
#define AS5600_MANG_HIGH(val)            (((val) >> 8) & 0x0F)
#define AS5600_MANG_LOW(val)             ((val) & 0xFF)

/* RAW_ANGLE: Raw angle reading (0-4095) */
#define AS5600_RAW_ANGLE_COMBINE(high, low)    (((high) << 8) | (low))
#define AS5600_RAW_ANGLE_HIGH(val)            (((val) >> 8) & 0x0F)
#define AS5600_RAW_ANGLE_LOW(val)             ((val) & 0xFF)

/* ANGLE: Filtered angle reading (0-4095) */
#define AS5600_ANGLE_COMBINE(high, low)    (((high) << 8) | (low))
#define AS5600_ANGLE_HIGH(val)            (((val) >> 8) & 0x0F)
#define AS5600_ANGLE_LOW(val)             ((val) & 0xFF)

/* MAGNITUDE: Magnitude reading (0-4095) */
#define AS5600_MAGNITUDE_COMBINE(high, low)    (((high) << 8) | (low))
#define AS5600_MAGNITUDE_HIGH(val)            (((val) >> 8) & 0x0F)
#define AS5600_MAGNITUDE_LOW(val)             ((val) & 0xFF)

/* ========================================================================== */
/* Validation Constants                                                      */
/* ========================================================================== */
/* Safe default values */
#define AS5600_CONF_H_SAFE_DEFAULT    0x0000
#define AS5600_CONF_L_SAFE_DEFAULT    0x0000
#define AS5600_ZPOS_H_SAFE_DEFAULT    0x0000
#define AS5600_ZPOS_L_SAFE_DEFAULT    0x0000
#define AS5600_MPOS_H_SAFE_DEFAULT    0x000F
#define AS5600_MPOS_L_SAFE_DEFAULT    0x00FF
#define AS5600_MANG_H_SAFE_DEFAULT    0x000F
#define AS5600_MANG_L_SAFE_DEFAULT    0x00FF

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

#ifdef ENABLE_LEGACY_REG_NAMES
/* Legacy register names enabled */
#define ZMCO_ADDR AS5600_ZMCO_ADDR
#define CHIP_ZMCO_ADDR AS5600_ZMCO_ADDR
#define ZPOS_H_ADDR AS5600_ZPOS_H_ADDR
#define CHIP_ZPOS_H_ADDR AS5600_ZPOS_H_ADDR
#define ZPOS_L_ADDR AS5600_ZPOS_L_ADDR
#define CHIP_ZPOS_L_ADDR AS5600_ZPOS_L_ADDR
#define MPOS_H_ADDR AS5600_MPOS_H_ADDR
#define CHIP_MPOS_H_ADDR AS5600_MPOS_H_ADDR
#define MPOS_L_ADDR AS5600_MPOS_L_ADDR
#define CHIP_MPOS_L_ADDR AS5600_MPOS_L_ADDR
#define MANG_H_ADDR AS5600_MANG_H_ADDR
#define CHIP_MANG_H_ADDR AS5600_MANG_H_ADDR
#define MANG_L_ADDR AS5600_MANG_L_ADDR
#define CHIP_MANG_L_ADDR AS5600_MANG_L_ADDR
#define CONF_H_ADDR AS5600_CONF_H_ADDR
#define CHIP_CONF_H_ADDR AS5600_CONF_H_ADDR
#define CONF_L_ADDR AS5600_CONF_L_ADDR
#define CHIP_CONF_L_ADDR AS5600_CONF_L_ADDR
#define RAW_ANGLE_H_ADDR AS5600_RAW_ANGLE_H_ADDR
#define CHIP_RAW_ANGLE_H_ADDR AS5600_RAW_ANGLE_H_ADDR
#define RAW_ANGLE_L_ADDR AS5600_RAW_ANGLE_L_ADDR
#define CHIP_RAW_ANGLE_L_ADDR AS5600_RAW_ANGLE_L_ADDR
#define ANGLE_H_ADDR AS5600_ANGLE_H_ADDR
#define CHIP_ANGLE_H_ADDR AS5600_ANGLE_H_ADDR
#define ANGLE_L_ADDR AS5600_ANGLE_L_ADDR
#define CHIP_ANGLE_L_ADDR AS5600_ANGLE_L_ADDR
#define STATUS_ADDR AS5600_STATUS_ADDR
#define CHIP_STATUS_ADDR AS5600_STATUS_ADDR
#define AGC_ADDR AS5600_AGC_ADDR
#define CHIP_AGC_ADDR AS5600_AGC_ADDR
#define MAGNITUDE_H_ADDR AS5600_MAGNITUDE_H_ADDR
#define CHIP_MAGNITUDE_H_ADDR AS5600_MAGNITUDE_H_ADDR
#define MAGNITUDE_L_ADDR AS5600_MAGNITUDE_L_ADDR
#define CHIP_MAGNITUDE_L_ADDR AS5600_MAGNITUDE_L_ADDR
#define BURN_ADDR AS5600_BURN_ADDR
#define CHIP_BURN_ADDR AS5600_BURN_ADDR
#define ZMCO_MASK AS5600_ZMCO_MASK
#define CHIP_ZMCO_MASK AS5600_ZMCO_MASK
#define ZPOS_H_MASK AS5600_ZPOS_H_MASK
#define CHIP_ZPOS_H_MASK AS5600_ZPOS_H_MASK
#define ZPOS_L_MASK AS5600_ZPOS_L_MASK
#define CHIP_ZPOS_L_MASK AS5600_ZPOS_L_MASK
#define MPOS_H_MASK AS5600_MPOS_H_MASK
#define CHIP_MPOS_H_MASK AS5600_MPOS_H_MASK
#define MPOS_L_MASK AS5600_MPOS_L_MASK
#define CHIP_MPOS_L_MASK AS5600_MPOS_L_MASK
#define MANG_H_MASK AS5600_MANG_H_MASK
#define CHIP_MANG_H_MASK AS5600_MANG_H_MASK
#define MANG_L_MASK AS5600_MANG_L_MASK
#define CHIP_MANG_L_MASK AS5600_MANG_L_MASK
#define CONF_H_MASK AS5600_CONF_H_MASK
#define CHIP_CONF_H_MASK AS5600_CONF_H_MASK
#define CONF_L_MASK AS5600_CONF_L_MASK
#define CHIP_CONF_L_MASK AS5600_CONF_L_MASK
#define RAW_ANGLE_H_MASK AS5600_RAW_ANGLE_H_MASK
#define CHIP_RAW_ANGLE_H_MASK AS5600_RAW_ANGLE_H_MASK
#define RAW_ANGLE_L_MASK AS5600_RAW_ANGLE_L_MASK
#define CHIP_RAW_ANGLE_L_MASK AS5600_RAW_ANGLE_L_MASK
#define ANGLE_H_MASK AS5600_ANGLE_H_MASK
#define CHIP_ANGLE_H_MASK AS5600_ANGLE_H_MASK
#define ANGLE_L_MASK AS5600_ANGLE_L_MASK
#define CHIP_ANGLE_L_MASK AS5600_ANGLE_L_MASK
#define STATUS_MASK AS5600_STATUS_MASK
#define CHIP_STATUS_MASK AS5600_STATUS_MASK
#define AGC_MASK AS5600_AGC_MASK
#define CHIP_AGC_MASK AS5600_AGC_MASK
#define MAGNITUDE_H_MASK AS5600_MAGNITUDE_H_MASK
#define CHIP_MAGNITUDE_H_MASK AS5600_MAGNITUDE_H_MASK
#define MAGNITUDE_L_MASK AS5600_MAGNITUDE_L_MASK
#define CHIP_MAGNITUDE_L_MASK AS5600_MAGNITUDE_L_MASK
#define BURN_MASK AS5600_BURN_MASK
#define CHIP_BURN_MASK AS5600_BURN_MASK
#define CONF_H_PM_POS AS5600_CONF_H_PM_POS
#define CONF_H_PM_MSK AS5600_CONF_H_PM_MSK
#define CONF_H_PM_NOM_NORMAL_ AS5600_CONF_H_PM_NOM_NORMAL_
#define CONF_H_PM_LPM1_LOW_POWER_1_ AS5600_CONF_H_PM_LPM1_LOW_POWER_1_
#define CONF_H_PM_LPM2_LOW_POWER_2_ AS5600_CONF_H_PM_LPM2_LOW_POWER_2_
#define CONF_H_PM_LPM3_LOW_POWER_3_ AS5600_CONF_H_PM_LPM3_LOW_POWER_3_
#define CONF_H_HYST_POS AS5600_CONF_H_HYST_POS
#define CONF_H_HYST_MSK AS5600_CONF_H_HYST_MSK
#define CONF_H_HYST_OFF AS5600_CONF_H_HYST_OFF
#define CONF_H_HYST_1_LSB AS5600_CONF_H_HYST__1_LSB
#define CONF_H_HYST_2_LSBS AS5600_CONF_H_HYST__2_LSBS
#define CONF_H_HYST_3_LSBS AS5600_CONF_H_HYST__3_LSBS
#define CONF_H_OUTS_POS AS5600_CONF_H_OUTS_POS
#define CONF_H_OUTS_MSK AS5600_CONF_H_OUTS_MSK
#define CONF_H_OUTS_ANALOG_FULL_RANGE_ AS5600_CONF_H_OUTS_ANALOG_FULL_RANGE_
#define CONF_H_OUTS_ANALOG_REDUCED_RANGE_ AS5600_CONF_H_OUTS_ANALOG_REDUCED_RANGE_
#define CONF_H_OUTS_DIGITAL_PWM AS5600_CONF_H_OUTS_DIGITAL_PWM
#define CONF_H_OUTS_RESERVED AS5600_CONF_H_OUTS_RESERVED
#define CONF_H_PWMF_POS AS5600_CONF_H_PWMF_POS
#define CONF_H_PWMF_MSK AS5600_CONF_H_PWMF_MSK
#define CONF_H_PWMF_115_HZ AS5600_CONF_H_PWMF__115_HZ
#define CONF_H_PWMF_230_HZ AS5600_CONF_H_PWMF__230_HZ
#define CONF_H_PWMF_460_HZ AS5600_CONF_H_PWMF__460_HZ
#define CONF_H_PWMF_920_HZ AS5600_CONF_H_PWMF__920_HZ
#define CONF_L_SF_POS AS5600_CONF_L_SF_POS
#define CONF_L_SF_MSK AS5600_CONF_L_SF_MSK
#define CONF_L_SF_16X AS5600_CONF_L_SF__16X
#define CONF_L_SF_8X AS5600_CONF_L_SF__8X
#define CONF_L_SF_4X AS5600_CONF_L_SF__4X
#define CONF_L_SF_2X AS5600_CONF_L_SF__2X
#define CONF_L_FTH_POS AS5600_CONF_L_FTH_POS
#define CONF_L_FTH_MSK AS5600_CONF_L_FTH_MSK
#define CONF_L_FTH_SLOW_FILTER_ONLY AS5600_CONF_L_FTH_SLOW_FILTER_ONLY
#define CONF_L_FTH_6_LSBS AS5600_CONF_L_FTH__6_LSBS
#define CONF_L_FTH_7_LSBS AS5600_CONF_L_FTH__7_LSBS
#define CONF_L_FTH_9_LSBS AS5600_CONF_L_FTH__9_LSBS
#define CONF_L_FTH_18_LSBS AS5600_CONF_L_FTH__18_LSBS
#define CONF_L_FTH_21_LSBS AS5600_CONF_L_FTH__21_LSBS
#define CONF_L_FTH_24_LSBS AS5600_CONF_L_FTH__24_LSBS
#define CONF_L_FTH_10_LSBS AS5600_CONF_L_FTH__10_LSBS
#define CONF_L_WD_POS AS5600_CONF_L_WD_POS
#define CONF_L_WD_MSK AS5600_CONF_L_WD_MSK
#define CONF_L_WD_OFF AS5600_CONF_L_WD_OFF
#define CONF_L_WD_ON AS5600_CONF_L_WD_ON
#define STATUS_MH_POS AS5600_STATUS_MH_POS
#define STATUS_MH_MSK AS5600_STATUS_MH_MSK
#define STATUS_ML_POS AS5600_STATUS_ML_POS
#define STATUS_ML_MSK AS5600_STATUS_ML_MSK
#define STATUS_MD_POS AS5600_STATUS_MD_POS
#define STATUS_MD_MSK AS5600_STATUS_MD_MSK
#define ZPOS_COMBINE(high, low) AS5600_ZPOS_COMBINE(high, low)
#define ZPOS_HIGH(val) AS5600_ZPOS_HIGH(val)
#define ZPOS_LOW(val) AS5600_ZPOS_LOW(val)
#define MPOS_COMBINE(high, low) AS5600_MPOS_COMBINE(high, low)
#define MPOS_HIGH(val) AS5600_MPOS_HIGH(val)
#define MPOS_LOW(val) AS5600_MPOS_LOW(val)
#define MANG_COMBINE(high, low) AS5600_MANG_COMBINE(high, low)
#define MANG_HIGH(val) AS5600_MANG_HIGH(val)
#define MANG_LOW(val) AS5600_MANG_LOW(val)
#define RAW_ANGLE_COMBINE(high, low) AS5600_RAW_ANGLE_COMBINE(high, low)
#define RAW_ANGLE_HIGH(val) AS5600_RAW_ANGLE_HIGH(val)
#define RAW_ANGLE_LOW(val) AS5600_RAW_ANGLE_LOW(val)
#define ANGLE_COMBINE(high, low) AS5600_ANGLE_COMBINE(high, low)
#define ANGLE_HIGH(val) AS5600_ANGLE_HIGH(val)
#define ANGLE_LOW(val) AS5600_ANGLE_LOW(val)
#define MAGNITUDE_COMBINE(high, low) AS5600_MAGNITUDE_COMBINE(high, low)
#define MAGNITUDE_HIGH(val) AS5600_MAGNITUDE_HIGH(val)
#define MAGNITUDE_LOW(val) AS5600_MAGNITUDE_LOW(val)
#define CONF_H_SAFE_DEFAULT AS5600_CONF_H_SAFE_DEFAULT
#define CONF_L_SAFE_DEFAULT AS5600_CONF_L_SAFE_DEFAULT
#define ZPOS_H_SAFE_DEFAULT AS5600_ZPOS_H_SAFE_DEFAULT
#define ZPOS_L_SAFE_DEFAULT AS5600_ZPOS_L_SAFE_DEFAULT
#define MPOS_H_SAFE_DEFAULT AS5600_MPOS_H_SAFE_DEFAULT
#define MPOS_L_SAFE_DEFAULT AS5600_MPOS_L_SAFE_DEFAULT
#define MANG_H_SAFE_DEFAULT AS5600_MANG_H_SAFE_DEFAULT
#define MANG_L_SAFE_DEFAULT AS5600_MANG_L_SAFE_DEFAULT
#endif /* ENABLE_LEGACY_REG_NAMES */

#endif /* AS5600_REGISTERS_GENERATED_H */
