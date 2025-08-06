/**
 * @file l6470_registers_generated.c
 * @brief L6470 Register Validation Implementation - AUTO-GENERATED
 * 
 * THIS FILE IS AUTO-GENERATED FROM SCHEMA: l6470_registers.yaml
 * DO NOT EDIT MANUALLY - CHANGES WILL BE OVERWRITTEN
 */

#include "l6470_registers_generated.h"

/* Register size lookup table */
static const uint8_t l6470_register_sizes[] = {
    [1] = 3,  /* ABS_POS */
    [2] = 2,  /* EL_POS */
    [3] = 3,  /* MARK */
    [4] = 3,  /* SPEED */
    [5] = 2,  /* ACC */
    [6] = 2,  /* DEC */
    [7] = 2,  /* MAX_SPEED */
    [8] = 2,  /* MIN_SPEED */
    [9] = 1,  /* KVAL_HOLD */
    [10] = 1,  /* KVAL_RUN */
    [11] = 1,  /* KVAL_ACC */
    [12] = 1,  /* KVAL_DEC */
    [13] = 2,  /* INT_SPD */
    [21] = 2,  /* FS_SPD */
    [19] = 1,  /* OCD_TH */
    [20] = 1,  /* STALL_TH */
    [22] = 1,  /* STEP_MODE */
    [23] = 1,  /* ALARM_EN */
    [24] = 2,  /* CONFIG */
    [25] = 2,  /* STATUS */
};

/* Register access type lookup */
static const bool l6470_register_readonly[] = {
    [1] = false,  /* ABS_POS */
    [2] = false,  /* EL_POS */
    [3] = false,  /* MARK */
    [4] = true,  /* SPEED */
    [5] = false,  /* ACC */
    [6] = false,  /* DEC */
    [7] = false,  /* MAX_SPEED */
    [8] = false,  /* MIN_SPEED */
    [9] = false,  /* KVAL_HOLD */
    [10] = false,  /* KVAL_RUN */
    [11] = false,  /* KVAL_ACC */
    [12] = false,  /* KVAL_DEC */
    [13] = false,  /* INT_SPD */
    [21] = false,  /* FS_SPD */
    [19] = false,  /* OCD_TH */
    [20] = false,  /* STALL_TH */
    [22] = false,  /* STEP_MODE */
    [23] = false,  /* ALARM_EN */
    [24] = false,  /* CONFIG */
    [25] = true,  /* STATUS */
};

uint8_t l6470_get_register_size(uint8_t reg_addr) {
    if (reg_addr < sizeof(l6470_register_sizes)) {
        return l6470_register_sizes[reg_addr];
    }
    return 0;  /* Invalid register */
}

bool l6470_is_register_readonly(uint8_t reg_addr) {
    if (reg_addr < sizeof(l6470_register_readonly)) {
        return l6470_register_readonly[reg_addr];
    }
    return true;  /* Default to read-only for safety */
}

bool l6470_validate_register(uint8_t reg_addr, uint32_t value) {
    /* Basic size validation */
    uint8_t size = l6470_get_register_size(reg_addr);
    if (size == 0) return false;

    /* Value range validation based on size */
    uint32_t max_value = (1UL << (size * 8)) - 1;
    if (value > max_value) return false;

    /* Register-specific validation */
    switch (reg_addr) {
        case L6470_REG_ACC:
            return (value >= 0x1 && value <= 0xFFF);
        case L6470_REG_DEC:
            return (value >= 0x1 && value <= 0xFFF);
        case L6470_REG_MAX_SPEED:
            return (value >= 0x0 && value <= 0x3FF);
        case L6470_REG_MIN_SPEED:
            return (value >= 0x0 && value <= 0xFFF);
        case L6470_REG_KVAL_HOLD:
            return (value >= 0x0 && value <= 0xFF);
        case L6470_REG_KVAL_RUN:
            return (value >= 0x0 && value <= 0xFF);
        case L6470_REG_KVAL_ACC:
            return (value >= 0x0 && value <= 0xFF);
        case L6470_REG_KVAL_DEC:
            return (value >= 0x0 && value <= 0xFF);
        case L6470_REG_INT_SPD:
            return (value >= 0x0 && value <= 0x3FFF);
        case L6470_REG_FS_SPD:
            return (value >= 0x0 && value <= 0x3FF);
        case L6470_REG_OCD_TH:
            return (value >= 0x0 && value <= 0x1F);
        case L6470_REG_STALL_TH:
            return (value >= 0x0 && value <= 0x7F);
        default:
            return true;  /* No specific validation */
    }
}

uint32_t l6470_get_safe_default(uint8_t reg_addr) {
    switch (reg_addr) {
        default:
            return 0;  /* No default defined */
    }
}