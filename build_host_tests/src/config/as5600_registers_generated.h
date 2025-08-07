/**
 * @file as5600_registers_generated.h
 * @brief AS5600 Register Definitions - AUTO-GENERATED
 * 
 * THIS FILE IS AUTO-GENERATED FROM SCHEMA: as5600_registers.yaml
 * DO NOT EDIT MANUALLY - CHANGES WILL BE OVERWRITTEN
 * 
 * Generated: 2025-08-07 00:18:38 UTC
 * Chip: AS5600
 * Datasheet: Rev 1.21
 * Framework: Custom
 * 
 * @note This file provides SSOT register definitions for embedded driver
 * @note development. All register addresses, bit fields, and validation
 * @note constants are generated from the schema to ensure consistency.
 * 
 * @warning SAFETY CRITICAL - Validate all register configurations
 * @warning before deployment. Incorrect settings may damage hardware.
 */
#ifndef AS5600_REGISTERS_GENERATED_H
#define AS5600_REGISTERS_GENERATED_H

#include <stdint.h>
#include <stdbool.h>

/* SSOT Configuration Dependencies */
#include "hardware_config.h"
#include "safety_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* AS5600 Register Addresses                                       */
/* ========================================================================== */

#define AS5600_REG_ZMCO                0x00    /* Programming count register (OTP) */
#define AS5600_REG_ZPOS_H              0x01    /* Zero position MSB (OTP) */
#define AS5600_REG_ZPOS_L              0x02    /* Zero position LSB (OTP) */
#define AS5600_REG_MPOS_H              0x03    /* Maximum position MSB (OTP) */
#define AS5600_REG_MPOS_L              0x04    /* Maximum position LSB (OTP) */
#define AS5600_REG_MANG_H              0x05    /* Maximum angle MSB (OTP) */
#define AS5600_REG_MANG_L              0x06    /* Maximum angle LSB (OTP) */
#define AS5600_REG_CONF_H              0x07    /* Configuration MSB (OTP) */
#define AS5600_REG_CONF_L              0x08    /* Configuration LSB (OTP) */
#define AS5600_REG_RAW_ANGLE_H         0x0C    /* Raw angle MSB (unfiltered) */
#define AS5600_REG_RAW_ANGLE_L         0x0D    /* Raw angle LSB (unfiltered) */
#define AS5600_REG_ANGLE_H             0x0E    /* Filtered angle MSB */
#define AS5600_REG_ANGLE_L             0x0F    /* Filtered angle LSB */
#define AS5600_REG_STATUS              0x0B    /* Status register */
#define AS5600_REG_AGC                 0x1A    /* Automatic Gain Control */
#define AS5600_REG_MAGNITUDE_H         0x1B    /* Magnitude MSB */
#define AS5600_REG_MAGNITUDE_L         0x1C    /* Magnitude LSB */
#define AS5600_REG_BURN                0xFF    /* Burn command register */

/* ========================================================================== */
/* AS5600 Register Bit Field Definitions                           */
/* ========================================================================== */

/* CONF_H Register Bit Fields */
#define AS5600_CONF_H_PM_Pos                     0U
#define AS5600_CONF_H_PM_Msk                     (0x3U)
#define AS5600_CONF_H_PM_0                       0U    /* NOM (Normal) */
#define AS5600_CONF_H_PM_1                       1U    /* LPM1 (Low Power 1) */
#define AS5600_CONF_H_PM_2                       2U    /* LPM2 (Low Power 2) */
#define AS5600_CONF_H_PM_3                       3U    /* LPM3 (Low Power 3) */
#define AS5600_CONF_H_HYST_Pos                   2U
#define AS5600_CONF_H_HYST_Msk                   (0xCU)
#define AS5600_CONF_H_HYST_0                     0U    /* OFF */
#define AS5600_CONF_H_HYST_1                     1U    /* 1 LSB */
#define AS5600_CONF_H_HYST_2                     2U    /* 2 LSBs */
#define AS5600_CONF_H_HYST_3                     3U    /* 3 LSBs */
#define AS5600_CONF_H_OUTS_Pos                   4U
#define AS5600_CONF_H_OUTS_Msk                   (0x30U)
#define AS5600_CONF_H_OUTS_0                     0U    /* Analog (full range) */
#define AS5600_CONF_H_OUTS_1                     1U    /* Analog (reduced range) */
#define AS5600_CONF_H_OUTS_2                     2U    /* Digital PWM */
#define AS5600_CONF_H_OUTS_3                     3U    /* Reserved */
#define AS5600_CONF_H_PWMF_Pos                   6U
#define AS5600_CONF_H_PWMF_Msk                   (0xC0U)
#define AS5600_CONF_H_PWMF_0                     0U    /* 115 Hz */
#define AS5600_CONF_H_PWMF_1                     1U    /* 230 Hz */
#define AS5600_CONF_H_PWMF_2                     2U    /* 460 Hz */
#define AS5600_CONF_H_PWMF_3                     3U    /* 920 Hz */

/* CONF_L Register Bit Fields */
#define AS5600_CONF_L_SF_Pos                     0U
#define AS5600_CONF_L_SF_Msk                     (0x3U)
#define AS5600_CONF_L_SF_0                       0U    /* 16x */
#define AS5600_CONF_L_SF_1                       1U    /* 8x */
#define AS5600_CONF_L_SF_2                       2U    /* 4x */
#define AS5600_CONF_L_SF_3                       3U    /* 2x */
#define AS5600_CONF_L_FTH_Pos                    2U
#define AS5600_CONF_L_FTH_Msk                    (0x1CU)
#define AS5600_CONF_L_FTH_0                      0U    /* Slow filter only */
#define AS5600_CONF_L_FTH_1                      1U    /* 6 LSBs */
#define AS5600_CONF_L_FTH_2                      2U    /* 7 LSBs */
#define AS5600_CONF_L_FTH_3                      3U    /* 9 LSBs */
#define AS5600_CONF_L_FTH_4                      4U    /* 18 LSBs */
#define AS5600_CONF_L_FTH_5                      5U    /* 21 LSBs */
#define AS5600_CONF_L_FTH_6                      6U    /* 24 LSBs */
#define AS5600_CONF_L_FTH_7                      7U    /* 10 LSBs */
#define AS5600_CONF_L_WD                         (1U << 5)
#define AS5600_CONF_L_WD_0                       0U    /* OFF */
#define AS5600_CONF_L_WD_1                       1U    /* ON */

/* STATUS Register Bit Fields */
#define AS5600_STATUS_MH                         (1U << 3)
#define AS5600_STATUS_ML                         (1U << 4)
#define AS5600_STATUS_MD                         (1U << 5)

/* ========================================================================== */
/* AS5600 Combined Register Helper Macros                          */
/* ========================================================================== */

#define AS5600_COMBINE_ZPOS(high, low) \
    (((uint16_t)(high) << 8) | (uint16_t)(low))

#define AS5600_GET_ZPOS_HIGH(value) \
    ((uint8_t)((value) >> 8))

#define AS5600_GET_ZPOS_LOW(value) \
    ((uint8_t)((value) & 0xFF))

#define AS5600_COMBINE_MPOS(high, low) \
    (((uint16_t)(high) << 8) | (uint16_t)(low))

#define AS5600_GET_MPOS_HIGH(value) \
    ((uint8_t)((value) >> 8))

#define AS5600_GET_MPOS_LOW(value) \
    ((uint8_t)((value) & 0xFF))

#define AS5600_COMBINE_MANG(high, low) \
    (((uint16_t)(high) << 8) | (uint16_t)(low))

#define AS5600_GET_MANG_HIGH(value) \
    ((uint8_t)((value) >> 8))

#define AS5600_GET_MANG_LOW(value) \
    ((uint8_t)((value) & 0xFF))

#define AS5600_COMBINE_RAW_ANGLE(high, low) \
    (((uint16_t)(high) << 8) | (uint16_t)(low))

#define AS5600_GET_RAW_ANGLE_HIGH(value) \
    ((uint8_t)((value) >> 8))

#define AS5600_GET_RAW_ANGLE_LOW(value) \
    ((uint8_t)((value) & 0xFF))

#define AS5600_COMBINE_ANGLE(high, low) \
    (((uint16_t)(high) << 8) | (uint16_t)(low))

#define AS5600_GET_ANGLE_HIGH(value) \
    ((uint8_t)((value) >> 8))

#define AS5600_GET_ANGLE_LOW(value) \
    ((uint8_t)((value) & 0xFF))

#define AS5600_COMBINE_MAGNITUDE(high, low) \
    (((uint16_t)(high) << 8) | (uint16_t)(low))

#define AS5600_GET_MAGNITUDE_HIGH(value) \
    ((uint8_t)((value) >> 8))

#define AS5600_GET_MAGNITUDE_LOW(value) \
    ((uint8_t)((value) & 0xFF))

/* ========================================================================== */
/* AS5600 Validation Constants                                     */
/* ========================================================================== */

/* Safe Default Values */
#define AS5600_SAFE_DEFAULT_CONF_H               0x00U
#define AS5600_SAFE_DEFAULT_CONF_L               0x00U
#define AS5600_SAFE_DEFAULT_polling_rate         0x64U

/* ========================================================================== */
/* AS5600 Register Operation Functions                             */
/* ========================================================================== */

/**
 * @brief Validate AS5600 register configuration
 * @param reg_addr Register address
 * @param value Register value
 * @return true if valid, false otherwise
 */
bool as5600_validate_register(uint8_t reg_addr, uint32_t value);

/**
 * @brief Get register size in bytes
 * @param reg_addr Register address
 * @return Size in bytes (1-3)
 */
uint8_t as5600_get_register_size(uint8_t reg_addr);

/**
 * @brief Check if register is read-only
 * @param reg_addr Register address
 * @return true if read-only, false if writable
 */
bool as5600_is_register_readonly(uint8_t reg_addr);

/**
 * @brief Get safe default value for register
 * @param reg_addr Register address
 * @return Default value or 0 if no default defined
 */
uint32_t as5600_get_safe_default(uint8_t reg_addr);

#ifdef __cplusplus
}
#endif

#endif /* AS5600_REGISTERS_GENERATED_H */
