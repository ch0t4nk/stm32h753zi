/**
 * @file mock_gpio.c
 * @brief Mock GPIO port instances for testing
 * @author STM32H753ZI Motor Control Project
 * @date 2024
 */

#include "mock_hal.h"

// Mock GPIO port instances
static GPIO_TypeDef mock_gpioa = {0};
static GPIO_TypeDef mock_gpiob = {0};
static GPIO_TypeDef mock_gpioc = {0};

GPIO_TypeDef* GPIOA = &mock_gpioa;
GPIO_TypeDef* GPIOB = &mock_gpiob;
GPIO_TypeDef* GPIOC = &mock_gpioc;
