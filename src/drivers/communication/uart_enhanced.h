/**
 * @file uart_enhanced.h
 * @brief Enhanced UART implementation with comprehensive error handling
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * @note Enhanced version of UART implementation that addresses all ERRATA
 * issues and provides robust error handling following AN4908 guidelines.
 */

#ifndef UART_ENHANCED_H
#define UART_ENHANCED_H

#include "common/error_handling.h"
#include "config/comm_config.h"
#include "config/hardware_config.h"
#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

/* ==========================================================================
 * UART Enhanced Error Handling (Following ERRATA Guidelines)
 * ==========================================================================
 */

/**
 * @brief UART Error Status Flags
 * @note Based on STM32H753ZI ERRATA 2.20.x requirements
 */
typedef enum {
    UART_ERROR_NONE = 0x00,
    UART_ERROR_PARITY = 0x01,
    UART_ERROR_NOISE = 0x02,
    UART_ERROR_FRAME = 0x04,
    UART_ERROR_OVERRUN = 0x08,
    UART_ERROR_DMA_LOCK = 0x10,         // ERRATA 2.20.4 detection
    UART_ERROR_ABREN_CORRUPTION = 0x20, // ERRATA 2.20.5 detection
    UART_ERROR_TIMEOUT = 0x40,
    UART_ERROR_BUFFER_FULL = 0x80
} UART_ErrorFlags_t;

/**
 * @brief UART Statistics for Monitoring
 * @note Helps detect ERRATA issues in production
 */
typedef struct {
    uint32_t tx_bytes_total;
    uint32_t rx_bytes_total;
    uint32_t tx_errors;
    uint32_t rx_errors;
    uint32_t timeout_count;
    uint32_t noise_errors; // Track ERRATA 2.20.6
    uint32_t frame_errors;
    uint32_t overrun_errors;
    uint32_t last_error_time;
    UART_ErrorFlags_t last_error_type;
} UART_Statistics_t;

/**
 * @brief Enhanced UART Handle Structure
 * @note Extends HAL handle with error tracking and ERRATA compliance
 */
typedef struct {
    UART_HandleTypeDef *hal_handle; // STM32 HAL UART handle
    UART_Statistics_t stats;        // Error statistics
    UART_ErrorFlags_t error_flags;  // Current error state
    bool initialized;               // Initialization status
    bool dma_enabled;               // DMA status (should be false per ERRATA)
    uint32_t baud_rate_actual;      // Calculated actual baud rate
    float baud_rate_error_percent;  // Baud rate accuracy
    uint32_t clock_frequency;       // APB clock frequency
} UART_Enhanced_t;

/* ==========================================================================
 * Function Prototypes
 * ==========================================================================
 */

/**
 * @brief Initialize enhanced UART with comprehensive error handling
 * @param uart_enhanced Pointer to enhanced UART structure
 * @param hal_handle Pointer to STM32 HAL UART handle
 * @return SystemError_t Success or error code
 *
 * @note Performs ERRATA compliance checks and clock verification
 */
SystemError_t UART_Enhanced_Init(UART_Enhanced_t *uart_enhanced,
                                 UART_HandleTypeDef *hal_handle);

/**
 * @brief Transmit data with enhanced error detection
 * @param uart_enhanced Pointer to enhanced UART structure
 * @param data Pointer to data buffer
 * @param length Number of bytes to transmit
 * @param timeout_ms Timeout in milliseconds
 * @return SystemError_t Success or error code
 *
 * @note Avoids DMA usage per ERRATA 2.20.4
 */
SystemError_t UART_Enhanced_Transmit(UART_Enhanced_t *uart_enhanced,
                                     const uint8_t *data, uint16_t length,
                                     uint32_t timeout_ms);

/**
 * @brief Receive data with enhanced error detection
 * @param uart_enhanced Pointer to enhanced UART structure
 * @param data Pointer to receive buffer
 * @param length Number of bytes to receive
 * @param timeout_ms Timeout in milliseconds
 * @return SystemError_t Success or error code
 *
 * @note Implements noise error detection per ERRATA 2.20.6
 */
SystemError_t UART_Enhanced_Receive(UART_Enhanced_t *uart_enhanced,
                                    uint8_t *data, uint16_t length,
                                    uint32_t timeout_ms);

/**
 * @brief Transmit string with automatic length calculation
 * @param uart_enhanced Pointer to enhanced UART structure
 * @param str Null-terminated string to transmit
 * @param timeout_ms Timeout in milliseconds
 * @return SystemError_t Success or error code
 */
SystemError_t UART_Enhanced_TransmitString(UART_Enhanced_t *uart_enhanced,
                                           const char *str,
                                           uint32_t timeout_ms);

/**
 * @brief Check for and handle UART errors
 * @param uart_enhanced Pointer to enhanced UART structure
 * @return UART_ErrorFlags_t Current error flags
 *
 * @note Detects all ERRATA-related issues
 */
UART_ErrorFlags_t UART_Enhanced_CheckErrors(UART_Enhanced_t *uart_enhanced);

/**
 * @brief Clear UART errors and reset statistics
 * @param uart_enhanced Pointer to enhanced UART structure
 * @return SystemError_t Success or error code
 */
SystemError_t UART_Enhanced_ClearErrors(UART_Enhanced_t *uart_enhanced);

/**
 * @brief Get UART statistics for monitoring
 * @param uart_enhanced Pointer to enhanced UART structure
 * @return UART_Statistics_t Copy of current statistics
 */
UART_Statistics_t
UART_Enhanced_GetStatistics(const UART_Enhanced_t *uart_enhanced);

/**
 * @brief Verify baud rate accuracy
 * @param uart_enhanced Pointer to enhanced UART structure
 * @return SystemError_t Success if within tolerance, error otherwise
 *
 * @note Calculates actual vs. configured baud rate based on AN4908
 */
SystemError_t UART_Enhanced_VerifyBaudRate(UART_Enhanced_t *uart_enhanced);

/**
 * @brief Perform UART health check
 * @param uart_enhanced Pointer to enhanced UART structure
 * @return SystemError_t Overall health status
 *
 * @note Comprehensive check including ERRATA compliance
 */
SystemError_t UART_Enhanced_HealthCheck(UART_Enhanced_t *uart_enhanced);

/**
 * @brief Reset UART peripheral (recovery from errors)
 * @param uart_enhanced Pointer to enhanced UART structure
 * @return SystemError_t Success or error code
 *
 * @note Safe reset procedure following ERRATA guidelines
 */
SystemError_t UART_Enhanced_Reset(UART_Enhanced_t *uart_enhanced);

/* ==========================================================================
 * Baud Rate Calculation and Verification (AN4908 Compliant)
 * ==========================================================================
 */

/**
 * @brief Calculate optimal UART baud rate for given clock
 * @param clock_freq APB clock frequency in Hz
 * @param desired_baud Desired baud rate
 * @param oversampling Oversampling factor (8 or 16)
 * @param actual_baud Pointer to store actual baud rate
 * @param error_percent Pointer to store error percentage
 * @return uint32_t BRR register value
 *
 * @note Implements AN4908 baud rate calculation algorithm
 */
uint32_t UART_CalculateBaudRate(uint32_t clock_freq, uint32_t desired_baud,
                                uint8_t oversampling, uint32_t *actual_baud,
                                float *error_percent);

/**
 * @brief Verify UART clock configuration
 * @param uart_instance UART instance (USART1, USART2, etc.)
 * @return uint32_t Actual APB clock frequency
 *
 * @note Determines correct APB domain for UART instance
 */
uint32_t UART_GetClockFrequency(USART_TypeDef *uart_instance);

/* ==========================================================================
 * ERRATA-Specific Workarounds
 * ==========================================================================
 */

/**
 * @brief Check for DMA lock condition (ERRATA 2.20.4)
 * @param uart_enhanced Pointer to enhanced UART structure
 * @return bool True if DMA lock detected
 */
bool UART_CheckDMALock(const UART_Enhanced_t *uart_enhanced);

/**
 * @brief Check for noise error with ONEBIT setting (ERRATA 2.20.6)
 * @param uart_enhanced Pointer to enhanced UART structure
 * @return bool True if noise error with ONEBIT detected
 */
bool UART_CheckNoiseErrorWithOnebit(const UART_Enhanced_t *uart_enhanced);

/**
 * @brief Safely configure UART without triggering ERRATA issues
 * @param hal_handle Pointer to STM32 HAL UART handle
 * @return SystemError_t Success or error code
 */
SystemError_t UART_SafeConfiguration(UART_HandleTypeDef *hal_handle);

/* ==========================================================================
 * Enhanced Printf Implementation
 * ==========================================================================
 */

/**
 * @brief Enhanced printf redirection with error handling
 * @param uart_enhanced Pointer to enhanced UART structure
 * @param ch Character to output
 * @return int Character written or EOF on error
 */
int UART_Enhanced_Putchar(UART_Enhanced_t *uart_enhanced, int ch);

/**
 * @brief Set the UART instance for printf redirection
 * @param uart_enhanced Pointer to enhanced UART structure
 */
void UART_Enhanced_SetPrintfTarget(UART_Enhanced_t *uart_enhanced);

/* ==========================================================================
 * Macro Definitions for Easy Usage
 * ==========================================================================
 */

// Timeout values based on baud rate and message length
#define UART_TIMEOUT_FOR_BYTES(bytes, baud_rate)                              \
    (((bytes * 10 * 1000) / (baud_rate)) + 100)

// Error checking macros
#define UART_IS_ERROR(uart) ((uart)->error_flags != UART_ERROR_NONE)
#define UART_HAS_TIMEOUT_ERROR(uart) ((uart)->error_flags & UART_ERROR_TIMEOUT)
#define UART_HAS_COMM_ERROR(uart)                                             \
    ((uart)->error_flags &                                                    \
     (UART_ERROR_PARITY | UART_ERROR_NOISE | UART_ERROR_FRAME))

// Statistics access macros
#define UART_GET_TX_COUNT(uart) ((uart)->stats.tx_bytes_total)
#define UART_GET_RX_COUNT(uart) ((uart)->stats.rx_bytes_total)
#define UART_GET_ERROR_COUNT(uart)                                            \
    ((uart)->stats.tx_errors + (uart)->stats.rx_errors)

#endif /* UART_ENHANCED_H */

/**
 * @note Enhanced UART Implementation Guidelines:
 * 1. Always use enhanced functions instead of direct HAL calls
 * 2. Check error status after each operation
 * 3. Monitor statistics for ERRATA-related issues
 * 4. Never enable DMA (per ERRATA 2.20.4)
 * 5. Use oversampling 16 for best reliability
 * 6. Verify baud rate accuracy periodically
 * 7. Implement proper timeout handling
 * 8. Log all communication errors for debugging
 */
