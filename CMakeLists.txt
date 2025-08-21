
cmake_minimum_required(VERSION 3.15)
message(STATUS "=== [DEBUG] CMake configure started ===")

# Set CMake policies to suppress warnings
if(POLICY CMP0109)
    cmake_policy(SET CMP0109 NEW)  # find_program() requires permission to execute but not to read
endif()

# STM32H753ZI Motor Control Project
project(stm32h753_ihm02a1
    VERSION 1.0.0
    DESCRIPTION "STM32H753ZI stepper motor control with L6470 drivers and AS5600 encoders"
    LANGUAGES C ASM
)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# STM32H753ZI specific definitions
add_compile_definitions(
    STM32H753xx
    USE_HAL_DRIVER
    ARM_MATH_CM7
    PREFETCH_ENABLE=1
    INSTRUCTION_CACHE_ENABLE=1
    DATA_CACHE_ENABLE=1
    SIMULATION_MODE=1
    FIRMWARE_BUILD=1
)

# Option to enable legacy (unprefixed) generated register names globally.
# Default OFF. Use -DUSE_LEGACY_REG_NAMES=ON to enable for a transitional build.
option(USE_LEGACY_REG_NAMES "Enable legacy unprefixed generated register names" OFF)
if(USE_LEGACY_REG_NAMES)
    message(STATUS "Legacy generated register names enabled via CMake option")
    add_compile_definitions(ENABLE_LEGACY_REG_NAMES=1)
endif()

# Compiler flags for STM32H753ZI (Cortex-M7)

# Only add GCC/Clang-specific flags for those compilers
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
# Debug: Print SOURCES list to verify inclusion of safety modules
message(STATUS "SOURCES list:")
foreach(src ${SOURCES})
    message(STATUS "  ${src}")
endforeach()
    add_compile_options(
        -mcpu=cortex-m7
        -mthumb
        -mfpu=fpv5-d16
        -mfloat-abi=hard
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wnull-dereference
        -Wjump-misses-init
        -Wdouble-promotion
        -Wshadow
        -fdata-sections
        -ffunction-sections
        -Og
        -g3
        -ggdb
    )
endif()

# Enable testing and add additional options for analysis builds
option(ENABLE_TESTING "Enable unit testing" ON)
option(ENABLE_COVERAGE "Enable code coverage analysis" OFF)
option(ENABLE_STATIC_ANALYSIS "Enable static analysis tools" OFF)
option(BUILD_DOCUMENTATION "Build documentation with Doxygen" OFF)
option(BUILD_UART_DMA_TEST "Build a firmware variant that enables the UART DMA device test (TEST_UART_DMA)" OFF)

# Safe-mode: disable motor power outputs for bring-up
option(SAFE_NO_MOTOR_POWER "Build firmware with motor outputs disabled for safe hardware bring-up" ON)
if(SAFE_NO_MOTOR_POWER)
    add_compile_definitions(SAFE_NO_MOTOR_POWER=1)
    message(STATUS "SAFE_NO_MOTOR_POWER is ON - motor outputs will be disabled in this build")
endif()

# Find Python3 early for STATUS.md automation and other tools
# Use explicit Windows path for virtual environment
# CRITICAL: Clear cache first to override any old Linux paths
unset(PYTHON3_EXE CACHE)
message(STATUS "DEBUG: WIN32=${WIN32}")
message(STATUS "DEBUG: CMAKE_HOST_WIN32=${CMAKE_HOST_WIN32}")
message(STATUS "DEBUG: CMAKE_HOST_SYSTEM_NAME=${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "DEBUG: CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")
if(CMAKE_HOST_WIN32)
    message(STATUS "DEBUG: Using CMAKE_HOST_WIN32 for detection")
    # Windows: Use explicit path to working virtual environment Python
    set(PYTHON3_EXE "${CMAKE_SOURCE_DIR}/.venv/Scripts/python.exe")
    message(STATUS "DEBUG: Set PYTHON3_EXE to: ${PYTHON3_EXE}")
    message(STATUS "DEBUG: File exists check: ${CMAKE_SOURCE_DIR}/.venv/Scripts/python.exe")
    if(EXISTS "${CMAKE_SOURCE_DIR}/.venv/Scripts/python.exe")
        message(STATUS "DEBUG: Windows Python exists - using it")
    else()
        message(STATUS "DEBUG: Windows Python NOT found - using fallback")
        # Fallback to system Python if venv not found
        find_program(PYTHON3_EXE NAMES 
            "python.exe"
            "python3.exe"
            "python"
        )
    endif()
else()
    # Linux/macOS: Use standard virtual environment paths
    find_program(PYTHON3_EXE NAMES 
        "${CMAKE_SOURCE_DIR}/.venv/bin/python3"
        "${CMAKE_SOURCE_DIR}/.venv/bin/python"
        "python3"
        "python"
    )
endif()
if(PYTHON3_EXE)
    message(STATUS "python3 found for automation: ${PYTHON3_EXE}")
else()
    message(STATUS "Python3 not found - automation features disabled")
endif()

# Set build type specific flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG=1)
    add_compile_options(-DDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(NDEBUG=1)
    add_compile_options(-O2 -flto)
    set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Code coverage flags
if(ENABLE_COVERAGE)
    add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
endif()

# Include directories (SSOT configuration)
# CRITICAL: Core/Inc must come BEFORE HAL driver includes to ensure correct hal_conf.h
include_directories(
    src
    src/config
    src/common
    src/drivers
    src/controllers
    src/safety
    src/communication
    src/application
    src/optimization
    src/hal_abstraction
    Core/Inc                                            # MUST be first - contains correct hal_conf.h
    Drivers/STM32H7xx_HAL_Driver/Inc                    # Match source directory case
    Drivers/STM32H7xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32H7xx/Include
    Drivers/CMSIS/Include
    Drivers/BSP/STM32H7xx_Nucleo
    00_reference/stm32h7xx_nucleo_bsp_md
    Middlewares/Third_Party/FreeRTOS/Source/include
    Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1
    Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
)

# Add FreeRTOS compile definitions
add_compile_definitions(
    USE_FREERTOS=1
)

# Auto-generated register headers from schemas
add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/src/config/l6470_registers_generated.h
    COMMAND python ${CMAKE_SOURCE_DIR}/scripts/schema_generator.py 
            ${CMAKE_SOURCE_DIR}/schemas/l6470_registers.yaml
    DEPENDS ${CMAKE_SOURCE_DIR}/schemas/l6470_registers.yaml
            ${CMAKE_SOURCE_DIR}/scripts/schema_generator.py
    COMMENT "Generating L6470 register headers from schema"
    VERBATIM
)

# -------------------------------------------------------------------------
# Workspace config generator (configure-time)
# Run scripts/generate_workspace_config.py during CMake configure to produce
# a generated header that reflects the current workspace/toolchain settings.
# Output is placed into the binary dir to avoid committing generated files.
# This follows the project's SSOT practice: generated overlay -> src/config
# but stored in the build tree to make builds reproducible and avoid source
# contamination.
# -------------------------------------------------------------------------
set(WORKSPACE_CONFIG_OUTPUT "${CMAKE_BINARY_DIR}/generated/include/workspace_config.generated.h")
add_custom_command(
    OUTPUT ${WORKSPACE_CONFIG_OUTPUT}
    COMMAND ${PYTHON3_EXE} ${CMAKE_SOURCE_DIR}/scripts/generate_workspace_config.py --input ${CMAKE_SOURCE_DIR}/config/workflow_toolchain.json --output ${WORKSPACE_CONFIG_OUTPUT}
    DEPENDS ${CMAKE_SOURCE_DIR}/scripts/generate_workspace_config.py ${CMAKE_SOURCE_DIR}/config/workflow_toolchain.json
    COMMENT "Generating workspace_config.generated.h into build/generated/include"
    VERBATIM
)

add_custom_target(generate-workspace-config
    DEPENDS ${WORKSPACE_CONFIG_OUTPUT}
    COMMENT "Generate workspace config header"
)

# Validate generated overlay presence and expected macros
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/generated/include/workspace_config.validated
    COMMAND ${PYTHON3_EXE} ${CMAKE_SOURCE_DIR}/scripts/validate_generated_overlay.py --file ${WORKSPACE_CONFIG_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/generated/include/workspace_config.validated
    DEPENDS ${WORKSPACE_CONFIG_OUTPUT} ${CMAKE_SOURCE_DIR}/scripts/validate_generated_overlay.py
    COMMENT "Validating generated workspace overlay"
    VERBATIM
)
add_custom_target(validate-generated-overlay
    DEPENDS ${CMAKE_BINARY_DIR}/generated/include/workspace_config.validated
    COMMENT "Validate generated overlay header"
)

# Define a variable for the generated include directory so we can add it
# to specific targets rather than globally. Target-scoped includes are safer
# and follow industrial best-practice.
set(GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated/include")

# Export the top-level binary dir so child CMakeLists (host_tests) can
# reliably locate the generated include directory. Using a CACHE variable
# makes this explicit and robust when the subdirectory is configured.
set(PARENT_CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR} CACHE PATH "Top-level binary dir")


add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/src/config/as5600_registers_generated.h
    COMMAND python ${CMAKE_SOURCE_DIR}/scripts/schema_generator.py 
            ${CMAKE_SOURCE_DIR}/schemas/as5600_registers.yaml
    DEPENDS ${CMAKE_SOURCE_DIR}/schemas/as5600_registers.yaml
            ${CMAKE_SOURCE_DIR}/scripts/schema_generator.py
    COMMENT "Generating AS5600 register headers from schema"
    VERBATIM
)

# Create target for schema generation
add_custom_target(generate_schemas
    DEPENDS ${CMAKE_SOURCE_DIR}/src/config/l6470_registers_generated.h
            ${CMAKE_SOURCE_DIR}/src/config/as5600_registers_generated.h
    COMMENT "Generating all register schema headers"
)

# Source files

# Debug: Print working directory and source dir
message(STATUS "[DEBUG] CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "[DEBUG] CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")

file(GLOB_RECURSE SOURCES
    "src/hal_abstraction/hal_abstraction_stm32h7.c"
    "Core/Src/gpio.c"
    "Core/Src/freertos.c"
    "src/drivers/l6470/*.c"
    "src/drivers/as5600/*.c"
    "src/controllers/*.c"
    "src/safety/*.c"
    "src/communication/*.c"
    "src/config/hardware_detection.c"
    "src/rtos/*.c"
    "src/telemetry/*.c"
    "src/optimization/*.c"
    # "src/simulation/*.c"
    "Drivers/STM32H7xx_HAL_Driver/Src/*.c"
    # "drivers/BSP/STM32H7xx_Nucleo/*.c"
    "Core/Src/stm32h7xx_hal_msp.c"
    "Core/Src/stm32h7xx_it.c"
    "Core/Src/system_stm32h7xx.c"
    "Core/Src/stm32h7xx_hal_timebase_tim.c"
    "Core/Src/main.c"
    # "src/application/uart_test.c"
    "src/enhanced_clock_config.c"
    "src/config/clock_config.c"
    "src/config/clock_config_480mhz.c"
    "Core/Src/syscalls.c"
)

# Debug: Print number of files found for each pattern
file(GLOB SAFETY_SOURCES "src/safety/*.c")
list(LENGTH SAFETY_SOURCES SAFETY_COUNT)
message(STATUS "[DEBUG] SAFETY_SOURCES count: ${SAFETY_COUNT}")
foreach(f ${SAFETY_SOURCES})
    message(STATUS "[DEBUG] SAFETY_SOURCE: ${f}")
endforeach()

list(LENGTH SOURCES SOURCES_COUNT)
message(STATUS "[DEBUG] Total SOURCES count: ${SOURCES_COUNT}")

# Debug: Print SOURCES list to verify inclusion of safety modules
message(STATUS "[DEBUG] SOURCES list:")
foreach(src ${SOURCES})
    message(STATUS "[DEBUG]   ${src}")
endforeach()

# Add FreeRTOS source files when using RTOS
file(GLOB FREERTOS_SOURCES
    "Middlewares/Third_Party/FreeRTOS/Source/*.c"
    "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/*.c"
    "Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c"
    "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c"
)

list(APPEND SOURCES ${FREERTOS_SOURCES})

# Remove template files that are not needed for basic build
list(FILTER SOURCES EXCLUDE REGEX ".*timebase_rtc.*template.*")
list(FILTER SOURCES EXCLUDE REGEX ".*hal_msp_template.*")
list(FILTER SOURCES EXCLUDE REGEX ".*timebase_tim_template.*")

# Exclude legacy files in favor of HAL abstraction versions
# (Do not exclude emergency_stop.c; use real safety logic)
list(FILTER SOURCES EXCLUDE REGEX ".*hardware_simulation\\.c$")  # Use motor_simulation.c instead (newer simulation framework)

# Use the correct startup file for GCC
set(STARTUP_FILE "${CMAKE_SOURCE_DIR}/startup_stm32h753xx.s")

# Create main executable
add_executable(${PROJECT_NAME} ${SOURCES} ${STARTUP_FILE})

if(BUILD_UART_DMA_TEST)
    message(STATUS "Building UART DMA device test variant")
    # Create a test variant target that adds TEST_UART_DMA and the test source
    set(TEST_TARGET ${PROJECT_NAME}_uart_dma_test)
    add_executable(${TEST_TARGET} ${SOURCES} ${CMAKE_SOURCE_DIR}/src/tests/uart_dma_device_test.c ${STARTUP_FILE})
    target_compile_definitions(${TEST_TARGET} PRIVATE TEST_UART_DMA=1)
    target_include_directories(${TEST_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/src/tests)

    if(EXISTS ${GENERATED_INCLUDE_DIR})
        target_include_directories(${TEST_TARGET} BEFORE PRIVATE ${GENERATED_INCLUDE_DIR})
    else()
        file(MAKE_DIRECTORY ${GENERATED_INCLUDE_DIR})
        target_include_directories(${TEST_TARGET} BEFORE PRIVATE ${GENERATED_INCLUDE_DIR})
    endif()

    # Post-build: generate binary/hex for test target
    add_custom_command(TARGET ${TEST_TARGET} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TEST_TARGET}> ${CMAKE_BINARY_DIR}/${TEST_TARGET}.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${TEST_TARGET}> ${CMAKE_BINARY_DIR}/${TEST_TARGET}.hex
        COMMENT "Building binary and hex files for UART DMA test target"
    )

    # Add a CTest test that runs the host-side Python script if Python is available.
    if(PYTHON3_EXE)
        enable_testing()
        add_test(NAME uart_dma_host_run
            COMMAND ${PYTHON3_EXE} ${CMAKE_SOURCE_DIR}/scripts/uart_dma_host_test.py --port COM5 --baud 115200 --size 512 --iters 4
        )
        set_tests_properties(uart_dma_host_run PROPERTIES TIMEOUT 30)
    endif()
endif()

# Add generated include dir to the main target only (target-scoped include)
if(EXISTS ${GENERATED_INCLUDE_DIR})
    target_include_directories(${PROJECT_NAME} BEFORE PRIVATE ${GENERATED_INCLUDE_DIR})
else()
    # Ensure the directory is created at configure/build time when needed
    file(MAKE_DIRECTORY ${GENERATED_INCLUDE_DIR})
    target_include_directories(${PROJECT_NAME} BEFORE PRIVATE ${GENERATED_INCLUDE_DIR})
endif()

# Set target properties
if(NOT CMAKE_HOST_WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "-Wl,--gc-sections -Wl,--print-memory-usage -Wl,-Map=\"${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map\" -lc -lm"
    )

    # Link libraries (math) only for non-Windows hosts / cross toolchains
    target_link_libraries(${PROJECT_NAME} 
        m  # Math library
    )
else()
    # On Windows host builds, avoid adding -lm (it becomes m.lib and breaks lld-link).
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "-Wl,--gc-sections -Wl,--print-memory-usage -Wl,-Map=\"${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map\" -lc"
    )
endif()

# Create binary and hex files
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMENT "Building binary and hex files"
)

# Print size information
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
    COMMENT "Binary size information:"
)

# STATUS.md automation - Phase 2: Post-build status update
if(PYTHON3_EXE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${PYTHON3_EXE} ${CMAKE_SOURCE_DIR}/scripts/auto_update_status.py --source build --workspace ${CMAKE_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Updating STATUS.md with build metrics"
        VERBATIM
    )
else()
    message(STATUS "Python3 not found - STATUS.md automation disabled")
endif()

# STATUS.md automation target for manual use
if(PYTHON3_EXE)
    add_custom_target(update-status
        COMMAND ${PYTHON3_EXE} ${CMAKE_SOURCE_DIR}/scripts/auto_update_status.py --verbose
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Manually updating STATUS.md"
        VERBATIM
    )
    
    add_custom_target(update-status-preview
        COMMAND ${PYTHON3_EXE} ${CMAKE_SOURCE_DIR}/scripts/auto_update_status.py --dry-run --verbose
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Preview STATUS.md updates"
        VERBATIM
    )
endif()

# Add subdirectories for examples (optional)
if(EXISTS "${CMAKE_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
endif()

# ============================================================================
# TESTING AND ANALYSIS FRAMEWORK
# ============================================================================

# Testing framework setup
if(ENABLE_TESTING)
    enable_testing()
    include(CTest)
    
    # Add tests directory if it exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    endif()
endif()

# Static Analysis Tools
if(ENABLE_STATIC_ANALYSIS)
    # Find clang-tidy
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        set(CLANG_TIDY_COMMAND 
            ${CLANG_TIDY_EXE}
            -checks=*,-modernize-*,-fuchsia-*,-google-*,-zircon-*,-readability-magic-numbers,-cppcoreguidelines-avoid-magic-numbers
            -header-filter=.*
            -format-style=file
        )
        
        # Apply clang-tidy to main target
        set_target_properties(${PROJECT_NAME} PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}"
            C_CLANG_TIDY "${CLANG_TIDY_COMMAND}"
        )
        
        # Create standalone static analysis target
        add_custom_target(static-analysis
            COMMAND ${CLANG_TIDY_EXE} 
            -checks=*,-modernize-*,-fuchsia-*,-google-*,-zircon-*,-readability-magic-numbers,-cppcoreguidelines-avoid-magic-numbers
            -header-filter=.*
            src/safety/*.c src/controllers/*.c src/communication/*.c src/drivers/*/*.c
            --
            -Isrc -Isrc/config -Isrc/common -Isrc/drivers -Isrc/controllers -Isrc/safety -Isrc/communication
            COMMENT "Running clang-tidy static analysis"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endif()
    
    # Find cppcheck
    find_program(CPPCHECK_EXE NAMES "cppcheck")
    if(CPPCHECK_EXE)
        add_custom_target(cppcheck
            COMMAND ${CPPCHECK_EXE}
            --enable=all
            --std=c11
            --platform=unix32
            --check-config
            --suppressions-list=${CMAKE_SOURCE_DIR}/.cppcheck-suppressions
            --xml
            --xml-version=2
            --output-file=${CMAKE_BINARY_DIR}/cppcheck-report.xml
            src/
            COMMENT "Running cppcheck static analysis"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endif()
endif()

# Code Coverage Analysis
if(ENABLE_COVERAGE)
    find_program(GCOV_EXE NAMES "gcov")
    find_program(LCOV_EXE NAMES "lcov")
    find_program(GENHTML_EXE NAMES "genhtml")
    
    if(GCOV_EXE AND LCOV_EXE AND GENHTML_EXE)
        # Add coverage target
        add_custom_target(coverage
            COMMAND ${LCOV_EXE} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV_EXE} --remove coverage.info '/usr/*' --output-file coverage.info
            COMMAND ${LCOV_EXE} --remove coverage.info '*/tests/*' --output-file coverage.info
            COMMAND ${LCOV_EXE} --remove coverage.info '*/00_reference/*' --output-file coverage.info
            COMMAND ${GENHTML_EXE} coverage.info --output-directory coverage_html
            COMMENT "Generating code coverage report"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        
        # Coverage must run after tests
        if(TARGET test_safety_systems)
            add_dependencies(coverage test_safety_systems)
        endif()
    endif()
endif()

# Documentation Generation
if(BUILD_DOCUMENTATION)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        # Set Doxygen configuration
        set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/docs/Doxyfile)
        
        # Check if Doxyfile.in exists, otherwise use existing Doxyfile
        if(EXISTS ${DOXYGEN_IN})
            configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        else()
            set(DOXYGEN_OUT ${CMAKE_SOURCE_DIR}/docs/Doxyfile)
        endif()
        
        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
        )
    endif()
endif()

# ============================================================================
# UTILITY TARGETS
# ============================================================================

# Format code target
find_program(CLANG_FORMAT_EXE NAMES "clang-format")
if(CLANG_FORMAT_EXE)
    add_custom_target(format
        COMMAND find src -name '*.c' -o -name '*.h' | xargs ${CLANG_FORMAT_EXE} -i
        COMMENT "Formatting source code with clang-format"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

# SSOT validation target - Use pre-found Python
if(PYTHON3_EXE AND EXISTS "${CMAKE_SOURCE_DIR}/scripts/validate_ssot.py")
    # Ensure the generated overlay is validated before running the SSOT
    # validator so we fail early on missing/invalid generated headers.
    add_custom_target(validate-ssot
        DEPENDS validate-generated-overlay
        COMMAND ${PYTHON3_EXE} scripts/validate_ssot.py
        COMMENT "Validating SSOT configuration (uses generated workspace_config)"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

# Combined analysis target
add_custom_target(analyze
    COMMENT "Running all analysis tools"
)

if(TARGET static-analysis)
    add_dependencies(analyze static-analysis)
endif()
if(TARGET cppcheck)
    add_dependencies(analyze cppcheck)
endif()
if(TARGET validate-ssot)
    add_dependencies(analyze validate-ssot)
endif()

# All target for comprehensive build
add_custom_target(all-enhanced
    DEPENDS ${PROJECT_NAME}
    COMMENT "Building project with all enhancements"
)

if(TARGET analyze)
    add_dependencies(all-enhanced analyze)
endif()
if(BUILD_DOCUMENTATION AND TARGET docs)
    add_dependencies(all-enhanced docs)
endif()

# Print configuration summary
message(STATUS "=== STM32H753ZI Motor Control Project Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Testing enabled: ${ENABLE_TESTING}")
message(STATUS "Coverage enabled: ${ENABLE_COVERAGE}")
message(STATUS "Static analysis enabled: ${ENABLE_STATIC_ANALYSIS}")
message(STATUS "Documentation enabled: ${BUILD_DOCUMENTATION}")
if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
endif()
if(CPPCHECK_EXE)
    message(STATUS "cppcheck found: ${CPPCHECK_EXE}")
endif()
if(PYTHON3_EXE)
    message(STATUS "python3 found: ${PYTHON3_EXE}")
endif()
message(STATUS "=======================================================")

# Safety-critical project warnings
message(WARNING "This is a safety-critical motor control system.")
message(WARNING "Always run comprehensive tests before deployment.")
message(WARNING "Validate all safety systems before connecting hardware.")
