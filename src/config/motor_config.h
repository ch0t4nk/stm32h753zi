/**
 * @file motor_config.h
 * @brief Motor and L6470 Driver Configuration - Single Source of Truth (SSOT)
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * @note This file contains ALL motor parameters, L6470 settings, and motion
 * limits. Never hardcode these values elsewhere - always reference this SSOT.
 *
 * TODO: See .github/instructions/l6470-registers.instructions.md for motor
 * characterization data
 * TODO: See .github/instructions/l6470-registers.instructions.md for register
 * configuration details
 */

#ifndef MOTOR_CONFIG_H
#define MOTOR_CONFIG_H

#include "common/data_types.h"
#include <stdint.h>

/* ==========================================================================
 */
/* Motor Count and Identification (SSOT)                                     */
/* ==========================================================================
 */
#define MAX_MOTORS 2 // Two motors in this system
#define MOTOR_1_ID 0 // First motor identifier
#define MOTOR_2_ID 1 // Second motor identifier

/* ==========================================================================
 */
/* X-CUBE-SPN2 + MCSDK Motor Control Configuration (SSOT)                    */
/* ==========================================================================
 */
// Primary Framework: X-CUBE-SPN2 for L6470 stepper control
#define SPN2_FRAMEWORK_ENABLED 1 // Enable X-CUBE-SPN2 framework
#define SPN2_IHM02A1_BOARD 1     // X-NUCLEO-IHM02A1 board support
#define SPN2_DUAL_MOTOR_CONFIG 1 // Dual motor configuration

// MCSDK Selective Enhancement Layer
#define MCSDK_POSITION_CONTROL 1  // Enhanced position control algorithms
#define MCSDK_MOTION_PROFILING 1  // Advanced motion profiling
#define MCSDK_SAFETY_MONITORING 1 // Enhanced safety and fault detection
#define MCSDK_WORKBENCH_SUPPORT 1 // Motor Control Workbench integration

// Integration Layer Configuration
#define HYBRID_CONTROL_MODE 1  // SPN2 base + MCSDK enhancement
#define CLOSED_LOOP_FEEDBACK 1 // AS5600 encoder feedback integration

/* ==========================================================================
 */
/* Motor Physical Parameters (SSOT)                                          */
/* ==========================================================================
 */
// TODO: See .github/instructions/l6470-registers.instructions.md for motor
// specifications

// Motor electrical characteristics
#define MOTOR_VOLTAGE_V 12.0f     // Motor supply voltage
#define MOTOR_CURRENT_MA 1000     // Motor rated current (mA)
#define MOTOR_RESISTANCE_OHM 2.8f // Motor winding resistance
#define MOTOR_INDUCTANCE_MH 3.2f  // Motor winding inductance

// Motor mechanical characteristics
#define MOTOR_STEPS_PER_REV 200 // Full steps per revolution (1.8°/step)
#define MOTOR_MICROSTEPS 128    // Microstepping setting (1/128)
#define MOTOR_TOTAL_STEPS (MOTOR_STEPS_PER_REV * MOTOR_MICROSTEPS)
#define MOTOR_DEGREES_PER_STEP (360.0f / MOTOR_TOTAL_STEPS)

// Motor time constants (for characterization)
#define MOTOR_TYPICAL_MECHANICAL_TIME_CONSTANT_S                               \
  ((timestamp_ms_t)0.1f) // 100ms typical mechanical time constant
#define MOTOR_TYPICAL_ELECTRICAL_TIME_CONSTANT_S                               \
  ((timestamp_ms_t)0.002f) // 2ms typical electrical time constant
#define MOTOR_CHARACTERIZATION_SWEEP_AMPLITUDE_DEG                             \
  ((angle_deg_t)5.0f) // 5 degree amplitude for safety
#define MOTOR_CHARACTERIZATION_SAMPLE_DELAY_MS                                 \
  ((timestamp_ms_t)1) // 1ms delay for 1kHz sampling rate

// Real-time control timing constants (SSOT)
#define MOTOR_CONTROL_LOOP_PERIOD_TICKS                                        \
  ((timestamp_ms_t)(1000 - 1)) // 1ms period for timer
#define MOTOR_POSITION_CONTROL_TIMESTEP_MS                                     \
  ((timestamp_ms_t)1) // 1ms time step for position control
#define MOTOR_MULTI_MOTOR_TIMESTEP_MS                                          \
  ((timestamp_ms_t)2) // 2ms time step (500Hz) for coordination

// Demo timing constants (SSOT)
#define DEMO_TIMER_PERIOD_1MS                                                  \
  ((timestamp_ms_t)1000) // 1ms period for demo timer

// Motor mechanical limits
#define MOTOR_MAX_SPEED_RPM 100.0f   // Maximum safe speed
#define MOTOR_MIN_SPEED_RPM 0.1f     // Minimum controllable speed
#define MOTOR_MAX_ACCEL_RPM_S 50.0f  // Maximum acceleration
#define MOTOR_MAX_DECEL_RPM_S 100.0f // Maximum deceleration (can be higher)

// Motor current limits (for compatibility with optimization system)
#define MOTOR_MAX_CURRENT_A (MOTOR_CURRENT_MA / 1000.0f) // Convert mA to A
#define MOTOR_MAX_TEMPERATURE_C 85.0f // Maximum motor temperature

// Speed conversion constants
#define MOTOR_MAX_SPEED_DPS                                                    \
  (MOTOR_MAX_SPEED_RPM * 6.0f) // Convert RPM to degrees/second
#define MOTOR_MIN_SPEED_DPS                                                    \
  (MOTOR_MIN_SPEED_RPM * 6.0f) // Convert RPM to degrees/second

// Control loop timing
#define MOTOR_CONTROL_LOOP_PERIOD_MS 20 // 50Hz control loop (20ms period)

// Position limits (degrees)
#define MOTOR_MIN_ANGLE_DEG 0.0f   // Minimum position
#define MOTOR_MAX_ANGLE_DEG 360.0f // Maximum position (full rotation)
#define MOTOR_HOME_ANGLE_DEG 0.0f  // Home/reference position

// Motor-specific position limits
#define MOTOR1_MIN_POSITION_DEG -180.0f // Motor 1 minimum position
#define MOTOR1_MAX_POSITION_DEG 180.0f  // Motor 1 maximum position
#define MOTOR2_MIN_POSITION_DEG -90.0f  // Motor 2 minimum position
#define MOTOR2_MAX_POSITION_DEG 90.0f   // Motor 2 maximum position

/* ==========================================================================
 */
/* L6470 Driver Register Configuration (SSOT)                                */
/* ==========================================================================
 */
// TODO: See .github/instructions/l6470-registers.instructions.md for detailed
// register descriptions

// Motion Profile Parameters
#define L6470_MAX_SPEED 0x041 // Max speed register value (100 RPM equiv)
#define L6470_MIN_SPEED 0x000 // Min speed register value
#define L6470_ACC 0x08A       // Acceleration register value
#define L6470_DEC 0x08A       // Deceleration register value
#define L6470_FS_SPD 0x027    // Full step speed threshold

// L6470 Safe Default Values (for simulation and initialization)
#define L6470_SAFE_DEFAULT_ACC L6470_ACC // Safe default acceleration
#define L6470_SAFE_DEFAULT_DEC L6470_DEC // Safe default deceleration
#define L6470_SAFE_DEFAULT_MAX_SPEED L6470_MAX_SPEED // Safe default max speed
#define L6470_SAFE_DEFAULT_OCD_TH 0x8 // Safe overcurrent threshold
#define L6470_SAFE_DEFAULT_KVAL_RUN                                            \
  L6470_KVAL_RUN // Safe default running current

// Current Control (KVAL registers)
#define L6470_KVAL_HOLD 0x29 // Holding current (25% of max)
#define L6470_KVAL_RUN 0x29  // Running current (25% of max)
#define L6470_KVAL_ACC 0x29  // Acceleration current (25% of max)
#define L6470_KVAL_DEC 0x29  // Deceleration current (25% of max)

// KVAL Default Values (for motor characterization compatibility)
#define MOTOR1_KVAL_HOLD_DEFAULT L6470_KVAL_HOLD
#define MOTOR1_KVAL_RUN_DEFAULT L6470_KVAL_RUN
#define MOTOR1_KVAL_ACC_DEFAULT L6470_KVAL_ACC
#define MOTOR1_KVAL_DEC_DEFAULT L6470_KVAL_DEC

// Motion Profile Default Values (for motor characterization compatibility)
#define MOTOR1_ACCELERATION_DEFAULT L6470_ACC
#define MOTOR1_DECELERATION_DEFAULT L6470_DEC
#define MOTOR1_MAX_SPEED_DEFAULT L6470_MAX_SPEED
#define MOTOR1_MIN_SPEED_DEFAULT L6470_MIN_SPEED

// Back-EMF Compensation (BEMF parameters)
#define L6470_INT_SPD 0x0408  // Intersect speed for BEMF
#define L6470_ST_SLP 0x19     // Start slope for BEMF
#define L6470_FN_SLP_ACC 0x29 // Final slope acceleration
#define L6470_FN_SLP_DEC 0x29 // Final slope deceleration

// Step Mode Configuration
#define L6470_STEP_MODE 0x07 // 1/128 microstepping
#define L6470_SYNC_EN 0x80   // Enable sync output
#define L6470_SYNC_SEL 0x00  // Sync signal selection

// Overcurrent Detection
#define L6470_OCD_TH 0x0F   // Overcurrent threshold (3A)
#define L6470_STALL_TH 0x40 // Stall detection threshold

// Switching Configuration
#define L6470_PWM_FREQ 0x00 // PWM frequency (15.6 kHz)
#define L6470_PWM_MUL 0x00  // PWM multiplier
#define L6470_PWM_DIV 0x00  // PWM divisor

// Slew Rate and Driver Configuration
#define L6470_SLEW_RATE 0x0290 // Slew rate configuration
#define L6470_OC_SD 0x08       // Overcurrent shutdown enable
#define L6470_POW_SR 0x00      // Power slew rate
#define L6470_TSW 0x01         // Switching time

/* ==========================================================================
 */
/* Motor Limits and Safety Parameters (SSOT)                                 */
/* ==========================================================================
 */
// Physical and safety limits
#define MOTOR_MAX_POSITION_STEPS                                               \
  (200 * 16 * 360) // 200 steps/rev * 16 microsteps * 360°
#define MOTOR_MAX_SPEED_STEPS_PER_SEC 3200 // Maximum speed in steps/sec
#define MOTOR_MAX_CURRENT_MA 1000          // Maximum current in milliamps
// NOTE: MOTOR_CURRENT_MA defined above as 1000mA - using that value for normal
// operation

// Motor control aliases for motion profile compatibility
#define MOTOR_MAX_SPEED MOTOR_MAX_SPEED_STEPS_PER_SEC // Alias for steps/sec
#define MOTOR_MAX_ACCELERATION 1600 // Maximum acceleration in steps/sec²

// Motor 1 acceleration (from SSOT - for communication protocol compatibility)
#define MOTOR1_ACCELERATION L6470_ACC // Use L6470 register value

/* ==========================================================================
 */
/* L6470 Command Definitions (SSOT)                                          */
/* ==========================================================================
 */
// TODO: See .github/instructions/l6470-registers.instructions.md for command
// protocol implementation

// Movement Commands
#define L6470_CMD_NOP 0x00       // No operation
#define L6470_CMD_SETPARAM 0x00  // Set parameter (OR with param address)
#define L6470_CMD_GETPARAM 0x20  // Get parameter (OR with param address)
#define L6470_CMD_RUN 0x50       // Run with constant speed
#define L6470_CMD_STEPCLOCK 0x58 // Step clock mode
#define L6470_CMD_MOVE 0x40      // Move N steps
#define L6470_CMD_GOTO 0x60      // Go to absolute position
#define L6470_CMD_GOTO_DIR 0x68  // Go to position with direction
#define L6470_CMD_GOUNTIL 0x82   // Go until switch/flag
#define L6470_CMD_RELEASESW 0x92 // Release switch

// L6470 Command Padding and Special Values (SSOT)
#define L6470_CMD_PADDING 0x00   // Padding byte for multi-motor commands
#define L6470_PARAM_NULL 0x00    // Null parameter value
#define L6470_DEFAULT_VALUE 0x00 // Default return value for stubs

// Control Commands
#define L6470_CMD_HARDSTOP 0xB8 // Immediate stop
#define L6470_CMD_SOFTSTOP 0xB0 // Soft stop with deceleration
#define L6470_CMD_HARDHIZ 0xA8  // Immediate high-Z
#define L6470_CMD_SOFTHIZ 0xA0  // Soft high-Z

// Status Commands
#define L6470_CMD_GETSTATUS 0xD0 // Get status and clear flags
#define L6470_CMD_RESERVED1 0xEB // Reserved
#define L6470_CMD_RESERVED2 0xF8 // Reserved

// Reset Commands
#define L6470_CMD_RESET_POS 0xD8    // Reset position counter
#define L6470_CMD_RESET_DEVICE 0xC0 // Reset device

/* ==========================================================================
 */
/* L6470 Register Addresses (SSOT)                                           */
/* ==========================================================================
 */
// Motion Control Registers
#define L6470_REG_ABS_POS 0x01   // Absolute position (22-bit)
#define L6470_REG_EL_POS 0x02    // Electrical position (9-bit)
#define L6470_REG_MARK 0x03      // Mark position (22-bit)
#define L6470_REG_SPEED 0x04     // Current speed (20-bit)
#define L6470_REG_ACC 0x05       // Acceleration (12-bit)
#define L6470_REG_DEC 0x06       // Deceleration (12-bit)
#define L6470_REG_MAX_SPEED 0x07 // Maximum speed (10-bit)
#define L6470_REG_MIN_SPEED 0x08 // Minimum speed (13-bit)
#define L6470_REG_FS_SPD 0x15    // Full step speed (10-bit)

/* ==========================================================================
 */
/* L6470 Bit Masks (SSOT) - Hardware-defined bit patterns                   */
/* ==========================================================================
 */
// Status register and data bit masks (from L6470 datasheet)
#define L6470_STATUS_MASK_16BIT 0xFFFF     // 16-bit status register mask
#define L6470_POSITION_MASK_22BIT 0x3FFFFF // 22-bit position value mask
#define L6470_DATA_MASK_24BIT 0xFFFFFF // 24-bit data mask for 3-byte operations
#define L6470_BYTE_MASK 0xFF           // 8-bit byte mask for SPI operations
#define L6470_STATUS_STUB_VALUE 0xDEAD // Mock status value for testing

// Direction control bits (from L6470 datasheet)
#define L6470_DIRECTION_FORWARD 0x01 // Forward direction bit
#define L6470_DIRECTION_REVERSE 0x00 // Reverse direction bit (default)

// Speed calculation constants (from L6470 datasheet specifications)
#define L6470_SPEED_SCALE_FACTOR 0x100 // Speed scaling factor (256)
#define L6470_MAX_SPEED_VALUE 0x3FF    // Maximum 10-bit speed value (1023)

/* ==========================================================================
 */
/* HAL Mock Test Patterns (SSOT) - Test validation constants                */
/* ==========================================================================
 */
// Standard test patterns for HAL abstraction validation
#define HAL_MOCK_TEST_PATTERN_BASE                                             \
  0xA5 // Primary test pattern (alternating bits)
#define HAL_MOCK_TEST_PATTERN_ALT 0x5A  // Alternate test pattern (inverted)
#define HAL_MOCK_TEST_PATTERN_INIT 0x00 // Initial test state pattern

// Current Control Registers
#define L6470_REG_KVAL_HOLD 0x09 // Holding KVAL
#define L6470_REG_KVAL_RUN 0x0A  // Running KVAL
#define L6470_REG_KVAL_ACC 0x0B  // Acceleration starting KVAL
#define L6470_REG_KVAL_DEC 0x0C  // Deceleration starting KVAL

// BEMF Registers
#define L6470_REG_INT_SPD 0x0D    // Intersect speed
#define L6470_REG_ST_SLP 0x0E     // Start slope
#define L6470_REG_FN_SLP_ACC 0x0F // Final slope acceleration
#define L6470_REG_FN_SLP_DEC 0x10 // Final slope deceleration

// Configuration Registers
#define L6470_REG_K_THERM 0x11   // Thermal compensation factor
#define L6470_REG_ADC_OUT 0x12   // ADC output
#define L6470_REG_OCD_TH 0x13    // Overcurrent threshold
#define L6470_REG_STALL_TH 0x14  // Stall threshold
#define L6470_REG_STEP_MODE 0x16 // Step mode and sync
#define L6470_REG_ALARM_EN 0x17  // Alarm enable
#define L6470_REG_CONFIG 0x18    // IC configuration
#define L6470_REG_STATUS 0x19    // Status register

/* ==========================================================================
 */
/* Motion Control Parameters (SSOT)                                          */
/* ==========================================================================
 */
// TODO: See .github/instructions/l6470-registers.instructions.md for
// closed-loop tuning parameters

// Control Loop Timing
#define CONTROL_LOOP_FREQ_HZ 1000 // Control loop frequency
#define CONTROL_LOOP_PERIOD_MS (1000 / CONTROL_LOOP_FREQ_HZ)

// Position Control Parameters
#define POSITION_TOLERANCE_DEG 0.1f // Acceptable position error
#define VELOCITY_TOLERANCE_RPM 0.5f // Acceptable velocity error
#define SETTLING_TIME_MS 500        // Time to reach target position

// Safety Limits
#define MAX_POSITION_ERROR_DEG 5.0f  // Maximum position error before fault
#define MAX_FOLLOWING_ERROR_DEG 2.0f // Maximum real-time following error
#define STALL_DETECTION_TIME_MS 1000 // Time to detect motor stall

// Homing and Calibration
#define HOMING_SPEED_RPM 10.0f     // Speed for homing moves
#define CALIBRATION_SAMPLES 100    // Encoder samples for calibration
#define ENCODER_RESOLUTION_BITS 12 // AS5600 resolution
#define ENCODER_VALUE_MASK 0x0FFF  // 12-bit encoder value mask (AS5600)

/* ==========================================================================
 */
/* Motor State Definitions (SSOT)                                            */
/* ==========================================================================
 */
// TODO: See .github/instructions/l6470-registers.instructions.md for state
// machine implementation

typedef enum {
  MOTOR_STATE_UNINITIALIZED = 0, // Motor not initialized
  MOTOR_STATE_IDLE,              // Motor stopped and ready
  MOTOR_STATE_ACCELERATING,      // Motor accelerating to target speed
  MOTOR_STATE_RUNNING,           // Motor running at constant speed
  MOTOR_STATE_DECELERATING,      // Motor decelerating to stop
  MOTOR_STATE_HOMING,            // Motor performing homing sequence
  MOTOR_STATE_FAULT,             // Motor in fault state
  MOTOR_STATE_EMERGENCY_STOP,    // Emergency stop activated
  MOTOR_STATE_COUNT              // Number of states (for validation)
} MotorState_t;

typedef enum {
  MOTOR_MODE_OPEN_LOOP = 0,    // Open-loop stepper control
  MOTOR_MODE_CLOSED_LOOP,      // Closed-loop with encoder feedback
  MOTOR_MODE_TORQUE_CONTROL,   // Torque control mode
  MOTOR_MODE_SPEED_CONTROL,    // Speed control mode
  MOTOR_MODE_POSITION_CONTROL, // Position control mode
  MOTOR_MODE_COUNT             // Number of modes
} MotorControlMode_t;

/* ==========================================================================
 */
/* Fault and Error Definitions (SSOT)                                        */
/* ==========================================================================
 */
// TODO: See .github/instructions/l6470-registers.instructions.md for fault
// handling procedures
//
// NOTE: Motor fault definitions moved to src/safety/fault_monitor.h for SSOT
// Use MotorFaultType_t from fault_monitor.h instead of local definitions
//
// #include "safety/fault_monitor.h" to access motor fault enumerations

/* ==========================================================================
 */
/* Motor Performance Monitoring (SSOT)                                       */
/* ==========================================================================
 */
// TODO: See .github/instructions/l6470-registers.instructions.md for
// performance tracking

// Performance thresholds
#define MOTOR_EFFICIENCY_MIN_PCT 80.0f // Minimum acceptable efficiency
#define MOTOR_TEMP_WARNING_C 70.0f     // Temperature warning threshold
#define MOTOR_TEMP_FAULT_C 85.0f       // Temperature fault threshold
#define MOTOR_VIBRATION_MAX_G 2.0f // Maximum vibration (if sensor available)

// Statistics collection
#define STATS_SAMPLE_INTERVAL_MS 100 // Statistics sampling rate
#define STATS_HISTORY_SAMPLES 600    // 1 minute of history at 100ms rate

// ============================================================================
// Motor Communication IRQ Numbers
// ============================================================================

// L6470 SPI Communication IRQ
#define MOTOR_SPI_IRQ SPI2_IRQn // L6470 stepper driver SPI interrupt

// ============================================================================
// L6470 Hardware Constants
// ============================================================================

// L6470 Speed Calculation Constants - keeping only one set
#define L6470_SPEED_SCALE_FACTOR_HEX                                           \
  0x100 // 256 decimal - speed calculation scaling factor
#define L6470_SPEED_CALC_DIVISOR                                               \
  0x3FF // 1023 decimal - speed calculation divisor
// Note: L6470_SPEED_SCALE_FACTOR and L6470_MAX_SPEED_VALUE already defined
// above

// L6470 Register Value Constants
#define L6470_MIN_SPEED_DEFAULT 0x000 // Minimum speed register default value
#define L6470_FS_SPD_DEFAULT 0x027    // Full step speed register default value

// L6470 Direction Control Constants
#define L6470_DIRECTION_FORWARD_BIT 0x01 // Forward direction bit
#define L6470_DIRECTION_REVERSE_BIT 0x00 // Reverse direction bit

#endif /* MOTOR_CONFIG_H */

/**
 * @note Motor Configuration SSOT Rules:
 * 1. All motor parameters MUST be defined here
 * 2. Never hardcode L6470 register values in source files
 * 3. All motion limits and safety parameters centralized
 * 4. Motor states and modes defined once for consistency
 * 5. Fault definitions must match communication protocol error codes
 */
