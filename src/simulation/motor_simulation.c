/**
 * @file motor_simulation.c
 * @brief Motor and Encoder Simulation Framework Implementation
 *
 * Safe simulation of L6470 stepper drivers and AS5600 encoders
 * for testing control algorithms without hardware.
 */

#include "motor_simulation.h"
#include "config/motor_config.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ==========================================================================
 */
/* Private Variables                                                         */
/* ==========================================================================
 */

static MotorSimulation_t g_simulation = {0};
static bool g_simulation_initialized = false;

/* ==========================================================================
 */
/* Private Function Declarations                                             */
/* ==========================================================================
 */

static void l6470_sim_update_motion(uint8_t motor_id, float delta_time_s);
static void as5600_sim_update_position(uint8_t encoder_id, float delta_time_s);
static uint16_t as5600_sim_add_noise(uint16_t angle, float noise_amplitude);
static void l6470_sim_update_status_register(uint8_t motor_id);
static bool l6470_sim_validate_register_write(uint8_t reg_addr,
                                              uint32_t value);

/* ==========================================================================
 */
/* Global Simulation Control Implementation                                  */
/* ==========================================================================
 */

bool motor_simulation_init(SimulationMode_t mode) {
    if (g_simulation_initialized) {
        return true; // Already initialized
    }

    memset(&g_simulation, 0, sizeof(g_simulation));
    g_simulation.mode = mode;
    g_simulation.initialized = true;
    g_simulation.simulation_time_ms = 0;
    g_simulation.tick_counter = 0;

    // Initialize all motors to safe defaults
    for (uint8_t i = 0; i < SIM_MAX_MOTORS; i++) {
        L6470_SimState_t *motor = &g_simulation.motors[i];
        motor->motor_id = i;
        motor->initialized = false;
        motor->enabled = false;
        motor->current_position = 0;
        motor->target_position = 0;
        motor->current_speed = 0.0f;
        motor->target_speed = 0.0f;
        motor->acceleration = 1000.0f; // steps/s²
        motor->deceleration = 1000.0f; // steps/s²
        motor->motion_state = SIM_MOTOR_STOPPED;
        motor->status_register = L6470_STATUS_HIZ; // Start in high-Z state
        motor->busy = false;
        motor->direction = true;
        motor->active_fault = SIM_FAULT_NONE;
        motor->command_count = 0;

        // Initialize registers to safe defaults
        motor->registers[L6470_REG_ACC] = L6470_SAFE_DEFAULT_ACC;
        motor->registers[L6470_REG_DEC] = L6470_SAFE_DEFAULT_DEC;
        motor->registers[L6470_REG_MAX_SPEED] = L6470_SAFE_DEFAULT_MAX_SPEED;
        motor->registers[L6470_REG_OCD_TH] = L6470_SAFE_DEFAULT_OCD_TH;
        motor->registers[L6470_REG_KVAL_RUN] = L6470_SAFE_DEFAULT_KVAL_RUN;
    }

    // Initialize all encoders
    for (uint8_t i = 0; i < SIM_MAX_ENCODERS; i++) {
        AS5600_SimState_t *encoder = &g_simulation.encoders[i];
        encoder->encoder_id = i;
        encoder->initialized = false;
        encoder->magnet_detected = true; // Assume magnet present
        encoder->raw_angle = 0;
        encoder->filtered_angle = 0;
        encoder->actual_position_deg = 0.0f;
        encoder->pattern = SIM_PATTERN_STATIC;
        encoder->pattern_parameter = 0.0f;
        encoder->magnitude = 2048; // Mid-range magnetic field
        encoder->agc_value = 128;  // Optimal AGC value
        encoder->magnet_too_strong = false;
        encoder->magnet_too_weak = false;
        encoder->active_fault = SIM_FAULT_NONE;
        encoder->noise_amplitude = 0.0f;
        encoder->noise_enabled = false;

        // Initialize registers
        encoder->registers[AS5600_REG_STATUS] =
            0x20; // MD bit set (magnet detected)
        encoder->registers[AS5600_REG_AGC] = 128; // Optimal AGC
    }

    g_simulation_initialized = true;
    return true;
}

void motor_simulation_update(uint32_t delta_time_ms) {
    if (!g_simulation_initialized) {
        return;
    }

    float delta_time_s = delta_time_ms / 1000.0f;
    g_simulation.simulation_time_ms += delta_time_ms;
    g_simulation.tick_counter++;
    g_simulation.total_updates++;

    // Update motor simulations
    for (uint8_t i = 0; i < SIM_MAX_MOTORS; i++) {
        if (g_simulation.motors[i].initialized) {
            l6470_sim_update_motion(i, delta_time_s);
            l6470_sim_update_status_register(i);
        }
    }

    // Update encoder simulations
    for (uint8_t i = 0; i < SIM_MAX_ENCODERS; i++) {
        if (g_simulation.encoders[i].initialized) {
            as5600_sim_update_position(i, delta_time_s);
        }
    }
}

bool motor_simulation_is_active(void) {
    return g_simulation_initialized &&
           (g_simulation.mode != SIM_MODE_DISABLED);
}

void motor_simulation_reset(void) {
    g_simulation_initialized = false;
    memset(&g_simulation, 0, sizeof(g_simulation));
}

/* ==========================================================================
 */
/* L6470 Simulation Implementation                                           */
/* ==========================================================================
 */

bool l6470_sim_init(uint8_t motor_id) {
    if (motor_id >= SIM_MAX_MOTORS || !g_simulation_initialized) {
        return false;
    }

    L6470_SimState_t *motor = &g_simulation.motors[motor_id];
    motor->initialized = true;
    motor->enabled = true;
    motor->last_update_time = g_simulation.simulation_time_ms;

    // Clear status flags except HIZ
    motor->status_register = L6470_STATUS_HIZ;
    motor->busy = false;

    return true;
}

bool l6470_sim_write_register(uint8_t motor_id, uint8_t reg_addr,
                              uint32_t value) {
    if (motor_id >= SIM_MAX_MOTORS ||
        !g_simulation.motors[motor_id].initialized) {
        return false;
    }

    // Validate register write
    if (!l6470_sim_validate_register_write(reg_addr, value)) {
        return false;
    }

    L6470_SimState_t *motor = &g_simulation.motors[motor_id];
    motor->registers[reg_addr] = value;

    // Update simulation parameters based on register writes
    switch (reg_addr) {
    case L6470_REG_ACC:
        motor->acceleration =
            (float)value * 250e-9f * powf(2, 40); // Convert to steps/s²
        break;

    case L6470_REG_DEC:
        motor->deceleration =
            (float)value * 250e-9f * powf(2, 40); // Convert to steps/s²
        break;

    case L6470_REG_ABS_POS:
        motor->current_position = (int32_t)value;
        break;

    default:
        break;
    }

    return true;
}

bool l6470_sim_read_register(uint8_t motor_id, uint8_t reg_addr,
                             uint32_t *value) {
    if (motor_id >= SIM_MAX_MOTORS ||
        !g_simulation.motors[motor_id].initialized || value == NULL) {
        return false;
    }

    L6470_SimState_t *motor = &g_simulation.motors[motor_id];

    // Handle special read-only registers
    switch (reg_addr) {
    case L6470_REG_ABS_POS:
        *value = (uint32_t)motor->current_position;
        break;

    case L6470_REG_SPEED:
        *value = (uint32_t)(motor->current_speed / (250e-9f * powf(2, 28)));
        break;

    case L6470_REG_STATUS:
        *value = motor->status_register;
        // Clear flags after reading (per L6470 behavior)
        motor->status_register &=
            ~(L6470_STATUS_WRONG_CMD | L6470_STATUS_NOTPERF_CMD);
        break;

    default:
        *value = motor->registers[reg_addr];
        break;
    }

    return true;
}

bool l6470_sim_execute_command(uint8_t motor_id, uint8_t command,
                               uint32_t parameter) {
    if (motor_id >= SIM_MAX_MOTORS ||
        !g_simulation.motors[motor_id].initialized) {
        return false;
    }

    L6470_SimState_t *motor = &g_simulation.motors[motor_id];
    motor->command_count++;
    g_simulation.total_commands++;

    // Extract base command (remove parameter bits)
    uint8_t base_command = command & 0xF0;

    switch (base_command) {
    case L6470_CMD_RUN:
        motor->direction = (command & 0x01) != 0;
        motor->target_speed =
            (float)parameter * 250e-9f * powf(2, 28); // Convert to steps/s
        motor->motion_state = SIM_MOTOR_ACCELERATING;
        motor->busy = true;
        motor->status_register &= ~L6470_STATUS_HIZ;
        break;

    case L6470_CMD_MOVE:
        motor->direction = (command & 0x01) != 0;
        motor->target_position =
            motor->current_position +
            (motor->direction ? (int32_t)parameter : -(int32_t)parameter);
        motor->motion_state = SIM_MOTOR_ACCELERATING;
        motor->busy = true;
        motor->status_register &= ~L6470_STATUS_HIZ;
        break;

    case L6470_CMD_GOTO:
        motor->target_position = (int32_t)parameter;
        motor->direction = (motor->target_position > motor->current_position);
        motor->motion_state = SIM_MOTOR_ACCELERATING;
        motor->busy = true;
        motor->status_register &= ~L6470_STATUS_HIZ;
        break;

    case L6470_CMD_SOFT_STOP:
        motor->target_speed = 0.0f;
        motor->motion_state = SIM_MOTOR_DECELERATING;
        break;

    case L6470_CMD_HARD_STOP:
        motor->current_speed = 0.0f;
        motor->target_speed = 0.0f;
        motor->motion_state = SIM_MOTOR_STOPPED;
        motor->busy = false;
        break;

    case L6470_CMD_SOFT_HIZ:
        motor->target_speed = 0.0f;
        motor->motion_state = SIM_MOTOR_DECELERATING;
        // Will set HIZ when stopped
        break;

    case L6470_CMD_HARD_HIZ:
        motor->current_speed = 0.0f;
        motor->target_speed = 0.0f;
        motor->motion_state = SIM_MOTOR_STOPPED;
        motor->busy = false;
        motor->status_register |= L6470_STATUS_HIZ;
        break;

    case L6470_CMD_RESET_POS:
        motor->current_position = 0;
        motor->target_position = 0;
        break;

    default:
        // Unknown command
        motor->status_register |= L6470_STATUS_WRONG_CMD;
        return false;
    }

    return true;
}

int32_t l6470_sim_get_position(uint8_t motor_id) {
    if (motor_id >= SIM_MAX_MOTORS ||
        !g_simulation.motors[motor_id].initialized) {
        return 0;
    }

    return g_simulation.motors[motor_id].current_position;
}

float l6470_sim_get_speed(uint8_t motor_id) {
    if (motor_id >= SIM_MAX_MOTORS ||
        !g_simulation.motors[motor_id].initialized) {
        return 0.0f;
    }

    return g_simulation.motors[motor_id].current_speed;
}

/* ==========================================================================
 */
/* AS5600 Simulation Implementation                                          */
/* ==========================================================================
 */

bool as5600_sim_init(uint8_t encoder_id) {
    if (encoder_id >= SIM_MAX_ENCODERS || !g_simulation_initialized) {
        return false;
    }

    AS5600_SimState_t *encoder = &g_simulation.encoders[encoder_id];
    encoder->initialized = true;
    encoder->pattern_start_time = g_simulation.simulation_time_ms;

    return true;
}

bool as5600_sim_read_register(uint8_t encoder_id, uint8_t reg_addr,
                              uint8_t *value) {
    if (encoder_id >= SIM_MAX_ENCODERS ||
        !g_simulation.encoders[encoder_id].initialized || value == NULL) {
        return false;
    }

    AS5600_SimState_t *encoder = &g_simulation.encoders[encoder_id];

    switch (reg_addr) {
    case AS5600_REG_ANGLE_H:
        *value = (encoder->filtered_angle >> 8) & 0x0F;
        break;

    case AS5600_REG_ANGLE_L:
        *value = encoder->filtered_angle & 0xFF;
        break;

    case AS5600_REG_RAW_ANGLE_H:
        *value = (encoder->raw_angle >> 8) & 0x0F;
        break;

    case AS5600_REG_RAW_ANGLE_L:
        *value = encoder->raw_angle & 0xFF;
        break;

    case AS5600_REG_STATUS:
        *value = 0;
        if (encoder->magnet_detected)
            *value |= 0x20; // MD bit
        if (encoder->magnet_too_weak)
            *value |= 0x10; // ML bit
        if (encoder->magnet_too_strong)
            *value |= 0x08; // MH bit
        break;

    case AS5600_REG_AGC:
        *value = encoder->agc_value;
        break;

    case AS5600_REG_MAGNITUDE_H:
        *value = (encoder->magnitude >> 8) & 0x0F;
        break;

    case AS5600_REG_MAGNITUDE_L:
        *value = encoder->magnitude & 0xFF;
        break;

    default:
        *value = encoder->registers[reg_addr];
        break;
    }

    return true;
}

void as5600_sim_set_motion_pattern(uint8_t encoder_id, MotionPattern_t pattern,
                                   float parameter) {
    if (encoder_id >= SIM_MAX_ENCODERS ||
        !g_simulation.encoders[encoder_id].initialized) {
        return;
    }

    AS5600_SimState_t *encoder = &g_simulation.encoders[encoder_id];
    encoder->pattern = pattern;
    encoder->pattern_parameter = parameter;
    encoder->pattern_start_time = g_simulation.simulation_time_ms;
}

void as5600_sim_set_position(uint8_t encoder_id, float position_deg) {
    if (encoder_id >= SIM_MAX_ENCODERS ||
        !g_simulation.encoders[encoder_id].initialized) {
        return;
    }

    AS5600_SimState_t *encoder = &g_simulation.encoders[encoder_id];
    encoder->actual_position_deg = fmodf(position_deg, 360.0f);
    if (encoder->actual_position_deg < 0.0f) {
        encoder->actual_position_deg += 360.0f;
    }

    // Convert to 12-bit representation
    uint16_t angle_12bit =
        (uint16_t)((encoder->actual_position_deg / 360.0f) * 4096.0f);
    encoder->raw_angle = angle_12bit;
    encoder->filtered_angle =
        as5600_sim_add_noise(angle_12bit, encoder->noise_amplitude);
}

/* ==========================================================================
 */
/* Private Function Implementations                                          */
/* ==========================================================================
 */

static void l6470_sim_update_motion(uint8_t motor_id, float delta_time_s) {
    L6470_SimState_t *motor = &g_simulation.motors[motor_id];

    switch (motor->motion_state) {
    case SIM_MOTOR_ACCELERATING:
        if (motor->current_speed < motor->target_speed) {
            motor->current_speed += motor->acceleration * delta_time_s;
            if (motor->current_speed >= motor->target_speed) {
                motor->current_speed = motor->target_speed;
                motor->motion_state = SIM_MOTOR_CONSTANT_SPEED;
            }
        }
        break;

    case SIM_MOTOR_DECELERATING:
        if (motor->current_speed > 0.0f) {
            motor->current_speed -= motor->deceleration * delta_time_s;
            if (motor->current_speed <= 0.0f) {
                motor->current_speed = 0.0f;
                motor->motion_state = SIM_MOTOR_STOPPED;
                motor->busy = false;
            }
        }
        break;

    case SIM_MOTOR_CONSTANT_SPEED:
        // Check if we need to start decelerating to reach target position
        if (motor->target_position != motor->current_position) {
            float distance_remaining = fabsf(
                (float)(motor->target_position - motor->current_position));
            float decel_distance =
                (motor->current_speed * motor->current_speed) /
                (2.0f * motor->deceleration);

            if (distance_remaining <= decel_distance) {
                motor->motion_state = SIM_MOTOR_DECELERATING;
            }
        }
        break;

    case SIM_MOTOR_STOPPED:
        motor->current_speed = 0.0f;
        motor->busy = false;
        break;
    }

    // Update position based on speed
    float position_delta = motor->current_speed * delta_time_s;
    if (motor->direction) {
        motor->current_position += (int32_t)position_delta;
    } else {
        motor->current_position -= (int32_t)position_delta;
    }
}

static void as5600_sim_update_position(uint8_t encoder_id,
                                       float delta_time_s) {
    AS5600_SimState_t *encoder = &g_simulation.encoders[encoder_id];
    float elapsed_time_s =
        (g_simulation.simulation_time_ms - encoder->pattern_start_time) /
        1000.0f;

    switch (encoder->pattern) {
    case SIM_PATTERN_LINEAR:
        // Linear motion at constant velocity (degrees/second)
        encoder->actual_position_deg +=
            encoder->pattern_parameter * delta_time_s;
        break;

    case SIM_PATTERN_SINUSOIDAL:
        // Sinusoidal motion: amplitude * sin(2*pi*frequency*time)
        encoder->actual_position_deg =
            encoder->pattern_parameter *
            sinf(2.0f * M_PI * 0.1f * elapsed_time_s);
        break;

    case SIM_PATTERN_STATIC:
    default:
        // No motion
        break;
    }

    // Normalize to 0-360 degrees
    encoder->actual_position_deg = fmodf(encoder->actual_position_deg, 360.0f);
    if (encoder->actual_position_deg < 0.0f) {
        encoder->actual_position_deg += 360.0f;
    }

    // Convert to 12-bit representation
    uint16_t angle_12bit =
        (uint16_t)((encoder->actual_position_deg / 360.0f) * 4096.0f);
    encoder->raw_angle = angle_12bit;
    encoder->filtered_angle =
        as5600_sim_add_noise(angle_12bit, encoder->noise_amplitude);
}

static uint16_t as5600_sim_add_noise(uint16_t angle, float noise_amplitude) {
    if (noise_amplitude <= 0.0f) {
        return angle;
    }

    // Simple random noise generation
    float noise = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * noise_amplitude;
    float angle_deg = (float)angle * 360.0f / 4096.0f;
    angle_deg += noise;

    // Normalize and convert back
    angle_deg = fmodf(angle_deg, 360.0f);
    if (angle_deg < 0.0f)
        angle_deg += 360.0f;

    return (uint16_t)((angle_deg / 360.0f) * 4096.0f);
}

static void l6470_sim_update_status_register(uint8_t motor_id) {
    L6470_SimState_t *motor = &g_simulation.motors[motor_id];

    // Update motion status bits
    motor->status_register &= ~L6470_STATUS_MOT_STATUS_Msk;
    switch (motor->motion_state) {
    case SIM_MOTOR_STOPPED:
        motor->status_register |= (0 << 5);
        break;
    case SIM_MOTOR_ACCELERATING:
        motor->status_register |= (1 << 5);
        break;
    case SIM_MOTOR_DECELERATING:
        motor->status_register |= (2 << 5);
        break;
    case SIM_MOTOR_CONSTANT_SPEED:
        motor->status_register |= (3 << 5);
        break;
    }

    // Update direction bit
    if (motor->direction) {
        motor->status_register |= L6470_STATUS_DIR;
    } else {
        motor->status_register &= ~L6470_STATUS_DIR;
    }

    // Update busy bit
    if (motor->busy) {
        motor->status_register |= L6470_STATUS_BUSY;
    } else {
        motor->status_register &= ~L6470_STATUS_BUSY;
    }

    // Simulate fault conditions if enabled
    if (motor->active_fault != SIM_FAULT_NONE) {
        switch (motor->active_fault) {
        case SIM_FAULT_OVERCURRENT:
            motor->status_register |= L6470_STATUS_OCD;
            break;
        case SIM_FAULT_OVERHEAT:
            motor->status_register |= L6470_STATUS_TH_SD;
            break;
        case SIM_FAULT_UNDERVOLTAGE:
            motor->status_register |= L6470_STATUS_UVLO;
            break;
        case SIM_FAULT_STALL:
            motor->status_register |= L6470_STATUS_STEP_LOSS_A;
            break;
        default:
            break;
        }
    }
}

static bool l6470_sim_validate_register_write(uint8_t reg_addr,
                                              uint32_t value) {
    // Use generated validation function
    return l6470_validate_register_value(reg_addr, value);
}
