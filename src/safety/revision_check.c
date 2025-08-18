/**
 * @file revision_check.c
 * @brief STM32H7 Silicon Revision Safety Validation Implementation
 * @author STM32H753ZI Project Team
 * @date 2025-08-15
 */

#include "revision_check.h"
#include <stdio.h>

#include "config/safety_config.h" // For STM32H7_REV_ID_Y, STM32H7_REV_ID_V

/* Private variables */
static STM32H7_Revision_t detected_revision = STM32H7_REV_UNKNOWN;
static bool revision_detected = false;

/* Safety limits table */
static const RevisionSafetyLimits_t revision_limits[] = {
    {STM32H7_REV_Y, 400000000UL, 200000000UL, false, "Revision Y"},
    {STM32H7_REV_V, 480000000UL, 240000000UL, true, "Revision V"},
    {STM32H7_REV_FUTURE, 480000000UL, 240000000UL, true, "Future Revision"}};

/* ==========================================================================
 */
/* Public Function Implementations                                           */
/* ==========================================================================
 */

/**
 * @brief Detect STM32H7 silicon revision at runtime
 */
STM32H7_Revision_t Revision_Detect(void) {
    if (revision_detected) {
        return detected_revision;
    }

    // Read DBGMCU_IDCODE register to get revision information
    uint32_t idcode = DBGMCU->IDCODE;
    uint32_t dev_id = idcode & 0xFFF;          // Device ID (bits 11:0)
    uint32_t rev_id = (idcode >> 16) & 0xFFFF; // Revision ID (bits 31:16)

    if (rev_id == STM32H7_REV_ID_Y) {
        detected_revision = STM32H7_REV_Y;
    } else if (rev_id == STM32H7_REV_ID_V) {
        detected_revision = STM32H7_REV_V;
    } else {
        detected_revision = STM32H7_REV_FUTURE;
    }
    revision_detected = true;
    return detected_revision;
}

/**
 * @brief Get revision safety limits for the detected revision
 */
const RevisionSafetyLimits_t *Revision_GetLimits(void) {
    STM32H7_Revision_t rev = Revision_Detect();
    for (size_t i = 0;
         i < sizeof(revision_limits) / sizeof(revision_limits[0]); ++i) {
        if (revision_limits[i].revision == rev) {
            return &revision_limits[i];
        }
    }
    return &revision_limits[0]; // Default to first entry
}

/**
 * @brief Print revision info (for diagnostics)
 */
void Revision_PrintInfo(void) {
    const RevisionSafetyLimits_t *limits = Revision_GetLimits();
    printf("[Revision Check] Detected: %s | Max SYSCLK: %lu Hz | Max HCLK: "
           "%lu Hz | VOS0: %s\n",
           limits->revision_name, limits->max_sysclk_hz, limits->max_hclk_hz,
           limits->vos0_available ? "YES" : "NO");
}
