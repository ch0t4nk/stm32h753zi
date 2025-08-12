/**
 * @file real_time_control.c
 * @brief Real-Time Motor Control Task System
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @details High-frequency real-time control tasks for precise motor control
 *          with deterministic timing and priority-based scheduling
 */

#include "real_time_control.h"
#include "config/motor_config.h"
#include "config/safety_config.h"
#include "hal_abstraction/hal_abstraction.h"
#include "motion_profile.h"
#include "multi_motor_coordinator.h"
#include "position_control.h"
#include "safety/fault_monitor.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_tim.h"
#include <string.h>

// Real-time control system state
static RTControlSystem_t rt_control_system;
static bool rt_system_initialized = false;

// Timer handles for real-time tasks
static TIM_HandleTypeDef htim_control_loop;
static TIM_HandleTypeDef htim_safety_monitor;

/**
 * @brief Initialize real-time control system
 * @return SystemError_t Operation result
 */
SystemError_t rt_control_init(void) {
    // Clear system state
    memset(&rt_control_system, 0, sizeof(rt_control_system));

    // Initialize control tasks
    for (uint8_t i = 0; i < RT_MAX_TASKS; i++) {
        rt_control_system.tasks[i].id = i;
        rt_control_system.tasks[i].enabled = false;
        rt_control_system.tasks[i].state = RT_TASK_IDLE;
        rt_control_system.tasks[i].priority = RT_PRIORITY_LOW;
        rt_control_system.tasks[i].period_us = 0;
        rt_control_system.tasks[i].last_execution = 0;
        rt_control_system.tasks[i].execution_count = 0;
        rt_control_system.tasks[i].max_execution_time = 0;
        rt_control_system.tasks[i].missed_deadlines = 0;
    }

    // Initialize timing statistics
    rt_control_system.timing.system_start_time = HAL_Abstraction_GetTick();
    rt_control_system.timing.total_cycles = 0;
    rt_control_system.timing.max_jitter_us = 0;
    rt_control_system.timing.overrun_count = 0;

    // Initialize performance monitoring
    rt_control_system.performance.cpu_utilization = 0.0f;
    rt_control_system.performance.max_cpu_utilization = 0.0f;
    rt_control_system.performance.memory_usage = 0;
    rt_control_system.performance.stack_usage = 0;

    // Configure control loop timer (1kHz for position control)
    SystemError_t result = configure_control_timer();
    if (result != SYSTEM_OK) {
        return result;
    }

    // Configure safety monitoring timer (10kHz for fault detection)
    result = configure_safety_timer();
    if (result != SYSTEM_OK) {
        return result;
    }

    // Create default tasks
    result = create_default_tasks();
    if (result != SYSTEM_OK) {
        return result;
    }

    rt_system_initialized = true;

    return SYSTEM_OK;
}

/**
 * @brief Start real-time control system
 * @return SystemError_t Operation result
 */
SystemError_t rt_control_start(void) {
    if (!rt_system_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Start control loop timer
    if (HAL_TIM_Base_Start_IT(&htim_control_loop) != HAL_OK) {
        return ERROR_TIMER_START_FAILED;
    }

    // Start safety monitoring timer
    if (HAL_TIM_Base_Start_IT(&htim_safety_monitor) != HAL_OK) {
        HAL_TIM_Base_Stop_IT(&htim_control_loop);
        return ERROR_TIMER_START_FAILED;
    }

    rt_control_system.system_state = RT_SYSTEM_RUNNING;
    rt_control_system.timing.system_start_time = HAL_Abstraction_GetTick();

    return SYSTEM_OK;
}

/**
 * @brief Stop real-time control system
 * @return SystemError_t Operation result
 */
SystemError_t rt_control_stop(void) {
    if (!rt_system_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Stop timers
    HAL_TIM_Base_Stop_IT(&htim_control_loop);
    HAL_TIM_Base_Stop_IT(&htim_safety_monitor);

    rt_control_system.system_state = RT_SYSTEM_STOPPED;

    return SYSTEM_OK;
}

/**
 * @brief Create real-time task
 * @param task_config Task configuration
 * @param task_id Pointer to store assigned task ID
 * @return SystemError_t Operation result
 */
SystemError_t rt_control_create_task(RTTaskConfig_t *task_config,
                                     uint8_t *task_id) {
    if (!rt_system_initialized || task_config == NULL || task_id == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // Find available task slot
    uint8_t free_slot = RT_MAX_TASKS;
    for (uint8_t i = 0; i < RT_MAX_TASKS; i++) {
        if (!rt_control_system.tasks[i].enabled) {
            free_slot = i;
            break;
        }
    }

    if (free_slot >= RT_MAX_TASKS) {
        return ERROR_NO_FREE_TASK_SLOTS;
    }

    // Configure task
    RTTask_t *task = &rt_control_system.tasks[free_slot];
    task->id = free_slot;
    task->enabled = true;
    task->state = RT_TASK_READY;
    task->priority = task_config->priority;
    task->period_us = task_config->period_us;
    task->deadline_us = task_config->deadline_us;
    task->function = task_config->function;
    task->context = task_config->context;
    task->last_execution = 0;
    task->execution_count = 0;
    task->max_execution_time = 0;
    task->missed_deadlines = 0;

    strncpy(task->name, task_config->name, RT_TASK_NAME_MAX - 1);
    task->name[RT_TASK_NAME_MAX - 1] = '\0';

    *task_id = free_slot;

    return SYSTEM_OK;
}

/**
 * @brief Enable/disable real-time task
 * @param task_id Task identifier
 * @param enable True to enable, false to disable
 * @return SystemError_t Operation result
 */
SystemError_t rt_control_enable_task(uint8_t task_id, bool enable) {
    if (!rt_system_initialized || task_id >= RT_MAX_TASKS) {
        return ERROR_INVALID_PARAMETER;
    }

    RTTask_t *task = &rt_control_system.tasks[task_id];

    if (enable && !task->enabled) {
        task->enabled = true;
        task->state = RT_TASK_READY;
        task->last_execution = get_microsecond_timestamp();
    } else if (!enable && task->enabled) {
        task->enabled = false;
        task->state = RT_TASK_IDLE;
    }

    return SYSTEM_OK;
}

/**
 * @brief Get real-time system status
 * @param status Pointer to store status
 * @return SystemError_t Operation result
 */
SystemError_t rt_control_get_status(RTSystemStatus_t *status) {
    if (!rt_system_initialized || status == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    status->system_state = rt_control_system.system_state;
    status->uptime_ms =
        HAL_Abstraction_GetTick() - rt_control_system.timing.system_start_time;
    status->total_cycles = rt_control_system.timing.total_cycles;
    status->overrun_count = rt_control_system.timing.overrun_count;
    status->max_jitter_us = rt_control_system.timing.max_jitter_us;

    // Copy performance data
    memcpy(&status->performance, &rt_control_system.performance,
           sizeof(RTPerformance_t));

    // Count active tasks
    status->active_tasks = 0;
    for (uint8_t i = 0; i < RT_MAX_TASKS; i++) {
        if (rt_control_system.tasks[i].enabled) {
            status->active_tasks++;
        }
    }

    return SYSTEM_OK;
}

/**
 * @brief Get task statistics
 * @param task_id Task identifier
 * @param stats Pointer to store statistics
 * @return SystemError_t Operation result
 */
SystemError_t rt_control_get_task_stats(uint8_t task_id,
                                        RTTaskStats_t *stats) {
    if (!rt_system_initialized || task_id >= RT_MAX_TASKS || stats == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    RTTask_t *task = &rt_control_system.tasks[task_id];

    if (!task->enabled) {
        return ERROR_TASK_NOT_ENABLED;
    }

    stats->execution_count = task->execution_count;
    stats->missed_deadlines = task->missed_deadlines;
    stats->max_execution_time = task->max_execution_time;
    stats->average_execution_time =
        task->total_execution_time /
        (task->execution_count > 0 ? task->execution_count : 1);
    stats->cpu_utilization =
        ((float)task->total_execution_time /
         (float)(get_microsecond_timestamp() -
                 rt_control_system.timing.system_start_time)) *
        100.0f;

    strncpy(stats->name, task->name, RT_TASK_NAME_MAX);

    return SYSTEM_OK;
}

/**
 * @brief Control loop timer interrupt handler
 */
void rt_control_loop_handler(void) {
    if (!rt_system_initialized) {
        return;
    }

    uint32_t start_time = get_microsecond_timestamp();

    // Execute high-priority control tasks
    execute_priority_tasks(RT_PRIORITY_HIGH);
    execute_priority_tasks(RT_PRIORITY_NORMAL);

    // Update timing statistics
    uint32_t execution_time = get_microsecond_timestamp() - start_time;
    update_timing_statistics(execution_time);

    rt_control_system.timing.total_cycles++;
}

/**
 * @brief Safety monitoring timer interrupt handler
 */
void rt_safety_monitor_handler(void) {
    if (!rt_system_initialized) {
        return;
    }

    // Execute critical safety tasks
    execute_priority_tasks(RT_PRIORITY_CRITICAL);

    // Update performance monitoring
    update_performance_monitoring();
}

/**
 * @brief Configure control loop timer
 */
static SystemError_t configure_control_timer(void) {
    // Configure TIM2 for 1kHz control loop (1ms period)
    htim_control_loop.Instance = TIM2;
    htim_control_loop.Init.Prescaler =
        (SystemCoreClock / 1000000) - 1; // 1MHz timer clock
    htim_control_loop.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim_control_loop.Init.Period =
        (uint32_t)MOTOR_CONTROL_LOOP_PERIOD_TICKS; // 1ms period
    htim_control_loop.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim_control_loop.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim_control_loop) != HAL_OK) {
        return ERROR_TIMER_CONFIG_FAILED;
    }

    return SYSTEM_OK;
}

/**
 * @brief Configure safety monitoring timer
 */
static SystemError_t configure_safety_timer(void) {
    // Configure TIM3 for 10kHz safety monitoring (100us period)
    htim_safety_monitor.Instance = TIM3;
    htim_safety_monitor.Init.Prescaler =
        (SystemCoreClock / 1000000) - 1; // 1MHz timer clock
    htim_safety_monitor.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim_safety_monitor.Init.Period = 100 - 1; // 100us period
    htim_safety_monitor.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim_safety_monitor.Init.AutoReloadPreload =
        TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim_safety_monitor) != HAL_OK) {
        return ERROR_TIMER_CONFIG_FAILED;
    }

    return SYSTEM_OK;
}

/**
 * @brief Create default real-time tasks
 */
static SystemError_t create_default_tasks(void) {
    SystemError_t result;
    uint8_t task_id;

    // Position control task (1kHz)
    RTTaskConfig_t pos_control_config = {.name = "PositionControl",
                                         .priority = RT_PRIORITY_HIGH,
                                         .period_us = 1000,
                                         .deadline_us = 800,
                                         .function = position_control_task,
                                         .context = NULL};
    result = rt_control_create_task(&pos_control_config, &task_id);
    if (result != SYSTEM_OK)
        return result;

    // Motion profile update task (1kHz)
    RTTaskConfig_t motion_profile_config = {.name = "MotionProfile",
                                            .priority = RT_PRIORITY_HIGH,
                                            .period_us = 1000,
                                            .deadline_us = 500,
                                            .function = motion_profile_task,
                                            .context = NULL};
    result = rt_control_create_task(&motion_profile_config, &task_id);
    if (result != SYSTEM_OK)
        return result;

    // Multi-motor coordination task (500Hz)
    RTTaskConfig_t coordination_config = {.name = "Coordination",
                                          .priority = RT_PRIORITY_NORMAL,
                                          .period_us = 2000,
                                          .deadline_us = 1500,
                                          .function = coordination_task,
                                          .context = NULL};
    result = rt_control_create_task(&coordination_config, &task_id);
    if (result != SYSTEM_OK)
        return result;

    // Safety monitoring task (10kHz)
    RTTaskConfig_t safety_config = {.name = "SafetyMonitor",
                                    .priority = RT_PRIORITY_CRITICAL,
                                    .period_us = 100,
                                    .deadline_us = 50,
                                    .function = safety_monitor_task,
                                    .context = NULL};
    result = rt_control_create_task(&safety_config, &task_id);
    if (result != SYSTEM_OK)
        return result;

    return SYSTEM_OK;
}

/**
 * @brief Execute tasks of specified priority
 */
static void execute_priority_tasks(RTTaskPriority_t priority) {
    uint32_t current_time = get_microsecond_timestamp();

    for (uint8_t i = 0; i < RT_MAX_TASKS; i++) {
        RTTask_t *task = &rt_control_system.tasks[i];

        if (!task->enabled || task->priority != priority) {
            continue;
        }

        // Check if task is ready to execute
        if ((current_time - task->last_execution) >= task->period_us) {
            execute_task(task, current_time);
        }
    }
}

/**
 * @brief Execute individual task
 */
static void execute_task(RTTask_t *task, uint32_t current_time) {
    // Check for deadline miss
    uint32_t elapsed = current_time - task->last_execution;
    if (elapsed > task->deadline_us && task->execution_count > 0) {
        task->missed_deadlines++;
    }

    // Record execution start
    uint32_t execution_start = get_microsecond_timestamp();
    task->state = RT_TASK_RUNNING;

    // Execute task function
    if (task->function != NULL) {
        task->function(task->context);
    }

    // Record execution time
    uint32_t execution_time = get_microsecond_timestamp() - execution_start;

    task->last_execution = current_time;
    task->execution_count++;
    task->total_execution_time += execution_time;
    task->state = RT_TASK_READY;

    // Update maximum execution time
    if (execution_time > task->max_execution_time) {
        task->max_execution_time = execution_time;
    }
}

/**
 * @brief Update timing statistics
 */
static void update_timing_statistics(uint32_t execution_time) {
    // Check for timing overrun
    if (execution_time > RT_CONTROL_LOOP_PERIOD_US) {
        rt_control_system.timing.overrun_count++;
    }

    // Calculate jitter
    static uint32_t last_execution_time = 0;
    if (last_execution_time > 0) {
        uint32_t jitter = (execution_time > last_execution_time)
                              ? (execution_time - last_execution_time)
                              : (last_execution_time - execution_time);

        if (jitter > rt_control_system.timing.max_jitter_us) {
            rt_control_system.timing.max_jitter_us = jitter;
        }
    }
    last_execution_time = execution_time;
}

/**
 * @brief Update performance monitoring
 */
static void update_performance_monitoring(void) {
    static uint32_t last_update = 0;
    uint32_t current_time = get_microsecond_timestamp();

    // Update every 100ms
    if ((current_time - last_update) >= 100000) {
        // Calculate CPU utilization
        uint32_t total_execution_time = 0;
        for (uint8_t i = 0; i < RT_MAX_TASKS; i++) {
            if (rt_control_system.tasks[i].enabled) {
                total_execution_time +=
                    rt_control_system.tasks[i].total_execution_time;
            }
        }

        uint32_t elapsed_time =
            current_time - rt_control_system.timing.system_start_time;
        rt_control_system.performance.cpu_utilization =
            ((float)total_execution_time / (float)elapsed_time) * 100.0f;

        if (rt_control_system.performance.cpu_utilization >
            rt_control_system.performance.max_cpu_utilization) {
            rt_control_system.performance.max_cpu_utilization =
                rt_control_system.performance.cpu_utilization;
        }

        // Update memory usage (simplified)
        rt_control_system.performance.memory_usage = sizeof(RTControlSystem_t);

        last_update = current_time;
    }
}

/**
 * @brief Get microsecond timestamp
 */
static uint32_t get_microsecond_timestamp(void) {
    // Use DWT cycle counter for high precision timing
    return DWT->CYCCNT / (SystemCoreClock / 1000000);
}

/**
 * @brief Position control real-time task
 */
static void position_control_task(void *context) {
    (void)context; // Unused parameter

    // Update position control for all motors
    for (uint8_t motor_id = 0; motor_id < MAX_MOTORS; motor_id++) {
        if (position_control_is_enabled(motor_id)) {
            position_control_update(
                motor_id,
                (float)MOTOR_POSITION_CONTROL_TIMESTEP_MS); // 1ms time step
        }
    }
}

/**
 * @brief Motion profile real-time task
 */
static void motion_profile_task(void *context) {
    (void)context; // Unused parameter

    // Update motion profiles for all motors
    for (uint8_t motor_id = 0; motor_id < MAX_MOTORS; motor_id++) {
        if (motion_profile_is_active(motor_id)) {
            // TODO: Get active profile and execute it properly
            // For now, skip execution until motion_profile_update function is
            // implemented motion_profile_execute requires MotionProfile_t
            // *profile parameter
            (void)motor_id; // Suppress unused variable warning
        }
    }
}

/**
 * @brief Multi-motor coordination real-time task
 */
static void coordination_task(void *context) {
    (void)context; // Unused parameter

    // Update multi-motor coordination
    multi_motor_update(
        (float)MOTOR_MULTI_MOTOR_TIMESTEP_MS); // 2ms time step (500Hz)
}

/**
 * @brief Safety monitoring real-time task
 */
static void safety_monitor_task(void *context) {
    (void)context; // Unused parameter

    // Perform safety checks
    fault_monitor_check();

    // Check system performance
    if (rt_control_system.performance.cpu_utilization >
        CPU_UTILIZATION_WARNING_THRESHOLD) {
        fault_monitor_record_system_fault(
            SYSTEM_FAULT_RTOS_ERROR, FAULT_SEVERITY_CRITICAL,
            rt_control_system.performance.cpu_utilization);
    }

    if (rt_control_system.timing.overrun_count > MAX_ALLOWED_OVERRUNS) {
        fault_monitor_record_system_fault(
            SYSTEM_FAULT_RTOS_ERROR, FAULT_SEVERITY_CRITICAL,
            rt_control_system.timing.overrun_count);
    }
}

/**
 * @brief Timer interrupt callback for control loop
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == &htim_control_loop) {
        rt_control_loop_handler();
    } else if (htim == &htim_safety_monitor) {
        rt_safety_monitor_handler();
    }
}
