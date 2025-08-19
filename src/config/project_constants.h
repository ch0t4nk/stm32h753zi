/**
 * @file project_constants.h
 * @brief Project-level Single Source of Truth (SSOT) constants
 *
 * This header centralizes small project constants that are used across the
 * firmware and may sometimes be materialized as hardcoded values in low-level
 * code (for example static buffer sizes or magic numbers). Rules:
 *
 * - Prefer referencing SSOT_* macros from application code instead of
 *   hardcoding numerics.
 * - If a consumer requires a materialized constant (e.g. static array size),
 *   create a local conversion macro in the consumer that maps the SSOT to the
 *   materialized value. Document whether the SSOT can be changed at runtime
 *   or requires a rebuild/reflash.
 *
 * Update semantics comments below indicate whether changing the value at
 * runtime is supported (Runtime = yes/no). If Runtime=no then a firmware
 * rebuild and device reset/reflash is required.
 */

#ifndef PROJECT_CONSTANTS_H
#define PROJECT_CONSTANTS_H

#include <stdint.h>

/* ------------------------------------------------------------------------- */
/* System state validation values (SSOT) */
/* ------------------------------------------------------------------------- */

/**
 * SSOT_SYSTEM_STATE_MAGIC
 * - Purpose: Magic value used to validate the in-memory SystemState_t
 * - Units: 32-bit identifier
 * - Runtime: no (change requires rebuild/reflash)
 */
#define SSOT_SYSTEM_STATE_MAGIC (0x53595354UL) /* 'SYST' */

/**
 * SSOT_SYSTEM_STATE_VERSION
 * - Purpose: Version number for the SystemState_t layout
 * - Units: 16-bit integer
 * - Runtime: no (changing requires coordinated firmware updates)
 */
#define SSOT_SYSTEM_STATE_VERSION ((uint16_t)0x0100)

/* ------------------------------------------------------------------------- */
/* Motor / hardware constants (SSOT)                                         */
/* ------------------------------------------------------------------------- */

/**
 * SSOT_MAX_MOTORS
 * - Purpose: Number of motors the firmware supports (array sizes, loops)
 * - Units: count
 * - Runtime: no (array-backed structures depend on this at compile time)
 */
#define SSOT_MAX_MOTORS 2U

/* Oscillator default values used by system startup logic. Runtime: no */
#ifndef SSOT_HSE_VALUE
#define SSOT_HSE_VALUE ((uint32_t)25000000U)
#endif

#ifndef SSOT_CSI_VALUE
#define SSOT_CSI_VALUE ((uint32_t)4000000U)
#endif

#ifndef SSOT_HSI_VALUE
#define SSOT_HSI_VALUE ((uint32_t)64000000U)
#endif

/* ------------------------------------------------------------------------- */
/* Communication constants (SSOT)                                            */
/* ------------------------------------------------------------------------- */

/**
 * SSOT_MAX_MESSAGE_PAYLOAD
 * - Purpose: Canonical maximum message payload for comm protocols
 * - Units: bytes
 * - Runtime: no (used for static buffer sizing in many modules)
 */
#define SSOT_MAX_MESSAGE_PAYLOAD 1024U

/**
 * SSOT_DEFAULT_UART_BAUDRATE
 * - Purpose: Default UART baudrate for debug/console channels
 * - Units: bits per second
 * - Runtime: yes (drivers may allow runtime configuration)
 */
#define SSOT_DEFAULT_UART_BAUDRATE 115200U

/* ------------------------------------------------------------------------- */
/* Error code bases (SSOT) */
/* ------------------------------------------------------------------------- */

/**
 * SSOT_ERROR_CODE_SYSTEM_BASE
 * - Purpose: Base for system-level error codes
 * - Units: numeric base (add offsets for specific features)
 * - Runtime: no
 */
#define SSOT_ERROR_CODE_SYSTEM_BASE 0x00010000U

/**
 * SSOT_ERROR_CODE_MOTOR_BASE
 * - Purpose: Base for motor subsystem error codes
 * - Units: numeric base
 * - Runtime: no
 */
#define SSOT_ERROR_CODE_MOTOR_BASE 0x00020000U

/**
 * SSOT_ERROR_RT_BASE
 * - Purpose: Base for real-time control subsystem error codes
 * - Units: numeric base
 * - Runtime: no
 */
#ifndef SSOT_ERROR_RT_BASE
#define SSOT_ERROR_RT_BASE 0x00006100U
#endif

/* ------------------------------------------------------------------------- */
/* Project feature flags (workflow-level SSOT may override at build time)    */
/* ------------------------------------------------------------------------- */

/**
 * SSOT_FEATURE_SEMANTIC_SEARCH
 * - Purpose: Enable semantic search integration
 * - Units: boolean (0/1)
 * - Runtime: yes (scripts use this to alter behavior)
 */
#define SSOT_FEATURE_SEMANTIC_SEARCH 1

#ifndef SSOT_NOISE_SEED
/* SSOT_NOISE_SEED
 * - Purpose: Canonical seed used by simulation/random generators used in
 *   optimization/simulation modules
 * - Units: 32-bit value
 * - Runtime: no (changing requires rebuild/reflash for deterministic tests)
 */
#define SSOT_NOISE_SEED 0x12345678U
#endif

#ifndef SSOT_ERROR_ADAPTIVE_CURRENT_BASE
/* SSOT_ERROR_ADAPTIVE_CURRENT_BASE
 * - Purpose: Base value for adaptive-current-algorithm-specific errors
 * - Units: numeric base
 * - Runtime: no
 */
#define SSOT_ERROR_ADAPTIVE_CURRENT_BASE 0x7000U
#endif

/* ------------------------------------------------------------------------- */
/* Hardcoded Exceptions Index
 * - Purpose: Quick index for maintainers to find files that intentionally
 *   retain hardcoded values for safety or vendor reasons. See the
 *   corresponding headers (for example clock_config.h) for details.
 *
 * Entries are lightweight, reviewable string descriptors. When adding an
 * entry include: file path, brief rationale, and Runtime: yes/no.
 */
#define HARDCODED_EXCEPTION_INDEX_CLOCK_STARTUP                               \
    "src/config/clock_config.h: Core/Src/system_stm32h7xx.c - vendor reset "  \
    "values; Runtime=no"

/* KVAL default used by telemetry/telemetry-optimizer code (kept in SSOT so
 * simulation and telemetry modules can reference a canonical value). Runtime:
 * no (changing affects recorded telemetry and simulations reproducibility)
 */
#ifndef SSOT_KVAL_DEFAULT
#define SSOT_KVAL_DEFAULT ((uint8_t)0x29U)
#endif

/* Additional documented hardcoded exceptions - append with file, short
 * rationale and runtime semantics (yes/no). These mirror entries in
 * `src/config/clock_config.h` for cross-reference audits.
 */
#define HARDCODED_EXCEPTION_STARTUP_MASKS                                     \
    "Core/Src/system_stm32h7xx.c: RCC reset masks and PLL DIV/FRAC reset "    \
    "values left local due to vendor startup dependencies; Runtime=no"

#define HARDCODED_EXCEPTION_SIMULATION_BITMASKS                               \
    "src/simulation/motor_simulation.c: simulation status/flag bitmasks "     \
    "(MD/ML/MH) kept local for simulator clarity; Runtime=yes"

/* Additional documented hardcoded exceptions (append as needed). */
#define HARDCODED_EXCEPTION_SYSTEM_TIMING                                     \
    "src/drivers/lowlevel/rtc_clock.c: vendor timing constants; Runtime=no"

#if 1 /* Legacy short-base compatibility mapping to SSOT */
/* Short 0x1xxx..0x8xxx legacy bases kept for compatibility. Prefer
 * SSOT_ERROR_CODE_* macros above in new code.
 */
#ifndef SSOT_LEGACY_ERROR_GENERIC_BASE
#define SSOT_LEGACY_ERROR_GENERIC_BASE 0x1000U
#endif

#ifndef SSOT_LEGACY_ERROR_MOTOR_BASE
#define SSOT_LEGACY_ERROR_MOTOR_BASE 0x2000U
#endif

#ifndef SSOT_LEGACY_ERROR_ENCODER_BASE
#define SSOT_LEGACY_ERROR_ENCODER_BASE 0x3000U
#endif

#ifndef SSOT_LEGACY_ERROR_COMM_BASE
#define SSOT_LEGACY_ERROR_COMM_BASE 0x4000U
#endif

#ifndef SSOT_LEGACY_ERROR_SAFETY_BASE
#define SSOT_LEGACY_ERROR_SAFETY_BASE 0x5000U
#endif

#ifndef SSOT_LEGACY_ERROR_SYSTEM_BASE
#define SSOT_LEGACY_ERROR_SYSTEM_BASE 0x6000U
#endif

#ifndef SSOT_LEGACY_ERROR_CONFIG_BASE
#define SSOT_LEGACY_ERROR_CONFIG_BASE 0x7000U
#endif

#ifndef SSOT_LEGACY_ERROR_TEST_BASE
#define SSOT_LEGACY_ERROR_TEST_BASE 0x8000U
#endif
#endif /* Legacy mapping */
#endif /* PROJECT_CONSTANTS_H */
