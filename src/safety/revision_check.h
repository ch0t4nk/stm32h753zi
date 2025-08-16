/**
 * @file revision_check.h
 * @brief STM32H7 Silicon Revision Safety Validation
 * @author STM32H753ZI Project Team
 * @date 2025-08-15
 *
 * @note This module provides runtime validation of STM32H7 silicon revision
 * to ensure safe 480MHz operation is only attempted on compatible hardware.
 *
 * @warning According to ST errata, 480MHz is only safe on Revision V and
 * later. Earlier revisions (Y) are limited to 400MHz maximum.
 */

#ifndef REVISION_CHECK_H
#define REVISION_CHECK_H

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

/* ==========================================================================
 */
/* Silicon Revision Definitions                                              */
/* ==========================================================================
 */

/** @brief STM32H7 silicon revision enumeration */
typedef enum {
    STM32H7_REV_UNKNOWN = 0, /**< Unknown or unsupported revision */
    STM32H7_REV_Y = 1,       /**< Revision Y (400MHz max, VOS0 unavailable) */
    STM32H7_REV_V = 2,       /**< Revision V (480MHz capable with VOS0) */
    STM32H7_REV_FUTURE = 3   /**< Future revision (assume 480MHz capable) */
} STM32H7_Revision_t;

/** @brief Maximum safe frequencies by revision */
typedef struct {
    STM32H7_Revision_t revision;
    uint32_t max_sysclk_hz;
    uint32_t max_hclk_hz;
    bool vos0_available;
    const char *revision_name;
} RevisionSafetyLimits_t;

/* ==========================================================================
 */
/* Safety Function Declarations                                              */
/* ==========================================================================
 */

/**
 * @brief Detect STM32H7 silicon revision at runtime
 * @return STM32H7_Revision_t Detected silicon revision
 */
STM32H7_Revision_t Revision_Detect(void);

/**
 * @brief Validate if target frequency is safe for detected revision
 * @param target_sysclk_hz Target system clock frequency in Hz
 * @return bool true if safe, false if unsafe
 */
bool Revision_ValidateFrequency(uint32_t target_sysclk_hz);

/**
 * @brief Get safety limits for detected revision
 * @param limits Pointer to structure to fill with limits
 * @return bool true if successful, false if revision unknown
 */
bool Revision_GetSafetyLimits(RevisionSafetyLimits_t *limits);

/**
 * @brief Check if VOS0 voltage scaling is available on this revision
 * @return bool true if VOS0 is available, false otherwise
 */
bool Revision_IsVOS0Available(void);

/**
 * @brief Print silicon revision information to debug output
 */
void Revision_PrintInfo(void);

/**
 * @brief Enforce safety limits before clock configuration
 * @param target_sysclk_hz Target system clock frequency
 * @return HAL_StatusTypeDef HAL_OK if safe, HAL_ERROR if unsafe
 */
HAL_StatusTypeDef Revision_EnforceSafetyLimits(uint32_t target_sysclk_hz);

/* ==========================================================================
 */
/* Compile-Time Safety Checks                                                */
/* ==========================================================================
 */

// These will be validated at runtime, but provide compile-time documentation
#define STM32H7_REV_Y_MAX_FREQ_HZ 400000000UL // 400MHz maximum for Rev Y
#define STM32H7_REV_V_MAX_FREQ_HZ 480000000UL // 480MHz maximum for Rev V

// Ensure our target frequency doesn't exceed ANY revision capability
#if defined(TARGET_SYSCLK_FREQUENCY_HZ) &&                                    \
    (TARGET_SYSCLK_FREQUENCY_HZ > STM32H7_REV_V_MAX_FREQ_HZ)
#error                                                                        \
    "Target SYSCLK frequency exceeds maximum capability of any STM32H7 revision"
#endif

#endif /* REVISION_CHECK_H */

/**
 * @note Silicon Revision Safety Rules:
 * 1. ALWAYS check revision before configuring clocks above 400MHz
 * 2. NEVER attempt VOS0 on revision Y (hardware limitation)
 * 3. VALIDATE runtime revision against compile-time assumptions
 * 4. ENFORCE frequency limits based on actual silicon capabilities
 * 5. PROVIDE clear error messages for unsupported configurations
 */
