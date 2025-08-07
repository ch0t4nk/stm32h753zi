---
applyTo: "src/rtos/**/*.{c,h}"
description: "Advanced RTOS features including power management, dynamic optimization, standards compliance, and telemetry for STM32H753ZI stepper motor project"
---

# Advanced RTOS Features Instructions

## Overview
This instruction file provides comprehensive guidance for the advanced RTOS features implemented in Phase 5A of the STM32H753ZI stepper motor control project. These features elevate the system to industrial-grade production readiness with sophisticated power management, performance optimization, standards compliance, and real-time monitoring capabilities.

## Phase 5A Industrial Enhancement Features

### 1. **Power Management System** (`src/rtos/power_management.h/.c`)

#### Purpose and Architecture
The power management system provides intelligent CPU frequency scaling, thermal protection, and power optimization for industrial applications requiring energy efficiency without compromising safety or performance.

#### **Core Capabilities**
- **4-Mode Power Architecture**: Active (480MHz) → Quiet (240MHz) → Idle (120MHz) → Sleep (120MHz minimal)
- **Thermal Throttling**: Automatic frequency reduction at 75°C warning, forced idle at 85°C critical
- **Predictive Optimization**: AI-driven power management with historical pattern analysis
- **Safety Compliance**: SIL-2 power management with fail-safe thermal protection
- **Power Savings**: Estimated 50-80% energy reduction in low-activity periods

#### **API Usage Patterns**
```c
// Initialize power management system
PowerManagementConfiguration_t power_config = {
    .thermal_warning_threshold = 75.0f,
    .thermal_critical_threshold = 85.0f,
    .enable_predictive_optimization = true,
    .enable_thermal_throttling = true
};
SystemError_t result = power_management_init(&power_config);

// Signal system activity to prevent unnecessary power reduction
power_management_signal_activity(ACTIVITY_MOTOR_MOTION);
power_management_signal_activity(ACTIVITY_COMMUNICATION);

// Monitor and respond to thermal conditions
float cpu_temp = get_cpu_temperature();
power_management_thermal_check(cpu_temp);

// Request specific power mode for energy optimization
power_management_request_mode(POWER_MODE_QUIET, false);

// Emergency power conservation
if (battery_level_critical()) {
    power_management_emergency_conservation();
}
```

#### **Integration Requirements**
- **FreeRTOS Integration**: Power modes adjust task periods using FreeRTOS period management
- **Thermal Monitoring**: Requires CPU temperature sensor integration
- **Activity Detection**: System activity signals from motor control, communication, and user interface tasks
- **Safety Compliance**: Power management must never compromise safety system operation

#### **Configuration SSOT Integration**
```c
// Power management configuration in freertos_config_ssot.h
#define POWER_ACTIVE_CPU_FREQ_MHZ          480    // Full performance mode
#define POWER_QUIET_CPU_FREQ_MHZ           240    // Reduced performance mode
#define POWER_IDLE_CPU_FREQ_MHZ            120    // Low power mode
#define POWER_THERMAL_WARNING_THRESHOLD    75.0f  // Thermal management threshold
#define POWER_THERMAL_CRITICAL_THRESHOLD   85.0f  // Emergency thermal protection
#define POWER_IDLE_TIMEOUT_MS              10000  // Transition to idle after inactivity
#define POWER_SLEEP_TIMEOUT_MS             30000  // Transition to sleep after extended inactivity
```

### 2. **Dynamic Task Tuning System** (`src/rtos/dynamic_task_tuning.h`)

#### Purpose and Architecture
The dynamic task tuning system provides adaptive performance optimization by automatically adjusting task periods based on CPU utilization, system load, and performance metrics for optimal real-time operation.

#### **Core Capabilities**
- **CPU Utilization Feedback**: Target 60% CPU utilization with ±10% tolerance
- **Intelligent Period Adjustment**: Automatic task period tuning based on system load
- **Performance History**: Predictive analysis using 32-sample execution history
- **Safety Boundaries**: Minimum/maximum period limits to maintain real-time guarantees
- **Emergency Load Reduction**: Automatic load reduction under high CPU stress

#### **API Usage Patterns**
```c
// Initialize dynamic tuning system
dynamic_tuning_init();

// Register tasks for dynamic tuning with safety boundaries
dynamic_tuning_register_task(
    motor_control_task_handle,
    "MotorControl",
    1,    // Base period: 1ms
    1,    // Minimum period: 1ms (never compromise motor control)
    5     // Maximum period: 5ms (maximum allowed for motor control)
);

dynamic_tuning_register_task(
    telemetry_task_handle,
    "Telemetry",
    100,  // Base period: 100ms
    50,   // Minimum period: 50ms
    1000  // Maximum period: 1000ms (flexible for telemetry)
);

// Record task execution times for performance analysis
void motor_control_task_wrapper(void) {
    uint32_t start_time = get_execution_time_us();
    
    // Execute motor control logic
    motor_controller_update();
    
    uint32_t execution_time = get_execution_time_us() - start_time;
    dynamic_tuning_record_execution(motor_control_task_handle, execution_time);
}

// Update tuning algorithm periodically (called from system monitoring task)
dynamic_tuning_update();

// Get performance metrics for monitoring
SystemPerformanceMetrics_t metrics;
dynamic_tuning_get_metrics(&metrics);

// Emergency load reduction if CPU overload detected
if (metrics.cpu_utilization_percent > 95) {
    dynamic_tuning_emergency_load_reduction();
}
```

#### **Integration Requirements**
- **FreeRTOS Task Management**: Direct integration with FreeRTOS task period management
- **Execution Time Measurement**: Requires high-resolution timing (DWT or TIM counters)
- **CPU Utilization Monitoring**: Integration with FreeRTOS runtime statistics
- **Safety Constraints**: Tuning must respect safety-critical task timing requirements

#### **Performance Optimization Guidelines**
```c
// Task registration priorities and constraints
dynamic_tuning_register_task(motor_control_task, "MotorControl", 1, 1, 2);      // Critical: minimal adjustment
dynamic_tuning_register_task(safety_monitor_task, "SafetyMonitor", 2, 2, 2);   // Critical: no adjustment
dynamic_tuning_register_task(can_comm_task, "CANComm", 10, 5, 50);             // Moderate: flexible
dynamic_tuning_register_task(telemetry_task, "Telemetry", 100, 50, 1000);      // Flexible: wide range
```

### 3. **Standards Compliance Framework** (`src/rtos/standards_compliance.h`)

#### Purpose and Architecture
The standards compliance framework provides comprehensive support for industrial standards compliance including MISRA-C, IEC 61508 SIL-2, ISO 26262, and automated documentation for certification-ready embedded systems.

#### **Core Capabilities**
- **MISRA-C:2012 Compliance**: Automated violation tracking with justification framework
- **IEC 61508 SIL-2 Support**: Functional safety compliance with requirement traceability
- **ISO 26262 ASIL Support**: Automotive safety integrity level compliance
- **Automated Reporting**: Compliance reports and documentation for certification authorities
- **Traceability Matrix**: Requirements-to-implementation-to-test mapping

#### **API Usage Patterns**
```c
// Initialize standards compliance framework
ComplianceConfiguration_t compliance_config = {
    .misra_checking_enabled = true,
    .sil_validation_enabled = true,
    .target_sil = SIL_2,
    .project_identifier = "STM32H753ZI-MotorControl-v1.0",
    .certification_authority = "TUV-SUD"
};
standards_compliance_init(&compliance_config);

// Record MISRA-C violations with justifications
standards_record_misra_violation(
    15.5,  // Rule number
    MISRA_CATEGORY_ADVISORY,
    __FILE__,
    __LINE__,
    "Switch statement without default case",
    "All enum values explicitly handled, default case unnecessary"
);

// Register safety functions for SIL compliance tracking
standards_register_safety_function(
    "emergency_stop_system",
    SIL_2,
    0x001,
    "REQ-SAFETY-001: Emergency stop shall activate within 1ms"
);

standards_register_safety_function(
    "motor_overcurrent_protection",
    SIL_2,
    0x002,
    "REQ-SAFETY-002: Motor current shall be limited to 2A maximum"
);

// Add requirement traceability
standards_add_requirement_trace(
    REQ_MOTOR_001,
    "Motor position accuracy shall be ±0.1 degrees",
    "Motor_Control_Requirements_v1.2.doc",
    "src/controllers/motor_controller.c",
    156
);

// Generate compliance reports
char report_buffer[8192];
uint32_t report_size = standards_generate_compliance_report(
    report_buffer, 
    sizeof(report_buffer), 
    "SIL-2"
);

// Validate SIL compliance status
char validation_report[4096];
SystemError_t compliance_status = standards_validate_sil_compliance(
    SIL_2, 
    validation_report, 
    sizeof(validation_report)
);
```

#### **Code Annotation Macros**
```c
// Use compliance annotation macros in source code
void critical_safety_function(void) {
    SIL_FUNCTION(2, 0x001);  // Mark as SIL-2 function
    REQ_TRACE(REQ_SAFETY_001);  // Link to requirement
    
    // Justified MISRA violation
    MISRA_JUSTIFIED(15.5, "All enum values handled explicitly");
    
    switch (safety_state) {
        case SAFETY_NORMAL:
            // Normal operation
            break;
        case SAFETY_WARNING:
            // Warning state handling
            break;
        case SAFETY_EMERGENCY:
            // Emergency handling
            break;
        // No default needed - all enum values covered
    }
    
    // Safety assertion with SIL compliance
    SIL_ASSERT(motor_current < MAX_MOTOR_CURRENT_A, SIL_2);
}
```

#### **Integration Requirements**
- **Static Analysis Integration**: Compatible with PC-Lint, PolySpace, and other tools
- **Build System Integration**: Automated compliance checking in build process
- **Documentation Generation**: Automated generation of certification documents
- **Version Control Integration**: Compliance tracking with source code changes

### 4. **Telemetry Dashboard System** (`src/rtos/telemetry_dashboard.h`)

#### Purpose and Architecture
The telemetry dashboard system provides real-time industrial monitoring with HTTP/JSON API, web interface, and comprehensive system telemetry over STLink USB CDC interface for production diagnostics and maintenance.

#### **Core Capabilities**
- **HTTP/JSON API**: RESTful API over STLink USB CDC interface
- **Real-Time Web Interface**: Live dashboard with motor, system, and safety telemetry
- **Multi-Domain Telemetry**: Motors, performance, safety, communication status
- **Historical Data**: Time-series data collection and analysis
- **Performance Metrics**: Real-time monitoring of control loop timing and system health

#### **API Usage Patterns**
```c
// Initialize telemetry dashboard
DashboardConfiguration_t dashboard_config = {
    .enabled = true,
    .update_interval_ms = 500,
    .history_depth = 120,  // 1 minute of history at 500ms updates
    .enable_motor_telemetry = true,
    .enable_system_telemetry = true,
    .enable_safety_telemetry = true,
    .enable_web_interface = true,
    .dashboard_title = "STM32H753ZI Motor Control System"
};
telemetry_dashboard_init(&dashboard_config);

// Start HTTP server
telemetry_dashboard_start();

// Update telemetry data (called from telemetry task)
telemetry_dashboard_update();

// Get current telemetry snapshot
TelemetrySnapshot_t snapshot;
telemetry_dashboard_get_snapshot(&snapshot);

// Generate JSON data for external systems
char json_buffer[4096];
uint32_t json_size = telemetry_dashboard_generate_json(
    json_buffer, 
    sizeof(json_buffer), 
    true  // Include history
);

// Send telemetry over USB CDC
telemetry_dashboard_send_usb(&snapshot, "JSON");

// Process HTTP requests (called from USB CDC receive)
HttpRequest_t request;
HttpResponse_t response;
telemetry_parse_http_request(usb_rx_buffer, usb_rx_length, &request);
telemetry_dashboard_process_request(&request, &response);
```

#### **REST API Endpoints**
```http
GET /api/v1/telemetry/current          - Current telemetry snapshot
GET /api/v1/telemetry/motors           - Motor telemetry data
GET /api/v1/telemetry/system           - System telemetry data
GET /api/v1/telemetry/safety           - Safety telemetry data
GET /api/v1/telemetry/performance      - Performance telemetry data
GET /api/v1/telemetry/history?duration=60  - Historical data (last 60 seconds)
GET /api/v1/status                     - System status summary
GET /api/v1/config                     - System configuration
POST /api/v1/config                    - Update system configuration
GET /api/v1/diagnostics                - System diagnostics
GET /dashboard                         - HTML dashboard interface
GET /dashboard/live                    - Live updating dashboard
```

#### **Integration Requirements**
- **USB CDC Interface**: Real-time communication over STLink USB
- **FreeRTOS Integration**: Telemetry task with configurable update intervals
- **System Monitoring**: Integration with all system components for comprehensive telemetry
- **Memory Management**: Efficient buffering for real-time data streaming

#### **Telemetry Data Structure**
```c
// Complete telemetry snapshot structure
typedef struct {
    MotorTelemetryData_t motors[2];        // Both motor status
    SystemTelemetryData_t system;          // CPU, memory, temperature
    CommTelemetryData_t communication;     // CAN, UART, USB statistics
    SafetyTelemetryData_t safety;          // Safety events, fault status
    PerformanceTelemetryData_t performance; // Real-time performance metrics
    uint32_t snapshot_timestamp;           // Timestamp for correlation
    uint32_t snapshot_id;                  // Unique identifier
} TelemetrySnapshot_t;
```

## Integration Architecture

### **SSOT Configuration Integration**
All Phase 5A features integrate with the centralized SSOT configuration system:

```c
// Enhanced SSOT configuration in freertos_config_ssot.h
// Power Management Configuration
#define POWER_ACTIVE_CPU_FREQ_MHZ          480
#define POWER_QUIET_CPU_FREQ_MHZ           240
#define POWER_IDLE_CPU_FREQ_MHZ            120
#define POWER_THERMAL_WARNING_THRESHOLD    75.0f
#define POWER_THERMAL_CRITICAL_THRESHOLD   85.0f

// Dynamic Tuning Configuration
#define DYNAMIC_TUNING_TARGET_CPU_UTIL     60
#define DYNAMIC_TUNING_CPU_UTIL_TOLERANCE  10
#define DYNAMIC_TUNING_HISTORY_SIZE        32
#define DYNAMIC_TUNING_MEASUREMENT_WINDOW_MS 5000

// Standards Compliance Configuration
#define STANDARDS_COMPLIANCE_SIL_LEVEL     SIL_2
#define STANDARDS_MISRA_CHECKING_ENABLED   1
#define STANDARDS_TRACEABILITY_ENABLED     1

// Telemetry Configuration
#define TELEMETRY_UPDATE_INTERVAL_MS       500
#define TELEMETRY_HISTORY_DEPTH           120
#define TELEMETRY_HTTP_PORT               8080
#define TELEMETRY_MAX_JSON_SIZE           3072
```

### **FreeRTOS Task Integration**
Phase 5A features integrate seamlessly with existing FreeRTOS task architecture:

```c
// Task integration in main RTOS system
void system_monitoring_task(void *argument) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    for(;;) {
        // Update power management
        power_management_update();
        
        // Update dynamic tuning
        dynamic_tuning_update();
        
        // Update telemetry
        telemetry_dashboard_update();
        
        // Update standards compliance monitoring
        standards_compliance_monitor_update();
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    }
}
```

### **Safety System Integration**
All Phase 5A features maintain compatibility with existing safety systems:

```c
// Safety-compliant power management
SystemError_t safe_power_mode_transition(PowerMode_t new_mode) {
    // Validate transition doesn't compromise safety
    if (safety_system_is_emergency_active()) {
        return ERROR_SAFETY_VIOLATION;
    }
    
    // Check if motor operations allow power reduction
    if (new_mode != POWER_MODE_ACTIVE && motor_controller_is_moving()) {
        return ERROR_OPERATION_NOT_ALLOWED;
    }
    
    return power_management_request_mode(new_mode, false);
}
```

## Build System Integration

### **CMake Integration**
```cmake
# Phase 5A Advanced RTOS Features
set(PHASE5A_SOURCES
    src/rtos/power_management.c
    src/rtos/dynamic_task_tuning.c
    src/rtos/standards_compliance.c
    src/rtos/telemetry_dashboard.c
)

# Add Phase 5A sources to main target
target_sources(${PROJECT_NAME} PRIVATE ${PHASE5A_SOURCES})

# Phase 5A compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    PHASE5A_ADVANCED_FEATURES_ENABLED=1
    POWER_MANAGEMENT_ENABLED=1
    DYNAMIC_TUNING_ENABLED=1
    STANDARDS_COMPLIANCE_ENABLED=1
    TELEMETRY_DASHBOARD_ENABLED=1
)
```

### **Validation Integration**
```bash
# Automated validation for Phase 5A features
python3 scripts/validate_phase5a_compliance.py
python3 scripts/validate_power_management.py
python3 scripts/validate_standards_compliance.py
python3 scripts/generate_telemetry_test_data.py
```

## Best Practices and Guidelines

### **1. Power Management Best Practices**
- Always validate power mode transitions for safety impact
- Monitor thermal conditions continuously
- Use predictive optimization for energy-critical applications
- Implement graceful degradation under thermal stress

### **2. Dynamic Tuning Best Practices**
- Register safety-critical tasks with minimal adjustment ranges
- Monitor CPU utilization trends for optimal target adjustment
- Use performance history for predictive optimization
- Implement emergency load reduction for fault tolerance

### **3. Standards Compliance Best Practices**
- Document all MISRA violations with clear justifications
- Maintain comprehensive requirement traceability
- Regular compliance validation and reporting
- Integrate static analysis tools in build process

### **4. Telemetry Best Practices**
- Configure appropriate update intervals for application needs
- Use JSON API for external system integration
- Monitor telemetry system performance impact
- Implement efficient data buffering for real-time streaming

## Troubleshooting and Diagnostics

### **Common Integration Issues**
1. **Power Management Conflicts**: Ensure safety systems override power optimization
2. **Dynamic Tuning Oscillation**: Adjust tuning parameters for system stability
3. **Standards Compliance Overhead**: Optimize compliance checking for performance
4. **Telemetry Performance Impact**: Balance update frequency with system performance

### **Debugging Tools**
```c
// Debug functions for Phase 5A features
void power_management_print_status(void);
void dynamic_tuning_print_performance_report(void);
void standards_print_compliance_status(void);
void telemetry_dashboard_print_statistics(void);

// Validation functions
bool power_management_validate_state(void);
bool dynamic_tuning_validate_configuration(void);
bool standards_validate_internal_consistency(void);
bool telemetry_validate_data_consistency(void);
```

### **Performance Monitoring**
Monitor Phase 5A feature overhead:
- Power management: <2% CPU overhead
- Dynamic tuning: <1% CPU overhead
- Standards compliance: <0.5% CPU overhead
- Telemetry dashboard: <3% CPU overhead (configurable update rate)

## Conclusion

Phase 5A advanced RTOS features provide industrial-grade capabilities while maintaining the safety, performance, and reliability requirements of the STM32H753ZI motor control system. All features are designed for seamless integration with existing codebase and compliance with industrial standards for certification-ready deployment.

Remember: **Advanced features enhance capability without compromising safety, performance, or system reliability.**
