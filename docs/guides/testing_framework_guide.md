# Testing Framework Guide

## Overview
The STM32H753ZI project uses a dual testing approach: Unity for embedded C unit testing and GoogleTest for C++ integration testing. This guide covers testing strategies, frameworks, and best practices for both simulation and hardware-based testing.

## Testing Architecture

### Dual Testing Framework
```
Testing Strategy
├── Unity Framework (Embedded C)
│   ├── Unit Tests (driver-level)
│   ├── HAL Abstraction Tests
│   └── Simulation Integration Tests
└── GoogleTest Framework (C++)
    ├── Integration Tests
    ├── Performance Tests
    └── System-level Tests
```

### Test Categories
1. **Unit Tests**: Individual function/module testing
2. **Integration Tests**: Component interaction testing
3. **Simulation Tests**: Hardware-free validation
4. **Hardware Tests**: Real hardware validation
5. **Performance Tests**: Timing and efficiency validation

## Unity Framework (Embedded C Testing)

### Setup and Configuration
```c
// test_example.c
#include "unity.h"
#include "drivers/l6470/l6470_driver.h"
#include "hal_abstraction/hal_abstraction.h"

void setUp(void) {
    // Initialize test environment
    mock_hal_reset();
    simulation_reset();
}

void tearDown(void) {
    // Cleanup after test
    mock_hal_verify_all_expectations();
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_l6470_initialization);
    RUN_TEST(test_l6470_movement);
    RUN_TEST(test_l6470_status_reading);
    
    return UNITY_END();
}
```

### Basic Test Patterns
```c
// Simple assertion test
void test_l6470_initialization(void) {
    SystemError_t result = l6470_init(0);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

// Range testing
void test_motor_position_limits(void) {
    int32_t position = l6470_sim_get_position(0);
    TEST_ASSERT_INT_WITHIN(5, 0, position);  // Within ±5 of 0
}

// Array comparison
void test_register_values(void) {
    uint8_t expected[] = {0x12, 0x34, 0x56};
    uint8_t actual[3];
    l6470_read_registers(0, L6470_REG_ABS_POS, actual, 3);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, actual, 3);
}

// Float comparison
void test_motor_speed_calculation(void) {
    float expected_speed = 1000.0f;
    float actual_speed = l6470_calculate_speed(0x1000);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_speed, actual_speed);
}
```

### Mock HAL Testing
```c
// Setup mock expectations
void test_spi_communication(void) {
    // Expect SPI initialization
    mock_hal_expect_spi_init(HAL_SPI_INSTANCE_2, SYSTEM_OK);
    
    // Setup SPI response
    uint8_t response[] = {0x00, 0x12, 0x34};
    mock_hal_set_spi_response(HAL_SPI_INSTANCE_2, response, 3);
    
    // Test the function
    uint16_t status = l6470_get_status(0);
    TEST_ASSERT_EQUAL(0x1234, status);
    
    // Verify mock interactions
    TEST_ASSERT_TRUE(mock_hal_verify_spi_transaction_count(HAL_SPI_INSTANCE_2, 1));
}
```

### Error Handling Tests
```c
void test_error_handling(void) {
    // Setup error condition
    mock_hal_set_spi_error(HAL_SPI_INSTANCE_2, ERROR_HAL_SPI_TIMEOUT);
    
    // Test error propagation
    SystemError_t result = l6470_get_status(0);
    TEST_ASSERT_EQUAL(ERROR_DRIVER_COMMUNICATION_TIMEOUT, result);
}

void test_fault_injection(void) {
    // Inject L6470 fault
    l6470_sim_set_fault(0, L6470_STATUS_UVLO);
    
    // Test fault detection
    uint16_t status = l6470_get_status(0);
    TEST_ASSERT_TRUE(status & L6470_STATUS_UVLO);
    
    // Test fault recovery
    l6470_reset_device(0);
    l6470_sim_clear_faults(0);
    status = l6470_get_status(0);
    TEST_ASSERT_FALSE(status & L6470_STATUS_UVLO);
}
```

## GoogleTest Framework (C++ Testing)

### Setup and Configuration
```cpp
// test_integration.cpp
#include <gtest/gtest.h>
extern "C" {
    #include "drivers/l6470/l6470_driver.h"
    #include "drivers/as5600/as5600_driver.h"
    #include "simulation/motor_simulation.h"
}

class MotorControlTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize simulation
        ASSERT_TRUE(motor_simulation_is_active());
        simulation_reset();
        
        // Initialize drivers
        ASSERT_EQ(SYSTEM_OK, l6470_init(0));
        ASSERT_EQ(SYSTEM_OK, as5600_init(0));
    }
    
    void TearDown() override {
        // Cleanup
        simulation_reset();
    }
};
```

### Integration Test Examples
```cpp
TEST_F(MotorControlTest, BasicMovement) {
    // Command movement
    EXPECT_EQ(SYSTEM_OK, l6470_move(0, 1000));
    
    // Wait for completion
    while (l6470_is_busy(0)) {
        motor_simulation_update(10);
    }
    
    // Verify position
    int32_t position = l6470_sim_get_position(0);
    EXPECT_EQ(1000, position);
}

TEST_F(MotorControlTest, EncoderFollowsMotor) {
    // Link encoder to motor
    EXPECT_EQ(SIM_ERROR_NONE, as5600_sim_link_to_motor(0, 0));
    
    // Move motor
    EXPECT_EQ(SYSTEM_OK, l6470_move(0, 500));
    
    // Update simulation
    for (int i = 0; i < 10; i++) {
        motor_simulation_update(100);
    }
    
    // Verify encoder follows
    uint16_t encoder_pos = as5600_read_angle(0);
    int32_t motor_pos = l6470_sim_get_position(0);
    EXPECT_NEAR(motor_pos, encoder_pos, 10);
}
```

### Parameterized Tests
```cpp
class MotorSpeedTest : public ::testing::TestWithParam<float> {
    // Test setup...
};

TEST_P(MotorSpeedTest, SpeedControl) {
    float target_speed = GetParam();
    
    // Start motor at target speed
    EXPECT_EQ(SYSTEM_OK, l6470_run(0, target_speed));
    
    // Allow time to reach speed
    for (int i = 0; i < 50; i++) {
        motor_simulation_update(20);
    }
    
    // Verify speed
    float actual_speed = l6470_sim_get_speed(0);
    EXPECT_FLOAT_EQ(target_speed, actual_speed);
}

INSTANTIATE_TEST_SUITE_P(
    SpeedRange,
    MotorSpeedTest,
    ::testing::Values(100.0f, 500.0f, 1000.0f, 2000.0f)
);
```

## Test Organization

### Directory Structure
```
tests/
├── unit/                          # Unity unit tests
│   ├── test_l6470_driver.c
│   ├── test_as5600_driver.c
│   ├── test_hal_abstraction.c
│   └── test_simulation.c
├── integration/                   # GoogleTest integration tests
│   ├── test_motor_control.cpp
│   ├── test_closed_loop.cpp
│   └── test_multi_motor.cpp
├── fixtures/                      # Test data and fixtures
│   ├── motor_profiles.json
│   └── expected_responses.h
└── mocks/                         # Mock implementations
    ├── mock_hal.c
    └── mock_spi.c
```

### Test Naming Conventions
```c
// Unit tests: test_[module]_[function]_[condition]
void test_l6470_init_success(void);
void test_l6470_move_invalid_motor_id(void);
void test_as5600_read_angle_communication_error(void);

// Integration tests: [Component]Test.[Scenario]
TEST_F(MotorControlTest, BasicMovement);
TEST_F(ClosedLoopTest, PositionTracking);
TEST_F(SafetyTest, EmergencyStop);
```

## Simulation-Based Testing

### Basic Simulation Tests
```c
void test_simulation_framework(void) {
    // Verify simulation is active
    TEST_ASSERT_TRUE(motor_simulation_is_active());
    
    // Test motor simulation
    l6470_sim_move(0, 1000);
    motor_simulation_update(1000);  // 1 second
    
    int32_t position = l6470_sim_get_position(0);
    TEST_ASSERT_EQUAL(1000, position);
    
    // Test encoder simulation
    as5600_sim_set_position(0, 2048);  // 180 degrees
    uint16_t angle = as5600_sim_get_angle(0);
    TEST_ASSERT_EQUAL(2048, angle);
}
```

### Advanced Simulation Scenarios
```c
void test_coordinated_motion(void) {
    // Initialize both motors
    TEST_ASSERT_EQUAL(SYSTEM_OK, l6470_init(0));
    TEST_ASSERT_EQUAL(SYSTEM_OK, l6470_init(1));
    
    // Start coordinated movement
    l6470_move(0, 1000);  // Motor 0: 1000 steps
    l6470_move(1, 500);   // Motor 1: 500 steps
    
    // Simulate until both complete
    int timeout = 1000;
    while ((l6470_is_busy(0) || l6470_is_busy(1)) && timeout-- > 0) {
        motor_simulation_update(10);
    }
    
    // Verify completion
    TEST_ASSERT_FALSE(l6470_is_busy(0));
    TEST_ASSERT_FALSE(l6470_is_busy(1));
    TEST_ASSERT_EQUAL(1000, l6470_sim_get_position(0));
    TEST_ASSERT_EQUAL(500, l6470_sim_get_position(1));
}
```

## Performance Testing

### Timing Tests
```c
void test_control_loop_timing(void) {
    uint32_t start_time = HAL_GetTick();
    int iterations = 1000;
    
    for (int i = 0; i < iterations; i++) {
        // Simulate control loop
        uint16_t encoder_pos = as5600_read_angle(0);
        int32_t motor_pos = l6470_sim_get_position(0);
        
        int32_t error = encoder_pos - motor_pos;
        if (abs(error) > 10) {
            l6470_move(0, error / 2);
        }
        
        motor_simulation_update(1);
    }
    
    uint32_t elapsed = HAL_GetTick() - start_time;
    float avg_time = (float)elapsed / iterations;
    
    // Verify real-time performance
    TEST_ASSERT_LESS_THAN(1.0f, avg_time);  // <1ms per loop
}
```

### Memory Usage Tests
```c
void test_memory_usage(void) {
    // Measure stack usage
    uint32_t stack_before = get_stack_pointer();
    
    // Run typical operations
    l6470_init(0);
    l6470_move(0, 1000);
    uint16_t status = l6470_get_status(0);
    
    uint32_t stack_after = get_stack_pointer();
    uint32_t stack_used = stack_before - stack_after;
    
    // Verify reasonable stack usage
    TEST_ASSERT_LESS_THAN(512, stack_used);  // <512 bytes
}
```

## Continuous Integration

### CMake Test Configuration
```cmake
# Enable testing
enable_testing()

# Unity tests
add_executable(test_unity
    tests/unit/test_l6470_driver.c
    tests/unit/test_as5600_driver.c
    tests/mocks/mock_hal.c
    src/drivers/l6470/l6470_driver.c
    src/drivers/as5600/as5600_driver.c
)

target_link_libraries(test_unity
    unity
    hal_abstraction_mock
    simulation_framework
)

add_test(NAME unity_tests COMMAND test_unity)

# GoogleTest tests
find_package(GTest REQUIRED)

add_executable(test_integration
    tests/integration/test_motor_control.cpp
    tests/integration/test_closed_loop.cpp
)

target_link_libraries(test_integration
    GTest::gtest_main
    motor_drivers
    simulation_framework
)

add_test(NAME integration_tests COMMAND test_integration)
```

### GitHub Actions Workflow
```yaml
name: Test Suite
on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Install Dependencies
        run: |
          sudo apt-get update
          sudo apt-get install gcc-arm-none-eabi
          sudo apt-get install libgtest-dev
      
      - name: Build Tests
        run: |
          cmake -S . -B build -DBUILD_TESTS=ON
          cmake --build build
      
      - name: Run Unit Tests
        run: |
          cd build
          ctest --output-on-failure -R unity_tests
      
      - name: Run Integration Tests
        run: |
          cd build
          ctest --output-on-failure -R integration_tests
      
      - name: Generate Coverage
        run: |
          gcov build/CMakeFiles/test_*.dir/src/**/*.gcno
          lcov --capture --directory build --output-file coverage.info
      
      - name: Upload Coverage
        uses: codecov/codecov-action@v1
        with:
          file: ./coverage.info
```

## Test Data Management

### Test Fixtures
```c
// fixtures/motor_test_data.h
typedef struct {
    int32_t input_steps;
    uint16_t expected_status;
    float expected_speed;
} MotorTestCase_t;

static const MotorTestCase_t motor_test_cases[] = {
    {1000, L6470_STATUS_DIR, 500.0f},
    {-500, 0x0000, 250.0f},
    {0, L6470_STATUS_HIZ, 0.0f}
};

#define MOTOR_TEST_CASE_COUNT (sizeof(motor_test_cases) / sizeof(motor_test_cases[0]))
```

### Data-Driven Tests
```c
void test_motor_movement_cases(void) {
    for (int i = 0; i < MOTOR_TEST_CASE_COUNT; i++) {
        setUp();  // Reset for each case
        
        const MotorTestCase_t *test_case = &motor_test_cases[i];
        
        // Execute test case
        l6470_move(0, test_case->input_steps);
        
        // Wait for completion
        while (l6470_is_busy(0)) {
            motor_simulation_update(10);
        }
        
        // Verify results
        uint16_t status = l6470_get_status(0);
        float speed = l6470_sim_get_speed(0);
        
        TEST_ASSERT_EQUAL_MESSAGE(test_case->expected_status, 
                                 status & L6470_STATUS_DIR,
                                 "Status mismatch in test case");
        
        tearDown();
    }
}
```

## Best Practices

### Test Design Principles
1. **Arrange-Act-Assert**: Clear test structure
2. **One Assertion Per Test**: Focus on single behavior
3. **Descriptive Names**: Self-documenting test names
4. **Independent Tests**: No dependencies between tests
5. **Fast Execution**: Quick feedback loops

### Simulation Testing Guidelines
1. **Start Simple**: Basic functionality before complex scenarios
2. **Test Edge Cases**: Boundary conditions and error states
3. **Use Realistic Data**: Representative of actual usage
4. **Verify Timing**: Ensure real-time requirements
5. **Document Assumptions**: Clear test prerequisites

### Mock Usage Best Practices
1. **Minimal Mocking**: Only mock external dependencies
2. **Verify Interactions**: Check mock call patterns
3. **Setup/Teardown**: Clean state between tests
4. **Realistic Responses**: Simulate actual hardware behavior
5. **Error Injection**: Test error handling paths

## Troubleshooting

### Common Test Issues
```c
// Issue: Test fails intermittently
// Solution: Ensure proper setup/teardown
void setUp(void) {
    mock_hal_reset();           // Reset mock state
    simulation_reset();         // Reset simulation
    test_motor_id = 0;         // Reset test variables
}

// Issue: Mock expectations not met
// Solution: Verify call order and parameters
mock_hal_expect_spi_init(HAL_SPI_INSTANCE_2, SYSTEM_OK);
mock_hal_expect_gpio_write(HAL_GPIO_PORT_A, HAL_GPIO_PIN_9, HAL_GPIO_PIN_SET);
// ... test code ...
TEST_ASSERT_TRUE(mock_hal_verify_all_expectations());

// Issue: Simulation timing issues
// Solution: Use appropriate time steps
motor_simulation_update(10);  // Small steps for accuracy
TEST_ASSERT_TRUE(simulation_is_stable());  // Verify convergence
```

### Debug Features
```c
// Enable test debugging
#define TEST_DEBUG_ENABLED 1

void debug_test_state(void) {
    printf("Motor 0 position: %d\n", l6470_sim_get_position(0));
    printf("Motor 0 speed: %.2f\n", l6470_sim_get_speed(0));
    printf("Mock call count: %d\n", mock_hal_get_call_count());
}
```

## Related Documentation
- [HAL Abstraction Guide](hal_abstraction_guide.md)
- [Simulation Framework Guide](SIMULATION_FRAMEWORK_GUIDE.md)
- [Simulation Testing Workflows](SIMULATION_TESTING_WORKFLOWS.md)
- [Core Software Instructions](../../.github/instructions/core-software.instructions.md)
