/**
 * @file position_control.h
 * @brief Advanced Position Control with Encoder Feedback - Header
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @details Closed-loop position control system using AS5600 magnetic encoders
 *          Features PID control, feedforward compensation, and multiple homing
 * methods
 */

#ifndef POSITION_CONTROL_H
#define POSITION_CONTROL_H

#include "common/data_types.h"
#include "common/error_codes.h"
#include <stdbool.h>
#include <stdint.h>

// Position control configuration constants
#define POSITION_SETTLED_THRESHOLD 10 // Steps for position settled
#define VELOCITY_SETTLED_THRESHOLD 5  // Steps/sec for velocity settled
#define MAX_POSITION_STEPS 1000000    // Maximum absolute position
#define POSITION_FILTER_ALPHA 0.8f    // Position filter coefficient
#define VELOCITY_FILTER_ALPHA 0.7f    // Velocity filter coefficient

// Default PID parameters (SSOT from motor_config.h)
#define PID_KP_DEFAULT 2.0f
#define PID_KI_DEFAULT 0.1f
#define PID_KD_DEFAULT 0.05f
#define PID_INTEGRAL_LIMIT 1000.0f
#define PID_OUTPUT_LIMIT 2000.0f // Steps/sec

// Feedforward parameters
#define FEEDFORWARD_VEL_GAIN 0.8f
#define FEEDFORWARD_ACCEL_GAIN 0.1f
#define FEEDFORWARD_FRICTION_COMP 50.0f // Steps/sec

// Error codes specific to position control
#define ERROR_POSITION_ERROR_EXCESSIVE 0x4100
#define ERROR_POSITION_OUT_OF_RANGE 0x4101
#define ERROR_HOMING_TIMEOUT 0x4102
#define ERROR_HOMING_FAILED 0x4103

/**
 * @brief Homing methods enumeration
 */
typedef enum {
    HOMING_METHOD_LIMIT_SWITCH = 0, ///< Use limit switch for homing
    HOMING_METHOD_ENCODER_INDEX,    ///< Use encoder index pulse
    HOMING_METHOD_CURRENT_POSITION  ///< Set current position as home
} HomingMethod_t;

/**
 * @brief PID controller structure
 */
typedef struct {
    float kp;             ///< Proportional gain
    float ki;             ///< Integral gain
    float kd;             ///< Derivative gain
    float integral;       ///< Integral accumulator
    float integral_limit; ///< Integral windup limit
    float output_limit;   ///< Output saturation limit
} PIDController_t;

/**
 * @brief Feedforward compensation structure
 */
typedef struct {
    float velocity_gain;         ///< Velocity feedforward gain
    float acceleration_gain;     ///< Acceleration feedforward gain
    float friction_compensation; ///< Static friction compensation
} FeedforwardController_t;

/**
 * @brief Position control limits structure
 */
typedef struct {
    int32_t max_position_error; ///< Maximum allowed position error
    uint32_t max_velocity;      ///< Maximum velocity limit
    uint32_t max_acceleration;  ///< Maximum acceleration limit
} ControlLimits_t;

/**
 * @brief Position control state structure
 */
typedef struct {
    int32_t current_position;  ///< Current encoder position
    int32_t target_position;   ///< Target position
    int32_t position_error;    ///< Position error (target - current)
    int32_t filtered_position; ///< Filtered position
    float velocity;            ///< Current velocity
    bool enabled;              ///< Control loop enabled flag
    bool homed;                ///< Homing completed flag
} ControlState_t;

/**
 * @brief Position filtering structure
 */
typedef struct {
    float position_filter_alpha; ///< Position filter coefficient
    float velocity_filter_alpha; ///< Velocity filter coefficient
} PositionFilter_t;

/**
 * @brief Control history for derivative calculation
 */
typedef struct {
    int32_t last_position;         ///< Previous position
    float last_error;              ///< Previous error
    float last_pid_output;         ///< Last PID output
    float last_feedforward_output; ///< Last feedforward output
    float last_target_velocity;    ///< Last target velocity
} ControlHistory_t;

/**
 * @brief Complete position controller structure
 */
typedef struct {
    PIDController_t pid;                 ///< PID controller
    FeedforwardController_t feedforward; ///< Feedforward controller
    ControlLimits_t limits;              ///< Control limits
    ControlState_t state;                ///< Current state
    PositionFilter_t filter;             ///< Filtering parameters
    ControlHistory_t history;            ///< Control history
} PositionControl_t;

/**
 * @brief Homing configuration structure
 */
typedef struct {
    HomingMethod_t method;     ///< Homing method
    int8_t search_direction;   ///< Search direction (+1 or -1)
    uint32_t search_speed;     ///< Homing search speed
    uint32_t backoff_speed;    ///< Backoff speed after switch hit
    uint32_t backoff_distance; ///< Backoff distance in steps
    int32_t home_offset;       ///< Home position offset
    uint32_t timeout_ms;       ///< Homing timeout in milliseconds
} HomingConfig_t;

/**
 * @brief Position control status structure
 */
typedef struct {
    bool enabled;             ///< Controller enabled
    bool homed;               ///< Homing completed
    int32_t current_position; ///< Current position
    int32_t target_position;  ///< Target position
    int32_t position_error;   ///< Position error
    float velocity;           ///< Current velocity
    float pid_output;         ///< Last PID output
    float feedforward_output; ///< Last feedforward output
    bool position_settled;    ///< Position settled flag
} PositionControlStatus_t;

// Core position control functions
SystemError_t position_control_init(void);
SystemError_t position_control_init_motor(uint8_t motor_id);
SystemError_t position_control_update(uint8_t motor_id, uint32_t dt_ms);

// Position target management
SystemError_t position_control_set_target(uint8_t motor_id,
                                          int32_t target_position);
SystemError_t position_control_enable(uint8_t motor_id, bool enable);

// Homing functions
SystemError_t position_control_home(uint8_t motor_id,
                                    HomingConfig_t *homing_config);

// Status and monitoring
SystemError_t position_control_get_status(uint8_t motor_id,
                                          PositionControlStatus_t *status);

// PID tuning functions
SystemError_t position_control_set_pid_gains(uint8_t motor_id, float kp,
                                             float ki, float kd);
SystemError_t position_control_get_pid_gains(uint8_t motor_id, float *kp,
                                             float *ki, float *kd);

// Feedforward tuning functions
SystemError_t position_control_set_feedforward_gains(uint8_t motor_id,
                                                     float vel_gain,
                                                     float accel_gain,
                                                     float friction_comp);
SystemError_t position_control_get_feedforward_gains(uint8_t motor_id,
                                                     float *vel_gain,
                                                     float *accel_gain,
                                                     float *friction_comp);

// Limit configuration
SystemError_t position_control_set_limits(uint8_t motor_id,
                                          ControlLimits_t *limits);
SystemError_t position_control_get_limits(uint8_t motor_id,
                                          ControlLimits_t *limits);

// Filter configuration
SystemError_t position_control_set_filter_params(uint8_t motor_id,
                                                 float pos_alpha,
                                                 float vel_alpha);
SystemError_t position_control_get_filter_params(uint8_t motor_id,
                                                 float *pos_alpha,
                                                 float *vel_alpha);

// Utility functions
bool position_control_is_enabled(uint8_t motor_id);
bool position_control_is_homed(uint8_t motor_id);
bool position_control_is_settled(uint8_t motor_id);
int32_t position_control_get_position_error(uint8_t motor_id);

// Internal functions (declared for testing)
static SystemError_t read_encoder_position(uint8_t motor_id,
                                           int32_t *position);
static void apply_position_filter(PositionControl_t *ctrl);
static void calculate_velocity(PositionControl_t *ctrl, uint32_t dt_ms);
static float calculate_pid_output(PositionControl_t *ctrl, uint32_t dt_ms);
static float calculate_feedforward_output(PositionControl_t *ctrl,
                                          uint32_t target_velocity,
                                          uint32_t dt_ms);
static float apply_output_limits(PositionControl_t *ctrl, float output);
static SystemError_t send_motor_command(uint8_t motor_id, float output);
static void update_control_history(PositionControl_t *ctrl);

// Homing internal functions
static SystemError_t perform_limit_switch_homing(uint8_t motor_id,
                                                 HomingConfig_t *config);
static SystemError_t perform_encoder_index_homing(uint8_t motor_id,
                                                  HomingConfig_t *config);
static SystemError_t perform_current_position_homing(uint8_t motor_id,
                                                     HomingConfig_t *config);
static bool check_limit_switch(uint8_t motor_id);

// External dependencies that need to be implemented
extern SystemError_t motor_set_velocity(uint8_t motor_id, int32_t velocity);
extern SystemError_t motor_run(uint8_t motor_id, int8_t direction,
                               uint32_t speed);
extern SystemError_t motor_stop(uint8_t motor_id);
extern SystemError_t as5600_read_angle(uint8_t motor_id, uint16_t *angle);
extern SystemError_t as5600_set_zero_position(uint8_t motor_id);

#endif // POSITION_CONTROL_H
