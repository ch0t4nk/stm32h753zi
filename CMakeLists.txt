cmake_minimum_required(VERSION 3.15)

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
    HSE_VALUE=8000000
    LSE_VALUE=32768
    VDD_VALUE=3300
    PREFETCH_ENABLE=1
    INSTRUCTION_CACHE_ENABLE=1
    DATA_CACHE_ENABLE=1
)

# Compiler flags for STM32H753ZI (Cortex-M7)
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

# Include directories (SSOT configuration)
include_directories(
    src/config
    src/common
    Core/Inc
    drivers/STM32H7xx_HAL_Driver/Inc
    drivers/STM32H7xx_HAL_Driver/Inc/Legacy
    drivers/CMSIS/Device/ST/STM32H7xx/Include
    drivers/CMSIS/Include
    00_reference/stm32h7xx_nucleo_bsp_md
)

# Source files
file(GLOB_RECURSE SOURCES
    "src/common/*.c"
    "src/application/*.c"
    "drivers/STM32H7xx_HAL_Driver/Src/*.c"
    "Core/Src/stm32h7xx_hal_msp.c"
    "Core/Src/stm32h7xx_it.c"
    "Core/Src/system_stm32h7xx.c"
)

# Remove template files that are not needed for basic build
list(FILTER SOURCES EXCLUDE REGEX ".*timebase_rtc.*template.*")
list(FILTER SOURCES EXCLUDE REGEX ".*hal_msp_template.*")

# Use the correct startup file for GCC
set(STARTUP_FILE "${CMAKE_SOURCE_DIR}/startup_stm32h753xx.s")

# Create main executable
add_executable(${PROJECT_NAME} ${SOURCES} ${STARTUP_FILE})

# Set the linker script
set_target_properties(${PROJECT_NAME} PROPERTIES
    LINK_FLAGS "-T${CMAKE_SOURCE_DIR}/STM32H753XX_FLASH.ld -Wl,--gc-sections -Wl,--print-memory-usage -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map -lc -lm -lnosys"
)

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    m  # Math library
)

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

# Add subdirectories for examples (optional)
if(EXISTS "${CMAKE_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
endif()

# Add tests (optional)
if(EXISTS "${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()
