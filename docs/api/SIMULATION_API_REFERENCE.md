# Simulation API Reference

## Overview
Complete API reference for the STM32H753ZI simulation framework, covering both L6470 stepper driver simulation and AS5600 encoder simulation.

## Core Simulation Control

### Framework Initialization
```c
#include "simulation/motor_simulation.h"

/**
 * @brief Initialize the simulation framework
 * @return true if simulation is active, false if hardware mode
 */
bool motor_simulation_is_active(void);

/**
 * @brief Update simulation state
 * @param dt_ms Time step in milliseconds
 * @return Simulation error code
 */
simulation_error_t motor_simulation_update(uint32_t dt_ms);

/**
 * @brief Reset all simulation state
 */
void motor_simulation_reset(void);
```

### Simulation Configuration
```c
/**
 * @brief Set simulation mode for all devices
 * @param mode Simulation mode (BASIC, REALISTIC, FAULT_INJECTION)
 * @return Simulation error code
 */
simulation_error_t simulation_set_mode(SimulationMode_t mode);

/**
 * @brief Get current simulation mode
 * @return Current simulation mode
 */
SimulationMode_t simulation_get_mode(void);
```

## L6470 Stepper Driver Simulation

### Motor Control APIs
```c
#include "simulation/hardware_simulation.h"

/**
 * @brief Simulate motor movement (relative)
 * @param motor_id Motor identifier (0-1)
 * @param steps Number of steps to move (positive = forward)
 * @return Simulation error code
 */
simulation_error_t l6470_sim_move(uint8_t motor_id, int32_t steps);

/**
 * @brief Simulate continuous motor rotation
 * @param motor_id Motor identifier (0-1)
 * @param speed Speed in steps per second
 * @return Simulation error code
 */
simulation_error_t l6470_sim_run(uint8_t motor_id, float speed);

/**
 * @brief Simulate motor stop
 * @param motor_id Motor identifier (0-1)
 * @return Simulation error code
 */
simulation_error_t l6470_sim_stop(uint8_t motor_id);

/**
 * @brief Simulate immediate motor stop (hard stop)
 * @param motor_id Motor identifier (0-1)
 * @return Simulation error code
 */
simulation_error_t l6470_sim_hard_stop(uint8_t motor_id);

/**
 * @brief Simulate motor high-impedance state
 * @param motor_id Motor identifier (0-1)
 * @return Simulation error code
 */
simulation_error_t l6470_sim_hi_z(uint8_t motor_id);
```

### Position and Status APIs
```c
/**
 * @brief Get simulated motor position
 * @param motor_id Motor identifier (0-1)
 * @return Current position in steps
 */
int32_t l6470_sim_get_position(uint8_t motor_id);

/**
 * @brief Get simulated motor speed
 * @param motor_id Motor identifier (0-1)
 * @return Current speed in steps per second
 */
float l6470_sim_get_speed(uint8_t motor_id);

/**
 * @brief Get simulated motor state
 * @param motor_id Motor identifier (0-1)
 * @return Motor state (STOPPED, ACCELERATING, RUNNING, DECELERATING)
 */
motor_state_t l6470_sim_get_state(uint8_t motor_id);

/**
 * @brief Get simulated status register
 * @param motor_id Motor identifier (0-1)
 * @return 16-bit status register value
 */
uint16_t l6470_sim_get_status(uint8_t motor_id);

/**
 * @brief Check if motor is busy (moving)
 * @param motor_id Motor identifier (0-1)
 * @return true if motor is busy, false if idle
 */
bool l6470_sim_is_busy(uint8_t motor_id);
```

### Register Simulation APIs
```c
/**
 * @brief Read simulated L6470 register
 * @param motor_id Motor identifier (0-1)
 * @param reg_addr Register address
 * @return Register value
 */
uint32_t l6470_sim_read_register(uint8_t motor_id, uint8_t reg_addr);

/**
 * @brief Write simulated L6470 register
 * @param motor_id Motor identifier (0-1)
 * @param reg_addr Register address
 * @param value Value to write
 * @return Simulation error code
 */
simulation_error_t l6470_sim_write_register(uint8_t motor_id, uint8_t reg_addr, uint32_t value);

/**
 * @brief Execute simulated L6470 command
 * @param motor_id Motor identifier (0-1)
 * @param command Command byte
 * @param param Command parameter (if applicable)
 * @return Simulation error code
 */
simulation_error_t l6470_sim_execute_command(uint8_t motor_id, uint8_t command, uint32_t param);
```

### Fault Injection APIs
```c
/**
 * @brief Inject fault condition
 * @param motor_id Motor identifier (0-1)
 * @param fault_mask Fault flags to set (see L6470_STATUS_* constants)
 * @return Simulation error code
 */
simulation_error_t l6470_sim_set_fault(uint8_t motor_id, uint16_t fault_mask);

/**
 * @brief Clear fault conditions
 * @param motor_id Motor identifier (0-1)
 * @return Simulation error code
 */
simulation_error_t l6470_sim_clear_faults(uint8_t motor_id);

/**
 * @brief Set undervoltage lockout condition
 * @param motor_id Motor identifier (0-1)
 * @param enable true to enable UVLO fault, false to clear
 * @return Simulation error code
 */
simulation_error_t l6470_sim_set_uvlo(uint8_t motor_id, bool enable);

/**
 * @brief Set thermal warning/shutdown
 * @param motor_id Motor identifier (0-1)
 * @param level 0=none, 1=warning, 2=shutdown
 * @return Simulation error code
 */
simulation_error_t l6470_sim_set_thermal(uint8_t motor_id, uint8_t level);

/**
 * @brief Set overcurrent detection
 * @param motor_id Motor identifier (0-1)
 * @param enable true to trigger OCD, false to clear
 * @return Simulation error code
 */
simulation_error_t l6470_sim_set_overcurrent(uint8_t motor_id, bool enable);
```

## AS5600 Encoder Simulation

### Position APIs
```c
/**
 * @brief Set simulated encoder position
 * @param encoder_id Encoder identifier (0-1)
 * @param position Position in 12-bit counts (0-4095)
 * @return Simulation error code
 */
simulation_error_t as5600_sim_set_position(uint8_t encoder_id, uint16_t position);

/**
 * @brief Get simulated encoder position
 * @param encoder_id Encoder identifier (0-1)
 * @return Position in 12-bit counts (0-4095)
 */
uint16_t as5600_sim_get_position(uint8_t encoder_id);

/**
 * @brief Get simulated raw (unfiltered) angle
 * @param encoder_id Encoder identifier (0-1)
 * @return Raw angle in 12-bit counts
 */
uint16_t as5600_sim_get_raw_angle(uint8_t encoder_id);

/**
 * @brief Get simulated filtered angle
 * @param encoder_id Encoder identifier (0-1)
 * @return Filtered angle in 12-bit counts
 */
uint16_t as5600_sim_get_angle(uint8_t encoder_id);
```

### Motion Pattern APIs
```c
/**
 * @brief Set encoder motion pattern
 * @param encoder_id Encoder identifier (0-1)
 * @param pattern Motion pattern type
 * @param frequency Pattern frequency in Hz
 * @return Simulation error code
 */
simulation_error_t as5600_sim_set_pattern(uint8_t encoder_id, 
                                         SimulationPattern_t pattern,
                                         float frequency);

/**
 * @brief Link encoder to motor (encoder follows motor position)
 * @param encoder_id Encoder identifier (0-1)
 * @param motor_id Motor to link to (0-1, or 0xFF to unlink)
 * @return Simulation error code
 */
simulation_error_t as5600_sim_link_to_motor(uint8_t encoder_id, uint8_t motor_id);

/**
 * @brief Set encoder noise level
 * @param encoder_id Encoder identifier (0-1)
 * @param noise_level Noise amplitude in counts (0-10)
 * @return Simulation error code
 */
simulation_error_t as5600_sim_set_noise(uint8_t encoder_id, uint8_t noise_level);
```

### Status and Diagnostic APIs
```c
/**
 * @brief Get simulated status register
 * @param encoder_id Encoder identifier (0-1)
 * @return 8-bit status register value
 */
uint8_t as5600_sim_get_status(uint8_t encoder_id);

/**
 * @brief Get simulated AGC (Automatic Gain Control) value
 * @param encoder_id Encoder identifier (0-1)
 * @return AGC value (0-255)
 */
uint8_t as5600_sim_get_agc(uint8_t encoder_id);

/**
 * @brief Get simulated magnitude
 * @param encoder_id Encoder identifier (0-1)
 * @return Magnitude value (0-4095)
 */
uint16_t as5600_sim_get_magnitude(uint8_t encoder_id);

/**
 * @brief Set magnetic field strength simulation
 * @param encoder_id Encoder identifier (0-1)
 * @param strength Field strength (0=no magnet, 255=optimal)
 * @return Simulation error code
 */
simulation_error_t as5600_sim_set_magnet_strength(uint8_t encoder_id, uint8_t strength);
```

## Error Handling and Validation

### Error Types
```c
typedef enum {
    SIM_ERROR_NONE = 0,           /**< No error */
    SIM_ERROR_INVALID_ID,         /**< Invalid motor/encoder ID */
    SIM_ERROR_INVALID_PARAMETER,  /**< Invalid parameter value */
    SIM_ERROR_NOT_INITIALIZED,    /**< Simulation not initialized */
    SIM_ERROR_HARDWARE_MODE,      /**< Operation not available in hardware mode */
    SIM_ERROR_TIMEOUT,            /**< Operation timeout */
    SIM_ERROR_INTERNAL            /**< Internal simulation error */
} simulation_error_t;
```

### Validation APIs
```c
/**
 * @brief Validate register value for L6470
 * @param reg_addr Register address
 * @param value Value to validate
 * @return true if valid, false if invalid
 */
bool l6470_validate_register_value(uint8_t reg_addr, uint32_t value);

/**
 * @brief Validate AS5600 register access
 * @param reg_addr Register address
 * @param is_write true for write access, false for read
 * @return true if valid, false if invalid
 */
bool as5600_validate_register_access(uint8_t reg_addr, bool is_write);

/**
 * @brief Get last simulation error
 * @return Last error code
 */
simulation_error_t simulation_get_last_error(void);

/**
 * @brief Clear simulation error state
 */
void simulation_clear_error(void);
```

## System Integration APIs

### HAL Integration
```c
/**
 * @brief Check if HAL call should be simulated
 * @param peripheral Peripheral type (SPI, I2C, etc.)
 * @return true if simulate, false if pass to real HAL
 */
bool simulation_should_intercept_hal(hal_peripheral_t peripheral);

/**
 * @brief Simulate SPI transaction
 * @param instance SPI instance
 * @param tx_data Transmit data buffer
 * @param rx_data Receive data buffer
 * @param size Transaction size
 * @return HAL status code
 */
HAL_StatusTypeDef simulation_spi_transmit_receive(SPI_HandleTypeDef *hspi,
                                                uint8_t *tx_data,
                                                uint8_t *rx_data,
                                                uint16_t size);

/**
 * @brief Simulate I2C read transaction
 * @param instance I2C instance
 * @param dev_addr Device address
 * @param reg_addr Register address
 * @param data Data buffer
 * @param size Read size
 * @return HAL status code
 */
HAL_StatusTypeDef simulation_i2c_read(I2C_HandleTypeDef *hi2c,
                                    uint16_t dev_addr,
                                    uint16_t reg_addr,
                                    uint8_t *data,
                                    uint16_t size);
```

### Debug and Monitoring
```c
/**
 * @brief Get comprehensive simulation status
 * @param status Pointer to status structure
 * @return Simulation error code
 */
simulation_error_t hardware_simulation_get_status(SimulationStatus_t *status);

/**
 * @brief Enable/disable simulation logging
 * @param enable true to enable, false to disable
 */
void simulation_set_logging(bool enable);

/**
 * @brief Get simulation performance metrics
 * @param metrics Pointer to metrics structure
 * @return Simulation error code
 */
simulation_error_t simulation_get_metrics(SimulationMetrics_t *metrics);

/**
 * @brief Reset simulation performance counters
 */
void simulation_reset_metrics(void);
```

## Data Structures

### Simulation Status
```c
typedef struct {
    /* L6470 Motor Status */
    int32_t motor_position;         /**< Current position in steps */
    float motor_speed;              /**< Current speed in steps/sec */
    motor_state_t motor_state;      /**< Current motor state */
    int8_t motor_direction;         /**< 1 = forward, -1 = reverse */
    
    /* AS5600 Encoder Status */
    uint16_t encoder_angle;         /**< Current angle (0-4095) */
    uint16_t encoder_raw_angle;     /**< Raw angle (0-4095) */
    uint8_t encoder_status;         /**< Status register */
    uint8_t encoder_agc;            /**< AGC value */
    uint16_t encoder_magnitude;     /**< Magnitude value */
    
    /* System Status */
    bool simulation_active;         /**< true if simulation running */
    SimulationMode_t mode;          /**< Current simulation mode */
    uint32_t update_count;          /**< Number of simulation updates */
    uint32_t last_update_time;      /**< Last update timestamp */
} SimulationStatus_t;
```

### Performance Metrics
```c
typedef struct {
    float avg_update_time_ms;       /**< Average update time */
    float max_update_time_ms;       /**< Maximum update time */
    uint32_t total_updates;         /**< Total simulation updates */
    uint32_t hal_intercepts;        /**< HAL calls intercepted */
    uint32_t register_accesses;     /**< Total register accesses */
    uint32_t command_executions;    /**< Commands executed */
} SimulationMetrics_t;
```

## Constants and Macros

### Motor Constants
```c
#define SIM_MAX_MOTORS              2
#define SIM_MAX_ENCODERS            2
#define SIM_POSITION_RESOLUTION     4096
#define SIM_TICK_RATE_HZ           1000

#define SIM_DEFAULT_ACCELERATION   1000.0f   /* steps/s² */
#define SIM_DEFAULT_MAX_SPEED      2000.0f   /* steps/s */
#define SIM_DEFAULT_MIN_SPEED      0.1f      /* steps/s */
```

### Register Addresses
```c
/* L6470 Status Register Bits */
#define L6470_STATUS_HIZ           (1 << 0)
#define L6470_STATUS_BUSY          (1 << 1)
#define L6470_STATUS_SW_F          (1 << 2)
#define L6470_STATUS_SW_EVN        (1 << 3)
#define L6470_STATUS_DIR           (1 << 4)
#define L6470_STATUS_NOTPERF_CMD   (1 << 7)
#define L6470_STATUS_WRONG_CMD     (1 << 8)
#define L6470_STATUS_UVLO          (1 << 9)
#define L6470_STATUS_TH_WRN        (1 << 10)
#define L6470_STATUS_TH_SD         (1 << 11)
#define L6470_STATUS_OCD           (1 << 12)
#define L6470_STATUS_STEP_LOSS_A   (1 << 13)
#define L6470_STATUS_STEP_LOSS_B   (1 << 14)
#define L6470_STATUS_SCK_MOD       (1 << 15)

/* AS5600 Status Register Bits */
#define AS5600_STATUS_MH           (1 << 3)   /* Magnet too strong */
#define AS5600_STATUS_ML           (1 << 4)   /* Magnet too weak */
#define AS5600_STATUS_MD           (1 << 5)   /* Magnet detected */
```

## Usage Examples

### Basic Motor Control
```c
// Initialize simulation
if (motor_simulation_is_active()) {
    printf("Simulation mode active\n");
    
    // Move motor 1000 steps
    simulation_error_t result = l6470_sim_move(0, 1000);
    if (result == SIM_ERROR_NONE) {
        // Wait for completion
        while (l6470_sim_is_busy(0)) {
            motor_simulation_update(10);  // 10ms steps
        }
        
        // Check final position
        int32_t pos = l6470_sim_get_position(0);
        printf("Final position: %d\n", pos);
    }
}
```

### Encoder Monitoring
```c
// Link encoder to motor
as5600_sim_link_to_motor(0, 0);  // Encoder 0 follows Motor 0

// Start motor movement
l6470_sim_run(0, 500.0f);  // 500 steps/sec

// Monitor encoder feedback
for (int i = 0; i < 100; i++) {
    motor_simulation_update(50);  // 50ms steps
    
    uint16_t angle = as5600_sim_get_angle(0);
    int32_t motor_pos = l6470_sim_get_position(0);
    
    printf("Step %d: Motor=%d, Encoder=%d\n", i, motor_pos, angle);
}
```

## Related Documentation
- [Simulation Framework Guide](../guides/SIMULATION_FRAMEWORK_GUIDE.md)
- [Simulation Testing Workflows](../guides/SIMULATION_TESTING_WORKFLOWS.md)
- [HAL Abstraction Guide](../guides/hal_abstraction_guide.md)
- [L6470 Driver API](l6470_driver_api.md)
- [AS5600 Driver API](as5600_driver_api.md)
