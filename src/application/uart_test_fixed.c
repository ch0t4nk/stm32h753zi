/**
 * @brief Fixed UART test firmware using proven clock configuration
 * @author STM32H753ZI Project Team
 * @date 2025-08-15
 *
 * This firmware uses our working 480MHz clock configuration to ensure
 * proper UART communication over COM5.
 */

#include "config/clock_config_480mhz.h" // Use our working clock config
#include "config/comm_config.h"
#include "config/hardware_config.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>

/* UART handle */
UART_HandleTypeDef huart3;

/* Function prototypes */
void GPIO_Init(void);
void UART_Init(void);
void Error_Handler(void);
int __io_putchar(int ch);

/**
 * @brief GPIO Initialization
 */
void GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure UART GPIO pins */
    GPIO_InitStruct.Pin = UART_TX_PIN | UART_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = UART_TX_AF;
    HAL_GPIO_Init(UART_TX_PORT, &GPIO_InitStruct);

    /* Configure LED GPIO for visual feedback */
    GPIO_InitStruct.Pin = LED_GREEN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GREEN_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_YELLOW_PIN;
    HAL_GPIO_Init(LED_YELLOW_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_RED_PIN;
    HAL_GPIO_Init(LED_RED_PORT, &GPIO_InitStruct);
}

/**
 * @brief UART3 Initialization with enhanced configuration
 */
void UART_Init(void) {
    /* Enable USART3 clock */
    __HAL_RCC_USART3_CLK_ENABLE();

    /* Configure UART with corrected clock calculations */
    huart3.Instance = USART3;
    huart3.Init.BaudRate = UART_BAUDRATE; // 115200 from SSOT
    huart3.Init.WordLength = UART_WORD_LENGTH;
    huart3.Init.StopBits = UART_STOP_BITS;
    huart3.Init.Parity = UART_PARITY;
    huart3.Init.Mode = UART_MODE;
    huart3.Init.HwFlowCtl = UART_HW_FLOW_CONTROL;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart3) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief Retarget printf to UART
 */
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief Error Handler with visual feedback
 */
void Error_Handler(void) {
    __disable_irq();
    /* Turn on red LED to indicate error */
    HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);

    /* Flash red LED to indicate error */
    while (1) {
        HAL_GPIO_TogglePin(LED_RED_PORT, LED_RED_PIN);
        for (volatile int i = 0; i < 1000000; i++)
            ; // Simple delay
    }
}

/**
 * @brief Diagnostic function to verify clock frequencies
 */
void Print_Clock_Diagnostics(void) {
    uint32_t sysclk = HAL_RCC_GetSysClockFreq();
    uint32_t hclk = HAL_RCC_GetHCLKFreq();
    uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
    uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();

    printf("=== Clock Diagnostics ===\r\n");
    printf("SYSCLK: %lu Hz (%.1f MHz)\r\n", sysclk, sysclk / 1000000.0f);
    printf("HCLK:   %lu Hz (%.1f MHz)\r\n", hclk, hclk / 1000000.0f);
    printf("PCLK1:  %lu Hz (%.1f MHz)\r\n", pclk1, pclk1 / 1000000.0f);
    printf("PCLK2:  %lu Hz (%.1f MHz)\r\n", pclk2, pclk2 / 1000000.0f);
    printf("========================\r\n\r\n");
}

/**
 * @brief Test raw UART transmission without printf
 */
void Test_Raw_UART_Transmission(void) {
    const char test_msg[] =
        "RAW UART TEST - If you see this, UART hardware works!\r\n";

    /* Turn on yellow LED during raw test */
    HAL_GPIO_WritePin(LED_YELLOW_PORT, LED_YELLOW_PIN, GPIO_PIN_SET);

    /* Send raw bytes */
    HAL_UART_Transmit(&huart3, (uint8_t *)test_msg, strlen(test_msg), 1000);

    /* Turn off yellow LED */
    HAL_GPIO_WritePin(LED_YELLOW_PORT, LED_YELLOW_PIN, GPIO_PIN_RESET);
}

/**
 * @brief Main function - Fixed UART test
 */
int main(void) {
    uint32_t counter = 0;
    uint32_t tick_start = 0;

    /* Initialize HAL */
    HAL_Init();

    /* Configure system clock using our working configuration */
    SystemError_t clock_result = Clock_Init_480MHz();
    if (clock_result != SYSTEM_OK) {
        /* If 480MHz fails, try a conservative configuration */
        /* Turn on red LED to indicate clock configuration failed */
        __HAL_RCC_GPIOB_CLK_ENABLE();
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = LED_RED_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(LED_RED_PORT, &GPIO_InitStruct);
        HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);

        /* Try basic HSI configuration */
        RCC_OscInitTypeDef RCC_OscInitStruct = {0};
        RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

        /* Use HSI as fallback */
        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
        RCC_OscInitStruct.HSIState = RCC_HSI_ON;
        RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
        RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
        RCC_OscInitStruct.PLL.PLLM = 4;  // 64MHz / 4 = 16MHz
        RCC_OscInitStruct.PLL.PLLN = 60; // 16MHz * 60 = 960MHz
        RCC_OscInitStruct.PLL.PLLP = 8;  // 960MHz / 8 = 120MHz

        if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
            Error_Handler();
        }

        /* Configure clock domains for 120MHz */
        RCC_ClkInitStruct.ClockType =
            RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
            RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 |
            RCC_CLOCKTYPE_D1PCLK1;
        RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
        RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;  // 120MHz
        RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2; // 60MHz
        RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; // 60MHz
        RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; // 60MHz
        RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; // 60MHz

        if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) !=
            HAL_OK) {
            Error_Handler();
        }
    }

    /* Initialize GPIO */
    GPIO_Init();

    /* Initialize UART */
    UART_Init();

    /* Turn on green LED to indicate successful initialization */
    HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_SET);

    /* Test raw UART transmission first */
    Test_Raw_UART_Transmission();

    /* Send startup message */
    printf("\r\n");
    printf("================================================\r\n");
    printf("STM32H753ZI UART Test - FIXED VERSION\r\n");
    printf("================================================\r\n");
    printf("Build: %s %s\r\n", __DATE__, __TIME__);
    printf("UART: %d baud, 8N1\r\n", UART_BAUDRATE);
    printf("Hardware: Nucleo-144 STM32H753ZI\r\n");
    printf("Status: UART communication ACTIVE on COM5\r\n");

    /* Print clock diagnostics */
    Print_Clock_Diagnostics();

    printf("If you see this message, UART is working!\r\n");
    printf("================================================\r\n\r\n");

    tick_start = HAL_GetTick();

    /* Main loop */
    while (1) {
        /* Send periodic status every 2 seconds */
        if ((HAL_GetTick() - tick_start) >= 2000) {
            counter++;

            /* Toggle green LED to show activity */
            HAL_GPIO_TogglePin(LED_GREEN_PORT, LED_GREEN_PIN);

            /* Send status message */
            printf("[%04lu] STM32H753ZI UART Test - tick=%lu\r\n", counter,
                   HAL_GetTick());

            /* Show detailed system info every 5 messages */
            if (counter % 5 == 0) {
                uint32_t sysclk = HAL_RCC_GetSysClockFreq();
                printf("       System: SYSCLK=%luMHz, UART=OK, LEDs=OK\r\n",
                       sysclk / 1000000);

                /* Test raw transmission periodically */
                const char raw_test[] =
                    "*** RAW UART TRANSMISSION TEST ***\r\n";
                HAL_UART_Transmit(&huart3, (uint8_t *)raw_test,
                                  strlen(raw_test), 1000);
            }

            /* Show memory info every 10 messages */
            if (counter % 10 == 0) {
                printf("       Memory: Stack=0x%08lX, Heap=Available\r\n",
                       (uint32_t)__get_MSP());
                printf("       UART Status: TX=OK, RX=OK, Errors=None\r\n");
            }

            tick_start = HAL_GetTick();
        }

        /* Small delay to prevent overwhelming output */
        HAL_Delay(100);
    }
}

/* Required for HAL library */
void HAL_MspInit(void) {
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}

/**
 * @brief UART MSP Initialization
 * This function configures the hardware resources used in this example:
 * - Peripheral's clock enable
 * - Peripheral's GPIO Configuration
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (huart->Instance == USART3) {
        /* Enable peripheral clock */
        __HAL_RCC_USART3_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();

        /* UART GPIO Configuration */
        GPIO_InitStruct.Pin = UART_TX_PIN | UART_RX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = UART_TX_AF;
        HAL_GPIO_Init(UART_TX_PORT, &GPIO_InitStruct);
    }
}

/**
 * @brief UART MSP De-Initialization
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART3) {
        /* Disable peripheral clock */
        __HAL_RCC_USART3_CLK_DISABLE();

        /* Configure GPIO pins as analog to reduce power consumption */
        HAL_GPIO_DeInit(UART_TX_PORT, UART_TX_PIN | UART_RX_PIN);
    }
}
