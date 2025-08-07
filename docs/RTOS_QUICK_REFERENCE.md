# FreeRTOS Quick Reference Guide

## 🚀 **Quick Start**

### **Current Status**
- ✅ **FreeRTOS Infrastructure**: Complete and operational
- ✅ **Scheduler Running**: Default task executing at 1ms intervals
- ✅ **SSOT Configuration**: All parameters centralized and validated
- ⏳ **Custom Tasks**: Ready for implementation (10% remaining)

### **Next Steps** (2-3 hours total)
1. **Add custom tasks** to `Core/Src/main.c` USER CODE sections
2. **Implement task functions** using existing safety/motor/comm code
3. **Build and test** RTOS-enabled firmware

---

## 📋 **SSOT Configuration Reference**

### **Task Priorities** (Higher number = Higher priority)
```c
#define SAFETY_MONITOR_TASK_PRIORITY    (4)  // Highest - safety critical
#define MOTOR_CONTROL_TASK_PRIORITY     (3)  // High - real-time control  
#define CAN_COMM_TASK_PRIORITY          (2)  // Medium - communication
#define TELEMETRY_TASK_PRIORITY         (1)  // Low - status updates
```

### **Stack Sizes** (in words - multiply by 4 for bytes)
```c
#define MOTOR_CONTROL_TASK_STACK_SIZE   (512)  // 2KB
#define SAFETY_MONITOR_TASK_STACK_SIZE  (384)  // 1.5KB
#define CAN_COMM_TASK_STACK_SIZE        (256)  // 1KB
#define TELEMETRY_TASK_STACK_SIZE       (256)  // 1KB
```

### **Timing Periods** (milliseconds)
```c
#define MOTOR_CONTROL_PERIOD_MS         (1)    // 1kHz control loop
#define SAFETY_CHECK_PERIOD_MS          (2)    // 500Hz safety monitoring
#define CAN_POLL_PERIOD_MS              (10)   // 100Hz CAN communication
#define TELEMETRY_PERIOD_MS             (100)  // 10Hz status updates
```

---

## 🔧 **Implementation Templates**

### **1. Add Task Handles** (in `main.c` USER CODE Variables)
```c
/* USER CODE BEGIN Variables */
// Motor Control Task
osThreadId_t motorControlTaskHandle;
const osThreadAttr_t motorControlTask_attributes = {
  .name = "MotorControl",
  .priority = (osPriority_t) MOTOR_CONTROL_TASK_PRIORITY,
  .stack_size = MOTOR_CONTROL_TASK_STACK_SIZE * 4
};

// Safety Monitor Task  
osThreadId_t safetyMonitorTaskHandle;
const osThreadAttr_t safetyMonitorTask_attributes = {
  .name = "SafetyMonitor", 
  .priority = (osPriority_t) SAFETY_MONITOR_TASK_PRIORITY,
  .stack_size = SAFETY_MONITOR_TASK_STACK_SIZE * 4
};
/* USER CODE END Variables */
```

### **2. Add Function Prototypes** (in `main.c` USER CODE FunctionPrototypes)
```c
/* USER CODE BEGIN FunctionPrototypes */
void MotorControlTaskFunction(void *argument);
void SafetyMonitorTaskFunction(void *argument);
/* USER CODE END FunctionPrototypes */
```

### **3. Create Tasks** (in `main.c` USER CODE RTOS_THREADS)
```c
/* USER CODE BEGIN RTOS_THREADS */
// Create motor control task
motorControlTaskHandle = osThreadNew(MotorControlTaskFunction, NULL, &motorControlTask_attributes);

// Create safety monitor task  
safetyMonitorTaskHandle = osThreadNew(SafetyMonitorTaskFunction, NULL, &safetyMonitorTask_attributes);
/* USER CODE END RTOS_THREADS */
```

### **4. Implement Task Functions** (in `main.c` USER CODE 4)

#### **Safety Monitor Task**
```c
/* USER CODE BEGIN 4 */
void SafetyMonitorTaskFunction(void *argument) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for(;;) {
        // Use existing safety_system_task() - zero changes required
        SystemError_t result = safety_system_task();
        
        if (result == ERROR_SAFETY_EMERGENCY_STOP) {
            // Emergency stop handling
            // TODO: Add emergency stop signaling via semaphore
        }
        
        // 500Hz safety monitoring using SSOT configuration  
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SAFETY_CHECK_PERIOD_MS));
    }
}
/* USER CODE END 4 */
```

#### **Motor Control Task**
```c
void MotorControlTaskFunction(void *argument) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for(;;) {
        // TODO: Extract motor control logic from main_application_run()
        // Use existing motor control functions here
        
        // Precise 1kHz timing using SSOT configuration
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(MOTOR_CONTROL_PERIOD_MS));
    }
}
```

---

## 🔍 **Key RTOS Functions**

### **Task Management**
```c
// Create task
osThreadId_t osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr);

// Delay functions
osStatus_t osDelay(uint32_t ticks);                    // Relative delay
osStatus_t osDelayUntil(uint32_t ticks);               // Absolute delay (precise timing)

// Task control
osStatus_t osThreadSuspend(osThreadId_t thread_id);
osStatus_t osThreadResume(osThreadId_t thread_id);
```

### **Timing Conversion**
```c
// Convert milliseconds to ticks
#define pdMS_TO_TICKS(ms) ((ms) * configTICK_RATE_HZ / 1000)

// Example: 1ms = 1 tick (at 1kHz tick rate)
vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
```

### **Queue Operations**
```c
// Create queue
QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize);

// Send/receive
BaseType_t xQueueSend(QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait);
BaseType_t xQueueReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait);
```

### **Semaphore Operations**
```c
// Create semaphores
SemaphoreHandle_t xSemaphoreCreateBinary(void);
SemaphoreHandle_t xSemaphoreCreateMutex(void);

// Give/take
BaseType_t xSemaphoreGive(SemaphoreHandle_t xSemaphore);
BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait);
```

---

## 🚨 **Common Patterns**

### **Periodic Task Pattern**
```c
void PeriodicTaskFunction(void *argument) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for(;;) {
        // Do periodic work
        PerformTaskWork();
        
        // Wait for next period (precise timing)
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(PERIOD_MS));
    }
}
```

### **Event-Driven Task Pattern**
```c
void EventDrivenTaskFunction(void *argument) {
    EventMessage_t message;
    
    for(;;) {
        // Wait for events
        if (xQueueReceive(event_queue, &message, portMAX_DELAY) == pdPASS) {
            ProcessEvent(&message);
        }
    }
}
```

### **Thread-Safe Hardware Access**
```c
SystemError_t SafeHardwareAccess(void) {
    if (xSemaphoreTake(hardware_mutex, pdMS_TO_TICKS(TIMEOUT_MS)) == pdTRUE) {
        SystemError_t result = HardwareOperation();
        xSemaphoreGive(hardware_mutex);
        return result;
    }
    return ERROR_TIMEOUT;
}
```

---

## 🛠️ **Build and Debug**

### **Build Commands**
```bash
# Build RTOS-enabled firmware
./scripts/fix_cmake.sh

# Build with status update
cmake --build build --target update-status
```

### **Debugging with RTOS**
```c
// Enable FreeRTOS debugging features
#define configUSE_TRACE_FACILITY                1
#define configUSE_STATS_FORMATTING_FUNCTIONS    1
#define INCLUDE_uxTaskGetStackHighWaterMark     1

// Debug functions
void vTaskList(char *pcWriteBuffer);              // List all tasks
void vTaskGetRunTimeStats(char *pcWriteBuffer);   // Runtime statistics
UBaseType_t uxTaskGetStackHighWaterMark(TaskHandle_t xTask);  // Stack usage
```

### **Memory Monitoring**
```c
// Heap monitoring
size_t xPortGetFreeHeapSize(void);                // Current free heap
size_t xPortGetMinimumEverFreeHeapSize(void);     // Minimum free heap ever
```

---

## ⚠️ **Important Notes**

### **Migration from Bare-Metal**
- **Keep existing functions**: Wrap them in RTOS tasks, don't rewrite
- **Use SSOT parameters**: All timing and priorities from configuration headers
- **Thread-safe access**: Protect shared resources with mutexes
- **Emergency stop**: Maintain <1ms response time with highest priority task

### **Performance Requirements**
- **Motor Control**: Must maintain 1kHz timing (1ms ±10µs)
- **Safety Response**: <1ms emergency stop response
- **Memory Usage**: RTOS overhead <10% of current flash usage
- **Stack Monitoring**: Check stack high water marks regularly

### **Common Pitfalls**
- **Don't use HAL_Delay()** in tasks - use `osDelay()` or `vTaskDelayUntil()`
- **Protect shared resources** with mutexes (SPI, I2C, global variables)
- **Check return values** of RTOS API calls
- **Use correct stack sizes** from SSOT configuration

---

## 📚 **File Locations**

### **Configuration Files**
- **SSOT Config**: `src/config/freertos_config_ssot.h`
- **Resource Declarations**: `src/config/rtos_resources_config.h`
- **FreeRTOS Config**: `Core/Inc/FreeRTOSConfig.h`

### **Implementation Files**
- **Main Integration**: `Core/Src/main.c` (USER CODE sections)
- **RTOS Application**: `Core/Src/freertos.c` (generated)
- **Task Functions**: Add to `Core/Src/main.c` USER CODE 4

### **Documentation**
- **Complete Guide**: `docs/RTOS_SYSTEM_DOCUMENTATION.md`
- **Implementation Plan**: `docs/PHASE1_FREERTOS_IMPLEMENTATION_PLAN.md`
- **This Quick Reference**: `docs/RTOS_QUICK_REFERENCE.md`

---

## 🎯 **Success Checklist**

- [ ] Custom tasks added to `main.c` USER CODE sections
- [ ] Task functions implemented using existing code
- [ ] Build completes without errors
- [ ] All tasks show "Running" state in debugger
- [ ] Motor control maintains 1kHz timing
- [ ] Safety system maintains <1ms response
- [ ] Memory usage within acceptable limits
- [ ] Emergency stop functionality preserved

---

**Quick Reference Version**: 1.0  
**Last Updated**: August 07, 2025  
**For detailed information**: See `docs/RTOS_SYSTEM_DOCUMENTATION.md`
