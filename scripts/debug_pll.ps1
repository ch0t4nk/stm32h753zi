# STM32H753ZI PLL Configuration Debug Session
# PowerShell version for Windows debugging

Write-Host "🔧 STM32H753ZI PLL Configuration Debug Setup" -ForegroundColor Cyan
Write-Host "==============================================`n"

# STM32CubeCLT paths
$STLINK_GDB_SERVER = "C:\ST\STM32CubeCLT_1.19.0\STLink-gdb-server\bin\ST-LINK_gdbserver.exe"
$ARM_GDB = "C:\ST\STM32CubeCLT_1.19.0\GNU-tools-for-STM32\bin\arm-none-eabi-gdb.exe"

# Check if tools exist
if (-not (Test-Path $STLINK_GDB_SERVER)) {
    Write-Host "❌ STLink GDB Server not found at: $STLINK_GDB_SERVER" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $ARM_GDB)) {
    Write-Host "❌ ARM GDB not found at: $ARM_GDB" -ForegroundColor Red
    exit 1
}

Write-Host "✅ STM32CubeCLT tools found" -ForegroundColor Green

# Check if firmware is built
$ELF_FILE = "build\Debug\stm32h753_ihm02a1.elf"
if (-not (Test-Path $ELF_FILE)) {
    Write-Host "❌ Firmware not found: $ELF_FILE" -ForegroundColor Red
    Write-Host "💡 Run build task first: Build (CMake)" -ForegroundColor Yellow
    exit 1
}

Write-Host "✅ Firmware found: $ELF_FILE" -ForegroundColor Green

# Start STLink GDB Server
Write-Host "🚀 Starting STLink GDB Server..." -ForegroundColor Cyan
$STLinkProcess = Start-Process -FilePath $STLINK_GDB_SERVER -ArgumentList "--swd", "--port-number", "61234", "--halt", "--verbose" -PassThru -WindowStyle Minimized

# Wait for server to start
Write-Host "⏳ Waiting for STLink GDB Server to initialize..." -ForegroundColor Yellow
Start-Sleep -Seconds 3

# Display debug session info
Write-Host "🐛 Starting PLL configuration debug session..." -ForegroundColor Cyan
Write-Host "📍 Breakpoints will be set at:" -ForegroundColor White
Write-Host "   - Clock_Init()"
Write-Host "   - Clock_ConfigurePLL_HSI()"
Write-Host "   - HAL_RCC_OscConfig()"
Write-Host ""
Write-Host "🔍 Use these GDB commands during debugging:" -ForegroundColor White
Write-Host "   - debug_pll_step    : Show detailed register status"
Write-Host "   - show_rcc_status   : Show RCC registers"
Write-Host "   - show_pwr_status   : Show power management registers"
Write-Host "   - continue          : Continue execution"
Write-Host "   - step              : Single step"
Write-Host "   - quit              : Exit debugger"
Write-Host ""
Write-Host "Press Enter to start GDB session..." -ForegroundColor Yellow
Read-Host

try {
    # Launch ARM GDB with our debug script
    & $ARM_GDB $ELF_FILE -x "debug_pll_config.gdb"
}
finally {
    # Cleanup
    Write-Host "`n🧹 Cleaning up..." -ForegroundColor Cyan
    if ($STLinkProcess -and !$STLinkProcess.HasExited) {
        $STLinkProcess.Kill()
        $STLinkProcess.WaitForExit(5000)
    }
    Write-Host "✅ Debug session complete" -ForegroundColor Green
}
