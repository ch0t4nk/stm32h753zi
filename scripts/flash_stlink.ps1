#!/usr/bin/env pwsh
#
# STM32H753ZI Flash Script using ST-Link
# Flashes firmware to STM32H753ZI Nucleo board
#

param(
    [switch]$Verify,
    [switch]$Reset,
    [switch]$Verbose,
    [string]$BinFile = "build\Debug\stm32h753_ihm02a1.bin",
    [string]$ElfFile = "build\Debug\stm32h753_ihm02a1.elf"
)

# Colors for output
$Red = [System.ConsoleColor]::Red
$Green = [System.ConsoleColor]::Green
$Yellow = [System.ConsoleColor]::Yellow
$Blue = [System.ConsoleColor]::Blue

function Write-ColorOutput($ForegroundColor) {
    # Function to write colored output
    if ($args) {
        Write-Host $args -ForegroundColor $ForegroundColor
    }
}

Write-ColorOutput $Blue "🚀 STM32H753ZI Firmware Flash Script"
Write-ColorOutput $Blue "=================================="

# Check if binary file exists
if (-not (Test-Path $BinFile)) {
    Write-ColorOutput $Red "❌ Error: Binary file not found: $BinFile"
    Write-ColorOutput $Yellow "💡 Run 'cmake --build build/Debug' first to build firmware"
    exit 1
}

# Get file info
$fileInfo = Get-Item $BinFile
Write-ColorOutput $Green "✅ Found firmware binary: $BinFile"
$fileSizeKB = [math]::Round($fileInfo.Length / 1024, 1)
Write-ColorOutput $Green "📊 File size: $($fileInfo.Length) bytes ($fileSizeKB KB)"

# Check for ST-Link tools
$stlinkPaths = @(
    "C:\ST\STM32CubeCLT_1.18.0\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe",
    "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe",
    "C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK_CLI.exe"
)

$stlinkTool = $null
foreach ($path in $stlinkPaths) {
    if (Test-Path $path) {
        $stlinkTool = $path
        break
    }
}

if (-not $stlinkTool) {
    Write-ColorOutput $Red "❌ Error: ST-Link programming tool not found"
    Write-ColorOutput $Yellow "💡 Please install STM32CubeProgrammer or ST-LINK Utility"
    Write-ColorOutput $Yellow "📥 Download from: https://www.st.com/en/development-tools/stm32cubeprog.html"
    exit 1
}

Write-ColorOutput $Green "✅ Found ST-Link tool: $stlinkTool"

# Connect to STM32H753ZI
Write-ColorOutput $Blue "🔗 Connecting to STM32H753ZI..."

if ($stlinkTool -like "*STM32_Programmer_CLI*") {
    # Using STM32CubeProgrammer
    Write-ColorOutput $Blue "📡 Using STM32CubeProgrammer"
    
    # Connect
    & "$stlinkTool" -c port=SWD -c port=SWD reset=HWrst
    if ($LASTEXITCODE -ne 0) {
        Write-ColorOutput $Red "❌ Error: Failed to connect to STM32H753ZI"
        Write-ColorOutput $Yellow "💡 Check USB connection and ensure ST-Link drivers are installed"
        exit 1
    }
    
    Write-ColorOutput $Green "✅ Connected to STM32H753ZI via ST-Link"
    
    # Erase flash
    Write-ColorOutput $Blue "🧹 Erasing flash memory..."
    & "$stlinkTool" -c port=SWD -e all
    if ($LASTEXITCODE -ne 0) {
        Write-ColorOutput $Red "❌ Error: Failed to erase flash"
        exit 1
    }
    
    # Program flash
    Write-ColorOutput $Blue "⚡ Programming flash memory..."
    $flashCmd = @("-c", "port=SWD", "-w", $BinFile, "0x08000000")
    if ($Verify) {
        $flashCmd += "-v"
    }
    
    & "$stlinkTool" @flashCmd
    if ($LASTEXITCODE -ne 0) {
        Write-ColorOutput $Red "❌ Error: Failed to program flash"
        exit 1
    }
    
    # Reset if requested
    if ($Reset) {
        Write-ColorOutput $Blue "🔄 Resetting microcontroller..."
        & "$stlinkTool" -c port=SWD -rst
    }
    
    # Disconnect
    & "$stlinkTool" -c port=SWD -disconn
    
}
else {
    # Using ST-LINK Utility
    Write-ColorOutput $Blue "📡 Using ST-LINK Utility"
    
    $flashArgs = @("-c", "-p", $BinFile, "0x08000000")
    if ($Verify) {
        $flashArgs += "-v"
    }
    if ($Reset) {
        $flashArgs += "-rst"
    }
    
    & "$stlinkTool" @flashArgs
    if ($LASTEXITCODE -ne 0) {
        Write-ColorOutput $Red "❌ Error: Failed to flash firmware"
        exit 1
    }
}

Write-ColorOutput $Green "🎉 Firmware flashed successfully!"
Write-ColorOutput $Green "✅ STM32H753ZI ready for testing"

if ($Reset) {
    Write-ColorOutput $Blue "🚀 Microcontroller reset - firmware should be running"
    Write-ColorOutput $Yellow "💡 Connect to serial port to see debug output"
    Write-ColorOutput $Yellow "🔌 UART3 on USB VCP (Virtual COM Port)"
}

Write-ColorOutput $Blue "📊 Flash Summary:"
Write-ColorOutput $Blue "  - Target: STM32H753ZI"
Write-ColorOutput $Blue "  - Binary: $BinFile"
Write-ColorOutput $Blue "  - Size: $($fileInfo.Length) bytes"
Write-ColorOutput $Blue "  - Address: 0x08000000"
Write-ColorOutput $Blue "  - Verify: $Verify"
Write-ColorOutput $Blue "  - Reset: $Reset"
