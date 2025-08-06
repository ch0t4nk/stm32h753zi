cmake_minimum_required(VERSION 3.15)

# Test configuration
project(stm32h753_tests LANGUAGES C)

# Use local Unity testing framework
set(UNITY_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/unity)

# Test directories
set(TEST_UNIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/unit)
set(TEST_INTEGRATION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/integration)
set(TEST_MOCKS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mocks)
set(TEST_FIXTURES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fixtures)

# Include test directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${TEST_MOCKS_DIR}
    ${TEST_FIXTURES_DIR}
    ${UNITY_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/src/safety
    ${CMAKE_SOURCE_DIR}/src/controllers
    ${CMAKE_SOURCE_DIR}/src/communication
    ${CMAKE_SOURCE_DIR}/src/drivers
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/drivers/STM32H7xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/drivers/CMSIS/Device/ST/STM32H7xx/Include
    ${CMAKE_SOURCE_DIR}/drivers/CMSIS/Include
)

# Create Unity library
add_library(unity STATIC ${UNITY_SOURCE_DIR}/unity.c)
target_include_directories(unity PUBLIC ${UNITY_SOURCE_DIR})

# Test source files
set(SAFETY_TEST_SOURCES
    ${TEST_UNIT_DIR}/test_safety_systems.c
    ${TEST_UNIT_DIR}/test_emergency_stop.c
    ${TEST_UNIT_DIR}/test_watchdog.c
    ${TEST_UNIT_DIR}/test_fault_monitor.c
    ${TEST_MOCKS_DIR}/mock_hal.c
    ${TEST_MOCKS_DIR}/mock_gpio.c
    ${CMAKE_SOURCE_DIR}/src/safety/safety_system.c
    ${CMAKE_SOURCE_DIR}/src/safety/emergency_stop.c
    ${CMAKE_SOURCE_DIR}/src/safety/watchdog_manager.c
    ${CMAKE_SOURCE_DIR}/src/safety/fault_monitor.c
)

# Create test executables using local Unity
message(STATUS "Using local Unity testing framework - enabling unit tests")

# Safety systems test suite (only existing test files)
add_executable(test_safety_systems ${SAFETY_TEST_SOURCES})
target_link_libraries(test_safety_systems unity)
target_compile_definitions(test_safety_systems PRIVATE
    STM32H753xx
    USE_HAL_DRIVER
    UNITY_TESTING
)

# Basic safety test (exists)
add_executable(test_basic_safety
    ${TEST_UNIT_DIR}/test_basic_safety.c
    ${TEST_MOCKS_DIR}/mock_hal.c
)
target_link_libraries(test_basic_safety unity)
target_compile_definitions(test_basic_safety PRIVATE
    STM32H753xx
    USE_HAL_DRIVER
    UNITY_TESTING
)

# Unity emergency stop test (exists)
add_executable(test_emergency_stop_unity
    ${TEST_UNIT_DIR}/test_emergency_stop_unity.c
    ${TEST_MOCKS_DIR}/mock_hal.c
)
target_link_libraries(test_emergency_stop_unity unity)
target_compile_definitions(test_emergency_stop_unity PRIVATE
    STM32H753xx
    USE_HAL_DRIVER
    UNITY_TESTING
)

# Add tests to CTest (only existing tests)
enable_testing()
add_test(NAME SafetySystemsTest COMMAND test_safety_systems)
add_test(NAME BasicSafetyTest COMMAND test_basic_safety)
add_test(NAME EmergencyStopUnityTest COMMAND test_emergency_stop_unity)

# Test properties
set_tests_properties(SafetySystemsTest PROPERTIES
    TIMEOUT 30
    LABELS "safety;unit"
)

set_tests_properties(BasicSafetyTest PROPERTIES
    TIMEOUT 30
    LABELS "basic;safety;unit"
)

set_tests_properties(EmergencyStopUnityTest PROPERTIES
    TIMEOUT 30
    LABELS "emergency-stop;safety;unit"
)

# Test summary
message(STATUS "=== Test Configuration ===")
message(STATUS "Unity source: ${UNITY_SOURCE_DIR}")
message(STATUS "Test unit dir: ${TEST_UNIT_DIR}")
message(STATUS "Test mocks dir: ${TEST_MOCKS_DIR}")
message(STATUS "Safety tests: enabled (existing files only)")
message(STATUS "Basic safety test: enabled")
message(STATUS "Emergency stop Unity test: enabled")
message(STATUS "===========================")
