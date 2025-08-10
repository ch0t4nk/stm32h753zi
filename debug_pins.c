#include "src/config/hardware_config.h"
#include "src/hal_abstraction/hal_abstraction.h"
#include <stdio.h>

int main() {
    printf("=== SSOT Pin Configuration Test ===\n");
    printf("ESTOP_BUTTON_PORT = %d\n", ESTOP_BUTTON_PORT);
    printf("ESTOP_BUTTON_PIN = %d (bit mask)\n", ESTOP_BUTTON_PIN);
    printf("ESTOP_BUTTON_PIN_INDEX = %d (array index)\n",
           ESTOP_BUTTON_PIN_INDEX);
    printf("HAL_GPIO_PORT_MAX = %d\n", HAL_GPIO_PORT_MAX);
    printf("HAL_GPIO_PORT_C = %d\n", HAL_GPIO_PORT_C);

    // Test SSOT consistency
    printf("\n=== SSOT Consistency Check ===\n");
    printf("USER_BUTTON_PIN = %d (bit mask)\n", USER_BUTTON_PIN);
    printf("USER_BUTTON_PIN_INDEX = %d (array index)\n",
           USER_BUTTON_PIN_INDEX);
    printf("LED_RED_PIN = %d (bit mask)\n", LED_RED_PIN);
    printf("LED_RED_PIN_INDEX = %d (array index)\n", LED_RED_PIN_INDEX);

    return 0;
}
