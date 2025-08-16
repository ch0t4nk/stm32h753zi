/**
 * @file hardware_presence_config.h
 * @brief Hardware Presence Detection Configuration for STM32H753ZI Development
 * @author STM32H753ZI Project Team
 * @date August 2025
 * @version 1.0
 *
 * This file defines which hardware components are actually connected for
 * conditional initialization and testing. This allows the same firmware
 * to work with partial hardware setups during development.
 */

#ifndef HARDWARE_PRESENCE_CONFIG_H
#define HARDWARE_PRESENCE_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Hardware Presence Flags (SSOT)
// =============================================================================

/** @brief Motor driver hardware presence flags */
#define MOTOR1_L6470_PRESENT (0) // Set to 1 when L6470 #1 is connected
#define MOTOR2_L6470_PRESENT (0) // Set to 1 when L6470 #2 is connected
#define IHM02A1_SHIELD_PRESENT                                                \
    (0) // Set to 1 when X-NUCLEO-IHM02A1 shield is connected

/** @brief Encoder hardware presence flags */
#define ENCODER1_AS5600_PRESENT (0) // Set to 1 when AS5600 #1 is connected
#define ENCODER2_AS5600_PRESENT (0) // Set to 1 when AS5600 #2 is connected

/** @brief Safety hardware presence flags */
#define EMERGENCY_STOP_BUTTON_PRESENT                                         \
    (0)                          // Set to 1 when E-stop button is connected
#define SAFETY_RELAY_PRESENT (0) // Set to 1 when safety relays are connected
#define WATCHDOG_EXTERNAL_PRESENT                                             \
    (0) // Set to 1 when external watchdog is connected

/** @brief Communication hardware presence flags */
#define CAN_TRANSCEIVER_PRESENT                                               \
    (1) // CAN transceiver typically present on Nucleo
#define EXTERNAL_UART_PRESENT                                                 \
    (0) // Set to 1 when external UART devices connected

/** @brief Power monitoring hardware presence flags */
#define CURRENT_SENSE_PRESENT                                                 \
    (0) // Set to 1 when current sensors are connected
#define VOLTAGE_MONITOR_PRESENT                                               \
    (0) // Set to 1 when voltage monitoring is connected

/** @brief Development mode flags */
#define DEVELOPMENT_MODE_ENABLED (1)   // Enable development-specific features
#define SIMULATION_MODE_ENABLED (0)    // Enable pure simulation mode
#define HARDWARE_TEST_MODE_ENABLED (1) // Enable hardware-in-the-loop testing

// =============================================================================
// Conditional Hardware Groups
// =============================================================================

/** @brief Motor control system presence (requires both motors and encoders) */
#define MOTOR_CONTROL_SYSTEM_PRESENT                                          \
    (MOTOR1_L6470_PRESENT && MOTOR2_L6470_PRESENT &&                          \
     ENCODER1_AS5600_PRESENT && ENCODER2_AS5600_PRESENT)

/** @brief Basic safety system presence (minimal safety features) */
#define BASIC_SAFETY_SYSTEM_PRESENT                                           \
    (EMERGENCY_STOP_BUTTON_PRESENT || DEVELOPMENT_MODE_ENABLED)

/** @brief Full safety system presence (all safety features) */
#define FULL_SAFETY_SYSTEM_PRESENT                                            \
    (EMERGENCY_STOP_BUTTON_PRESENT && SAFETY_RELAY_PRESENT)

/** @brief Communication system presence */
#define COMMUNICATION_SYSTEM_PRESENT                                          \
    (CAN_TRANSCEIVER_PRESENT || EXTERNAL_UART_PRESENT)

// =============================================================================
// Hardware Detection Functions
// =============================================================================

/**
 * @brief Runtime hardware detection results
 */
typedef struct {
    bool motor1_detected;
    bool motor2_detected;
    bool encoder1_detected;
    bool encoder2_detected;
    bool emergency_stop_detected;
    bool can_transceiver_detected;
    bool safety_relay_detected;
} HardwarePresence_t;

/**
 * @brief Perform runtime hardware detection
 * @return HardwarePresence_t Structure with detected hardware
 */
HardwarePresence_t detect_hardware_presence(void);

/**
 * @brief Check if motor control is possible with current hardware
 * @param presence Hardware presence detection results
 * @return bool True if motor control is possible
 */
bool is_motor_control_possible(const HardwarePresence_t *presence);

/**
 * @brief Check if safety system can operate with current hardware
 * @param presence Hardware presence detection results
 * @return bool True if basic safety system can operate
 */
bool is_safety_system_possible(const HardwarePresence_t *presence);

// =============================================================================
// Development Mode Overrides
// =============================================================================

#if DEVELOPMENT_MODE_ENABLED

/** @brief Development mode allows bypassing hardware requirements */
#define DEV_BYPASS_MOTOR_HARDWARE (1)   // Allow motor control without hardware
#define DEV_BYPASS_SAFETY_HARDWARE (1)  // Allow safety system without hardware
#define DEV_BYPASS_ENCODER_HARDWARE (1) // Allow encoder simulation
#define DEV_MOCK_HARDWARE_RESPONSES (1) // Mock hardware responses

/** @brief Development logging levels */
#define DEV_LOG_HARDWARE_DETECTION (1) // Log hardware detection results
#define DEV_LOG_BYPASS_DECISIONS (1)   // Log bypass decisions
#define DEV_LOG_MOCK_RESPONSES (1)     // Log mock hardware responses

#else

// Production mode - strict hardware requirements
#define DEV_BYPASS_MOTOR_HARDWARE (0)
#define DEV_BYPASS_SAFETY_HARDWARE (0)
#define DEV_BYPASS_ENCODER_HARDWARE (0)
#define DEV_MOCK_HARDWARE_RESPONSES (0)
#define DEV_LOG_HARDWARE_DETECTION (0)
#define DEV_LOG_BYPASS_DECISIONS (0)
#define DEV_LOG_MOCK_RESPONSES (0)

#endif // DEVELOPMENT_MODE_ENABLED

#ifdef __cplusplus
}
#endif

#endif // HARDWARE_PRESENCE_CONFIG_H
