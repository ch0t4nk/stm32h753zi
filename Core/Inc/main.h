/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion
 * -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes
 * ------------------------------------------------------------------*/
/* USER CODE BEGIN HSE_VALUE */
/* Define HSE_VALUE before including HAL to override default 25MHz */
#define HSE_VALUE                                                             \
    8000000UL /*!< Value of the External oscillator in Hz - STM32H753ZI       \
                 Nucleo-144 uses 8MHz */
/* USER CODE END HSE_VALUE */

/* Define essential oscillator values before HAL include */
#ifndef CSI_VALUE
#define CSI_VALUE (4000000UL) /*!< Value of the Internal oscillator in Hz*/
#endif

#ifndef HSI_VALUE
#define HSI_VALUE (64000000UL) /*!< Value of the Internal oscillator in Hz*/
#endif

#ifndef TICK_INT_PRIORITY
#define TICK_INT_PRIORITY (15UL) /*!< Tick interrupt priority */
#endif

/* Essential assert_param definition */
#ifndef assert_param
#ifdef USE_FULL_ASSERT
#define assert_param(expr)                                                    \
    ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
void assert_failed(uint8_t *file, uint32_t line);
#else
#define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */
#endif /* assert_param */

#include "stm32h7xx_hal.h"

#include "stm32h7xx_nucleo.h"

/* Private includes
 * ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types
 * ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants
 * --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro
 * ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes
 * ---------------------------------------------*/
void Error_Handler(void);

void MX_FREERTOS_Init(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines
 * -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
