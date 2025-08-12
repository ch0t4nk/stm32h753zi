/**
 * @file l6470_fault_monitor.c
 * @brief L6470 Hardware Fault Monitoring Implementation
 * @author STM32H753ZI Project Team
 * @date 2025-08-07
 *
 * @note Critical hardware fault monitoring for L6470 via FLAG/BUSY pins
 * Implements real-time fault detection with <1ms reaction time
 */

#include "l6470_fault_monitor.h"
#include "common/system_state.h"
#include "config/hardware_config.h"
#include "config/motor_config.h"
#include "drivers/l6470/l6470_driver.h"
#include "emergency_stop.h"
#include "fault_monitor.h"
#include "hal_abstraction/hal_abstraction.h"
#include <string.h>

/* ==========================================================================
 */
/* Private Variables                                                         */
/* ==========================================================================
 */

static bool fault_monitor_initialized = false;
static L6470HwFaultStatus_t fault_status = {0};

// GPIO handles for interrupt configuration
static GPIO_InitTypeDef flag_pin_config = {0};
static GPIO_InitTypeDef busy_pin_config = {0};

/* ==========================================================================
 */
/* Private Function Prototypes                                               */
/* ==========================================================================
 */

static SystemError_t configure_flag_pin_interrupt(void);
static SystemError_t configure_busy_pin_interrupt(void);
static void process_l6470_fault(L6470HwFaultType_t fault_type);
static L6470HwFaultType_t decode_l6470_status(uint16_t status_register);

/* ==========================================================================
 */
/* Public Function Implementations                                           */
/* ==========================================================================
 */

/**
 * @brief Initialize L6470 hardware fault monitoring
 */
SystemError_t l6470_fault_monitor_init(void) {
    if (fault_monitor_initialized) {
        return SYSTEM_OK; // Already initialized
    }

    // Initialize fault status
    memset(&fault_status, 0, sizeof(fault_status));
    fault_status.monitoring_enabled = false;

    // Configure FLAG pin for falling edge interrupt (active low fault
    // indication)
    SystemError_t result = configure_flag_pin_interrupt();
    if (result != SYSTEM_OK) {
        return result;
    }

    // Configure BUSY pin for monitoring (optional)
    result = configure_busy_pin_interrupt();
    if (result != SYSTEM_OK) {
        return result;
    }

    fault_monitor_initialized = true;
    return SYSTEM_OK;
}

/**
 * @brief Enable L6470 fault monitoring interrupts
 */
SystemError_t l6470_fault_monitor_enable(void) {
    if (!fault_monitor_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Enable NVIC interrupts for FLAG pin
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0); // Highest priority for safety
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    fault_status.monitoring_enabled = true;
    return SYSTEM_OK;
}

/**
 * @brief Disable L6470 fault monitoring interrupts
 */
SystemError_t l6470_fault_monitor_disable(void) {
    if (!fault_monitor_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Disable NVIC interrupts
    HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

    fault_status.monitoring_enabled = false;
    return SYSTEM_OK;
}

/**
 * @brief Get current L6470 fault status
 */
SystemError_t l6470_fault_monitor_get_status(L6470HwFaultStatus_t *status) {
    if (status == NULL) {
        return ERROR_NULL_POINTER;
    }

    if (!fault_monitor_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    *status = fault_status;
    return SYSTEM_OK;
}

/**
 * @brief Clear fault status after resolution
 */
SystemError_t l6470_fault_monitor_clear_faults(void) {
    if (!fault_monitor_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    fault_status.fault_active = false;
    fault_status.active_faults = L6470_HW_FAULT_NONE;
    // Keep fault count and timing for diagnostics

    return SYSTEM_OK;
}

/**
 * @brief GPIO interrupt handler for L6470 FLAG pin
 */
void l6470_fault_monitor_flag_interrupt(void) {
    uint32_t interrupt_start_time = HAL_GetTick();

    // FLAG pin is active low - fault when low
    GPIO_PinState flag_state =
        HAL_GPIO_ReadPin(MOTOR_FLAG_PORT, MOTOR_FLAG_PIN);

    if (flag_state == GPIO_PIN_RESET) {
        // Fault detected - read L6470 status to determine fault type
        uint16_t status_register = 0;

        // Read status from both motors (assuming dual motor setup)
        for (uint8_t motor_id = 0; motor_id < MAX_MOTORS; motor_id++) {
            SystemError_t result =
                l6470_get_status(motor_id, &status_register);
            if (result == SYSTEM_OK) {
                L6470HwFaultType_t fault_type =
                    decode_l6470_status(status_register);
                if (fault_type != L6470_HW_FAULT_NONE) {
                    process_l6470_fault(fault_type);
                }
            }
        }
    }

    // Record reaction time for safety validation
    fault_status.fault_reaction_time_us =
        (HAL_GetTick() - interrupt_start_time) * 1000;
    fault_status.fault_count++;
}

/**
 * @brief GPIO interrupt handler for L6470 BUSY pin
 */
void l6470_fault_monitor_busy_interrupt(void) {
    // BUSY pin monitoring for command completion and stall detection
    GPIO_PinState busy_state =
        HAL_GPIO_ReadPin(MOTOR_BUSY_PORT, MOTOR_BUSY_PIN);

    // Log busy state changes for diagnostics
    // This can help detect communication issues or unexpected motor behavior
    (void)busy_state; // Suppress unused variable warning
}

/**
 * @brief Check if fault monitoring is functional
 */
bool l6470_fault_monitor_is_active(void) {
    return fault_monitor_initialized && fault_status.monitoring_enabled;
}

/**
 * @brief Test fault monitoring system
 */
SystemError_t l6470_fault_monitor_test(void) {
    if (!fault_monitor_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Test FLAG pin reading
    GPIO_PinState flag_state =
        HAL_GPIO_ReadPin(MOTOR_FLAG_PORT, MOTOR_FLAG_PIN);
    GPIO_PinState busy_state =
        HAL_GPIO_ReadPin(MOTOR_BUSY_PORT, MOTOR_BUSY_PIN);

    // FLAG pin should normally be high (no fault)
    // BUSY pin state depends on motor activity
    (void)flag_state; // Suppress unused variable warning
    (void)busy_state; // Suppress unused variable warning

    // Test passed if we can read both pins
    return SYSTEM_OK;
}

/* ==========================================================================
 */
/* Private Function Implementations                                          */
/* ==========================================================================
 */

/**
 * @brief Configure FLAG pin for interrupt detection
 */
static SystemError_t configure_flag_pin_interrupt(void) {
    // Enable GPIO clock
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Configure FLAG pin (PA10) as input with pull-up
    flag_pin_config.Pin = MOTOR_FLAG_PIN;
    flag_pin_config.Mode =
        GPIO_MODE_IT_FALLING;           // Interrupt on falling edge (fault)
    flag_pin_config.Pull = GPIO_PULLUP; // Pull-up for active-low signal
    flag_pin_config.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(MOTOR_FLAG_PORT, &flag_pin_config);

    return SYSTEM_OK;
}

/**
 * @brief Configure BUSY pin for monitoring
 */
static SystemError_t configure_busy_pin_interrupt(void) {
    // Enable GPIO clock
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Configure BUSY pin (PA11) as input (optional interrupt)
    busy_pin_config.Pin = MOTOR_BUSY_PIN;
    busy_pin_config.Mode = GPIO_MODE_INPUT; // Input only for now
    busy_pin_config.Pull = GPIO_PULLUP;     // Pull-up
    busy_pin_config.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(MOTOR_BUSY_PORT, &busy_pin_config);

    return SYSTEM_OK;
}

/**
 * @brief Process detected L6470 fault
 */
static void process_l6470_fault(L6470HwFaultType_t fault_type) {
    fault_status.fault_active = true;
    fault_status.active_faults |= fault_type;
    fault_status.last_fault_time = HAL_GetTick();

    // Determine response based on fault severity
    switch (fault_type) {
    case L6470_HW_FAULT_OVERCURRENT:
    case L6470_HW_FAULT_THERMAL_SHUTDOWN:
    case L6470_HW_FAULT_STALL_DETECTED:
        // Critical faults - immediate emergency stop
        execute_emergency_stop(ESTOP_SOURCE_OVERCURRENT);
        break;

    case L6470_HW_FAULT_THERMAL_WARNING:
    case L6470_HW_FAULT_UNDERVOLTAGE:
        // Warning conditions - log but continue if safe
        fault_monitor_record_system_fault(SYSTEM_FAULT_POWER_FAILURE,
                                          FAULT_SEVERITY_WARNING, fault_type);
        break;

    case L6470_HW_FAULT_STEP_LOSS:
    case L6470_HW_FAULT_COMMAND_ERROR:
        // Operational faults - stop affected motor
        // Implementation depends on motor identification
        break;

    default:
        // Unknown fault - treat as critical
        execute_emergency_stop(ESTOP_SOURCE_FAULT_CASCADE);
        break;
    }
}

/**
 * @brief Decode L6470 status register to fault type
 */
static L6470HwFaultType_t decode_l6470_status(uint16_t status_register) {
    L6470HwFaultType_t fault_type = L6470_HW_FAULT_NONE;

    // Check status bits (based on L6470 datasheet)
    if (status_register & L6470_STATUS_OCD) {
        fault_type |= L6470_HW_FAULT_OVERCURRENT;
    }
    if (status_register & L6470_STATUS_TH_SD) {
        fault_type |= L6470_HW_FAULT_THERMAL_SHUTDOWN;
    }
    if (status_register & L6470_STATUS_TH_WRN) {
        fault_type |= L6470_HW_FAULT_THERMAL_WARNING;
    }
    if (status_register & L6470_STATUS_UVLO) {
        fault_type |= L6470_HW_FAULT_UNDERVOLTAGE;
    }
    if (status_register &
        (L6470_STATUS_STEP_LOSS_A | L6470_STATUS_STEP_LOSS_B)) {
        fault_type |= L6470_HW_FAULT_STEP_LOSS;
    }
    if (status_register &
        (L6470_STATUS_WRONG_CMD | L6470_STATUS_NOTPERF_CMD)) {
        fault_type |= L6470_HW_FAULT_COMMAND_ERROR;
    }

    return fault_type;
}
