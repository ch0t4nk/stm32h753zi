#include "config/freertos_config_ssot.h"
#include <stdio.h>

void test_constants(void) {
    printf("Testing constants:\n");
    printf("MOTOR_CONTROL_PERIOD_MS = %d\n", MOTOR_CONTROL_PERIOD_MS);
    printf("MOTOR_CONTROL_TASK_PRIORITY = %d\n", MOTOR_CONTROL_TASK_PRIORITY);
    printf("SAFETY_CHECK_PERIOD_MS = %d\n", SAFETY_CHECK_PERIOD_MS);
    printf("SAFETY_MONITOR_TASK_PRIORITY = %d\n",
           SAFETY_MONITOR_TASK_PRIORITY);
    printf("RTOS_HEAP_SIZE_BYTES = %d\n", RTOS_HEAP_SIZE_BYTES);
}
