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

// Event Groups Configuration
/** @brief Maximum number of event group bits available (24 bits in FreeRTOS)
 */
#define MAX_EVENT_GROUP_BITS (24)

/** @brief System Event Group bit definitions for multi-task synchronization */
#define SYSTEM_EVENT_MOTORS_INITIALIZED (1UL << 0)   // Bit 0: Motors ready
#define SYSTEM_EVENT_SAFETY_CHECKS_PASSED (1UL << 1) // Bit 1: Safety validated
#define SYSTEM_EVENT_COMM_READY (1UL << 2) // Bit 2: Communication ready
#define SYSTEM_EVENT_SENSORS_CALIBRATED                                       \
    (1UL << 3) // Bit 3: Sensor calibration complete
#define SYSTEM_EVENT_STARTUP_COMPLETE                                         \
    (1UL << 4) // Bit 4: System startup complete
#define SYSTEM_EVENT_EMERGENCY_STOP                                           \
    (1UL << 5) // Bit 5: Emergency stop triggered
#define SYSTEM_EVENT_MOTION_COMPLETE                                          \
    (1UL << 6) // Bit 6: Motion sequence complete
#define SYSTEM_EVENT_HOMING_COMPLETE                                          \
    (1UL << 7) // Bit 7: Homing operation complete

/** @brief Communication Event Group bit definitions for protocol coordination
 */
#define COMM_EVENT_CAN_MESSAGE_READY (1UL << 0) // Bit 0: CAN message pending
#define COMM_EVENT_UART_TRANSMIT_COMPLETE (1UL << 1) // Bit 1: UART TX complete
#define COMM_EVENT_UART_RECEIVE_READY (1UL << 2) // Bit 2: UART RX data ready
#define COMM_EVENT_TELEMETRY_REQUEST                                          \
    (1UL << 3) // Bit 3: Telemetry data requested
#define COMM_EVENT_STATUS_UPDATE_REQUIRED                                     \
    (1UL << 4) // Bit 4: Status update needed
#define COMM_EVENT_ERROR_REPORT_PENDING                                       \
    (1UL << 5) // Bit 5: Error report pending

/** @brief Motion Event Group bit definitions for coordinated motion control */
#define MOTION_EVENT_MOTOR1_TARGET_REACHED                                    \
    (1UL << 0) // Bit 0: Motor 1 at target
#define MOTION_EVENT_MOTOR2_TARGET_REACHED                                    \
    (1UL << 1) // Bit 1: Motor 2 at target
#define MOTION_EVENT_SYNCHRONIZED_MOVE                                        \
    (1UL << 2) // Bit 2: Coordinated move active
#define MOTION_EVENT_POSITION_LIMIT_HIT                                       \
    (1UL << 3) // Bit 3: Position limit reached
#define MOTION_EVENT_SPEED_LIMIT_HIT (1UL << 4) // Bit 4: Speed limit reached
#define MOTION_EVENT_ESTOP_MOTION_STOPPED                                     \
    (1UL << 5) // Bit 5: Emergency stop motion halt

/** @brief Event group wait timeouts for different operations */
#define EVENT_GROUP_WAIT_STARTUP_MS                                           \
    (5000) // 5 second timeout for startup events
#define EVENT_GROUP_WAIT_MOTION_MS (2000) // 2 second timeout for motion events
#define EVENT_GROUP_WAIT_COMM_MS                                              \
    (500) // 500ms timeout for communication events
#define EVENT_GROUP_WAIT_EMERGENCY_MS (10) // 10ms timeout for emergency events

// Memory Pool Configuration
/** @brief Memory pool block sizes for dynamic allocation */
#define MEMORY_POOL_SMALL_BLOCK_SIZE (32)   // 32 bytes for small allocations
#define MEMORY_POOL_MEDIUM_BLOCK_SIZE (128) // 128 bytes for medium allocations
#define MEMORY_POOL_LARGE_BLOCK_SIZE (512)  // 512 bytes for large allocations

/** @brief Memory pool block counts */
#define MEMORY_POOL_SMALL_BLOCK_COUNT (16) // 16 small blocks (512 bytes total)
#define MEMORY_POOL_MEDIUM_BLOCK_COUNT                                        \
    (8)                                   // 8 medium blocks (1024 bytes total)
#define MEMORY_POOL_LARGE_BLOCK_COUNT (4) // 4 large blocks (2048 bytes total)

/** @brief Memory pool total allocation sizes */
#define MEMORY_POOL_SMALL_TOTAL_SIZE                                          \
    (MEMORY_POOL_SMALL_BLOCK_SIZE * MEMORY_POOL_SMALL_BLOCK_COUNT) // 512 bytes
#define MEMORY_POOL_MEDIUM_TOTAL_SIZE                                         \
    (MEMORY_POOL_MEDIUM_BLOCK_SIZE *                                          \
     MEMORY_POOL_MEDIUM_BLOCK_COUNT) // 1024 bytes
#define MEMORY_POOL_LARGE_TOTAL_SIZE                                          \
    (MEMORY_POOL_LARGE_BLOCK_SIZE *                                           \
     MEMORY_POOL_LARGE_BLOCK_COUNT) // 2048 bytes
#define MEMORY_POOL_TOTAL_ALLOCATION                                          \
    (MEMORY_POOL_SMALL_TOTAL_SIZE + MEMORY_POOL_MEDIUM_TOTAL_SIZE +           \
     MEMORY_POOL_LARGE_TOTAL_SIZE) // 3584 bytes

/** @brief Memory pool allocation timeouts */
#define MEMORY_POOL_ALLOC_TIMEOUT_MS                                          \
    (100) // 100ms timeout for memory allocation
#define MEMORY_POOL_FREE_TIMEOUT_MS                                           \
    (10) // 10ms timeout for memory deallocation

// Task Utilities Configuration
/** @brief Task statistics collection interval */
#define TASK_STATS_COLLECTION_PERIOD_MS (1000) // Collect stats every 1 second
#define TASK_STATS_HISTORY_SIZE (60)           // Keep 60 seconds of history

/** @brief CPU usage calculation parameters */
#define CPU_USAGE_CALCULATION_PERIOD_MS                                       \
    (100) // Calculate CPU usage every 100ms
#define CPU_USAGE_IDLE_THRESHOLD_PERCENT                                      \
    (95) // 95% idle threshold for power saving

/** @brief Stack monitoring parameters */
#define STACK_MONITOR_CHECK_PERIOD_MS                                         \
    (5000) // Check stack usage every 5 seconds
#define STACK_MONITOR_WARNING_THRESHOLD (80)  // Warn at 80% stack usage
#define STACK_MONITOR_CRITICAL_THRESHOLD (90) // Critical at 90% stack usage

// =============================================================================
// Advanced Power Management Configuration (Phase 2D Enhancement)
// =============================================================================

/** @brief Power management modes for energy optimization */
typedef enum {
    POWER_MODE_ACTIVE = 0, // Full performance mode
    POWER_MODE_QUIET,      // Reduced activity mode
    POWER_MODE_IDLE,       // Minimal activity mode
    POWER_MODE_SLEEP,      // Sleep mode with wake on interrupt
    POWER_MODE_COUNT
} PowerMode_t;

/** @brief Power management timing parameters */
#define POWER_IDLE_TIMEOUT_MS (30000)   // 30 seconds to idle
#define POWER_SLEEP_TIMEOUT_MS (300000) // 5 minutes to sleep
#define POWER_WAKE_DEBOUNCE_MS (100)    // 100ms wake debounce

/** @brief Quiet mode task period adjustments (multipliers) */
#define POWER_QUIET_MOTOR_PERIOD_MULTIPLIER (2)  // 2ms motor control in quiet
#define POWER_QUIET_SAFETY_PERIOD_MULTIPLIER (2) // 4ms safety checks in quiet
#define POWER_QUIET_COMM_PERIOD_MULTIPLIER (4) // 40ms CAN, 80ms UART in quiet
#define POWER_QUIET_TELEMETRY_PERIOD_MULTIPLIER                               \
    (10) // 100ms telemetry in quiet

/** @brief CPU frequency scaling for power modes */
#define POWER_ACTIVE_CPU_FREQ_MHZ (480) // Full 480MHz
#define POWER_QUIET_CPU_FREQ_MHZ (240)  // Half speed 240MHz
#define POWER_IDLE_CPU_FREQ_MHZ (120)   // Quarter speed 120MHz

// =============================================================================
// Dynamic Task Tuning Configuration (Phase 2D Enhancement)
// =============================================================================

/** @brief CPU utilization thresholds for dynamic tuning */
#define DYNAMIC_TUNE_CPU_LOW_THRESHOLD (30)       // Below 30% CPU usage
#define DYNAMIC_TUNE_CPU_HIGH_THRESHOLD (80)      // Above 80% CPU usage
#define DYNAMIC_TUNE_EVALUATION_PERIOD_MS (10000) // Evaluate every 10 seconds

/** @brief Task period adjustment limits (percentage) */
#define DYNAMIC_TUNE_MAX_PERIOD_INCREASE (200) // Maximum 2x period increase
#define DYNAMIC_TUNE_MAX_PERIOD_DECREASE (50)  // Minimum 0.5x period decrease
#define DYNAMIC_TUNE_ADJUSTMENT_STEP (10)      // 10% adjustment steps

// =============================================================================
// Standards & Compliance Configuration (Industrial Enhancement)
// =============================================================================

/** @brief Compliance and Standards Support */
#define MISRA_C_COMPLIANCE_ENABLED (1)     // Enable MISRA-C markers
#define IEC_61508_TRACEABILITY_ENABLED (1) // Enable safety traceability
#define SAFETY_INTEGRITY_LEVEL (2)         // SIL-2 target compliance

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

// Event Group Validation
_Static_assert(SYSTEM_EVENT_STARTUP_COMPLETE < (1UL << MAX_EVENT_GROUP_BITS),
               "System event bits exceed available event group range");
_Static_assert(
    COMM_EVENT_ERROR_REPORT_PENDING < (1UL << 8),
    "Communication event bits should fit in one byte for efficiency");
_Static_assert(MOTION_EVENT_ESTOP_MOTION_STOPPED < (1UL << 8),
               "Motion event bits should fit in one byte for efficiency");

// Memory Pool Validation
_Static_assert(MEMORY_POOL_TOTAL_ALLOCATION < (8 * 1024),
               "Memory pool allocation exceeds reasonable limit (8KB)");
_Static_assert(
    MEMORY_POOL_SMALL_BLOCK_SIZE >= 16,
    "Small memory blocks too small - minimum 16 bytes for alignment");
_Static_assert(MEMORY_POOL_LARGE_BLOCK_SIZE <= 1024,
               "Large memory blocks too big - maximum 1KB for efficiency");

// Task Utilities Validation
_Static_assert(TASK_STATS_HISTORY_SIZE * sizeof(uint32_t) < 1024,
               "Task statistics history too large - should fit in 1KB");
_Static_assert(STACK_MONITOR_WARNING_THRESHOLD <
                   STACK_MONITOR_CRITICAL_THRESHOLD,
               "Stack warning threshold must be less than critical threshold");
_Static_assert(
    CPU_USAGE_CALCULATION_PERIOD_MS <= TASK_STATS_COLLECTION_PERIOD_MS,
    "CPU usage calculation should be more frequent than stats collection");

// Advanced Power Management Validation
_Static_assert(POWER_MODE_COUNT <= 8,
               "Power mode count should fit in 3 bits for efficient storage");
_Static_assert(POWER_IDLE_TIMEOUT_MS < POWER_SLEEP_TIMEOUT_MS,
               "Idle timeout must be less than sleep timeout");
_Static_assert(POWER_QUIET_CPU_FREQ_MHZ <= POWER_ACTIVE_CPU_FREQ_MHZ,
               "Quiet mode CPU frequency cannot exceed active mode");
_Static_assert(POWER_IDLE_CPU_FREQ_MHZ <= POWER_QUIET_CPU_FREQ_MHZ,
               "Idle mode CPU frequency cannot exceed quiet mode");

// Dynamic Task Tuning Validation
_Static_assert(DYNAMIC_TUNE_CPU_LOW_THRESHOLD <
                   DYNAMIC_TUNE_CPU_HIGH_THRESHOLD,
               "CPU low threshold must be less than high threshold");
_Static_assert(DYNAMIC_TUNE_MAX_PERIOD_DECREASE <= 100,
               "Period decrease limit cannot exceed 100%");
_Static_assert(DYNAMIC_TUNE_MAX_PERIOD_INCREASE >= 100,
               "Period increase limit must be at least 100%");
_Static_assert(DYNAMIC_TUNE_ADJUSTMENT_STEP <= 50,
               "Adjustment step too large - maximum 50% for stability");

// Standards & Compliance Validation
_Static_assert(
    SAFETY_INTEGRITY_LEVEL >= 1 && SAFETY_INTEGRITY_LEVEL <= 4,
    "Safety Integrity Level must be between 1 and 4 (SIL-1 to SIL-4)");

#endif // FREERTOS_CONFIG_SSOT_H
