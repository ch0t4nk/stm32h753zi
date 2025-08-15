#!/usr/bin/env pwsh
# Test Enhanced Clock Configuration Fix
# Builds, flashes, and verifies the improved clock configuration

param(
    [switch]$Verbose = $false,
    [switch]$SkipBuild = $false,
    [switch]$MonitorOnly = $false
)

$WorkspaceRoot = $PSScriptRoot + "\.."
Set-Location $WorkspaceRoot

Write-Host "🔧 Testing Enhanced Clock Configuration Fix" -ForegroundColor Cyan
Write-Host "Key fixes applied:" -ForegroundColor Yellow
Write-Host "  ✅ HSE_VALUE corrected from 25MHz to 8MHz" -ForegroundColor Green
Write-Host "  ✅ PLL config optimized for 8MHz crystal (PLLM=2, PLLN=120)" -ForegroundColor Green
Write-Host "  ✅ HSI fallback improved (240MHz target)" -ForegroundColor Green
Write-Host ""

if (-not $MonitorOnly -and -not $SkipBuild) {
    Write-Host "🔨 Building firmware with enhanced clock config..."
    
    # Clean build to ensure all changes are picked up
    if (Test-Path "build") {
        Remove-Item -Recurse -Force build
        Write-Host "  🧹 Cleaned build directory" -ForegroundColor Yellow
    }
    
    # Build firmware
    $buildResult = & .\scripts\fix_cmake.ps1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ Build failed! Check compiler output above." -ForegroundColor Red
        exit 1
    }
    
    Write-Host "✅ Build successful!" -ForegroundColor Green
    
    # Flash firmware
    Write-Host "📡 Flashing enhanced firmware..."
    $flashResult = & .\scripts\flash_stlink.ps1 -Verify -Reset -Verbose:$Verbose
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ Flash failed! Check ST-Link connection." -ForegroundColor Red
        exit 1
    }
    
    Write-Host "✅ Flash successful!" -ForegroundColor Green
    Write-Host ""
}

# Monitor system startup and clock configuration
Write-Host "📊 Monitoring system clock configuration..." -ForegroundColor Cyan
Write-Host "Expected improvements:" -ForegroundColor Yellow
Write-Host "  🎯 HSE should start (8MHz crystal)" -ForegroundColor White
Write-Host "  🎯 PLL should be operational (480MHz VCO)" -ForegroundColor White
Write-Host "  🎯 SYSCLK should be 240MHz (HSE+PLL) or 240MHz (HSI+PLL)" -ForegroundColor White
Write-Host "  🎯 Should NOT be running on CSI 4MHz anymore" -ForegroundColor White
Write-Host ""

# Start serial monitoring
Write-Host "🔍 Starting serial monitor..." -ForegroundColor Green
Write-Host "Press Ctrl+C to stop monitoring" -ForegroundColor Gray

try {
    # Use Python serial monitor
    & $WorkspaceRoot\.venv\Scripts\python.exe scripts\simple_serial_monitor.py
}
catch {
    Write-Host "⚠️ Serial monitoring failed. Check COM port and connections." -ForegroundColor Yellow
    Write-Host "Manual verification:" -ForegroundColor Cyan
    Write-Host "  1. Check LED behavior - should be stable, not rapid blinking" -ForegroundColor White
    Write-Host "  2. Use STM32CubeProgrammer to verify RCC registers:" -ForegroundColor White
    Write-Host "     RCC_CR should show HSERDY=1 or HSIRDY=1 with PLLRDY=1" -ForegroundColor White
    Write-Host "     RCC_CFGR should show SWS=3 (PLL), not SWS=1 (CSI)" -ForegroundColor White
}

Write-Host ""
Write-Host "🎯 Expected register values after fix:" -ForegroundColor Cyan
Write-Host "  RCC_CR: HSERDY=1, PLLRDY=1 (HSE and PLL ready)" -ForegroundColor White
Write-Host "  RCC_CFGR: SWS=3 (system running on PLL)" -ForegroundColor White
Write-Host "  System frequency: 240MHz (significant improvement over 4MHz CSI)" -ForegroundColor White
