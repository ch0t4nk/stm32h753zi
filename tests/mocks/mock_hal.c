/**
 * @file mock_hal.c
 * @brief Mock implementations for STM32H7 HAL functions for unit testing
 * @author STM32H753ZI Motor Control Project
 * @date 2024
 *
 * Mock HAL implementation for isolated unit testing of safety systems
 * without requiring actual STM32H7 hardware.
 */

#include "mock_hal.h"
#include "hal_abstraction/hal_abstraction.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
/* Rely on declarations from included mock_hal.h / mock_hal_abstraction.h
 * for all abstraction symbols. Avoid repeating prototypes here to prevent
 * conflicting declarations across translation units. */

/* Forward-declare compatibility hooks provided by mock_hal_abstraction.c.
 * These are declared as void* / void to avoid struct redefinition conflicts
 * between the two mock headers in different translation units. We delegate
 * reset/get-state operations to the abstraction mock which maintains the
 * gpio_configured/gpio_states view expected by many tests.
 */
extern void HAL_Abstraction_Mock_Reset(void);
extern void *HAL_Abstraction_Mock_GetState(void);

// =============================================================================
// MOCK GPIO PORT INSTANCES
// =============================================================================

#ifdef UNITY_TESTING
// Mock instances for unit testing
static GPIO_TypeDef mock_gpioa = {0};
static GPIO_TypeDef mock_gpiob = {0};
static GPIO_TypeDef mock_gpioc = {0};
static SysTick_Type mock_systick = {.VAL = 0x00FFFFFF};

GPIO_TypeDef *GPIOA = &mock_gpioa;
GPIO_TypeDef *GPIOB = &mock_gpiob;
GPIO_TypeDef *GPIOC = &mock_gpioc;
SysTick_Type *SysTick = &mock_systick;
#endif

// =============================================================================
// MOCK STATE VARIABLES
// =============================================================================

/* Use the canonical mock state provided by mock_hal_abstraction.c */
static bool mock_initialized = false;

/* legacy local call counter to preserve historic MockHAL_GetCallCount()
 * without depending on internal mock_hal_state layout. */
static uint32_t legacy_call_count = 0;

static MockHAL_State_t *get_shared_mock_state(void) {
    return MockHAL_GetState();
}

/* Ensure the abstraction mock is initialized and keep a local flag so
 * legacy helpers avoid repeated initialization. The real MockHAL_Init is
 * implemented in mock_hal_abstraction.c; call it and update local state.
 */
static void ensure_initialized(void) {
    if (!mock_initialized) {
        MockHAL_Init();
        mock_initialized = true;
        legacy_call_count = 0;
    }
}

/* Local legacy flags used by some older mock helpers to avoid depending on
 * internal fields in mock_hal_abstraction's MockHAL_Internal_State_t. */
static bool legacy_estop_state = false;
static bool legacy_fault_pin_state = false;

// =============================================================================
// MOCK HAL INITIALIZATION
// =============================================================================

/* MockHAL_Init / MockHAL_Reset / MockHAL_GetState are provided by
 * mock_hal_abstraction.c to ensure a single canonical implementation.
 * Do not redefine them here to avoid duplicate symbols. */

// =============================================================================
// MOCK GPIO FUNCTIONS
// =============================================================================

void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init) {
    if (!mock_initialized)
        MockHAL_Init();

    MockHAL_State_t *state = get_shared_mock_state();
    (void)state; // suppress unused when not needed

    // Mock GPIO initialization - just track that it was called
    // In a real mock, we might validate pin configuration
    (void)GPIOx; // Suppress unused parameter warning
    (void)GPIO_Init;
}

void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                       GPIO_PinState PinState) {
    if (!mock_initialized)
        MockHAL_Init();
    legacy_call_count++;

    HAL_GPIO_Port_t port = HAL_GPIO_PORT_A;
    if (GPIOx == GPIOA)
        port = HAL_GPIO_PORT_A;
    else if (GPIOx == GPIOB)
        port = HAL_GPIO_PORT_B;
    else if (GPIOx == GPIOC)
        port = HAL_GPIO_PORT_C;

    HAL_GPIO_State_t abstract_state =
        (PinState == GPIO_PIN_SET) ? HAL_GPIO_STATE_SET : HAL_GPIO_STATE_RESET;

    /* Delegate to abstraction helper which updates the canonical mock. */
    HAL_Abstraction_Mock_SetGPIOState(port, (uint32_t)GPIO_Pin,
                                      abstract_state);
}

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    if (!mock_initialized)
        MockHAL_Init();
    HAL_GPIO_Port_t port = HAL_GPIO_PORT_A;
    if (GPIOx == GPIOA)
        port = HAL_GPIO_PORT_A;
    else if (GPIOx == GPIOB)
        port = HAL_GPIO_PORT_B;
    else if (GPIOx == GPIOC)
        port = HAL_GPIO_PORT_C;

    HAL_GPIO_State_t state_val = HAL_GPIO_STATE_RESET;
    if (HAL_Abstraction_GPIO_Read(port, (uint32_t)GPIO_Pin, &state_val) ==
        SYSTEM_OK) {
        return (state_val == HAL_GPIO_STATE_SET) ? GPIO_PIN_SET
                                                 : GPIO_PIN_RESET;
    }
    return GPIO_PIN_RESET;
}

// =============================================================================
// MOCK TIMER FUNCTIONS
// =============================================================================

uint32_t HAL_GetTick(void) {
    if (!mock_initialized)
        MockHAL_Init();
    MockHAL_State_t *s = get_shared_mock_state();
    return s ? s->system_tick : 0;
}

void HAL_Delay(uint32_t Delay) {
    if (!mock_initialized)
        MockHAL_Init();
    /* Use abstraction mock delay to keep canonical state consistent */
    HAL_Abstraction_Delay(Delay);
}

// =============================================================================
// MOCK WATCHDOG FUNCTIONS
// =============================================================================

HAL_StatusTypeDef HAL_IWDG_Refresh(IWDG_HandleTypeDef *hiwdg) {
    if (!mock_initialized)
        MockHAL_Init();
    MockHAL_State_t *s = get_shared_mock_state();
    if (s) {
        s->watchdog_refresh_count++;
    }
    legacy_call_count++;
    return HAL_OK;
}

// =============================================================================
// MOCK UTILITY FUNCTIONS
// =============================================================================

void MockHAL_SetTick(uint32_t tick) {
    if (!mock_initialized)
        MockHAL_Init();
    MockHAL_State_t *s = get_shared_mock_state();
    if (s)
        s->system_tick = tick;
}

void MockHAL_AdvanceTick(uint32_t increment) {
    if (!mock_initialized)
        MockHAL_Init();
    MockHAL_State_t *s = get_shared_mock_state();
    if (s)
        s->system_tick += increment;
}

void MockHAL_SetEmergencyStopState(bool active) {
    if (!mock_initialized)
        MockHAL_Init();
    legacy_estop_state = active;
    /* Keep internal abstraction state in sync when available */
    MockHAL_SetEstopActive(active);
}

bool MockHAL_GetEmergencyStopState(void) {
    if (!mock_initialized)
        MockHAL_Init();
    return legacy_estop_state;
}

void MockHAL_SetFaultPinState(bool active) {
    if (!mock_initialized)
        MockHAL_Init();
    legacy_fault_pin_state = active;
    MockHAL_SetFaultPinActive(active);
}

bool MockHAL_WasGPIOWritten(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                            GPIO_PinState expected_state) {
    if (!mock_initialized)
        return false;
    MockHAL_State_t *s = get_shared_mock_state();
    if (!s)
        return false;

    HAL_GPIO_Port_t port = HAL_GPIO_PORT_A;
    if (GPIOx == GPIOA)
        port = HAL_GPIO_PORT_A;
    else if (GPIOx == GPIOB)
        port = HAL_GPIO_PORT_B;
    else if (GPIOx == GPIOC)
        port = HAL_GPIO_PORT_C;

    int pin_index = -1;
    if (GPIO_Pin < MOCK_GPIO_INDEX_SPACE) {
        pin_index = (int)GPIO_Pin;
    } else {
        uint32_t mask = GPIO_Pin;
        if ((mask != 0) && ((mask & (mask - 1UL)) == 0UL)) {
            int idx = 0;
            while ((mask & 1u) == 0u) {
                mask >>= 1;
                idx++;
                if (idx >= (int)MOCK_GPIO_INDEX_SPACE)
                    return false;
            }
            pin_index = idx;
        } else {
            return false;
        }
    }

    if (pin_index < 0 || pin_index >= MOCK_GPIO_INDEX_SPACE)
        return false;

    HAL_GPIO_State_t found = s->gpio_states[port][pin_index];
    HAL_GPIO_State_t expect = (expected_state == GPIO_PIN_SET)
                                  ? HAL_GPIO_STATE_SET
                                  : HAL_GPIO_STATE_RESET;
    return found == expect;
}

uint32_t MockHAL_GetCallCount(void) {
    return legacy_call_count;
}

uint32_t MockHAL_GetWatchdogRefreshCount(void) {
    MockHAL_State_t *s = get_shared_mock_state();
    return s ? s->watchdog_refresh_count : 0;
}

void MockHAL_PrintState(void) {
    MockHAL_State_t *s = get_shared_mock_state();
    printf("=== Mock HAL State ===\n");
    if (s) {
        printf("System tick: %lu\n", (unsigned long)s->system_tick);
        printf("Call count: %lu\n", (unsigned long)legacy_call_count);
        printf("Watchdog refreshes: %lu\n",
               (unsigned long)s->watchdog_refresh_count);
    } else {
        printf("Mock state unavailable\n");
    }
    printf("=====================\n");
}
