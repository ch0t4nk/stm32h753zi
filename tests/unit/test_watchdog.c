/**
 * @file test_watchdog.c
 * @brief Focused unit tests for watchdog management
 * @author STM32H753ZI Motor Control Project
 * @date 2024
 */

#include <unity.h>
#include "mock_hal.h"
#include "watchdog_manager.h"

void setUp(void) {
    MockHAL_Reset();
    watchdog_init();
}

void tearDown(void) {
    MockHAL_Reset();
}

void test_watchdog_refresh_interval(void) {
    uint32_t initial_refreshes = MockHAL_GetWatchdogRefreshCount();
    
    // Simulate normal operation with regular refreshes
    for (int i = 0; i < 5; i++) {
        MockHAL_AdvanceTick(WATCHDOG_REFRESH_INTERVAL_MS);
        watchdog_process();
    }
    
    uint32_t final_refreshes = MockHAL_GetWatchdogRefreshCount();
    TEST_ASSERT_EQUAL(initial_refreshes + 5, final_refreshes);
}

void test_watchdog_timeout_calculation(void) {
    // Test timeout calculation based on refresh interval
    uint32_t expected_timeout = WATCHDOG_REFRESH_INTERVAL_MS * WATCHDOG_TIMEOUT_MULTIPLIER;
    uint32_t actual_timeout = watchdog_get_timeout_ms();
    
    TEST_ASSERT_EQUAL(expected_timeout, actual_timeout);
}

void test_watchdog_recovery_after_timeout(void) {
    // Force timeout
    MockHAL_AdvanceTick(WATCHDOG_TIMEOUT_MS + 100);
    watchdog_process();
    TEST_ASSERT_EQUAL(WATCHDOG_TIMEOUT, watchdog_get_state());
    
    // Test recovery procedure
    SystemError_t result = watchdog_recover();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Verify normal operation resumed
    TEST_ASSERT_EQUAL(WATCHDOG_ACTIVE, watchdog_get_state());
    
    // Verify refresh works after recovery
    result = watchdog_refresh();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void test_watchdog_statistics(void) {
    // Perform multiple refresh cycles
    for (int i = 0; i < 10; i++) {
        MockHAL_AdvanceTick(WATCHDOG_REFRESH_INTERVAL_MS);
        watchdog_process();
    }
    
    WatchdogStats_t stats = watchdog_get_statistics();
    TEST_ASSERT_EQUAL(10, stats.refresh_count);
    TEST_ASSERT_EQUAL(0, stats.timeout_count);
    TEST_ASSERT_GREATER_THAN(0, stats.total_runtime_ms);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_watchdog_refresh_interval);
    RUN_TEST(test_watchdog_timeout_calculation);
    RUN_TEST(test_watchdog_recovery_after_timeout);
    RUN_TEST(test_watchdog_statistics);
    
    return UNITY_END();
}
