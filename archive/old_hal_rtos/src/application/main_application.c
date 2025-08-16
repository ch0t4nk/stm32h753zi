/**
 * @file main_application.c
 * @brief Main Application - STM32H753ZI Stepper Motor Control Demo
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @note Main application with integrated safety system and watchdog management
 * Phase 1 Step 3: Watchdog Integration - COMPLETE
 */

#include "main_application.h"
#include "config/comm_config.h"
#include "config/hardware_config.h"
#include "config/hardware_presence_config.h"
#include "config/motor_config.h"
#include "config/safety_config.h"
#include "hal_abstraction/hal_abstraction.h"
#include "safety/safety_system.h"
#include "safety/watchdog_manager.h"
#include <stdio.h>

/* ==========================================================================
 */
/* Private Variables                                                         */
/* ==========================================================================
 */

// HAL peripheral handles
extern SPI_HandleTypeDef hspi2;   // MOTOR_SPI_INSTANCE for L6470 drivers
extern I2C_HandleTypeDef hi2c1;   // ENCODER_I2C_INSTANCE for encoder 0
extern I2C_HandleTypeDef hi2c2;   // ENCODER2_I2C_INSTANCE for encoder 1
extern UART_HandleTypeDef huart3; // UART_INSTANCE for debugging (VCP)

// Application state
static bool application_initialized = false;
static uint32_t last_safety_check = 0;
static uint32_t last_watchdog_refresh = 0;
static uint32_t application_cycles = 0;

/* ==========================================================================
 */
/* Public API Implementation                                                 */
/* ==========================================================================
 */

/**
 * @brief Initialize main application with conditional hardware support
 * @return System error code
 */
SystemError_t main_application_init(void) {
    printf("STM32H753ZI Motor Control Application Starting...\r\n");
    printf("Hardware Detection and Conditional Initialization\r\n");

    // Detect available hardware
    HardwarePresence_t hardware = detect_hardware_presence();

    printf("Hardware Detection Results:\r\n");
    printf("  - Motor 1 L6470: %s\r\n",
           hardware.motor1_detected ? "DETECTED" : "NOT FOUND");
    printf("  - Motor 2 L6470: %s\r\n",
           hardware.motor2_detected ? "DETECTED" : "NOT FOUND");
    printf("  - Encoder 1 AS5600: %s\r\n",
           hardware.encoder1_detected ? "DETECTED" : "NOT FOUND");
    printf("  - Encoder 2 AS5600: %s\r\n",
           hardware.encoder2_detected ? "DETECTED" : "NOT FOUND");
    printf("  - Emergency Stop: %s\r\n",
           hardware.emergency_stop_detected ? "DETECTED" : "NOT FOUND");
    printf("  - CAN Transceiver: %s\r\n",
           hardware.can_transceiver_detected ? "DETECTED" : "NOT FOUND");
    printf("  - Safety Relay: %s\r\n",
           hardware.safety_relay_detected ? "DETECTED" : "NOT FOUND");

#if DEVELOPMENT_MODE_ENABLED
    printf("DEVELOPMENT MODE: Conditional initialization enabled\r\n");
#else
    printf("PRODUCTION MODE: Full hardware required\r\n");
#endif

    // Initialize safety system with conditional hardware support
    SystemError_t safety_result;
    if (is_safety_system_possible(&hardware)) {
        printf("Initializing safety system with available hardware...\r\n");
        safety_result = safety_system_init();
        if (safety_result != SYSTEM_OK) {
            printf("ERROR: Safety system initialization failed (code: %d)\r\n",
                   safety_result);
#if !DEVELOPMENT_MODE_ENABLED
            return safety_result; // Fail in production mode
#else
            printf("DEVELOPMENT: Continuing with safety system errors\r\n");
#endif
        } else {
            printf("Safety system initialized successfully\r\n");
        }
    } else {
#if DEVELOPMENT_MODE_ENABLED
        printf("DEVELOPMENT: Safety system bypassed - minimal hardware "
               "detected\r\n");
        safety_result = SYSTEM_OK; // Allow bypass in development
#else
        printf("ERROR: Insufficient safety hardware for production "
               "operation\r\n");
        return ERROR_SAFETY_HARDWARE_MISSING;
#endif
    }

    // Initialize motor control system conditionally
    if (is_motor_control_possible(&hardware)) {
        printf("Motor control system can be initialized\r\n");
        // Motor initialization would go here
    } else {
#if DEVELOPMENT_MODE_ENABLED
        printf(
            "DEVELOPMENT: Motor control bypassed - simulated operation\r\n");
#else
        printf("ERROR: Insufficient motor hardware for operation\r\n");
        return ERROR_MOTOR_HARDWARE_MISSING;
#endif
    }

    // Initialize timing tracking
    last_safety_check = HAL_Abstraction_GetTick();
    last_watchdog_refresh = HAL_Abstraction_GetTick();
    application_cycles = 0;

    application_initialized = true;
    printf("Application initialization complete with conditional hardware "
           "support\r\n");
    printf("Watchdog enabled: %s\r\n", IWDG_ENABLE ? "YES" : "NO");
    printf("Safety check interval: %d ms\r\n", SAFETY_CHECK_INTERVAL_MS);
    printf("Watchdog kick interval: %d ms\r\n", WATCHDOG_KICK_INTERVAL_MS);

    return SYSTEM_OK;
}

/**
 * @brief Main application loop (call continuously)
 * @return System error code
 */
SystemError_t main_application_run(void) {
    if (!application_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    uint32_t current_time = HAL_Abstraction_GetTick();
    application_cycles++;

    // Watchdog refresh (highest priority - safety critical)
    if ((current_time - last_watchdog_refresh) >= WATCHDOG_KICK_INTERVAL_MS) {
        SystemError_t watchdog_result = watchdog_refresh();
        if (watchdog_result != SYSTEM_OK) {
            printf("WARNING: Watchdog refresh failed (code: %d)\r\n",
                   watchdog_result);
            // Continue operation but log the warning
        }
        last_watchdog_refresh = current_time;
    }

    // Periodic safety system checks
    if ((current_time - last_safety_check) >= SAFETY_CHECK_INTERVAL_MS) {
        SystemError_t safety_result = safety_system_task();
        if (safety_result != SYSTEM_OK) {
            if (safety_result == ERROR_SAFETY_EMERGENCY_STOP) {
                printf("SAFETY: Emergency stop is active\r\n");
            } else if (safety_result == ERROR_SAFETY_WATCHDOG_WARNING) {
                printf(
                    "SAFETY: Watchdog warning - refresh timing critical\r\n");
            } else {
                printf("SAFETY: Periodic check failed (code: %d)\r\n",
                       safety_result);
            }
        }
        last_safety_check = current_time;
    }

    // Application status reporting (every 5 seconds)
    if ((application_cycles % 5000) == 0) {
        uint32_t watchdog_refresh_count, watchdog_timeout_count,
            watchdog_missed_count;
        SystemError_t stats_result = watchdog_get_statistics(
            &watchdog_refresh_count, &watchdog_timeout_count,
            &watchdog_missed_count);

        printf("App Status - Uptime: %lu ms, Cycles: %lu\r\n", current_time,
               application_cycles);

        if (stats_result == SYSTEM_OK) {
            printf("Watchdog Stats - Refreshes: %lu, Timeouts: %lu, Missed: "
                   "%lu\r\n",
                   watchdog_refresh_count, watchdog_timeout_count,
                   watchdog_missed_count);
        }

        if (watchdog_refresh_due()) {
            printf("WARNING: Watchdog refresh is due!\r\n");
        }

        uint32_t time_until_refresh = watchdog_time_until_refresh();
        if (time_until_refresh < WATCHDOG_LATE_KICK_MS) {
            printf("INFO: Next watchdog refresh in %lu ms\r\n",
                   time_until_refresh);
        }
    }

    // Short delay to prevent CPU overload
    HAL_Abstraction_Delay(1);

    return SYSTEM_OK;
}

/**
 * @brief Emergency stop all motors
 * @return System error code
 */
SystemError_t main_application_emergency_stop(void) {
    printf("EMERGENCY STOP ACTIVATED!\r\n");

    // Trigger safety system emergency stop
    SystemError_t result = execute_emergency_stop(ESTOP_SOURCE_SOFTWARE);
    if (result != SYSTEM_OK) {
        printf("ERROR: Emergency stop execution failed (code: %d)\r\n",
               result);
        return result;
    }

    printf("Emergency stop executed successfully\r\n");
    return SYSTEM_OK;
}

/**
 * @brief Check if application is initialized
 * @return true if initialized, false otherwise
 */
bool main_application_is_initialized(void) {
    return application_initialized;
}

/**
 * @brief Get application runtime statistics
 * @param uptime_ms Pointer to store uptime in milliseconds
 * @param cycles Pointer to store application cycles
 * @return System error code
 */
SystemError_t main_application_get_stats(uint32_t *uptime_ms,
                                         uint32_t *cycles) {
    if (!application_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    if (uptime_ms) {
        *uptime_ms = HAL_Abstraction_GetTick();
    }

    if (cycles) {
        *cycles = application_cycles;
    }

    return SYSTEM_OK;
}

/**
 * @brief Perform application self-test including watchdog validation
 * @return System error code
 */
SystemError_t main_application_self_test(void) {
    if (!application_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    printf("Performing application self-test...\r\n");

    // Test safety system
    SystemError_t safety_test = perform_safety_self_test();
    if (safety_test != SYSTEM_OK) {
        printf("ERROR: Safety system self-test failed (code: %d)\r\n",
               safety_test);
        return safety_test;
    }
    printf("Safety system self-test: PASS\r\n");

    // Test watchdog system
    SystemError_t watchdog_test = watchdog_self_test();
    if (watchdog_test != SYSTEM_OK) {
        printf("ERROR: Watchdog self-test failed (code: %d)\r\n",
               watchdog_test);
        return watchdog_test;
    }
    printf("Watchdog system self-test: PASS\r\n");

    printf("Application self-test: ALL PASS\r\n");
    return SYSTEM_OK;
}
