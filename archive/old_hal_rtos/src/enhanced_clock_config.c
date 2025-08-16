/* STM32H753ZI Clock Configuration Fix
 * Handles ST-Link MCO timing and HSE startup issues
 * Insert this code before your existing SystemClock_Config() call
 */

#include "stm32h7xx_hal.h"

/* Forward declaration of Error_Handler (defined in main.c) */
extern void Error_Handler(void);

/**
 * @brief Enhanced HSE startup with ST-Link MCO handling
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef Enhanced_HSE_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    HAL_StatusTypeDef status;
    uint32_t retry_count = 0;
    const uint32_t MAX_RETRIES = 10;

    // Configure power supply first
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
        // Wait for voltage scaling ready
    }

    // Add delay for ST-Link MCO stabilization
    HAL_Delay(10); // 10ms delay for ST-Link MCO to stabilize

    // Try HSE configuration with retries
    do {
        // Configure HSE with timeout handling
        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
        RCC_OscInitStruct.HSEState = RCC_HSE_ON;
        RCC_OscInitStruct.PLL.PLLState =
            RCC_PLL_NONE; // Configure PLL separately

        status = HAL_RCC_OscConfig(&RCC_OscInitStruct);

        if (status == HAL_OK) {
            // HSE started successfully
            break;
        }

        // HSE failed to start - try recovery
        retry_count++;

        if (retry_count < MAX_RETRIES) {
            // Reset RCC and try again
            __HAL_RCC_HSE_CONFIG(RCC_HSE_OFF);
            HAL_Delay(5); // Short delay
            // Loop will retry HSE_ON
        }

    } while (retry_count < MAX_RETRIES);

    if (status != HAL_OK) {
        // HSE failed completely - fall back to HSI
        return HAL_ERROR;
    }

    // Now configure PLL with HSE as source (8MHz crystal)
    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_NONE; // Don't reconfigure HSE
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM =
        2; // 8MHz/2 = 4MHz VCO input (optimal for 2-16MHz range)
    RCC_OscInitStruct.PLL.PLLN = 120; // 4MHz*120 = 480MHz VCO
    RCC_OscInitStruct.PLL.PLLP = 2;   // 480MHz/2 = 240MHz SYSCLK (safe target)
    RCC_OscInitStruct.PLL.PLLQ = 4;   // 480MHz/4 = 120MHz
    RCC_OscInitStruct.PLL.PLLR = 2;   // 480MHz/2 = 240MHz
    RCC_OscInitStruct.PLL.PLLRGE =
        RCC_PLL1VCIRANGE_2; // 2-4MHz VCO input range
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE; // Wide VCO (192-960MHz)
    RCC_OscInitStruct.PLL.PLLFRACN = 0;

    status = HAL_RCC_OscConfig(&RCC_OscInitStruct);

    return status;
}

/**
 * @brief Fallback clock configuration using HSI
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef Fallback_HSI_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    // Configure HSI with PLL for better performance than 64MHz (HSI = 64MHz)
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 4;  // 64MHz/4 = 16MHz VCO input
    RCC_OscInitStruct.PLL.PLLN = 30; // 16MHz*30 = 480MHz VCO
    RCC_OscInitStruct.PLL.PLLP =
        2; // 480MHz/2 = 240MHz SYSCLK (target same as HSE)
    RCC_OscInitStruct.PLL.PLLQ = 4; // 480MHz/4 = 120MHz
    RCC_OscInitStruct.PLL.PLLR = 2; // 480MHz/2 = 240MHz
    RCC_OscInitStruct.PLL.PLLRGE =
        RCC_PLL1VCIRANGE_3; // 8-16MHz VCO input range
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE; // Wide VCO (192-960MHz)
    RCC_OscInitStruct.PLL.PLLFRACN = 0;

    return HAL_RCC_OscConfig(&RCC_OscInitStruct);
}

/**
 * @brief Enhanced SystemClock_Config with robust HSE handling
 * @retval None
 */
void Enhanced_SystemClock_Config(void) {
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    HAL_StatusTypeDef status;

    // Try enhanced HSE configuration first
    status = Enhanced_HSE_Config();

    if (status != HAL_OK) {
        // HSE failed - try HSI fallback with PLL for better performance
        status = Fallback_HSI_Config();

        if (status != HAL_OK) {
            // Complete failure
            Error_Handler();
        }
    }

    // Configure system clocks
    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;  // 240MHz HCLK
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2; // 120MHz APB3
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; // 120MHz APB1
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; // 120MHz APB2
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; // 120MHz APB4

    // Configure with appropriate flash latency for 240MHz
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief Check which clock source is actually running
 * @retval uint32_t Clock source identifier
 */
uint32_t Check_Active_Clock_Source(void) {
    // Read RCC_CFGR register to check system clock source
    uint32_t rcc_cfgr = RCC->CFGR;
    uint32_t sws = (rcc_cfgr >> 3) & 0x7; // SWS bits [5:3]

    return sws; // 0=HSI, 1=CSI, 2=HSE, 3=PLL1
}

/**
 * @brief Get actual system clock frequency
 * @retval uint32_t Frequency in Hz
 */
uint32_t Get_System_Clock_Frequency(void) {
    return HAL_RCC_GetSysClockFreq();
}
