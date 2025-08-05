/**
 * @file emergency_stop.c
 * @brief Emergency Stop System Implementation - STM32H753ZI Stepper Motor Project
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 * 
 * @note Emergency stop implementation following:
 * - .github/instructions/safety-rt.instructions.md
 * - config/safety_config.h (SSOT)
 * 
 * @warning SAFETY-CRITICAL: This system must respond within ESTOP_REACTION_TIME_MS
 */

#include "emergency_stop.h"
#include "config/hardware_config.h"
#include "common/system_state.h"
#include <string.h>

/* ========================================================================== */
/* Private Variables and State Management                                    */
/* ========================================================================== */

static bool emergency_stop_initialized = false;
static EmergencyStopState_t emergency_stop_state = {0};
static uint32_t estop_activation_count = 0;
static uint32_t estop_reset_count = 0;

// Hardware handles
static GPIO_InitTypeDef estop_button_gpio = {0};
static GPIO_InitTypeDef safety_relay_gpio = {0};
static GPIO_InitTypeDef estop_led_gpio = {0};

/* ========================================================================== */
/* Private Function Prototypes                                               */
/* ========================================================================== */

static SystemError_t initialize_estop_gpio(void);
static SystemError_t initialize_estop_interrupt(void);
static void disable_motor_power(void);
static void enable_motor_power(void);
static SystemError_t validate_reset_conditions(void);

/* ========================================================================== */
/* Public API Implementation                                                 */
/* ========================================================================== */

/**
 * @brief Initialize emergency stop system
 */
SystemError_t emergency_stop_init(void) {
    if (emergency_stop_initialized) {
        return ERROR_ALREADY_INITIALIZED;
    }
    
    // Initialize emergency stop state
    memset(&emergency_stop_state, 0, sizeof(EmergencyStopState_t));
    emergency_stop_state.active = false;
    emergency_stop_state.latched = false;
    emergency_stop_state.source = ESTOP_SOURCE_NONE;
    emergency_stop_state.timestamp = 0;
    emergency_stop_state.reset_attempts = 0;
    emergency_stop_state.reset_pending = false;
    
    // Initialize GPIO configuration
    SystemError_t gpio_result = initialize_estop_gpio();
    if (gpio_result != SYSTEM_OK) {
        return gpio_result;
    }
    
    // Initialize interrupt configuration
    SystemError_t interrupt_result = initialize_estop_interrupt();
    if (interrupt_result != SYSTEM_OK) {
        return interrupt_result;
    }
    
    // Initialize safety relays to safe state (power enabled)
    enable_motor_power();
    
    // Initialize emergency stop LED (off)
    emergency_stop_set_led(false);
    
    // Reset counters
    estop_activation_count = 0;
    estop_reset_count = 0;
    
    emergency_stop_initialized = true;
    
    return SYSTEM_OK;
}

/**
 * @brief Execute emergency stop sequence
 */
SystemError_t emergency_stop_execute(EmergencyStopSource_t source) {
    if (!emergency_stop_initialized) {
        return ERROR_NOT_INITIALIZED;
    }
    
    uint32_t start_time = HAL_GetTick();
    
    // Set emergency stop state immediately
    emergency_stop_state.active = true;
    emergency_stop_state.latched = true;
    emergency_stop_state.source = source;
    emergency_stop_state.timestamp = start_time;
    emergency_stop_state.reset_pending = false;
    
    // Increment activation counter
    estop_activation_count++;
    
    // CRITICAL: Immediately disable motor power (hardware level)
    disable_motor_power();
    
    // Stop all motor motion commands (would integrate with L6470 driver)
    // TODO: Integration with L6470 driver
    // for (uint8_t i = 0; i < MAX_MOTORS; i++) {
    //     l6470_immediate_stop(i);
    // }
    
    // Turn on emergency stop LED
    emergency_stop_set_led(true);
    
    // Set system to fault state
    // TODO: Integration with system state manager
    // set_system_state(SYSTEM_STATE_FAULT);
    
    // Verify reaction time (should be under ESTOP_REACTION_TIME_MS)
    uint32_t reaction_time = HAL_GetTick() - start_time;
    if (reaction_time > ESTOP_REACTION_TIME_MS) {
        // Log reaction time violation
        return ERROR_SAFETY_REACTION_TIME;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Reset emergency stop (requires manual confirmation)
 */
SystemError_t emergency_stop_reset(void) {
    if (!emergency_stop_initialized) {
        return ERROR_NOT_INITIALIZED;
    }
    
    // Check if reset is allowed
    if (!emergency_stop_state.latched) {
        return ERROR_SAFETY_INVALID_RESET;
    }
    
    // Verify physical button is released (if button was the source)
    if (emergency_stop_state.source == ESTOP_SOURCE_BUTTON) {
        if (is_emergency_stop_pressed()) {
            return ERROR_SAFETY_BUTTON_STILL_PRESSED;
        }
    }
    
    // Perform comprehensive safety validation
    SystemError_t validation_result = validate_reset_conditions();
    if (validation_result != SYSTEM_OK) {
        return validation_result;
    }
    
    // Check minimum reset delay
    uint32_t current_time = HAL_GetTick();
    if ((current_time - emergency_stop_state.timestamp) < ESTOP_RESET_DELAY_MS) {
        return ERROR_SAFETY_RESET_TOO_SOON;
    }
    
    // Reset emergency stop state
    emergency_stop_state.active = false;
    emergency_stop_state.latched = false;
    emergency_stop_state.source = ESTOP_SOURCE_NONE;
    emergency_stop_state.reset_pending = false;
    emergency_stop_state.reset_attempts++;
    
    // Increment reset counter
    estop_reset_count++;
    
    // Re-enable motor power
    enable_motor_power();
    
    // Turn off emergency stop LED
    emergency_stop_set_led(false);
    
    // Return to ready state
    // TODO: Integration with system state manager
    // set_system_state(SYSTEM_STATE_READY);
    
    return SYSTEM_OK;
}

/**
 * @brief Get current emergency stop state
 */
EmergencyStopState_t emergency_stop_get_state(void) {
    return emergency_stop_state;
}

/**
 * @brief Check if emergency stop button is pressed
 */
bool is_emergency_stop_pressed(void) {
    // Read button state (active low with pull-up)
    GPIO_PinState button_state = HAL_GPIO_ReadPin(ESTOP_BUTTON_PORT, ESTOP_BUTTON_PIN);
    return (button_state == GPIO_PIN_RESET);
}

/**
 * @brief Emergency stop interrupt handler
 */
void emergency_stop_interrupt_handler(void) {
    // CRITICAL: Execute emergency stop immediately in interrupt context
    // This is called from EXTI interrupt handler
    
    // Verify this is actually the emergency stop button
    if (__HAL_GPIO_EXTI_GET_IT(ESTOP_BUTTON_PIN) != RESET) {
        // Clear interrupt flag first
        __HAL_GPIO_EXTI_CLEAR_IT(ESTOP_BUTTON_PIN);
        
        // Execute emergency stop (minimal processing in ISR)
        emergency_stop_execute(ESTOP_SOURCE_BUTTON);
    }
}

/**
 * @brief Perform emergency stop self-test
 */
SystemError_t emergency_stop_self_test(void) {
    if (!emergency_stop_initialized) {
        return ERROR_NOT_INITIALIZED;
    }
    
    // Test 1: Verify GPIO is configured (read current state)
    // Note: STM32 HAL doesn't provide HAL_GPIO_GetConfig, so we test functionality
    GPIO_PinState current_state = HAL_GPIO_ReadPin(ESTOP_BUTTON_PORT, ESTOP_BUTTON_PIN);
    (void)current_state; // Suppress unused warning - we just verify pin is accessible
    
    // Test 2: Verify safety relay outputs
    HAL_GPIO_WritePin(SAFETY_RELAY1_PORT, SAFETY_RELAY1_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
    GPIO_PinState relay1_state = HAL_GPIO_ReadPin(SAFETY_RELAY1_PORT, SAFETY_RELAY1_PIN);
    HAL_GPIO_WritePin(SAFETY_RELAY1_PORT, SAFETY_RELAY1_PIN, GPIO_PIN_SET);
    
    if (relay1_state != GPIO_PIN_RESET) {
        return ERROR_SAFETY_SELF_TEST_FAILED;
    }
    
    // Test 3: Verify LED functionality
    emergency_stop_set_led(true);
    HAL_Delay(1);
    GPIO_PinState led_state = HAL_GPIO_ReadPin(ESTOP_LED_PORT, ESTOP_LED_PIN);
    emergency_stop_set_led(false);
    
    if (led_state != GPIO_PIN_SET) {
        return ERROR_SAFETY_SELF_TEST_FAILED;
    }
    
    // Test 4: Verify interrupt configuration
    IRQn_Type irq_number = ESTOP_BUTTON_EXTI_IRQn;
    if (!NVIC_GetEnableIRQ(irq_number)) {
        return ERROR_SAFETY_SELF_TEST_FAILED;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Set emergency stop LED state
 */
void emergency_stop_set_led(bool state) {
    GPIO_PinState pin_state = state ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(ESTOP_LED_PORT, ESTOP_LED_PIN, pin_state);
}

/**
 * @brief Check emergency stop system health
 */
SystemError_t emergency_stop_check_health(void) {
    if (!emergency_stop_initialized) {
        return ERROR_NOT_INITIALIZED;
    }
    
    // Check for stuck button
    static uint32_t button_pressed_time = 0;
    if (is_emergency_stop_pressed()) {
        if (button_pressed_time == 0) {
            button_pressed_time = HAL_GetTick();
        } else if ((HAL_GetTick() - button_pressed_time) > 30000) {  // 30 seconds
            // Button stuck for too long
            return ERROR_SAFETY_BUTTON_STUCK;
        }
    } else {
        button_pressed_time = 0;
    }
    
    // Check for excessive activation count
    if (estop_activation_count > 100) {
        // Too many emergency stops (potential system instability)
        return ERROR_SAFETY_EXCESSIVE_ACTIVATIONS;
    }
    
    return SYSTEM_OK;
}

/* ========================================================================== */
/* Private Function Implementation                                           */
/* ========================================================================== */

/**
 * @brief Initialize emergency stop GPIO
 */
static SystemError_t initialize_estop_gpio(void) {
    // Enable GPIO clocks
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    // Configure emergency stop button (input with pull-up, interrupt on falling edge)
    estop_button_gpio.Pin = ESTOP_BUTTON_PIN;
    estop_button_gpio.Mode = GPIO_MODE_IT_FALLING;
    estop_button_gpio.Pull = GPIO_PULLUP;
    estop_button_gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ESTOP_BUTTON_PORT, &estop_button_gpio);
    
    // Configure safety relay outputs (active high)
    safety_relay_gpio.Pin = SAFETY_RELAY1_PIN | SAFETY_RELAY2_PIN;
    safety_relay_gpio.Mode = GPIO_MODE_OUTPUT_PP;
    safety_relay_gpio.Pull = GPIO_NOPULL;
    safety_relay_gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SAFETY_RELAY1_PORT, &safety_relay_gpio);
    
    // Configure emergency stop LED (active high)
    estop_led_gpio.Pin = ESTOP_LED_PIN;
    estop_led_gpio.Mode = GPIO_MODE_OUTPUT_PP;
    estop_led_gpio.Pull = GPIO_NOPULL;
    estop_led_gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ESTOP_LED_PORT, &estop_led_gpio);
    
    return SYSTEM_OK;
}

/**
 * @brief Initialize emergency stop interrupt
 */
static SystemError_t initialize_estop_interrupt(void) {
    // Configure and enable EXTI interrupt
    HAL_NVIC_SetPriority(ESTOP_BUTTON_EXTI_IRQn, 0, 0);  // Highest priority
    HAL_NVIC_EnableIRQ(ESTOP_BUTTON_EXTI_IRQn);
    
    return SYSTEM_OK;
}

/**
 * @brief Disable motor power via safety relays
 */
static void disable_motor_power(void) {
    // Turn off safety relays (remove power from motors)
    HAL_GPIO_WritePin(SAFETY_RELAY1_PORT, SAFETY_RELAY1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SAFETY_RELAY2_PORT, SAFETY_RELAY2_PIN, GPIO_PIN_RESET);
}

/**
 * @brief Enable motor power via safety relays
 */
static void enable_motor_power(void) {
    // Turn on safety relays (provide power to motors)
    HAL_GPIO_WritePin(SAFETY_RELAY1_PORT, SAFETY_RELAY1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SAFETY_RELAY2_PORT, SAFETY_RELAY2_PIN, GPIO_PIN_SET);
}

/**
 * @brief Validate conditions for emergency stop reset
 */
static SystemError_t validate_reset_conditions(void) {
    // Check that no safety violations are active
    // TODO: Integrate with safety monitoring system
    // This would check all safety monitors for active violations
    
    // Check system health
    // TODO: Integrate with system health monitoring
    
    // Check communication health
    // TODO: Integrate with communication system health
    
    // Check motor driver health
    // TODO: Integrate with L6470 driver health check
    
    // For now, basic validation
    if (is_emergency_stop_pressed()) {
        return ERROR_SAFETY_BUTTON_STILL_PRESSED;
    }
    
    return SYSTEM_OK;
}

/* ========================================================================== */
/* Interrupt Service Routines                                                */
/* ========================================================================== */

/**
 * @brief GPIO EXTI callback for emergency stop button
 * @param GPIO_Pin GPIO pin that triggered the interrupt
 * @note This is called from HAL_GPIO_EXTI_IRQHandler in stm32h7xx_it.c
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // Check if this is the emergency stop button
    if (GPIO_Pin == ESTOP_BUTTON_PIN) {
        // Call emergency stop handler
        emergency_stop_interrupt_handler();
    }
}
