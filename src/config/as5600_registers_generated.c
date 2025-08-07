/**
 * @file as5600_registers_generated.c
 * @brief AS5600 Register Validation Implementation - AUTO-GENERATED
 * 
 * THIS FILE IS AUTO-GENERATED FROM SCHEMA: as5600_registers.yaml
 * DO NOT EDIT MANUALLY - CHANGES WILL BE OVERWRITTEN
 */

#include "as5600_registers_generated.h"

/* Register size lookup table */
static const uint8_t as5600_register_sizes[] = {
    [0] = 1,  /* ZMCO */
    [1] = 1,  /* ZPOS_H */
    [2] = 1,  /* ZPOS_L */
    [3] = 1,  /* MPOS_H */
    [4] = 1,  /* MPOS_L */
    [5] = 1,  /* MANG_H */
    [6] = 1,  /* MANG_L */
    [7] = 1,  /* CONF_H */
    [8] = 1,  /* CONF_L */
    [12] = 1,  /* RAW_ANGLE_H */
    [13] = 1,  /* RAW_ANGLE_L */
    [14] = 1,  /* ANGLE_H */
    [15] = 1,  /* ANGLE_L */
    [11] = 1,  /* STATUS */
    [26] = 1,  /* AGC */
    [27] = 1,  /* MAGNITUDE_H */
    [28] = 1,  /* MAGNITUDE_L */
    [255] = 1,  /* BURN */
};

/* Register access type lookup */
static const bool as5600_register_readonly[] = {
    [0] = true,  /* ZMCO */
    [1] = false,  /* ZPOS_H */
    [2] = false,  /* ZPOS_L */
    [3] = false,  /* MPOS_H */
    [4] = false,  /* MPOS_L */
    [5] = false,  /* MANG_H */
    [6] = false,  /* MANG_L */
    [7] = false,  /* CONF_H */
    [8] = false,  /* CONF_L */
    [12] = true,  /* RAW_ANGLE_H */
    [13] = true,  /* RAW_ANGLE_L */
    [14] = true,  /* ANGLE_H */
    [15] = true,  /* ANGLE_L */
    [11] = true,  /* STATUS */
    [26] = true,  /* AGC */
    [27] = true,  /* MAGNITUDE_H */
    [28] = true,  /* MAGNITUDE_L */
    [255] = false,  /* BURN */
};

uint8_t as5600_get_register_size(uint8_t reg_addr) {
    if (reg_addr < sizeof(as5600_register_sizes)) {
        return as5600_register_sizes[reg_addr];
    }
    return 0;  /* Invalid register */
}

bool as5600_is_register_readonly(uint8_t reg_addr) {
    if (reg_addr < sizeof(as5600_register_readonly)) {
        return as5600_register_readonly[reg_addr];
    }
    return true;  /* Default to read-only for safety */
}

bool as5600_validate_register(uint8_t reg_addr, uint32_t value) {
    /* Basic size validation */
    uint8_t size = as5600_get_register_size(reg_addr);
    if (size == 0) return false;

    /* Value range validation based on size */
    uint32_t max_value = (1UL << (size * 8)) - 1;
    if (value > max_value) return false;

    /* Register-specific validation */
    switch (reg_addr) {
        case AS5600_REG_ZMCO:
            return (value >= 0x0 && value <= 0x3);
        case AS5600_REG_AGC:
            return (value >= 0x0 && value <= 0xFF);
        default:
            return true;  /* No specific validation */
    }
}

uint32_t as5600_get_safe_default(uint8_t reg_addr) {
    switch (reg_addr) {
        case AS5600_REG_CONF_H:
            return 0x0;
        case AS5600_REG_CONF_L:
            return 0x0;
        default:
            return 0;  /* No default defined */
    }
}