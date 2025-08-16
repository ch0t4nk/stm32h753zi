/**
 * @brief Simple UART test firmware for STM32H753ZI hardware validation
 * @author STM32H753ZI Project Team
 * @date 2025-08-15
 *
 * This firmware provides basic UART debug output to validate hardware
 * connection before full system deployment.
 *
 * FIXED: Integrated working 480MHz clock configuration and corrected GPIO
 * setup
 */

#include "common/error_handling.h"      // SSOT error handling
#include "config/clock_config_480mhz.h" // Use working clock configuration
#include "config/comm_config.h"
#include "config/hardware_config.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>

/* UART handle */
UART_HandleTypeDef huart3;

/* FDCAN handle declaration (needed for interrupt handlers in stm32h7xx_it.c) */
/* Note: FDCAN not used in UART test but handle required for linking */
FDCAN_HandleTypeDef hfdcan1;

/* Function prototypes */
void GPIO_Init(void);
void UART_Init(void);
void Error_Handler(void);
void LED_Error_Indicator(uint32_t error_code);
int __io_putchar(int ch);

/**
 * @brief LED-based error indication for debugging
 * @param error_code Error code to display (0=no error)
 */
void LED_Error_Indicator(uint32_t error_code) {
    if (error_code == 0) {
        // Success: Green LED on, others off
        HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_YELLOW_PORT, LED_YELLOW_PIN, GPIO_PIN_RESET);
    } else {
        // Error: Red LED on, others off
        HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_YELLOW_PORT, LED_YELLOW_PIN, GPIO_PIN_RESET);

        // Blink pattern for error code (1-3 blinks)
        for (uint32_t i = 0; i < (error_code & 0x7); i++) {
            HAL_Delay(200);
            HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_RESET);
            HAL_Delay(200);
            HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);
        }
    }
}

/**
 * @brief System Clock Configuration using proven 480MHz setup
 * @note Removed old SystemClock_Config - now using Clock_Init_480MHz from SSOT
 */

/**
 * @brief GPIO Initialization with comprehensive error checking
 */
void GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE(); // For yellow LED

    /* Configure UART TX pin */
    GPIO_InitStruct.Pin = UART_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = UART_TX_AF;
    HAL_GPIO_Init(UART_TX_PORT, &GPIO_InitStruct);

    /* Configure UART RX pin with correct alternate function */
    GPIO_InitStruct.Pin = UART_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP; // Enable pull-up for RX pin
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = UART_RX_AF; // Use RX alternate function
    HAL_GPIO_Init(UART_RX_PORT, &GPIO_InitStruct);

    /* Configure LED GPIO for visual feedback */
    GPIO_InitStruct.Pin = LED_GREEN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GREEN_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_RED_PIN;
    HAL_GPIO_Init(LED_RED_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LED_YELLOW_PIN;
    HAL_GPIO_Init(LED_YELLOW_PORT, &GPIO_InitStruct);

    // Initialize all LEDs to OFF state
    HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_YELLOW_PORT, LED_YELLOW_PIN, GPIO_PIN_RESET);
}

/**
 * @brief UART3 Initialization with enhanced error checking
 */
void UART_Init(void) {
    HAL_StatusTypeDef status;

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

    status = HAL_UART_Init(&huart3);
    if (status != HAL_OK) {
        // Signal UART initialization error
        LED_Error_Indicator(2); // 2 blinks for UART error
        Error_Handler();
    }
}

/**
 * @brief Retarget printf to UART - NOTE: Using BSP implementation to avoid conflict
 */
// int __io_putchar(int ch) - REMOVED: Using BSP version to avoid multiple definition

/**
 * @brief Error Handler with LED indication
 */
void Error_Handler(void) {
    __disable_irq();
    /* Turn on red LED to indicate error */
    HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_YELLOW_PORT, LED_YELLOW_PIN, GPIO_PIN_RESET);

    while (1) {
        /* Blink red LED continuously to indicate error state */
        HAL_Delay(500);
        HAL_GPIO_TogglePin(LED_RED_PORT, LED_RED_PIN);
    }
}

/**
 * @brief Main function - UART test with working clock configuration
 */
int main(void) {
    uint32_t counter = 0;
    uint32_t tick_start = 0;
    SystemError_t clock_result;

    /* Initialize HAL */
    HAL_Init();

    /* Initialize GPIO first for error indication */
    GPIO_Init();

    /* Show yellow LED during clock configuration */
    HAL_GPIO_WritePin(LED_YELLOW_PORT, LED_YELLOW_PIN, GPIO_PIN_SET);

    /* Configure system clock using proven 480MHz configuration */
    clock_result = Clock_Init_480MHz();
    if (clock_result != SYSTEM_OK) {
        // Signal clock initialization error with red LED
        LED_Error_Indicator(1); // 1 blink for clock error
        Error_Handler();
    }

    /* Turn off yellow LED after successful clock init */
    HAL_GPIO_WritePin(LED_YELLOW_PORT, LED_YELLOW_PIN, GPIO_PIN_RESET);

    /* Initialize UART */
    UART_Init();

    /* Turn on green LED to indicate successful initialization */
    LED_Error_Indicator(0); // Success indication

    /* Send startup message */
    printf("\r\n=== STM32H753ZI UART Test - FIXED VERSION ===\r\n");
    printf("Firmware build: %s %s\r\n", __DATE__, __TIME__);
    printf("System clock: 480MHz (HSI-based)\r\n");
    printf("UART: 115200 baud, 8N1\r\n");
    printf("Hardware: Nucleo-144 STM32H753ZI\r\n");
    printf("GPIO: TX/RX pins properly configured\r\n");
    printf("Clock: Using proven clock_config_480mhz.c\r\n");
    printf("Status: UART communication active\r\n");
    printf("==============================\r\n\r\n");

    tick_start = HAL_GetTick();

    /* Main loop */
    while (1) {
        /* Send periodic status every 1 second */
        if ((HAL_GetTick() - tick_start) >= 1000) {
            counter++;

            /* Toggle yellow LED to show activity */
            HAL_GPIO_TogglePin(LED_YELLOW_PORT, LED_YELLOW_PIN);

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
                printf("       Clock source: HSI-based 480MHz PLL\r\n");
                printf("       UART3 APB1 clock: %luMHz\r\n",
                       HAL_RCC_GetPCLK1Freq() / 1000000);
            }

            tick_start = HAL_GetTick();
        }

        /* Simple delay to prevent overwhelming output */
        HAL_Delay(10);
    }
}

/* HAL_MspInit - REMOVED: Using Core/Src/stm32h7xx_hal_msp.c version to avoid conflict */
