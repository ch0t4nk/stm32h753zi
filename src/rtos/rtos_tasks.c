/**
 * @file rtos_tasks.c
 * @brief FreeRTOS Task Implementations for STM32H753ZI Motor Control System
 * @author STM32H753ZI Project Team
 * @date August 2025
 * @version 1.0
 *
 * This file implements FreeRTOS tasks that integrate existing application
 * functionality into a multi-tasking real-time system. All tasks follow
 * SSOT configuration and safety-critical design principles.
 *
 * @note Phase 2: RTOS Tasks Integration - IMPLEMENTATION
 * Integration Architecture:
 * - MotorControlTask: Calls motor_controller_update() at 1kHz
 * - SafetyMonitorTask: Calls safety_system_task() at 500Hz
 * - CAN/UART Tasks: Handle communication protocols
 * - TelemetryTask: System monitoring and health reporting
 */

/* ==========================================================================
 */
/* Includes */
/* ==========================================================================
 */

#include "rtos_tasks.h"

// SSOT Configuration
#include "config/comm_config.h"
#include "config/freertos_config_ssot.h"
#include "config/motor_config.h"

// Common system includes
#include "common/error_codes.h"

// Existing application modules
#include "application/main_application.h"
#include "controllers/motor_controller.h"
#include "hal_abstraction/hal_abstraction.h"
#include "safety/safety_system.h"
#include "safety/watchdog_manager.h"

// Communication modules (when available)
#ifdef ENABLE_CAN_COMMUNICATION
#include "communication/can_protocol.h"
#endif

#ifdef ENABLE_UART_COMMUNICATION
#include "communication/uart_protocol.h"
#endif

#include <stdio.h>
#include <string.h> // For memset, memcpy

// Forward declarations for static functions
static SystemError_t init_event_groups(void);
static SystemError_t init_memory_pools(void);
static SystemError_t init_task_utilities(void);

/* ==========================================================================
 */
/* Private Variables                                                         */
/* ==========================================================================
 */

// Task handles (defined in rtos_resources_config.h, implemented here)
TaskHandle_t motor_control_task_handle = NULL;
TaskHandle_t safety_monitor_task_handle = NULL;
TaskHandle_t can_comm_task_handle = NULL;
TaskHandle_t uart_comm_task_handle = NULL;
TaskHandle_t telemetry_task_handle = NULL;

// Queue handles (defined in rtos_resources_config.h, implemented here)
QueueHandle_t motor_command_queue = NULL;
QueueHandle_t can_message_queue = NULL;
QueueHandle_t uart_message_queue = NULL;
QueueHandle_t telemetry_queue = NULL;
QueueHandle_t safety_event_queue = NULL;

// Mutex handles (defined in rtos_resources_config.h, implemented here)
SemaphoreHandle_t spi_mutex = NULL;
SemaphoreHandle_t i2c_mutex = NULL;
SemaphoreHandle_t can_mutex = NULL;
SemaphoreHandle_t motor_state_mutex = NULL;
SemaphoreHandle_t error_log_mutex = NULL;

// Semaphore handles for ISR-to-Task communication
SemaphoreHandle_t motor_timer_semaphore = NULL;
SemaphoreHandle_t encoder_interrupt_semaphore = NULL;

// Timer handles
TimerHandle_t watchdog_timer = NULL;
TimerHandle_t performance_monitor_timer = NULL;

// Task statistics and monitoring
static uint32_t motor_task_cycles = 0;
static uint32_t safety_task_cycles = 0;
static uint32_t can_task_cycles = 0;
static uint32_t uart_task_cycles = 0;
static uint32_t telemetry_task_cycles = 0;

static bool tasks_initialized = false;
static bool emergency_stop_active = false;

/* ==========================================================================
 */
/* Task Implementations                                                      */
/* ==========================================================================
 */

/**
 * @brief High-priority motor control task (1kHz)
 *
 * Integrates existing motor_controller_update() into FreeRTOS task.
 * Provides deterministic 1ms timing for precise motor control.
 */
void MotorControlTask(void *argument) {
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(MOTOR_CONTROL_PERIOD_MS);
    MotorCommand_t received_command;

    // Initialize the xLastWakeTime variable with the current time
    xLastWakeTime = xTaskGetTickCount();

    printf("MotorControlTask: Started with %dms period (Priority %d)\r\n",
           MOTOR_CONTROL_PERIOD_MS, MOTOR_CONTROL_TASK_PRIORITY);

    for (;;) {
        motor_task_cycles++;

        // Check for emergency stop
        if (emergency_stop_active) {
            // During emergency stop, just wait and don't execute motor control
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
            continue;
        }

        // Process any pending motor commands (non-blocking)
        while (xQueueReceive(motor_command_queue, &received_command, 0) ==
               pdTRUE) {
            // Process the motor command using existing controller
            printf("MotorControlTask: Processing command for motor %d\r\n",
                   received_command.motor_id);
            // TODO: Integrate with motor_controller_set_target() or similar
            // function
        }

        // Execute main motor control loop using existing function
        SystemError_t result;

        // Protect motor state with mutex
        if (xSemaphoreTake(motor_state_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
            result = motor_controller_update();
            xSemaphoreGive(motor_state_mutex);
        } else {
            result = ERROR_BUSY;
            printf(
                "MotorControlTask: Failed to acquire motor state mutex\r\n");
        }

        // Handle any errors from motor control
        if (result != SYSTEM_OK) {
            SafetyEvent_t safety_event = {
                .event_type = SAFETY_EVENT_MOTOR_ERROR,
                .error_code = result,
                .timestamp = xTaskGetTickCount(),
                .source_task = xTaskGetCurrentTaskHandle()};

            // Send to safety monitor (non-blocking to prevent task blocking)
            if (xQueueSend(safety_event_queue, &safety_event, 0) != pdTRUE) {
                printf("MotorControlTask: Safety event queue full!\r\n");
            }
        }

        // Performance monitoring (every 1000 cycles = 1 second)
        if ((motor_task_cycles % 1000) == 0) {
            UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            printf("MotorControlTask: Cycles=%lu, Stack Free=%lu words\r\n",
                   motor_task_cycles, uxHighWaterMark);
        }

        // Wait for the next cycle (precise 1ms timing)
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief Highest-priority safety monitoring task (500Hz)
 *
 * Integrates existing safety_system_task() into FreeRTOS task.
 * Provides continuous safety monitoring with 2ms period.
 */
void SafetyMonitorTask(void *argument) {
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(SAFETY_CHECK_PERIOD_MS);
    SafetyEvent_t received_event;

    // Initialize the xLastWakeTime variable with the current time
    xLastWakeTime = xTaskGetTickCount();

    printf("SafetyMonitorTask: Started with %dms period (Priority %d - "
           "HIGHEST)\r\n",
           SAFETY_CHECK_PERIOD_MS, SAFETY_MONITOR_TASK_PRIORITY);

    for (;;) {
        safety_task_cycles++;

        // Process any safety events from other tasks (non-blocking)
        while (xQueueReceive(safety_event_queue, &received_event, 0) ==
               pdTRUE) {
            printf("SafetyMonitorTask: Received safety event type %d from "
                   "task 0x%p\r\n",
                   received_event.event_type, received_event.source_task);

            // Handle critical safety events immediately
            if (received_event.event_type == SAFETY_EVENT_EMERGENCY_STOP) {
                emergency_stop_active = true;
                printf("SafetyMonitorTask: EMERGENCY STOP ACTIVATED!\r\n");

                // Suspend motor control task during emergency
                if (motor_control_task_handle != NULL) {
                    vTaskSuspend(motor_control_task_handle);
                    printf(
                        "SafetyMonitorTask: Motor control task suspended\r\n");
                }
            }
        }

        // Execute periodic safety checks using existing function
        SystemError_t safety_result = safety_system_task();

        // Handle safety system results
        if (safety_result == ERROR_SAFETY_EMERGENCY_STOP) {
            if (!emergency_stop_active) {
                emergency_stop_active = true;
                printf("SafetyMonitorTask: Safety system triggered emergency "
                       "stop\r\n");

                // Suspend motor control during emergency
                if (motor_control_task_handle != NULL) {
                    vTaskSuspend(motor_control_task_handle);
                }
            }
        } else if (safety_result == SYSTEM_OK && emergency_stop_active) {
            // Safety system cleared - resume normal operation
            emergency_stop_active = false;
            printf("SafetyMonitorTask: Emergency stop cleared - resuming "
                   "operation\r\n");

            // Resume motor control task
            if (motor_control_task_handle != NULL) {
                vTaskResume(motor_control_task_handle);
            }
        } else if (safety_result != SYSTEM_OK) {
            printf("SafetyMonitorTask: Safety check warning (code: %d)\r\n",
                   safety_result);
        }

        // Performance monitoring (every 500 cycles = 1 second at 500Hz)
        if ((safety_task_cycles % 500) == 0) {
            UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            printf("SafetyMonitorTask: Cycles=%lu, Stack Free=%lu words, "
                   "Emergency=%s\r\n",
                   safety_task_cycles, uxHighWaterMark,
                   emergency_stop_active ? "ACTIVE" : "CLEARED");
        }

        // Wait for the next cycle (precise 2ms timing)
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief CAN communication task (100Hz)
 *
 * Handles CAN bus communication for motor commands and status.
 */
void CANCommTask(void *argument) {
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(CAN_POLL_PERIOD_MS);

    // Initialize the xLastWakeTime variable with the current time
    xLastWakeTime = xTaskGetTickCount();

    printf("CANCommTask: Started with %dms period (Priority %d)\r\n",
           CAN_POLL_PERIOD_MS, CAN_COMM_TASK_PRIORITY);

    for (;;) {
        can_task_cycles++;

        // Skip CAN processing during emergency stop
        if (emergency_stop_active) {
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
            continue;
        }

// TODO: Integrate CAN communication when available
#ifdef ENABLE_CAN_COMMUNICATION
        // Check for incoming CAN messages
        CANMessage_t received_message;
        if (can_protocol_receive(&received_message) == SYSTEM_OK) {
            // Process CAN message and convert to motor command if applicable
            MotorCommand_t motor_command;
            if (can_protocol_parse_motor_command(
                    &received_message, &motor_command) == SYSTEM_OK) {
                // Send to motor control task
                if (xQueueSend(motor_command_queue, &motor_command,
                               pdMS_TO_TICKS(10)) != pdTRUE) {
                    printf("CANCommTask: Motor command queue full\r\n");
                }
            }
        }

        // Send any queued CAN messages
        CANMessage_t outgoing_message;
        if (xQueueReceive(can_message_queue, &outgoing_message, 0) == pdTRUE) {
            SystemError_t result = can_protocol_transmit(&outgoing_message);
            if (result != SYSTEM_OK) {
                printf("CANCommTask: CAN transmission failed (code: %d)\r\n",
                       result);
            }
        }
#else
        // CAN communication not yet implemented - placeholder
        if ((can_task_cycles % 1000) == 0) {
            printf("CANCommTask: Running (CAN implementation pending)\r\n");
        }
#endif

        // Performance monitoring (every 100 cycles = 1 second at 100Hz)
        if ((can_task_cycles % 100) == 0) {
            UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            printf("CANCommTask: Cycles=%lu, Stack Free=%lu words\r\n",
                   can_task_cycles, uxHighWaterMark);
        }

        // Wait for the next cycle
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief UART communication task (20Hz)
 *
 * Handles UART communication for debug output and commands.
 */
void UARTCommTask(void *argument) {
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(UART_POLL_PERIOD_MS);

    // Initialize the xLastWakeTime variable with the current time
    xLastWakeTime = xTaskGetTickCount();

    printf("UARTCommTask: Started with %dms period (Priority %d)\r\n",
           UART_POLL_PERIOD_MS, UART_COMM_TASK_PRIORITY);

    for (;;) {
        uart_task_cycles++;

// TODO: Integrate UART communication when available
#ifdef ENABLE_UART_COMMUNICATION
        // Process UART messages
        UARTMessage_t received_message;
        if (xQueueReceive(uart_message_queue, &received_message, 0) ==
            pdTRUE) {
            // Process UART message
            SystemError_t result = uart_protocol_process(&received_message);
            if (result != SYSTEM_OK) {
                printf("UARTCommTask: UART processing failed (code: %d)\r\n",
                       result);
            }
        }
#else
        // UART communication placeholder - basic status reporting
        if ((uart_task_cycles % 20) ==
            0) { // Every 20 cycles = 1 second at 20Hz
            printf("UARTCommTask: System Status - Motor cycles: %lu, Safety "
                   "cycles: %lu, Emergency: %s\r\n",
                   motor_task_cycles, safety_task_cycles,
                   emergency_stop_active ? "ACTIVE" : "NORMAL");
        }
#endif

        // Performance monitoring (every 20 cycles = 1 second at 20Hz)
        if ((uart_task_cycles % 20) == 0) {
            UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            printf("UARTCommTask: Cycles=%lu, Stack Free=%lu words\r\n",
                   uart_task_cycles, uxHighWaterMark);
        }

        // Wait for the next cycle
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief Telemetry and monitoring task (10Hz)
 *
 * Collects system telemetry and health monitoring data.
 */
void TelemetryTask(void *argument) {
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(TELEMETRY_PERIOD_MS);

    // Initialize the xLastWakeTime variable with the current time
    xLastWakeTime = xTaskGetTickCount();

    printf("TelemetryTask: Started with %dms period (Priority %d)\r\n",
           TELEMETRY_PERIOD_MS, TELEMETRY_TASK_PRIORITY);

    for (;;) {
        telemetry_task_cycles++;

        // Collect system telemetry
        uint32_t uptime = xTaskGetTickCount();
        size_t free_heap = xPortGetFreeHeapSize();
        size_t min_ever_free_heap = xPortGetMinimumEverFreeHeapSize();

        // Get task statistics
        UBaseType_t motor_stack_free = 0;
        UBaseType_t safety_stack_free = 0;

        if (motor_control_task_handle != NULL) {
            motor_stack_free =
                uxTaskGetStackHighWaterMark(motor_control_task_handle);
        }

        if (safety_monitor_task_handle != NULL) {
            safety_stack_free =
                uxTaskGetStackHighWaterMark(safety_monitor_task_handle);
        }

        // Report telemetry every 10 cycles (1 second at 10Hz)
        if ((telemetry_task_cycles % 10) == 0) {
            printf("\r\n=== SYSTEM TELEMETRY ===\r\n");
            printf("Uptime: %lu ms\r\n", uptime);
            printf("Heap Free: %u bytes (Min Ever: %u bytes)\r\n", free_heap,
                   min_ever_free_heap);
            printf("Task Cycles - Motor: %lu, Safety: %lu, CAN: %lu, UART: "
                   "%lu\r\n",
                   motor_task_cycles, safety_task_cycles, can_task_cycles,
                   uart_task_cycles);
            printf("Task Stack Free - Motor: %lu, Safety: %lu words\r\n",
                   motor_stack_free, safety_stack_free);
            printf("Emergency Stop: %s\r\n",
                   emergency_stop_active ? "ACTIVE" : "NORMAL");
            printf("========================\r\n\r\n");
        }

        // TODO: Send telemetry data to external systems when communication is
        // available
        TelemetryData_t telemetry_data = {
            .uptime_ms = uptime,
            .free_heap_bytes = free_heap,
            .motor_task_cycles = motor_task_cycles,
            .safety_task_cycles = safety_task_cycles,
            .emergency_stop_active = emergency_stop_active};

        // TODO: Actually transmit telemetry_data when comm protocol ready
        (void)telemetry_data; // Suppress unused variable warning

        // Performance monitoring
        if ((telemetry_task_cycles % 10) == 0) {
            UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            printf("TelemetryTask: Cycles=%lu, Stack Free=%lu words\r\n",
                   telemetry_task_cycles, uxHighWaterMark);
        }

        // Wait for the next cycle
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief Timer service callback for watchdog and maintenance
 */
void TimerServiceCallback(TimerHandle_t xTimer) {
    static uint32_t timer_callback_count = 0;
    timer_callback_count++;

    // Refresh watchdog using existing function
    SystemError_t watchdog_result = watchdog_refresh();
    if (watchdog_result != SYSTEM_OK) {
        printf("TimerServiceCallback: Watchdog refresh failed (code: %d)\r\n",
               watchdog_result);
    }

    // Periodic maintenance (every 10 callbacks = 5 seconds)
    if ((timer_callback_count % 10) == 0) {
        printf("TimerServiceCallback: System maintenance - Count: %lu\r\n",
               timer_callback_count);

        // Collect memory statistics
        size_t free_heap = xPortGetFreeHeapSize();
        size_t min_ever_free = xPortGetMinimumEverFreeHeapSize();

        if (free_heap < (RTOS_HEAP_SIZE_BYTES / 4)) {
            printf("TimerServiceCallback: WARNING - Low heap memory: %u "
                   "bytes (min ever: %u)\r\n",
                   free_heap, min_ever_free);
        }
    }
}

/* ==========================================================================
 */
/* Task Management Functions                                                 */
/* ==========================================================================
 */

/**
 * @brief Initialize all RTOS tasks and resources
 */
SystemError_t rtos_tasks_init(void) {
    printf("RTOS Tasks: Initializing FreeRTOS task system...\r\n");

    if (tasks_initialized) {
        printf("RTOS Tasks: Already initialized\r\n");
        return SYSTEM_OK;
    }

    // Create mutexes for resource protection
    spi_mutex = xSemaphoreCreateMutex();
    i2c_mutex = xSemaphoreCreateMutex();
    can_mutex = xSemaphoreCreateMutex();
    motor_state_mutex = xSemaphoreCreateMutex();
    error_log_mutex = xSemaphoreCreateMutex();

    if (!spi_mutex || !i2c_mutex || !can_mutex || !motor_state_mutex ||
        !error_log_mutex) {
        printf("RTOS Tasks: ERROR - Failed to create mutexes\r\n");
        return ERROR_INSUFFICIENT_RESOURCES;
    }

    // Create binary semaphores for ISR-to-Task communication
    motor_timer_semaphore = xSemaphoreCreateBinary();
    encoder_interrupt_semaphore = xSemaphoreCreateBinary();

    if (!motor_timer_semaphore || !encoder_interrupt_semaphore) {
        printf("RTOS Tasks: ERROR - Failed to create semaphores\r\n");
        return ERROR_INSUFFICIENT_RESOURCES;
    }

    // Create queues for inter-task communication
    motor_command_queue =
        xQueueCreate(MOTOR_COMMAND_QUEUE_SIZE, sizeof(MotorCommand_t));
    can_message_queue =
        xQueueCreate(CAN_MESSAGE_QUEUE_SIZE, sizeof(CANMessage_t));
    uart_message_queue =
        xQueueCreate(UART_MESSAGE_QUEUE_SIZE, sizeof(UARTMessage_t));
    telemetry_queue =
        xQueueCreate(TELEMETRY_QUEUE_SIZE, sizeof(TelemetryData_t));
    safety_event_queue =
        xQueueCreate(SAFETY_EVENT_QUEUE_SIZE, sizeof(SafetyEvent_t));

    if (!motor_command_queue || !can_message_queue || !uart_message_queue ||
        !telemetry_queue || !safety_event_queue) {
        printf("RTOS Tasks: ERROR - Failed to create queues\r\n");
        return ERROR_MEMORY_ALLOCATION;
    }

    // Create software timers
    watchdog_timer = xTimerCreate("WatchdogTimer",
                                  pdMS_TO_TICKS(WATCHDOG_REFRESH_PERIOD_MS),
                                  pdTRUE, // Auto-reload
                                  (void *)0, TimerServiceCallback);

    if (!watchdog_timer) {
        printf("RTOS Tasks: ERROR - Failed to create watchdog timer\r\n");
        return ERROR_MEMORY_ALLOCATION;
    }

    printf("RTOS Tasks: All resources created successfully\r\n");
    tasks_initialized = true;

    return rtos_tasks_start();
}

/**
 * @brief Start all RTOS tasks
 */
SystemError_t rtos_tasks_start(void) {
    if (!tasks_initialized) {
        printf("RTOS Tasks: ERROR - Tasks not initialized\r\n");
        return ERROR_NOT_INITIALIZED;
    }

    printf("RTOS Tasks: Creating and starting all tasks...\r\n");

    // Create Motor Control Task (Priority 3)
    BaseType_t result = xTaskCreate(
        MotorControlTask, "MotorControl", MOTOR_CONTROL_TASK_STACK_SIZE, NULL,
        MOTOR_CONTROL_TASK_PRIORITY, &motor_control_task_handle);

    if (result != pdPASS) {
        printf("RTOS Tasks: ERROR - Failed to create Motor Control Task\r\n");
        return ERROR_MEMORY_ALLOCATION;
    }

    // Create Safety Monitor Task (Priority 4 - HIGHEST)
    result = xTaskCreate(
        SafetyMonitorTask, "SafetyMonitor", SAFETY_MONITOR_TASK_STACK_SIZE,
        NULL, SAFETY_MONITOR_TASK_PRIORITY, &safety_monitor_task_handle);

    if (result != pdPASS) {
        printf("RTOS Tasks: ERROR - Failed to create Safety Monitor Task\r\n");
        return ERROR_MEMORY_ALLOCATION;
    }

    // Create CAN Communication Task (Priority 2)
    result = xTaskCreate(CANCommTask, "CANComm", CAN_COMM_TASK_STACK_SIZE,
                         NULL, CAN_COMM_TASK_PRIORITY, &can_comm_task_handle);

    if (result != pdPASS) {
        printf(
            "RTOS Tasks: ERROR - Failed to create CAN Communication Task\r\n");
        return ERROR_MEMORY_ALLOCATION;
    }

    // Create UART Communication Task (Priority 1)
    result =
        xTaskCreate(UARTCommTask, "UARTComm", UART_COMM_TASK_STACK_SIZE, NULL,
                    UART_COMM_TASK_PRIORITY, &uart_comm_task_handle);

    if (result != pdPASS) {
        printf("RTOS Tasks: ERROR - Failed to create UART Communication "
               "Task\r\n");
        return ERROR_MEMORY_ALLOCATION;
    }

    // Create Telemetry Task (Priority 1)
    result =
        xTaskCreate(TelemetryTask, "Telemetry", TELEMETRY_TASK_STACK_SIZE,
                    NULL, TELEMETRY_TASK_PRIORITY, &telemetry_task_handle);

    if (result != pdPASS) {
        printf("RTOS Tasks: ERROR - Failed to create Telemetry Task\r\n");
        return ERROR_MEMORY_ALLOCATION;
    }

    // Start software timers
    if (xTimerStart(watchdog_timer, pdMS_TO_TICKS(100)) != pdPASS) {
        printf("RTOS Tasks: ERROR - Failed to start watchdog timer\r\n");
        return ERROR_OPERATION_FAILED;
    }

    printf("RTOS Tasks: All tasks created and started successfully!\r\n");
    printf("Task Configuration:\r\n");
    printf("  Motor Control: Priority %d, Stack %d words, Period %d ms\r\n",
           MOTOR_CONTROL_TASK_PRIORITY, MOTOR_CONTROL_TASK_STACK_SIZE,
           MOTOR_CONTROL_PERIOD_MS);
    printf("  Safety Monitor: Priority %d, Stack %d words, Period %d ms\r\n",
           SAFETY_MONITOR_TASK_PRIORITY, SAFETY_MONITOR_TASK_STACK_SIZE,
           SAFETY_CHECK_PERIOD_MS);
    printf("  CAN Comm: Priority %d, Stack %d words, Period %d ms\r\n",
           CAN_COMM_TASK_PRIORITY, CAN_COMM_TASK_STACK_SIZE,
           CAN_POLL_PERIOD_MS);
    printf("  UART Comm: Priority %d, Stack %d words, Period %d ms\r\n",
           UART_COMM_TASK_PRIORITY, UART_COMM_TASK_STACK_SIZE,
           UART_POLL_PERIOD_MS);
    printf("  Telemetry: Priority %d, Stack %d words, Period %d ms\r\n",
           TELEMETRY_TASK_PRIORITY, TELEMETRY_TASK_STACK_SIZE,
           TELEMETRY_PERIOD_MS);

    // Initialize Phase 2D advanced components
    printf("RTOS Tasks: Initializing Phase 2D advanced integration...\r\n");

    SystemError_t phase2d_result;

    phase2d_result = init_event_groups();
    if (phase2d_result != SYSTEM_OK) {
        printf("RTOS Tasks: ERROR - Phase 2D event groups initialization "
               "failed\r\n");
        return phase2d_result;
    }

    phase2d_result = init_memory_pools();
    if (phase2d_result != SYSTEM_OK) {
        printf("RTOS Tasks: ERROR - Phase 2D memory pools initialization "
               "failed\r\n");
        return phase2d_result;
    }

    phase2d_result = init_task_utilities();
    if (phase2d_result != SYSTEM_OK) {
        printf("RTOS Tasks: ERROR - Phase 2D task utilities initialization "
               "failed\r\n");
        return phase2d_result;
    }

    printf("RTOS Tasks: Phase 2D advanced integration complete!\r\n");
    printf("RTOS Tasks: Event groups, memory pools, and task utilities "
           "operational\r\n");

    return SYSTEM_OK;
}

/**
 * @brief Emergency stop all tasks
 */
SystemError_t rtos_tasks_emergency_stop(void) {
    printf("RTOS Tasks: EMERGENCY STOP - Suspending non-critical tasks\r\n");

    emergency_stop_active = true;

    // Suspend motor control task
    if (motor_control_task_handle != NULL) {
        vTaskSuspend(motor_control_task_handle);
        printf("RTOS Tasks: Motor control task suspended\r\n");
    }

    // Keep safety monitor running (critical for recovery)
    // Keep telemetry running for monitoring
    // Communication tasks continue for emergency reporting

    return SYSTEM_OK;
}

/**
 * @brief Resume normal task operation
 */
SystemError_t rtos_tasks_resume(void) {
    printf("RTOS Tasks: Resuming normal operation\r\n");

    emergency_stop_active = false;

    // Resume motor control task
    if (motor_control_task_handle != NULL) {
        vTaskResume(motor_control_task_handle);
        printf("RTOS Tasks: Motor control task resumed\r\n");
    }

    return SYSTEM_OK;
}

/* ==========================================================================
 */
/* Inter-Task Communication Functions                                        */
/* ==========================================================================
 */

/**
 * @brief Send motor command to motor control task
 */
SystemError_t rtos_send_motor_command(const MotorCommand_t *command,
                                      uint32_t timeout_ms) {
    if (!tasks_initialized || !command) {
        return ERROR_INVALID_PARAMETER;
    }

    TickType_t timeout_ticks =
        (timeout_ms == 0) ? 0 : pdMS_TO_TICKS(timeout_ms);

    if (xQueueSend(motor_command_queue, command, timeout_ticks) == pdTRUE) {
        return SYSTEM_OK;
    }

    return ERROR_QUEUE_FULL;
}

/**
 * @brief Send safety event to safety monitor task
 */
SystemError_t rtos_send_safety_event(const SafetyEvent_t *event,
                                     uint32_t timeout_ms) {
    if (!tasks_initialized || !event) {
        return ERROR_INVALID_PARAMETER;
    }

    TickType_t timeout_ticks =
        (timeout_ms == 0) ? 0 : pdMS_TO_TICKS(timeout_ms);

    if (xQueueSend(safety_event_queue, event, timeout_ticks) == pdTRUE) {
        return SYSTEM_OK;
    }

    return ERROR_QUEUE_FULL;
}

/**
 * @brief Send telemetry data
 */
SystemError_t rtos_send_telemetry(const TelemetryData_t *data,
                                  uint32_t timeout_ms) {
    if (!tasks_initialized || !data) {
        return ERROR_INVALID_PARAMETER;
    }

    TickType_t timeout_ticks =
        (timeout_ms == 0) ? 0 : pdMS_TO_TICKS(timeout_ms);

    if (xQueueSend(telemetry_queue, data, timeout_ticks) == pdTRUE) {
        return SYSTEM_OK;
    }

    return ERROR_QUEUE_FULL;
}

/* ==========================================================================
 */
/* Phase 2D: Advanced Integration - Event Groups Implementation            */
/* ==========================================================================
 */

// Event Group Handles (defined in header as extern)
EventGroupHandle_t system_event_group = NULL;
EventGroupHandle_t comm_event_group = NULL;
EventGroupHandle_t motion_event_group = NULL;

/**
 * @brief Initialize event groups for Phase 2D advanced integration
 */
static SystemError_t init_event_groups(void) {
    printf("RTOS Tasks: Initializing event groups...\r\n");

    system_event_group = xEventGroupCreate();
    comm_event_group = xEventGroupCreate();
    motion_event_group = xEventGroupCreate();

    if (!system_event_group || !comm_event_group || !motion_event_group) {
        printf("RTOS Tasks: ERROR - Failed to create event groups\r\n");
        return ERROR_MEMORY_ALLOCATION;
    }

    printf("RTOS Tasks: Event groups initialized successfully\r\n");
    return SYSTEM_OK;
}

/**
 * @brief Wait for system startup completion
 */
SystemError_t rtos_wait_system_startup(uint32_t timeout_ms) {
    if (!system_event_group) {
        return ERROR_NOT_INITIALIZED;
    }

    EventBits_t required_events =
        SYSTEM_EVENT_MOTORS_INITIALIZED | SYSTEM_EVENT_SAFETY_CHECKS_PASSED |
        SYSTEM_EVENT_COMM_READY | SYSTEM_EVENT_SENSORS_CALIBRATED;

    EventBits_t result =
        xEventGroupWaitBits(system_event_group, required_events, pdFALSE,
                            pdTRUE, pdMS_TO_TICKS(timeout_ms));

    if ((result & required_events) == required_events) {
        // Set startup complete event
        xEventGroupSetBits(system_event_group, SYSTEM_EVENT_STARTUP_COMPLETE);
        return SYSTEM_OK;
    }

    return ERROR_TIMEOUT;
}

/**
 * @brief Signal emergency stop to all tasks
 */
SystemError_t rtos_signal_emergency_stop(void) {
    if (!system_event_group) {
        return ERROR_NOT_INITIALIZED;
    }

    xEventGroupSetBits(system_event_group, SYSTEM_EVENT_EMERGENCY_STOP);
    printf("RTOS Tasks: Emergency stop event signaled to all tasks\r\n");
    return SYSTEM_OK;
}

/**
 * @brief Wait for coordinated motion completion
 */
SystemError_t rtos_wait_motion_complete(uint32_t motors, uint32_t timeout_ms) {
    if (!motion_event_group) {
        return ERROR_NOT_INITIALIZED;
    }

    EventBits_t required_events = 0;

    if (motors & (1 << 0)) { // Motor 1
        required_events |= MOTION_EVENT_MOTOR1_TARGET_REACHED;
    }
    if (motors & (1 << 1)) { // Motor 2
        required_events |= MOTION_EVENT_MOTOR2_TARGET_REACHED;
    }

    EventBits_t result =
        xEventGroupWaitBits(motion_event_group, required_events, pdTRUE,
                            pdTRUE, pdMS_TO_TICKS(timeout_ms));

    if ((result & required_events) == required_events) {
        xEventGroupSetBits(motion_event_group, SYSTEM_EVENT_MOTION_COMPLETE);
        return SYSTEM_OK;
    }

    return ERROR_TIMEOUT;
}

/* ==========================================================================
 */
/* Phase 2D: Advanced Integration - Memory Pools Implementation            */
/* ==========================================================================
 */

// Memory Pool Handles (defined in header as extern)
osMemoryPoolId_t small_memory_pool = NULL;
osMemoryPoolId_t medium_memory_pool = NULL;
osMemoryPoolId_t large_memory_pool = NULL;

// Memory pool storage areas (statically allocated)
static uint8_t small_pool_storage[MEMORY_POOL_SMALL_TOTAL_SIZE]
    __attribute__((aligned(4)));
static uint8_t medium_pool_storage[MEMORY_POOL_MEDIUM_TOTAL_SIZE]
    __attribute__((aligned(4)));
static uint8_t large_pool_storage[MEMORY_POOL_LARGE_TOTAL_SIZE]
    __attribute__((aligned(4)));

/**
 * @brief Initialize memory pools for Phase 2D advanced integration
 */
static SystemError_t init_memory_pools(void) {
    printf("RTOS Tasks: Initializing memory pools...\r\n");

    const osMemoryPoolAttr_t small_pool_attr = {
        .name = "SmallPool",
        .attr_bits = 0U,
        .cb_mem = NULL,
        .cb_size = 0U,
        .mp_mem = small_pool_storage,
        .mp_size = sizeof(small_pool_storage)};

    const osMemoryPoolAttr_t medium_pool_attr = {
        .name = "MediumPool",
        .attr_bits = 0U,
        .cb_mem = NULL,
        .cb_size = 0U,
        .mp_mem = medium_pool_storage,
        .mp_size = sizeof(medium_pool_storage)};

    const osMemoryPoolAttr_t large_pool_attr = {
        .name = "LargePool",
        .attr_bits = 0U,
        .cb_mem = NULL,
        .cb_size = 0U,
        .mp_mem = large_pool_storage,
        .mp_size = sizeof(large_pool_storage)};

    small_memory_pool =
        osMemoryPoolNew(MEMORY_POOL_SMALL_BLOCK_COUNT,
                        MEMORY_POOL_SMALL_BLOCK_SIZE, &small_pool_attr);

    medium_memory_pool =
        osMemoryPoolNew(MEMORY_POOL_MEDIUM_BLOCK_COUNT,
                        MEMORY_POOL_MEDIUM_BLOCK_SIZE, &medium_pool_attr);

    large_memory_pool =
        osMemoryPoolNew(MEMORY_POOL_LARGE_BLOCK_COUNT,
                        MEMORY_POOL_LARGE_BLOCK_SIZE, &large_pool_attr);

    if (!small_memory_pool || !medium_memory_pool || !large_memory_pool) {
        printf("RTOS Tasks: ERROR - Failed to create memory pools\r\n");
        return ERROR_MEMORY_ALLOCATION;
    }

    printf("RTOS Tasks: Memory pools initialized - Small: %d blocks, Medium: "
           "%d blocks, Large: %d blocks\r\n",
           MEMORY_POOL_SMALL_BLOCK_COUNT, MEMORY_POOL_MEDIUM_BLOCK_COUNT,
           MEMORY_POOL_LARGE_BLOCK_COUNT);
    return SYSTEM_OK;
}

/**
 * @brief Allocate memory from appropriate pool
 */
void *rtos_memory_alloc(size_t size, uint32_t timeout_ms) {
    osMemoryPoolId_t pool = NULL;

    // Select appropriate pool based on size
    if (size <= MEMORY_POOL_SMALL_BLOCK_SIZE) {
        pool = small_memory_pool;
    } else if (size <= MEMORY_POOL_MEDIUM_BLOCK_SIZE) {
        pool = medium_memory_pool;
    } else if (size <= MEMORY_POOL_LARGE_BLOCK_SIZE) {
        pool = large_memory_pool;
    } else {
        // Size too large for any pool
        return NULL;
    }

    if (!pool) {
        return NULL;
    }

    return osMemoryPoolAlloc(pool, timeout_ms);
}

/**
 * @brief Free memory back to appropriate pool
 */
SystemError_t rtos_memory_free(void *ptr, size_t size) {
    if (!ptr) {
        return ERROR_INVALID_PARAMETER;
    }

    osMemoryPoolId_t pool = NULL;

    // Select appropriate pool based on size
    if (size <= MEMORY_POOL_SMALL_BLOCK_SIZE) {
        pool = small_memory_pool;
    } else if (size <= MEMORY_POOL_MEDIUM_BLOCK_SIZE) {
        pool = medium_memory_pool;
    } else if (size <= MEMORY_POOL_LARGE_BLOCK_SIZE) {
        pool = large_memory_pool;
    } else {
        return ERROR_INVALID_PARAMETER;
    }

    if (!pool) {
        return ERROR_NOT_INITIALIZED;
    }

    osStatus_t status = osMemoryPoolFree(pool, ptr);
    return (status == osOK) ? SYSTEM_OK : ERROR_INVALID_PARAMETER;
}

/**
 * @brief Get memory pool usage statistics
 */
SystemError_t rtos_get_memory_stats(MemoryPoolStats_t *stats) {
    if (!stats) {
        return ERROR_INVALID_PARAMETER;
    }

    if (!small_memory_pool || !medium_memory_pool || !large_memory_pool) {
        return ERROR_NOT_INITIALIZED;
    }

    // Small pool stats (default pool for this interface)
    stats->total_blocks = MEMORY_POOL_SMALL_BLOCK_COUNT;
    stats->available_blocks = osMemoryPoolGetSpace(small_memory_pool);
    stats->peak_usage =
        MEMORY_POOL_SMALL_BLOCK_COUNT - stats->available_blocks;
    stats->allocation_failures =
        0; // TODO: Implement allocation failure tracking

    return SYSTEM_OK;
}

/* ==========================================================================
 */
/* Phase 2D: Advanced Integration - Task Utilities Implementation          */
/* ==========================================================================
 */

// Task performance tracking variables
static TaskPerformanceStats_t task_performance[5] = {0}; // 5 tasks
static uint32_t last_idle_time = 0;
static uint32_t system_start_time = 0;

/**
 * @brief Initialize task utilities for Phase 2D advanced integration
 */
static SystemError_t init_task_utilities(void) {
    printf("RTOS Tasks: Initializing task utilities...\r\n");

    system_start_time = xTaskGetTickCount();
    last_idle_time = 0;

    // Initialize performance tracking structures
    memset(task_performance, 0, sizeof(task_performance));

    printf("RTOS Tasks: Task utilities initialized\r\n");
    return SYSTEM_OK;
}

/**
 * @brief Get comprehensive system performance statistics
 */
SystemError_t rtos_get_system_performance(SystemPerformanceStats_t *stats) {
    if (!stats) {
        return ERROR_INVALID_PARAMETER;
    }

    // Get heap information
    stats->free_heap_size = xPortGetFreeHeapSize();
    stats->minimum_free_heap = xPortGetMinimumEverFreeHeapSize();

    // Calculate system CPU usage
    stats->system_cpu_usage = rtos_calculate_cpu_usage();

    // Get memory pool statistics
    SystemError_t result =
        rtos_get_memory_stats((MemoryPoolStats_t *)&stats->small_pool);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Get task-specific performance data
    TaskStatus_t *task_status_array;
    UBaseType_t task_count = uxTaskGetNumberOfTasks();

    task_status_array = pvPortMalloc(task_count * sizeof(TaskStatus_t));
    if (task_status_array != NULL) {
        uxTaskGetSystemState(task_status_array, task_count, NULL);

        // Map task performance data to our structure
        for (UBaseType_t i = 0; i < task_count && i < 5; i++) {
            task_performance[i].stack_high_water_mark =
                task_status_array[i].usStackHighWaterMark;
            task_performance[i].execution_count++;
        }

        // Copy to stats structure
        memcpy(&stats->motor_task, &task_performance[0],
               sizeof(TaskPerformanceStats_t));
        memcpy(&stats->safety_task, &task_performance[1],
               sizeof(TaskPerformanceStats_t));
        memcpy(&stats->can_task, &task_performance[2],
               sizeof(TaskPerformanceStats_t));
        memcpy(&stats->uart_task, &task_performance[3],
               sizeof(TaskPerformanceStats_t));
        memcpy(&stats->telemetry_task, &task_performance[4],
               sizeof(TaskPerformanceStats_t));

        vPortFree(task_status_array);
    }

    return SYSTEM_OK;
}

/**
 * @brief Monitor stack usage for all tasks
 */
SystemError_t rtos_monitor_stack_usage(void) {
    TaskStatus_t *task_status_array;
    UBaseType_t task_count = uxTaskGetNumberOfTasks();
    SystemError_t result = SYSTEM_OK;

    task_status_array = pvPortMalloc(task_count * sizeof(TaskStatus_t));
    if (task_status_array == NULL) {
        return ERROR_MEMORY_ALLOCATION;
    }

    uxTaskGetSystemState(task_status_array, task_count, NULL);

    for (UBaseType_t i = 0; i < task_count; i++) {
        uint32_t stack_usage_percent =
            ((task_status_array[i].usStackHighWaterMark * 100) /
             (task_status_array[i].usStackHighWaterMark +
              (task_status_array[i].uxCurrentPriority * 256))); // Approximate

        if (stack_usage_percent > STACK_MONITOR_CRITICAL_THRESHOLD) {
            printf("RTOS Tasks: CRITICAL - Task %s stack usage: %lu%%\r\n",
                   task_status_array[i].pcTaskName, stack_usage_percent);
            result = ERROR_STACK_OVERFLOW;
        } else if (stack_usage_percent > STACK_MONITOR_WARNING_THRESHOLD) {
            printf("RTOS Tasks: WARNING - Task %s stack usage: %lu%%\r\n",
                   task_status_array[i].pcTaskName, stack_usage_percent);
        }
    }

    vPortFree(task_status_array);
    return result;
}

/**
 * @brief Calculate system CPU usage
 */
uint32_t rtos_calculate_cpu_usage(void) {
    uint32_t current_time = xTaskGetTickCount();
    uint32_t elapsed_time = current_time - system_start_time;

    if (elapsed_time == 0) {
        return 0;
    }

    // Simplified CPU usage calculation
    // In a real implementation, this would use more sophisticated timing
    uint32_t estimated_idle_time = elapsed_time / 10; // Assume 10% overhead
    uint32_t cpu_usage = 100 - ((estimated_idle_time * 100) / elapsed_time);

    return (cpu_usage > 100) ? 100 : cpu_usage;
}

/* ==========================================================================
 */
/* Phase 2D: Updated Initialization Function                               */
/* ==========================================================================
 */

/**
 * @brief Updated rtos_tasks_init to include Phase 2D components
 */
SystemError_t rtos_tasks_init_phase2d(void) {
    SystemError_t result;

    // Initialize base RTOS system first
    result = rtos_tasks_init();
    if (result != SYSTEM_OK) {
        return result;
    }

    // Initialize Phase 2D advanced components
    printf("RTOS Tasks: Initializing Phase 2D advanced integration...\r\n");

    result = init_event_groups();
    if (result != SYSTEM_OK) {
        return result;
    }

    result = init_memory_pools();
    if (result != SYSTEM_OK) {
        return result;
    }

    result = init_task_utilities();
    if (result != SYSTEM_OK) {
        return result;
    }

    printf("RTOS Tasks: Phase 2D advanced integration complete!\r\n");
    printf("RTOS Tasks: Event groups, memory pools, and task utilities "
           "operational\r\n");

    return SYSTEM_OK;
}
