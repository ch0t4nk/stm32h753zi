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

# Basic HAL Abstraction Test (demonstrates core concepts)
add_executable(test_hal_abstraction_basic_corrected
    ${CMAKE_SOURCE_DIR}/test_hal_abstraction_basic_corrected.c
    ${CMAKE_SOURCE_DIR}/../../src/hal_abstraction/hal_abstraction_mock.c
)

target_link_libraries(test_hal_abstraction_basic_corrected unity)

target_compile_definitions(test_hal_abstraction_basic_corrected PRIVATE
    UNITY_TESTING=1
    HOST_TESTING=1
)

target_include_directories(test_hal_abstraction_basic_corrected PRIVATE
    ${CMAKE_SOURCE_DIR}/../../src/hal_abstraction
    ${CMAKE_SOURCE_DIR}/../../src/safety
)

# Add tests to CTest
add_test(NAME HALAbstractionBasicTest COMMAND test_hal_abstraction_basic_corrected)
set_tests_properties(HALAbstractionBasicTest PROPERTIES
    TIMEOUT 30
    LABELS "hal-abstraction;basic;architecture"
)

# Print configuration
message(STATUS "=== Host Testing Configuration ===")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "Unity library: ENABLED")
message(STATUS "===================================")
