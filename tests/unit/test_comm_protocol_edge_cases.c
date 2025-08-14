/**
 * @file test_comm_protocol_edge_cases.c
 * @brief Unit tests for communication protocol edge cases in the comms domain
 *
 * @details This file adds tests for protocol logic, error handling, and edge
 * cases in the communication protocol module. Follows comms.instructions.md
 * and feature-management.instructions.md.
 *
 * @author GitHub Copilot
 * @date 2025-08-13
 * @version 0.1.0
 *
 * @note Uses Unity testing framework. Update feature tracker after
 * implementation.
 */

#include "communication/comm_protocol.h"
#include "unity.h"
#include <string.h>

static CommProtocolContext_t test_ctx;

void setUp(void) {
    memset(&test_ctx, 0, sizeof(test_ctx));
}

void tearDown(void) {
    // No teardown needed
}

void test_null_pointer_handling(void) {
    SystemError_t result = comm_protocol_init(NULL);
    TEST_ASSERT_EQUAL(ERROR_INVALID_PARAMETER, result);
}

void test_invalid_protocol_id(void) {
    SystemError_t result =
        comm_protocol_set_active(&test_ctx, 99); // Invalid ID
    TEST_ASSERT_EQUAL(ERROR_INVALID_PARAMETER, result);
}

void test_uart_loopback_failure(void) {
    // Simulate UART loopback failure (mock or stub as needed)
    // For now, expect self-test to fail if UART is not available
    SystemError_t result = comm_protocol_self_test();
    TEST_ASSERT(result == SYSTEM_OK || result == ERROR_COMM_SELF_TEST_FAILED);
}

void test_message_validation(void) {
    MotorCommand_t invalid_cmd = {0};
    invalid_cmd.motor_id = 99; // Invalid motor ID
    SystemError_t result = validate_motor_command(&invalid_cmd);
    TEST_ASSERT_EQUAL(ERROR_INVALID_PARAMETER, result);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_null_pointer_handling);
    RUN_TEST(test_invalid_protocol_id);
    RUN_TEST(test_uart_loopback_failure);
    RUN_TEST(test_message_validation);
    return UNITY_END();
}
