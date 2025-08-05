# Host-based Testing CMakeLists.txt
# This builds tests for the host system (not ARM cross-compiled)

cmake_minimum_required(VERSION 3.16)
project(STM32H753ZI_HostTests C)

# Use host compiler
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_C_FLAGS "-Wall -Wextra -g -DUNIT_TESTING=1 -DHOST_TESTING=1")

# Enable testing
enable_testing()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/../../src
    ${CMAKE_SOURCE_DIR}/../../src/config
    ${CMAKE_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/../mocks
    ${CMAKE_SOURCE_DIR}/../../external/unity
)

# Unity library
add_library(unity STATIC
    ${CMAKE_SOURCE_DIR}/../../external/unity/unity.c
)

target_include_directories(unity PUBLIC
    ${CMAKE_SOURCE_DIR}/../../external/unity
)

# Unity-based emergency stop test
add_executable(test_emergency_stop_unity
    ${CMAKE_SOURCE_DIR}/../unit/test_emergency_stop_unity.c
    ${CMAKE_SOURCE_DIR}/../mocks/mock_hal.c
)

target_link_libraries(test_emergency_stop_unity unity)

# Add test to CTest
add_test(NAME UnityEmergencyStopTest COMMAND test_emergency_stop_unity)
set_tests_properties(UnityEmergencyStopTest PROPERTIES
    TIMEOUT 30
    LABELS "unity;emergency-stop;safety"
)

# Print configuration
message(STATUS "=== Host Testing Configuration ===")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "Unity library: ENABLED")
message(STATUS "===================================")
