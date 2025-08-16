/**
 * @file uart_enhanced.c
 * @brief Enhanced UART implementation with comprehensive error handling
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * @note Implementation addresses STM32H753ZI ERRATA issues:
 * - 2.20.4: DMA stream locked when transferring data to/from USART
 * - 2.20.5: Received data may be corrupted upon clearing the ABREN bit
 * - 2.20.6: Noise error flag set while ONEBIT is set
 */

#include "uart_enhanced.h"
#include "config/comm_config.h"
#include <stdio.h>
#include <string.h>

/* ==========================================================================
 * Private Variables
 * ==========================================================================
 */
static UART_Enhanced_t *g_printf_uart =
    NULL; // Global UART for printf redirection

/* ==========================================================================
 * Private Function Prototypes
 * ==========================================================================
 */
static void UART_UpdateStatistics(UART_Enhanced_t *uart_enhanced, bool is_tx,
                                  uint16_t bytes, bool error_occurred);
static SystemError_t UART_CheckHALErrors(UART_Enhanced_t *uart_enhanced);

/* ==========================================================================
 * Public Function Implementations
 * ==========================================================================
 */

/**
 * @brief Initialize enhanced UART with comprehensive error handling
 */
SystemError_t UART_Enhanced_Init(UART_Enhanced_t *uart_enhanced,
                                 UART_HandleTypeDef *hal_handle) {
    if (uart_enhanced == NULL || hal_handle == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // Initialize enhanced UART structure
    memset(uart_enhanced, 0, sizeof(UART_Enhanced_t));
    uart_enhanced->hal_handle = hal_handle;

    // Verify HAL handle is initialized
    if (hal_handle->Instance == NULL) {
        return ERROR_UART_NOT_INITIALIZED;
    }

    // Check for DMA usage (should be disabled per ERRATA 2.20.4)
    uart_enhanced->dma_enabled =
        (hal_handle->hdmatx != NULL || hal_handle->hdmarx != NULL);
    if (uart_enhanced->dma_enabled) {
        // Warning: DMA detected - this may cause ERRATA 2.20.4 issues
        uart_enhanced->error_flags |= UART_ERROR_DMA_LOCK;
    }

    // Get actual clock frequency for this UART instance
    uart_enhanced->clock_frequency =
        UART_GetClockFrequency(hal_handle->Instance);

    // Verify baud rate accuracy
    SystemError_t baud_result = UART_Enhanced_VerifyBaudRate(uart_enhanced);
    if (baud_result != SYSTEM_OK) {
        uart_enhanced->error_flags |=
            UART_ERROR_TIMEOUT; // Use as general config error
    }

    // Perform safe configuration check
    SystemError_t config_result = UART_SafeConfiguration(hal_handle);
    if (config_result != SYSTEM_OK) {
        return config_result;
    }

    uart_enhanced->initialized = true;
    return SYSTEM_OK;
}

/**
 * @brief Transmit data with enhanced error detection
 */
SystemError_t UART_Enhanced_Transmit(UART_Enhanced_t *uart_enhanced,
                                     const uint8_t *data, uint16_t length,
                                     uint32_t timeout_ms) {
    if (uart_enhanced == NULL || data == NULL || length == 0) {
        return ERROR_INVALID_PARAMETER;
    }

    if (!uart_enhanced->initialized) {
        return ERROR_UART_NOT_INITIALIZED;
    }

    // Clear any existing errors
    __HAL_UART_CLEAR_FLAG(uart_enhanced->hal_handle, UART_CLEAR_PEF);
    __HAL_UART_CLEAR_FLAG(uart_enhanced->hal_handle, UART_CLEAR_FEF);
    __HAL_UART_CLEAR_FLAG(uart_enhanced->hal_handle, UART_CLEAR_NEF);
    __HAL_UART_CLEAR_FLAG(uart_enhanced->hal_handle, UART_CLEAR_OREF);

    // Use HAL polling mode (no DMA to avoid ERRATA 2.20.4)
    HAL_StatusTypeDef hal_status = HAL_UART_Transmit(
        uart_enhanced->hal_handle, (uint8_t *)data, length, timeout_ms);

    // Check for HAL errors
    bool error_occurred = (hal_status != HAL_OK);
    if (error_occurred) {
        UART_CheckHALErrors(uart_enhanced);
    }

    // Update statistics
    UART_UpdateStatistics(uart_enhanced, true, length, error_occurred);

    // Convert HAL status to system error
    switch (hal_status) {
    case HAL_OK:
        return SYSTEM_OK;
    case HAL_TIMEOUT:
        uart_enhanced->error_flags |= UART_ERROR_TIMEOUT;
        return ERROR_TIMEOUT;
    case HAL_ERROR:
        return ERROR_UART_COMMUNICATION_FAILED;
    case HAL_BUSY:
        return ERROR_UART_BUSY;
    default:
        return ERROR_UART_COMMUNICATION_FAILED;
    }
}

/**
 * @brief Receive data with enhanced error detection
 */
SystemError_t UART_Enhanced_Receive(UART_Enhanced_t *uart_enhanced,
                                    uint8_t *data, uint16_t length,
                                    uint32_t timeout_ms) {
    if (uart_enhanced == NULL || data == NULL || length == 0) {
        return ERROR_INVALID_PARAMETER;
    }

    if (!uart_enhanced->initialized) {
        return ERROR_UART_NOT_INITIALIZED;
    }

    // Clear any existing errors
    __HAL_UART_CLEAR_FLAG(uart_enhanced->hal_handle, UART_CLEAR_PEF);
    __HAL_UART_CLEAR_FLAG(uart_enhanced->hal_handle, UART_CLEAR_FEF);
    __HAL_UART_CLEAR_FLAG(uart_enhanced->hal_handle, UART_CLEAR_NEF);
    __HAL_UART_CLEAR_FLAG(uart_enhanced->hal_handle, UART_CLEAR_OREF);

    // Use HAL polling mode (no DMA to avoid ERRATA 2.20.4)
    HAL_StatusTypeDef hal_status =
        HAL_UART_Receive(uart_enhanced->hal_handle, data, length, timeout_ms);

    // Check for HAL errors and ERRATA-specific issues
    bool error_occurred = (hal_status != HAL_OK);
    if (error_occurred) {
        UART_CheckHALErrors(uart_enhanced);

        // Check for noise error with ONEBIT (ERRATA 2.20.6)
        if (UART_CheckNoiseErrorWithOnebit(uart_enhanced)) {
            uart_enhanced->error_flags |= UART_ERROR_NOISE;
        }
    }

    // Update statistics
    UART_UpdateStatistics(uart_enhanced, false, length, error_occurred);

    // Convert HAL status to system error
    switch (hal_status) {
    case HAL_OK:
        return SYSTEM_OK;
    case HAL_TIMEOUT:
        uart_enhanced->error_flags |= UART_ERROR_TIMEOUT;
        return ERROR_TIMEOUT;
    case HAL_ERROR:
        return ERROR_UART_COMMUNICATION_FAILED;
    case HAL_BUSY:
        return ERROR_UART_BUSY;
    default:
        return ERROR_UART_COMMUNICATION_FAILED;
    }
}

/**
 * @brief Transmit string with automatic length calculation
 */
SystemError_t UART_Enhanced_TransmitString(UART_Enhanced_t *uart_enhanced,
                                           const char *str,
                                           uint32_t timeout_ms) {
    if (str == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    uint16_t length = strlen(str);
    return UART_Enhanced_Transmit(uart_enhanced, (const uint8_t *)str, length,
                                  timeout_ms);
}

/**
 * @brief Check for and handle UART errors
 */
UART_ErrorFlags_t UART_Enhanced_CheckErrors(UART_Enhanced_t *uart_enhanced) {
    if (uart_enhanced == NULL || !uart_enhanced->initialized) {
        return UART_ERROR_NONE;
    }

    // Check HAL-level errors
    UART_CheckHALErrors(uart_enhanced);

    // Check for ERRATA-specific issues
    if (UART_CheckDMALock(uart_enhanced)) {
        uart_enhanced->error_flags |= UART_ERROR_DMA_LOCK;
    }

    if (UART_CheckNoiseErrorWithOnebit(uart_enhanced)) {
        uart_enhanced->error_flags |= UART_ERROR_NOISE;
    }

    return uart_enhanced->error_flags;
}

/**
 * @brief Clear UART errors and reset statistics
 */
SystemError_t UART_Enhanced_ClearErrors(UART_Enhanced_t *uart_enhanced) {
    if (uart_enhanced == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // Clear error flags
    uart_enhanced->error_flags = UART_ERROR_NONE;

    // Clear HAL error flags
    if (uart_enhanced->hal_handle != NULL) {
        __HAL_UART_CLEAR_FLAG(uart_enhanced->hal_handle, UART_CLEAR_PEF);
        __HAL_UART_CLEAR_FLAG(uart_enhanced->hal_handle, UART_CLEAR_FEF);
        __HAL_UART_CLEAR_FLAG(uart_enhanced->hal_handle, UART_CLEAR_NEF);
        __HAL_UART_CLEAR_FLAG(uart_enhanced->hal_handle, UART_CLEAR_OREF);

        // Reset HAL error code
        uart_enhanced->hal_handle->ErrorCode = HAL_UART_ERROR_NONE;
    }

    // Optionally reset error counters (keep total byte counts)
    uart_enhanced->stats.tx_errors = 0;
    uart_enhanced->stats.rx_errors = 0;
    uart_enhanced->stats.timeout_count = 0;
    uart_enhanced->stats.noise_errors = 0;
    uart_enhanced->stats.frame_errors = 0;
    uart_enhanced->stats.overrun_errors = 0;

    return SYSTEM_OK;
}

/**
 * @brief Get UART statistics for monitoring
 */
UART_Statistics_t
UART_Enhanced_GetStatistics(const UART_Enhanced_t *uart_enhanced) {
    UART_Statistics_t empty_stats = {0};

    if (uart_enhanced == NULL) {
        return empty_stats;
    }

    return uart_enhanced->stats;
}

/**
 * @brief Verify baud rate accuracy
 */
SystemError_t UART_Enhanced_VerifyBaudRate(UART_Enhanced_t *uart_enhanced) {
    if (uart_enhanced == NULL || uart_enhanced->hal_handle == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    uint32_t desired_baud = uart_enhanced->hal_handle->Init.BaudRate;
    uint8_t oversampling =
        (uart_enhanced->hal_handle->Init.OverSampling == UART_OVERSAMPLING_8)
            ? 8
            : 16;

    // Calculate actual baud rate
    uint32_t actual_baud;
    float error_percent;

    UART_CalculateBaudRate(uart_enhanced->clock_frequency, desired_baud,
                           oversampling, &actual_baud, &error_percent);

    // Store results
    uart_enhanced->baud_rate_actual = actual_baud;
    uart_enhanced->baud_rate_error_percent = error_percent;

    // Check if error is within acceptable limits (±2.5%)
    if (error_percent > 2.5f) {
        return ERROR_UART_BAUD_RATE_ERROR;
    }

    return SYSTEM_OK;
}

/**
 * @brief Perform UART health check
 */
SystemError_t UART_Enhanced_HealthCheck(UART_Enhanced_t *uart_enhanced) {
    if (uart_enhanced == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (!uart_enhanced->initialized) {
        return ERROR_UART_NOT_INITIALIZED;
    }

    // Check for critical errors
    if (uart_enhanced->error_flags &
        (UART_ERROR_DMA_LOCK | UART_ERROR_ABREN_CORRUPTION)) {
        return ERROR_UART_CRITICAL_ERROR;
    }

    // Check baud rate accuracy
    SystemError_t baud_result = UART_Enhanced_VerifyBaudRate(uart_enhanced);
    if (baud_result != SYSTEM_OK) {
        return baud_result;
    }

    // Check error rate (more than 10% error rate is concerning)
    uint32_t total_operations = uart_enhanced->stats.tx_bytes_total +
                                uart_enhanced->stats.rx_bytes_total;
    uint32_t total_errors =
        uart_enhanced->stats.tx_errors + uart_enhanced->stats.rx_errors;

    if (total_operations > 100 &&
        (total_errors * 100 / total_operations) > 10) {
        return ERROR_UART_HIGH_ERROR_RATE;
    }

    return SYSTEM_OK;
}

/**
 * @brief Reset UART peripheral (recovery from errors)
 */
SystemError_t UART_Enhanced_Reset(UART_Enhanced_t *uart_enhanced) {
    if (uart_enhanced == NULL || uart_enhanced->hal_handle == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // Store configuration
    UART_InitTypeDef init_backup = uart_enhanced->hal_handle->Init;

    // Deinitialize UART
    HAL_StatusTypeDef hal_status = HAL_UART_DeInit(uart_enhanced->hal_handle);
    if (hal_status != HAL_OK) {
        return ERROR_UART_RESET_FAILED;
    }

    // Reinitialize with same configuration
    uart_enhanced->hal_handle->Init = init_backup;
    hal_status = HAL_UART_Init(uart_enhanced->hal_handle);
    if (hal_status != HAL_OK) {
        return ERROR_UART_INIT_FAILED;
    }

    // Clear errors and reset statistics
    UART_Enhanced_ClearErrors(uart_enhanced);

    return SYSTEM_OK;
}

/* ==========================================================================
 * Baud Rate Calculation Functions
 * ==========================================================================
 */

/**
 * @brief Calculate optimal UART baud rate for given clock
 */
uint32_t UART_CalculateBaudRate(uint32_t clock_freq, uint32_t desired_baud,
                                uint8_t oversampling, uint32_t *actual_baud,
                                float *error_percent) {
    if (actual_baud == NULL || error_percent == NULL) {
        return 0;
    }

    // Calculate BRR value (following AN4908 algorithm)
    uint32_t brr = (clock_freq + (desired_baud * oversampling / 2)) /
                   (desired_baud * oversampling);

    // Ensure BRR is at least 1
    if (brr == 0) {
        brr = 1;
    }

    // Calculate actual baud rate
    *actual_baud = clock_freq / (brr * oversampling);

    // Calculate error percentage
    if (desired_baud > 0) {
        float error = ((float)*actual_baud - (float)desired_baud) /
                      (float)desired_baud * 100.0f;
        *error_percent = (error < 0) ? -error : error; // Absolute value
    } else {
        *error_percent = 100.0f; // Invalid desired baud rate
    }

    return brr;
}

/**
 * @brief Verify UART clock configuration
 */
uint32_t UART_GetClockFrequency(USART_TypeDef *uart_instance) {
    // Determine which APB domain the UART is on
    if (uart_instance == USART1 || uart_instance == USART6) {
        // APB2 domain
        return APB2_CLOCK_FREQ;
    } else {
        // APB1 domain (USART2, USART3, UART4, UART5, UART7, UART8)
        return APB1_CLOCK_FREQ;
    }
}

/* ==========================================================================
 * ERRATA-Specific Workaround Functions
 * ==========================================================================
 */

/**
 * @brief Check for DMA lock condition (ERRATA 2.20.4)
 */
bool UART_CheckDMALock(const UART_Enhanced_t *uart_enhanced) {
    if (uart_enhanced == NULL || uart_enhanced->hal_handle == NULL) {
        return false;
    }

    // Check if DMA is enabled (should not be per ERRATA)
    return uart_enhanced->dma_enabled;
}

/**
 * @brief Check for noise error with ONEBIT setting (ERRATA 2.20.6)
 */
bool UART_CheckNoiseErrorWithOnebit(const UART_Enhanced_t *uart_enhanced) {
    if (uart_enhanced == NULL || uart_enhanced->hal_handle == NULL) {
        return false;
    }

    // Check if noise error flag is set AND ONEBIT sampling is enabled
    bool noise_error =
        __HAL_UART_GET_FLAG(uart_enhanced->hal_handle, UART_FLAG_NE);
    bool onebit_enabled = (uart_enhanced->hal_handle->Init.OneBitSampling ==
                           UART_ONE_BIT_SAMPLE_ENABLE);

    return (noise_error && onebit_enabled);
}

/**
 * @brief Safely configure UART without triggering ERRATA issues
 */
SystemError_t UART_SafeConfiguration(UART_HandleTypeDef *hal_handle) {
    if (hal_handle == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // Check for ERRATA 2.20.4: Ensure DMA is not used
    if (hal_handle->hdmatx != NULL || hal_handle->hdmarx != NULL) {
        // Warning: DMA detected - this may cause stream lock issues
        return ERROR_UART_CONFIG_WARNING;
    }

    // Check for ERRATA 2.20.5: Ensure ABREN is not used
    // (This would be in advanced features, which our implementation doesn't
    // use)

    // Check for ERRATA 2.20.6: Warn about ONEBIT with noise detection
    if (hal_handle->Init.OneBitSampling == UART_ONE_BIT_SAMPLE_ENABLE) {
        // This configuration may cause spurious noise errors
        return ERROR_UART_CONFIG_WARNING;
    }

    return SYSTEM_OK;
}

/* ==========================================================================
 * Enhanced Printf Implementation
 * ==========================================================================
 */

/**
 * @brief Enhanced printf redirection with error handling
 */
int UART_Enhanced_Putchar(UART_Enhanced_t *uart_enhanced, int ch) {
    if (uart_enhanced == NULL || !uart_enhanced->initialized) {
        return EOF;
    }

    uint8_t c = (uint8_t)ch;
    SystemError_t result = UART_Enhanced_Transmit(uart_enhanced, &c, 1, 100);

    return (result == SYSTEM_OK) ? ch : EOF;
}

/**
 * @brief Set the UART instance for printf redirection
 */
void UART_Enhanced_SetPrintfTarget(UART_Enhanced_t *uart_enhanced) {
    g_printf_uart = uart_enhanced;
}

/**
 * @brief Standard library putchar redirection
 */
int __io_putchar(int ch) {
    if (g_printf_uart != NULL) {
        return UART_Enhanced_Putchar(g_printf_uart, ch);
    }
    return EOF;
}

/* ==========================================================================
 * Private Function Implementations
 * ==========================================================================
 */

/**
 * @brief Update UART statistics
 */
static void UART_UpdateStatistics(UART_Enhanced_t *uart_enhanced, bool is_tx,
                                  uint16_t bytes, bool error_occurred) {
    if (uart_enhanced == NULL) {
        return;
    }

    if (is_tx) {
        uart_enhanced->stats.tx_bytes_total += bytes;
        if (error_occurred) {
            uart_enhanced->stats.tx_errors++;
        }
    } else {
        uart_enhanced->stats.rx_bytes_total += bytes;
        if (error_occurred) {
            uart_enhanced->stats.rx_errors++;
        }
    }

    if (error_occurred) {
        uart_enhanced->stats.last_error_time = HAL_GetTick();
        uart_enhanced->stats.last_error_type = uart_enhanced->error_flags;
    }
}

/**
 * @brief Check HAL-level errors and update flags
 */
static SystemError_t UART_CheckHALErrors(UART_Enhanced_t *uart_enhanced) {
    if (uart_enhanced == NULL || uart_enhanced->hal_handle == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    uint32_t hal_error = uart_enhanced->hal_handle->ErrorCode;

    // Clear previous error flags
    uart_enhanced->error_flags &= ~(UART_ERROR_PARITY | UART_ERROR_NOISE |
                                    UART_ERROR_FRAME | UART_ERROR_OVERRUN);

    // Map HAL errors to enhanced error flags
    if (hal_error & HAL_UART_ERROR_PE) {
        uart_enhanced->error_flags |= UART_ERROR_PARITY;
        uart_enhanced->stats.tx_errors++; // Could be TX or RX
    }

    if (hal_error & HAL_UART_ERROR_NE) {
        uart_enhanced->error_flags |= UART_ERROR_NOISE;
        uart_enhanced->stats.noise_errors++;
    }

    if (hal_error & HAL_UART_ERROR_FE) {
        uart_enhanced->error_flags |= UART_ERROR_FRAME;
        uart_enhanced->stats.frame_errors++;
    }

    if (hal_error & HAL_UART_ERROR_ORE) {
        uart_enhanced->error_flags |= UART_ERROR_OVERRUN;
        uart_enhanced->stats.overrun_errors++;
    }

    return (hal_error == HAL_UART_ERROR_NONE)
               ? SYSTEM_OK
               : ERROR_UART_COMMUNICATION_FAILED;
}
