/**
 * @file multi_motor_coordinator.h
 * @brief Multi-Motor Coordination and Synchronization System - Header
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @details Coordinates multiple stepper motors for synchronized motion,
 *          load balancing, and complex multi-axis movements
 */

#ifndef MULTI_MOTOR_COORDINATOR_H
#define MULTI_MOTOR_COORDINATOR_H

#include "common/data_types.h"
#include "common/error_codes.h"
#include "config/motor_config.h"
#include <stdbool.h>
#include <stdint.h>

// Multi-motor coordination constants
#define SYNC_TOLERANCE_DEFAULT 50  // Default sync tolerance in steps
#define MAX_SYNC_WAIT_MS 5000      // Maximum wait time for synchronization
#define LOAD_BALANCE_THRESHOLD 100 // Load balance activation threshold
#define MAX_LOAD_DIFFERENCE 200    // Maximum allowed load difference

// Error codes specific to multi-motor coordination
#define ERROR_MOTION_ACTIVE 0x5100         // Motion already active
#define ERROR_NO_MOTORS_ENABLED 0x5101     // No motors enabled for operation
#define ERROR_SYNC_TIMEOUT 0x5102          // Synchronization timeout
#define ERROR_LOAD_IMBALANCE 0x5103        // Load imbalance detected
#define ERROR_VELOCITY_OUT_OF_RANGE 0x5104 // Velocity exceeds limits

/**
 * @brief Coordination modes enumeration
 */
typedef enum {
    COORDINATION_MODE_INDEPENDENT = 0, ///< Motors operate independently
    COORDINATION_MODE_SYNCHRONIZED,    ///< Motors move in synchronized fashion
    COORDINATION_MODE_LOAD_SHARING,    ///< Motors share load equally
    COORDINATION_MODE_MASTER_SLAVE     ///< One master, others follow
} CoordinationMode_t;

/**
 * @brief Motor priority levels
 */
typedef enum {
    MOTOR_PRIORITY_LOW = 0, ///< Low priority motor
    MOTOR_PRIORITY_NORMAL,  ///< Normal priority motor
    MOTOR_PRIORITY_HIGH     ///< High priority motor
} MotorPriority_t;

/**
 * @brief Individual motor target structure
 */
typedef struct {
    bool enabled;            ///< Motor enabled for this move
    int32_t target_position; ///< Target position in steps
    uint32_t max_velocity;   ///< Maximum velocity for this move
    uint32_t acceleration;   ///< Acceleration for this move
    uint32_t deceleration;   ///< Deceleration for this move
} MotorTarget_t;

/**
 * @brief Coordinated move command structure
 */
typedef struct {
    MotorTarget_t motor_targets[MAX_MOTORS]; ///< Individual motor targets
    uint32_t total_move_time_ms; ///< Total time for coordinated move
    bool wait_for_completion;    ///< Wait for all motors to complete
    uint8_t sequence_id;         ///< Sequence identifier
} CoordinatedMoveCommand_t;

/**
 * @brief Motor state tracking structure
 */
typedef struct {
    uint8_t motor_id;         ///< Motor identifier
    bool enabled;             ///< Motor enabled in coordination
    bool synchronized;        ///< Motor participating in sync
    bool load_share_enabled;  ///< Motor participating in load sharing
    MotorPriority_t priority; ///< Motor priority level
    int32_t current_position; ///< Current motor position
    bool settled;             ///< Motor position settled
    bool profile_active;      ///< Motion profile active
    int32_t slave_offset;     ///< Offset from master (master-slave mode)
} CoordinatorMotorState_t;    // Renamed to avoid conflict with motor_config.h

/**
 * @brief Synchronization configuration structure
 */
typedef struct {
    uint8_t sync_master;         ///< Master motor for synchronization
    int32_t sync_tolerance;      ///< Synchronization tolerance in steps
    uint32_t max_sync_wait_ms;   ///< Maximum wait time for sync
    bool enable_sync_correction; ///< Enable automatic sync correction
} SynchronizationConfig_t;

/**
 * @brief Load sharing configuration structure
 */
typedef struct {
    bool load_balance_enabled;       ///< Load balancing enabled
    int32_t load_threshold;          ///< Threshold to activate load balancing
    int32_t max_load_difference;     ///< Maximum allowed load difference
    uint32_t balance_update_rate_ms; ///< Load balance update rate
} LoadSharingConfig_t;

/**
 * @brief Coordinated motion state structure
 */
typedef struct {
    CoordinationMode_t mode; ///< Current coordination mode
    bool active;             ///< Coordinated motion active
    uint32_t start_time;     ///< Motion start timestamp
    uint8_t sequence_step;   ///< Current sequence step
} CoordinatedMotionState_t;

/**
 * @brief Multi-motor coordinator main structure
 */
typedef struct {
    CoordinatorMotorState_t
        motor_states[MAX_MOTORS];          ///< Individual motor states
    SynchronizationConfig_t sync_config;   ///< Synchronization configuration
    LoadSharingConfig_t load_config;       ///< Load sharing configuration
    CoordinatedMotionState_t motion_state; ///< Motion state
    CoordinatedMoveCommand_t current_move; ///< Current move command
} MultiMotorCoordinator_t;

/**
 * @brief Coordination status structure
 */
typedef struct {
    CoordinationMode_t mode;             ///< Current coordination mode
    bool active;                         ///< Coordinated motion active
    uint8_t sync_master;                 ///< Current sync master
    bool load_balance_enabled;           ///< Load balancing status
    bool motor_enabled[MAX_MOTORS];      ///< Motor enabled status
    bool motor_synchronized[MAX_MOTORS]; ///< Motor sync status
    bool motor_load_share[MAX_MOTORS];   ///< Motor load share status
    int32_t motor_positions[MAX_MOTORS]; ///< Current motor positions
    bool motor_settled[MAX_MOTORS];      ///< Motor settled status
    int32_t max_sync_error;              ///< Maximum synchronization error
} CoordinationStatus_t;

// Core coordination functions
SystemError_t multi_motor_init(void);
SystemError_t multi_motor_enable_motor(uint8_t motor_id, bool enable);
SystemError_t multi_motor_set_mode(CoordinationMode_t mode);

// Motion control functions
SystemError_t multi_motor_coordinated_move(CoordinatedMoveCommand_t *move_cmd);
SystemError_t multi_motor_update(uint32_t dt_ms);
SystemError_t multi_motor_stop_coordinated_motion(void);

// Status and monitoring
SystemError_t multi_motor_get_status(CoordinationStatus_t *status);

// Configuration functions
SystemError_t multi_motor_configure_sync(SynchronizationConfig_t *config);
SystemError_t multi_motor_configure_load_sharing(LoadSharingConfig_t *config);

// Utility functions
bool multi_motor_is_motion_active(void);
CoordinationMode_t multi_motor_get_current_mode(void);
SystemError_t multi_motor_wait_for_completion(uint32_t timeout_ms);

// Advanced coordination functions
SystemError_t multi_motor_execute_sequence(CoordinatedMoveCommand_t *sequence,
                                           uint8_t sequence_length);
SystemError_t multi_motor_emergency_stop(void);
SystemError_t multi_motor_home_all_motors(void);

// Internal functions (declared for testing)
// Internal functions are implemented in multi_motor_coordinator.c
// (Static functions should not be declared in headers)

#endif // MULTI_MOTOR_COORDINATOR_H
