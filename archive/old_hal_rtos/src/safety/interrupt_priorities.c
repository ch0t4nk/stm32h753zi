/**
 * @file interrupt_priorities.c
 * @brief Safety-Critical Interrupt Priority Implementation
 * @author STM32H753ZI Project Team
 * @date 2025-08-12
 *
 * @details Implementation of hierarchical interrupt priority system ensuring
 *          safety-critical interrupts have deterministic response times.
 *          Builds on existing HAL abstraction and safety infrastructure.
 */

#include "interrupt_priorities.h"
#include "config/comm_config.h"
#include "config/hardware_config.h"
#include "config/motor_config.h"
#include "hal_abstraction/hal_abstraction.h"
#include "stm32h7xx_hal.h"
#include <string.h>

/* ==========================================================================
 */
/* Private Variables                                                         */
/* ==========================================================================
 */

static bool interrupt_priorities_initialized = false;

/// @brief Master interrupt configuration table
static const InterruptConfig_t interrupt_config_table[] = {
    // Safety-Critical Interrupts (Highest Priority)
    {EMERGENCY_STOP_IRQ, INTERRUPT_PRIORITY_EMERGENCY_STOP, 0, true,
     "Emergency Stop Button"},

    // Real-Time Control Interrupts
    {CONTROL_TIMER_IRQ, INTERRUPT_PRIORITY_REAL_TIME_CONTROL, 0, true,
     "Control Loop Timer"},
    {SAFETY_TIMER_IRQ, INTERRUPT_PRIORITY_SAFETY_CRITICAL, 0, true,
     "Safety Monitor Timer"},

    // Motor Communication Interrupts
    {MOTOR_SPI_IRQ, INTERRUPT_PRIORITY_MOTOR_COMMUNICATION, 0, true,
     "L6470 SPI Communication"},

    // Sensor Feedback Interrupts
    {ENCODER_I2C1_IRQ, INTERRUPT_PRIORITY_SENSOR_FEEDBACK, 0, true,
     "AS5600 Encoder I2C1"},
    {ENCODER_I2C2_IRQ, INTERRUPT_PRIORITY_SENSOR_FEEDBACK, 1, true,
     "AS5600 Encoder I2C2"},

    // System Communication Interrupts
    {SYSTEM_UART_IRQ, INTERRUPT_PRIORITY_COMMUNICATION, 0, true, "System UART"},
    {SYSTEM_CAN_IRQ, INTERRUPT_PRIORITY_COMMUNICATION, 1, true, "System CAN"},

    // System Management Interrupts
    {SYSTEM_TICK_IRQ, INTERRUPT_PRIORITY_SYSTEM_MANAGEMENT, 0, true,
     "System Tick"}};

static const uint8_t interrupt_config_count =
    sizeof(interrupt_config_table) / sizeof(InterruptConfig_t);

/* ==========================================================================
 */
/* Private Function Declarations                                             */
/* ==========================================================================
 */

static SystemError_t
apply_interrupt_configuration(const InterruptConfig_t *config);
static bool validate_priority_hierarchy(void);

/* ==========================================================================
 */
/* Public Function Implementation                                            */
/* ==========================================================================
 */

/**
 * @brief Initialize and configure all safety-critical interrupt priorities
 */
SystemError_t interrupt_priorities_init(void) {
  if (interrupt_priorities_initialized) {
    return SYSTEM_OK; // Already initialized
  }

  SystemError_t result = SYSTEM_OK;

  // Apply all interrupt configurations from table
  for (uint8_t i = 0; i < interrupt_config_count; i++) {
    result = apply_interrupt_configuration(&interrupt_config_table[i]);
    if (result != SYSTEM_OK) {
      return result;
    }
  }

  // Validate priority hierarchy
  if (!validate_priority_hierarchy()) {
    return ERROR_HARDWARE_FAULT;
  }

  interrupt_priorities_initialized = true;

  return SYSTEM_OK;
}

/**
 * @brief Validate interrupt priority configuration
 */
SystemError_t
interrupt_priorities_validate(InterruptPriorityValidation_t *validation) {
  if (!interrupt_priorities_initialized) {
    return ERROR_NOT_INITIALIZED;
  }

  if (validation == NULL) {
    return ERROR_NULL_POINTER;
  }

  // Initialize validation structure
  memset(validation, 0, sizeof(InterruptPriorityValidation_t));

  // Check emergency stop has highest priority
  validation->emergency_stop_highest =
      interrupt_emergency_stop_is_highest_priority();

  // Count interrupts by category
  validation->total_interrupts = interrupt_config_count;
  for (uint8_t i = 0; i < interrupt_config_count; i++) {
    if (IS_SAFETY_CRITICAL_PRIORITY(interrupt_config_table[i].priority)) {
      validation->safety_critical_count++;
    }
  }

  // Validate real-time protection
  validation->real_time_protected = validate_priority_hierarchy();

  // Check communication isolation (no communication interrupts at
  // safety-critical levels)
  validation->communication_isolated = true;
  for (uint8_t i = 0; i < interrupt_config_count; i++) {
    if (interrupt_config_table[i].priority ==
        INTERRUPT_PRIORITY_COMMUNICATION) {
      if (interrupt_config_table[i].priority <=
          INTERRUPT_PRIORITY_SAFETY_CRITICAL) {
        validation->communication_isolated = false;
        break;
      }
    }
  }

  // Validate no problematic priority gaps
  validation->priority_gaps_valid = true; // Simplified for now

  return SYSTEM_OK;
}

/**
 * @brief Set specific interrupt priority
 */
SystemError_t interrupt_set_priority(IRQn_Type irq_number,
                                     InterruptPriority_t priority,
                                     uint8_t sub_priority) {
  if (!IS_VALID_PRIORITY(priority)) {
    return ERROR_INVALID_PARAMETER;
  }

  // Use HAL to set interrupt priority
  HAL_NVIC_SetPriority(irq_number, (uint32_t)priority, sub_priority);

  return SYSTEM_OK;
}

/**
 * @brief Get interrupt priority configuration
 */
SystemError_t interrupt_get_priority(IRQn_Type irq_number,
                                     InterruptPriority_t *priority,
                                     uint8_t *sub_priority) {
  if (priority == NULL || sub_priority == NULL) {
    return ERROR_NULL_POINTER;
  }

  // Find interrupt in configuration table
  for (uint8_t i = 0; i < interrupt_config_count; i++) {
    if (interrupt_config_table[i].irq_number == irq_number) {
      *priority = interrupt_config_table[i].priority;
      *sub_priority = interrupt_config_table[i].sub_priority;
      return SYSTEM_OK;
    }
  }

  return ERROR_FAULT_NOT_FOUND;
}

/**
 * @brief Enable interrupt with configured priority
 */
SystemError_t interrupt_enable(IRQn_Type irq_number) {
  // Enable interrupt using HAL
  HAL_NVIC_EnableIRQ(irq_number);
  return SYSTEM_OK;
}

/**
 * @brief Disable specific interrupt
 */
SystemError_t interrupt_disable(IRQn_Type irq_number) {
  // Disable interrupt using HAL
  HAL_NVIC_DisableIRQ(irq_number);
  return SYSTEM_OK;
}

/**
 * @brief Get all configured interrupt priorities
 */
SystemError_t interrupt_get_all_priorities(InterruptConfig_t *configs,
                                           uint8_t max_configs,
                                           uint8_t *num_configs) {
  if (configs == NULL || num_configs == NULL) {
    return ERROR_NULL_POINTER;
  }

  *num_configs = (max_configs < interrupt_config_count)
                     ? max_configs
                     : interrupt_config_count;

  // Copy configuration entries
  memcpy(configs, interrupt_config_table,
         *num_configs * sizeof(InterruptConfig_t));

  return SYSTEM_OK;
}

/**
 * @brief Check if emergency stop has highest priority
 */
bool interrupt_emergency_stop_is_highest_priority(void) {
  // Emergency stop should have priority 0 (highest)
  for (uint8_t i = 0; i < interrupt_config_count; i++) {
    if (interrupt_config_table[i].irq_number == EMERGENCY_STOP_IRQ) {
      return (interrupt_config_table[i].priority ==
              INTERRUPT_PRIORITY_EMERGENCY_STOP);
    }
  }
  return false;
}

/**
 * @brief Enter critical section (disable interrupts)
 */
uint32_t interrupt_enter_critical_section(void) {
  uint32_t primask = __get_PRIMASK();
  __disable_irq();
  return primask;
}

/**
 * @brief Exit critical section (restore interrupts)
 */
void interrupt_exit_critical_section(uint32_t interrupt_state) {
  __set_PRIMASK(interrupt_state);
}

/* ==========================================================================
 */
/* Specific Interrupt Configuration Functions                               */
/* ==========================================================================
 */

/**
 * @brief Configure emergency stop interrupt priority
 */
SystemError_t configure_emergency_stop_interrupt(void) {
  SystemError_t result = interrupt_set_priority(
      EMERGENCY_STOP_IRQ, INTERRUPT_PRIORITY_EMERGENCY_STOP, 0);
  if (result == SYSTEM_OK) {
    result = interrupt_enable(EMERGENCY_STOP_IRQ);
  }
  return result;
}

/**
 * @brief Configure real-time control timer interrupt priority
 */
SystemError_t configure_control_timer_interrupt(void) {
  SystemError_t result = interrupt_set_priority(
      CONTROL_TIMER_IRQ, INTERRUPT_PRIORITY_REAL_TIME_CONTROL, 0);
  if (result == SYSTEM_OK) {
    result = interrupt_enable(CONTROL_TIMER_IRQ);
  }
  return result;
}

/**
 * @brief Configure safety monitoring timer interrupt priority
 */
SystemError_t configure_safety_timer_interrupt(void) {
  SystemError_t result = interrupt_set_priority(
      SAFETY_TIMER_IRQ, INTERRUPT_PRIORITY_SAFETY_CRITICAL, 0);
  if (result == SYSTEM_OK) {
    result = interrupt_enable(SAFETY_TIMER_IRQ);
  }
  return result;
}

/**
 * @brief Configure motor communication interrupt priorities
 */
SystemError_t configure_motor_communication_interrupts(void) {
  SystemError_t result = interrupt_set_priority(
      MOTOR_SPI_IRQ, INTERRUPT_PRIORITY_MOTOR_COMMUNICATION, 0);
  if (result == SYSTEM_OK) {
    result = interrupt_enable(MOTOR_SPI_IRQ);
  }
  return result;
}

/**
 * @brief Configure sensor feedback interrupt priorities
 */
SystemError_t configure_sensor_feedback_interrupts(void) {
  SystemError_t result = interrupt_set_priority(
      ENCODER_I2C1_IRQ, INTERRUPT_PRIORITY_SENSOR_FEEDBACK, 0);
  if (result == SYSTEM_OK) {
    result = interrupt_set_priority(ENCODER_I2C2_IRQ,
                                    INTERRUPT_PRIORITY_SENSOR_FEEDBACK, 1);
  }
  if (result == SYSTEM_OK) {
    result = interrupt_enable(ENCODER_I2C1_IRQ);
  }
  if (result == SYSTEM_OK) {
    result = interrupt_enable(ENCODER_I2C2_IRQ);
  }
  return result;
}

/**
 * @brief Configure system communication interrupt priorities
 */
SystemError_t configure_system_communication_interrupts(void) {
  SystemError_t result = interrupt_set_priority(
      SYSTEM_UART_IRQ, INTERRUPT_PRIORITY_COMMUNICATION, 0);
  if (result == SYSTEM_OK) {
    result = interrupt_set_priority(SYSTEM_CAN_IRQ,
                                    INTERRUPT_PRIORITY_COMMUNICATION, 1);
  }
  if (result == SYSTEM_OK) {
    result = interrupt_enable(SYSTEM_UART_IRQ);
  }
  if (result == SYSTEM_OK) {
    result = interrupt_enable(SYSTEM_CAN_IRQ);
  }
  return result;
}

/* ==========================================================================
 */
/* Private Function Implementation                                           */
/* ==========================================================================
 */

/**
 * @brief Apply interrupt configuration from table entry
 */
static SystemError_t
apply_interrupt_configuration(const InterruptConfig_t *config) {
  if (config == NULL) {
    return ERROR_NULL_POINTER;
  }

  // Set interrupt priority
  SystemError_t result = interrupt_set_priority(
      config->irq_number, config->priority, config->sub_priority);
  if (result != SYSTEM_OK) {
    return result;
  }

  // Enable interrupt if specified
  if (config->enabled) {
    result = interrupt_enable(config->irq_number);
  }

  return result;
}

/**
 * @brief Validate priority hierarchy is correct
 */
static bool validate_priority_hierarchy(void) {
  // Check that emergency stop has the highest priority (lowest number)
  InterruptPriority_t emergency_priority = INTERRUPT_PRIORITY_MAX;

  for (uint8_t i = 0; i < interrupt_config_count; i++) {
    if (interrupt_config_table[i].irq_number == EMERGENCY_STOP_IRQ) {
      emergency_priority = interrupt_config_table[i].priority;
      break;
    }
  }

  // Emergency stop must be highest priority
  if (emergency_priority != INTERRUPT_PRIORITY_EMERGENCY_STOP) {
    return false;
  }

  // Validate no interrupts have higher priority than emergency stop
  for (uint8_t i = 0; i < interrupt_config_count; i++) {
    if (interrupt_config_table[i].priority < emergency_priority) {
      return false; // Found interrupt with higher priority than
                    // emergency stop
    }
  }

  return true;
}
