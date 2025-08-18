/**
 * @file safety_system.h
 * @brief Safety System Manager - STM32H753ZI Stepper Motor Project
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @note Critical safety system implementation following:
 * - .github/instructions/safety-rt.instructions.md
 * - config/safety_config.h (SSOT)
 *
 * @warning This system is SAFETY-CRITICAL. All modifications must be
 * thoroughly tested and validated before deployment.
 */

#ifndef SAFETY_SYSTEM_H
#define SAFETY_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Watchdog configuration structure (SSOT)
 */
typedef struct {
    uint32_t timeout_ms;           /**< Watchdog timeout period */
    uint32_t warning_threshold_ms; /**< Warning before timeout */
    bool enabled;                  /**< Watchdog enabled */
    uint32_t last_refresh;         /**< Last refresh timestamp */
    uint32_t refresh_count;        /**< Total refresh count */
    uint32_t timeout_count;        /**< Timeout event count */
    uint32_t missed_refresh_count; /**< Missed refresh count */
} WatchdogConfig_t;
#define SAFETY_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

// ...[content omitted for brevity, see previous message for full content]...

#ifdef __cplusplus
}
#endif

#endif /* SAFETY_SYSTEM_H */
