cmake_minimum_required(VERSION 3.15)

# Host Testing Build Configuration
# This CMakeLists.txt is for host testing compilation using local GCC
# Part of the dual-build system - separate from ARM firmware compilation

project(stm32h753_host_tests LANGUAGES C)

# Host compilation uses system default gcc/g++ (no cross-compilation)
set(CMAKE_BUILD_TYPE Debug)

# Host-specific compiler flags
set(CMAKE_C_FLAGS 
    "-Wall -Wextra -Wpedantic -g -O0 -DSIMULATION_ENABLED=1 -DTEST_MODE=1 -DHOST_TESTING=1 -DUNITY_TESTING=1 -DSTM32H753xx -DUSE_HAL_DRIVER"
)

# Use local Unity testing framework
set(UNITY_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../external/unity)

# Test directories
set(TEST_UNIT_DIR ${CMAKE_SOURCE_DIR}/../tests/unit)
set(TEST_MOCKS_DIR ${CMAKE_SOURCE_DIR}/../tests/mocks)
set(TEST_FIXTURES_DIR ${CMAKE_SOURCE_DIR}/../tests/fixtures)

# Include directories for host testing
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${TEST_MOCKS_DIR}
    ${TEST_FIXTURES_DIR}
    ${UNITY_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../src
    ${CMAKE_SOURCE_DIR}/../src/config
    ${CMAKE_SOURCE_DIR}/../src/common
    ${CMAKE_SOURCE_DIR}/../src/safety
    ${CMAKE_SOURCE_DIR}/../src/controllers
    ${CMAKE_SOURCE_DIR}/../src/communication
    ${CMAKE_SOURCE_DIR}/../src/drivers
    ${CMAKE_SOURCE_DIR}/../src/hal_abstraction
    ${CMAKE_SOURCE_DIR}/../src/telemetry
)

# Create Unity library for host testing
add_library(unity_host STATIC ${UNITY_SOURCE_DIR}/unity.c)
target_include_directories(unity_host PUBLIC ${UNITY_SOURCE_DIR})

# Host testing helper function
function(add_host_test test_name test_source)
    if(EXISTS "${test_source}")
        message(STATUS "Adding host test: ${test_name} (source: ${test_source})")
        add_executable(${test_name} ${test_source} ${ARGN})
        target_link_libraries(${test_name} unity_host)
        
        # Host compilation flags (no ARM cross-compilation)
        target_compile_options(${test_name} PRIVATE
            -Wall -Wextra -Wpedantic -Wno-unused-parameter
            -fdata-sections -ffunction-sections
            -Og -g3 -ggdb -std=gnu11
        )
        
        # Host testing definitions
        target_compile_definitions(${test_name} PRIVATE
            STM32H753xx
            USE_HAL_DRIVER
            UNITY_TESTING
            HOST_TESTING=1
            SIMULATION_ENABLED=1
            TEST_MODE=1
        )
        
        # Link math library for mathematical functions
        target_link_libraries(${test_name} m)
        
        add_test(NAME ${test_name} COMMAND ${test_name})
    else()
        message(STATUS "Skipping host test: ${test_name} (source not found: ${test_source})")
    endif()
endfunction()

# Phase 1.3 Dynamic Motor Tuning Host Tests
add_host_test(test_phase1_3_validation_host
    ${TEST_UNIT_DIR}/test_phase1_3_validation.c
    ${TEST_MOCKS_DIR}/mock_hal_abstraction.c
)

# Full integration tests (commented out until dependencies are ready)
# add_host_test(test_optimization_telemetry_host
#     ${TEST_UNIT_DIR}/test_optimization_telemetry.c
#     ${TEST_MOCKS_DIR}/mock_hal_abstraction.c
#     ${CMAKE_SOURCE_DIR}/../src/telemetry/optimization_telemetry.c
# )

# add_host_test(test_motor_characterization_host
#     ${TEST_UNIT_DIR}/test_motor_characterization.c
#     ${TEST_MOCKS_DIR}/mock_hal_abstraction.c
#     ${CMAKE_SOURCE_DIR}/../src/controllers/motor_characterization.c
#     ${CMAKE_SOURCE_DIR}/../src/telemetry/optimization_telemetry.c
# )

# Legacy safety tests for host testing
add_host_test(test_safety_systems_hal_abstracted_host
    ${TEST_UNIT_DIR}/test_safety_systems_hal_abstracted.c
    ${TEST_MOCKS_DIR}/mock_hal_abstraction.c
    ${CMAKE_SOURCE_DIR}/../src/safety/emergency_stop_abstracted.c
)

# Enable CTest framework for host testing
enable_testing()

# Host testing summary
message(STATUS "=== Host Testing Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Unity source: ${UNITY_SOURCE_DIR}")
message(STATUS "Test unit dir: ${TEST_UNIT_DIR}")
message(STATUS "Test mocks dir: ${TEST_MOCKS_DIR}")
message(STATUS "Host testing: enabled")
message(STATUS "Simulation mode: enabled")
message(STATUS "Phase 1.3 tests: enabled")
message(STATUS "===================================")
