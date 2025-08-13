/**
 * @file safety_state_manager.h
 * @br/**
 * @brief Safety event definitions moved to data_types.h (SSOT)
 */

/**
 * @brief Safety event callback function type
 */
typedef SystemError_t (*SafetyEventCallback_t)(SafetyEventType_t event,
                                               void *event_data);
fety State Management System *@details Coordinates all safety systems with
        event -
    driven architecture **Addresses TODOs : *-src / safety /
                                            safety_system.c : 676 -
    Integrate with system state manager * -src / safety /
        emergency_stop.c : 139,
    202 - Integration with system state manager * -src / safety /
              emergency_stop.c : 400,
    404,
    407 -
        Integrate with monitoring systems **@note Generated from Strategic
            Enhancement Implementation *@date 2025 -
        08 -
        07 * /

#ifndef SAFETY_STATE_MANAGER_H
#define SAFETY_STATE_MANAGER_H

#include "common/common_types.h"
#include "common/data_types.h"
#include "config/safety_config.h"

#ifdef __cplusplus
            extern "C" {
#endif

  /* ==========================================================================
   * Safety State Manager Types and Definitions
   * ==========================================================================
   */

  /**
   * @brief System safety states
   */
  typedef enum {
    SAFETY_STATE_INIT = 0,       ///< Initialization state
    SAFETY_STATE_NORMAL,         ///< Normal operation
    SAFETY_STATE_WARNING,        ///< Warning conditions detected
    SAFETY_STATE_FAULT,          ///< Fault conditions detected
    SAFETY_STATE_EMERGENCY_STOP, ///< Emergency stop activated
    SAFETY_STATE_SHUTDOWN,       ///< System shutdown
    SAFETY_STATE_RECOVERY,       ///< Recovery from fault
    SAFETY_STATE_COUNT           ///< Number of states
  } SafetyState_t;

  /**
   * @brief Safety event types for event-driven architecture
   */
  typedef enum {
    SAFETY_EVENT_INIT_COMPLETE = 0,
    SAFETY_EVENT_FAULT_DETECTED,
    SAFETY_EVENT_EMERGENCY_STOP,
    SAFETY_EVENT_FAULT_CLEARED,
    SAFETY_EVENT_RECOVERY_COMPLETE,
    SAFETY_EVENT_SHUTDOWN_REQUEST,
    SAFETY_EVENT_COUNT
  } SafetyEvent_t;

  /**
   * @brief Safety event callback function type
   */
  typedef SystemError_t (*SafetyEventCallback_t)(SafetyEvent_t event,
                                                 void *event_data);

  /**
   * @brief Safety state transition structure
   */
  typedef struct {
    SafetyState_t current_state;     ///< Current state
    SafetyEventType_t trigger_event; ///< Event that triggers transition
    SafetyState_t next_state;        ///< Next state after transition
    SafetyEventCallback_t callback;  ///< Optional callback for transition
  } SafetyStateTransition_t;

  /**
   * @brief Safety monitoring status
   */
  typedef struct {
    bool emergency_stop_active;     ///< Emergency stop status
    bool fault_monitor_active;      ///< Fault monitoring status
    bool communication_healthy;     ///< Communication system health
    bool motor_controllers_healthy; ///< Motor controller health
    uint32_t fault_count;           ///< Total fault count
    timestamp_ms_t last_update_ms;  ///< Last monitoring update
  } SafetyMonitoringStatus_t;

  /* ==========================================================================
   * Safety State Manager API
   * ==========================================================================
   */

  /**
   * @brief Initialize safety state manager
   * @return SystemError_t SYSTEM_OK on success
   */
  SystemError_t safety_state_manager_init(void);

  /**
   * @brief Process safety event
   * @param event Safety event to process
   * @param event_data Optional event data
   * @return SystemError_t SYSTEM_OK on success
   */
  SystemError_t safety_state_manager_process_event(SafetyEventType_t event,
                                                   void *event_data);

  /**
   * @brief Get current safety state
   * @return SafetyState_t Current safety state
   */
  SafetyState_t safety_state_manager_get_state(void);

  /**
   * @brief Register event callback
   * @param event Event type to register for
   * @param callback Callback function
   * @return SystemError_t SYSTEM_OK on success
   */
  SystemError_t safety_state_manager_register_callback(
      SafetyEventType_t event, SafetyEventCallback_t callback);

  /**
   * @brief Update monitoring status
   * @param status Current monitoring status
   * @return SystemError_t SYSTEM_OK on success
   */
  SystemError_t safety_state_manager_update_monitoring(
      const SafetyMonitoringStatus_t *status);

  /**
   * @brief Get monitoring status
   * @param status Pointer to status structure to fill
   * @return SystemError_t SYSTEM_OK on success
   */
  SystemError_t safety_state_manager_get_monitoring_status(
      SafetyMonitoringStatus_t * status);

  /**
   * @brief Force emergency stop through state manager
   * @return SystemError_t SYSTEM_OK on success
   */
  SystemError_t safety_state_manager_force_emergency_stop(void);

  /**
   * @brief Check if emergency stop is active
   * @return bool true if emergency stop active
   */
  bool safety_state_manager_is_emergency_stop_active(void);

  /**
   * @brief Process periodic safety monitoring
   * Called from main control loop at 1kHz
   * @return SystemError_t SYSTEM_OK on success
   */
  SystemError_t safety_state_manager_periodic_update(void);

#ifdef __cplusplus
}
#endif

#endif /* SAFETY_STATE_MANAGER_H */
