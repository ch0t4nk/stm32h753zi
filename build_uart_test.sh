#!/bin/bash
# Build script for STM32H753ZI UART test firmware

echo "🔧 Building STM32H753ZI UART Test Firmware"

# Set up paths
PROJECT_ROOT="/workspaces/code"
BUILD_DIR="$PROJECT_ROOT/build_uart_test"
TOOLCHAIN_FILE="$PROJECT_ROOT/cmake/gcc-arm-none-eabi.cmake"

# Clean previous build
echo "🧹 Cleaning previous build..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Check ARM GCC
echo "📋 Checking ARM GCC toolchain..."
if ! which arm-none-eabi-gcc > /dev/null; then
    echo "❌ ARM GCC not found"
    exit 1
fi

arm-none-eabi-gcc --version | head -1

# Create simple CMakeLists.txt for UART test
cat > "$BUILD_DIR/CMakeLists.txt" << 'EOF'
cmake_minimum_required(VERSION 3.16)

# Project setup
project(uart_test LANGUAGES C ASM)

# ARM Cortex-M7 flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdata-sections -ffunction-sections -Wall -Wextra")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -specs=nano.specs -specs=nosys.specs -Wl,--gc-sections")

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/../src
    ${CMAKE_SOURCE_DIR}/../Core/Inc
    ${CMAKE_SOURCE_DIR}/../Drivers/STM32H7xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy
    ${CMAKE_SOURCE_DIR}/../Drivers/CMSIS/Device/ST/STM32H7xx/Include
    ${CMAKE_SOURCE_DIR}/../Drivers/CMSIS/Include
)

# Compile definitions
add_compile_definitions(
    USE_HAL_DRIVER
    STM32H753xx
    ARM_MATH_CM7
    __FPU_PRESENT=1
)

# Source files
set(SOURCES
    ../src/application/uart_test.c
    ../Core/Src/system_stm32h7xx.c
    ../startup_stm32h753xx.s
    ../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c
    ../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c
    ../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c
    ../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c
    ../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c
    ../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c
    ../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c
    ../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c
    ../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c
    ../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c
)

# Create executable
add_executable(uart_test.elf ${SOURCES})

# Linker script
set_target_properties(uart_test.elf PROPERTIES
    LINK_FLAGS "-T${CMAKE_SOURCE_DIR}/../STM32H753XX_FLASH.ld -Wl,-Map=${CMAKE_BINARY_DIR}/uart_test.map"
)

# Generate additional output formats
add_custom_command(TARGET uart_test.elf POST_BUILD
    COMMAND arm-none-eabi-objcopy -Obinary uart_test.elf uart_test.bin
    COMMAND arm-none-eabi-objcopy -Oihex uart_test.elf uart_test.hex
    COMMAND arm-none-eabi-size uart_test.elf
    COMMENT "Generating binary and hex files"
)
EOF

echo "📁 Created CMakeLists.txt for UART test"

# Configure with toolchain
echo "⚙️ Configuring build..."
cd "$BUILD_DIR"
cmake -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" -G Ninja .

# Build
echo "🔨 Building firmware..."
ninja

if [ $? -eq 0 ]; then
    echo "✅ UART test firmware build successful!"
    echo "📄 Output files:"
    ls -la uart_test.*
    echo ""
    echo "🔌 Ready to flash to STM32H753ZI"
    echo "💡 Flash command: openocd -f interface/stlink.cfg -f target/stm32h7x.cfg -c \"program uart_test.elf verify reset exit\""
else
    echo "❌ Build failed"
    exit 1
fi
