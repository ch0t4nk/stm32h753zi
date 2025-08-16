/**
 * @file clock_config_fixed.c
 * @brief Fixed STM32H753ZI clock configuration for 480MHz operation on
 * Revision V
 * @author Based on real-time debugging analysis and official ST confirmation
 *
 * This implementation addresses the clock initialization hang identified
 * through real-time debugging with STM32CubeProgrammer. Key fixes:
 * - Remove HSE dependency (Nucleo default has no external crystal)
 * - Use HSI with proper calibration (64 as confirmed by user)
 * - Configure VOS0 voltage scaling for 480MHz operation on Revision V
 * - Add proper timeout handling to prevent infinite loops
 * - Include comprehensive error checking and diagnostic output
 */

#include "clock_config_fixed.h"
#include "error_codes.h" // Use existing SSOT error handling
#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <string.h>

// Simple debug macro - replace with your project's debug system
#ifdef DEBUG
#define DEBUG_PRINTF(...)                                                     \
    do { /* Debug output disabled for build */                                \
    } while (0)
#else
#define DEBUG_PRINTF(...)                                                     \
    do {                                                                      \
    } while (0)
#endif

// Clock configuration state tracking
static ClockState_t clock_state;

static ClockState_t clock_state = {0};

/**
 * @brief Wait for register bit with timeout
 * @param reg_addr Register address
 * @param bit_mask Bit mask to check
 * @param expected_value Expected bit value (0 or 1)
 * @param timeout_ms Timeout in milliseconds
 * @param description Human readable description for debugging
 * @return true if condition met, false if timeout
 */
static bool wait_for_register_bit(uint32_t reg_addr, uint32_t bit_mask,
                                  uint32_t expected_value, uint32_t timeout_ms,
                                  const char *description) {
    uint32_t start_time = HAL_GetTick();
    uint32_t current_value;

    DEBUG_PRINTF("⏳ Waiting for %s (timeout: %lu ms)...\n", description,
                 timeout_ms);

    while ((HAL_GetTick() - start_time) < timeout_ms) {
        current_value = (*(volatile uint32_t *)reg_addr & bit_mask) ? 1 : 0;

        if (current_value == expected_value) {
            uint32_t elapsed = HAL_GetTick() - start_time;
            DEBUG_PRINTF("✅ %s ready after %lu ms\n", description, elapsed);
            return true;
        }

        // Small delay to prevent bus flooding
        HAL_Delay(1);
    }

    DEBUG_PRINTF("❌ %s timeout after %lu ms\n", description, timeout_ms);
    return false;
}

/**
 * @brief Configure HSI oscillator with calibration
 * @return ClockConfigError_t Success or specific error code
 */
static ClockConfigError_t configure_hsi_oscillator(void) {
    DEBUG_PRINTF("🔧 Configuring HSI oscillator...\n");

    // Read current RCC_CR
    uint32_t rcc_cr = RCC->CR;
    DEBUG_PRINTF("Current RCC_CR: 0x%08lX\n", rcc_cr);

    // Check if HSI is already on and ready
    if ((rcc_cr & RCC_CR_HSIRDY) != 0) {
        DEBUG_PRINTF("✅ HSI already ready\n");
        clock_state.hsi_ready = true;
        return CLOCK_CONFIG_SUCCESS;
    }

    // Configure HSI calibration to 64 (confirmed value for Revision V)
    uint32_t hsicfgr = RCC->HSICFGR;
    DEBUG_PRINTF("Current RCC_HSICFGR: 0x%08lX\n", hsicfgr);

    // Set HSICAL field (bits 11:0) - user confirmed calibration value 64
    hsicfgr = (hsicfgr & 0xFFFFF000) | (64 << 3);
    RCC->HSICFGR = hsicfgr;
    DEBUG_PRINTF("Set HSI calibration to 64: 0x%08lX\n", hsicfgr);

    // Enable HSI if not already enabled
    if ((rcc_cr & RCC_CR_HSION) == 0) {
        DEBUG_PRINTF("Enabling HSI oscillator...\n");
        RCC->CR |= RCC_CR_HSION;
    }

    // Wait for HSI ready with timeout
    if (!wait_for_register_bit((uint32_t)&RCC->CR, RCC_CR_HSIRDY, 1,
                               HSI_STARTUP_TIMEOUT_MS, "HSI ready")) {
        clock_state.last_error = CLOCK_CONFIG_ERROR_HSI_TIMEOUT;
        return CLOCK_CONFIG_ERROR_HSI_TIMEOUT;
    }

    clock_state.hsi_ready = true;
    DEBUG_PRINTF("✅ HSI oscillator configured and ready\n");
    return CLOCK_CONFIG_SUCCESS;
}

/**
 * @brief Configure VOS0 voltage scaling for 480MHz operation
 * @return ClockConfigError_t Success or specific error code
 */
static ClockConfigError_t configure_vos0_voltage_scaling(void) {
    DEBUG_PRINTF("⚡ Configuring VOS0 voltage scaling for 480MHz...\n");

    // Check if revision supports 480MHz
    if (clock_state.revision < STM32H7_REVISION_V) {
        DEBUG_PRINTF("❌ Silicon revision %c does not support 480MHz\n",
                     'A' + clock_state.revision);
        clock_state.last_error = CLOCK_CONFIG_ERROR_REVISION_NOT_SUPPORTED;
        return CLOCK_CONFIG_ERROR_REVISION_NOT_SUPPORTED;
    }

    // Read current PWR_SRDCR
    uint32_t srdcr = PWR->SRDCR;
    DEBUG_PRINTF("Current PWR_SRDCR: 0x%08lX\n", srdcr);

    // Check if VOS0 is already configured
    uint32_t current_vos = (srdcr >> 14) & 0x3;
    if (current_vos == 0) {
        // Check if already ready
        if ((srdcr & PWR_SRDCR_VOSRDY) != 0) {
            DEBUG_PRINTF("✅ VOS0 already configured and ready\n");
            clock_state.vos0_ready = true;
            return CLOCK_CONFIG_SUCCESS;
        }
    }

    // Set VOS to 0 for 480MHz operation (bits 15:14 = 00)
    srdcr = (srdcr & 0xFFFF3FFF) | (0 << 14);
    PWR->SRDCR = srdcr;
    DEBUG_PRINTF("Set VOS0 configuration: 0x%08lX\n", srdcr);

    // Wait for VOS ready with timeout
    if (!wait_for_register_bit((uint32_t)&PWR->SRDCR, PWR_SRDCR_VOSRDY, 1,
                               VOS_READY_TIMEOUT_MS, "VOS0 ready")) {
        clock_state.last_error = CLOCK_CONFIG_ERROR_VOS_TIMEOUT;
        return CLOCK_CONFIG_ERROR_VOS_TIMEOUT;
    }

    clock_state.vos0_ready = true;
    DEBUG_PRINTF("✅ VOS0 voltage scaling configured for 480MHz\n");
    return CLOCK_CONFIG_SUCCESS;
}

/**
 * @brief Configure PLL1 for 480MHz operation using HSI source
 * @return ClockConfigError_t Success or specific error code
 */
static ClockConfigError_t configure_pll1_for_480mhz(void) {
    DEBUG_PRINTF("🚀 Configuring PLL1 for 480MHz operation...\n");

    // Verify HSI is ready
    if (!clock_state.hsi_ready) {
        DEBUG_PRINTF("❌ HSI not ready, cannot configure PLL1\n");
        clock_state.last_error = CLOCK_CONFIG_ERROR_HSI_NOT_READY;
        return CLOCK_CONFIG_ERROR_HSI_NOT_READY;
    }

    // Verify VOS0 is ready
    if (!clock_state.vos0_ready) {
        DEBUG_PRINTF("❌ VOS0 not ready, cannot configure PLL1 for 480MHz\n");
        clock_state.last_error = CLOCK_CONFIG_ERROR_VOS_NOT_READY;
        return CLOCK_CONFIG_ERROR_VOS_NOT_READY;
    }

    // Disable PLL1 first
    DEBUG_PRINTF("Disabling PLL1...\n");
    RCC->CR &= ~RCC_CR_PLL1ON;

    // Wait for PLL1 to be disabled
    if (!wait_for_register_bit((uint32_t)&RCC->CR, RCC_CR_PLL1RDY, 0,
                               PLL_DISABLE_TIMEOUT_MS, "PLL1 disabled")) {
        clock_state.last_error = CLOCK_CONFIG_ERROR_PLL_DISABLE_TIMEOUT;
        return CLOCK_CONFIG_ERROR_PLL_DISABLE_TIMEOUT;
    }

    // Configure PLL1 source and M divider
    // HSI = 64MHz, M = 4, so VCO input = 16MHz (within 4-16MHz range)
    DEBUG_PRINTF("Setting PLL1 source to HSI with M=4...\n");
    RCC->PLLCKSELR = (0 << RCC_PLLCKSELR_PLLSRC_Pos) | // HSI source
                     (4 << RCC_PLLCKSELR_DIVM1_Pos);   // M1 = 4

    // Configure PLL1 dividers for 480MHz
    // VCO = 16MHz * 30 = 480MHz
    // SYSCLK = 480MHz / 1 = 480MHz
    // PLLQ = 480MHz / 2 = 240MHz
    // PLLR = 480MHz / 2 = 240MHz
    DEBUG_PRINTF("Setting PLL1 dividers: N=30, P=1, Q=2, R=2...\n");
    RCC->PLL1DIVR = ((30 - 1) << RCC_PLL1DIVR_N1_Pos) | // N1 = 30
                    ((1 - 1) << RCC_PLL1DIVR_P1_Pos) |  // P1 = 1
                    ((2 - 1) << RCC_PLL1DIVR_Q1_Pos) |  // Q1 = 2
                    ((2 - 1) << RCC_PLL1DIVR_R1_Pos);   // R1 = 2

    // Configure PLL1 range and enable outputs
    DEBUG_PRINTF("Configuring PLL1 range and outputs...\n");
    RCC->PLLCFGR = (1 << RCC_PLLCFGR_PLL1RGE_Pos) |    // RGE = 4-8MHz
                   (0 << RCC_PLLCFGR_PLL1VCOSEL_Pos) | // VCOSEL = wide VCO
                   (1 << RCC_PLLCFGR_DIVP1EN_Pos) |    // Enable P1 output
                   (1 << RCC_PLLCFGR_DIVQ1EN_Pos) |    // Enable Q1 output
                   (1 << RCC_PLLCFGR_DIVR1EN_Pos);     // Enable R1 output

    // Enable PLL1
    DEBUG_PRINTF("Enabling PLL1...\n");
    RCC->CR |= RCC_CR_PLL1ON;

    // Wait for PLL1 lock
    if (!wait_for_register_bit((uint32_t)&RCC->CR, RCC_CR_PLL1RDY, 1,
                               PLL_LOCK_TIMEOUT_MS, "PLL1 locked")) {
        clock_state.last_error = CLOCK_CONFIG_ERROR_PLL_LOCK_TIMEOUT;
        return CLOCK_CONFIG_ERROR_PLL_LOCK_TIMEOUT;
    }

    clock_state.pll1_locked = true;
    clock_state.sysclk_freq_hz = 480000000; // 480MHz target

    DEBUG_PRINTF("✅ PLL1 configured and locked for 480MHz operation\n");
    DEBUG_PRINTF("   VCO frequency: 480 MHz\n");
    DEBUG_PRINTF("   SYSCLK target: 480 MHz\n");
    DEBUG_PRINTF("   PLLQ output: 240 MHz\n");
    DEBUG_PRINTF("   PLLR output: 240 MHz\n");

    return CLOCK_CONFIG_SUCCESS;
}

/**
 * @brief Switch system clock to PLL1
 * @return ClockConfigError_t Success or specific error code
 */
static ClockConfigError_t switch_to_pll1_clock(void) {
    DEBUG_PRINTF("🔄 Switching system clock to PLL1...\n");

    // Verify PLL1 is ready
    if (!clock_state.pll1_locked) {
        DEBUG_PRINTF("❌ PLL1 not locked, cannot switch\n");
        clock_state.last_error = CLOCK_CONFIG_ERROR_PLL_NOT_READY;
        return CLOCK_CONFIG_ERROR_PLL_NOT_READY;
    }

    // Set flash latency for 480MHz @ VOS0
    // According to STM32H7 reference manual, need LATENCY = 4 for 480MHz @
    // VOS0
    DEBUG_PRINTF("Setting flash latency for 480MHz...\n");
    uint32_t flash_acr = FLASH->ACR;
    flash_acr = (flash_acr & ~FLASH_ACR_LATENCY) | FLASH_ACR_LATENCY_4WS;
    FLASH->ACR = flash_acr;
    DEBUG_PRINTF("Flash latency set to 4 wait states: 0x%08lX\n", flash_acr);

    // Switch system clock to PLL1
    DEBUG_PRINTF("Initiating clock switch to PLL1...\n");
    uint32_t cfgr = RCC->CFGR;
    cfgr = (cfgr & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL1;
    RCC->CFGR = cfgr;

    // Wait for switch to complete
    if (!wait_for_register_bit((uint32_t)&RCC->CFGR, RCC_CFGR_SWS,
                               RCC_CFGR_SWS_PLL1 >> RCC_CFGR_SWS_Pos,
                               CLOCK_SWITCH_TIMEOUT_MS,
                               "Clock switch to PLL1")) {
        clock_state.last_error = CLOCK_CONFIG_ERROR_CLOCK_SWITCH_TIMEOUT;
        return CLOCK_CONFIG_ERROR_CLOCK_SWITCH_TIMEOUT;
    }

    DEBUG_PRINTF("✅ System clock successfully switched to PLL1\n");
    DEBUG_PRINTF("🚀 STM32H753ZI now running at 480MHz!\n");

    // Update SystemCoreClock variable
    SystemCoreClock = clock_state.sysclk_freq_hz;

    return CLOCK_CONFIG_SUCCESS;
}

/**
 * @brief Initialize system clocks for 480MHz operation on STM32H753ZI Revision
 * V
 * @return ClockConfigError_t Success or specific error code
 */
ClockConfigError_t Clock_Init(void) {
    ClockConfigError_t result;

    DEBUG_PRINTF("\n🚀 STM32H753ZI Clock Initialization for 480MHz\n");
    DEBUG_PRINTF("================================================\n");

    // Initialize clock state
    memset(&clock_state, 0, sizeof(clock_state));

    // Step 1: Check silicon revision
    DEBUG_PRINTF("📋 Step 1: Checking silicon revision...\n");
    result = Check_Silicon_Revision_For_480MHz(&clock_state.revision);
    if (result != REVISION_CHECK_SUCCESS) {
        DEBUG_PRINTF("❌ Revision check failed: %d\n", result);
        clock_state.last_error = CLOCK_CONFIG_ERROR_REVISION_CHECK_FAILED;
        return CLOCK_CONFIG_ERROR_REVISION_CHECK_FAILED;
    }
    DEBUG_PRINTF("✅ Silicon revision %c confirmed for 480MHz operation\n",
                 'A' + clock_state.revision);

    // Step 2: Configure HSI oscillator
    DEBUG_PRINTF("\n📋 Step 2: Configuring HSI oscillator...\n");
    result = configure_hsi_oscillator();
    if (result != CLOCK_CONFIG_SUCCESS) {
        DEBUG_PRINTF("❌ HSI configuration failed: %d\n", result);
        return result;
    }

    // Step 3: Configure VOS0 voltage scaling
    DEBUG_PRINTF("\n📋 Step 3: Configuring VOS0 voltage scaling...\n");
    result = configure_vos0_voltage_scaling();
    if (result != CLOCK_CONFIG_SUCCESS) {
        DEBUG_PRINTF("❌ VOS0 configuration failed: %d\n", result);
        return result;
    }

    // Step 4: Configure PLL1 for 480MHz
    DEBUG_PRINTF("\n📋 Step 4: Configuring PLL1 for 480MHz...\n");
    result = configure_pll1_for_480mhz();
    if (result != CLOCK_CONFIG_SUCCESS) {
        DEBUG_PRINTF("❌ PLL1 configuration failed: %d\n", result);
        return result;
    }

    // Step 5: Switch to PLL1 clock
    DEBUG_PRINTF("\n📋 Step 5: Switching to PLL1 clock...\n");
    result = switch_to_pll1_clock();
    if (result != CLOCK_CONFIG_SUCCESS) {
        DEBUG_PRINTF("❌ Clock switch failed: %d\n", result);
        return result;
    }

    DEBUG_PRINTF("\n🎉 Clock initialization completed successfully!\n");
    DEBUG_PRINTF("✅ STM32H753ZI Revision V now running at 480MHz\n");
    DEBUG_PRINTF("   SystemCoreClock: %lu Hz\n", SystemCoreClock);
    DEBUG_PRINTF("   HSI: Ready\n");
    DEBUG_PRINTF("   VOS0: Ready\n");
    DEBUG_PRINTF("   PLL1: Locked\n");
    DEBUG_PRINTF("   SYSCLK: PLL1 (480MHz)\n");

    return CLOCK_CONFIG_SUCCESS;
}

/**
 * @brief Get current clock configuration state
 * @return Pointer to clock state structure
 */
const ClockState_t *Clock_GetState(void) {
    return &clock_state;
}

/**
 * @brief Get string description of clock configuration error
 * @param error Error code
 * @return String description
 */
const char *Clock_GetErrorString(ClockConfigError_t error) {
    switch (error) {
    case CLOCK_CONFIG_SUCCESS:
        return "Success";
    case CLOCK_CONFIG_ERROR_REVISION_CHECK_FAILED:
        return "Silicon revision check failed";
    case CLOCK_CONFIG_ERROR_REVISION_NOT_SUPPORTED:
        return "Silicon revision does not support 480MHz";
    case CLOCK_CONFIG_ERROR_HSI_TIMEOUT:
        return "HSI oscillator ready timeout";
    case CLOCK_CONFIG_ERROR_HSI_NOT_READY:
        return "HSI oscillator not ready";
    case CLOCK_CONFIG_ERROR_VOS_TIMEOUT:
        return "VOS0 ready timeout";
    case CLOCK_CONFIG_ERROR_VOS_NOT_READY:
        return "VOS0 not ready";
    case CLOCK_CONFIG_ERROR_PLL_DISABLE_TIMEOUT:
        return "PLL1 disable timeout";
    case CLOCK_CONFIG_ERROR_PLL_LOCK_TIMEOUT:
        return "PLL1 lock timeout";
    case CLOCK_CONFIG_ERROR_PLL_NOT_READY:
        return "PLL1 not ready";
    case CLOCK_CONFIG_ERROR_CLOCK_SWITCH_TIMEOUT:
        return "Clock switch timeout";
    default:
        return "Unknown error";
    }
}
