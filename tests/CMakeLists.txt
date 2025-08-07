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

# Create test executables using local Unity (only for files that exist)
message(STATUS "Using local Unity testing framework - enabling unit tests")

# Helper function to conditionally add test executable
function(add_test_if_exists test_name test_source)
    if(EXISTS "${test_source}")
        message(STATUS "Adding test: ${test_name} (source: ${test_source})")
        add_executable(${test_name} ${test_source} ${ARGN})
        target_link_libraries(${test_name} unity)
        
        # Use host compilation for tests, not ARM cross-compilation
        target_compile_options(${test_name} PRIVATE
            # Remove ARM-specific flags and use host compiler settings
            -Wall -Wextra -Wpedantic -Wno-unused-parameter
            -fdata-sections -ffunction-sections
            -Og -g3 -ggdb -std=gnu11
        )
        
        target_compile_definitions(${test_name} PRIVATE
            # Keep STM32 definitions for compatibility but remove ARM-specific ones
            STM32H753xx
            USE_HAL_DRIVER
            UNITY_TESTING
            # Remove ARM-specific definitions that don't apply to host tests
        )
        add_test(NAME ${test_name} COMMAND ${test_name})
    else()
        message(STATUS "Skipping test: ${test_name} (source not found: ${test_source})")
    endif()
endfunction()

# HAL Abstracted Safety Tests (modern approach)
add_test_if_exists(test_safety_systems_hal_abstracted
    ${TEST_UNIT_DIR}/test_safety_systems_hal_abstracted.c
    ${TEST_MOCKS_DIR}/mock_hal_abstraction.c
    ${CMAKE_SOURCE_DIR}/src/safety/emergency_stop_abstracted.c
)

# Legacy tests (conditionally add if they exist)
add_test_if_exists(test_safety_systems
    ${TEST_UNIT_DIR}/test_safety_systems.c
    ${TEST_MOCKS_DIR}/mock_hal.c
)

add_test_if_exists(test_basic_safety
    ${TEST_UNIT_DIR}/test_basic_safety.c
    ${TEST_MOCKS_DIR}/mock_hal.c
)

add_test_if_exists(test_emergency_stop_unity
    ${TEST_UNIT_DIR}/test_emergency_stop_unity.c
    ${TEST_MOCKS_DIR}/mock_hal.c
)

# Enable CTest framework
enable_testing()

# Note: Individual tests are registered automatically by add_test_if_exists function

# Test summary
message(STATUS "=== Test Configuration ===")
message(STATUS "Unity source: ${UNITY_SOURCE_DIR}")
message(STATUS "Test unit dir: ${TEST_UNIT_DIR}")
message(STATUS "Test mocks dir: ${TEST_MOCKS_DIR}")
message(STATUS "Safety tests: enabled (existing files only)")
message(STATUS "Basic safety test: enabled")
message(STATUS "Emergency stop Unity test: enabled")
message(STATUS "===========================")
