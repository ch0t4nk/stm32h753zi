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
#include <stdio.h>
#include <string.h>

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

static MockHAL_State_t mock_hal_state = {0};
static bool mock_initialized = false;

// =============================================================================
// MOCK HAL INITIALIZATION
// =============================================================================

void MockHAL_Init(void) {
    memset(&mock_hal_state, 0, sizeof(MockHAL_State_t));
    mock_hal_state.system_tick = 0;
    mock_hal_state.gpio_state_count = 0;
    mock_hal_state.call_count = 0;
    mock_initialized = true;
}

void MockHAL_Reset(void) {
    MockHAL_Init();
}

MockHAL_State_t *MockHAL_GetState(void) {
    return &mock_hal_state;
}

// =============================================================================
// MOCK GPIO FUNCTIONS
// =============================================================================

void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init) {
    if (!mock_initialized)
        MockHAL_Init();

    mock_hal_state.call_count++;

    // Mock GPIO initialization - just track that it was called
    // In a real mock, we might validate pin configuration
    (void)GPIOx; // Suppress unused parameter warning
    (void)GPIO_Init;
}

void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                       GPIO_PinState PinState) {
    if (!mock_initialized)
        MockHAL_Init();

    mock_hal_state.call_count++;

    // Record GPIO state change
    if (mock_hal_state.gpio_state_count < MAX_GPIO_STATES) {
        MockGPIO_State_t *state =
            &mock_hal_state.gpio_states[mock_hal_state.gpio_state_count];
        state->port = GPIOx;
        state->pin = GPIO_Pin;
        state->state = PinState;
        state->timestamp = mock_hal_state.system_tick;
        mock_hal_state.gpio_state_count++;
    }

    // Handle emergency stop pin specifically
    if (GPIOx == GPIOA && GPIO_Pin == GPIO_PIN_0) {
        mock_hal_state.emergency_stop_state = (PinState == GPIO_PIN_SET);
    }
}

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    if (!mock_initialized)
        MockHAL_Init();

    mock_hal_state.call_count++;

    // Return mock emergency stop state
    if (GPIOx == GPIOA && GPIO_Pin == GPIO_PIN_0) {
        return mock_hal_state.emergency_stop_state ? GPIO_PIN_SET
                                                   : GPIO_PIN_RESET;
    }

    // Return mock fault pin state
    if (GPIOx == GPIOB && GPIO_Pin == GPIO_PIN_1) {
        return mock_hal_state.fault_pin_state ? GPIO_PIN_SET : GPIO_PIN_RESET;
    }

    // Default return
    return GPIO_PIN_RESET;
}

// =============================================================================
// MOCK TIMER FUNCTIONS
// =============================================================================

uint32_t HAL_GetTick(void) {
    if (!mock_initialized)
        MockHAL_Init();
    return mock_hal_state.system_tick;
}

void HAL_Delay(uint32_t Delay) {
    if (!mock_initialized)
        MockHAL_Init();
    mock_hal_state.system_tick += Delay;
    mock_hal_state.call_count++;
}

// =============================================================================
// MOCK WATCHDOG FUNCTIONS
// =============================================================================

HAL_StatusTypeDef HAL_IWDG_Refresh(IWDG_HandleTypeDef *hiwdg) {
    if (!mock_initialized)
        MockHAL_Init();

    mock_hal_state.call_count++;
    mock_hal_state.watchdog_refresh_count++;
    mock_hal_state.last_watchdog_refresh = mock_hal_state.system_tick;

    return HAL_OK;
}

// =============================================================================
// MOCK UTILITY FUNCTIONS
// =============================================================================

void MockHAL_SetTick(uint32_t tick) {
    if (!mock_initialized)
        MockHAL_Init();
    mock_hal_state.system_tick = tick;
}

void MockHAL_AdvanceTick(uint32_t increment) {
    if (!mock_initialized)
        MockHAL_Init();
    mock_hal_state.system_tick += increment;
}

void MockHAL_SetEmergencyStopState(bool active) {
    if (!mock_initialized)
        MockHAL_Init();
    mock_hal_state.emergency_stop_state = active;
}

bool MockHAL_GetEmergencyStopState(void) {
    if (!mock_initialized)
        MockHAL_Init();
    return mock_hal_state.emergency_stop_state;
}

void MockHAL_SetFaultPinState(bool active) {
    if (!mock_initialized)
        MockHAL_Init();
    mock_hal_state.fault_pin_state = active;
}

bool MockHAL_WasGPIOWritten(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                            GPIO_PinState expected_state) {
    if (!mock_initialized)
        return false;

    for (int i = 0; i < mock_hal_state.gpio_state_count; i++) {
        MockGPIO_State_t *state = &mock_hal_state.gpio_states[i];
        if (state->port == GPIOx && state->pin == GPIO_Pin &&
            state->state == expected_state) {
            return true;
        }
    }
    return false;
}

uint32_t MockHAL_GetCallCount(void) {
    return mock_hal_state.call_count;
}

uint32_t MockHAL_GetWatchdogRefreshCount(void) {
    return mock_hal_state.watchdog_refresh_count;
}

void MockHAL_PrintState(void) {
    printf("=== Mock HAL State ===\n");
    printf("System tick: %lu\n", mock_hal_state.system_tick);
    printf("Call count: %lu\n", mock_hal_state.call_count);
    printf("Emergency stop: %s\n",
           mock_hal_state.emergency_stop_state ? "ACTIVE" : "INACTIVE");
    printf("Fault pin: %s\n",
           mock_hal_state.fault_pin_state ? "ACTIVE" : "INACTIVE");
    printf("Watchdog refreshes: %lu\n", mock_hal_state.watchdog_refresh_count);
    printf("GPIO state changes: %d\n", mock_hal_state.gpio_state_count);
    printf("=====================\n");
}
