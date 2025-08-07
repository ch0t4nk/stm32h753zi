/**
 * @file rtos_tasks.h
 * @brief FreeRTOS Task Implementations for STM32H753ZI Motor Control System
 * @author STM32H753ZI Project Team
 * @date August 2025
 * @version 1.0
 *
 * This file defines the FreeRTOS task functions that integrate existing
 * application functionality into a multi-tasking real-time system.
 * All tasks use SSOT configuration parameters and follow safety-critical
 * design principles.
 *
 * @note Phase 2: RTOS Tasks Integration
 * Integrates existing functions:
 * - motor_controller_update() -> MotorControlTask
 * - safety_system_task() -> SafetyMonitorTask
 * - CAN/UART communication -> CommunicationTasks
 * - Telemetry and monitoring -> TelemetryTask
 */

#ifndef RTOS_TASKS_H
#define RTOS_TASKS_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
 */
/* Includes */
/* ==========================================================================
 */

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "common/data_types.h"
#include "common/error_codes.h"
#include "communication/comm_protocol.h"
#include "config/freertos_config_ssot.h"
#include "config/rtos_resources_config.h"

/* ==========================================================================
 */
/* Task Function Declarations                                                */
/* ==========================================================================
 */

/**
 * @brief High-priority motor control task
 *
 * Executes motor_controller_update() at 1kHz (1ms period) for precise
 * real-time control. Integrates with existing motor control infrastructure
 * while providing deterministic timing through FreeRTOS scheduler.
 *
 * @param argument Task argument (unused)
 * @note Priority: 3 (MOTOR_CONTROL_TASK_PRIORITY)
 * @note Stack: 2KB (MOTOR_CONTROL_TASK_STACK_SIZE)
 * @note Period: 1ms (MOTOR_CONTROL_PERIOD_MS)
 */
void MotorControlTask(void *argument);

/**
 * @brief Highest-priority safety monitoring task
 *
 * Executes safety_system_task() at 500Hz (2ms period) for continuous
 * safety monitoring. Handles emergency stop conditions and system faults
 * with immediate response capability.
 *
 * @param argument Task argument (unused)
 * @note Priority: 4 (SAFETY_MONITOR_TASK_PRIORITY) - HIGHEST
 * @note Stack: 1.5KB (SAFETY_MONITOR_TASK_STACK_SIZE)
 * @note Period: 2ms (SAFETY_CHECK_PERIOD_MS)
 */
void SafetyMonitorTask(void *argument);

/**
 * @brief CAN communication task
 *
 * Handles CAN bus communication at 100Hz (10ms period) for motor commands
 * and status reporting. Processes incoming CAN messages and sends responses.
 *
 * @param argument Task argument (unused)
 * @note Priority: 2 (CAN_COMM_TASK_PRIORITY)
 * @note Stack: 1KB (CAN_COMM_TASK_STACK_SIZE)
 * @note Period: 10ms (CAN_POLL_PERIOD_MS)
 */
void CANCommTask(void *argument);

/**
 * @brief UART communication task
 *
 * Handles UART communication at 20Hz (50ms period) for debug output
 * and command interface. Lower priority than CAN for resource allocation.
 *
 * @param argument Task argument (unused)
 * @note Priority: 1 (UART_COMM_TASK_PRIORITY)
 * @note Stack: 1KB (UART_COMM_TASK_STACK_SIZE)
 * @note Period: 50ms (UART_POLL_PERIOD_MS)
 */
void UARTCommTask(void *argument);

/**
 * @brief Telemetry and monitoring task
 *
 * Collects system telemetry at 10Hz (100ms period) for monitoring
 * motor status, system health, and performance metrics.
 *
 * @param argument Task argument (unused)
 * @note Priority: 1 (TELEMETRY_TASK_PRIORITY)
 * @note Stack: 1KB (TELEMETRY_TASK_STACK_SIZE)
 * @note Period: 100ms (TELEMETRY_PERIOD_MS)
 */
void TelemetryTask(void *argument);

/**
 * @brief Timer service callback
 *
 * Software timer callback for periodic operations that don't require
 * dedicated tasks. Handles watchdog refresh and system maintenance.
 *
 * @param xTimer Timer handle
 * @note Priority: 2 (TIMER_SERVICE_TASK_PRIORITY)
 * @note Period: 500ms (WATCHDOG_REFRESH_PERIOD_MS)
 */
void TimerServiceCallback(TimerHandle_t xTimer);

/* ==========================================================================
 */
/* Task Management Functions                                                 */
/* ==========================================================================
 */

/**
 * @brief Initialize all RTOS tasks and resources
 *
 * Creates all tasks, queues, semaphores, and timers according to SSOT
 * configuration. Must be called before starting the scheduler.
 *
 * @return SYSTEM_OK on success, error code on failure
 * @note Call this function in main() before osKernelStart()
 */
SystemError_t rtos_tasks_init(void);

/**
 * @brief Start all RTOS tasks
 *
 * Enables all tasks to begin execution. Tasks will start running
 * according to their priorities once the scheduler starts.
 *
 * @return SYSTEM_OK on success, error code on failure
 * @note Called automatically by rtos_tasks_init()
 */
SystemError_t rtos_tasks_start(void);

/**
 * @brief Emergency stop all tasks
 *
 * Safely suspends all non-critical tasks during emergency conditions.
 * Safety monitor task continues running for fault handling.
 *
 * @return SYSTEM_OK on success, error code on failure
 */
SystemError_t rtos_tasks_emergency_stop(void);

/**
 * @brief Resume normal task operation
 *
 * Resumes all tasks after emergency stop or system fault recovery.
 *
 * @return SYSTEM_OK on success, error code on failure
 */
SystemError_t rtos_tasks_resume(void);

/**
 * @brief Get task performance statistics
 *
 * Retrieves runtime statistics for all tasks including CPU usage,
 * stack high water marks, and execution times.
 *
 * @param stats Pointer to task statistics structure
 * @return SYSTEM_OK on success, error code on failure
 */
SystemError_t rtos_tasks_get_stats(TaskStats_t *stats);

/* ==========================================================================
 */
/* Inter-Task Communication                                                  */
/* ==========================================================================
 */

/**
 * @brief Send motor command to motor control task
 *
 * Thread-safe function to send motor commands from any task or ISR.
 *
 * @param command Pointer to motor command structure
 * @param timeout_ms Timeout in milliseconds (0 = no wait)
 * @return SYSTEM_OK on success, error code on failure
 */
SystemError_t rtos_send_motor_command(const MotorCommand_t *command,
                                      uint32_t timeout_ms);

/**
 * @brief Send safety event to safety monitor task
 *
 * Thread-safe function to report safety events from any task or ISR.
 *
 * @param event Pointer to safety event structure
 * @param timeout_ms Timeout in milliseconds (0 = no wait)
 * @return SYSTEM_OK on success, error code on failure
 */
SystemError_t rtos_send_safety_event(const SafetyEvent_t *event,
                                     uint32_t timeout_ms);

/**
 * @brief Send telemetry data
 *
 * Thread-safe function to send telemetry data from any task.
 *
 * @param data Pointer to telemetry data structure
 * @param timeout_ms Timeout in milliseconds (0 = no wait)
 * @return SYSTEM_OK on success, error code on failure
 */
SystemError_t rtos_send_telemetry(const TelemetryData_t *data,
                                  uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif // RTOS_TASKS_H
