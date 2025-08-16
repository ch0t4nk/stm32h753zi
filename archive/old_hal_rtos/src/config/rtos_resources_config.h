/**
 * @file rtos_resources_config.h
 * @brief FreeRTOS Resources Configuration for STM32H753ZI Stepper Motor
 * Control
 *
 * This file declares all FreeRTOS task handles, queues, semaphores, and
 * mutexes used throughout the system. These are the shared RTOS resources that
 * enable inter-task communication and synchronization.
 *
 * @author STM32H753ZI Development Team
 * @date August 2025
 * @version 1.0
 */

#ifndef RTOS_RESOURCES_CONFIG_H
#define RTOS_RESOURCES_CONFIG_H

#include "FreeRTOS.h"
#include "freertos_config_ssot.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

// =============================================================================
// Task Handle Declarations (Global Access)
// =============================================================================

/** @brief Motor control task handle (high priority, 1kHz control loop) */
extern TaskHandle_t motor_control_task_handle;

/** @brief Safety monitor task handle (highest priority, critical safety) */
extern TaskHandle_t safety_monitor_task_handle;

/** @brief CAN communication task handle (medium priority) */
extern TaskHandle_t can_comm_task_handle;

/** @brief UART communication task handle (low-medium priority) */
extern TaskHandle_t uart_comm_task_handle;

/** @brief Telemetry task handle (low priority, status broadcasting) */
extern TaskHandle_t telemetry_task_handle;

// =============================================================================
// Queue Handle Declarations (Inter-Task Communication)
// =============================================================================

/** @brief Motor command queue for external motor control requests */
extern QueueHandle_t motor_command_queue;

/** @brief CAN message queue for incoming/outgoing CAN messages */
extern QueueHandle_t can_message_queue;

/** @brief UART message queue for debug console and commands */
extern QueueHandle_t uart_message_queue;

/** @brief Telemetry queue for system status broadcasting */
extern QueueHandle_t telemetry_queue;

/** @brief Safety event queue for critical safety notifications */
extern QueueHandle_t safety_event_queue;

// =============================================================================
// Mutex Handle Declarations (Resource Protection)
// =============================================================================

/** @brief SPI bus mutex for thread-safe L6470 driver communication */
extern SemaphoreHandle_t spi_mutex;

/** @brief Motor state mutex for protecting shared motor state variables */
extern SemaphoreHandle_t motor_state_mutex;

/** @brief Error log mutex for thread-safe error logging */
extern SemaphoreHandle_t error_log_mutex;

/** @brief I2C1 bus mutex for AS5600 encoder #1 protection */
extern SemaphoreHandle_t i2c1_mutex;

/** @brief I2C2 bus mutex for AS5600 encoder #2 protection */
extern SemaphoreHandle_t i2c2_mutex;

/** @brief CAN bus mutex for thread-safe CAN-FD communication */
extern SemaphoreHandle_t can_mutex;

// =============================================================================
// Binary Semaphore Declarations (ISR-to-Task Signaling)
// =============================================================================

/** @brief Emergency stop semaphore for immediate safety response */
extern SemaphoreHandle_t emergency_stop_semaphore;

/** @brief Motor timer semaphore for 1kHz control loop timing */
extern SemaphoreHandle_t motor_timer_semaphore;

/** @brief Encoder interrupt semaphore for position update signaling */
extern SemaphoreHandle_t encoder_interrupt_semaphore;

/** @brief CAN receive semaphore for message reception notification */
extern SemaphoreHandle_t can_receive_semaphore;

/** @brief UART receive semaphore for command reception notification */
extern SemaphoreHandle_t uart_receive_semaphore;

// =============================================================================
// Counting Semaphore Declarations (Resource Counting)
// =============================================================================

/** @brief Motor availability semaphore (2 motors available) */
extern SemaphoreHandle_t motor_available_semaphore;

/** @brief Encoder data ready semaphore (dual encoder system) */
extern SemaphoreHandle_t encoder_data_ready_semaphore;

// =============================================================================
// Software Timer Declarations (Periodic Operations)
// =============================================================================

/** @brief Watchdog refresh timer for independent safety monitoring */
extern TimerHandle_t watchdog_refresh_timer;

/** @brief System health check timer for periodic diagnostics */
extern TimerHandle_t health_check_timer;

/** @brief LED status timer for visual system status indication */
extern TimerHandle_t led_status_timer;

// =============================================================================
// Task Function Prototypes (Implementation in separate files)
// =============================================================================

/**
 * @brief Motor control task function - High priority real-time control
 * @param pvParameters Task parameters (unused)
 */
void MotorControlTaskFunction(void *pvParameters);

/**
 * @brief Safety monitor task function - Highest priority safety oversight
 * @param pvParameters Task parameters (unused)
 */
void SafetyMonitorTaskFunction(void *pvParameters);

/**
 * @brief CAN communication task function - Medium priority messaging
 * @param pvParameters Task parameters (unused)
 */
void CommunicationTaskFunction(void *pvParameters);

/**
 * @brief Telemetry task function - Low priority status broadcasting
 * @param pvParameters Task parameters (unused)
 */
void TelemetryTaskFunction(void *pvParameters);

// =============================================================================
// Timer Callback Prototypes (Implementation in separate files)
// =============================================================================

/**
 * @brief Watchdog refresh timer callback
 * @param xTimer Timer handle
 */
void WatchdogRefreshTimerCallback(TimerHandle_t xTimer);

/**
 * @brief System health check timer callback
 * @param xTimer Timer handle
 */
void HealthCheckTimerCallback(TimerHandle_t xTimer);

/**
 * @brief LED status timer callback
 * @param xTimer Timer handle
 */
void LedStatusTimerCallback(TimerHandle_t xTimer);

// =============================================================================
// Resource Initialization Function
// =============================================================================

/**
 * @brief Initialize all FreeRTOS resources (tasks, queues, semaphores, timers)
 *
 * This function creates all RTOS resources in the correct order and validates
 * their creation. Must be called before starting the scheduler.
 *
 * @return SystemError_t SYSTEM_OK if all resources created successfully
 */
SystemError_t RTOS_InitializeResources(void);

/**
 * @brief Validate RTOS resource creation and configuration
 *
 * This function checks that all critical RTOS resources were created
 * successfully and are ready for use.
 *
 * @return SystemError_t SYSTEM_OK if validation passes
 */
SystemError_t RTOS_ValidateResources(void);

// =============================================================================
// RTOS Resource Monitoring (Debug and Safety)
// =============================================================================

/**
 * @brief Get current RTOS resource utilization statistics
 *
 * @param heap_free_bytes Pointer to store free heap size
 * @param min_heap_free_bytes Pointer to store minimum ever free heap
 * @param task_count Pointer to store current task count
 * @return SystemError_t SYSTEM_OK if statistics retrieved successfully
 */
SystemError_t RTOS_GetResourceUtilization(size_t *heap_free_bytes,
                                          size_t *min_heap_free_bytes,
                                          UBaseType_t *task_count);

/**
 * @brief Check for RTOS resource health and integrity
 *
 * Validates that all critical RTOS resources are still functional
 * and have not been corrupted.
 *
 * @return SystemError_t SYSTEM_OK if all resources are healthy
 */
SystemError_t RTOS_CheckResourceHealth(void);

// =============================================================================
// Compile-time Resource Validation
// =============================================================================

// Ensure task handles are properly sized for the system
_Static_assert(sizeof(TaskHandle_t) <= sizeof(void *),
               "TaskHandle_t size incompatible with system");

// Validate queue and semaphore handle sizes
_Static_assert(sizeof(QueueHandle_t) <= sizeof(void *),
               "QueueHandle_t size incompatible with system");
_Static_assert(sizeof(SemaphoreHandle_t) <= sizeof(void *),
               "SemaphoreHandle_t size incompatible with system");

#endif // RTOS_RESOURCES_CONFIG_H
