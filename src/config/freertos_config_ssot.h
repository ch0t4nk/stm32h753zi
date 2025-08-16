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

#include <stdint.h>

// =============================================================================
// FreeRTOS Core Configuration (SSOT)
// =============================================================================

/** @brief Total heap size allocated to FreeRTOS (32KB for stable operation)
 */
#define RTOS_HEAP_SIZE_BYTES (32 * 1024)

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

// ...existing code...

#endif // FREERTOS_CONFIG_SSOT_H
