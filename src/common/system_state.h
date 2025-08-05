/**
 * @file system_state.h
 * @brief Central System State - Single Source of Truth (SSOT)
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * @note This file defines the central system state structure that serves as the
 *       authoritative source for all system status information. All modules
 *       must reference this single instance - never duplicate state
 * information.
 *
 * TODO: See .instructions/system-state.md for state synchronization details
 * TODO: See .instructions/thread-safety.md for multi-threaded access patterns
 */

#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include "config/motor_config.h"
#include "config/safety_config.h"
#include "error_codes.h"
#include <stdbool.h>
#include <stdint.h>

/* ========================================================================== */
/* System State Structure Overview (SSOT)                                    */
/* ========================================================================== */
// TODO: See .instructions/state-management.md for state update procedures

#define SYSTEM_STATE_VERSION 0x0100   // State structure version (v1.0)
#define SYSTEM_STATE_MAGIC 0xDEADBEEF // Magic number for validation
#define MAX_STATE_OBSERVERS 8         // Maximum state change observers

/* ========================================================================== */
/* Motor State Information (SSOT)                                            */
/* ========================================================================== */

typedef struct {
  // Current motor status
  MotorState_t current_state;      // Current motor state (enum from motor_config.h)
  MotorControlMode_t control_mode; // Current control mode
  MotorFaultFlags_t fault_flags;   // Active fault flags

  // Position information
  float current_angle_deg;  // Current angle in degrees
  float target_angle_deg;   // Target angle in degrees
  float home_angle_deg;     // Home reference angle
  float position_error_deg; // Position error (target - current)

  // Motion information
  float current_speed_rpm;  // Current speed in RPM
  float target_speed_rpm;   // Target speed in RPM
  float acceleration_rpm_s; // Current acceleration
  float speed_error_rpm;    // Speed error (target - current)

  // Driver status (L6470 specific)
  uint16_t driver_status_reg; // L6470 status register value
  bool driver_busy;           // Driver busy flag
  bool driver_fault;          // Driver fault flag
  uint32_t step_count;        // Accumulated step count

  // Performance metrics
  uint32_t move_start_time_ms;    // Time when current move started
  uint32_t move_duration_ms;      // Duration of current move
  float average_speed_rpm;        // Average speed during move
  uint32_t total_moves_completed; // Total completed moves
  uint32_t total_faults_detected; // Total faults detected

  // Timing information
  uint32_t last_update_time_ms;   // Last state update timestamp
  uint32_t update_rate_hz;        // State update rate
  uint32_t control_loop_overruns; // Control loop timing violations

} MotorStateInfo_t;

/* ========================================================================== */
/* Encoder State Information (SSOT)                                          */
/* ========================================================================== */

typedef struct {
  // Raw encoder data
  uint16_t raw_angle_value; // Raw 12-bit angle value (0-4095)
  float raw_angle_deg;      // Raw angle in degrees (0-360)
  uint8_t agc_value;        // AS5600 AGC value (magnet strength)
  bool magnet_detected;     // Magnet presence detection

  // Processed encoder data
  float calibrated_angle_deg; // Calibrated angle with offset
  float filtered_angle_deg;   // Filtered angle (noise reduction)
  float angular_velocity_rpm; // Calculated angular velocity
  float zero_offset_deg;      // Zero position offset

  // Encoder health monitoring
  bool communication_ok;               // I2C communication status
  uint32_t communication_errors;       // Cumulative communication errors
  uint32_t last_valid_reading_time_ms; // Timestamp of last valid reading
  uint32_t consecutive_errors;         // Consecutive error count

  // Data quality metrics
  float noise_level_deg;         // Measured noise level
  float max_angle_jump_deg;      // Maximum single-sample angle change
  uint32_t angle_jumps_detected; // Count of angle jumps detected
  bool stuck_detector_flag;      // Encoder stuck detection flag

  // Calibration status
  bool calibration_valid;            // Calibration data validity
  uint32_t calibration_timestamp_ms; // When calibration was performed
  uint32_t calibration_sample_count; // Number of calibration samples
  float calibration_accuracy_deg;    // Estimated calibration accuracy

} EncoderState_t;

/* ========================================================================== */
/* Communication State Information (SSOT)                                    */
/* ========================================================================== */

typedef struct {
  // UART Communication
  struct {
    bool initialized;               // UART initialization status
    bool connected;                 // Host connection status
    uint32_t bytes_transmitted;     // Total bytes transmitted
    uint32_t bytes_received;        // Total bytes received
    uint32_t transmission_errors;   // Transmission error count
    uint32_t last_activity_time_ms; // Last communication activity
    uint32_t command_count;         // Total commands processed
    uint32_t invalid_commands;      // Invalid command count
  } uart;

  // CAN Bus Communication
  struct {
    bool initialized;                // CAN initialization status
    bool bus_active;                 // CAN bus activity status
    uint32_t messages_transmitted;   // Total CAN messages sent
    uint32_t messages_received;      // Total CAN messages received
    uint32_t transmission_errors;    // CAN transmission errors
    uint32_t bus_off_events;         // Bus-off event count
    uint32_t last_heartbeat_time_ms; // Last heartbeat received
    uint8_t node_status;             // This node's status
    uint8_t active_nodes;            // Count of active nodes on bus
  } can;

  // Ethernet Communication
  struct {
    bool initialized;                // Ethernet initialization status
    bool link_up;                    // Physical link status
    bool ip_assigned;                // IP address assignment status
    uint32_t tcp_connections_active; // Active TCP connections
    uint32_t tcp_connections_total;  // Total TCP connections handled
    uint32_t bytes_transmitted;      // Total bytes transmitted
    uint32_t bytes_received;         // Total bytes received
    uint32_t connection_errors;      // Connection error count
    uint32_t last_activity_time_ms;  // Last network activity
  } ethernet;

  // I2C Communication (for encoders)
  struct {
    bool initialized;                  // I2C initialization status
    uint32_t transactions_completed;   // Successful I2C transactions
    uint32_t transaction_errors;       // I2C transaction errors
    uint32_t timeout_events;           // I2C timeout events
    uint32_t arbitration_lost_events;  // Arbitration lost events
    uint32_t last_transaction_time_ms; // Last I2C transaction time
  } i2c;

  // SPI Communication (for motor drivers)
  struct {
    bool initialized;                  // SPI initialization status
    uint32_t transactions_completed;   // Successful SPI transactions
    uint32_t transaction_errors;       // SPI transaction errors
    uint32_t timeout_events;           // SPI timeout events
    uint32_t last_transaction_time_ms; // Last SPI transaction time
  } spi;

} CommunicationState_t;

/* ========================================================================== */
/* Safety System State Information (SSOT)                                    */
/* ========================================================================== */

typedef struct {
  // Overall safety status
  SafetyState_t current_state;       // Current safety state (enum from safety_config.h)
  SafetyLevel_t current_level;       // Current safety level
  SafetyFaultFlags_t active_faults;  // Currently active safety faults
  SafetyFaultFlags_t latched_faults; // Latched safety faults (require reset)

  // Emergency stop status
  bool emergency_stop_active;      // Emergency stop currently active
  uint32_t emergency_stop_time_ms; // Time when E-stop was activated
  uint32_t emergency_stop_source;  // Source that triggered E-stop
  bool emergency_stop_resetable;   // E-stop can be reset

  // Watchdog status
  bool watchdog_enabled;               // Watchdog timer enabled
  uint32_t watchdog_last_kick_time_ms; // Last watchdog kick time
  uint32_t watchdog_kicks_missed;      // Count of missed watchdog kicks
  uint32_t watchdog_timeout_events;    // Watchdog timeout event count

  // Power monitoring
  float supply_voltage_v;      // Current supply voltage
  float supply_current_a;      // Current supply current
  bool power_supply_ok;        // Power supply within limits
  uint32_t brownout_events;    // Brownout event count
  uint32_t overvoltage_events; // Overvoltage event count

  // Temperature monitoring
  float mcu_temperature_c;    // MCU internal temperature
  float motor1_temperature_c; // Motor 1 temperature (if available)
  float motor2_temperature_c; // Motor 2 temperature (if available)
  bool temperature_warning;   // Temperature warning active
  bool temperature_critical;  // Temperature critical

  // Safety check counters
  uint32_t safety_checks_performed;      // Total safety checks performed
  uint32_t safety_violations_detected;   // Safety violations detected
  uint32_t safety_recoveries_attempted;  // Recovery attempts made
  uint32_t safety_recoveries_successful; // Successful recoveries

  // Safety timing
  uint32_t last_safety_check_time_ms; // Last safety check timestamp
  uint32_t safety_check_interval_ms;  // Current safety check interval
  uint32_t safety_check_overruns;     // Safety check timing violations

} SafetyStateInfo_t;

/* ========================================================================== */
/* System Performance State Information (SSOT)                               */
/* ========================================================================== */

typedef struct {
  // CPU utilization
  float cpu_usage_percent;             // CPU utilization percentage
  uint32_t cpu_usage_peak_percent;     // Peak CPU utilization
  uint32_t task_switches_per_second;   // Task switching rate
  uint32_t interrupt_count_per_second; // Interrupt rate

  // Memory utilization
  uint32_t heap_used_bytes;      // Heap memory used
  uint32_t heap_free_bytes;      // Heap memory free
  uint32_t heap_peak_used_bytes; // Peak heap usage
  uint32_t stack_used_bytes;     // Stack memory used
  uint32_t stack_free_bytes;     // Stack memory free

  // Control loop performance
  uint32_t control_loop_frequency_hz;    // Actual control loop frequency
  uint32_t control_loop_jitter_us;       // Control loop timing jitter
  uint32_t control_Loop_max_duration_us; // Maximum control loop duration
  uint32_t control_loop_overruns;        // Control loop overrun count

  // Communication performance
  uint32_t uart_throughput_bps;     // UART throughput
  uint32_t can_message_rate;        // CAN message rate
  uint32_t ethernet_throughput_bps; // Ethernet throughput
  uint32_t i2c_transaction_rate;    // I2C transaction rate

  // Error statistics
  uint32_t total_errors_detected;    // Total errors detected
  uint32_t critical_errors_detected; // Critical errors detected
  uint32_t error_recovery_successes; // Successful error recoveries
  uint32_t error_recovery_failures;  // Failed error recoveries

} PerformanceState_t;

/* ========================================================================== */
/* Main System State Structure (SSOT)                                        */
/* ========================================================================== */

typedef struct {
  // State structure metadata
  uint32_t magic_number;        // Magic number for validation
  uint16_t version;             // State structure version
  uint32_t checksum;            // State structure checksum
  uint32_t last_update_time_ms; // Last state update timestamp

  // System identification
  uint32_t system_id;         // Unique system identifier
  uint32_t hardware_revision; // Hardware revision number
  uint32_t firmware_version;  // Firmware version number
  uint32_t boot_count;        // Number of system boots

  // System timing
  uint32_t system_uptime_ms;     // System uptime in milliseconds
  uint32_t system_start_time_ms; // System start timestamp
  uint32_t last_reset_reason;    // Reason for last reset
  uint32_t total_runtime_hours;  // Total runtime in hours

  // Motor states (one per motor)
  MotorStateInfo_t motors[MAX_MOTORS]; // Motor state information

  // Encoder states (one per encoder)
  EncoderState_t encoders[MAX_MOTORS]; // Encoder state information

  // Communication state
  CommunicationState_t communication; // Communication status

  // Safety system state
  SafetyStateInfo_t safety; // Safety system status

  // Performance metrics
  PerformanceState_t performance; // System performance metrics

  // System status
  uint32_t last_error_code;  // Last error code encountered
  uint32_t system_warnings;  // Active system warnings
  bool system_ready;         // System ready for operation
  bool calibration_required; // System calibration required
  bool maintenance_required; // Maintenance required flag

  // State change tracking
  uint32_t state_change_counter; // Incremented on state changes
  uint32_t observers_registered; // Number of state observers
  void (*observers[MAX_STATE_OBSERVERS])(
      void); // State change observer callbacks

} SystemState_t;

/* ========================================================================== */
/* Global System State Instance (SSOT)                                       */
/* ========================================================================== */

// Single global system state instance - THE authoritative source
extern SystemState_t g_system_state;

/* ========================================================================== */
/* System State Access Functions (SSOT)                                      */
/* ========================================================================== */
// TODO: See .instructions/state-access.md for thread-safe access patterns

/**
 * @brief Initialize the system state structure
 * @return SystemError_t Error code or SYSTEM_OK
 */
SystemError_t system_state_init(void);

/**
 * @brief Update system state timestamp and checksum
 * @return SystemError_t Error code or SYSTEM_OK
 */
SystemError_t system_state_update(void);

/**
 * @brief Validate system state integrity
 * @return true if state is valid, false otherwise
 */
bool system_state_validate(void);

/**
 * @brief Register state change observer
 * @param observer Function to call on state changes
 * @return SystemError_t Error code or SYSTEM_OK
 */
SystemError_t system_state_register_observer(void (*observer)(void));

/**
 * @brief Notify all observers of state change
 */
void system_state_notify_observers(void);

/* ========================================================================== */
/* Motor State Access Functions (SSOT)                                       */
/* ========================================================================== */

/**
 * @brief Set motor target angle
 * @param motor_id Motor identifier (0 or 1)
 * @param angle_deg Target angle in degrees
 * @return SystemError_t Error code or SYSTEM_OK
 */
SystemError_t system_state_set_motor_target_angle(uint8_t motor_id,
                                                  float angle_deg);

/**
 * @brief Get motor current angle
 * @param motor_id Motor identifier (0 or 1)
 * @param angle_deg Pointer to store current angle
 * @return SystemError_t Error code or SYSTEM_OK
 */
SystemError_t system_state_get_motor_current_angle(uint8_t motor_id,
                                                   float *angle_deg);

/**
 * @brief Set motor fault flags
 * @param motor_id Motor identifier (0 or 1)
 * @param fault_flags Fault flags to set
 * @return SystemError_t Error code or SYSTEM_OK
 */
SystemError_t system_state_set_motor_fault_flags(uint8_t motor_id,
                                                 MotorFaultFlags_t fault_flags);

/**
 * @brief Clear motor fault flags
 * @param motor_id Motor identifier (0 or 1)
 * @param fault_flags Fault flags to clear
 * @return SystemError_t Error code or SYSTEM_OK
 */
SystemError_t
system_state_clear_motor_fault_flags(uint8_t motor_id,
                                     MotorFaultFlags_t fault_flags);

/* ========================================================================== */
/* Encoder State Access Functions (SSOT)                                     */
/* ========================================================================== */

/**
 * @brief Update encoder raw angle value
 * @param encoder_id Encoder identifier (0 or 1)
 * @param raw_value Raw 12-bit encoder value
 * @return SystemError_t Error code or SYSTEM_OK
 */
SystemError_t system_state_set_encoder_raw_angle(uint8_t encoder_id,
                                                 uint16_t raw_value);

/**
 * @brief Get encoder calibrated angle
 * @param encoder_id Encoder identifier (0 or 1)
 * @param angle_deg Pointer to store calibrated angle
 * @return SystemError_t Error code or SYSTEM_OK
 */
SystemError_t system_state_get_encoder_angle(uint8_t encoder_id,
                                             float *angle_deg);

/**
 * @brief Set encoder communication status
 * @param encoder_id Encoder identifier (0 or 1)
 * @param communication_ok Communication status
 * @return SystemError_t Error code or SYSTEM_OK
 */
SystemError_t system_state_set_encoder_comm_status(uint8_t encoder_id,
                                                   bool communication_ok);

/* ========================================================================== */
/* Safety State Access Functions (SSOT)                                      */
/* ========================================================================== */

/**
 * @brief Set emergency stop state
 * @param active Emergency stop active flag
 * @param source Source that triggered emergency stop
 * @return SystemError_t Error code or SYSTEM_OK
 */
SystemError_t system_state_set_emergency_stop(bool active, uint32_t source);

/**
 * @brief Set safety fault flags
 * @param fault_flags Safety fault flags to set
 * @return SystemError_t Error code or SYSTEM_OK
 */
SystemError_t
system_state_set_safety_fault_flags(SafetyFaultFlags_t fault_flags);

/**
 * @brief Clear safety fault flags
 * @param fault_flags Safety fault flags to clear
 * @return SystemError_t Error code or SYSTEM_OK
 */
SystemError_t
system_state_clear_safety_fault_flags(SafetyFaultFlags_t fault_flags);

/**
 * @brief Update watchdog kick timestamp
 * @return SystemError_t Error code or SYSTEM_OK
 */
SystemError_t system_state_update_watchdog_kick(void);

/* ========================================================================== */
/* Performance State Access Functions (SSOT)                                 */
/* ========================================================================== */

/**
 * @brief Update CPU usage statistics
 * @param usage_percent Current CPU usage percentage
 * @return SystemError_t Error code or SYSTEM_OK
 */
SystemError_t system_state_update_cpu_usage(float usage_percent);

/**
 * @brief Update memory usage statistics
 * @param heap_used_bytes Heap memory used
 * @param stack_used_bytes Stack memory used
 * @return SystemError_t Error code or SYSTEM_OK
 */
SystemError_t system_state_update_memory_usage(uint32_t heap_used_bytes,
                                               uint32_t stack_used_bytes);

/**
 * @brief Increment error counter
 * @param error_code Error code that occurred
 * @return SystemError_t Error code or SYSTEM_OK
 */
SystemError_t system_state_increment_error_count(uint32_t error_code);

#endif /* SYSTEM_STATE_H */

/**
 * @note System State SSOT Rules:
 * 1. Only ONE global system state instance exists (g_system_state)
 * 2. ALL modules access state through provided access functions
 * 3. Never duplicate state information in other structures
 * 4. State changes must notify observers for consistency
 * 5. Always validate state integrity after updates
 * 6. Use thread-safe access patterns in multi-threaded environments
 */
