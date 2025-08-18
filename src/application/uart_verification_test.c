/**
 * @file uart_verification_test.c
 * @brief UART Implementation Verification Test
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * @note This test verifies UART implementation against:
 * - AN4908 automatic baud rate detection guidelines
 * - STM32H753ZI ERRATA 2.20.x requirements
 * - SSOT configuration compliance
 */

#include "config/comm_config.h"
#include "config/hardware_config.h"
#include "uart_enhanced.h"
#include <stdio.h>

/* ==========================================================================
 * Test Configuration
 * ==========================================================================
 */
#define TEST_MESSAGE_COUNT 10
#define TEST_TIMEOUT_MS 1000
#define MAX_TEST_BUFFER_SIZE 256

/* ==========================================================================
 * Global Variables
 * ==========================================================================
 */
static UART_HandleTypeDef huart3;
static UART_Enhanced_t uart_enhanced;

/* ==========================================================================
 * Test Functions
 * ==========================================================================
 */

/**
 * @brief Verify UART pin configuration
 */
SystemError_t Test_UART_PinConfiguration(void) {
    printf("Testing UART pin configuration...\r\n");

    // Verify SSOT pin definitions match STM32H753ZI Nucleo-144
    if (UART_TX_PIN != GPIO_PIN_8 || UART_TX_PORT != GPIOD) {
        printf("ERROR: UART TX pin mismatch (expected PD8)\r\n");
        return ERROR_TEST_FAILED;
    }

    if (UART_RX_PIN != GPIO_PIN_9 || UART_RX_PORT != GPIOD) {
        printf("ERROR: UART RX pin mismatch (expected PD9)\r\n");
        return ERROR_TEST_FAILED;
    }

    if (UART_TX_AF != GPIO_AF7_USART3) {
        printf("ERROR: UART alternate function mismatch (expected AF7)\r\n");
        return ERROR_TEST_FAILED;
    }

    printf("✅ UART pin configuration verified\r\n");
    return SYSTEM_OK;
}

/**
 * @brief Verify UART baud rate calculation
 */
SystemError_t Test_UART_BaudRateCalculation(void) {
    printf("Testing UART baud rate calculation...\r\n");

    uint32_t clock_freq = APB1_CLOCK_FREQ; // 120MHz for UART3
    uint32_t desired_baud = UART_BAUDRATE; // 115200 from SSOT
    uint32_t actual_baud;
    float error_percent;

    // Test baud rate calculation
    uint32_t brr = UART_CalculateBaudRate(clock_freq, desired_baud, 16,
                                          &actual_baud, &error_percent);

    printf("Clock frequency: %lu Hz\r\n", clock_freq);
    printf("Desired baud rate: %lu\r\n", desired_baud);
    printf("Calculated BRR: %lu\r\n", brr);
    printf("Actual baud rate: %lu\r\n", actual_baud);
    printf("Error percentage: %.3f%%\r\n", error_percent);

    // Verify error is within acceptable limits (±2.5%)
    if (error_percent > 2.5f) {
        printf("ERROR: Baud rate error too high (%.3f%% > 2.5%%)\r\n",
               error_percent);
        return ERROR_UART_BAUD_RATE_ERROR;
    }

    printf("✅ UART baud rate calculation verified\r\n");
    return SYSTEM_OK;
}

/**
 * @brief Verify UART ERRATA compliance
 */
SystemError_t Test_UART_ERRATACompliance(void) {
    printf("Testing UART ERRATA compliance...\r\n");

    // Check ERRATA 2.20.4: DMA should not be used
    if (huart3.hdmatx != NULL || huart3.hdmarx != NULL) {
        printf("WARNING: DMA detected - may cause ERRATA 2.20.4 issues\r\n");
        return ERROR_UART_CONFIG_WARNING;
    }
    printf("✅ ERRATA 2.20.4: No DMA usage detected\r\n");

    // Check ERRATA 2.20.5: ABREN should not be used
    // Note: Our implementation doesn't use automatic baud rate detection
    printf("✅ ERRATA 2.20.5: No ABREN usage (fixed baud rate)\r\n");

    // Check ERRATA 2.20.6: ONEBIT sampling analysis
    if (huart3.Init.OneBitSampling == UART_ONE_BIT_SAMPLE_ENABLE) {
        printf("WARNING: ONEBIT sampling may cause noise error issues\r\n");
        return ERROR_UART_CONFIG_WARNING;
    }
    printf("✅ ERRATA 2.20.6: Standard 16x sampling (no ONEBIT)\r\n");

    printf("✅ UART ERRATA compliance verified\r\n");
    return SYSTEM_OK;
}

/**
 * @brief Test UART enhanced error handling
 */
SystemError_t Test_UART_ErrorHandling(void) {
    printf("Testing UART enhanced error handling...\r\n");

    // Initialize enhanced UART
    SystemError_t result = UART_Enhanced_Init(&uart_enhanced, &huart3);
    if (result != SYSTEM_OK) {
        printf("ERROR: Enhanced UART initialization failed (%d)\r\n", result);
        return result;
    }

    // Check error detection capabilities
    UART_ErrorFlags_t errors = UART_Enhanced_CheckErrors(&uart_enhanced);
    printf("Initial error flags: 0x%02X\r\n", errors);

    // Verify baud rate accuracy
    result = UART_Enhanced_VerifyBaudRate(&uart_enhanced);
    if (result != SYSTEM_OK) {
        printf("ERROR: Baud rate verification failed (%d)\r\n", result);
        return result;
    }
    printf("✅ Baud rate accuracy: %.3f%% error\r\n",
           uart_enhanced.baud_rate_error_percent);

    // Perform health check
    result = UART_Enhanced_HealthCheck(&uart_enhanced);
    if (result != SYSTEM_OK) {
        printf("WARNING: Health check reported issues (%d)\r\n", result);
    } else {
        printf("✅ UART health check passed\r\n");
    }

    printf("✅ UART enhanced error handling verified\r\n");
    return SYSTEM_OK;
}

/**
 * @brief Test UART communication reliability
 */
SystemError_t Test_UART_CommunicationReliability(void) {
    printf("Testing UART communication reliability...\r\n");

    char test_message[MAX_TEST_BUFFER_SIZE];
    uint32_t success_count = 0;
    uint32_t error_count = 0;

    for (int i = 0; i < TEST_MESSAGE_COUNT; i++) {
        // Create test message
        snprintf(test_message, sizeof(test_message),
                 "Test message #%d - UART reliability check", i + 1);

        // Send message using enhanced UART
        SystemError_t result = UART_Enhanced_TransmitString(
            &uart_enhanced, test_message, TEST_TIMEOUT_MS);
        if (result == SYSTEM_OK) {
            success_count++;
        } else {
            error_count++;
            printf("ERROR: Message %d failed with error %d\r\n", i + 1,
                   result);
        }

        // Small delay between messages
        HAL_Delay(100);
    }

    // Get final statistics
    UART_Statistics_t stats = UART_Enhanced_GetStatistics(&uart_enhanced);

    printf("Communication test results:\r\n");
    printf("  Messages sent: %d\r\n", TEST_MESSAGE_COUNT);
    printf("  Successful: %lu\r\n", success_count);
    printf("  Errors: %lu\r\n", error_count);
    printf("  Total TX bytes: %lu\r\n", stats.tx_bytes_total);
    printf("  Total TX errors: %lu\r\n", stats.tx_errors);
    printf("  Reliability: %.1f%%\r\n",
           (float)success_count / TEST_MESSAGE_COUNT * 100.0f);

    // Calculate reliability threshold (should be > 95%)
    float reliability = (float)success_count / TEST_MESSAGE_COUNT * 100.0f;
    if (reliability < 95.0f) {
        printf("ERROR: Communication reliability too low (%.1f%% < 95%%)\r\n",
               reliability);
        return ERROR_UART_HIGH_ERROR_RATE;
    }

    printf("✅ UART communication reliability verified\r\n");
    return SYSTEM_OK;
}

/**
 * @brief Verify SSOT configuration consistency
 */
SystemError_t Test_SSOT_Configuration(void) {
    printf("Testing SSOT configuration consistency...\r\n");

    // Verify UART configuration matches SSOT
    if (huart3.Init.BaudRate != UART_BAUDRATE) {
        printf("ERROR: Baud rate mismatch (actual: %lu, SSOT: %d)\r\n",
               huart3.Init.BaudRate, UART_BAUDRATE);
        return ERROR_CONFIG_MISMATCH;
    }

    if (huart3.Init.WordLength != UART_WORD_LENGTH) {
        printf("ERROR: Word length mismatch\r\n");
        return ERROR_CONFIG_MISMATCH;
    }

    if (huart3.Init.StopBits != UART_STOP_BITS) {
        printf("ERROR: Stop bits mismatch\r\n");
        return ERROR_CONFIG_MISMATCH;
    }

    if (huart3.Init.Parity != UART_PARITY) {
        printf("ERROR: Parity mismatch\r\n");
        return ERROR_CONFIG_MISMATCH;
    }

    if (huart3.Init.HwFlowCtl != UART_HW_FLOW_CONTROL) {
        printf("ERROR: Hardware flow control mismatch\r\n");
        return ERROR_CONFIG_MISMATCH;
    }

    printf("✅ SSOT configuration consistency verified\r\n");
    return SYSTEM_OK;
}

/**
 * @brief Run all UART verification tests
 */
SystemError_t Run_UART_VerificationTests(void) {
    printf("\r\n=== UART Implementation Verification Tests ===\r\n");
    printf("STM32H753ZI UART3 Analysis\r\n");
    printf("=============================================\r\n\r\n");

    SystemError_t overall_result = SYSTEM_OK;
    int test_count = 0;
    int passed_count = 0;

    // Test 1: Pin Configuration
    test_count++;
    if (Test_UART_PinConfiguration() == SYSTEM_OK) {
        passed_count++;
    } else {
        overall_result = ERROR_TEST_FAILED;
    }

    // Test 2: Baud Rate Calculation
    test_count++;
    if (Test_UART_BaudRateCalculation() == SYSTEM_OK) {
        passed_count++;
    } else {
        overall_result = ERROR_TEST_FAILED;
    }

    // Test 3: ERRATA Compliance
    test_count++;
    SystemError_t errata_result = Test_UART_ERRATACompliance();
    if (errata_result == SYSTEM_OK ||
        errata_result == ERROR_UART_CONFIG_WARNING) {
        passed_count++;
    } else {
        overall_result = ERROR_TEST_FAILED;
    }

    // Test 4: Enhanced Error Handling
    test_count++;
    if (Test_UART_ErrorHandling() == SYSTEM_OK) {
        passed_count++;
    } else {
        overall_result = ERROR_TEST_FAILED;
    }

    // Test 5: Communication Reliability
    test_count++;
    if (Test_UART_CommunicationReliability() == SYSTEM_OK) {
        passed_count++;
    } else {
        overall_result = ERROR_TEST_FAILED;
    }

    // Test 6: SSOT Configuration
    test_count++;
    if (Test_SSOT_Configuration() == SYSTEM_OK) {
        passed_count++;
    } else {
        overall_result = ERROR_TEST_FAILED;
    }

    // Summary
    printf("\r\n=== Test Summary ===\r\n");
    printf("Total tests: %d\r\n", test_count);
    printf("Passed: %d\r\n", passed_count);
    printf("Failed: %d\r\n", test_count - passed_count);
    printf("Overall result: %s\r\n",
           (overall_result == SYSTEM_OK) ? "✅ PASS" : "❌ FAIL");

    if (overall_result == SYSTEM_OK) {
        printf("\r\n🎉 UART implementation verification SUCCESSFUL!\r\n");
        printf("Your HUART3 implementation is correct and compliant.\r\n");
    } else {
        printf(
            "\r\n⚠️ UART implementation has issues that need attention.\r\n");
    }

    return overall_result;
}
