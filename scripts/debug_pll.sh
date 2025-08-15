#!/bin/bash
# STM32H753ZI PLL Configuration Debug Session
# This script starts STLink GDB server and connects ARM GDB for debugging

echo "🔧 STM32H753ZI PLL Configuration Debug Setup"
echo "=============================================="

# Check if STM32CubeCLT is available
STLINK_GDB_SERVER="C:/ST/STM32CubeCLT_1.19.0/STLink-gdb-server/bin/ST-LINK_gdbserver.exe"
ARM_GDB="C:/ST/STM32CubeCLT_1.19.0/GNU-tools-for-STM32/bin/arm-none-eabi-gdb.exe"

if [ ! -f "$STLINK_GDB_SERVER" ]; then
    echo "❌ STLink GDB Server not found at: $STLINK_GDB_SERVER"
    exit 1
fi

if [ ! -f "$ARM_GDB" ]; then
    echo "❌ ARM GDB not found at: $ARM_GDB"
    exit 1
fi

echo "✅ STM32CubeCLT tools found"

# Start STLink GDB Server in background
echo "🚀 Starting STLink GDB Server..."
"$STLINK_GDB_SERVER" --swd --port-number 61234 --halt --verbose &
STLINK_PID=$!

# Wait for server to start
echo "⏳ Waiting for STLink GDB Server to initialize..."
sleep 3

# Check if firmware is built
ELF_FILE="build/Debug/stm32h753_ihm02a1.elf"
if [ ! -f "$ELF_FILE" ]; then
    echo "❌ Firmware not found: $ELF_FILE"
    echo "💡 Run build task first: Build (CMake)"
    kill $STLINK_PID
    exit 1
fi

echo "✅ Firmware found: $ELF_FILE"

# Start debugging session
echo "🐛 Starting PLL configuration debug session..."
echo "📍 Breakpoints will be set at:"
echo "   - Clock_Init()"
echo "   - Clock_ConfigurePLL_HSI()"
echo "   - HAL_RCC_OscConfig()"
echo ""
echo "🔍 Use these GDB commands during debugging:"
echo "   - debug_pll_step    : Show detailed register status"
echo "   - show_rcc_status   : Show RCC registers"
echo "   - show_pwr_status   : Show power management registers"
echo "   - continue          : Continue execution"
echo "   - step              : Single step"
echo "   - quit              : Exit debugger"
echo ""
echo "Press Enter to start GDB session..."
read

# Launch ARM GDB with our debug script
"$ARM_GDB" "$ELF_FILE" -x debug_pll_config.gdb

# Cleanup
echo "🧹 Cleaning up..."
kill $STLINK_PID 2>/dev/null
echo "✅ Debug session complete"
