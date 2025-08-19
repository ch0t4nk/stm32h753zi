/**
 * @file interrupt_priorities.h
 * @brief Safety-Critical Interrupt Priority Configuration
 * @author STM32H753ZI Project Team
 * @date 2025-08-12
 *
 * @details Hierarchical interrupt priority system ensuring safety-critical
 *          interrupts have highest priority for deterministic real-time
 *          operation following .github/instructions/safety-rt.instructions.md
 *
 * @note SAFETY-CRITICAL: Emergency stop must have highest interrupt priority
 *       to ensure <1ms response time regardless of system load
 */

#ifndef INTERRUPT_PRIORITIES_H
#define INTERRUPT_PRIORITIES_H

#include "common/system_state.h"
#include "config/hardware_config.h"
#include "config/safety_config.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef UNITY_TESTING
/* Minimal stub for IRQn_Type used in host-tests */
typedef int IRQn_Type;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
 */
/* Interrupt Priority Definitions (SSOT)                                    */
/* ==========================================================================
 */

/**
 * @brief Safety-critical interrupt priority levels
 * @note Lower numbers = higher priority (ARM Cortex-M convention)
 */
typedef enum {
    INTERRUPT_PRIORITY_EMERGENCY_STOP = 0,      ///< Emergency stop (highest)
    INTERRUPT_PRIORITY_SAFETY_CRITICAL = 1,     ///< Safety-critical systems
    INTERRUPT_PRIORITY_REAL_TIME_CONTROL = 2,   ///< Real-time control loop
    INTERRUPT_PRIORITY_MOTOR_COMMUNICATION = 3, ///< Motor driver communication
    INTERRUPT_PRIORITY_SENSOR_FEEDBACK = 4,     ///< Sensor feedback (encoders)
    INTERRUPT_PRIORITY_COMMUNICATION = 5,       ///< General communication
    INTERRUPT_PRIORITY_SYSTEM_MANAGEMENT = 6,   ///< System management
    INTERRUPT_PRIORITY_BACKGROUND = 7,          ///< Background tasks (lowest)
    INTERRUPT_PRIORITY_MAX = 8
} InterruptPriority_t;

/**
 * @brief Interrupt configuration structure
 */
typedef struct {
    IRQn_Type irq_number;         ///< Interrupt request number
    InterruptPriority_t priority; ///< Interrupt priority level
    uint8_t sub_priority;         ///< Sub-priority within same level
    bool enabled;                 ///< Interrupt enabled
    const char *description;      ///< Interrupt description
} InterruptConfig_t;

/**
 * @brief Interrupt priority validation results
 */
typedef struct {
    bool emergency_stop_highest;   ///< Emergency stop has highest priority
    bool real_time_protected;      ///< Real-time tasks properly prioritized
    bool communication_isolated;   ///< Communication properly isolated
    bool priority_gaps_valid;      ///< No priority gaps that affect safety
    uint8_t total_interrupts;      ///< Total configured interrupts
    uint8_t safety_critical_count; ///< Number of safety-critical interrupts
} InterruptPriorityValidation_t;

/* ==========================================================================
 */
/* Public Function Declarations                                              */
/* ==========================================================================
 */

/**
 * @brief Initialize and configure all safety-critical interrupt priorities
 * @return SystemError_t Success or error code
 *
 * @details Configures hierarchical interrupt priority system with emergency
 *          stop at highest priority, followed by real-time control and
 *          motor communication systems
 */
SystemError_t interrupt_priorities_init(void);

/**
 * @brief Validate interrupt priority configuration
 * @param validation Pointer to validation results
 * @return SystemError_t Success or error code
 */
SystemError_t
interrupt_priorities_validate(InterruptPriorityValidation_t *validation);

/**
 * @brief Set specific interrupt priority
 * @param irq_number Interrupt request number
 * @param priority Priority level
 * @param sub_priority Sub-priority within level
 * @return SystemError_t Success or error code
 */
SystemError_t interrupt_set_priority(IRQn_Type irq_number,
                                     InterruptPriority_t priority,
                                     uint8_t sub_priority);

/**
 * @brief Get interrupt priority configuration
 * @param irq_number Interrupt request number
 * @param priority Pointer to priority output
 * @param sub_priority Pointer to sub-priority output
 * @return SystemError_t Success or error code
 */
SystemError_t interrupt_get_priority(IRQn_Type irq_number,
                                     InterruptPriority_t *priority,
                                     uint8_t *sub_priority);

/**
 * @brief Enable interrupt with configured priority
 * @param irq_number Interrupt request number
 * @return SystemError_t Success or error code
 */
SystemError_t interrupt_enable(IRQn_Type irq_number);

/**
 * @brief Disable specific interrupt
 * @param irq_number Interrupt request number
 * @return SystemError_t Success or error code
 */
SystemError_t interrupt_disable(IRQn_Type irq_number);

/**
 * @brief Get all configured interrupt priorities
 * @param configs Array to store configurations
 * @param max_configs Maximum number of configurations to return
 * @param num_configs Pointer to number of configurations returned
 * @return SystemError_t Success or error code
 */
SystemError_t interrupt_get_all_priorities(InterruptConfig_t *configs,
                                           uint8_t max_configs,
                                           uint8_t *num_configs);

/**
 * @brief Check if emergency stop has highest priority
 * @return bool True if emergency stop has highest priority
 */
bool interrupt_emergency_stop_is_highest_priority(void);

/**
 * @brief Enter critical section (disable interrupts)
 * @return uint32_t Previous interrupt state for restoration
 */
uint32_t interrupt_enter_critical_section(void);

/**
 * @brief Exit critical section (restore interrupts)
 * @param interrupt_state Previous interrupt state to restore
 */
void interrupt_exit_critical_section(uint32_t interrupt_state);

/* ==========================================================================
 */
/* Specific Interrupt Configuration Functions                               */
/* ==========================================================================
 */

/**
 * @brief Configure emergency stop interrupt priority
 * @return SystemError_t Success or error code
 */
SystemError_t configure_emergency_stop_interrupt(void);

/**
 * @brief Configure real-time control timer interrupt priority
 * @return SystemError_t Success or error code
 */
SystemError_t configure_control_timer_interrupt(void);

/**
 * @brief Configure safety monitoring timer interrupt priority
 * @return SystemError_t Success or error code
 */
SystemError_t configure_safety_timer_interrupt(void);

/**
 * @brief Configure motor communication interrupt priorities
 * @return SystemError_t Success or error code
 */
SystemError_t configure_motor_communication_interrupts(void);

/**
 * @brief Configure sensor feedback interrupt priorities
 * @return SystemError_t Success or error code
 */
SystemError_t configure_sensor_feedback_interrupts(void);

/**
 * @brief Configure system communication interrupt priorities
 * @return SystemError_t Success or error code
 */
SystemError_t configure_system_communication_interrupts(void);

/* ==========================================================================
 */
/* Safety Validation Macros                                                 */
/* ==========================================================================
 */

/**
 * @brief Check if interrupt priority is safety-critical
 */
#define IS_SAFETY_CRITICAL_PRIORITY(priority)                                 \
    ((priority) <= INTERRUPT_PRIORITY_SAFETY_CRITICAL)

/**
 * @brief Check if interrupt priority is real-time
 */
#define IS_REAL_TIME_PRIORITY(priority)                                       \
    ((priority) <= INTERRUPT_PRIORITY_REAL_TIME_CONTROL)

/**
 * @brief Validate priority level range
 */
#define IS_VALID_PRIORITY(priority) ((priority) < INTERRUPT_PRIORITY_MAX)

/* ==========================================================================
 */
/* Hardware-Specific Interrupt Mappings                                     */
/* ==========================================================================
 */

/// @brief Emergency stop button interrupt
#define EMERGENCY_STOP_IRQ EXTI15_10_IRQn

/// @brief Control loop timer interrupt
#define CONTROL_TIMER_IRQ TIM2_IRQn

/// @brief Safety monitoring timer interrupt (defined in hardware_config.h)
// #define SAFETY_TIMER_IRQ TIM3_IRQn // Using SSOT from hardware_config.h

/// @brief L6470 SPI communication interrupt
#define MOTOR_SPI_IRQ SPI2_IRQn

/// @brief AS5600 I2C communication interrupts
#define ENCODER_I2C1_IRQ I2C1_EV_IRQn
#define ENCODER_I2C2_IRQ I2C2_EV_IRQn

/// @brief System UART communication interrupt
#define SYSTEM_UART_IRQ USART3_IRQn

/// @brief CAN communication interrupt
#define SYSTEM_CAN_IRQ FDCAN1_IT0_IRQn

/// @brief System tick interrupt
#define SYSTEM_TICK_IRQ SysTick_IRQn

#ifdef __cplusplus
}
#endif

#endif /* INTERRUPT_PRIORITIES_H */
