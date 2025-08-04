---
applyTo: "src/config/motor_config.h"
description: "L6470 stepper driver configuration and motor control parameters"
---

# L6470 Stepper Driver Configuration Instructions

## Overview
This instruction file provides comprehensive guidance for configuring the L6470 dSPIN stepper motor drivers used in the X-NUCLEO-IHM02A1 expansion shield.

## Enhanced Documentation Integration
**Reference Assets**: This instruction leverages comprehensive L6470 and STM32H7 documentation:
- **X-CUBE-SPN2 Documentation**: 2.1MB L6470-specific reference materials with 356 searchable keywords
- **Unified Search System**: Enhanced search across STM32H7 and L6470 documentation (88.1MB total coverage)
- **Copilot-Optimized Search Commands**:
  ```bash
  # Search L6470 functions with semantic categorization
  # NOTE: L6470 functions are concatenated (e.g., "voidL6470_GoHome")
  python3 scripts/search_enhanced_docs.py function L6470 --scope L6470
  
  # Find L6470 registers with improved targeting
  python3 scripts/search_enhanced_docs.py register ABS_POS --scope L6470
  
  # Search motor control concepts across all documentation
  python3 scripts/search_enhanced_docs.py concept "motor_control" --scope all
  
  # Find stepper driver configuration
  python3 scripts/search_enhanced_docs.py concept "stepper_configuration" --scope L6470
  ```
- **L6470 Search Patterns**: 
  - ✅ Use `function L6470` instead of `function L6470_Init` (functions are concatenated)
  - ✅ Use broad terms: `register SPEED`, `register ABS`, `register CONFIG`
  - ✅ Concept searches work best: `concept motor`, `concept stepper`, `concept daisy`
- **Token-Efficient Categories**: Semantic groupings reduce Copilot search overhead by 7x
- **Cross-Reference Integration**: STM32H7 SPI + L6470 driver unified documentation access

## L6470 Driver Characteristics

### Key Specifications (from ST L6470 Datasheet)
- **Supply Voltage**: 8V to 45V motor supply
- **Motor Current**: Up to 7A RMS per phase
- **Microstepping**: Up to 1/128 step resolution
- **SPI Interface**: Up to 5MHz, Mode 3 (CPOL=1, CPHA=1)
- **Daisy-Chain**: Multiple devices on single SPI bus
- **Built-in Motion Profile**: Acceleration/deceleration curves

### Register Configuration Strategy

#### Essential Registers for SSOT Configuration
Based on L6470 datasheet and ST application notes:

```c
// Motion Profile Parameters (SSOT Reference)
#define MOTOR_ACCELERATION      0x08A           // Acceleration: ~100 step/s²
#define MOTOR_DECELERATION      0x08A           // Deceleration: ~100 step/s²
#define MOTOR_MAX_SPEED         0x041           // Max speed: ~100 step/s
#define MOTOR_MIN_SPEED         0x000           // Min speed: 0 (stopped)
#define MOTOR_FS_SPD            0x027           // Full step speed: ~40 step/s

// Current Control (Critical for Motor Safety)
#define MOTOR_KVAL_HOLD         0x29            // Holding voltage: ~25%
#define MOTOR_KVAL_RUN          0x29            // Running voltage: ~25%  
#define MOTOR_KVAL_ACC          0x29            // Acceleration voltage: ~25%
#define MOTOR_KVAL_DEC          0x29            // Deceleration voltage: ~25%

// Step Mode Configuration
#define MOTOR_STEP_MODE         STEP_MODE_1_16  // 1/16 microstepping
#define MOTOR_SYNC_MODE         SYNC_EN         // Sync output enabled
```

#### Protection Parameters
```c
// Overcurrent Protection (Critical for Safety)
#define MOTOR_OCD_TH            OCD_TH_3000mA   // 3A overcurrent threshold
#define MOTOR_STALL_TH          STALL_TH_2000mA // 2A stall detection

// Thermal Protection
#define MOTOR_THERMAL_WARNING   TW_ENABLE       // Enable thermal warning
#define MOTOR_THERMAL_SHUTDOWN  TS_ENABLE       // Enable thermal shutdown

// Supply Voltage Monitoring
#define MOTOR_UVLO_ENABLE       UVLO_ENABLE     // Under-voltage lockout
```

## Motor Characterization Requirements

### Stepper Motor Specifications
Define motor-specific parameters based on your stepper motors:

```c
// Motor Physical Characteristics (Update based on actual motors)
#define MOTOR_STEPS_PER_REV     200             // Typical bipolar stepper
#define MOTOR_GEAR_RATIO        1.0f            // Direct drive (no gearbox)
#define MOTOR_HOLDING_TORQUE    0.4f            // Nm (motor datasheet)
#define MOTOR_RATED_CURRENT     1.5f            // A (motor datasheet)
#define MOTOR_PHASE_RESISTANCE  2.0f            // Ohm (motor datasheet)
#define MOTOR_PHASE_INDUCTANCE  3.0f            // mH (motor datasheet)
```

### Motion Profile Tuning
```c
// Calculated Motion Parameters
#define STEPS_PER_DEGREE        (MOTOR_STEPS_PER_REV / 360.0f)
#define MAX_VELOCITY_DEG_S      180.0f          // Maximum velocity in deg/s
#define ACCELERATION_DEG_S2     90.0f           // Acceleration in deg/s²

// L6470 Register Values (derived from motor specs)
#define L6470_MAX_SPEED_VAL     ((uint16_t)(MAX_VELOCITY_DEG_S * STEPS_PER_DEGREE * 67.108864))
#define L6470_ACC_VAL           ((uint16_t)(ACCELERATION_DEG_S2 * STEPS_PER_DEGREE * 0.137438))
```

## SPI Communication Protocol

### Daisy-Chain Configuration
The X-NUCLEO-IHM02A1 uses two L6470s in daisy-chain configuration:

```c
// SPI Transaction Structure for Daisy-Chain
typedef struct {
    uint8_t motor1_command;     // Command for first L6470
    uint8_t motor2_command;     // Command for second L6470
    // Additional data bytes as needed for each command
} L6470_DaisyChain_t;

// Example: Move both motors simultaneously
void l6470_dual_move(int32_t motor1_steps, int32_t motor2_steps) {
    // Send command to both motors in single SPI transaction
    // Motor 1 data shifted first, Motor 2 data shifted second
}
```

### Command Protocol Implementation
```c
// Essential L6470 Commands (from datasheet)
#define L6470_CMD_NOP           0x00    // No operation
#define L6470_CMD_SETPARAM      0x00    // Set parameter (+ register address)
#define L6470_CMD_GETPARAM      0x20    // Get parameter (+ register address)
#define L6470_CMD_RUN           0x50    // Run at constant speed
#define L6470_CMD_STEPCLOCK     0x58    // Step clock mode
#define L6470_CMD_MOVE          0x40    // Move with specified steps
#define L6470_CMD_GOTO          0x60    // Go to absolute position
#define L6470_CMD_GOTO_DIR      0x68    // Go to position with direction
#define L6470_CMD_GOUNTIL       0x82    // Go until switch event
#define L6470_CMD_RELEASESW     0x92    // Release switch
#define L6470_CMD_GOHOME        0x70    // Go to home position
#define L6470_CMD_GOMARK        0x78    // Go to mark position
#define L6470_CMD_RESETPOS      0xD8    // Reset position counter
#define L6470_CMD_RESETDEVICE   0xC0    // Reset device
#define L6470_CMD_SOFTSTOP      0xB0    // Soft stop (with deceleration)
#define L6470_CMD_HARDSTOP      0xB8    // Hard stop (immediate)
#define L6470_CMD_SOFTHIZ       0xA0    // Soft HiZ (high impedance)
#define L6470_CMD_HARDHIZ       0xA8    // Hard HiZ (immediate)
#define L6470_CMD_GETSTATUS     0xD0    // Get status register
```

## Register Configuration Sequences

### Initialization Sequence
```c
void l6470_init_sequence(void) {
    // 1. Reset devices
    l6470_send_command(L6470_CMD_RESETDEVICE);
    HAL_Delay(10);  // Allow reset to complete
    
    // 2. Configure step mode
    l6470_set_param(L6470_REG_STEP_MODE, MOTOR_STEP_MODE);
    
    // 3. Configure motion profile
    l6470_set_param(L6470_REG_ACC, MOTOR_ACCELERATION);
    l6470_set_param(L6470_REG_DEC, MOTOR_DECELERATION);
    l6470_set_param(L6470_REG_MAX_SPEED, MOTOR_MAX_SPEED);
    
    // 4. Configure current control
    l6470_set_param(L6470_REG_KVAL_HOLD, MOTOR_KVAL_HOLD);
    l6470_set_param(L6470_REG_KVAL_RUN, MOTOR_KVAL_RUN);
    l6470_set_param(L6470_REG_KVAL_ACC, MOTOR_KVAL_ACC);
    l6470_set_param(L6470_REG_KVAL_DEC, MOTOR_KVAL_DEC);
    
    // 5. Configure protection
    l6470_set_param(L6470_REG_OCD_TH, MOTOR_OCD_TH);
    l6470_set_param(L6470_REG_STALL_TH, MOTOR_STALL_TH);
    
    // 6. Enable outputs
    l6470_send_command(L6470_CMD_RELEASESW);
}
```

### Status Monitoring
```c
// Status Register Bits (Critical for Safety)
#define L6470_STATUS_HIZ        (1<<0)   // High impedance state
#define L6470_STATUS_BUSY       (1<<1)   // Motor busy
#define L6470_STATUS_SW_F       (1<<2)   // Switch falling edge
#define L6470_STATUS_SW_EVN     (1<<3)   // Switch event
#define L6470_STATUS_DIR        (1<<4)   // Direction
#define L6470_STATUS_MOT_STATUS (3<<5)   // Motor status (2 bits)
#define L6470_STATUS_NOTPERF_CMD (1<<7)  // Command not performed
#define L6470_STATUS_WRONG_CMD  (1<<8)   // Wrong command
#define L6470_STATUS_UVLO       (1<<9)   // Under-voltage lockout
#define L6470_STATUS_TH_WRN     (1<<10)  // Thermal warning
#define L6470_STATUS_TH_SD      (1<<11)  // Thermal shutdown
#define L6470_STATUS_OCD        (1<<12)  // Overcurrent detection
#define L6470_STATUS_STEP_LOSS_A (1<<13) // Stall detection phase A
#define L6470_STATUS_STEP_LOSS_B (1<<14) // Stall detection phase B
#define L6470_STATUS_SCK_MOD    (1<<15)  // Step clock mode
```

## Safety Implementation Requirements

### Fault Detection and Recovery
```c
typedef enum {
    L6470_FAULT_NONE = 0,
    L6470_FAULT_OVERCURRENT,
    L6470_FAULT_THERMAL_WARNING,
    L6470_FAULT_THERMAL_SHUTDOWN,
    L6470_FAULT_UNDER_VOLTAGE,
    L6470_FAULT_STEP_LOSS,
    L6470_FAULT_COMMUNICATION
} L6470_Fault_t;

L6470_Fault_t l6470_check_faults(uint16_t status) {
    if (status & L6470_STATUS_OCD) return L6470_FAULT_OVERCURRENT;
    if (status & L6470_STATUS_TH_SD) return L6470_FAULT_THERMAL_SHUTDOWN;
    if (status & L6470_STATUS_TH_WRN) return L6470_FAULT_THERMAL_WARNING;
    if (status & L6470_STATUS_UVLO) return L6470_FAULT_UNDER_VOLTAGE;
    if (status & (L6470_STATUS_STEP_LOSS_A | L6470_STATUS_STEP_LOSS_B)) 
        return L6470_FAULT_STEP_LOSS;
    return L6470_FAULT_NONE;
}
```

### Emergency Stop Implementation
```c
void l6470_emergency_stop(void) {
    // Immediate stop with high impedance
    l6470_send_command(L6470_CMD_HARDHIZ);
    
    // Clear any pending commands
    l6470_send_command(L6470_CMD_NOP);
    
    // Log fault condition
    system_log_error(ERROR_MOTOR_EMERGENCY_STOP);
}
```

## Performance Optimization

### SPI Timing Optimization
```c
// Optimize SPI timing for daisy-chain efficiency
#define L6470_SPI_TIMEOUT_MS    10      // Maximum wait for SPI completion
#define L6470_CS_SETUP_TIME_US  1       // CS setup time before SPI
#define L6470_CS_HOLD_TIME_US   1       // CS hold time after SPI
```

### Motion Profile Optimization
```c
// Pre-calculated values for common motions
#define MOTION_PROFILE_SMOOTH   1       // Smooth acceleration profile
#define MOTION_PROFILE_FAST     2       // Fast positioning profile
#define MOTION_PROFILE_PRECISE  3       // High precision profile

typedef struct {
    uint16_t acceleration;
    uint16_t deceleration;
    uint16_t max_speed;
    uint8_t step_mode;
} MotionProfile_t;
```

## Integration with Closed-Loop Control

### Position Feedback Integration
```c
// Interface with AS5600 encoders for closed-loop control
typedef struct {
    int32_t commanded_position;     // L6470 commanded position
    float actual_position;          // AS5600 measured position (degrees)
    float position_error;           // Error for closed-loop correction
    uint32_t last_update_time;      // Timestamp of last update
} MotorPosition_t;
```

### Error Correction Strategy
```c
// Closed-loop position correction
#define POSITION_ERROR_THRESHOLD    2.0f    // Degrees
#define CORRECTION_GAIN            0.5f     // Proportional gain

void l6470_closed_loop_correction(uint8_t motor_id) {
    MotorPosition_t *pos = &motor_positions[motor_id];
    
    if (fabs(pos->position_error) > POSITION_ERROR_THRESHOLD) {
        int32_t correction_steps = (int32_t)(pos->position_error * 
                                           STEPS_PER_DEGREE * CORRECTION_GAIN);
        l6470_relative_move(motor_id, correction_steps);
    }
}
```

Remember to always validate L6470 configuration against your specific motor specifications and mechanical constraints. Monitor status registers continuously for safe operation.
