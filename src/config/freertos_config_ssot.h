/**
 * @file freertos_config_ssot.h
 * @brief FreeRTOS Single Source of Truth Configuration for STM32H753ZI Stepper
 * Motor Control
 *
 * This file defines all FreeRTOS configuration parameters in a centralized
 * location following the project's SSOT principles. All RTOS-related code must
 * reference these definitions rather than hardcoding values.
 *
 * @author STM32H753ZI Development Team
 * @date August 2025
 * @version 1.0
 */

#ifndef FREERTOS_CONFIG_SSOT_H
#define FREERTOS_CONFIG_SSOT_H

// =============================================================================
// FreeRTOS Core Configuration (SSOT)
// =============================================================================

/** @brief Total heap size allocated to FreeRTOS (8KB conservative allocation)
 */
#define RTOS_HEAP_SIZE_BYTES (8 * 1024)

/** @brief Maximum number of task priority levels (0=Idle, 4=Highest) */
#define RTOS_MAX_TASK_PRIORITIES (5)

/** @brief FreeRTOS tick rate in Hz (1ms precision for motor control) */
#define RTOS_TICK_RATE_HZ (1000)

/** @brief Minimal stack size for tasks in words (128 words = 512 bytes) */
#define RTOS_MINIMAL_STACK_SIZE (128)

/** @brief Total heap size (same as RTOS_HEAP_SIZE_BYTES for consistency) */
#define RTOS_TOTAL_HEAP_SIZE RTOS_HEAP_SIZE_BYTES

/** @brief Heap management algorithm (Heap_4 with coalescence) */
#define RTOS_HEAP_ALGORITHM (4)

// =============================================================================
// Task Stack Sizes (SSOT) - All values in words (4 bytes each)
// =============================================================================

/** @brief Motor control task stack size (512 words = 2KB) */
#define MOTOR_CONTROL_TASK_STACK_SIZE (512)

/** @brief Safety monitor task stack size (384 words = 1.5KB) */
#define SAFETY_MONITOR_TASK_STACK_SIZE (384)

/** @brief CAN communication task stack size (256 words = 1KB) */
#define CAN_COMM_TASK_STACK_SIZE (256)

/** @brief UART communication task stack size (256 words = 1KB) */
#define UART_COMM_TASK_STACK_SIZE (256)

/** @brief Telemetry task stack size (256 words = 1KB) */
#define TELEMETRY_TASK_STACK_SIZE (256)

/** @brief Timer service task stack size (128 words = 512 bytes) */
#define TIMER_SERVICE_TASK_STACK_SIZE (128)

/** @brief Idle task stack size (128 words = 512 bytes) */
#define IDLE_TASK_STACK_SIZE (128)

// =============================================================================
// Task Priorities (SSOT) - Higher number = Higher priority
// =============================================================================

/** @brief Safety monitor task priority (highest - critical safety functions)
 */
#define SAFETY_MONITOR_TASK_PRIORITY (4)

/** @brief Motor control task priority (high - real-time control loop) */
#define MOTOR_CONTROL_TASK_PRIORITY (3)

/** @brief Timer service task priority (medium-high) */
#define TIMER_SERVICE_TASK_PRIORITY (2)

/** @brief CAN communication task priority (medium) */
#define CAN_COMM_TASK_PRIORITY (2)

/** @brief UART communication task priority (low-medium) */
#define UART_COMM_TASK_PRIORITY (1)

/** @brief Telemetry task priority (low) */
#define TELEMETRY_TASK_PRIORITY (1)

/** @brief Idle task priority (lowest - system idle) */
#define IDLE_TASK_PRIORITY (0)

// =============================================================================
// Queue Sizes (SSOT) - Number of items each queue can hold
// =============================================================================

/** @brief Motor command queue size (8 command entries) */
#define MOTOR_COMMAND_QUEUE_SIZE (8)

/** @brief CAN message queue size (16 CAN messages) */
#define CAN_MESSAGE_QUEUE_SIZE (16)

/** @brief UART message queue size (8 UART messages) */
#define UART_MESSAGE_QUEUE_SIZE (8)

/** @brief Telemetry queue size (4 telemetry entries) */
#define TELEMETRY_QUEUE_SIZE (4)

/** @brief Safety event queue size (4 safety events) */
#define SAFETY_EVENT_QUEUE_SIZE (4)

// =============================================================================
// Timing Configuration (SSOT) - All values in milliseconds
// =============================================================================

/** @brief Motor control task period (1ms = 1kHz control loop) */
#define MOTOR_CONTROL_PERIOD_MS (1)

/** @brief Safety monitoring period (2ms = 500Hz safety checks) */
#define SAFETY_CHECK_PERIOD_MS (2)

/** @brief CAN communication polling period (10ms = 100Hz) */
#define CAN_POLL_PERIOD_MS (10)

/** @brief UART communication polling period (50ms = 20Hz) */
#define UART_POLL_PERIOD_MS (50)

/** @brief Telemetry broadcasting period (100ms = 10Hz) */
#define TELEMETRY_PERIOD_MS (100)

/** @brief Watchdog refresh period (500ms) */
#define WATCHDOG_REFRESH_PERIOD_MS (500)

// =============================================================================
// Interrupt Priorities (SSOT) - For FreeRTOS compatibility
// =============================================================================

/** @brief Maximum syscall interrupt priority (5 = highest FreeRTOS priority)
 */
#define RTOS_MAX_SYSCALL_INTERRUPT_PRIORITY (5)

/** @brief Kernel interrupt priority (15 = lowest priority) */
#define RTOS_KERNEL_INTERRUPT_PRIORITY (15)

/** @brief Emergency stop interrupt priority (0 = highest hardware priority) */
#define EMERGENCY_STOP_INTERRUPT_PRIORITY (0)

/** @brief Motor timer interrupt priority (5 = FreeRTOS compatible) */
#define MOTOR_TIMER_INTERRUPT_PRIORITY (5)

/** @brief Encoder interrupt priority (6 = lower than motor timer) */
#define ENCODER_INTERRUPT_PRIORITY (6)

/** @brief CAN interrupt priority (7 = communication priority) */
#define CAN_INTERRUPT_PRIORITY (7)

/** @brief UART interrupt priority (8 = lowest communication priority) */
#define UART_INTERRUPT_PRIORITY (8)

// =============================================================================
// Memory Management (SSOT)
// =============================================================================

/** @brief Maximum task name length including null terminator */
#define RTOS_MAX_TASK_NAME_LENGTH (16)

/** @brief Queue registry size for debugging */
#define RTOS_QUEUE_REGISTRY_SIZE (8)

/** @brief Stack overflow check level (2 = comprehensive checking) */
#define RTOS_STACK_OVERFLOW_CHECK (2)

// =============================================================================
// Safety and Performance Configuration (SSOT)
// =============================================================================

/** @brief Enable/disable task runtime statistics (0 = disabled for
 * performance) */
#define RTOS_ENABLE_RUNTIME_STATS (0)

/** @brief Enable/disable trace facility (1 = enabled for debugging) */
#define RTOS_ENABLE_TRACE_FACILITY (1)

/** @brief Enable/disable malloc failed hook (1 = enabled for safety) */
#define RTOS_ENABLE_MALLOC_FAILED_HOOK (1)

/** @brief Enable/disable stack high water mark tracking (1 = enabled) */
#define RTOS_ENABLE_STACK_MONITORING (1)

// =============================================================================
// Compile-time Validation (SSOT Enforcement)
// =============================================================================

// Validate task priorities are within valid range
_Static_assert(SAFETY_MONITOR_TASK_PRIORITY < RTOS_MAX_TASK_PRIORITIES,
               "Safety monitor task priority exceeds maximum");
_Static_assert(MOTOR_CONTROL_TASK_PRIORITY < RTOS_MAX_TASK_PRIORITIES,
               "Motor control task priority exceeds maximum");
_Static_assert(IDLE_TASK_PRIORITY == 0, "Idle task must have priority 0");

// Validate heap size is reasonable for STM32H753ZI (1MB RAM)
_Static_assert(RTOS_HEAP_SIZE_BYTES <= (64 * 1024),
               "RTOS heap size too large - exceeds safe limit");
_Static_assert(RTOS_HEAP_SIZE_BYTES >= (4 * 1024),
               "RTOS heap size too small - insufficient for tasks");

// Validate timing periods are reasonable
_Static_assert(MOTOR_CONTROL_PERIOD_MS >= 1,
               "Motor control period too fast - may cause system overload");
_Static_assert(SAFETY_CHECK_PERIOD_MS <= 10,
               "Safety check period too slow - may compromise safety");

// Validate queue sizes are reasonable
_Static_assert(MOTOR_COMMAND_QUEUE_SIZE >= 4,
               "Motor command queue too small - may cause command loss");
_Static_assert(SAFETY_EVENT_QUEUE_SIZE >= 2,
               "Safety event queue too small - may lose critical events");

#endif // FREERTOS_CONFIG_SSOT_H
