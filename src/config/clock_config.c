/**
 * @file clock_config.c
 * @brief Centralized Clock Management Implementation
 * @author STM32H753ZI Project Team
 * @date 2025-08-15
 *
 * @note This file implements the centralized clock management system
 * providing unified clock initialization, monitoring, and fallback handling.
 */

#include "clock_config.h"
#include "common/error_codes.h"
#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdio.h>

/* Forward declaration of Error_Handler (defined in main.c) */
extern void Error_Handler(void);

/* Private variables */
static ClockHealth_t current_clock_health = {0};
static bool clock_initialized = false;

/* ==========================================================================
 */
/* Private Function Declarations                                             */
/* ==========================================================================
 */

static HAL_StatusTypeDef Clock_ConfigureHSE(void);
static HAL_StatusTypeDef Clock_ConfigureHSI(void);
static HAL_StatusTypeDef Clock_ConfigureCSI(void);
static HAL_StatusTypeDef Clock_ConfigurePLL_HSE(void);
static HAL_StatusTypeDef Clock_ConfigurePLL_HSI(void);
static HAL_StatusTypeDef Clock_ConfigureSystemClocks(void);
static void Clock_UpdateHealthStatus(void);

/* ==========================================================================
 */
/* Public Function Implementations                                           */
/* ==========================================================================
 */

/**
 * @brief Initialize clock system with automatic source selection
 */
HAL_StatusTypeDef Clock_Init(void) {
    HAL_StatusTypeDef status = HAL_ERROR;

    CLOCK_DEBUG_PRINT("Initializing clock system...");

    // Configure power supply first
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
        // Wait for voltage scaling ready
    }

    // Try clock sources in priority order
    CLOCK_DEBUG_PRINT("Attempting HSE configuration...");
    status = Clock_ConfigureHSE();
    if (status == HAL_OK) {
        status = Clock_ConfigurePLL_HSE();
        if (status == HAL_OK) {
            current_clock_health.active_source = CLOCK_SOURCE_HSE;
            CLOCK_DEBUG_PRINT("HSE + PLL1 configuration successful");
        }
    }

    if (status != HAL_OK) {
        CLOCK_DEBUG_PRINT("HSE failed, attempting HSI configuration...");
        status = Clock_ConfigureHSI();
        if (status == HAL_OK) {
            status = Clock_ConfigurePLL_HSI();
            if (status == HAL_OK) {
                current_clock_health.active_source = CLOCK_SOURCE_HSI;
                CLOCK_DEBUG_PRINT("HSI + PLL1 configuration successful");
            }
        }
    }

    if (status != HAL_OK) {
        CLOCK_DEBUG_PRINT(
            "HSI failed, attempting CSI emergency configuration...");
        status = Clock_ConfigureCSI();
        if (status == HAL_OK) {
            current_clock_health.active_source = CLOCK_SOURCE_CSI;
            CLOCK_DEBUG_PRINT("CSI emergency configuration successful");
        }
    }

    if (status != HAL_OK) {
        CLOCK_DEBUG_PRINT("All clock sources failed!");
        Error_Handler();
        return HAL_ERROR;
    }

    // Configure system clocks (AHB, APB prescalers)
    status = Clock_ConfigureSystemClocks();
    if (status != HAL_OK) {
        CLOCK_DEBUG_PRINT("System clock configuration failed!");
        Error_Handler();
        return HAL_ERROR;
    }

    // CRITICAL: Update SystemCoreClock variable
    status = Clock_UpdateSystemClock();
    if (status != HAL_OK) {
        CLOCK_DEBUG_PRINT("SystemCoreClock update failed!");
        Error_Handler();
        return HAL_ERROR;
    }

    clock_initialized = true;
    Clock_UpdateHealthStatus();

    CLOCK_DEBUG_PRINT("Clock initialization complete");
    Clock_PrintStatus();

    return HAL_OK;
}

/**
 * @brief Update SystemCoreClock and all dependent timing systems
 */
HAL_StatusTypeDef Clock_UpdateSystemClock(void) {
    if (!clock_initialized) {
        return HAL_ERROR;
    }

    // Update HAL's SystemCoreClock variable
    SystemCoreClockUpdate();

    // Reconfigure SysTick with correct frequency
    uint32_t systick_reload = SYSTICK_RELOAD_VALUE(SystemCoreClock);

    if (HAL_SYSTICK_Config(systick_reload) != HAL_OK) {
        CLOCK_DEBUG_PRINT("SysTick configuration failed!");
        return HAL_ERROR;
    }

    // Update health status
    current_clock_health.systick_reload = systick_reload;
    current_clock_health.sysclk_frequency = SystemCoreClock;
    current_clock_health.last_update_tick = HAL_GetTick();

    CLOCK_DEBUG_PRINT("SystemCoreClock updated to %lu Hz", SystemCoreClock);
    CLOCK_DEBUG_PRINT("SysTick reload value: %lu", systick_reload);

    return HAL_OK;
}

/**
 * @brief Get current active clock source
 */
ClockSource_t Clock_GetActiveSource(void) {
    // Read RCC_CFGR register to check system clock source
    uint32_t rcc_cfgr = RCC->CFGR;
    uint32_t sws = (rcc_cfgr >> 3) & 0x7; // SWS bits [5:3]

    switch (sws) {
    case 0:
        return CLOCK_SOURCE_HSI;
    case 1:
        return CLOCK_SOURCE_CSI;
    case 2:
        return CLOCK_SOURCE_HSE;
    case 3:
        return current_clock_health.active_source; // PLL - use stored source
    default:
        return CLOCK_SOURCE_INVALID;
    }
}

/**
 * @brief Get actual system clock frequency
 */
uint32_t Clock_GetSystemFrequency(void) {
    return HAL_RCC_GetSysClockFreq();
}

/**
 * @brief Get actual AHB clock frequency
 */
uint32_t Clock_GetAHBFrequency(void) {
    return HAL_RCC_GetHCLKFreq();
}

/**
 * @brief Get actual APB1 clock frequency
 */
uint32_t Clock_GetAPB1Frequency(void) {
    return HAL_RCC_GetPCLK1Freq();
}

/**
 * @brief Get actual APB2 clock frequency
 */
uint32_t Clock_GetAPB2Frequency(void) {
    return HAL_RCC_GetPCLK2Freq();
}

/**
 * @brief Validate clock configuration against specifications
 */
HAL_StatusTypeDef Clock_ValidateConfiguration(void) {
    uint32_t sysclk = Clock_GetSystemFrequency();
    uint32_t hclk = Clock_GetAHBFrequency();
    uint32_t pclk1 = Clock_GetAPB1Frequency();
    uint32_t pclk2 = Clock_GetAPB2Frequency();

    // Validate frequencies are within specifications
    if (sysclk > 240000000UL) {
        CLOCK_DEBUG_PRINT("SYSCLK exceeds maximum (240 MHz): %lu Hz", sysclk);
        return HAL_ERROR;
    }

    if (hclk > sysclk) {
        CLOCK_DEBUG_PRINT("HCLK exceeds SYSCLK: %lu > %lu", hclk, sysclk);
        return HAL_ERROR;
    }

    if (pclk1 > 120000000UL || pclk2 > 120000000UL) {
        CLOCK_DEBUG_PRINT(
            "APB frequency exceeds maximum (120 MHz): PCLK1=%lu, PCLK2=%lu",
            pclk1, pclk2);
        return HAL_ERROR;
    }

    // Check PLL lock status
    if (current_clock_health.active_source != CLOCK_SOURCE_CSI) {
        if (!__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY)) {
            CLOCK_DEBUG_PRINT("PLL not locked!");
            return HAL_ERROR;
        }
    }

    return HAL_OK;
}

/**
 * @brief Print detailed clock status to debug output
 */
void Clock_PrintStatus(void) {
    Clock_UpdateHealthStatus();

    printf("\r\n=== Clock Status ===\r\n");
    printf("Active Source: ");
    switch (current_clock_health.active_source) {
    case CLOCK_SOURCE_HSE:
        printf("HSE (External Crystal)\r\n");
        break;
    case CLOCK_SOURCE_HSI:
        printf("HSI (Internal Oscillator)\r\n");
        break;
    case CLOCK_SOURCE_CSI:
        printf("CSI (Low Power Oscillator)\r\n");
        break;
    default:
        printf("Unknown\r\n");
        break;
    }

    printf("SYSCLK: %lu Hz (%.1f MHz)\r\n",
           current_clock_health.sysclk_frequency,
           current_clock_health.sysclk_frequency / 1000000.0f);
    printf("HCLK:   %lu Hz (%.1f MHz)\r\n",
           current_clock_health.hclk_frequency,
           current_clock_health.hclk_frequency / 1000000.0f);
    printf("PCLK1:  %lu Hz (%.1f MHz)\r\n",
           current_clock_health.pclk1_frequency,
           current_clock_health.pclk1_frequency / 1000000.0f);
    printf("PCLK2:  %lu Hz (%.1f MHz)\r\n",
           current_clock_health.pclk2_frequency,
           current_clock_health.pclk2_frequency / 1000000.0f);

    printf("SysTick Reload: %lu (%.3f ms period)\r\n",
           current_clock_health.systick_reload,
           (current_clock_health.systick_reload + 1) * 1000.0f /
               current_clock_health.sysclk_frequency);

    printf("PLL Locked: %s\r\n",
           current_clock_health.pll_locked ? "Yes" : "No");
    printf("HSE Ready:  %s\r\n",
           current_clock_health.hse_ready ? "Yes" : "No");
    printf("HSI Ready:  %s\r\n",
           current_clock_health.hsi_ready ? "Yes" : "No");
    printf("==================\r\n\r\n");
}

/**
 * @brief Get current clock health status
 */
HAL_StatusTypeDef Clock_GetHealth(ClockHealth_t *health) {
    if (!health || !clock_initialized) {
        return HAL_ERROR;
    }

    Clock_UpdateHealthStatus();
    *health = current_clock_health;

    return HAL_OK;
}

/**
 * @brief Monitor clock stability and detect issues
 */
HAL_StatusTypeDef Clock_MonitorStability(void) {
    if (!clock_initialized) {
        return HAL_ERROR;
    }

    // Check if clock source changed unexpectedly
    ClockSource_t current_source = Clock_GetActiveSource();
    if (current_source != current_clock_health.active_source) {
        CLOCK_DEBUG_PRINT("Clock source changed unexpectedly!");
        return HAL_ERROR;
    }

    // Validate current configuration
    return Clock_ValidateConfiguration();
}

/* ==========================================================================
 */
/* Private Function Implementations                                          */
/* ==========================================================================
 */

/**
 * @brief Configure HSE with timeout and stabilization
 */
static HAL_StatusTypeDef Clock_ConfigureHSE(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    // Add delay for ST-Link MCO stabilization
    HAL_Delay(10); // 10ms delay for ST-Link MCO to stabilize

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE; // Configure PLL separately

    HAL_StatusTypeDef status = HAL_RCC_OscConfig(&RCC_OscInitStruct);

    if (status == HAL_OK) {
        CLOCK_DEBUG_PRINT("HSE configured successfully");
    } else {
        CLOCK_DEBUG_PRINT("HSE configuration failed");
    }

    return status;
}

/**
 * @brief Configure HSI oscillator
 */
static HAL_StatusTypeDef Clock_ConfigureHSI(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE; // Configure PLL separately

    HAL_StatusTypeDef status = HAL_RCC_OscConfig(&RCC_OscInitStruct);

    if (status == HAL_OK) {
        CLOCK_DEBUG_PRINT("HSI configured successfully");
    } else {
        CLOCK_DEBUG_PRINT("HSI configuration failed");
    }

    return status;
}

/**
 * @brief Configure CSI oscillator (emergency fallback)
 */
static HAL_StatusTypeDef Clock_ConfigureCSI(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_CSI;
    RCC_OscInitStruct.CSIState = RCC_CSI_ON;
    RCC_OscInitStruct.CSICalibrationValue = RCC_CSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_NONE; // No PLL for CSI emergency mode

    HAL_StatusTypeDef status = HAL_RCC_OscConfig(&RCC_OscInitStruct);

    if (status == HAL_OK) {
        CLOCK_DEBUG_PRINT("CSI configured successfully (emergency mode)");
    } else {
        CLOCK_DEBUG_PRINT("CSI configuration failed");
    }

    return status;
}

/**
 * @brief Configure PLL1 with HSE source
 */
static HAL_StatusTypeDef Clock_ConfigurePLL_HSE(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_NONE; // Don't change oscillators
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = PLL1_HSE_M_DIVIDER;
    RCC_OscInitStruct.PLL.PLLN = PLL1_HSE_N_MULTIPLIER;
    RCC_OscInitStruct.PLL.PLLP = PLL1_HSE_P_DIVIDER;
    RCC_OscInitStruct.PLL.PLLQ = PLL1_HSE_Q_DIVIDER;
    RCC_OscInitStruct.PLL.PLLR = PLL1_HSE_R_DIVIDER;
    RCC_OscInitStruct.PLL.PLLRGE =
        RCC_PLL1VCIRANGE_2; // 4-8MHz VCO input range
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE; // Wide VCO (192-960MHz)
    RCC_OscInitStruct.PLL.PLLFRACN = 0;

    HAL_StatusTypeDef status = HAL_RCC_OscConfig(&RCC_OscInitStruct);

    if (status == HAL_OK) {
        CLOCK_DEBUG_PRINT("PLL1 (HSE) configured successfully");
    } else {
        CLOCK_DEBUG_PRINT("PLL1 (HSE) configuration failed");
    }

    return status;
}

/**
 * @brief Configure PLL1 with HSI source
 */
static HAL_StatusTypeDef Clock_ConfigurePLL_HSI(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_NONE; // Don't change oscillators
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = PLL1_HSI_M_DIVIDER;
    RCC_OscInitStruct.PLL.PLLN = PLL1_HSI_N_MULTIPLIER;
    RCC_OscInitStruct.PLL.PLLP = PLL1_HSI_P_DIVIDER;
    RCC_OscInitStruct.PLL.PLLQ = PLL1_HSI_Q_DIVIDER;
    RCC_OscInitStruct.PLL.PLLR = PLL1_HSI_R_DIVIDER;
    RCC_OscInitStruct.PLL.PLLRGE =
        RCC_PLL1VCIRANGE_3; // 8-16MHz VCO input range
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE; // Wide VCO (192-960MHz)
    RCC_OscInitStruct.PLL.PLLFRACN = 0;

    HAL_StatusTypeDef status = HAL_RCC_OscConfig(&RCC_OscInitStruct);

    if (status == HAL_OK) {
        CLOCK_DEBUG_PRINT("PLL1 (HSI) configured successfully");
    } else {
        CLOCK_DEBUG_PRINT("PLL1 (HSI) configuration failed");
    }

    return status;
}

/**
 * @brief Configure system clocks (AHB, APB prescalers)
 */
static HAL_StatusTypeDef Clock_ConfigureSystemClocks(void) {
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;

    if (current_clock_health.active_source == CLOCK_SOURCE_CSI) {
        // CSI emergency mode - use CSI directly
        RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_CSI;
        RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;  // 4MHz HCLK
        RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1; // 4MHz APB3
        RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1; // 4MHz APB1
        RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1; // 4MHz APB2
        RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1; // 4MHz APB4

        // Use FLASH_LATENCY_0 for low frequency
        return HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);
    } else {
        // Normal mode - use PLL
        RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
        RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;  // 240MHz HCLK
        RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2; // 120MHz APB3
        RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; // 120MHz APB1
        RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; // 120MHz APB2
        RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; // 120MHz APB4

        // Use appropriate flash latency for 240MHz
        return HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
    }
}

/**
 * @brief Update internal health status
 */
static void Clock_UpdateHealthStatus(void) {
    current_clock_health.sysclk_frequency = Clock_GetSystemFrequency();
    current_clock_health.hclk_frequency = Clock_GetAHBFrequency();
    current_clock_health.pclk1_frequency = Clock_GetAPB1Frequency();
    current_clock_health.pclk2_frequency = Clock_GetAPB2Frequency();
    current_clock_health.pll_locked = __HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY);
    current_clock_health.hse_ready = __HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY);
    current_clock_health.hsi_ready = __HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY);
    current_clock_health.last_update_tick = HAL_GetTick();
}
