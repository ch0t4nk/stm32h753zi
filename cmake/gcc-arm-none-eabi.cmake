# CMake toolchain for ARM GCC cross-compilation  
# SSOT-based configuration from src/config/workspace_config.h
# References STM32CubeCLT 1.19.0 toolchain paths

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_C_COMPILER_ID GNU)
set(CMAKE_CXX_COMPILER_ID GNU)

# SSOT: Toolchain paths from workspace_config.h
# STM32CUBECLT_VERSION = "1.19.0"
# ARM_TOOLCHAIN_BASE_PATH = "C:/ST/STM32CubeCLT_1.19.0/GNU-tools-for-STM32/bin"
set(ARM_TOOLCHAIN_DIR "C:/ST/STM32CubeCLT_1.19.0/GNU-tools-for-STM32/bin")

# SSOT: Toolchain executables from workspace_config.h
set(CMAKE_C_COMPILER   "${ARM_TOOLCHAIN_DIR}/arm-none-eabi-gcc.exe")
set(CMAKE_CXX_COMPILER "${ARM_TOOLCHAIN_DIR}/arm-none-eabi-g++.exe")
set(CMAKE_ASM_COMPILER "${ARM_TOOLCHAIN_DIR}/arm-none-eabi-gcc.exe")
set(CMAKE_AR           "${ARM_TOOLCHAIN_DIR}/arm-none-eabi-ar.exe")
set(CMAKE_OBJCOPY      "${ARM_TOOLCHAIN_DIR}/arm-none-eabi-objcopy.exe")
set(CMAKE_SIZE         "${ARM_TOOLCHAIN_DIR}/arm-none-eabi-size.exe")
set(CMAKE_OBJDUMP      "${ARM_TOOLCHAIN_DIR}/arm-none-eabi-objdump.exe")

# Executable suffixes for embedded target
set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# SSOT: Target flags from workspace_config.h
# TARGET_CPU = "cortex-m7", TARGET_FPU = "fpv5-d16", TARGET_FLOAT_ABI = "hard"
set(TARGET_FLAGS "-mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard")

# SSOT: Compiler flags based on workspace_config.h TARGET_CFLAGS
set(CMAKE_C_FLAGS "${TARGET_FLAGS} -Wall -fdata-sections -ffunction-sections")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp -MMD -MP")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics")

# Build configuration specific flags
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-Os -g0")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -g0")

# SSOT: Linker configuration based on workspace_config.h TARGET_LDFLAGS and LINKER_SCRIPT
set(CMAKE_EXE_LINKER_FLAGS "${TARGET_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nano.specs --specs=nosys.specs")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T \"${CMAKE_SOURCE_DIR}/STM32H753XX_FLASH.ld\"")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--print-memory-usage")

# Standard libraries for embedded
set(TOOLCHAIN_LINK_LIBRARIES "c m")

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# Search for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
