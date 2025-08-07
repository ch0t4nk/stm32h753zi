```instructions
---
applyTo: "**/*.{c,h,md,cmake,txt,json,yml,yaml}"
description: "FreeRTOS integration guidance for STM32H753ZI stepper motor control project - Phase 5A roadmap"
---

# FreeRTOS Integration Instructions

## Overview
This instruction file provides comprehensive guidance for integrating FreeRTOS into the STM32H753ZI stepper motor control project. This represents **Phase 5A** of the project roadmap - transitioning from bare metal to RTOS-based architecture while maintaining our HAL abstraction layer and SSOT configuration system.

**PROJECT STATUS**: 🎉 Core project (Phases 1-4) COMPLETE. Phase 5A is the FreeRTOS integration roadmap.

## FreeRTOS Integration Strategy

### 🎯 Phase 5A Objectives
- **Migrate to Task-Based Architecture**: Convert current main loop to FreeRTOS tasks
- **Maintain HAL Abstraction**: All hardware access continues through `hal_abstraction.h`
- **Preserve SSOT Configuration**: FreeRTOS settings integrated into existing config system
- **Thread-Safe Communication**: Protect shared resources with mutexes/semaphores
- **Real-Time Performance**: Maintain deterministic control loop timing
- **Testing Compatibility**: Ensure FreeRTOS code works with our mock testing framework

### 🏗️ Target Architecture
```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                        │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────┐ │
│  │ Motor Control   │  │ Communication   │  │ Safety      │ │
│  │ Task            │  │ Tasks           │  │ Monitor     │ │
│  │ (High Priority) │  │ (Medium Prio)   │  │ (Highest)   │ │
│  └─────────────────┘  └─────────────────┘  └─────────────┘ │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                     FreeRTOS Kernel                        │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────┐ │
│  │ Task Scheduler  │  │ Queue/Semaphore │  │ Timer       │ │
│  │                 │  │ Management      │  │ Services    │ │
│  └─────────────────┘  └─────────────────┘  └─────────────┘ │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                  HAL Abstraction Layer                     │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────┐ │
│  │ GPIO/SPI/I2C    │  │ Timers/PWM      │  │ Interrupts  │ │
│  │ Thread-Safe     │  │ Thread-Safe     │  │ RTOS-aware  │ │
│  └─────────────────┘  └─────────────────┘  └─────────────┘ │
└─────────────────────────────────────────────────────────────┘
┌─────────────────────────────────────────────────────────────┐
│                    STM32H7 Hardware                        │
└─────────────────────────────────────────────────────────────┘
```

## SSOT Configuration for FreeRTOS

### Required Configuration Files
**Create these new files in `src/config/`:**

```c
// src/config/freertos_config_ssot.h
#ifndef FREERTOS_CONFIG_SSOT_H
#define FREERTOS_CONFIG_SSOT_H

// FreeRTOS Core Configuration (SSOT)
#define RTOS_HEAP_SIZE_BYTES           (32 * 1024)    // 32KB heap
#define RTOS_MAX_TASK_PRIORITIES       (5)            // 0-4 priority levels
#define RTOS_TICK_RATE_HZ              (1000)         // 1ms tick
#define RTOS_MINIMAL_STACK_SIZE        (128)          // 128 words minimum
#define RTOS_TOTAL_HEAP_SIZE           RTOS_HEAP_SIZE_BYTES

// Task Stack Sizes (SSOT)
#define MOTOR_CONTROL_TASK_STACK_SIZE  (512)          // 512 words
#define CAN_COMM_TASK_STACK_SIZE       (256)          // 256 words  
#define UART_COMM_TASK_STACK_SIZE      (256)          // 256 words
#define SAFETY_MONITOR_TASK_STACK_SIZE (384)          // 384 words
#define IDLE_TASK_STACK_SIZE           (128)          // 128 words

// Task Priorities (SSOT) - Higher number = higher priority
#define SAFETY_MONITOR_TASK_PRIORITY   (4)            // Highest priority
#define MOTOR_CONTROL_TASK_PRIORITY    (3)            // High priority  
#define CAN_COMM_TASK_PRIORITY         (2)            // Medium priority
#define UART_COMM_TASK_PRIORITY        (1)            // Low priority
#define IDLE_TASK_PRIORITY             (0)            // Lowest priority

// Queue Sizes (SSOT)
#define MOTOR_COMMAND_QUEUE_SIZE       (8)            // 8 command entries
#define CAN_MESSAGE_QUEUE_SIZE         (16)           // 16 CAN messages
#define UART_MESSAGE_QUEUE_SIZE        (8)            // 8 UART messages
#define SAFETY_EVENT_QUEUE_SIZE        (4)            // 4 safety events

// Timing Configuration (SSOT)
#define MOTOR_CONTROL_PERIOD_MS        (10)           // 10ms control loop
#define SAFETY_CHECK_PERIOD_MS         (5)            // 5ms safety checks
#define CAN_POLL_PERIOD_MS             (20)           // 20ms CAN polling
#define UART_POLL_PERIOD_MS            (50)           // 50ms UART polling

#endif // FREERTOS_CONFIG_SSOT_H
```

```c
// src/config/rtos_resources_config.h  
#ifndef RTOS_RESOURCES_CONFIG_H
#define RTOS_RESOURCES_CONFIG_H

#include "freertos_config_ssot.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Task Handles (Declared here, defined in tasks.c)
extern TaskHandle_t motor_control_task_handle;
extern TaskHandle_t safety_monitor_task_handle;
extern TaskHandle_t can_comm_task_handle;
extern TaskHandle_t uart_comm_task_handle;

// Queue Handles
extern QueueHandle_t motor_command_queue;
extern QueueHandle_t can_message_queue;
extern QueueHandle_t uart_message_queue;
extern QueueHandle_t safety_event_queue;

// Semaphore/Mutex Handles
extern SemaphoreHandle_t spi_mutex;          // Protect SPI bus access
extern SemaphoreHandle_t motor_state_mutex;  // Protect motor state variables
extern SemaphoreHandle_t error_log_mutex;    // Protect error logging

// Binary Semaphores for ISR-to-Task Communication
extern SemaphoreHandle_t motor_timer_semaphore;
extern SemaphoreHandle_t encoder_interrupt_semaphore;

#endif // RTOS_RESOURCES_CONFIG_H
```

### FreeRTOS Configuration File
**Update/create `Core/Inc/FreeRTOSConfig.h` with SSOT integration:**

```c
// Core/Inc/FreeRTOSConfig.h
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

// Include our SSOT configuration
#include "freertos_config_ssot.h"

// FreeRTOS Configuration - Values from SSOT
#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCPU_CLOCK_HZ                      SystemCoreClock
#define configTICK_RATE_HZ                      RTOS_TICK_RATE_HZ
#define configMAX_PRIORITIES                    RTOS_MAX_TASK_PRIORITIES
#define configMINIMAL_STACK_SIZE                RTOS_MINIMAL_STACK_SIZE
#define configTOTAL_HEAP_SIZE                   RTOS_TOTAL_HEAP_SIZE
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_TRACE_FACILITY                1
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_MUTEXES                       1
#define configQUEUE_REGISTRY_SIZE               8
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_MALLOC_FAILED_HOOK            1
#define configUSE_APPLICATION_TASK_TAG          0
#define configUSE_COUNTING_SEMAPHORES           1
#define configGENERATE_RUN_TIME_STATS           0

// STM32H7-specific settings
#define configUSE_TICKLESS_IDLE                 0
#define configPRE_SLEEP_PROCESSING(x)           
#define configPOST_SLEEP_PROCESSING(x)          

// Interrupt priorities (must not use HAL priority levels)
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY   15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << 4)
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << 4)

// Task function prototypes
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskCleanUpResources           1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1

// Assert function for debug builds
#ifdef DEBUG
#define configASSERT(x) if((x) == 0) {taskDISABLE_INTERRUPTS(); for(;;);}
#endif

#endif // FREERTOS_CONFIG_H
```

## Task Architecture Design

### Motor Control Task (High Priority)
**Primary real-time control task:**

```c
// src/application/motor_control_task.c
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hal_abstraction.h"
#include "rtos_resources_config.h"
#include "motor_controller.h"

void motor_control_task(void *pvParameters) {
    MotorCommand_t command;
    TickType_t last_wake_time = xTaskGetTickCount();
    
    // Initialize motor control system using HAL abstraction
    SystemError_t init_result = motor_controller_init();
    if (init_result != SYSTEM_OK) {
        // Signal error to safety monitor
        safety_event_t event = {.type = SAFETY_MOTOR_INIT_FAILED, .error_code = init_result};
        xQueueSend(safety_event_queue, &event, 0);
        vTaskSuspend(NULL);  // Suspend this task if init fails
    }
    
    while (1) {
        // Check for new commands (non-blocking)
        if (xQueueReceive(motor_command_queue, &command, 0) == pdTRUE) {
            // Process new motor command
            SystemError_t result = motor_controller_process_command(&command);
            if (result != SYSTEM_OK) {
                // Log error without blocking
                // Note: Error logging should be thread-safe
            }
        }
        
        // Execute periodic control loop using HAL abstraction
        xSemaphoreTake(motor_state_mutex, portMAX_DELAY);
        SystemError_t control_result = motor_controller_update();
        xSemaphoreGive(motor_state_mutex);
        
        if (control_result != SYSTEM_OK) {
            // Signal safety monitor
            safety_event_t event = {.type = SAFETY_CONTROL_ERROR, .error_code = control_result};
            xQueueSend(safety_event_queue, &event, 0);
        }
        
        // Wait for next period (precise timing)
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MOTOR_CONTROL_PERIOD_MS));
    }
}
```

### Safety Monitor Task (Highest Priority)
**Critical safety oversight task:**

```c
// src/safety/safety_monitor_task.c
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hal_abstraction.h"
#include "rtos_resources_config.h"
#include "safety_monitor.h"

void safety_monitor_task(void *pvParameters) {
    safety_event_t event;
    TickType_t last_wake_time = xTaskGetTickCount();
    
    // Initialize safety systems using HAL abstraction
    SystemError_t init_result = safety_monitor_init();
    if (init_result != SYSTEM_OK) {
        // Critical failure - emergency stop
        HAL_Abstraction_GPIO_Write(EMERGENCY_STOP_PORT, EMERGENCY_STOP_PIN, HAL_GPIO_STATE_SET);
        // Log critical error and halt system
        for (;;) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
    
    while (1) {
        // Check for safety events (non-blocking)
        while (xQueueReceive(safety_event_queue, &event, 0) == pdTRUE) {
            SystemError_t response = safety_monitor_handle_event(&event);
            if (response == SYSTEM_ERROR_SAFETY_CRITICAL) {
                // Immediate emergency stop
                HAL_Abstraction_GPIO_Write(EMERGENCY_STOP_PORT, EMERGENCY_STOP_PIN, HAL_GPIO_STATE_SET);
                // Suspend motor control task
                vTaskSuspend(motor_control_task_handle);
            }
        }
        
        // Periodic safety checks using HAL abstraction
        SystemError_t safety_status = safety_monitor_periodic_check();
        if (safety_status != SYSTEM_OK) {
            // Handle safety issue based on severity
            safety_monitor_handle_periodic_error(safety_status);
        }
        
        // Wait for next period
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SAFETY_CHECK_PERIOD_MS));
    }
}
```

### Communication Tasks
**Thread-safe protocol handling:**

```c
// src/communication/can_comm_task.c
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hal_abstraction.h"
#include "rtos_resources_config.h"
#include "can_protocol.h"

void can_comm_task(void *pvParameters) {
    CANMessage_t message;
    TickType_t last_wake_time = xTaskGetTickCount();
    
    // Initialize CAN using HAL abstraction
    SystemError_t init_result = can_protocol_init();
    if (init_result != SYSTEM_OK) {
        // Signal initialization failure
        safety_event_t event = {.type = SAFETY_CAN_INIT_FAILED, .error_code = init_result};
        xQueueSend(safety_event_queue, &event, 0);
        vTaskSuspend(NULL);
    }
    
    while (1) {
        // Check for incoming CAN messages using HAL abstraction
        SystemError_t rx_result = can_protocol_receive(&message);
        if (rx_result == SYSTEM_OK) {
            // Process received message
            MotorCommand_t motor_cmd;
            if (can_protocol_parse_motor_command(&message, &motor_cmd) == SYSTEM_OK) {
                // Send to motor control task
                if (xQueueSend(motor_command_queue, &motor_cmd, pdMS_TO_TICKS(10)) != pdTRUE) {
                    // Queue full - log warning
                }
            }
        }
        
        // Check for outgoing messages
        if (xQueueReceive(can_message_queue, &message, 0) == pdTRUE) {
            SystemError_t tx_result = can_protocol_transmit(&message);
            if (tx_result != SYSTEM_OK) {
                // Log transmission error
            }
        }
        
        // Wait for next period
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(CAN_POLL_PERIOD_MS));
    }
}
```

## HAL Abstraction RTOS Integration

### Thread-Safe HAL Abstraction
**Modify HAL abstraction for thread safety:**

```c
// src/hal_abstraction/hal_abstraction_rtos.h
#ifndef HAL_ABSTRACTION_RTOS_H
#define HAL_ABSTRACTION_RTOS_H

#include "hal_abstraction.h"
#include "FreeRTOS.h"
#include "semphr.h"

// Thread-safe SPI operations
SystemError_t HAL_Abstraction_SPI_Transaction_ThreadSafe(
    HAL_SPI_Bus_t bus,
    const HAL_SPI_Transaction_t* transaction
);

// Thread-safe I2C operations  
SystemError_t HAL_Abstraction_I2C_MemRead_ThreadSafe(
    HAL_I2C_Bus_t bus,
    uint16_t device_addr,
    uint16_t mem_addr,
    uint8_t* data,
    uint16_t size
);

// Critical section macros for HAL abstraction
#define HAL_ABSTRACTION_ENTER_CRITICAL()  taskENTER_CRITICAL()
#define HAL_ABSTRACTION_EXIT_CRITICAL()   taskEXIT_CRITICAL()

// Initialize RTOS integration for HAL abstraction
SystemError_t HAL_Abstraction_RTOS_Init(void);

#endif // HAL_ABSTRACTION_RTOS_H
```

```c
// src/hal_abstraction/hal_abstraction_rtos.c
#include "hal_abstraction_rtos.h"
#include "rtos_resources_config.h"

SystemError_t HAL_Abstraction_RTOS_Init(void) {
    // Create mutexes for shared resources
    spi_mutex = xSemaphoreCreateMutex();
    if (spi_mutex == NULL) {
        return SYSTEM_ERROR_MEMORY_ALLOCATION;
    }
    
    // Create other synchronization objects
    motor_timer_semaphore = xSemaphoreCreateBinary();
    if (motor_timer_semaphore == NULL) {
        return SYSTEM_ERROR_MEMORY_ALLOCATION;
    }
    
    encoder_interrupt_semaphore = xSemaphoreCreateBinary();
    if (encoder_interrupt_semaphore == NULL) {
        return SYSTEM_ERROR_MEMORY_ALLOCATION;
    }
    
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_SPI_Transaction_ThreadSafe(
    HAL_SPI_Bus_t bus,
    const HAL_SPI_Transaction_t* transaction) {
    
    // Take SPI mutex with timeout
    if (xSemaphoreTake(spi_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return SYSTEM_ERROR_TIMEOUT;
    }
    
    // Perform SPI transaction using existing HAL abstraction
    SystemError_t result = HAL_Abstraction_SPI_Transaction(bus, transaction);
    
    // Release mutex
    xSemaphoreGive(spi_mutex);
    
    return result;
}
```

### Interrupt Service Routine Integration
**RTOS-aware ISR handlers:**

```c
// src/hal_abstraction/hal_abstraction_stm32h7_rtos.c
#include "hal_abstraction_rtos.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Timer interrupt for motor control timing
void TIM1_UP_TIM10_IRQHandler(void) {
    BaseType_t higher_priority_task_woken = pdFALSE;
    
    // Clear timer interrupt flag
    if (__HAL_TIM_GET_FLAG(&htim1, TIM_FLAG_UPDATE) != RESET) {
        __HAL_TIM_CLEAR_IT(&htim1, TIM_IT_UPDATE);
        
        // Signal motor control task
        xSemaphoreGiveFromISR(motor_timer_semaphore, &higher_priority_task_woken);
    }
    
    // Yield to higher priority task if necessary
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

// Encoder interrupt handler
void EXTI9_5_IRQHandler(void) {
    BaseType_t higher_priority_task_woken = pdFALSE;
    
    // Check if encoder interrupt
    if (__HAL_GPIO_EXTI_GET_IT(ENCODER_A_PIN) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(ENCODER_A_PIN);
        
        // Signal task waiting for encoder data
        xSemaphoreGiveFromISR(encoder_interrupt_semaphore, &higher_priority_task_woken);
    }
    
    portYIELD_FROM_ISR(higher_priority_task_woken);
}
```

## Testing Strategy for FreeRTOS

### Mock FreeRTOS for Host Testing
**Enable FreeRTOS code testing without RTOS:**

```c
// tests/mocks/freertos_mock.h
#ifndef FREERTOS_MOCK_H
#define FREERTOS_MOCK_H

#include <stdint.h>
#include <stdbool.h>

// Mock FreeRTOS types
typedef void* TaskHandle_t;
typedef void* QueueHandle_t;
typedef void* SemaphoreHandle_t;
typedef uint32_t TickType_t;
typedef uint32_t BaseType_t;

// Mock FreeRTOS constants
#define pdTRUE          1
#define pdFALSE         0
#define portMAX_DELAY   0xFFFFFFFF
#define pdMS_TO_TICKS(ms) (ms)

// Mock FreeRTOS functions
BaseType_t xQueueSend(QueueHandle_t queue, const void* item, TickType_t ticks);
BaseType_t xQueueReceive(QueueHandle_t queue, void* buffer, TickType_t ticks);
BaseType_t xSemaphoreTake(SemaphoreHandle_t semaphore, TickType_t ticks);
BaseType_t xSemaphoreGive(SemaphoreHandle_t semaphore);
void vTaskDelay(TickType_t ticks);
TickType_t xTaskGetTickCount(void);

// Mock critical sections (no-op for testing)
#define taskENTER_CRITICAL()
#define taskEXIT_CRITICAL()

#endif // FREERTOS_MOCK_H
```

```c
// tests/unit/test_motor_control_task.c
#include "unity.h"
#include "freertos_mock.h"  // Use mock instead of real FreeRTOS
#include "hal_abstraction_mock.h"
#include "motor_control_task.h"

void setUp(void) {
    // Setup mock HAL and FreeRTOS
    MockHAL_Reset();
    MockFreeRTOS_Reset();
}

void test_motor_control_task_initialization(void) {
    // Configure mock HAL to simulate successful initialization
    MockHAL_SetMotorInitResult(SYSTEM_OK);
    
    // Test motor control task initialization logic
    // (Task function called directly, not as RTOS task)
    motor_control_task_init();
    
    // Verify initialization sequence
    TEST_ASSERT_TRUE(MockHAL_WasMotorInitCalled());
}

void test_motor_command_processing(void) {
    MotorCommand_t test_command = {
        .motor_id = MOTOR_1,
        .command_type = MOTOR_CMD_MOVE_TO_POSITION,
        .target_position = 1000
    };
    
    // Queue mock command
    MockFreeRTOS_QueueCommand(&test_command);
    
    // Execute one iteration of task loop
    motor_control_task_iteration();
    
    // Verify command was processed
    TEST_ASSERT_TRUE(MockHAL_WasMotorMoveCommandCalled());
}
```

### Integration Testing with FreeRTOS
**Test actual RTOS behavior:**

```c
// tests/integration/test_task_communication.c
#include "unity.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "motor_control_task.h"
#include "can_comm_task.h"

void test_can_to_motor_communication(void) {
    // Create test queues
    QueueHandle_t test_motor_queue = xQueueCreate(MOTOR_COMMAND_QUEUE_SIZE, sizeof(MotorCommand_t));
    QueueHandle_t test_can_queue = xQueueCreate(CAN_MESSAGE_QUEUE_SIZE, sizeof(CANMessage_t));
    
    // Start tasks (integration test with actual FreeRTOS)
    xTaskCreate(motor_control_task, "MotorControl", MOTOR_CONTROL_TASK_STACK_SIZE, NULL, MOTOR_CONTROL_TASK_PRIORITY, NULL);
    xTaskCreate(can_comm_task, "CANComm", CAN_COMM_TASK_STACK_SIZE, NULL, CAN_COMM_TASK_PRIORITY, NULL);
    
    // Send test CAN message
    CANMessage_t test_message = {
        .id = 0x123,
        .data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
        .length = 8
    };
    
    xQueueSend(test_can_queue, &test_message, pdMS_TO_TICKS(100));
    
    // Allow tasks to process
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Verify motor command was received
    MotorCommand_t received_command;
    BaseType_t result = xQueueReceive(test_motor_queue, &received_command, pdMS_TO_TICKS(10));
    TEST_ASSERT_EQUAL(pdTRUE, result);
    TEST_ASSERT_EQUAL(MOTOR_1, received_command.motor_id);
}
```

## Memory and Performance Considerations

### Memory Usage Analysis
**Monitor FreeRTOS memory consumption:**

```c
// src/debug/rtos_debug.c
#include "FreeRTOS.h"
#include "task.h"
#include "freertos_config_ssot.h"

typedef struct {
    size_t total_heap_size;
    size_t free_heap_size;
    size_t minimum_ever_free_heap_size;
    UBaseType_t number_of_tasks;
    uint32_t total_stack_usage;
} RTOSMemoryInfo_t;

RTOSMemoryInfo_t get_rtos_memory_info(void) {
    RTOSMemoryInfo_t info;
    
    info.total_heap_size = configTOTAL_HEAP_SIZE;
    info.free_heap_size = xPortGetFreeHeapSize();
    info.minimum_ever_free_heap_size = xPortGetMinimumEverFreeHeapSize();
    info.number_of_tasks = uxTaskGetNumberOfTasks();
    
    // Calculate total stack usage
    TaskStatus_t* task_status_array;
    UBaseType_t array_size = uxTaskGetNumberOfTasks();
    task_status_array = pvPortMalloc(array_size * sizeof(TaskStatus_t));
    
    if (task_status_array != NULL) {
        uxTaskGetSystemState(task_status_array, array_size, NULL);
        
        info.total_stack_usage = 0;
        for (UBaseType_t i = 0; i < array_size; i++) {
            info.total_stack_usage += task_status_array[i].usStackHighWaterMark;
        }
        
        vPortFree(task_status_array);
    }
    
    return info;
}

// Debug function to print memory usage
void print_rtos_memory_usage(void) {
    RTOSMemoryInfo_t info = get_rtos_memory_info();
    
    printf("FreeRTOS Memory Usage:\n");
    printf("  Heap Total: %d bytes\n", info.total_heap_size);
    printf("  Heap Free: %d bytes\n", info.free_heap_size);
    printf("  Heap Min Free: %d bytes\n", info.minimum_ever_free_heap_size);
    printf("  Active Tasks: %d\n", info.number_of_tasks);
    printf("  Total Stack Usage: %d words\n", info.total_stack_usage);
}
```

### Performance Monitoring
**Track real-time performance:**

```c
// src/debug/rtos_performance.c
#include "FreeRTOS.h"
#include "task.h"

typedef struct {
    TickType_t task_start_time;
    TickType_t task_end_time;
    TickType_t max_execution_time;
    uint32_t execution_count;
} TaskPerformanceInfo_t;

static TaskPerformanceInfo_t motor_task_perf = {0};

// Call at start of motor control task iteration
void motor_task_performance_start(void) {
    motor_task_perf.task_start_time = xTaskGetTickCount();
}

// Call at end of motor control task iteration
void motor_task_performance_end(void) {
    motor_task_perf.task_end_time = xTaskGetTickCount();
    TickType_t execution_time = motor_task_perf.task_end_time - motor_task_perf.task_start_time;
    
    if (execution_time > motor_task_perf.max_execution_time) {
        motor_task_perf.max_execution_time = execution_time;
    }
    
    motor_task_perf.execution_count++;
}

// Get performance statistics
TaskPerformanceInfo_t get_motor_task_performance(void) {
    return motor_task_perf;
}
```

## Build System Integration

### CMake FreeRTOS Integration
**Add FreeRTOS to build system:**

```cmake
# Add to CMakeLists.txt

# FreeRTOS Source Files
set(FREERTOS_DIR ${CMAKE_SOURCE_DIR}/external/FreeRTOS)
set(FREERTOS_PORT_DIR ${FREERTOS_DIR}/portable/GCC/ARM_CM7/r0p1)

# FreeRTOS source files
set(FREERTOS_SOURCES
    ${FREERTOS_DIR}/croutine.c
    ${FREERTOS_DIR}/event_groups.c
    ${FREERTOS_DIR}/list.c
    ${FREERTOS_DIR}/queue.c
    ${FREERTOS_DIR}/stream_buffer.c
    ${FREERTOS_DIR}/tasks.c
    ${FREERTOS_DIR}/timers.c
    ${FREERTOS_PORT_DIR}/port.c
    ${CMAKE_SOURCE_DIR}/Core/Src/heap_4.c  # Memory management
)

# FreeRTOS include directories
set(FREERTOS_INCLUDES
    ${FREERTOS_DIR}/include
    ${FREERTOS_PORT_DIR}
    ${CMAKE_SOURCE_DIR}/Core/Inc  # For FreeRTOSConfig.h
)

# Add FreeRTOS to main target
target_sources(${TARGET_NAME} PRIVATE ${FREERTOS_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${FREERTOS_INCLUDES})

# Compiler definitions for FreeRTOS
target_compile_definitions(${TARGET_NAME} PRIVATE
    USE_FREERTOS=1
    configUSE_TICKLESS_IDLE=0
)

# Task source files
set(TASK_SOURCES
    src/application/motor_control_task.c
    src/safety/safety_monitor_task.c
    src/communication/can_comm_task.c
    src/communication/uart_comm_task.c
    src/application/rtos_init.c
)

target_sources(${TARGET_NAME} PRIVATE ${TASK_SOURCES})
```

### Host Testing CMake Configuration
**Build host tests without FreeRTOS:**

```cmake
# CMakeLists.txt for host tests

if(HOST_TESTS)
    # Use FreeRTOS mocks for host testing
    set(FREERTOS_MOCK_DIR ${CMAKE_SOURCE_DIR}/tests/mocks)
    
    target_include_directories(test_target PRIVATE
        ${FREERTOS_MOCK_DIR}
        src/config
        src/hal_abstraction
    )
    
    target_sources(test_target PRIVATE
        ${FREERTOS_MOCK_DIR}/freertos_mock.c
        tests/unit/test_motor_control_task.c
        tests/unit/test_safety_monitor_task.c
    )
    
    # Define host testing mode
    target_compile_definitions(test_target PRIVATE
        HOST_TESTING=1
        USE_FREERTOS_MOCK=1
    )
endif()
```

## Migration Strategy

### Phase 5A-1: Basic Task Structure
**Initial RTOS integration:**
1. ✅ Add FreeRTOS to build system
2. ✅ Create basic task structure (motor control, safety, communication)
3. ✅ Implement SSOT configuration for FreeRTOS
4. ✅ Add HAL abstraction thread safety
5. ✅ Create mock FreeRTOS for testing

### Phase 5A-2: Communication Integration
**Inter-task communication:**
1. ⏳ Implement queues for task communication
2. ⏳ Add mutexes for shared resource protection
3. ⏳ Integrate CAN and UART tasks with existing protocols
4. ⏳ Test task communication and synchronization

### Phase 5A-3: Performance Optimization
**Real-time performance tuning:**
1. ⏳ Optimize task priorities and timing
2. ⏳ Memory usage analysis and optimization
3. ⏳ Performance monitoring and profiling
4. ⏳ Real-time constraints validation

### Phase 5A-4: Complete Integration
**Full RTOS migration:**
1. ⏳ Migrate all existing functionality to tasks
2. ⏳ Comprehensive testing (unit, integration, hardware)
3. ⏳ Documentation updates
4. ⏳ Performance benchmarking vs bare metal

## Best Practices

### RTOS Design Principles
1. **Keep ISRs Minimal**: Use semaphores to defer processing to tasks
2. **Protect Shared Resources**: Always use mutexes for shared data
3. **Design for Determinism**: Use fixed priority scheduling
4. **Monitor Performance**: Track task execution times and stack usage
5. **Handle Errors Gracefully**: Implement proper error recovery in tasks

### HAL Abstraction Integration
1. **Maintain Abstraction**: All hardware access through HAL abstraction layer
2. **Thread-Safe Operations**: Add mutex protection for shared hardware resources
3. **Error Propagation**: Maintain existing error handling patterns
4. **Testing Compatibility**: Ensure RTOS code works with mock HAL

### SSOT Compliance
1. **Configuration Centralization**: All RTOS settings in SSOT config files
2. **Compile-Time Validation**: Use static assertions for configuration validation
3. **Documentation Sync**: Keep configuration documentation current
4. **Automated Validation**: Include RTOS configuration in SSOT validation scripts

Remember: **FreeRTOS integration maintains our professional architecture while adding real-time capabilities. All existing HAL abstraction and SSOT principles remain in place.**

```
