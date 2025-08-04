# CMake toolchain file for ARM Embedded GCC
# STM32H753ZI Stepper Motor Control Project

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Specify the cross compiler
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(CMAKE_SIZE arm-none-eabi-size)
set(CMAKE_DEBUGGER arm-none-eabi-gdb)

# STM32H753ZI specific settings
set(MCU_FAMILY STM32H7xx)
set(MCU_MODEL STM32H753xx)
set(CPU_PARAMETERS "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard")

set(STARTUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/startup/startup_stm32h753xx.s)
set(MCU_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/ldscripts/STM32H753ZITx_FLASH.ld)

# Compiler-specific options
set(CMAKE_C_FLAGS "${CPU_PARAMETERS} -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wjump-misses-init -Wdouble-promotion -Wshadow -fdata-sections -ffunction-sections")
set(CMAKE_CXX_FLAGS "${CPU_PARAMETERS} -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wdouble-promotion -Wshadow -fdata-sections -ffunction-sections")
set(CMAKE_ASM_FLAGS "${CPU_PARAMETERS} -x assembler-with-cpp")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS "${CPU_PARAMETERS} --specs=nano.specs -lc -lm -lnosys -Wl,--gc-sections")

# Debug/Release specific flags
set(CMAKE_C_FLAGS_DEBUG "-Og -g3 -ggdb")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g3 -ggdb")
set(CMAKE_C_FLAGS_RELEASE "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG")

# Disable test compilation for cross-compilation
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# STM32H7 HAL and CMSIS definitions
add_definitions(
    -DSTM32H753xx
    -DUSE_HAL_DRIVER
    -DHSE_VALUE=8000000
    -DLSE_VALUE=32768
    -DVDD_VALUE=3300
    -DPREFETCH_ENABLE=1
    -DINSTRUCTION_CACHE_ENABLE=1
    -DDATA_CACHE_ENABLE=1
)

# Set the file extension for executables
set(CMAKE_EXECUTABLE_SUFFIX_C .elf)
set(CMAKE_EXECUTABLE_SUFFIX_CXX .elf)

# Custom target for generating HEX files
function(add_hex_target target_name)
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${target_name}> ${target_name}.hex
        COMMENT "Generating HEX file for ${target_name}")
    
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${target_name}> ${target_name}.bin
        COMMENT "Generating BIN file for ${target_name}")
    
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${target_name}>
        COMMENT "Size information for ${target_name}")
endfunction()
