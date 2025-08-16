/**
 * @file clock_config_480mhz.c
 * @brief STM32H753ZI 480MHz clock configuration using SSOT error handling
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * This implementation addresses the clock initialization hang identified
 * through real-time debugging. It integrates with our existing SSOT
 * error handling system.
 */

#include "clock_config_480mhz.h"
#include "common/error_handling.h" // Use existing SSOT error handling
#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <string.h>

// Timeout constants for oscillator ready detection
#define HSI_READY_TIMEOUT_MS 2       // HSI should be ready within 2ms
#define PLL_READY_TIMEOUT_MS 10      // PLL should lock within 10ms
#define VOLTAGE_SCALING_TIMEOUT_MS 5 // VOS switching timeout

// Clock configuration state tracking
static struct {
    bool hsi_ready;
    bool vos0_ready;
    bool pll1_locked;
    uint32_t sysclk_freq_hz;
    SystemError_t last_error;
} clock_state;

// Simple debug macro - integrates with project debug system
#ifdef DEBUG
#define DEBUG_PRINTF(...)                                                     \
    do { /* Debug output - add your system's debug here */                    \
    } while (0)
#else
#define DEBUG_PRINTF(...)                                                     \
    do {                                                                      \
    } while (0)
#endif

/**
 * @brief Wait for register bit with timeout
 * @param reg_addr Register address to monitor
 * @param mask Bit mask to check
 * @param expected Expected value (0 or non-zero)
 * @param timeout_ms Timeout in milliseconds
 * @param description Description for debug output
 * @return true if bit reached expected state, false on timeout
 */
static bool wait_for_register_bit(uint32_t reg_addr, uint32_t mask,
                                  uint32_t expected, uint32_t timeout_ms,
                                  const char *description) {
    uint32_t start_time = HAL_GetTick();
    uint32_t current_time;

    DEBUG_PRINTF("🔍 Waiting for %s (timeout: %lu ms)...\n", description,
                 timeout_ms);

    do {
        uint32_t reg_value = *(volatile uint32_t *)reg_addr;
        bool bit_state = (reg_value & mask) != 0;

        if ((expected != 0) == bit_state) {
            DEBUG_PRINTF("✅ %s ready\n", description);
            return true;
        }

        current_time = HAL_GetTick();
        // Handle tick overflow
        if (current_time < start_time) {
            start_time = current_time;
        }
    } while ((current_time - start_time) < timeout_ms);

    DEBUG_PRINTF("❌ %s timeout\n", description);
    return false;
}

/**
 * @brief Configure HSI oscillator with proper calibration
 * @return SystemError_t Success or error code
 */
static SystemError_t configure_hsi_oscillator(void) {
    DEBUG_PRINTF("🔧 Configuring HSI oscillator...\n");

    // Enable HSI if not already enabled
    if ((RCC->CR & RCC_CR_HSION) == 0) {
        RCC->CR |= RCC_CR_HSION;
    }

    // HSI calibration is done automatically by hardware on STM32H7
    // No manual calibration register needed

    // Wait for HSI ready with timeout
    if (!wait_for_register_bit((uint32_t)&RCC->CR, RCC_CR_HSIRDY, 1,
                               HSI_READY_TIMEOUT_MS, "HSI ready")) {
        clock_state.last_error = ERROR_CLOCK_HSI_TIMEOUT;
        return ERROR_CLOCK_HSI_TIMEOUT;
    }

    clock_state.hsi_ready = true;
    DEBUG_PRINTF("✅ HSI oscillator configured and ready\n");
    return SYSTEM_OK;
}

/**
 * @brief Configure VOS0 voltage scaling for 480MHz operation
 * @return SystemError_t Success or error code
 */
static SystemError_t configure_vos0_voltage_scaling(void) {
    DEBUG_PRINTF("🔧 Configuring VOS0 voltage scaling...\n");

    // Configure for highest voltage scaling (VOS0) for 480MHz
    // Set voltage regulator to scale 0 (highest performance)
    PWR->D3CR &= ~PWR_D3CR_VOS; // Clear VOS bits
    // VOS0 = 00, already cleared above

    // Wait for voltage scaling ready
    if (!wait_for_register_bit((uint32_t)&PWR->D3CR, PWR_D3CR_VOSRDY, 1,
                               VOLTAGE_SCALING_TIMEOUT_MS, "VOS0 ready")) {
        clock_state.last_error = ERROR_CLOCK_VOS_TIMEOUT;
        return ERROR_CLOCK_VOS_TIMEOUT;
    }

    clock_state.vos0_ready = true;
    DEBUG_PRINTF("✅ VOS0 voltage scaling configured\n");
    return SYSTEM_OK;
}

/**
 * @brief Configure PLL1 for 480MHz using HSI source
 * @return SystemError_t Success or error code
 */
static SystemError_t configure_pll1_for_480mhz(void) {
    DEBUG_PRINTF("🔧 Configuring PLL1 for 480MHz...\n");

    // Verify prerequisites
    if (!clock_state.hsi_ready) {
        DEBUG_PRINTF("❌ HSI not ready for PLL configuration\n");
        clock_state.last_error = ERROR_CLOCK_HSI_TIMEOUT;
        return ERROR_CLOCK_HSI_TIMEOUT;
    }

    if (!clock_state.vos0_ready) {
        DEBUG_PRINTF("❌ VOS0 not ready for PLL configuration\n");
        clock_state.last_error = ERROR_CLOCK_VOS_TIMEOUT;
        return ERROR_CLOCK_VOS_TIMEOUT;
    }

    // Disable PLL1 before configuration
    RCC->CR &= ~RCC_CR_PLL1ON;

    // Wait for PLL1 to be disabled
    if (!wait_for_register_bit((uint32_t)&RCC->CR, RCC_CR_PLL1RDY, 0,
                               PLL_READY_TIMEOUT_MS, "PLL1 disabled")) {
        clock_state.last_error = ERROR_CLOCK_PLL_TIMEOUT;
        return ERROR_CLOCK_PLL_TIMEOUT;
    }

    // Configure PLL1 source to HSI
    RCC->PLLCKSELR &= ~RCC_PLLCKSELR_PLLSRC;
    RCC->PLLCKSELR |= RCC_PLLCKSELR_PLLSRC_HSI;

    // Configure PLL1 dividers for 480MHz from HSI (64MHz)
    // HSI (64MHz) / M(4) = 16MHz (PLL input)
    // 16MHz * N(60) = 960MHz (VCO)
    // 960MHz / P(2) = 480MHz (SYSCLK)
    RCC->PLLCKSELR &= ~RCC_PLLCKSELR_DIVM1;
    RCC->PLLCKSELR |= (4 << RCC_PLLCKSELR_DIVM1_Pos);

    RCC->PLL1DIVR = 0;
    RCC->PLL1DIVR |= ((60 - 1) << RCC_PLL1DIVR_N1_Pos); // N = 60
    RCC->PLL1DIVR |= ((2 - 1) << RCC_PLL1DIVR_P1_Pos);  // P = 2
    RCC->PLL1DIVR |= ((2 - 1) << RCC_PLL1DIVR_Q1_Pos);  // Q = 2
    RCC->PLL1DIVR |= ((2 - 1) << RCC_PLL1DIVR_R1_Pos);  // R = 2

    // Enable PLL1 P output
    RCC->PLLCFGR |= RCC_PLLCFGR_DIVP1EN;

    // Enable PLL1
    RCC->CR |= RCC_CR_PLL1ON;

    // Wait for PLL1 lock
    if (!wait_for_register_bit((uint32_t)&RCC->CR, RCC_CR_PLL1RDY, 1,
                               PLL_READY_TIMEOUT_MS, "PLL1 locked")) {
        clock_state.last_error = ERROR_CLOCK_PLL_TIMEOUT;
        return ERROR_CLOCK_PLL_TIMEOUT;
    }

    clock_state.pll1_locked = true;
    DEBUG_PRINTF("✅ PLL1 configured and locked at 480MHz\n");
    return SYSTEM_OK;
}

/**
 * @brief Switch system clock to PLL1
 * @return SystemError_t Success or error code
 */
static SystemError_t switch_to_pll1_clock(void) {
    DEBUG_PRINTF("🔧 Switching to PLL1 clock...\n");

    // Verify PLL1 is ready
    if (!clock_state.pll1_locked) {
        DEBUG_PRINTF("❌ PLL1 not ready for clock switch\n");
        clock_state.last_error = ERROR_CLOCK_PLL_TIMEOUT;
        return ERROR_CLOCK_PLL_TIMEOUT;
    }

    // Configure flash latency for 480MHz
    // VOS0, 480MHz requires 4 wait states
    __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_4);

    // Configure peripheral clocks before switching
    // HCLK = SYSCLK / 2 = 240MHz
    // PCLK1 = HCLK / 2 = 120MHz
    // PCLK2 = HCLK / 2 = 120MHz
    RCC->D1CFGR &= ~RCC_D1CFGR_HPRE;
    RCC->D1CFGR |= RCC_D1CFGR_HPRE_DIV2; // HCLK = SYSCLK/2

    RCC->D2CFGR &= ~(RCC_D2CFGR_D2PPRE1 | RCC_D2CFGR_D2PPRE2);
    RCC->D2CFGR |= RCC_D2CFGR_D2PPRE1_DIV2; // PCLK1 = HCLK/2
    RCC->D2CFGR |= RCC_D2CFGR_D2PPRE2_DIV2; // PCLK2 = HCLK/2

    // Switch system clock to PLL1
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL1;

    // Wait for clock switch to complete
    if (!wait_for_register_bit((uint32_t)&RCC->CFGR, RCC_CFGR_SWS,
                               RCC_CFGR_SWS_PLL1, 100,
                               "Clock switch to PLL1")) {
        clock_state.last_error = ERROR_CLOCK_SWITCH_TIMEOUT;
        return ERROR_CLOCK_SWITCH_TIMEOUT;
    }

    // Update system clock tracking
    clock_state.sysclk_freq_hz = 480000000;

    // Update HAL system clock
    SystemCoreClockUpdate();

    DEBUG_PRINTF("✅ System clock switched to PLL1 (480MHz)\n");
    return SYSTEM_OK;
}

/**
 * @brief Initialize 480MHz clock configuration for STM32H753ZI Revision V
 * @return SystemError_t Success or error code
 */
SystemError_t Clock_Init_480MHz(void) {
    SystemError_t result;

    DEBUG_PRINTF("\n🚀 STM32H753ZI 480MHz Clock Configuration\n");
    DEBUG_PRINTF("==========================================\n");

    // Initialize state
    memset(&clock_state, 0, sizeof(clock_state));

    // Step 1: Configure HSI oscillator
    result = configure_hsi_oscillator();
    if (result != SYSTEM_OK) {
        DEBUG_PRINTF("❌ HSI configuration failed\n");
        return result;
    }

    // Step 2: Configure VOS0 voltage scaling
    result = configure_vos0_voltage_scaling();
    if (result != SYSTEM_OK) {
        DEBUG_PRINTF("❌ VOS0 configuration failed\n");
        return result;
    }

    // Step 3: Configure PLL1 for 480MHz
    result = configure_pll1_for_480mhz();
    if (result != SYSTEM_OK) {
        DEBUG_PRINTF("❌ PLL1 configuration failed\n");
        return result;
    }

    // Step 4: Switch to PLL1 clock
    result = switch_to_pll1_clock();
    if (result != SYSTEM_OK) {
        DEBUG_PRINTF("❌ Clock switch failed\n");
        return result;
    }

    DEBUG_PRINTF("\n🎉 480MHz clock configuration completed successfully!\n");
    DEBUG_PRINTF("   SYSCLK: %lu MHz\n", clock_state.sysclk_freq_hz / 1000000);
    DEBUG_PRINTF("   HCLK:   %lu MHz\n", HAL_RCC_GetHCLKFreq() / 1000000);
    DEBUG_PRINTF("   PCLK1:  %lu MHz\n", HAL_RCC_GetPCLK1Freq() / 1000000);
    DEBUG_PRINTF("   PCLK2:  %lu MHz\n", HAL_RCC_GetPCLK2Freq() / 1000000);

    return SYSTEM_OK;
}

/**
 * @brief Get last error from clock configuration
 * @return SystemError_t Last error encountered
 */
SystemError_t Clock_Get_Last_Error(void) {
    return clock_state.last_error;
}

/**
 * @brief Check if system is running at 480MHz
 * @return true if running at 480MHz, false otherwise
 */
bool Clock_Is_Running_At_480MHz(void) {
    return (clock_state.hsi_ready && clock_state.vos0_ready &&
            clock_state.pll1_locked &&
            clock_state.sysclk_freq_hz == 480000000);
}
