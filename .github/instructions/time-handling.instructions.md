---
applyTo: "src/config/**/*.h"
description: "Time handling and timing systems for STM32H753ZI stepper motor project"
---

# Time Handling and Timing Systems Instructions

## Overview
This instruction file provides comprehensive guidance for implementing consistent, accurate, and efficient time handling throughout the STM32H753ZI stepper motor control project.

## Time Base Systems

### System Tick Configuration
STM32H753ZI operates at 480 MHz with configurable system tick:

```c
// System tick configuration (1ms resolution recommended)
#define SYSTEM_TICK_FREQUENCY_HZ        1000        // 1kHz = 1ms resolution
#define SYSTEM_TICK_PERIOD_MS           1           // 1ms per tick
#define SYSTEM_TICK_PERIOD_US           1000        // 1000μs per tick

// High-resolution timer for precise timing (use TIM2 - 32-bit)
#define HIRES_TIMER_FREQUENCY_HZ        1000000     // 1MHz = 1μs resolution
#define HIRES_TIMER_PERIOD_US           1           // 1μs per tick
#define HIRES_TIMER_MAX_VALUE           0xFFFFFFFF  // 32-bit timer
#define HIRES_TIMER_ROLLOVER_TIME_S     4294        // ~71 minutes

// Timer instance assignments
#define SYSTEM_TICK_TIMER               TIM7        // System tick timer
#define HIRES_TIMER                     TIM2        // High-resolution timer
#define MOTOR_PWM_TIMER                 TIM1        // Motor PWM generation
#define ENCODER_TIMER                   TIM3        // Encoder interface
#define WATCHDOG_TIMER                  IWDG        // Independent watchdog
```

### Time Data Types
```c
// Base time types with explicit units
typedef uint32_t SystemTime_t;          // System time in milliseconds
typedef uint32_t PreciseTime_t;         // Precise time in microseconds
typedef uint32_t Duration_t;            // Duration in milliseconds
typedef uint32_t PreciseDuration_t;     // Precise duration in microseconds
typedef uint64_t ExtendedTime_t;        // Extended time for long durations

// Time constants for readability
#define TIME_US_PER_MS                  1000UL
#define TIME_MS_PER_SECOND              1000UL
#define TIME_US_PER_SECOND              1000000UL
#define TIME_SECONDS_PER_MINUTE         60UL
#define TIME_MINUTES_PER_HOUR           60UL
#define TIME_HOURS_PER_DAY              24UL

// Time conversion macros
#define MS_TO_US(ms)                    ((ms) * TIME_US_PER_MS)
#define US_TO_MS(us)                    ((us) / TIME_US_PER_MS)
#define SECONDS_TO_MS(s)                ((s) * TIME_MS_PER_SECOND)
#define MS_TO_SECONDS(ms)               ((ms) / TIME_MS_PER_SECOND)
#define MINUTES_TO_MS(min)              ((min) * TIME_SECONDS_PER_MINUTE * TIME_MS_PER_SECOND)
#define HOURS_TO_MS(hr)                 ((hr) * TIME_MINUTES_PER_HOUR * TIME_SECONDS_PER_MINUTE * TIME_MS_PER_SECOND)

// Time validation macros
#define IS_VALID_TIMEOUT(timeout_ms)    ((timeout_ms) > 0 && (timeout_ms) < 0x7FFFFFFF)
#define IS_EXPIRED(start_time, timeout) ((HAL_GetTick() - (start_time)) >= (timeout))
```

### Time Source Management
```c
// Time source enumeration
typedef enum {
    TIME_SOURCE_SYSTICK = 0,            // HAL_GetTick() - 1ms resolution
    TIME_SOURCE_HIRES_TIMER = 1,        // High-resolution timer - 1μs resolution
    TIME_SOURCE_RTC = 2,                // Real-time clock
    TIME_SOURCE_EXTERNAL = 3            // External time reference
} TimeSource_t;

// Time source configuration
typedef struct {
    TimeSource_t source;                // Time source type
    uint32_t frequency_hz;              // Source frequency
    uint32_t resolution_us;             // Time resolution in microseconds
    bool is_monotonic;                  // True if time never goes backwards
    bool is_continuous;                 // True if time continues in sleep
    uint32_t rollover_period_s;         // Rollover period in seconds
} TimeSourceConfig_t;

// Time source configurations
static const TimeSourceConfig_t time_sources[] = {
    {TIME_SOURCE_SYSTICK, SYSTEM_TICK_FREQUENCY_HZ, 1000, true, false, 4294967},
    {TIME_SOURCE_HIRES_TIMER, HIRES_TIMER_FREQUENCY_HZ, 1, true, false, 4294},
    {TIME_SOURCE_RTC, 32768, 30517, true, true, 0}, // 1/32768 ≈ 30.517μs
    {TIME_SOURCE_EXTERNAL, 0, 0, false, false, 0}
};

// Current time source selection
static TimeSource_t g_current_time_source = TIME_SOURCE_SYSTICK;
```

## Time Acquisition Functions

### Basic Time Functions
```c
// System time acquisition (1ms resolution)
static inline SystemTime_t time_get_system_ms(void) {
    return HAL_GetTick();
}

// High-resolution time acquisition (1μs resolution)
static inline PreciseTime_t time_get_precise_us(void) {
    // Read 32-bit timer counter
    return __HAL_TIM_GET_COUNTER(&htim2);
}

// Combined time acquisition with rollover handling
typedef struct {
    SystemTime_t system_ms;             // System time in milliseconds
    PreciseTime_t precise_us;           // Precise time in microseconds
    uint32_t rollover_count;            // Number of timer rollovers
} CombinedTime_t;

CombinedTime_t time_get_combined(void) {
    static uint32_t last_precise_time = 0;
    static uint32_t rollover_count = 0;
    
    CombinedTime_t result;
    
    // Disable interrupts for atomic read
    __disable_irq();
    
    result.system_ms = HAL_GetTick();
    result.precise_us = __HAL_TIM_GET_COUNTER(&htim2);
    
    // Detect rollover
    if (result.precise_us < last_precise_time) {
        rollover_count++;
    }
    last_precise_time = result.precise_us;
    result.rollover_count = rollover_count;
    
    __enable_irq();
    
    return result;
}

// Extended time calculation (handles rollovers)
ExtendedTime_t time_get_extended_us(void) {
    CombinedTime_t combined = time_get_combined();
    return ((ExtendedTime_t)combined.rollover_count << 32) | combined.precise_us;
}
```

### Time Difference Calculations
```c
// Safe time difference calculation (handles rollover)
Duration_t time_diff_ms(SystemTime_t start_time, SystemTime_t end_time) {
    // Handle 32-bit unsigned rollover correctly
    return (Duration_t)(end_time - start_time);
}

PreciseDuration_t time_diff_us(PreciseTime_t start_time, PreciseTime_t end_time) {
    // Handle 32-bit unsigned rollover correctly
    return (PreciseDuration_t)(end_time - start_time);
}

// Extended time difference (no rollover concerns)
ExtendedTime_t time_diff_extended_us(ExtendedTime_t start_time, ExtendedTime_t end_time) {
    return end_time - start_time;
}

// Time comparison functions
bool time_is_after(SystemTime_t time1, SystemTime_t time2) {
    return (SystemTime_t)(time1 - time2) < 0x80000000UL;
}

bool time_is_before(SystemTime_t time1, SystemTime_t time2) {
    return (SystemTime_t)(time2 - time1) < 0x80000000UL;
}
```

## Timeout and Delay Management

### Timeout Structures and Functions
```c
// Timeout handle structure
typedef struct {
    SystemTime_t start_time;            // When timeout started
    Duration_t timeout_duration;        // Timeout duration in ms
    bool is_active;                     // Timeout is active
    bool has_expired;                   // Timeout has expired
} Timeout_t;

// Initialize timeout
void timeout_init(Timeout_t* timeout, Duration_t duration_ms) {
    timeout->start_time = time_get_system_ms();
    timeout->timeout_duration = duration_ms;
    timeout->is_active = true;
    timeout->has_expired = false;
}

// Check if timeout has expired
bool timeout_is_expired(Timeout_t* timeout) {
    if (!timeout->is_active || timeout->has_expired) {
        return timeout->has_expired;
    }
    
    SystemTime_t current_time = time_get_system_ms();
    Duration_t elapsed = time_diff_ms(timeout->start_time, current_time);
    
    if (elapsed >= timeout->timeout_duration) {
        timeout->has_expired = true;
        timeout->is_active = false;
    }
    
    return timeout->has_expired;
}

// Get remaining timeout duration
Duration_t timeout_get_remaining(const Timeout_t* timeout) {
    if (!timeout->is_active || timeout->has_expired) {
        return 0;
    }
    
    SystemTime_t current_time = time_get_system_ms();
    Duration_t elapsed = time_diff_ms(timeout->start_time, current_time);
    
    if (elapsed >= timeout->timeout_duration) {
        return 0;
    }
    
    return timeout->timeout_duration - elapsed;
}

// Reset timeout with new duration
void timeout_reset(Timeout_t* timeout, Duration_t new_duration_ms) {
    timeout->start_time = time_get_system_ms();
    timeout->timeout_duration = new_duration_ms;
    timeout->is_active = true;
    timeout->has_expired = false;
}

// Cancel timeout
void timeout_cancel(Timeout_t* timeout) {
    timeout->is_active = false;
    timeout->has_expired = false;
}
```

### Precise Delay Functions
```c
// High-precision delay using busy-wait (use sparingly)
void time_delay_us_precise(PreciseDuration_t delay_us) {
    if (delay_us == 0) return;
    
    PreciseTime_t start_time = time_get_precise_us();
    
    while (time_diff_us(start_time, time_get_precise_us()) < delay_us) {
        // Busy wait - CPU intensive but precise
        __NOP();
    }
}

// System delay using HAL (yields to other tasks in RTOS)
void time_delay_ms(Duration_t delay_ms) {
    if (delay_ms == 0) return;
    
    #if FEATURE_RTOS_ENABLED
        // Use RTOS delay - yields to other tasks
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    #else
        // Use HAL delay
        HAL_Delay(delay_ms);
    #endif
}

// Non-blocking delay check
bool time_delay_nb_ms(SystemTime_t* start_time, Duration_t delay_ms) {
    if (*start_time == 0) {
        *start_time = time_get_system_ms();
        return false; // First call - delay not complete
    }
    
    if (time_diff_ms(*start_time, time_get_system_ms()) >= delay_ms) {
        *start_time = 0; // Reset for next use
        return true; // Delay complete
    }
    
    return false; // Delay not complete
}
```

## Motor Control Timing

### Motion Profile Timing
```c
// Motion profile time parameters
typedef struct {
    Duration_t acceleration_time_ms;    // Time to reach max velocity
    Duration_t constant_time_ms;        // Time at constant velocity
    Duration_t deceleration_time_ms;    // Time to decelerate to stop
    Duration_t total_move_time_ms;      // Total movement time
    SystemTime_t move_start_time;       // When movement started
} MotionTiming_t;

// Calculate motion timing for trapezoidal profile
void motion_calculate_timing(MotionTiming_t* timing, 
                           float target_angle, 
                           float max_velocity_dps, 
                           float acceleration_dps2) {
    float total_distance = fabsf(target_angle);
    
    // Time to reach max velocity
    float accel_time_s = max_velocity_dps / acceleration_dps2;
    
    // Distance covered during acceleration
    float accel_distance = 0.5f * acceleration_dps2 * accel_time_s * accel_time_s;
    
    // Check if we reach max velocity
    if (2.0f * accel_distance >= total_distance) {
        // Triangular profile - never reach max velocity
        accel_time_s = sqrtf(total_distance / acceleration_dps2);
        timing->acceleration_time_ms = (Duration_t)(accel_time_s * 1000.0f);
        timing->constant_time_ms = 0;
        timing->deceleration_time_ms = timing->acceleration_time_ms;
    } else {
        // Trapezoidal profile
        timing->acceleration_time_ms = (Duration_t)(accel_time_s * 1000.0f);
        timing->deceleration_time_ms = timing->acceleration_time_ms;
        
        float constant_distance = total_distance - 2.0f * accel_distance;
        float constant_time_s = constant_distance / max_velocity_dps;
        timing->constant_time_ms = (Duration_t)(constant_time_s * 1000.0f);
    }
    
    timing->total_move_time_ms = timing->acceleration_time_ms + 
                                timing->constant_time_ms + 
                                timing->deceleration_time_ms;
    timing->move_start_time = 0; // Set when motion starts
}

// Get current motion phase
typedef enum {
    MOTION_PHASE_IDLE = 0,
    MOTION_PHASE_ACCELERATION = 1,
    MOTION_PHASE_CONSTANT = 2,
    MOTION_PHASE_DECELERATION = 3,
    MOTION_PHASE_COMPLETE = 4
} MotionPhase_t;

MotionPhase_t motion_get_current_phase(const MotionTiming_t* timing) {
    if (timing->move_start_time == 0) {
        return MOTION_PHASE_IDLE;
    }
    
    Duration_t elapsed = time_diff_ms(timing->move_start_time, time_get_system_ms());
    
    if (elapsed < timing->acceleration_time_ms) {
        return MOTION_PHASE_ACCELERATION;
    } else if (elapsed < (timing->acceleration_time_ms + timing->constant_time_ms)) {
        return MOTION_PHASE_CONSTANT;
    } else if (elapsed < timing->total_move_time_ms) {
        return MOTION_PHASE_DECELERATION;
    } else {
        return MOTION_PHASE_COMPLETE;
    }
}
```

### Control Loop Timing
```c
// Control loop timing configuration
#define MOTOR_CONTROL_PERIOD_MS         1       // 1000 Hz control loop
#define ENCODER_READ_PERIOD_MS          1       // 1000 Hz encoder reading
#define SAFETY_CHECK_PERIOD_MS          10      // 100 Hz safety checks
#define STATUS_UPDATE_PERIOD_MS         100     // 10 Hz status updates
#define COMM_PROCESS_PERIOD_MS          10      // 100 Hz communication

// Control loop timing structure
typedef struct {
    SystemTime_t last_motor_control;    // Last motor control update
    SystemTime_t last_encoder_read;     // Last encoder reading
    SystemTime_t last_safety_check;     // Last safety check
    SystemTime_t last_status_update;    // Last status update
    SystemTime_t last_comm_process;     // Last communication processing
    
    // Timing statistics
    Duration_t max_control_jitter_ms;   // Maximum control loop jitter
    Duration_t avg_control_period_ms;   // Average control period
    uint32_t missed_deadlines;          // Count of missed deadlines
} ControlLoopTiming_t;

// Initialize control loop timing
void control_timing_init(ControlLoopTiming_t* timing) {
    SystemTime_t current_time = time_get_system_ms();
    
    timing->last_motor_control = current_time;
    timing->last_encoder_read = current_time;
    timing->last_safety_check = current_time;
    timing->last_status_update = current_time;
    timing->last_comm_process = current_time;
    
    timing->max_control_jitter_ms = 0;
    timing->avg_control_period_ms = MOTOR_CONTROL_PERIOD_MS;
    timing->missed_deadlines = 0;
}

// Check if control task should run
bool control_timing_should_run_motor_control(ControlLoopTiming_t* timing) {
    SystemTime_t current_time = time_get_system_ms();
    Duration_t elapsed = time_diff_ms(timing->last_motor_control, current_time);
    
    if (elapsed >= MOTOR_CONTROL_PERIOD_MS) {
        // Update statistics
        Duration_t jitter = (elapsed > MOTOR_CONTROL_PERIOD_MS) ? 
                           (elapsed - MOTOR_CONTROL_PERIOD_MS) : 0;
        if (jitter > timing->max_control_jitter_ms) {
            timing->max_control_jitter_ms = jitter;
        }
        
        if (elapsed > (MOTOR_CONTROL_PERIOD_MS * 2)) {
            timing->missed_deadlines++;
        }
        
        // Update average period (simple moving average)
        timing->avg_control_period_ms = 
            (timing->avg_control_period_ms * 7 + elapsed) / 8;
        
        timing->last_motor_control = current_time;
        return true;
    }
    
    return false;
}

// Similar functions for other control tasks...
bool control_timing_should_run_encoder_read(ControlLoopTiming_t* timing);
bool control_timing_should_run_safety_check(ControlLoopTiming_t* timing);
bool control_timing_should_run_status_update(ControlLoopTiming_t* timing);
bool control_timing_should_run_comm_process(ControlLoopTiming_t* timing);
```

## Real-Time Clock Integration

### RTC Configuration for Absolute Time
```c
// RTC time structure
typedef struct {
    uint8_t hours;                      // 0-23
    uint8_t minutes;                    // 0-59
    uint8_t seconds;                    // 0-59
    uint16_t milliseconds;              // 0-999 (calculated)
    uint8_t day;                        // 1-31  
    uint8_t month;                      // 1-12
    uint16_t year;                      // Full year (e.g., 2024)
    uint8_t weekday;                    // 1-7 (Monday = 1)
} RTCDateTime_t;

// Initialize RTC with default time
void rtc_init_default_time(void) {
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    
    // Set default time: 2024-01-01 00:00:00
    sTime.Hours = 0;
    sTime.Minutes = 0;
    sTime.Seconds = 0;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    
    sDate.WeekDay = RTC_WEEKDAY_MONDAY;
    sDate.Month = RTC_MONTH_JANUARY;
    sDate.Date = 1;
    sDate.Year = 24; // RTC uses 2-digit year (2024 = 24)
    
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

// Get current RTC time
bool rtc_get_datetime(RTCDateTime_t* datetime) {
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    
    if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
        return false;
    }
    
    if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
        return false;
    }
    
    datetime->hours = sTime.Hours;
    datetime->minutes = sTime.Minutes;
    datetime->seconds = sTime.Seconds;
    datetime->milliseconds = 0; // RTC doesn't provide milliseconds
    datetime->day = sDate.Date;
    datetime->month = sDate.Month;
    datetime->year = 2000 + sDate.Year; // Convert to full year
    datetime->weekday = sDate.WeekDay;
    
    return true;
}

// Convert timestamp to readable string
void rtc_format_datetime(const RTCDateTime_t* datetime, char* buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, 
             "%04d-%02d-%02d %02d:%02d:%02d",
             datetime->year, datetime->month, datetime->day,
             datetime->hours, datetime->minutes, datetime->seconds);
}
```

## Performance Monitoring

### Timing Statistics
```c
// Performance timing structure
typedef struct {
    const char* name;                   // Function/task name
    uint32_t call_count;                // Number of calls
    PreciseDuration_t total_time_us;    // Total execution time
    PreciseDuration_t min_time_us;      // Minimum execution time
    PreciseDuration_t max_time_us;      // Maximum execution time
    PreciseDuration_t avg_time_us;      // Average execution time
    uint32_t overrun_count;             // Times exceeded target duration
    PreciseDuration_t target_time_us;   // Target execution time
} PerformanceStats_t;

// Performance monitoring macros
#if FEATURE_PERFORMANCE_COUNTERS
    #define PERF_MEASURE_START(stats_ptr) \
        PreciseTime_t perf_start_time = time_get_precise_us()
    
    #define PERF_MEASURE_END(stats_ptr) \
        do { \
            PreciseTime_t perf_end_time = time_get_precise_us(); \
            PreciseDuration_t duration = time_diff_us(perf_start_time, perf_end_time); \
            perf_update_stats((stats_ptr), duration); \
        } while(0)
#else
    #define PERF_MEASURE_START(stats_ptr) ((void)0)
    #define PERF_MEASURE_END(stats_ptr) ((void)0)
#endif

// Update performance statistics
void perf_update_stats(PerformanceStats_t* stats, PreciseDuration_t duration_us) {
    stats->call_count++;
    stats->total_time_us += duration_us;
    
    if (stats->call_count == 1) {
        stats->min_time_us = duration_us;
        stats->max_time_us = duration_us;
    } else {
        if (duration_us < stats->min_time_us) {
            stats->min_time_us = duration_us;
        }
        if (duration_us > stats->max_time_us) {
            stats->max_time_us = duration_us;
        }
    }
    
    stats->avg_time_us = stats->total_time_us / stats->call_count;
    
    if (stats->target_time_us > 0 && duration_us > stats->target_time_us) {
        stats->overrun_count++;
    }
}

// Performance statistics for key functions
static PerformanceStats_t perf_motor_control = {"motor_control", 0, 0, 0, 0, 0, 0, 500}; // 500μs target
static PerformanceStats_t perf_encoder_read = {"encoder_read", 0, 0, 0, 0, 0, 0, 100};   // 100μs target
static PerformanceStats_t perf_safety_check = {"safety_check", 0, 0, 0, 0, 0, 0, 200};   // 200μs target

// Generate performance report
void perf_generate_report(char* buffer, size_t buffer_size) {
    size_t offset = 0;
    
    offset += snprintf(buffer + offset, buffer_size - offset,
                      "Performance Report:\n"
                      "Function         | Calls  | Avg(μs) | Min(μs) | Max(μs) | Overruns\n"
                      "-----------------|--------|---------|---------|---------|----------\n");
    
    PerformanceStats_t* stats[] = {&perf_motor_control, &perf_encoder_read, &perf_safety_check};
    
    for (int i = 0; i < 3; i++) {
        offset += snprintf(buffer + offset, buffer_size - offset,
                          "%-16s | %6lu | %7lu | %7lu | %7lu | %8lu\n",
                          stats[i]->name,
                          stats[i]->call_count,
                          stats[i]->avg_time_us,
                          stats[i]->min_time_us,
                          stats[i]->max_time_us,
                          stats[i]->overrun_count);
    }
}
```

## Time Synchronization

### Network Time Synchronization (if Ethernet enabled)
```c
#if FEATURE_ETHERNET_ENABLED
// Simple NTP client for time synchronization
typedef struct {
    uint32_t server_ip;                 // NTP server IP address
    uint16_t server_port;               // NTP server port (123)
    SystemTime_t last_sync_time;        // Last successful sync
    int32_t time_offset_ms;             // Offset from system time
    bool sync_enabled;                  // Synchronization enabled
    uint32_t sync_interval_ms;          // Sync interval (default: 1 hour)
} NTPSync_t;

// Initialize NTP synchronization
void ntp_init(NTPSync_t* ntp, uint32_t server_ip) {
    ntp->server_ip = server_ip;
    ntp->server_port = 123;
    ntp->last_sync_time = 0;
    ntp->time_offset_ms = 0;
    ntp->sync_enabled = true;
    ntp->sync_interval_ms = HOURS_TO_MS(1); // Sync every hour
}

// Perform NTP synchronization (simplified implementation)
bool ntp_sync(NTPSync_t* ntp) {
    // This would implement actual NTP protocol
    // For now, just update sync time
    ntp->last_sync_time = time_get_system_ms();
    return true; // Success
}

// Get synchronized time
SystemTime_t ntp_get_synchronized_time(const NTPSync_t* ntp) {
    return time_get_system_ms() + ntp->time_offset_ms;
}

// Check if time sync is needed
bool ntp_needs_sync(const NTPSync_t* ntp) {
    if (!ntp->sync_enabled) return false;
    
    SystemTime_t current_time = time_get_system_ms();
    Duration_t elapsed = time_diff_ms(ntp->last_sync_time, current_time);
    
    return elapsed >= ntp->sync_interval_ms;
}
#endif // FEATURE_ETHERNET_ENABLED
```

Remember: **Accurate and consistent time handling is crucial for motor control, communication protocols, and system diagnostics. Always use appropriate time resolution for each application and handle timer rollovers correctly.**
