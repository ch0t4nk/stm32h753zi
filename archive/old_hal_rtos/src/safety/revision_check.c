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

    printf("[REVISION] DBGMCU_IDCODE: 0x%08lX, DEV_ID: 0x%03lX, REV_ID: "
           "0x%04lX\r\n",
           idcode, dev_id, rev_id);

    // Check if this is STM32H753ZI (DEV_ID should be 0x450)
    if (dev_id != 0x450) {
        printf("[REVISION] WARNING: Unexpected device ID 0x%03lX (expected "
               "0x450 for STM32H753ZI)\r\n",
               dev_id);
        detected_revision = STM32H7_REV_UNKNOWN;
        revision_detected = true;
        return detected_revision;
    }

    // Decode revision based on REV_ID
    // Based on STM32H7 reference manual and errata sheets
    switch (rev_id) {
    case STM32H7_REV_ID_Y: // Revision Y
        printf("[REVISION] Detected Revision Y (0x%04X) - 400MHz max, VOS0 "
               "unavailable\r\n",
               STM32H7_REV_ID_Y);
        detected_revision = STM32H7_REV_Y;
        break;

    case STM32H7_REV_ID_V: // Revision V
        printf("[REVISION] Detected Revision V (0x%04X) - 480MHz capable with "
               "VOS0\r\n",
               STM32H7_REV_ID_V);
        detected_revision = STM32H7_REV_V;
        break;

    default:
        if (rev_id >= STM32H7_REV_ID_FUTURE_MIN) {
            printf("[REVISION] Detected future revision (0x%04lX) - assuming "
                   "480MHz capable\r\n",
                   rev_id);
            detected_revision = STM32H7_REV_FUTURE;
        } else {
            printf("[REVISION] WARNING: Unknown revision 0x%04lX - assuming "
                   "conservative limits\r\n",
                   rev_id);
            detected_revision = STM32H7_REV_Y; // Conservative fallback
        }
        break;
    }

    revision_detected = true;
    return detected_revision;
}

/**
 * @brief Validate if target frequency is safe for detected revision
 */
bool Revision_ValidateFrequency(uint32_t target_sysclk_hz) {
    STM32H7_Revision_t revision = Revision_Detect();

    for (size_t i = 0;
         i < sizeof(revision_limits) / sizeof(revision_limits[0]); i++) {
        if (revision_limits[i].revision == revision) {
            bool is_safe =
                target_sysclk_hz <= revision_limits[i].max_sysclk_hz;

            if (!is_safe) {
                printf("[REVISION] SAFETY VIOLATION: Target frequency %lu Hz "
                       "exceeds %s maximum %lu Hz\r\n",
                       target_sysclk_hz, revision_limits[i].revision_name,
                       revision_limits[i].max_sysclk_hz);
            }

            return is_safe;
        }
    }

    // Unknown revision - be conservative
    printf(
        "[REVISION] SAFETY WARNING: Unknown revision, limiting to 400MHz\r\n");
    return target_sysclk_hz <= 400000000UL;
}

/**
 * @brief Get safety limits for detected revision
 */
bool Revision_GetSafetyLimits(RevisionSafetyLimits_t *limits) {
    if (!limits) {
        return false;
    }

    STM32H7_Revision_t revision = Revision_Detect();

    for (size_t i = 0;
         i < sizeof(revision_limits) / sizeof(revision_limits[0]); i++) {
        if (revision_limits[i].revision == revision) {
            *limits = revision_limits[i];
            return true;
        }
    }

    return false;
}

/**
 * @brief Check if VOS0 voltage scaling is available on this revision
 */
bool Revision_IsVOS0Available(void) {
    STM32H7_Revision_t revision = Revision_Detect();

    for (size_t i = 0;
         i < sizeof(revision_limits) / sizeof(revision_limits[0]); i++) {
        if (revision_limits[i].revision == revision) {
            return revision_limits[i].vos0_available;
        }
    }

    return false; // Conservative default
}

/**
 * @brief Print silicon revision information to debug output
 */
void Revision_PrintInfo(void) {
    STM32H7_Revision_t revision = Revision_Detect();
    RevisionSafetyLimits_t limits;

    printf("\r\n=== STM32H7 Silicon Revision Info ===\r\n");

    if (Revision_GetSafetyLimits(&limits)) {
        printf("Revision: %s\r\n", limits.revision_name);
        printf("Max SYSCLK: %lu Hz (%.0f MHz)\r\n", limits.max_sysclk_hz,
               limits.max_sysclk_hz / 1000000.0f);
        printf("Max HCLK: %lu Hz (%.0f MHz)\r\n", limits.max_hclk_hz,
               limits.max_hclk_hz / 1000000.0f);
        printf("VOS0 Available: %s\r\n", limits.vos0_available ? "Yes" : "No");

        if (limits.vos0_available) {
            printf("480MHz Operation: SAFE ✅\r\n");
        } else {
            printf("480MHz Operation: UNSAFE ❌ (Limited to 400MHz)\r\n");
        }
    } else {
        printf("Revision: Unknown (Conservative limits applied)\r\n");
        printf("Max SYSCLK: 400000000 Hz (400 MHz)\r\n");
        printf("VOS0 Available: No\r\n");
        printf("480MHz Operation: UNSAFE ❌\r\n");
    }

    printf("=====================================\r\n\r\n");
}

/**
 * @brief Enforce safety limits before clock configuration
 */
HAL_StatusTypeDef Revision_EnforceSafetyLimits(uint32_t target_sysclk_hz) {
    // First check if VOS0 is required for target frequency
    if (target_sysclk_hz > 400000000UL) {
        if (!Revision_IsVOS0Available()) {
            printf("[REVISION] CRITICAL SAFETY ERROR: Target frequency %lu Hz "
                   "requires VOS0, but VOS0 is not available on this "
                   "revision\r\n",
                   target_sysclk_hz);
            printf("[REVISION] SAFETY ACTION: Refusing to configure unsafe "
                   "frequency\r\n");
            return HAL_ERROR;
        }
    }

    // Validate frequency is within revision limits
    if (!Revision_ValidateFrequency(target_sysclk_hz)) {
        printf("[REVISION] CRITICAL SAFETY ERROR: Target frequency %lu Hz "
               "exceeds silicon revision capabilities\r\n",
               target_sysclk_hz);
        printf("[REVISION] SAFETY ACTION: Refusing to configure unsafe "
               "frequency\r\n");
        return HAL_ERROR;
    }

    printf("[REVISION] SAFETY CHECK PASSED: Target frequency %lu Hz is safe "
           "for this revision\r\n",
           target_sysclk_hz);

    return HAL_OK;
}
