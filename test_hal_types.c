/*
 * Test file to check HAL type definitions
 */

#include "Core/Inc/stm32h7xx_hal_conf.h"

/* Test to verify HAL types are properly defined */
HAL_StatusTypeDef test_status = HAL_OK;
HAL_LockTypeDef test_lock = HAL_UNLOCKED;

/* Test function using HAL types */
HAL_StatusTypeDef test_function(void) {
    return HAL_OK;
}
