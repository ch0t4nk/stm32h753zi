// Motor supply voltage for telemetry power calculation (used in telemetry)
#define MOTOR_SUPPLY_VOLTAGE                                                  \
    24.0f // Typical supply voltage for IHM02A1/L6470 system

// Floating-point constant for microseconds per second (used in timing
// calculations)
#define MICROSECONDS_PER_SECOND_F 1000000.0f
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
#define SPN2_FRAMEWORK_ENABLED 1 // Enable X-CUBE-SPN2 framework
#define SPN2_IHM02A1_BOARD 1     // X-NUCLEO-IHM02A1 board support
#define SPN2_DUAL_MOTOR_CONFIG 1 // Dual motor configuration

#define MCSDK_POSITION_CONTROL 1  // Enhanced position control algorithms
#define MCSDK_MOTION_PROFILING 1  // Advanced motion profiling
#define MCSDK_SAFETY_MONITORING 1 // Enhanced safety and fault detection
#define MCSDK_WORKBENCH_SUPPORT 1 // Motor Control Workbench integration

#define HYBRID_CONTROL_MODE 1  // SPN2 base + MCSDK enhancement
#define CLOSED_LOOP_FEEDBACK 1 // AS5600 encoder feedback integration

/* ==========================================================================
 */
/* Motor Physical Parameters (SSOT)                                          */
/* ==========================================================================
 */
#define MOTOR_VOLTAGE_V 12.0f     // Motor supply voltage
#define MOTOR_CURRENT_MA 1000     // Motor rated current (mA)
#define MOTOR_RESISTANCE_OHM 2.8f // Motor winding resistance
#define MOTOR_INDUCTANCE_MH 3.2f  // Motor winding inductance

#define MOTOR_STEPS_PER_REV 200 // Full steps per revolution (1.8°/step)
#define MOTOR_MICROSTEPS 128    // Microstepping setting (1/128)
#define MOTOR_TOTAL_STEPS (MOTOR_STEPS_PER_REV * MOTOR_MICROSTEPS)
#define MOTOR_DEGREES_PER_STEP (360.0f / MOTOR_TOTAL_STEPS)

// Position Control Tolerances
#define POSITION_TOLERANCE_DEG 0.1f // Position tolerance in degrees
#define POSITION_TOLERANCE_STEPS                                              \
    (POSITION_TOLERANCE_DEG / MOTOR_DEGREES_PER_STEP)

#define MOTOR_TYPICAL_MECHANICAL_TIME_CONSTANT_S ((timestamp_ms_t)0.1f)
#define MOTOR_TYPICAL_ELECTRICAL_TIME_CONSTANT_S ((timestamp_ms_t)0.002f)
#define MOTOR_CHARACTERIZATION_SWEEP_AMPLITUDE_DEG ((angle_deg_t)5.0f)
#define MOTOR_CHARACTERIZATION_SAMPLE_DELAY_MS ((timestamp_ms_t)1)

#define MOTOR_CONTROL_LOOP_PERIOD_TICKS ((timestamp_ms_t)(1000 - 1))
#define MOTOR_POSITION_CONTROL_TIMESTEP_MS ((timestamp_ms_t)1)
#define MOTOR_MULTI_MOTOR_TIMESTEP_MS ((timestamp_ms_t)2)

#define DEMO_TIMER_PERIOD_1MS ((timestamp_ms_t)1000)

#define MOTOR_MAX_SPEED_RPM 100.0f   // Maximum safe speed
#define MOTOR_MIN_SPEED_RPM 0.1f     // Minimum controllable speed
#define MOTOR_MAX_ACCEL_RPM_S 50.0f  // Maximum acceleration
#define MOTOR_MAX_DECEL_RPM_S 100.0f // Maximum deceleration (can be higher)

#define MOTOR_MAX_CURRENT_A (MOTOR_CURRENT_MA / 1000.0f)
#define MOTOR_MAX_TEMPERATURE_C 85.0f // Maximum motor temperature

#define MOTOR_MAX_SPEED_DPS (MOTOR_MAX_SPEED_RPM * 6.0f)
#define MOTOR_MIN_SPEED_DPS (MOTOR_MIN_SPEED_RPM * 6.0f)

#define MOTOR_CONTROL_LOOP_PERIOD_MS 20 // 50Hz control loop (20ms period)

#define MOTOR_MIN_ANGLE_DEG 0.0f   // Minimum position
#define MOTOR_MAX_ANGLE_DEG 360.0f // Maximum position (full rotation)
#define MOTOR_HOME_ANGLE_DEG 0.0f  // Home/reference position

#define MOTOR1_MIN_POSITION_DEG -180.0f // Motor 1 minimum position
#define MOTOR1_MAX_POSITION_DEG 180.0f  // Motor 1 maximum position
#define MOTOR2_MIN_POSITION_DEG -90.0f  // Motor 2 minimum position
#define MOTOR2_MAX_POSITION_DEG 90.0f   // Motor 2 maximum position

/* ==========================================================================
 */
/* L6470 Driver Register Configuration (SSOT)                                */
/* ==========================================================================
 */
#define L6470_MAX_SPEED 0x041
#define L6470_MIN_SPEED 0x000
#define L6470_ACC 0x08A
#define L6470_DEC 0x08A
#define L6470_FS_SPD 0x027
#define L6470_SAFE_DEFAULT_ACC L6470_ACC
#define L6470_SAFE_DEFAULT_DEC L6470_DEC
#define L6470_SAFE_DEFAULT_MAX_SPEED L6470_MAX_SPEED
#define L6470_SAFE_DEFAULT_OCD_TH 0x8
#define L6470_SAFE_DEFAULT_KVAL_RUN L6470_KVAL_RUN
#define L6470_KVAL_HOLD 0x29
#define L6470_KVAL_RUN 0x29
#define L6470_KVAL_ACC 0x29
#define L6470_KVAL_DEC 0x29
#define MOTOR1_KVAL_HOLD_DEFAULT L6470_KVAL_HOLD
#define MOTOR1_KVAL_RUN_DEFAULT L6470_KVAL_RUN
#define MOTOR1_KVAL_ACC_DEFAULT L6470_KVAL_ACC
#define MOTOR1_KVAL_DEC_DEFAULT L6470_KVAL_DEC
#define MOTOR1_ACCELERATION_DEFAULT L6470_ACC
#define MOTOR1_DECELERATION_DEFAULT L6470_DEC
#define MOTOR1_MAX_SPEED_DEFAULT L6470_MAX_SPEED
#define MOTOR1_MIN_SPEED_DEFAULT L6470_MIN_SPEED
#define L6470_INT_SPD 0x0408
#define L6470_ST_SLP 0x19
#define L6470_FN_SLP_ACC 0x29
#define L6470_FN_SLP_DEC 0x29
#define L6470_STEP_MODE 0x07
#define L6470_SYNC_EN 0x80
#define L6470_SYNC_SEL 0x00
#define L6470_OCD_TH 0x0F
#define L6470_STALL_TH 0x40
#define L6470_PWM_FREQ 0x00
#define L6470_PWM_MUL 0x00
#define L6470_PWM_DIV 0x00
#define L6470_SLEW_RATE 0x0290
#define L6470_OC_SD 0x08
#define L6470_POW_SR 0x00
#define L6470_TSW 0x01

#define MOTOR_MAX_POSITION_STEPS (200 * 16 * 360)
#define MOTOR_MAX_SPEED_STEPS_PER_SEC 3200
#define MOTOR_MAX_CURRENT_MA 1000
#define MOTOR_MAX_SPEED MOTOR_MAX_SPEED_STEPS_PER_SEC
#define MOTOR_MAX_ACCELERATION 1600
#define MOTOR1_ACCELERATION L6470_ACC

/* ==========================================================================
 */
/* Motor State Definitions (SSOT)                                            */
/* ==========================================================================
 */
typedef enum {
    MOTOR_STATE_UNINITIALIZED = 0,
    MOTOR_STATE_IDLE,
    MOTOR_STATE_ACCELERATING,
    MOTOR_STATE_RUNNING,
    MOTOR_STATE_DECELERATING,
    MOTOR_STATE_HOMING,
    MOTOR_STATE_FAULT,
    MOTOR_STATE_EMERGENCY_STOP,
    MOTOR_STATE_COUNT
} MotorState_t;

typedef enum {
    MOTOR_MODE_OPEN_LOOP = 0,
    MOTOR_MODE_CLOSED_LOOP,
    MOTOR_MODE_TORQUE_CONTROL,
    MOTOR_MODE_SPEED_CONTROL,
    MOTOR_MODE_POSITION_CONTROL,
    MOTOR_MODE_COUNT
} MotorControlMode_t;

/* ==========================================================================
 */
/* Control Loop Timing Configuration (SSOT)                                 */
/* ==========================================================================
 */
#define CONTROL_LOOP_FREQ_HZ 1000 // Control loop frequency
#define CONTROL_LOOP_PERIOD_MS                                                \
    (1000 / CONTROL_LOOP_FREQ_HZ) // Control loop period

#endif /* MOTOR_CONFIG_H */
