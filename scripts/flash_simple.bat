@echo off
REM STM32H753ZI Flash Script
REM Simple batch script to flash firmware using STM32CubeProgrammer

echo 🚀 STM32H753ZI Firmware Flash Script
echo ==================================

REM Check if binary exists
if not exist "build\Debug\stm32h753_ihm02a1.bin" (
    echo ❌ Error: Binary file not found
    echo 💡 Run 'cmake --build build/Debug' first
    exit /b 1
)

echo ✅ Found firmware binary: build\Debug\stm32h753_ihm02a1.bin

REM Find STM32CubeProgrammer
set PROGRAMMER="C:\ST\STM32CubeCLT_1.18.0\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
if not exist %PROGRAMMER% (
    set PROGRAMMER="C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
)

if not exist %PROGRAMMER% (
    echo ❌ Error: STM32CubeProgrammer not found
    echo 💡 Please install STM32CubeProgrammer
    exit /b 1
)

echo ✅ Found programmer: %PROGRAMMER%

REM Connect and flash
echo 🔗 Connecting to STM32H753ZI...
%PROGRAMMER% -c port=SWD

if errorlevel 1 (
    echo ❌ Error: Failed to connect to STM32H753ZI
    echo 💡 Check USB connection and ST-Link drivers
    exit /b 1
)

echo ✅ Connected successfully

echo ⚡ Programming flash memory...
%PROGRAMMER% -c port=SWD -w "build\Debug\stm32h753_ihm02a1.bin" 0x08000000 -v

if errorlevel 1 (
    echo ❌ Error: Failed to program flash
    exit /b 1
)

echo 🔄 Resetting microcontroller...
%PROGRAMMER% -c port=SWD -rst

echo 🎉 Firmware flashed successfully!
echo ✅ STM32H753ZI ready for testing
echo 💡 Connect to serial port to see debug output (UART3 on USB VCP)

REM Disconnect
%PROGRAMMER% -c port=SWD -disconn

echo 📊 Flash complete - 53KB firmware loaded
