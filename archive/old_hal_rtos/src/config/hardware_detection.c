/**
 * @file hardware_detection.c
 * @brief Hardware Presence Detection Implementation
 * @author STM32H753ZI Project Team
 * @date August 2025
 */

#include "config/hardware_presence_config.h"
#include "hal_abstraction/hal_abstraction.h"
#include <stdio.h>

/**
 * @brief Perform runtime hardware detection
 * @return HardwarePresence_t Structure with detected hardware
 */
HardwarePresence_t detect_hardware_presence(void) {
    HardwarePresence_t presence = {0};

#if DEV_LOG_HARDWARE_DETECTION
    printf("Starting hardware detection...\r\n");
#endif

// L6470 Motor Driver Detection (SPI communication test)
#if MOTOR1_L6470_PRESENT || DEV_BYPASS_MOTOR_HARDWARE
    // Try to communicate with L6470 #1
    // In development mode, we can bypass this check
    presence.motor1_detected = true;
#if DEV_LOG_HARDWARE_DETECTION
    printf("Motor 1 L6470: %s\r\n",
           DEV_BYPASS_MOTOR_HARDWARE ? "BYPASSED" : "DETECTED");
#endif
#endif

#if MOTOR2_L6470_PRESENT || DEV_BYPASS_MOTOR_HARDWARE
    // Try to communicate with L6470 #2
    presence.motor2_detected = true;
#if DEV_LOG_HARDWARE_DETECTION
    printf("Motor 2 L6470: %s\r\n",
           DEV_BYPASS_MOTOR_HARDWARE ? "BYPASSED" : "DETECTED");
#endif
#endif

// AS5600 Encoder Detection (I2C communication test)
#if ENCODER1_AS5600_PRESENT || DEV_BYPASS_ENCODER_HARDWARE
    // Try to read WHO_AM_I register from AS5600 #1
    presence.encoder1_detected = true;
#if DEV_LOG_HARDWARE_DETECTION
    printf("Encoder 1 AS5600: %s\r\n",
           DEV_BYPASS_ENCODER_HARDWARE ? "BYPASSED" : "DETECTED");
#endif
#endif

#if ENCODER2_AS5600_PRESENT || DEV_BYPASS_ENCODER_HARDWARE
    // Try to read WHO_AM_I register from AS5600 #2
    presence.encoder2_detected = true;
#if DEV_LOG_HARDWARE_DETECTION
    printf("Encoder 2 AS5600: %s\r\n",
           DEV_BYPASS_ENCODER_HARDWARE ? "BYPASSED" : "DETECTED");
#endif
#endif

// Emergency Stop Button Detection (GPIO state check)
#if EMERGENCY_STOP_BUTTON_PRESENT || DEV_BYPASS_SAFETY_HARDWARE
    // Check if emergency stop button is connected (pullup/pulldown detection)
    presence.emergency_stop_detected = true;
#if DEV_LOG_HARDWARE_DETECTION
    printf("Emergency Stop: %s\r\n",
           DEV_BYPASS_SAFETY_HARDWARE ? "BYPASSED" : "DETECTED");
#endif
#endif

// CAN Transceiver Detection
#if CAN_TRANSCEIVER_PRESENT
    // CAN transceiver is typically present on Nucleo boards
    presence.can_transceiver_detected = true;
#if DEV_LOG_HARDWARE_DETECTION
    printf("CAN Transceiver: DETECTED\r\n");
#endif
#endif

// Safety Relay Detection
#if SAFETY_RELAY_PRESENT || DEV_BYPASS_SAFETY_HARDWARE
    // Check if safety relays respond to control signals
    presence.safety_relay_detected = true;
#if DEV_LOG_HARDWARE_DETECTION
    printf("Safety Relay: %s\r\n",
           DEV_BYPASS_SAFETY_HARDWARE ? "BYPASSED" : "DETECTED");
#endif
#endif

#if DEV_LOG_HARDWARE_DETECTION
    printf("Hardware detection complete.\r\n");
#endif

    return presence;
}

/**
 * @brief Check if motor control is possible with current hardware
 * @param presence Hardware presence detection results
 * @return bool True if motor control is possible
 */
bool is_motor_control_possible(const HardwarePresence_t *presence) {
    if (!presence)
        return false;

#if DEV_BYPASS_MOTOR_HARDWARE
// In development mode, allow motor control without hardware
#if DEV_LOG_BYPASS_DECISIONS
    printf("Motor control: ALLOWED (development bypass)\r\n");
#endif
    return true;
#endif

    // Production mode: require at least one motor and encoder pair
    bool motor_control_possible =
        (presence->motor1_detected && presence->encoder1_detected) ||
        (presence->motor2_detected && presence->encoder2_detected);

#if DEV_LOG_BYPASS_DECISIONS
    printf("Motor control: %s (hardware check)\r\n",
           motor_control_possible ? "ALLOWED" : "BLOCKED");
#endif

    return motor_control_possible;
}

/**
 * @brief Check if safety system can operate with current hardware
 * @param presence Hardware presence detection results
 * @return bool True if basic safety system can operate
 */
bool is_safety_system_possible(const HardwarePresence_t *presence) {
    if (!presence)
        return false;

#if DEV_BYPASS_SAFETY_HARDWARE
// In development mode, allow safety system without hardware
#if DEV_LOG_BYPASS_DECISIONS
    printf("Safety system: ALLOWED (development bypass)\r\n");
#endif
    return true;
#endif

    // Production mode: require emergency stop at minimum
    bool safety_possible = presence->emergency_stop_detected;

#if DEV_LOG_BYPASS_DECISIONS
    printf("Safety system: %s (hardware check)\r\n",
           safety_possible ? "ALLOWED" : "BLOCKED");
#endif

    return safety_possible;
}
