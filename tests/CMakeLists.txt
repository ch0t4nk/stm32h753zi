cmake_minimum_required(VERSION 3.15)

# Test configuration
project(stm32h753_tests LANGUAGES C)

# Find Unity testing framework
find_package(PkgConfig QUIET)

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
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/src/safety
    ${CMAKE_SOURCE_DIR}/src/controllers
    ${CMAKE_SOURCE_DIR}/src/communication
    ${CMAKE_SOURCE_DIR}/src/drivers
)

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

# Create test executables if Unity is available
if(PkgConfig_FOUND)
    pkg_check_modules(UNITY unity)
    if(UNITY_FOUND)
        message(STATUS "Unity testing framework found - enabling unit tests")
        
        # Safety systems test suite
        add_executable(test_safety_systems ${SAFETY_TEST_SOURCES})
        target_link_libraries(test_safety_systems ${UNITY_LIBRARIES})
        target_include_directories(test_safety_systems PRIVATE ${UNITY_INCLUDE_DIRS})
        
        # Register tests
        add_test(NAME SafetySystemsUnitTests COMMAND test_safety_systems)
        add_test(NAME EmergencyStopTests COMMAND test_safety_systems --filter emergency_stop)
        add_test(NAME WatchdogTests COMMAND test_safety_systems --filter watchdog)
        add_test(NAME FaultMonitorTests COMMAND test_safety_systems --filter fault_monitor)
        
        # Test properties
        set_tests_properties(SafetySystemsUnitTests PROPERTIES
            TIMEOUT 30
            LABELS "safety;unit"
        )
        
    else()
        message(WARNING "Unity testing framework not found - unit tests disabled")
        message(STATUS "Install Unity with: sudo apt-get install libunity-dev")
    endif()
else()
    message(WARNING "PkgConfig not found - cannot detect Unity testing framework")
endif()

# Alternative: GoogleTest if Unity is not available
find_package(GTest QUIET)
if(GTest_FOUND AND NOT UNITY_FOUND)
    message(STATUS "GoogleTest found - enabling C++ unit tests")
    
    # Note: GoogleTest is primarily for C++, but can test C code
    add_executable(test_safety_gtest
        ${TEST_UNIT_DIR}/test_safety_gtest.cpp
        ${CMAKE_SOURCE_DIR}/src/safety/safety_system.c
        ${CMAKE_SOURCE_DIR}/src/safety/emergency_stop.c
        ${TEST_MOCKS_DIR}/mock_hal.c
    )
    target_link_libraries(test_safety_gtest GTest::GTest GTest::Main)
    add_test(NAME SafetySystemsGTest COMMAND test_safety_gtest)
endif()

# Integration tests (basic structure)
add_custom_target(integration-tests
    COMMAND echo "Integration tests placeholder - requires hardware setup"
    COMMENT "Running integration tests"
)

# Test data and fixtures
configure_file(
    ${TEST_FIXTURES_DIR}/test_config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/test_config.h
    @ONLY
)

# Test summary target
add_custom_target(test-summary
    COMMAND echo "=== Test Summary ==="
    COMMAND echo "Unit tests: Available with Unity or GoogleTest"
    COMMAND echo "Integration tests: Manual execution required"
    COMMAND echo "Hardware tests: Requires connected STM32H753ZI"
    COMMAND echo "==================="
    COMMENT "Test configuration summary"
)

# Basic test without external dependencies
add_executable(test_basic_safety
    ${TEST_UNIT_DIR}/test_basic_safety.c
    ${TEST_MOCKS_DIR}/mock_hal.c
)

target_include_directories(test_basic_safety PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/tests
    ${CMAKE_SOURCE_DIR}/tests/mocks
)

target_compile_definitions(test_basic_safety PRIVATE
    MAX_MOTORS=2
    UNIT_TESTING=1
    STM32H753xx
)

# Add basic test to CTest
add_test(NAME BasicSafetyTest COMMAND test_basic_safety)
set_tests_properties(BasicSafetyTest PROPERTIES
    TIMEOUT 10
    LABELS "basic;safety"
)

# Unity-based tests using local Unity installation
if(EXISTS "${CMAKE_SOURCE_DIR}/external/unity/unity.h")
    message(STATUS "Local Unity testing framework found - enabling Unity tests")
    
    # Create Unity library
    add_library(unity STATIC
        ${CMAKE_SOURCE_DIR}/external/unity/unity.c
    )
    
    target_include_directories(unity PUBLIC
        ${CMAKE_SOURCE_DIR}/external/unity
    )
    
    # Unity-based emergency stop test (host compiler for faster testing)
    add_executable(test_emergency_stop_unity
        ${TEST_UNIT_DIR}/test_emergency_stop_unity.c
        ${TEST_MOCKS_DIR}/mock_hal.c
    )
    
    # Use host compiler for tests (not ARM cross-compiler)
    set_target_properties(test_emergency_stop_unity PROPERTIES
        C_COMPILER "gcc"
        COMPILE_FLAGS "-DUNIT_TESTING=1 -DHOST_TESTING=1"
    )
    
    target_include_directories(test_emergency_stop_unity PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/config
        ${CMAKE_SOURCE_DIR}/tests
        ${CMAKE_SOURCE_DIR}/tests/mocks
        ${CMAKE_SOURCE_DIR}/external/unity
    )
    
    target_link_libraries(test_emergency_stop_unity unity)
    
    # Add Unity test to CTest
    add_test(NAME UnityEmergencyStopTest COMMAND test_emergency_stop_unity)
    set_tests_properties(UnityEmergencyStopTest PROPERTIES
        TIMEOUT 30
        LABELS "unity;emergency-stop;safety"
    )
    
else()
    message(WARNING "Local Unity framework not found at external/unity/")
    message(STATUS "Unity tests disabled - only basic tests available")
endif()

# Print test configuration
message(STATUS "=== Test Configuration ===")
message(STATUS "Unity available: ${UNITY_FOUND}")
message(STATUS "GoogleTest available: ${GTest_FOUND}")
message(STATUS "Test unit dir: ${TEST_UNIT_DIR}")
message(STATUS "Test mocks dir: ${TEST_MOCKS_DIR}")
message(STATUS "===========================")
