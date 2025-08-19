/**
 * @file clock_config.h
 * @brief SSOT for clock/reset register constants and documented hardcoded
 *        exceptions used by low-level startup code.
 *
 * This header centralizes named constants used by the reset and PLL setup
 * performed in `Core/Src/system_stm32h7xx.c`. For safety reasons some
 * low-level register bitmasks/values remain intentionally hardcoded in the
 * startup sequence; such exceptions are documented below so reviewers can
 * quickly understand why they are localized.
 */

#ifndef CLOCK_CONFIG_H
#define CLOCK_CONFIG_H

#include <stdint.h>

/* ------------------------------------------------------------------------- */
/* Default oscillator values (mirrors SSOT project_constants but grouped here)
 */
/* ------------------------------------------------------------------------- */
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
/* Reset values used by system startup (named for traceability)              */
/* These mirror vendor reset constants but give symbolic names for audits.   */
/* ------------------------------------------------------------------------- */
#define RCC_CFGR_RESET_VALUE ((uint32_t)0x00000000U)
#define RCC_PLLCKSELR_RESET ((uint32_t)0x02020200U)
#define RCC_PLLCFGR_RESET ((uint32_t)0x01FF0000U)
#define RCC_PLL1DIVR_RESET ((uint32_t)0x01010280U)
#define RCC_PLL1FRACR_RESET ((uint32_t)0x00000000U)
#define RCC_PLL2DIVR_RESET ((uint32_t)0x01010280U)
#define RCC_PLL2FRACR_RESET ((uint32_t)0x00000000U)
#define RCC_PLL3DIVR_RESET ((uint32_t)0x01010280U)
#define RCC_PLL3FRACR_RESET ((uint32_t)0x00000000U)

/* Mask values used in startup (documented reasons for being localized): */
#define RCC_CR_RESET_CLEAR_MASK ((uint32_t)0xEAF6ED7FU)

/* PLL fractional divisor used in VCO fractional calculation (vendor magic)
 * - Purpose: Divisor used to scale PLL fractional field to fractional value
 * - Units: numeric divisor
 * - Runtime: no (vendor-derived constant, changing without full analysis
 *            risks incorrect clock calculation)
 */
#define SSOT_PLL_FRACN_DIVISOR ((float_t)0x2000)

/* ------------------------------------------------------------------------- */
/* Hardcoded Exceptions Registry
 * - Purpose: Compact list of files/locations that intentionally keep
 *   hardcoded numeric values for safety, hardware timing, or vendor-compat
 *   reasons. Each entry must include a short rationale and whether changing
 *   it requires a rebuild/reset (Runtime: yes/no).
 *
 * For cross-reference and quick audits see `HARDCODED_EXCEPTION_INDEX_*`
 * macros in `src/config/project_constants.h`.
 */

/* Local example (kept for reviewer convenience) */
#define HARDCODED_EXCEPTION_SYSTEM_STARTUP                                    \
    "Core/Src/system_stm32h7xx.c: vendor reset values - safety/localized; "   \
    "Runtime=no"

/* Add more exceptions as needed; keep this list minimal and reviewable. */

#endif /* CLOCK_CONFIG_H */
