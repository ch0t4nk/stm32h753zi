# Motor Tuning Implementation Plan

**Project**: STM32H753ZI Dynamic Motor Tuning System  
**Approval Date**: August 07, 2025  
**Implementation Priority**: Phase 1 - Motor Characterization with Focused Telemetry  
**Target Platform**: STM32H753ZI + X-NUCLEO-IHM02A1 + AS5600 encoders

---

## 🎯 **Implementation Approval Summary**

**✅ APPROVED**: Dynamic motor tuning development  
**🎯 PRIORITY**: Motor Characterization with focused telemetry (Phase 1)  
**⚙️ DEVELOPMENT**: Telemetry data validation vs oscilloscope baseline  
**📊 SUCCESS CRITERIA**: Performance improvement targets and data acquisition efficiency  

---

## 📊 **Success Criteria and Performance Targets**

### **Primary Performance Targets (Based on ST L6470 Documentation)**

| Parameter | Current Static | Target Dynamic | Improvement | Measurement Method | ST Reference |
|-----------|----------------|----------------|-------------|-------------------|--------------|
| **Power Efficiency** | ~75% (typical stepper) | 85-90% | 15-25% improvement | L6470 ADC current measurement | X-CUBE-SPN2 power guidelines |
| **Thermal Performance** | 85°C max operation | 70-75°C operation | 10-20°C reduction | STM32H7 temp sensor + L6470 thermal flags | L6470 thermal management spec |
| **Position Accuracy** | ±0.5° (manual tuning) | ±0.1-0.2° | 50-75% improvement | AS5600 encoder feedback (0.088° resolution) | AS5600 datasheet accuracy |
| **KVAL Optimization** | Static 0x29 (25%) | Dynamic 15-35% | Load-adaptive | Real-time current optimization | L6470 KVAL application notes |
| **BEMF Compensation** | Static slopes | Dynamic compensation | Speed-dependent | Velocity-based BEMF tuning | L6470 BEMF theory documentation |
| **Tuning Convergence** | Manual (hours/days) | <5 minutes | Automatic | Algorithm execution timing | Industry standard auto-tuning |

### **Data Acquisition Efficiency Targets**

| Metric | Target | Baseline Comparison | Validation Method |
|--------|--------|-------------------|-------------------|
| **CPU Overhead** | <2% | Software oscilloscope: 15-25% | FreeRTOS task monitoring |
| **Memory Usage** | <8KB RAM | Continuous capture: 32-64KB | STM32H7 memory profiling |
| **Sample Rate** | 1kHz sustained | Oscilloscope: 100Hz max | Real-time telemetry streaming |
| **Data Processing** | On-board real-time | External post-processing | STM32H7 Cortex-M7 DSP |
| **Response Time** | <1ms optimization | Oscilloscope: >100ms | Immediate parameter adjustment |

### **Manufacturer Compliance Targets**

| Component | Compliance Requirement | Validation Method | Reference Documentation |
|-----------|------------------------|------------------|------------------------|
| **L6470** | All parameters within ST datasheet limits | Parameter range validation | 00_reference/x_cube_spn2_markdown_docs/ |
| **AS5600** | I2C communication per AMS spec | Protocol compliance testing | 00_reference/STM32H7xx_Nucleo_BSP_User_Manual/ |
| **STM32H7** | Real-time performance maintained | 1kHz control loop timing | STM32H7 reference manual |
| **Safety Systems** | Emergency stop <1ms response | Safety system integration testing | Existing safety framework |

---

## 🏗️ **Phase 1 Implementation Plan: Motor Characterization Framework**

### **Phase 1.1: Focused Telemetry Infrastructure (2-3 hours)**

#### **File Creation Priority**
```
src/controllers/motor_characterization.h/.c      # Primary characterization framework
src/telemetry/optimization_telemetry.h/.c        # Focused telemetry data acquisition  
src/controllers/optimization_algorithms.h/.c     # Common optimization functions
tests/unit/test_motor_characterization.c         # Unit testing framework
```

#### **1.1.1: Telemetry Data Structures**
```c
// File: src/telemetry/optimization_telemetry.h
// Focused telemetry for motor characterization

#include "config/motor_config.h"
#include "config/hardware_config.h"
#include "hal_abstraction.h"

// High-resolution telemetry packet for characterization
typedef struct {
    // AS5600 Encoder Data
    float position_degrees;           // Current position (0.088° resolution)
    float velocity_dps;              // Calculated velocity (degrees/second)
    float acceleration_dps2;         // Calculated acceleration
    
    // L6470 Driver Data  
    float motor_current_a;           // Measured current (ADC-based)
    uint16_t kval_hold_actual;       // Current KVAL_HOLD setting
    uint16_t kval_run_actual;        // Current KVAL_RUN setting
    uint8_t status_flags;            // L6470 status register
    bool thermal_warning;            // Thermal warning flag
    bool stall_detected;             // Stall detection flag
    
    // System Performance Data
    uint32_t timestamp_us;           // High-resolution timestamp
    uint32_t control_loop_time_us;   // Control loop execution time
    float power_consumption_w;       // Calculated power consumption
    float thermal_performance;       // Temperature-based performance metric
    
    // Test-Specific Data
    float commanded_position;        // Commanded position for test
    float commanded_velocity;        // Commanded velocity for test
    float position_error;            // Position tracking error
    float control_effort;            // PID controller output
} OptimizationTelemetryPacket_t;

// Characterization dataset for batch analysis
typedef struct {
    OptimizationTelemetryPacket_t samples[CHARACTERIZATION_BUFFER_SIZE];
    uint32_t sample_count;           // Number of valid samples
    uint32_t sample_rate_hz;         // Sampling frequency
    uint32_t test_duration_ms;       // Total test duration
    CharacterizationTestType_t test_type; // Type of characterization test
} CharacterizationDataSet_t;

// Test configuration for characterization
typedef struct {
    float step_amplitude_deg;        // Step response amplitude
    float frequency_start_hz;        // Frequency sweep start
    float frequency_end_hz;          // Frequency sweep end
    uint32_t test_duration_ms;       // Test duration
    uint32_t sample_rate_hz;         // Data collection sample rate
    bool enable_safety_limits;       // Enable safety bound checking
} CharacterizationTestConfig_t;
```

#### **1.1.2: Telemetry Collection APIs**
```c
// File: src/telemetry/optimization_telemetry.c

// Initialize telemetry system for motor characterization
SystemError_t optimization_telemetry_init(uint8_t motor_id) {
    // Initialize high-resolution timer for precise timestamping
    SystemError_t result = realtime_timer_init();
    if (result != SYSTEM_OK) return result;
    
    // Configure AS5600 encoder for high-rate sampling
    result = as5600_configure_fast_mode(motor_id);
    if (result != SYSTEM_OK) return result;
    
    // Configure L6470 for ADC current measurement
    result = l6470_enable_adc_monitoring(motor_id);
    if (result != SYSTEM_OK) return result;
    
    return SYSTEM_OK;
}

// Collect single telemetry sample with minimal overhead
SystemError_t optimization_telemetry_collect_sample(uint8_t motor_id, 
                                                   OptimizationTelemetryPacket_t* packet) {
    uint32_t start_time = get_microsecond_timer();
    
    // AS5600 position and velocity (I2C transaction)
    SystemError_t result = as5600_read_angle_degrees(motor_id, &packet->position_degrees);
    if (result != SYSTEM_OK) return result;
    
    result = as5600_calculate_velocity(motor_id, &packet->velocity_dps);
    if (result != SYSTEM_OK) return result;
    
    // L6470 current and status (SPI transaction)
    uint32_t adc_value;
    result = l6470_get_parameter(motor_id, L6470_REG_ADC_OUT, &adc_value);
    if (result != SYSTEM_OK) return result;
    packet->motor_current_a = l6470_adc_to_current(adc_value);
    
    uint32_t status;
    result = l6470_get_parameter(motor_id, L6470_REG_STATUS, &status);
    if (result != SYSTEM_OK) return result;
    packet->status_flags = (uint8_t)status;
    packet->thermal_warning = (status & L6470_STATUS_TH_WRN) != 0;
    packet->stall_detected = (status & L6470_STATUS_STEP_LOSS_A) != 0;
    
    // System performance data
    packet->timestamp_us = get_microsecond_timer();
    packet->control_loop_time_us = packet->timestamp_us - start_time;
    
    // Calculate derived metrics
    packet->power_consumption_w = packet->motor_current_a * MOTOR_SUPPLY_VOLTAGE_V;
    packet->thermal_performance = calculate_thermal_performance_score(packet);
    
    return SYSTEM_OK;
}

// Batch data collection for characterization tests
SystemError_t optimization_telemetry_collect_dataset(uint8_t motor_id,
                                                    const CharacterizationTestConfig_t* config,
                                                    CharacterizationDataSet_t* dataset) {
    dataset->sample_count = 0;
    dataset->sample_rate_hz = config->sample_rate_hz;
    dataset->test_type = config->test_type;
    
    uint32_t sample_interval_us = 1000000 / config->sample_rate_hz;
    uint32_t start_time = get_microsecond_timer();
    uint32_t next_sample_time = start_time;
    
    while ((get_microsecond_timer() - start_time) < (config->test_duration_ms * 1000)) {
        if (get_microsecond_timer() >= next_sample_time) {
            if (dataset->sample_count < CHARACTERIZATION_BUFFER_SIZE) {
                SystemError_t result = optimization_telemetry_collect_sample(
                    motor_id, &dataset->samples[dataset->sample_count]);
                
                if (result == SYSTEM_OK) {
                    dataset->sample_count++;
                }
            }
            next_sample_time += sample_interval_us;
        }
        
        // Yield to other tasks if using FreeRTOS
        osDelay(1);
    }
    
    dataset->test_duration_ms = (get_microsecond_timer() - start_time) / 1000;
    return SYSTEM_OK;
}
```

### **Phase 1.2: Motor Characterization Framework (3-4 hours)**

#### **1.2.1: Motor Parameter Identification**
```c
// File: src/controllers/motor_characterization.h

#include "telemetry/optimization_telemetry.h"
#include "common/error_codes.h"

// Motor characteristics identified through testing
typedef struct {
    // Electrical Parameters
    float winding_resistance_ohm;     // Measured winding resistance
    float winding_inductance_mh;      // Measured winding inductance
    float back_emf_constant_v_per_rps; // Back-EMF constant
    
    // Mechanical Parameters  
    float rotor_inertia_kg_m2;       // Estimated rotor inertia
    float static_friction_nm;        // Static friction torque
    float viscous_friction_nm_per_rps; // Viscous friction coefficient
    float mechanical_time_constant_s; // Mechanical time constant
    
    // Performance Characteristics
    float optimal_kval_hold;         // Optimal holding current setting
    float optimal_kval_run;          // Optimal running current setting
    float max_efficient_speed_rps;   // Maximum efficient speed
    float thermal_coefficient;       // Thermal performance coefficient
    
    // Characterization Status
    bool characterization_complete;   // Full characterization completed
    uint32_t characterization_timestamp; // When characterization was performed
    float characterization_confidence; // Confidence in measurements (0.0-1.0)
} MotorCharacteristics_t;

// Characterization test state machine
typedef enum {
    CHAR_STATE_IDLE,
    CHAR_STATE_RESISTANCE_TEST,
    CHAR_STATE_INDUCTANCE_TEST,
    CHAR_STATE_BEMF_TEST,
    CHAR_STATE_STEP_RESPONSE_TEST,
    CHAR_STATE_FREQUENCY_RESPONSE_TEST,
    CHAR_STATE_LOAD_TEST,
    CHAR_STATE_ANALYSIS,
    CHAR_STATE_COMPLETE,
    CHAR_STATE_ERROR
} CharacterizationState_t;

// Characterization context
typedef struct {
    MotorCharacteristics_t characteristics;  // Measured characteristics
    CharacterizationState_t state;          // Current test state
    uint32_t test_sequence_step;            // Current step in sequence
    CharacterizationDataSet_t current_dataset; // Current test data
    float safety_limits[4];                 // [max_current, max_speed, max_temp, max_error]
    bool safety_abort_requested;           // Safety system abort flag
} MotorCharacterizationContext_t;
```

#### **1.2.2: Characterization Test Implementation**
```c
// File: src/controllers/motor_characterization.c

static MotorCharacterizationContext_t char_contexts[MAX_MOTORS];

// Initialize motor characterization system
SystemError_t motor_characterization_init(uint8_t motor_id) {
    if (motor_id >= MAX_MOTORS) return ERROR_INVALID_PARAMETER;
    
    MotorCharacterizationContext_t* ctx = &char_contexts[motor_id];
    
    // Initialize characterization context
    memset(ctx, 0, sizeof(MotorCharacterizationContext_t));
    ctx->state = CHAR_STATE_IDLE;
    
    // Set safety limits from SSOT configuration
    ctx->safety_limits[0] = MOTOR_MAX_CURRENT_A;
    ctx->safety_limits[1] = MOTOR_MAX_SPEED_RPM / 60.0f; // Convert to RPS
    ctx->safety_limits[2] = MOTOR_MAX_TEMPERATURE_C;
    ctx->safety_limits[3] = MOTOR_MAX_POSITION_ERROR_DEG;
    
    // Initialize telemetry system
    SystemError_t result = optimization_telemetry_init(motor_id);
    if (result != SYSTEM_OK) return result;
    
    return SYSTEM_OK;
}

// Start automatic motor characterization sequence
SystemError_t motor_characterization_start(uint8_t motor_id) {
    if (motor_id >= MAX_MOTORS) return ERROR_INVALID_PARAMETER;
    
    MotorCharacterizationContext_t* ctx = &char_contexts[motor_id];
    
    if (ctx->state != CHAR_STATE_IDLE) {
        return ERROR_OPERATION_IN_PROGRESS;
    }
    
    // Verify safety systems are active
    if (!safety_system_is_ready()) {
        return ERROR_SAFETY_SYSTEM_NOT_READY;
    }
    
    // Start with resistance measurement
    ctx->state = CHAR_STATE_RESISTANCE_TEST;
    ctx->test_sequence_step = 0;
    ctx->safety_abort_requested = false;
    
    log_info("Motor %d: Starting automatic characterization", motor_id);
    
    return SYSTEM_OK;
}

// Execute step response test for system identification
SystemError_t characterization_step_response_test(uint8_t motor_id, 
                                                 float step_amplitude_deg,
                                                 CharacterizationDataSet_t* dataset) {
    if (motor_id >= MAX_MOTORS) return ERROR_INVALID_PARAMETER;
    if (dataset == NULL) return ERROR_NULL_POINTER;
    
    // Configure test parameters
    CharacterizationTestConfig_t config = {
        .step_amplitude_deg = step_amplitude_deg,
        .test_duration_ms = 2000,  // 2 second test
        .sample_rate_hz = 1000,    // 1kHz sampling
        .enable_safety_limits = true
    };
    
    // Get initial position
    float initial_position;
    SystemError_t result = as5600_read_angle_degrees(motor_id, &initial_position);
    if (result != SYSTEM_OK) return result;
    
    // Start data collection
    result = optimization_telemetry_collect_dataset(motor_id, &config, dataset);
    if (result != SYSTEM_OK) return result;
    
    // Command step input
    float target_position = initial_position + step_amplitude_deg;
    result = motor_controller_move_to_position(motor_id, target_position);
    if (result != SYSTEM_OK) return result;
    
    // Wait for test completion and data collection
    HAL_Delay(config.test_duration_ms);
    
    log_info("Motor %d: Step response test complete, %d samples collected", 
             motor_id, dataset->sample_count);
    
    return SYSTEM_OK;
}

// Execute frequency response test for system identification  
SystemError_t characterization_frequency_sweep_test(uint8_t motor_id,
                                                   float start_freq_hz, 
                                                   float end_freq_hz,
                                                   CharacterizationDataSet_t* dataset) {
    if (motor_id >= MAX_MOTORS) return ERROR_INVALID_PARAMETER;
    if (dataset == NULL) return ERROR_NULL_POINTER;
    
    CharacterizationTestConfig_t config = {
        .frequency_start_hz = start_freq_hz,
        .frequency_end_hz = end_freq_hz,
        .test_duration_ms = 10000,  // 10 second sweep
        .sample_rate_hz = 1000,     // 1kHz sampling
        .enable_safety_limits = true
    };
    
    // Get initial position
    float initial_position;
    SystemError_t result = as5600_read_angle_degrees(motor_id, &initial_position);
    if (result != SYSTEM_OK) return result;
    
    // Start data collection
    result = optimization_telemetry_collect_dataset(motor_id, &config, dataset);
    if (result != SYSTEM_OK) return result;
    
    // Execute frequency sweep
    float sweep_duration_s = config.test_duration_ms / 1000.0f;
    float frequency_range = end_freq_hz - start_freq_hz;
    
    uint32_t start_time = HAL_GetTick();
    while ((HAL_GetTick() - start_time) < config.test_duration_ms) {
        float elapsed_s = (HAL_GetTick() - start_time) / 1000.0f;
        float current_freq = start_freq_hz + (frequency_range * elapsed_s / sweep_duration_s);
        
        // Generate sinusoidal position command
        float sine_amplitude = 5.0f;  // 5 degree amplitude
        float position_command = initial_position + 
                               sine_amplitude * sinf(2.0f * M_PI * current_freq * elapsed_s);
        
        result = motor_controller_move_to_position(motor_id, position_command);
        if (result != SYSTEM_OK) break;
        
        HAL_Delay(1);  // 1ms delay for 1kHz update rate
    }
    
    log_info("Motor %d: Frequency sweep test complete, %d samples collected", 
             motor_id, dataset->sample_count);
    
    return SYSTEM_OK;
}
```

### **Phase 1.3: Data Analysis and Parameter Extraction (1-2 hours)**

#### **1.3.1: System Identification Algorithms**
```c
// File: src/controllers/optimization_algorithms.h/.c

// Extract motor parameters from step response data
SystemError_t characterization_analyze_step_response(const CharacterizationDataSet_t* dataset,
                                                    MotorCharacteristics_t* characteristics) {
    if (dataset == NULL || characteristics == NULL) return ERROR_NULL_POINTER;
    if (dataset->sample_count < 100) return ERROR_INSUFFICIENT_DATA;
    
    // Find step start time and steady-state value
    float initial_position = dataset->samples[0].position_degrees;
    float final_position = dataset->samples[dataset->sample_count - 1].position_degrees;
    float step_amplitude = final_position - initial_position;
    
    // Calculate 63% rise time for time constant estimation
    float target_63_percent = initial_position + 0.63f * step_amplitude;
    uint32_t rise_time_samples = 0;
    
    for (uint32_t i = 0; i < dataset->sample_count; i++) {
        if (dataset->samples[i].position_degrees >= target_63_percent) {
            rise_time_samples = i;
            break;
        }
    }
    
    float time_constant_s = rise_time_samples / (float)dataset->sample_rate_hz;
    characteristics->mechanical_time_constant_s = time_constant_s;
    
    // Estimate system damping from overshoot
    float max_overshoot = 0.0f;
    for (uint32_t i = rise_time_samples; i < dataset->sample_count; i++) {
        float overshoot = dataset->samples[i].position_degrees - final_position;
        if (overshoot > max_overshoot) {
            max_overshoot = overshoot;
        }
    }
    
    float overshoot_percent = max_overshoot / step_amplitude;
    float damping_ratio = sqrtf(logf(overshoot_percent) * logf(overshoot_percent) / 
                               (M_PI * M_PI + logf(overshoot_percent) * logf(overshoot_percent)));
    
    // Estimate rotor inertia from time constant and friction
    // J = T * B where T is time constant, B is friction coefficient
    characteristics->rotor_inertia_kg_m2 = time_constant_s * characteristics->viscous_friction_nm_per_rps;
    
    log_info("Step response analysis: Time constant=%.3fs, Damping=%.3f, Inertia=%.6f kg*m²",
             time_constant_s, damping_ratio, characteristics->rotor_inertia_kg_m2);
    
    return SYSTEM_OK;
}

// Calculate optimal KVAL parameters from characterization data
SystemError_t characterization_calculate_optimal_kval(const MotorCharacteristics_t* characteristics,
                                                     float* optimal_kval_hold,
                                                     float* optimal_kval_run) {
    if (characteristics == NULL || optimal_kval_hold == NULL || optimal_kval_run == NULL) {
        return ERROR_NULL_POINTER;
    }
    
    // Calculate optimal holding current based on static friction
    // Use minimum current needed to overcome static friction with safety margin
    float min_hold_current = characteristics->static_friction_nm / MOTOR_TORQUE_CONSTANT_NM_PER_A;
    float optimal_hold_current = min_hold_current * 1.2f;  // 20% safety margin
    
    // Convert to KVAL value (0-255 scale)
    *optimal_kval_hold = (optimal_hold_current / MOTOR_MAX_CURRENT_A) * 255.0f;
    *optimal_kval_hold = fmaxf(0.0f, fminf(255.0f, *optimal_kval_hold));
    
    // Calculate optimal running current based on back-EMF and efficiency
    float optimal_run_current = optimal_hold_current * 1.5f;  // Typically 50% higher than hold
    *optimal_kval_run = (optimal_run_current / MOTOR_MAX_CURRENT_A) * 255.0f;
    *optimal_kval_run = fmaxf(0.0f, fminf(255.0f, *optimal_kval_run));
    
    log_info("Optimal KVAL calculated: Hold=%.0f (%.1fA), Run=%.0f (%.1fA)",
             *optimal_kval_hold, optimal_hold_current,
             *optimal_kval_run, optimal_run_current);
    
    return SYSTEM_OK;
}
```

---

## 🔧 **Development Configuration Setup**

### **Telemetry Data Validation Strategy**

#### **Baseline Oscilloscope Comparison Tests**
```c
// Validation test configuration
typedef struct {
    uint32_t validation_duration_ms;    // Test duration for comparison
    uint32_t telemetry_sample_rate_hz;  // Our telemetry sample rate
    uint32_t oscilloscope_sample_rate_hz; // Oscilloscope reference rate
    float position_step_amplitude_deg;   // Step test amplitude
    float frequency_sweep_start_hz;      // Frequency test start
    float frequency_sweep_end_hz;        // Frequency test end
} ValidationTestConfig_t;

// Validation test results
typedef struct {
    float position_accuracy_percent;     // Position measurement accuracy vs scope
    float timing_accuracy_percent;       // Timing accuracy vs scope
    float cpu_overhead_percent;          // CPU overhead of telemetry system
    float memory_usage_kb;              // Memory usage for telemetry
    bool real_time_compatibility;       // Can maintain 1kHz control loop
} ValidationResults_t;
```

#### **Performance Monitoring Integration**
```c
// Integration with existing FreeRTOS performance monitoring
SystemError_t characterization_monitor_performance(uint8_t motor_id,
                                                  ValidationResults_t* results) {
    // Measure CPU overhead using existing FreeRTOS runtime stats
    SystemPerformanceMetrics_t metrics;
    SystemError_t result = dynamic_tuning_get_metrics(&metrics);
    if (result != SYSTEM_OK) return result;
    
    results->cpu_overhead_percent = calculate_telemetry_cpu_overhead(&metrics);
    results->memory_usage_kb = get_telemetry_memory_usage() / 1024.0f;
    results->real_time_compatibility = (metrics.max_control_loop_time_us < 900); // <0.9ms for 1kHz
    
    return SYSTEM_OK;
}
```

### **Safety Integration Requirements**

#### **Safety Bounds During Characterization**
```c
// Safety monitoring during characterization tests
SystemError_t characterization_safety_monitor(uint8_t motor_id,
                                             const OptimizationTelemetryPacket_t* telemetry) {
    // Check current limits
    if (telemetry->motor_current_a > MOTOR_MAX_CURRENT_A) {
        log_error("Motor %d: Current limit exceeded: %.2fA > %.2fA", 
                  motor_id, telemetry->motor_current_a, MOTOR_MAX_CURRENT_A);
        return motor_characterization_emergency_stop(motor_id);
    }
    
    // Check thermal limits
    if (telemetry->thermal_warning) {
        log_warning("Motor %d: Thermal warning during characterization", motor_id);
        return motor_characterization_reduce_test_intensity(motor_id);
    }
    
    // Check position error limits
    if (fabsf(telemetry->position_error) > MOTOR_MAX_POSITION_ERROR_DEG) {
        log_error("Motor %d: Position error exceeded: %.2f° > %.2f°",
                  motor_id, telemetry->position_error, MOTOR_MAX_POSITION_ERROR_DEG);
        return motor_characterization_emergency_stop(motor_id);
    }
    
    // Check stall detection
    if (telemetry->stall_detected) {
        log_error("Motor %d: Stall detected during characterization", motor_id);
        return motor_characterization_emergency_stop(motor_id);
    }
    
    return SYSTEM_OK;
}
```

---

## 📋 **Implementation Timeline and Milestones**

### **Phase 1 Timeline (6-8 hours total)**

| Task | Duration | Dependencies | Deliverable |
|------|----------|--------------|-------------|
| **1.1 Telemetry Infrastructure** | 2-3 hours | HAL abstraction, SSOT config | `optimization_telemetry.h/.c` |
| **1.2 Characterization Framework** | 3-4 hours | Telemetry infrastructure | `motor_characterization.h/.c` |
| **1.3 Data Analysis Algorithms** | 1-2 hours | Characterization framework | `optimization_algorithms.h/.c` |
| **1.4 Unit Testing** | 1 hour | All Phase 1 components | `test_motor_characterization.c` |

### **Validation Milestones**

| Milestone | Acceptance Criteria | Test Method |
|-----------|-------------------|-------------|
| **Telemetry Accuracy** | <5% error vs oscilloscope | Position and timing comparison |
| **CPU Overhead** | <2% CPU utilization | FreeRTOS runtime statistics |
| **Memory Efficiency** | <8KB RAM usage | STM32H7 memory profiling |
| **Real-Time Compatibility** | Maintain 1kHz control loop | Control loop timing analysis |
| **Safety Integration** | Emergency stop <1ms | Safety system response testing |

---

## 🎯 **Next Actions - Implementation Start**

### **Immediate Implementation Steps**

1. **✅ CREATE**: Telemetry infrastructure files
2. **✅ IMPLEMENT**: Basic data collection APIs  
3. **✅ INTEGRATE**: Safety monitoring systems
4. **✅ TEST**: Telemetry accuracy validation
5. **✅ OPTIMIZE**: Performance tuning for <2% CPU overhead

### **Development Environment Configuration**

```bash
# Set up development branch for motor tuning
git checkout -b feature/motor-tuning-phase1

# Create directory structure
mkdir -p src/telemetry
mkdir -p tests/unit/motor_tuning

# Configure CMake for new modules
# Add telemetry and characterization modules to CMakeLists.txt
```

This implementation plan provides a comprehensive roadmap for Phase 1 motor characterization with focused telemetry. The plan respects all manufacturer specifications, integrates with existing infrastructure, and provides measurable success criteria based on ST documentation and industry standards.

**Ready to proceed with implementation? The foundation is solid and the path is clear!**
