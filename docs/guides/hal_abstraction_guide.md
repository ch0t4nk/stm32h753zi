# HAL Abstraction Layer Guide

## Overview
The Hardware Abstraction Layer (HAL) provides a platform-independent interface for hardware operations, enabling code to work on both real STM32H753ZI hardware and simulation environments without modification.

## Architecture

### Layer Structure
```
Application Layer
    ↓
HAL Abstraction Interface (hal_abstraction.h)
    ↓
├── STM32H7 Implementation (hal_abstraction_stm32h7.c)
└── Mock Implementation (hal_abstraction_mock.c)
```

### Key Benefits
- **Platform Independence**: Same code works on hardware and simulation
- **Testability**: Mock implementation enables unit testing
- **Maintainability**: Clear separation between hardware and logic
- **Safety**: Controlled hardware access through validated interfaces

## Core Interfaces

### SPI Interface
```c
#include "hal_abstraction/hal_abstraction.h"

// Initialize SPI peripheral
SystemError_t HAL_Abstraction_SPI_Init(HAL_SPI_Instance_t instance);

// Transmit and receive data
SystemError_t HAL_Abstraction_SPI_TransmitReceive(
    HAL_SPI_Instance_t instance,
    uint8_t *tx_data,
    uint8_t *rx_data,
    uint16_t size,
    uint32_t timeout_ms
);

// Example usage with L6470
uint8_t tx_data[3] = {L6470_CMD_GET_STATUS, 0x00, 0x00};
uint8_t rx_data[3] = {0};
SystemError_t result = HAL_Abstraction_SPI_TransmitReceive(
    HAL_SPI_INSTANCE_2, tx_data, rx_data, 3, 1000
);
```

### I2C Interface
```c
// Initialize I2C peripheral
SystemError_t HAL_Abstraction_I2C_Init(HAL_I2C_Instance_t instance);

// Read from device register
SystemError_t HAL_Abstraction_I2C_ReadRegister(
    HAL_I2C_Instance_t instance,
    uint16_t device_address,
    uint16_t register_address,
    uint8_t *data,
    uint16_t size,
    uint32_t timeout_ms
);

// Example usage with AS5600
uint8_t angle_data[2];
SystemError_t result = HAL_Abstraction_I2C_ReadRegister(
    HAL_I2C_INSTANCE_1, AS5600_ADDRESS, AS5600_REG_ANGLE_H, 
    angle_data, 2, 1000
);
```

### GPIO Interface
```c
// Initialize GPIO pin
SystemError_t HAL_Abstraction_GPIO_Init(
    HAL_GPIO_Port_t port,
    HAL_GPIO_Pin_t pin,
    HAL_GPIO_Mode_t mode
);

// Write GPIO pin
SystemError_t HAL_Abstraction_GPIO_WritePin(
    HAL_GPIO_Port_t port,
    HAL_GPIO_Pin_t pin,
    HAL_GPIO_PinState_t state
);

// Read GPIO pin
HAL_GPIO_PinState_t HAL_Abstraction_GPIO_ReadPin(
    HAL_GPIO_Port_t port,
    HAL_GPIO_Pin_t pin
);

// Example usage for motor enable
HAL_Abstraction_GPIO_WritePin(
    HAL_GPIO_PORT_A, HAL_GPIO_PIN_9, HAL_GPIO_PIN_SET
);
```

### Timer Interface
```c
// Initialize timer
SystemError_t HAL_Abstraction_Timer_Init(
    HAL_Timer_Instance_t instance,
    uint32_t frequency_hz
);

// Start timer
SystemError_t HAL_Abstraction_Timer_Start(HAL_Timer_Instance_t instance);

// Get timer value
uint32_t HAL_Abstraction_Timer_GetValue(HAL_Timer_Instance_t instance);
```

## Implementation Details

### STM32H7 Implementation
**File**: `src/hal_abstraction/hal_abstraction_stm32h7.c`

**Features**:
- Direct mapping to STM32H7 HAL functions
- Hardware-specific optimizations
- Real peripheral initialization
- Interrupt handling integration

**Example**:
```c
SystemError_t HAL_Abstraction_SPI_Init(HAL_SPI_Instance_t instance) {
    switch (instance) {
        case HAL_SPI_INSTANCE_2:
            hspi2.Instance = SPI2;
            hspi2.Init.Mode = SPI_MODE_MASTER;
            hspi2.Init.Direction = SPI_DIRECTION_2LINES;
            // ... other configuration
            
            if (HAL_SPI_Init(&hspi2) != HAL_OK) {
                return ERROR_HAL_SPI_INIT_FAILED;
            }
            return SYSTEM_OK;
        default:
            return ERROR_HAL_INVALID_INSTANCE;
    }
}
```

### Mock Implementation
**File**: `src/hal_abstraction/hal_abstraction_mock.c`

**Features**:
- Simulation-friendly implementations
- Configurable responses for testing
- State tracking for validation
- Integration with simulation framework

**Example**:
```c
SystemError_t HAL_Abstraction_SPI_TransmitReceive(
    HAL_SPI_Instance_t instance,
    uint8_t *tx_data,
    uint8_t *rx_data,
    uint16_t size,
    uint32_t timeout_ms
) {
    // Log transaction for testing
    mock_spi_log_transaction(instance, tx_data, size);
    
    // Generate simulated response
    if (instance == HAL_SPI_INSTANCE_2) {
        return simulate_l6470_spi_response(tx_data, rx_data, size);
    }
    
    return SYSTEM_OK;
}
```

## Usage Patterns

### Driver Implementation
```c
// L6470 driver using HAL abstraction
SystemError_t l6470_init(uint8_t motor_id) {
    // Initialize SPI through HAL abstraction
    SystemError_t result = HAL_Abstraction_SPI_Init(HAL_SPI_INSTANCE_2);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Initialize GPIO pins
    result = HAL_Abstraction_GPIO_Init(
        MOTOR1_CS_PORT, MOTOR1_CS_PIN, HAL_GPIO_MODE_OUTPUT_PP
    );
    
    return result;
}

SystemError_t l6470_send_command(uint8_t motor_id, uint8_t command) {
    // Select device
    HAL_Abstraction_GPIO_WritePin(
        MOTOR1_CS_PORT, MOTOR1_CS_PIN, HAL_GPIO_PIN_RESET
    );
    
    // Send command
    uint8_t tx_data = command;
    uint8_t rx_data = 0;
    SystemError_t result = HAL_Abstraction_SPI_TransmitReceive(
        HAL_SPI_INSTANCE_2, &tx_data, &rx_data, 1, 1000
    );
    
    // Deselect device
    HAL_Abstraction_GPIO_WritePin(
        MOTOR1_CS_PORT, MOTOR1_CS_PIN, HAL_GPIO_PIN_SET
    );
    
    return result;
}
```

### Testing with Mock HAL
```c
#include "unity.h"
#include "hal_abstraction/hal_abstraction.h"

void test_l6470_initialization(void) {
    // Setup mock expectations
    mock_hal_expect_spi_init(HAL_SPI_INSTANCE_2, SYSTEM_OK);
    mock_hal_expect_gpio_init(HAL_GPIO_PORT_A, HAL_GPIO_PIN_9, SYSTEM_OK);
    
    // Test driver initialization
    SystemError_t result = l6470_init(0);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Verify mock calls
    TEST_ASSERT_TRUE(mock_hal_verify_all_expectations());
}
```

## Configuration

### Build Configuration
```cmake
# CMakeLists.txt - Select HAL implementation
if(BUILD_FOR_HARDWARE)
    target_sources(${TARGET_NAME} PRIVATE
        src/hal_abstraction/hal_abstraction_stm32h7.c
    )
    target_compile_definitions(${TARGET_NAME} PRIVATE
        HAL_IMPLEMENTATION_STM32H7=1
    )
else()
    target_sources(${TARGET_NAME} PRIVATE
        src/hal_abstraction/hal_abstraction_mock.c
    )
    target_compile_definitions(${TARGET_NAME} PRIVATE
        HAL_IMPLEMENTATION_MOCK=1
    )
endif()
```

### SSOT Configuration
**File**: `src/config/hardware_config.h`
```c
// HAL instance mappings
#define HAL_SPI_INSTANCE_L6470    HAL_SPI_INSTANCE_2
#define HAL_I2C_INSTANCE_AS5600   HAL_I2C_INSTANCE_1
#define HAL_TIMER_INSTANCE_MAIN   HAL_TIMER_INSTANCE_2

// GPIO pin mappings
#define MOTOR1_CS_PORT   HAL_GPIO_PORT_A
#define MOTOR1_CS_PIN    HAL_GPIO_PIN_9
#define MOTOR2_CS_PORT   HAL_GPIO_PORT_A
#define MOTOR2_CS_PIN    HAL_GPIO_PIN_10
```

## Error Handling

### Error Codes
```c
typedef enum {
    ERROR_HAL_SPI_INIT_FAILED = 0x2000,
    ERROR_HAL_SPI_TIMEOUT,
    ERROR_HAL_SPI_ERROR,
    ERROR_HAL_I2C_INIT_FAILED,
    ERROR_HAL_I2C_TIMEOUT,
    ERROR_HAL_I2C_NACK,
    ERROR_HAL_GPIO_INVALID_PORT,
    ERROR_HAL_GPIO_INVALID_PIN,
    ERROR_HAL_TIMER_INIT_FAILED,
    ERROR_HAL_INVALID_INSTANCE
} HAL_ErrorCode_t;
```

### Error Handling Pattern
```c
SystemError_t driver_operation(void) {
    SystemError_t result = HAL_Abstraction_SPI_TransmitReceive(
        HAL_SPI_INSTANCE_2, tx_data, rx_data, size, 1000
    );
    
    switch (result) {
        case SYSTEM_OK:
            return SYSTEM_OK;
        case ERROR_HAL_SPI_TIMEOUT:
            // Handle timeout
            return ERROR_DRIVER_COMMUNICATION_TIMEOUT;
        case ERROR_HAL_SPI_ERROR:
            // Handle SPI error
            return ERROR_DRIVER_HARDWARE_FAULT;
        default:
            return ERROR_DRIVER_UNKNOWN;
    }
}
```

## Testing Integration

### Unit Testing
```c
// Test with mock HAL
void test_driver_communication(void) {
    // Setup mock SPI response
    uint8_t expected_response[] = {0x12, 0x34};
    mock_hal_set_spi_response(HAL_SPI_INSTANCE_2, expected_response, 2);
    
    // Test driver function
    uint16_t result = driver_read_register(0x10);
    TEST_ASSERT_EQUAL(0x1234, result);
    
    // Verify SPI transaction
    uint8_t *tx_data = mock_hal_get_last_spi_tx(HAL_SPI_INSTANCE_2);
    TEST_ASSERT_EQUAL(0x10, tx_data[0]);  // Register address
}
```

### Integration Testing
```c
// Test with simulation
void test_motor_movement_integration(void) {
    // Initialize with simulation HAL
    TEST_ASSERT_EQUAL(SYSTEM_OK, l6470_init(0));
    
    // Command movement
    TEST_ASSERT_EQUAL(SYSTEM_OK, l6470_move(0, 1000));
    
    // Verify simulation response
    int32_t position = l6470_sim_get_position(0);
    TEST_ASSERT_EQUAL(1000, position);
}
```

## Performance Considerations

### Overhead Analysis
- **STM32H7 Implementation**: Minimal overhead (~1-2 instruction cycles)
- **Mock Implementation**: Slight overhead for logging (~10-20 cycles)
- **Memory Usage**: ~500 bytes additional RAM for abstraction layer

### Optimization Tips
1. **Inline Functions**: Use inline for critical path functions
2. **Compile-Time Selection**: Use conditional compilation for implementation selection
3. **Minimal Abstraction**: Only abstract what needs to be portable
4. **Efficient Mocking**: Use lightweight mocking for high-frequency operations

## Troubleshooting

### Common Issues

**1. HAL Not Initialized**
```c
// Always check initialization
if (HAL_Abstraction_SPI_Init(HAL_SPI_INSTANCE_2) != SYSTEM_OK) {
    printf("ERROR: SPI initialization failed\n");
    return ERROR_HAL_SPI_INIT_FAILED;
}
```

**2. Mock Expectations Not Met**
```c
// Verify all mock expectations
if (!mock_hal_verify_all_expectations()) {
    printf("ERROR: Mock HAL expectations not met\n");
    mock_hal_print_unmet_expectations();
}
```

**3. Timeout Issues**
```c
// Use appropriate timeouts
#define SPI_TIMEOUT_MS  1000
#define I2C_TIMEOUT_MS  500

SystemError_t result = HAL_Abstraction_SPI_TransmitReceive(
    instance, tx_data, rx_data, size, SPI_TIMEOUT_MS
);
```

### Debug Features
```c
// Enable HAL debugging
#define HAL_DEBUG_ENABLED 1

// Get HAL status
HAL_Status_t status;
HAL_Abstraction_GetStatus(&status);
printf("SPI2 Status: %s\n", status.spi2_initialized ? "OK" : "Not Init");
```

## Best Practices

1. **Always Use HAL Abstraction**: Never call STM32 HAL directly from drivers
2. **Check Return Values**: Always verify HAL function return codes
3. **Use SSOT Configuration**: Reference hardware config from centralized headers
4. **Test with Mocks**: Write unit tests using mock HAL implementation
5. **Handle Timeouts**: Always specify reasonable timeout values
6. **Document Assumptions**: Clearly document hardware dependencies

## Migration Guide

### From Direct HAL Usage
```c
// OLD: Direct STM32 HAL usage
HAL_SPI_TransmitReceive(&hspi2, tx_data, rx_data, size, 1000);

// NEW: HAL abstraction usage
HAL_Abstraction_SPI_TransmitReceive(
    HAL_SPI_INSTANCE_2, tx_data, rx_data, size, 1000
);
```

### Adding New Peripherals
1. Add interface to `hal_abstraction.h`
2. Implement in `hal_abstraction_stm32h7.c`
3. Implement mock in `hal_abstraction_mock.c`
4. Add configuration to `hardware_config.h`
5. Write unit tests with mock implementation

## Related Documentation
- [Dual Build System Instructions](../../.github/instructions/dual-build-system.instructions.md)
- [Core Software Instructions](../../.github/instructions/core-software.instructions.md)
- [Simulation Framework Guide](SIMULATION_FRAMEWORK_GUIDE.md)
- [Testing Framework Guide](testing_framework_guide.md)
