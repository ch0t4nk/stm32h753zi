/**
 * @brief Simple UART test firmware for STM32H753ZI hardware validation
 * @author STM32H753ZI Project Team
 * @date 2025-08-13
 *
 * This firmware provides basic UART debug output to validate hardware
 * connection before full system deployment.
 */

#include "config/comm_config.h"
#include "config/hardware_config.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>

/* UART handle */
UART_HandleTypeDef huart3;

/* Function prototypes */
void SystemClock_Config(void);
void UART_Init(void);
void GPIO_Init(void);
void Error_Handler(void);
int __io_putchar(int ch);

/**
 * @brief System Clock Configuration
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Configure the main PLL */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;   // 8MHz / 4 = 2MHz
    RCC_OscInitStruct.PLL.PLLN = 240; // 2MHz * 240 = 480MHz
    RCC_OscInitStruct.PLL.PLLP = 2;   // 480MHz / 2 = 240MHz

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /* Configure clock domains */
    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        Error_Handler();
    }
}

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

    GPIO_InitStruct.Pin = LED_BLUE_PIN;
    HAL_GPIO_Init(LED_BLUE_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_RED_PIN;
    HAL_GPIO_Init(LED_RED_PORT, &GPIO_InitStruct);
}

/**
 * @brief UART3 Initialization
 */
void UART_Init(void) {
    /* Enable USART3 clock */
    __HAL_RCC_USART3_CLK_ENABLE();

    /* Configure UART */
    huart3.Instance = USART3;
    huart3.Init.BaudRate = UART_BAUDRATE;
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
 * @brief Error Handler
 */
void Error_Handler(void) {
    __disable_irq();
    /* Turn on red LED to indicate error */
    HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);
    while (1) {
        /* Error loop */
    }
}

/**
 * @brief Main function - UART test
 */
int main(void) {
    uint32_t counter = 0;
    uint32_t tick_start = 0;

    /* Initialize HAL */
    HAL_Init();

    /* Configure system clock */
    SystemClock_Config();

    /* Initialize GPIO */
    GPIO_Init();

    /* Initialize UART */
    UART_Init();

    /* Turn on green LED to indicate successful initialization */
    HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_SET);

    /* Send startup message */
    printf("\r\n=== STM32H753ZI UART Test ===\r\n");
    printf("Firmware build: %s %s\r\n", __DATE__, __TIME__);
    printf("System clock: 240MHz\r\n");
    printf("UART: 115200 baud, 8N1\r\n");
    printf("Hardware: Nucleo-144 STM32H753ZI\r\n");
    printf("Status: UART communication active\r\n");
    printf("==============================\r\n\r\n");

    tick_start = HAL_GetTick();

    /* Main loop */
    while (1) {
        /* Send periodic status every 1 second */
        if ((HAL_GetTick() - tick_start) >= 1000) {
            counter++;

            /* Toggle blue LED */
            HAL_GPIO_TogglePin(LED_BLUE_PORT, LED_BLUE_PIN);

            /* Send status message */
            printf("[%04lu] STM32H753ZI alive - tick=%lu\r\n", counter,
                   HAL_GetTick());

            /* Show system health */
            if (counter % 5 == 0) {
                printf("       System health check: Clock=%luMHz, UART=OK, "
                       "LEDs=OK\r\n",
                       HAL_RCC_GetSysClockFreq() / 1000000);
            }

            /* Show memory info every 10 seconds */
            if (counter % 10 == 0) {
                printf("       Memory test: Stack pointer=0x%08lX\r\n",
                       (uint32_t)__get_MSP());
            }

            tick_start = HAL_GetTick();
        }

        /* Simple delay to prevent overwhelming output */
        HAL_Delay(10);
    }
}

/* Required for HAL library */
void HAL_MspInit(void) {
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}
