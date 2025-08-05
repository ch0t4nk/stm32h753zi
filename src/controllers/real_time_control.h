/**
 * @file real_time_control.h
 * @brief Real-Time Motor Control Task System - Header
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @details High-frequency real-time control tasks for precise motor control
 *          with deterministic timing and priority-based scheduling
 */

#ifndef REAL_TIME_CONTROL_H
#define REAL_TIME_CONTROL_H

#include "common/data_types.h"
#include "common/error_codes.h"
#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

// Real-time control system constants
#define RT_MAX_TASKS 8                 // Maximum number of RT tasks
#define RT_TASK_NAME_MAX 16            // Maximum task name length
#define RT_CONTROL_LOOP_PERIOD_US 1000 // Control loop period (1kHz)
#define RT_SAFETY_PERIOD_US 100        // Safety monitoring period (10kHz)

// Performance monitoring thresholds
#define CPU_UTILIZATION_WARNING_THRESHOLD 80.0f // CPU utilization warning (%)
#define MAX_ALLOWED_OVERRUNS 10                 // Maximum timing overruns

// Error codes specific to real-time control
#define ERROR_TIMER_CONFIG_FAILED 0x6100 // Timer configuration failed
#define ERROR_TIMER_START_FAILED 0x6101  // Timer start failed
#define ERROR_NO_FREE_TASK_SLOTS 0x6102  // No free task slots available
#define ERROR_TASK_NOT_ENABLED 0x6103    // Task not enabled
#define ERROR_DEADLINE_MISSED 0x6104     // Task deadline missed
#define ERROR_CPU_OVERLOAD 0x6105        // CPU overload detected

/**
 * @brief Real-time task priority levels
 */
typedef enum {
    RT_PRIORITY_CRITICAL = 0, ///< Critical priority (safety tasks)
    RT_PRIORITY_HIGH,         ///< High priority (control loops)
    RT_PRIORITY_NORMAL,       ///< Normal priority (coordination)
    RT_PRIORITY_LOW           ///< Low priority (logging, diagnostics)
} RTTaskPriority_t;

/**
 * @brief Real-time task states
 */
typedef enum {
    RT_TASK_IDLE = 0, ///< Task is idle/disabled
    RT_TASK_READY,    ///< Task is ready to run
    RT_TASK_RUNNING,  ///< Task is currently executing
    RT_TASK_BLOCKED   ///< Task is blocked waiting for resource
} RTTaskState_t;

/**
 * @brief Real-time system states
 */
typedef enum {
    RT_SYSTEM_STOPPED = 0, ///< System is stopped
    RT_SYSTEM_RUNNING,     ///< System is running normally
    RT_SYSTEM_ERROR        ///< System is in error state
} RTSystemState_t;

/**
 * @brief Real-time task function pointer
 */
typedef void (*RTTaskFunction_t)(void *context);

/**
 * @brief Real-time task configuration structure
 */
typedef struct {
    char name[RT_TASK_NAME_MAX]; ///< Task name
    RTTaskPriority_t priority;   ///< Task priority
    uint32_t period_us;          ///< Task period in microseconds
    uint32_t deadline_us;        ///< Task deadline in microseconds
    RTTaskFunction_t function;   ///< Task function pointer
    void *context;               ///< Task context data
} RTTaskConfig_t;

/**
 * @brief Real-time task control block
 */
typedef struct {
    uint8_t id;                  ///< Task identifier
    char name[RT_TASK_NAME_MAX]; ///< Task name
    bool enabled;                ///< Task enabled flag
    RTTaskState_t state;         ///< Current task state
    RTTaskPriority_t priority;   ///< Task priority
    uint32_t period_us;          ///< Task period in microseconds
    uint32_t deadline_us;        ///< Task deadline in microseconds
    RTTaskFunction_t function;   ///< Task function pointer
    void *context;               ///< Task context data

    // Execution statistics
    uint32_t last_execution;       ///< Last execution timestamp
    uint32_t execution_count;      ///< Total execution count
    uint32_t total_execution_time; ///< Total execution time
    uint32_t max_execution_time;   ///< Maximum execution time
    uint32_t missed_deadlines;     ///< Number of missed deadlines
} RTTask_t;

/**
 * @brief Real-time system timing statistics
 */
typedef struct {
    uint32_t system_start_time; ///< System start timestamp
    uint32_t total_cycles;      ///< Total control cycles executed
    uint32_t max_jitter_us;     ///< Maximum timing jitter
    uint32_t overrun_count;     ///< Number of timing overruns
} RTTiming_t;

/**
 * @brief Real-time system performance monitoring
 */
typedef struct {
    float cpu_utilization;     ///< Current CPU utilization (%)
    float max_cpu_utilization; ///< Maximum CPU utilization (%)
    uint32_t memory_usage;     ///< Memory usage in bytes
    uint32_t stack_usage;      ///< Stack usage in bytes
} RTPerformance_t;

/**
 * @brief Real-time control system structure
 */
typedef struct {
    RTSystemState_t system_state; ///< Current system state
    RTTask_t tasks[RT_MAX_TASKS]; ///< Real-time tasks array
    RTTiming_t timing;            ///< Timing statistics
    RTPerformance_t performance;  ///< Performance monitoring
} RTControlSystem_t;

/**
 * @brief Real-time system status structure
 */
typedef struct {
    RTSystemState_t system_state; ///< Current system state
    uint32_t uptime_ms;           ///< System uptime in milliseconds
    uint32_t total_cycles;        ///< Total control cycles
    uint32_t overrun_count;       ///< Timing overrun count
    uint32_t max_jitter_us;       ///< Maximum jitter
    uint8_t active_tasks;         ///< Number of active tasks
    RTPerformance_t performance;  ///< Performance data
} RTSystemStatus_t;

/**
 * @brief Real-time task statistics structure
 */
typedef struct {
    char name[RT_TASK_NAME_MAX];     ///< Task name
    uint32_t execution_count;        ///< Total executions
    uint32_t missed_deadlines;       ///< Missed deadlines
    uint32_t max_execution_time;     ///< Maximum execution time
    uint32_t average_execution_time; ///< Average execution time
    float cpu_utilization;           ///< Task CPU utilization (%)
} RTTaskStats_t;

// Core real-time control functions
SystemError_t rt_control_init(void);
SystemError_t rt_control_start(void);
SystemError_t rt_control_stop(void);

// Task management functions
SystemError_t rt_control_create_task(RTTaskConfig_t *task_config,
                                     uint8_t *task_id);
SystemError_t rt_control_enable_task(uint8_t task_id, bool enable);

// Status and monitoring functions
SystemError_t rt_control_get_status(RTSystemStatus_t *status);
SystemError_t rt_control_get_task_stats(uint8_t task_id, RTTaskStats_t *stats);

// Interrupt handlers
void rt_control_loop_handler(void);
void rt_safety_monitor_handler(void);

// Utility functions
bool rt_control_is_running(void);
uint32_t rt_control_get_cycle_count(void);
float rt_control_get_cpu_utilization(void);

// Internal functions (declared for testing)
static SystemError_t configure_control_timer(void);
static SystemError_t configure_safety_timer(void);
static SystemError_t create_default_tasks(void);
static void execute_priority_tasks(RTTaskPriority_t priority);
static void execute_task(RTTask_t *task, uint32_t current_time);
static void update_timing_statistics(uint32_t execution_time);
static void update_performance_monitoring(void);
static uint32_t get_microsecond_timestamp(void);

// Default task functions
static void position_control_task(void *context);
static void motion_profile_task(void *context);
static void coordination_task(void *context);
static void safety_monitor_task(void *context);

// HAL callback function
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

// External dependencies
extern void DWT_Init(void); // Enable DWT cycle counter for timing

#endif // REAL_TIME_CONTROL_H
