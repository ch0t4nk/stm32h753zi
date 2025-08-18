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

STM32H7_Revision_t Revision_Detect(void);
const RevisionSafetyLimits_t *Revision_GetLimits(void);
void Revision_PrintInfo(void);

#endif /* REVISION_CHECK_H */
