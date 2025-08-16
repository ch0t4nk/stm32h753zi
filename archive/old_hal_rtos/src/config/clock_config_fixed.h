/**
 * @file clock_config_fixed.h
 * @brief Fixed STM32H753ZI clock configuration header for 480MHz operation
 * @author Based on real-time debugging analysis and official ST confirmation
 */

#ifndef CLOCK_CONFIG_FIXED_H
#define CLOCK_CONFIG_FIXED_H

#include "safety/revision_check.h"
#include "stm32h7xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup Clock_Config_Constants Clock Configuration Constants
 * @{
 */

// Timeout values in milliseconds
#define HSI_STARTUP_TIMEOUT_MS 100  ///< HSI startup timeout
#define VOS_READY_TIMEOUT_MS 1000   ///< VOS ready timeout
#define PLL_DISABLE_TIMEOUT_MS 100  ///< PLL disable timeout
#define PLL_LOCK_TIMEOUT_MS 100     ///< PLL lock timeout
#define CLOCK_SWITCH_TIMEOUT_MS 100 ///< Clock switch timeout

// Target frequencies
#define TARGET_SYSCLK_FREQ_HZ 480000000 ///< Target SYSCLK frequency (480MHz)
#define HSI_FREQ_HZ 64000000            ///< HSI frequency (64MHz)
#define PLL1_VCO_FREQ_HZ 480000000      ///< PLL1 VCO frequency (480MHz)

// PLL1 configuration values for 480MHz from HSI
#define PLL1_M_DIVIDER 4     ///< PLL1 M divider (HSI/4 = 16MHz)
#define PLL1_N_MULTIPLIER 30 ///< PLL1 N multiplier (16MHz*30 = 480MHz)
#define PLL1_P_DIVIDER 1     ///< PLL1 P divider (480MHz/1 = 480MHz SYSCLK)
#define PLL1_Q_DIVIDER 2     ///< PLL1 Q divider (480MHz/2 = 240MHz)
#define PLL1_R_DIVIDER 2     ///< PLL1 R divider (480MHz/2 = 240MHz)

// HSI calibration value (confirmed for Revision V)
#define HSI_CALIBRATION_VALUE 64 ///< HSI calibration value

/**
 * @}
 */

/** @defgroup Clock_Config_Types Clock Configuration Types
 * @{
 */

/**
 * @brief Clock configuration error codes
 */
typedef enum {
    CLOCK_CONFIG_SUCCESS = 0,                  ///< Configuration successful
    CLOCK_CONFIG_ERROR_REVISION_CHECK_FAILED,  ///< Silicon revision check
                                               ///< failed
    CLOCK_CONFIG_ERROR_REVISION_NOT_SUPPORTED, ///< Silicon revision doesn't
                                               ///< support 480MHz
    CLOCK_CONFIG_ERROR_HSI_TIMEOUT,          ///< HSI oscillator ready timeout
    CLOCK_CONFIG_ERROR_HSI_NOT_READY,        ///< HSI oscillator not ready
    CLOCK_CONFIG_ERROR_VOS_TIMEOUT,          ///< VOS0 ready timeout
    CLOCK_CONFIG_ERROR_VOS_NOT_READY,        ///< VOS0 not ready
    CLOCK_CONFIG_ERROR_PLL_DISABLE_TIMEOUT,  ///< PLL1 disable timeout
    CLOCK_CONFIG_ERROR_PLL_LOCK_TIMEOUT,     ///< PLL1 lock timeout
    CLOCK_CONFIG_ERROR_PLL_NOT_READY,        ///< PLL1 not ready
    CLOCK_CONFIG_ERROR_CLOCK_SWITCH_TIMEOUT, ///< Clock switch timeout
    CLOCK_CONFIG_ERROR_COUNT                 ///< Total number of error types
} ClockConfigError_t;

/**
 * @brief Clock configuration state structure
 */
typedef struct {
    bool hsi_ready;                ///< HSI oscillator ready status
    bool vos0_ready;               ///< VOS0 voltage scaling ready status
    bool pll1_locked;              ///< PLL1 lock status
    uint32_t sysclk_freq_hz;       ///< Current SYSCLK frequency in Hz
    STM32H7_Revision_t revision;   ///< Silicon revision
    ClockConfigError_t last_error; ///< Last error encountered
} ClockState_t;

/**
 * @}
 */

/** @defgroup Clock_Config_Functions Clock Configuration Functions
 * @{
 */

/**
 * @brief Initialize system clocks for 480MHz operation on STM32H753ZI Revision
 * V
 *
 * This function configures the STM32H753ZI for maximum performance operation:
 * 1. Verifies silicon revision supports 480MHz (Revision V required)
 * 2. Configures HSI oscillator with proper calibration
 * 3. Sets VOS0 voltage scaling for 480MHz operation
 * 4. Configures PLL1 for 480MHz using HSI source
 * 5. Switches system clock to PLL1
 *
 * @note This function includes comprehensive timeout handling to prevent
 *       the infinite loops that were causing system hangs
 *
 * @return ClockConfigError_t Success or specific error code
 */
ClockConfigError_t Clock_Init(void);

/**
 * @brief Get current clock configuration state
 * @return Pointer to clock state structure (read-only)
 */
const ClockState_t *Clock_GetState(void);

/**
 * @brief Get string description of clock configuration error
 * @param error Error code
 * @return String description of the error
 */
const char *Clock_GetErrorString(ClockConfigError_t error);

/**
 * @brief Check if 480MHz operation is available on current silicon
 * @return true if 480MHz is supported, false otherwise
 */
static inline bool Clock_Is480MHzSupported(void) {
    const ClockState_t *state = Clock_GetState();
    return (state->revision >= STM32H7_REVISION_V);
}

/**
 * @brief Get current SYSCLK frequency
 * @return SYSCLK frequency in Hz
 */
static inline uint32_t Clock_GetSysclkFreq(void) {
    const ClockState_t *state = Clock_GetState();
    return state->sysclk_freq_hz;
}

/**
 * @brief Check if clock system is fully initialized and running at 480MHz
 * @return true if running at 480MHz, false otherwise
 */
static inline bool Clock_IsRunningAt480MHz(void) {
    const ClockState_t *state = Clock_GetState();
    return (state->hsi_ready && state->vos0_ready && state->pll1_locked &&
            state->sysclk_freq_hz == TARGET_SYSCLK_FREQ_HZ);
}

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_CONFIG_FIXED_H */
