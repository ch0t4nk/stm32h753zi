/**
 * @file motor_simulation.h
 * @brief Motor and Encoder Simulation Framework
 *
 * Provides safe simulation of L6470 stepper drivers and AS5600 encoders
 * for testing control algorithms without hardware risk.
 *
 * Features:
 * - L6470 register simulation with realistic responses
 * - AS5600 position simulation with configurable patterns
 * - Fault injection for safety system testing
 * - Motion profile validation
 *
 * @note Enable with SIMULATION_MODE in build configuration
 */

#ifndef MOTOR_SIMULATION_H
#define MOTOR_SIMULATION_H

#include "as5600_registers_generated.h"
#include "l6470_registers_generated.h"
#include "system_config.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
 */
/* Simulation Configuration */
/* ==========================================================================
 */

#define SIM_MAX_MOTORS 2
#define SIM_MAX_ENCODERS 2
#define SIM_POSITION_RESOLUTION 4096 // 12-bit resolution for AS5600
#define SIM_TICK_RATE_HZ 1000        // Simulation update rate

/* Simulation modes */
typedef enum {
    SIM_MODE_DISABLED = 0,
    SIM_MODE_BASIC,          // Basic register simulation
    SIM_MODE_REALISTIC,      // Realistic motion physics
    SIM_MODE_FAULT_INJECTION // Include fault scenarios
} SimulationMode_t;

/* Motion patterns for encoder simulation */
typedef enum {
    SIM_PATTERN_STATIC = 0,
    SIM_PATTERN_LINEAR,     // Constant velocity
    SIM_PATTERN_SINUSOIDAL, // Oscillating motion
    SIM_PATTERN_STEP,       // Discrete position changes
    SIM_PATTERN_NOISY       // Add realistic noise
} MotionPattern_t;

/* Fault types for safety testing */
typedef enum {
    SIM_FAULT_NONE = 0,
    SIM_FAULT_OVERCURRENT,
    SIM_FAULT_OVERHEAT,
    SIM_FAULT_UNDERVOLTAGE,
    SIM_FAULT_STALL,
    SIM_FAULT_MAGNET_WEAK,
    SIM_FAULT_MAGNET_STRONG,
    SIM_FAULT_COMMUNICATION
} SimulationFault_t;

/* ==========================================================================
 */
/* L6470 Simulation State                                                    */
/* ==========================================================================
 */

typedef struct {
    uint8_t motor_id;
    bool initialized;
    bool enabled;

    // Register shadow (simulated hardware state)
    uint32_t registers[256]; // All possible register addresses

    // Motion state
    int32_t current_position; // Steps
    int32_t target_position;  // Steps
    float current_speed;      // Steps/second
    float target_speed;       // Steps/second
    float acceleration;       // Steps/second²
    float deceleration;       // Steps/second²

    // Motion profile state
    enum {
        SIM_MOTOR_STOPPED = 0,
        SIM_MOTOR_ACCELERATING,
        SIM_MOTOR_CONSTANT_SPEED,
        SIM_MOTOR_DECELERATING
    } motion_state;

    // Status flags
    uint16_t status_register;
    bool busy;
    bool direction; // true = forward, false = reverse

    // Fault simulation
    SimulationFault_t active_fault;
    uint32_t fault_injection_time;

    // Performance tracking
    uint32_t command_count;
    uint32_t last_update_time;

} L6470_SimState_t;

/* ==========================================================================
 */
/* AS5600 Simulation State                                                   */
/* ==========================================================================
 */

typedef struct {
    uint8_t encoder_id;
    bool initialized;
    bool magnet_detected;

    // Register shadow
    uint8_t registers[256];

    // Position state
    uint16_t raw_angle;        // 0-4095 (12-bit)
    uint16_t filtered_angle;   // 0-4095 (12-bit)
    float actual_position_deg; // Actual position in degrees

    // Motion simulation
    MotionPattern_t pattern;
    float pattern_parameter; // Pattern-specific parameter
    uint32_t pattern_start_time;

    // Magnet simulation
    uint16_t magnitude; // Simulated magnetic field strength
    uint8_t agc_value;  // Automatic gain control

    // Status simulation
    bool magnet_too_strong;
    bool magnet_too_weak;

    // Fault simulation
    SimulationFault_t active_fault;

    // Noise parameters
    float noise_amplitude; // Degrees
    bool noise_enabled;

} AS5600_SimState_t;

/* ==========================================================================
 */
/* Global Simulation State                                                   */
/* ==========================================================================
 */

typedef struct {
    SimulationMode_t mode;
    bool initialized;
    uint32_t simulation_time_ms;
    uint32_t tick_counter;

    // Motor simulation states
    L6470_SimState_t motors[SIM_MAX_MOTORS];

    // Encoder simulation states
    AS5600_SimState_t encoders[SIM_MAX_ENCODERS];

    // Global fault injection
    bool fault_injection_enabled;
    float fault_probability; // Per second

    // Performance metrics
    uint32_t total_commands;
    uint32_t total_updates;

} MotorSimulation_t;

/* ==========================================================================
 */
/* L6470 Simulation Functions                                                */
/* ==========================================================================
 */

/**
 * @brief Initialize L6470 simulation
 * @param motor_id Motor identifier (0-1)
 * @return true if successful
 */
bool l6470_sim_init(uint8_t motor_id);

/**
 * @brief Simulate L6470 register write
 * @param motor_id Motor identifier
 * @param reg_addr Register address
 * @param value Value to write
 * @return true if successful
 */
bool l6470_sim_write_register(uint8_t motor_id, uint8_t reg_addr,
                              uint32_t value);

/**
 * @brief Simulate L6470 register read
 * @param motor_id Motor identifier
 * @param reg_addr Register address
 * @param value Pointer to store read value
 * @return true if successful
 */
bool l6470_sim_read_register(uint8_t motor_id, uint8_t reg_addr,
                             uint32_t *value);

/**
 * @brief Simulate L6470 command execution
 * @param motor_id Motor identifier
 * @param command Command opcode
 * @param parameter Command parameter
 * @return true if successful
 */
bool l6470_sim_execute_command(uint8_t motor_id, uint8_t command,
                               uint32_t parameter);

/**
 * @brief Get simulated motor position
 * @param motor_id Motor identifier
 * @return Current position in steps
 */
int32_t l6470_sim_get_position(uint8_t motor_id);

/**
 * @brief Get simulated motor speed
 * @param motor_id Motor identifier
 * @return Current speed in steps/second
 */
float l6470_sim_get_speed(uint8_t motor_id);

/**
 * @brief Inject fault into motor simulation
 * @param motor_id Motor identifier
 * @param fault_type Type of fault to inject
 * @param duration_ms Duration of fault in milliseconds
 */
void l6470_sim_inject_fault(uint8_t motor_id, SimulationFault_t fault_type,
                            uint32_t duration_ms);

/* ==========================================================================
 */
/* AS5600 Simulation Functions                                               */
/* ==========================================================================
 */

/**
 * @brief Initialize AS5600 simulation
 * @param encoder_id Encoder identifier (0-1)
 * @return true if successful
 */
bool as5600_sim_init(uint8_t encoder_id);

/**
 * @brief Simulate AS5600 register read
 * @param encoder_id Encoder identifier
 * @param reg_addr Register address
 * @param value Pointer to store read value
 * @return true if successful
 */
bool as5600_sim_read_register(uint8_t encoder_id, uint8_t reg_addr,
                              uint8_t *value);

/**
 * @brief Set encoder motion pattern
 * @param encoder_id Encoder identifier
 * @param pattern Motion pattern type
 * @param parameter Pattern-specific parameter
 */
void as5600_sim_set_motion_pattern(uint8_t encoder_id, MotionPattern_t pattern,
                                   float parameter);

/**
 * @brief Set encoder position directly (for testing)
 * @param encoder_id Encoder identifier
 * @param position_deg Position in degrees (0-360)
 */
void as5600_sim_set_position(uint8_t encoder_id, float position_deg);

/**
 * @brief Enable position noise simulation
 * @param encoder_id Encoder identifier
 * @param amplitude_deg Noise amplitude in degrees
 */
void as5600_sim_enable_noise(uint8_t encoder_id, float amplitude_deg);

/**
 * @brief Inject fault into encoder simulation
 * @param encoder_id Encoder identifier
 * @param fault_type Type of fault to inject
 * @param duration_ms Duration of fault in milliseconds
 */
void as5600_sim_inject_fault(uint8_t encoder_id, SimulationFault_t fault_type,
                             uint32_t duration_ms);

/* ==========================================================================
 */
/* Global Simulation Control                                                 */
/* ==========================================================================
 */

/**
 * @brief Initialize motor simulation system
 * @param mode Simulation mode
 * @return true if successful
 */
bool motor_simulation_init(SimulationMode_t mode);

/**
 * @brief Update simulation state (call periodically)
 * @param delta_time_ms Time elapsed since last update
 */
void motor_simulation_update(uint32_t delta_time_ms);

/**
 * @brief Enable global fault injection
 * @param probability Fault probability per second (0.0-1.0)
 */
void motor_simulation_enable_fault_injection(float probability);

/**
 * @brief Get simulation performance metrics
 * @param total_commands Pointer to store total commands processed
 * @param total_updates Pointer to store total updates performed
 * @param uptime_ms Pointer to store simulation uptime
 */
void motor_simulation_get_metrics(uint32_t *total_commands,
                                  uint32_t *total_updates,
                                  uint32_t *uptime_ms);

/**
 * @brief Reset simulation to initial state
 */
void motor_simulation_reset(void);

/**
 * @brief Check if simulation mode is active
 * @return true if simulation is enabled
 */
bool motor_simulation_is_active(void);

/* ==========================================================================
 */
/* Test Pattern Functions                                                    */
/* ==========================================================================
 */

/**
 * @brief Run motion profile test
 * @param motor_id Motor identifier
 * @param target_position Target position in steps
 * @param max_speed Maximum speed in steps/second
 * @return true if test started successfully
 */
bool motor_simulation_test_motion_profile(uint8_t motor_id,
                                          int32_t target_position,
                                          float max_speed);

/**
 * @brief Run encoder tracking test
 * @param encoder_id Encoder identifier
 * @param test_duration_ms Test duration in milliseconds
 * @return true if test started successfully
 */
bool motor_simulation_test_encoder_tracking(uint8_t encoder_id,
                                            uint32_t test_duration_ms);

/**
 * @brief Run closed-loop control test
 * @param motor_id Motor identifier
 * @param encoder_id Encoder identifier
 * @param target_angle Target angle in degrees
 * @return true if test started successfully
 */
bool motor_simulation_test_closed_loop(uint8_t motor_id, uint8_t encoder_id,
                                       float target_angle);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_SIMULATION_H */
