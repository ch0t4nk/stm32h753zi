#ifndef SAFETY_CONFIG_H
#define SAFETY_CONFIG_H

/**
 * @brief Emergency stop sources enumeration
 * Defines the various sources that can trigger an emergency stop
 */
typedef enum {
    ESTOP_SOURCE_NONE = 0,       /**< No emergency stop */
    ESTOP_SOURCE_BUTTON,         /**< Physical button */
    ESTOP_SOURCE_SOFTWARE,       /**< Software command */
    ESTOP_SOURCE_COMM_LOSS,      /**< Communication loss */
    ESTOP_SOURCE_FAULT_CASCADE,  /**< Fault cascade */
    ESTOP_SOURCE_WATCHDOG,       /**< Watchdog timeout */
    ESTOP_SOURCE_OVERHEAT,       /**< Overtemperature */
    ESTOP_SOURCE_OVERCURRENT,    /**< Overcurrent protection */
    ESTOP_SOURCE_POSITION_LIMIT, /**< Position limit violation */
    ESTOP_SOURCE_COUNT           /**< Number of sources */
} EmergencyStopSource_t;

/**
 * @brief Emergency Stop State enumeration
 * Defines the various states of the emergency stop system
 */
typedef enum {
    EMERGENCY_STOP_NORMAL = 0, /*!< Normal operation - no emergency stop */
    EMERGENCY_STOP_TRIGGERED,  /*!< Emergency stop has been triggered */
    EMERGENCY_STOP_FAULT,      /*!< Emergency stop system fault detected */
    EMERGENCY_STOP_RECOVERY,   /*!< Emergency stop recovery in progress */
    EMERGENCY_STOP_STATE_COUNT /*!< Number of emergency stop states */
} EmergencyStopState_t;

// Ratio of current limit to max current for safety (used in telemetry safety
// checks)
#define SAFETY_CURRENT_LIMIT_RATIO                                            \
    0.8f // 80% of max current (empirical safe margin)

/**
 * @brief Emergency stop status structure
 * Defines the current status and details of the emergency stop system
 */
typedef struct {
    bool active;                  /**< Emergency stop is active */
    bool latched;                 /**< Emergency stop is latched */
    EmergencyStopSource_t source; /**< Source of emergency stop */
    uint32_t timestamp;           /**< Timestamp when activated */
    uint32_t reset_attempts;      /**< Number of reset attempts */
    bool reset_pending;           /**< Reset operation pending */
} EmergencyStopStatus_t;
// Ratio of speed limit to max speed for safety (used in telemetry safety
// checks)
#define SAFETY_SPEED_LIMIT_RATIO                                              \
    0.9f // 90% of max speed (empirical safe margin)
/**
 * @file safety_config.h
 * @brief Safety Configuration - Single Source of Truth (SSOT)
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * @note This file contains ALL safety parameters, thresholds, and watchdog
 * settings. Never hardcode these values elsewhere - always reference this
 * SSOT.
 *
 * TODO: See .github/instructions/safety-systems.instructions.md for safety
 * system architecture
 * TODO: See .github/instructions/safety-systems.instructions.md for watchdog
 * configuration details
 */

#include <stdint.h>

/* ==========================================================================
 */
/* Safety System Configuration Overview (SSOT)                               */
/* ==========================================================================
 */
// TODO: See .github/instructions/safety-systems.instructions.md for fail-safe
// design principles

#define SAFETY_SYSTEM_VERSION "1.0.0"
#define SAFETY_CHECK_INTERVAL_MS 10  // Safety checks every 10ms
#define SAFETY_FAULT_HISTORY_SIZE 32 // Number of fault events to store
#define SAFETY_ENABLE_REDUNDANCY 1   // Enable redundant safety checks

// Fault monitoring configuration
#define MAX_FAULT_RECORDS 64       // Maximum fault records to store
#define FAULT_CHECK_INTERVAL_MS 50 // Fault check interval

// STM32H7 Silicon Revision IDs (SSOT)
#define STM32H7_REV_ID_Y 0x1000
#define STM32H7_REV_ID_V 0x1003
#define STM32H7_REV_ID_FUTURE_MIN 0x1004

/* ==========================================================================
 */
/* Watchdog Timer Configuration (SSOT)                                       */
/* ==========================================================================
 */
// TODO: See .github/instructions/safety-systems.instructions.md for IWDG and
// WWDG configuration details

// Independent Watchdog (IWDG) Configuration
#define IWDG_ENABLE 1       // Enable independent watchdog
#define IWDG_TIMEOUT_MS 100 // Watchdog timeout period
#define IWDG_PRESCALER IWDG_PRESCALER_32
#define IWDG_RELOAD_VALUE 1250 // Calculated for 100ms @ LSI 32kHz

// Window Watchdog (WWDG) Configuration
#define WWDG_ENABLE 1          // Enable window watchdog
#define WWDG_WINDOW_VALUE 127  // Window upper limit
#define WWDG_COUNTER_VALUE 127 // Initial counter value
#define WWDG_PRESCALER WWDG_PRESCALER_8

// Watchdog Kick Timing
#define WATCHDOG_KICK_INTERVAL_MS 50 // Kick watchdog every 50ms
#define WATCHDOG_LATE_KICK_MS 80     // Warning threshold for late kick
#define WATCHDOG_MISSED_KICK_MAX 2 // Max consecutive missed kicks before fault

/* ==========================================================================
 */
/* Motor Safety Limits (SSOT)                                                */
/* ==========================================================================
 */
// TODO: See .github/instructions/safety-systems.instructions.md for motor
// protection implementation

// Current Protection
#define MOTOR_OVERCURRENT_THRESHOLD_MA 1200 // Overcurrent fault threshold
#define MOTOR_OVERCURRENT_TIME_MS 100       // Time above threshold to fault
#define MOTOR_UNDERCURRENT_THRESHOLD_MA 50  // Minimum expected current
#define MOTOR_CURRENT_SPIKE_THRESHOLD 2000  // Instantaneous spike limit

// Thermal Protection
#define MOTOR_TEMP_WARNING_C 70.0f   // Temperature warning level
#define MOTOR_TEMP_CRITICAL_C 85.0f  // Critical temperature - reduce power
#define MOTOR_TEMP_SHUTDOWN_C 95.0f  // Emergency shutdown temperature
#define MOTOR_TEMP_HYSTERESIS_C 5.0f // Temperature hysteresis

// Position Safety Limits
#define MOTOR_POSITION_ERROR_MAX_DEG                                          \
    5.0f // Maximum position error before fault
#define MOTOR_POSITION_ERROR_TIME_MS 1000 // Time at max error before fault
#define MOTOR_RUNAWAY_THRESHOLD_DEG 10.0f // Runaway detection threshold
#define MOTOR_RUNAWAY_TIME_MS 500         // Time to detect runaway

// Speed Safety Limits
#define MOTOR_OVERSPEED_THRESHOLD_RPM 120.0f // 20% above maximum rated speed
#define MOTOR_OVERSPEED_TIME_MS 100          // Time above threshold to fault
#define MOTOR_STALL_DETECTION_TIME_MS 2000   // Time to detect stall condition
#define MOTOR_UNEXPECTED_MOVE_THRESHOLD 0.5f // Degrees of unexpected movement

/* ==========================================================================
 */
/* Encoder Safety Configuration (SSOT)                                       */
/* ==========================================================================
 */
// TODO: See .github/instructions/safety-systems.instructions.md for encoder
// fault detection

// Encoder Communication Faults
#define ENCODER_COMM_TIMEOUT_MS 100       // I2C communication timeout
#define ENCODER_COMM_RETRY_MAX 3          // Maximum communication retries
#define ENCODER_COMM_FAILURE_TIME_MS 1000 // Consecutive failure time to fault

// Encoder Data Validation
#define ENCODER_ANGLE_JUMP_THRESHOLD_DEG                                      \
    30.0f                                // Maximum single-sample angle change
#define ENCODER_NOISE_THRESHOLD_DEG 0.1f // Noise level threshold
#define ENCODER_STUCK_TIME_MS                                                 \
    5000 // Time without change to detect stuck encoder
#define ENCODER_AGC_MIN_VALUE 128 // AS5600 AGC minimum valid value
#define ENCODER_AGC_MAX_VALUE 255 // AS5600 AGC maximum valid value

// Encoder-Motor Correlation Checks
#define ENCODER_MOTOR_CORRELATION_MIN 0.95f // Minimum correlation coefficient
#define ENCODER_MOTOR_DEVIATION_MAX_DEG                                       \
    2.0f // Maximum deviation between encoder and steps
#define ENCODER_CALIBRATION_SAMPLES 100 // Samples for correlation calculation

/* ==========================================================================
 */
/* Communication Safety (SSOT)                                               */
/* ==========================================================================
 */
// TODO: See .github/instructions/comm-protocols.instructions.md for
// communication fault handling

// UART Safety Parameters
#define UART_COMMAND_TIMEOUT_MS 5000    // Maximum time to process command
#define UART_HEARTBEAT_TIMEOUT_MS 10000 // UART heartbeat timeout
#define UART_INVALID_CMD_MAX 5          // Max invalid commands before fault
#define UART_BUFFER_OVERFLOW_ACTION SAFETY_FAULT_COMMUNICATION

// CAN Bus Safety Parameters
#define CAN_HEARTBEAT_TIMEOUT_MS 3000 // CAN heartbeat timeout
#define CAN_ERROR_PASSIVE_MAX 10      // Max CAN error passive states
#define CAN_BUS_OFF_RECOVERY_TIME_MS                                          \
    30000                        // Time before bus-off recovery attempt
#define CAN_MESSAGE_QUEUE_MAX 32 // Maximum queued messages

// Ethernet Safety Parameters
#define ETH_CONNECTION_TIMEOUT_MS 30000 // Ethernet connection timeout
#define ETH_COMMAND_RATE_LIMIT 100      // Max commands per second
#define ETH_BUFFER_OVERFLOW_ACTION SAFETY_FAULT_COMMUNICATION

/* ==========================================================================
 */
/* System Safety Thresholds (SSOT)                                           */
/* ==========================================================================
 */
// TODO: See .github/instructions/safety-systems.instructions.md for overall
// system protection

// Power Supply Monitoring
#define POWER_SUPPLY_MIN_V 11.0f      // Minimum supply voltage
#define POWER_SUPPLY_MAX_V 13.5f      // Maximum supply voltage
#define POWER_SUPPLY_BROWNOUT_V 10.5f // Brownout detection threshold
#define POWER_SUPPLY_OVERVOLT_V 15.0f // Overvoltage shutdown threshold

// MCU Resource Monitoring
#define CPU_USAGE_WARNING_PCT 80     // CPU usage warning threshold
#define CPU_USAGE_CRITICAL_PCT 95    // CPU usage critical threshold
#define MEMORY_USAGE_WARNING_PCT 85  // Memory usage warning
#define MEMORY_USAGE_CRITICAL_PCT 95 // Memory usage critical

// Temperature Monitoring (MCU)
#define MCU_TEMP_WARNING_C 70.0f  // MCU temperature warning
#define MCU_TEMP_CRITICAL_C 85.0f // MCU temperature critical
#define MCU_TEMP_SHUTDOWN_C 95.0f // MCU temperature shutdown

/* ==========================================================================
 */
/* Emergency Stop Configuration (SSOT)                                       */
/* ==========================================================================
 */
// TODO: See .github/instructions/safety-systems.instructions.md for E-stop
// implementation

// Emergency Stop Sources
#define ESTOP_BUTTON_ENABLE 1        // Physical button E-stop
#define ESTOP_SOFTWARE_ENABLE 1      // Software command E-stop
#define ESTOP_COMM_LOSS_ENABLE 1     // Communication loss E-stop
#define ESTOP_FAULT_CASCADE_ENABLE 1 // Fault cascade E-stop

// Emergency Stop Timing
#define ESTOP_REACTION_TIME_MS 10 // Maximum E-stop reaction time
#define ESTOP_DEBOUNCE_TIME_MS                                                \
    50 // E-stop button debounce time (matches tests)
#define ESTOP_MOTOR_STOP_TIME_MS 100       // Time to fully stop motors
#define ESTOP_RESET_DELAY_MS 5000          // Delay before E-stop can be reset
#define ESTOP_RESET_CONFIRMATION_TIME 2000 // Time to confirm E-stop reset

// Emergency Stop Actions
#define ESTOP_ACTION_HARD_STOP 1    // Immediate motor stop
#define ESTOP_ACTION_POWER_OFF 1    // Remove motor power
#define ESTOP_ACTION_BRAKE_ENGAGE 0 // Engage mechanical brake (if available)
#define ESTOP_ACTION_FAULT_LOG 1    // Log fault event

/* ==========================================================================
 */
/* Safety State Definitions (SSOT)                                           */
/* ==========================================================================
 */
// TODO: See .github/instructions/safety-systems.instructions.md for state
// machine implementation

typedef enum {
    SAFETY_STATE_UNKNOWN = 0,    // Initial/unknown state
    SAFETY_STATE_SAFE,           // Normal safe operation
    SAFETY_STATE_WARNING,        // Warning condition detected
    SAFETY_STATE_FAULT,          // Fault condition - motors stopped
    SAFETY_STATE_EMERGENCY_STOP, // Emergency stop activated
    SAFETY_STATE_MAINTENANCE,    // Maintenance mode
    SAFETY_STATE_RECOVERY,       // Fault recovery in progress
    SAFETY_STATE_COUNT           // Number of states
} SafetyState_t;

typedef enum {
    SAFETY_LEVEL_NONE = 0,  // No safety restrictions
    SAFETY_LEVEL_LOW,       // Low safety restrictions
    SAFETY_LEVEL_MEDIUM,    // Medium safety restrictions
    SAFETY_LEVEL_HIGH,      // High safety restrictions
    SAFETY_LEVEL_CRITICAL,  // Critical safety restrictions
    SAFETY_LEVEL_EMERGENCY, // Emergency safety level
    SAFETY_LEVEL_COUNT      // Number of levels
} SafetyLevel_t;

/* ==========================================================================
 */
/* Fault Classification and Priorities (SSOT)                                */
/* ==========================================================================
 */
// TODO: See .github/instructions/safety-systems.instructions.md for fault
// handling priorities

typedef enum {
    SAFETY_FAULT_NONE = 0x00000000, // No fault

    // Critical Faults (Immediate E-stop)
    SAFETY_FAULT_ESTOP_BUTTON = 0x00000001, // Emergency stop button pressed
    SAFETY_FAULT_MOTOR_OVERCURRENT = 0x00000002, // Motor overcurrent
    SAFETY_FAULT_MOTOR_OVERHEAT = 0x00000004,    // Motor overheating
    SAFETY_FAULT_POWER_OVERVOLT = 0x00000008,    // Power supply overvoltage
    SAFETY_FAULT_MCU_OVERHEAT = 0x00000010,      // MCU overheating

    // Serious Faults (Stop motors, allow restart)
    SAFETY_FAULT_ENCODER_LOST = 0x00000100,   // Encoder communication lost
    SAFETY_FAULT_MOTOR_STALL = 0x00000200,    // Motor stall detected
    SAFETY_FAULT_POSITION_ERROR = 0x00000400, // Position error too large
    SAFETY_FAULT_POWER_BROWNOUT = 0x00000800, // Power supply brownout
    SAFETY_FAULT_WATCHDOG = 0x00001000,       // Watchdog timeout

    // Warning Faults (Continue with restrictions)
    SAFETY_FAULT_COMM_TIMEOUT = 0x00010000,  // Communication timeout
    SAFETY_FAULT_ENCODER_NOISE = 0x00020000, // Encoder noise detected
    SAFETY_FAULT_CPU_OVERLOAD = 0x00040000,  // CPU overload
    SAFETY_FAULT_MEMORY_LOW = 0x00080000,    // Low memory
    SAFETY_FAULT_TEMP_WARNING = 0x00100000,  // Temperature warning

    // System Faults
    SAFETY_FAULT_INITIALIZATION = 0x01000000, // System initialization failure
    SAFETY_FAULT_CONFIGURATION = 0x02000000,  // Configuration error
    SAFETY_FAULT_COMMUNICATION = 0x04000000   // General communication fault
} SafetyFaultFlags_t;

/* ==========================================================================
 */
/* Safety Check Configuration (SSOT)                                         */
/* ==========================================================================
 */
// TODO: See .github/instructions/safety-systems.instructions.md for safety
// check implementation

// Safety Check Enable/Disable Flags
#define SAFETY_CHECK_MOTOR_CURRENT 1  // Enable motor current monitoring
#define SAFETY_CHECK_MOTOR_TEMP 1     // Enable motor temperature monitoring
#define SAFETY_CHECK_ENCODER_VALID 1  // Enable encoder validation
#define SAFETY_CHECK_POSITION_LIMIT 1 // Enable position limit checking
#define SAFETY_CHECK_SPEED_LIMIT 1    // Enable speed limit checking
#define SAFETY_CHECK_COMM_TIMEOUT 1   // Enable communication timeout
#define SAFETY_CHECK_POWER_SUPPLY 1   // Enable power supply monitoring
#define SAFETY_CHECK_MCU_RESOURCES 1  // Enable MCU resource monitoring

// Safety Check Periods (milliseconds)
#define SAFETY_CHECK_CRITICAL_PERIOD 1 // Critical checks every 1ms
#define SAFETY_CHECK_NORMAL_PERIOD 10  // Normal checks every 10ms
#define SAFETY_CHECK_SLOW_PERIOD 100   // Slow checks every 100ms

// Safety Monitor Timer Configuration (SSOT)
#define SAFETY_MONITOR_PERIOD_TICKS                                           \
    ((uint32_t)(100 - 1)) // 100μs period for safety monitor timer (TIM3)

/* ==========================================================================
 */
/* Safety Recovery Configuration (SSOT)                                      */
/* ==========================================================================
 */
// TODO: See .github/instructions/safety-systems.instructions.md for fault
// recovery procedures

// Recovery Enable/Disable
#define SAFETY_AUTO_RECOVERY_ENABLE 1   // Enable automatic fault recovery
#define SAFETY_MANUAL_RECOVERY_ENABLE 1 // Enable manual fault recovery
#define SAFETY_RECOVERY_ATTEMPTS_MAX 3  // Maximum recovery attempts

// Recovery Timing
#define SAFETY_RECOVERY_DELAY_MS 5000     // Delay before recovery attempt
#define SAFETY_RECOVERY_TIMEOUT_MS 30000  // Maximum recovery time
#define SAFETY_RECOVERY_COOLDOWN_MS 60000 // Cooldown between recovery attempts

// Recovery Actions
#define RECOVERY_ACTION_MOTOR_RESET 1   // Reset motor drivers
#define RECOVERY_ACTION_ENCODER_RECAL 1 // Recalibrate encoders
#define RECOVERY_ACTION_COMM_RESTART 1  // Restart communications
#define RECOVERY_ACTION_SYSTEM_REBOOT 0 // System reboot (last resort)

#endif /* SAFETY_CONFIG_H */

/**
 * @note Safety Configuration SSOT Rules:
 * 1. All safety parameters MUST be defined here
 * 2. Never hardcode safety thresholds in source files
 * 3. Safety levels and fault priorities centralized
 * 4. Recovery procedures and timing parameters in one place
 * 5. All safety checks configurable via compile-time flags
 */
