/**
 * @file emergency_stop_abstracted.c
 * @brief Emergency Stop System Implementation with HAL Abstraction
 * @author STM32H753ZI Motor Control Project
 * @date 2024
 *
 * This implementation demonstrates how to use the HAL abstraction layer
 * to create testable, hardware-independent safety-critical code.
 *
 * @note Key benefits of HAL abstraction:
 * - Clean unit testing with mocks
 * - Hardware portability
 * - Professional embedded architecture
 * - Separation of concerns
 */

#include "emergency_stop_abstracted.h"
#include "config/hardware_config.h" // SSOT for hardware pin definitions
#include "config/safety_config.h"
#include "hal_abstraction/hal_abstraction.h"
#include <string.h>

/* ==========================================================================
 */
/* Private Types and Constants                                               */
/* ==========================================================================
 */

#define ESTOP_REACTION_TIME_MS 10   ///< Maximum reaction time (SIL-3)
#define ESTOP_HEARTBEAT_TIME_MS 100 ///< LED heartbeat when armed

/**
 * @brief Emergency stop system internal state
 */
typedef struct {
    EmergencyStopState_t state;        ///< Current state
    EmergencyStopSource_t last_source; ///< Last trigger source
    uint32_t state_entry_time;         ///< Time when current state was entered
    uint32_t last_button_time;  ///< Last button press time (for debounce)
    uint32_t trigger_count;     ///< Total number of triggers
    uint32_t last_trigger_time; ///< Timestamp of last trigger
    bool button_debounced;      ///< Button state after debouncing
    bool led_heartbeat_state;   ///< LED heartbeat state
    bool initialized;           ///< Initialization flag
} EmergencyStopContext_t;

/* ==========================================================================
 */
/* Private Variables                                                         */
/* ==========================================================================
 */

static EmergencyStopContext_t estop_context = {0};

/* ==========================================================================
 */
/* Private Function Declarations                                             */
/* ==========================================================================
 */

static SystemError_t estop_init_hardware(void);
static SystemError_t estop_read_button_state(bool *pressed);
static SystemError_t estop_process_button_debounce(void);
static SystemError_t estop_activate_safety_relays(bool active);
static SystemError_t estop_update_led_indication(void);
static void estop_change_state(EmergencyStopState_t new_state);

/* ==========================================================================
 */
/* Public API Implementation                                                 */
/* ==========================================================================
 */

SystemError_t emergency_stop_init(void) {
    // Reset context
    memset(&estop_context, 0, sizeof(estop_context));

    // Initialize hardware using HAL abstraction
    SystemError_t result = estop_init_hardware();
    if (result != SYSTEM_OK) {
        return result;
    }

    // Initialize state
    estop_context.state_entry_time = HAL_Abstraction_GetTick();
    // Initialize debounce reference to current time to avoid spurious triggers
    estop_context.last_button_time = estop_context.state_entry_time;
    estop_context.initialized = true;
    estop_change_state(EMERGENCY_STOP_ARMED);

    return SYSTEM_OK;
}

SystemError_t emergency_stop_execute(EmergencyStopSource_t source) {
    if (!estop_context.initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Record trigger information
    estop_context.last_source = source;
    estop_context.trigger_count++;
    estop_context.last_trigger_time = HAL_Abstraction_GetTick();

    // Activate safety systems immediately
    SystemError_t result = estop_activate_safety_relays(true);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Change to triggered state
    estop_change_state(EMERGENCY_STOP_TRIGGERED);

    return SYSTEM_OK;
}

SystemError_t emergency_stop_reset(void) {
    if (!estop_context.initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    if (estop_context.state != EMERGENCY_STOP_TRIGGERED) {
        return ERROR_INVALID_STATE;
    }

    // Check if button is still pressed (safety requirement)
    bool button_pressed;
    SystemError_t result = estop_read_button_state(&button_pressed);
    if (result != SYSTEM_OK) {
        return result;
    }

    if (button_pressed) {
        return ERROR_HARDWARE_FAULT; // Cannot reset while button is pressed
    }

    estop_change_state(EMERGENCY_STOP_RESET_PENDING);
    return SYSTEM_OK;
}

SystemError_t emergency_stop_process(void) {
    if (!estop_context.initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    SystemError_t result;

    // Process button debouncing
    result = estop_process_button_debounce();
    if (result != SYSTEM_OK) {
        return result;
    }

    // State machine processing
    switch (estop_context.state) {
    case EMERGENCY_STOP_ARMED:
        // Check for button press
        if (estop_context.button_debounced) {
            result = emergency_stop_execute(ESTOP_SOURCE_BUTTON);
        }
        break;

    case EMERGENCY_STOP_TRIGGERED:
        // Already triggered, maintain safety state
        break;

    case EMERGENCY_STOP_RESET_PENDING: {
        // Check if sufficient time has passed for reset
        uint32_t current_time = HAL_Abstraction_GetTick();
        if ((current_time - estop_context.state_entry_time) >
            1000) { // 1 second delay
            // Deactivate safety relays
            result = estop_activate_safety_relays(false);
            if (result == SYSTEM_OK) {
                estop_change_state(EMERGENCY_STOP_ARMED);
            }
        }
        break;
    }

    default:
        estop_change_state(EMERGENCY_STOP_FAULT);
        return ERROR_INVALID_STATE;
    }

    // Update LED indication
    result = estop_update_led_indication();

    return result;
}

EmergencyStopState_t emergency_stop_get_state(void) {
    return estop_context.state;
}

bool emergency_stop_is_active(void) {
    return (estop_context.state == EMERGENCY_STOP_TRIGGERED);
}

bool emergency_stop_is_button_pressed(void) {
    bool pressed = false;
    estop_read_button_state(&pressed);
    return pressed;
}

bool is_emergency_stop_pressed(void) {
    // Alias for compatibility with safety_system.c
    return emergency_stop_is_button_pressed();
}

void emergency_stop_interrupt_handler(void) {
    // Record interrupt time for debouncing
    estop_context.last_button_time = HAL_Abstraction_GetTick();
}

SystemError_t emergency_stop_self_test(void) {
    if (!estop_context.initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    SystemError_t result;

    // Test LED functionality
    result = emergency_stop_set_led(true);
    if (result != SYSTEM_OK)
        return result;

    HAL_Abstraction_Delay(100);

    result = emergency_stop_set_led(false);
    if (result != SYSTEM_OK)
        return result;

    // Test button reading
    bool button_state;
    result = estop_read_button_state(&button_state);
    if (result != SYSTEM_OK)
        return result;

    // Test safety relay control
    result = estop_activate_safety_relays(true);
    if (result != SYSTEM_OK)
        return result;

    HAL_Abstraction_Delay(50);

    result = estop_activate_safety_relays(false);

    return result;
}

SystemError_t emergency_stop_set_led(bool state) {
    HAL_GPIO_State_t gpio_state =
        state ? HAL_GPIO_STATE_SET : HAL_GPIO_STATE_RESET;
    return HAL_Abstraction_GPIO_Write(ESTOP_LED_PORT, ESTOP_LED_PIN,
                                      gpio_state);
}

SystemError_t emergency_stop_check_health(void) {
    if (!estop_context.initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Check if we're in a valid state
    if (estop_context.state >= EMERGENCY_STOP_FAULT) {
        return ERROR_SYSTEM_FAULT;
    }

    // Verify we can still read the button (HAL healthy)
    bool dummy_pressed = false;
    SystemError_t btn_res = estop_read_button_state(&dummy_pressed);
    if (btn_res != SYSTEM_OK) {
        return ERROR_NOT_INITIALIZED;
    }

    // Check timing constraints
    uint32_t current_time = HAL_Abstraction_GetTick();
    uint32_t state_duration = current_time - estop_context.state_entry_time;

    // Check for stuck states
    if (estop_context.state == EMERGENCY_STOP_RESET_PENDING &&
        state_duration > 5000) {
        return ERROR_TIMEOUT;
    }

    return SYSTEM_OK;
}

EmergencyStopSource_t emergency_stop_get_last_source(void) {
    return estop_context.last_source;
}

SystemError_t emergency_stop_get_statistics(uint32_t *trigger_count,
                                            uint32_t *last_trigger_time) {
    if (trigger_count == NULL || last_trigger_time == NULL) {
        return ERROR_NULL_POINTER;
    }

    *trigger_count = estop_context.trigger_count;
    *last_trigger_time = estop_context.last_trigger_time;

    return SYSTEM_OK;
}

/* ==========================================================================
 */
/* Private Function Implementation                                           */
/* ==========================================================================
 */

static SystemError_t estop_init_hardware(void) {
    SystemError_t result;

    // Configure emergency stop button as input with pull-up
    HAL_GPIO_Config_t button_config = {.pin = ESTOP_BUTTON_PIN,
                                       .mode = HAL_GPIO_MODE_INPUT,
                                       .pull = HAL_GPIO_PULLUP,
                                       .speed = HAL_GPIO_SPEED_FREQ_LOW,
                                       .alternate = 0};

    result = HAL_Abstraction_GPIO_Init(ESTOP_BUTTON_PORT, &button_config);
    if (result != SYSTEM_OK)
        return result;

    // Enable button interrupt
    result = HAL_Abstraction_GPIO_EnableInterrupt(
        ESTOP_BUTTON_PORT, ESTOP_BUTTON_PIN, ESTOP_BUTTON_TRIGGER_TYPE,
        0); // Highest priority
    if (result != SYSTEM_OK)
        return result;

    // Configure LED as output
    HAL_GPIO_Config_t led_config = {.pin = ESTOP_LED_PIN,
                                    .mode = HAL_GPIO_MODE_OUTPUT_PP,
                                    .pull = HAL_GPIO_NOPULL,
                                    .speed = HAL_GPIO_SPEED_FREQ_LOW,
                                    .alternate = 0};

    result = HAL_Abstraction_GPIO_Init(ESTOP_LED_PORT, &led_config);
    if (result != SYSTEM_OK)
        return result;

    // Configure safety relays as outputs
    HAL_GPIO_Config_t relay_config = {.pin = SAFETY_RELAY1_PIN,
                                      .mode = HAL_GPIO_MODE_OUTPUT_PP,
                                      .pull = HAL_GPIO_NOPULL,
                                      .speed = HAL_GPIO_SPEED_FREQ_LOW,
                                      .alternate = 0};

    result = HAL_Abstraction_GPIO_Init(SAFETY_RELAY1_PORT, &relay_config);
    if (result != SYSTEM_OK)
        return result;

    relay_config.pin = SAFETY_RELAY2_PIN;
    result = HAL_Abstraction_GPIO_Init(SAFETY_RELAY2_PORT, &relay_config);

    return result;
}

static SystemError_t estop_read_button_state(bool *pressed) {
    if (pressed == NULL) {
        return ERROR_NULL_POINTER;
    }

    HAL_GPIO_State_t gpio_state;
    SystemError_t result = HAL_Abstraction_GPIO_Read(
        ESTOP_BUTTON_PORT, ESTOP_BUTTON_PIN, &gpio_state);

    if (result == SYSTEM_OK) {
        // Button is active low (pressed when GPIO reads low)
        *pressed = (gpio_state == HAL_GPIO_STATE_RESET);
    }

    return result;
}

static SystemError_t estop_process_button_debounce(void) {
    bool current_button_state;
    SystemError_t result = estop_read_button_state(&current_button_state);
    if (result != SYSTEM_OK) {
        return result;
    }

    uint32_t current_time = HAL_Abstraction_GetTick();

    // Debounce logic
    if (current_button_state) {
        if ((current_time - estop_context.last_button_time) >
            ESTOP_DEBOUNCE_TIME_MS) {
            estop_context.button_debounced = true;
        }
    } else {
        estop_context.button_debounced = false;
        estop_context.last_button_time = current_time;
    }

    return SYSTEM_OK;
}

static SystemError_t estop_activate_safety_relays(bool active) {
    SystemError_t result;
    HAL_GPIO_State_t relay_state =
        active ? HAL_GPIO_STATE_SET : HAL_GPIO_STATE_RESET;

    // Activate/deactivate both safety relays
    result = HAL_Abstraction_GPIO_Write(SAFETY_RELAY1_PORT, SAFETY_RELAY1_PIN,
                                        relay_state);
    if (result != SYSTEM_OK)
        return result;

    result = HAL_Abstraction_GPIO_Write(SAFETY_RELAY2_PORT, SAFETY_RELAY2_PIN,
                                        relay_state);

    return result;
}

static SystemError_t estop_update_led_indication(void) {
    uint32_t current_time = HAL_Abstraction_GetTick();
    SystemError_t result = SYSTEM_OK;

    switch (estop_context.state) {
    case EMERGENCY_STOP_ARMED:
        // Heartbeat indication (slow blink)
        if ((current_time % (ESTOP_HEARTBEAT_TIME_MS * 2)) <
            ESTOP_HEARTBEAT_TIME_MS) {
            result = emergency_stop_set_led(true);
        } else {
            result = emergency_stop_set_led(false);
        }
        break;

    case EMERGENCY_STOP_TRIGGERED:
        // Solid on when triggered
        result = emergency_stop_set_led(true);
        break;

    case EMERGENCY_STOP_RESET_PENDING:
        // Fast blink during reset
        if ((current_time % 200) < 100) { // 200ms period, 50% duty
            result = emergency_stop_set_led(true);
        } else {
            result = emergency_stop_set_led(false);
        }
        break;

    case EMERGENCY_STOP_FAULT:
        // Very fast blink for fault
        if ((current_time % 100) < 50) { // 100ms period, 50% duty
            result = emergency_stop_set_led(true);
        } else {
            result = emergency_stop_set_led(false);
        }
        break;

    default:
        result = emergency_stop_set_led(false);
        break;
    }

    return result;
}

static void estop_change_state(EmergencyStopState_t new_state) {
    estop_context.state = new_state;
    estop_context.state_entry_time = HAL_Abstraction_GetTick();
}
