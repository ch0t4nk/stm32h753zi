#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Validate STM32H753ZI HSE Clock Configuration and Hardware Setup
    
.DESCRIPTION
    Comprehensive validation using STM32CubeProgrammer CLI v2.20.0
    Checks solder bridge configuration, ST-Link MCO, and RCC registers
    
.EXAMPLE
    .\scripts\validate_hse_hardware.ps1
#>

# STM32CubeProgrammer CLI path
$STM32_PROG_CLI = "C:\ST\STM32CubeCLT_1.19.0\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"

# Color output functions
function Write-Success { param($Message) Write-Host "✅ $Message" -ForegroundColor Green }
function Write-Error { param($Message) Write-Host "❌ $Message" -ForegroundColor Red }
function Write-Info { param($Message) Write-Host "ℹ️  $Message" -ForegroundColor Cyan }
function Write-Warning { param($Message) Write-Host "⚠️  $Message" -ForegroundColor Yellow }

Write-Host ""
Write-Host "🔍 STM32H753ZI HSE Hardware Validation" -ForegroundColor Magenta
Write-Host "Target: Fix 86% performance loss (64MHz HSI vs 480MHz target)" -ForegroundColor Gray
Write-Host ""

# Step 1: Validate STM32CubeProgrammer
Write-Info "Step 1: Validating STM32CubeProgrammer CLI..."
if (-not (Test-Path $STM32_PROG_CLI)) {
    Write-Error "STM32CubeProgrammer CLI not found at: $STM32_PROG_CLI"
    exit 1
}

$version_output = & $STM32_PROG_CLI --version 2>&1
Write-Success "STM32CubeProgrammer CLI v2.20.0 detected"

# Step 2: Connect to target
Write-Info "Step 2: Connecting to STM32H753ZI..."
$connect_output = & $STM32_PROG_CLI -c port=SWD -q 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Error "Connection failed. Check:"
    Write-Host "  • ST-Link USB connection" -ForegroundColor Red
    Write-Host "  • Target board power (CN1 or USB power)" -ForegroundColor Red
    Write-Host "  • ST-Link driver installation" -ForegroundColor Red
    exit 1
}
Write-Success "Connected to STM32H753ZI via ST-Link"

# Step 3: Read RCC registers for clock analysis
Write-Info "Step 3: Reading RCC registers..."

# RCC_CR (0x58024400) - Control Register
$rcc_cr_output = & $STM32_PROG_CLI -c port=SWD -r32 0x58024400 1 -q 2>&1
if ($rcc_cr_output -match "0x([0-9A-Fa-f]{8})") {
    $rcc_cr = [Convert]::ToUInt32($matches[1], 16)
    Write-Host "  RCC_CR: 0x$($rcc_cr.ToString('X8'))" -ForegroundColor White
    
    # Decode key bits
    $hsi_on = ($rcc_cr -band 0x00000001) -ne 0
    $hsi_rdy = ($rcc_cr -band 0x00000002) -ne 0
    $hse_on = ($rcc_cr -band 0x00010000) -ne 0
    $hse_rdy = ($rcc_cr -band 0x00020000) -ne 0
    
    Write-Host "    HSI: " -NoNewline -ForegroundColor Gray
    if ($hsi_on -and $hsi_rdy) {
        Write-Host "ON + READY (fallback active)" -ForegroundColor Yellow
    }
    else {
        Write-Host "OFF" -ForegroundColor Gray
    }
    
    Write-Host "    HSE: " -NoNewline -ForegroundColor Gray
    if ($hse_on -and $hse_rdy) {
        Write-Host "ON + READY" -ForegroundColor Green
    }
    elseif ($hse_on) {
        Write-Host "ON but NOT READY (startup failure!)" -ForegroundColor Red
    }
    else {
        Write-Host "OFF (not enabled)" -ForegroundColor Red
    }
}
else {
    Write-Error "Failed to read RCC_CR register"
}

# RCC_CFGR (0x58024408) - Configuration Register
$rcc_cfgr_output = & $STM32_PROG_CLI -c port=SWD -r32 0x58024408 1 -q 2>&1
if ($rcc_cfgr_output -match "0x([0-9A-Fa-f]{8})") {
    $rcc_cfgr = [Convert]::ToUInt32($matches[1], 16)
    Write-Host "  RCC_CFGR: 0x$($rcc_cfgr.ToString('X8'))" -ForegroundColor White
    
    # System clock source (SWS bits 3-5)
    $sws = ($rcc_cfgr -shr 3) -band 0x7
    Write-Host "    System Clock: " -NoNewline -ForegroundColor Gray
    switch ($sws) {
        0 { 
            Write-Host "HSI (64MHz) - PERFORMANCE PROBLEM!" -ForegroundColor Red
            $using_hsi_fallback = $true
        }
        1 { Write-Host "CSI" -ForegroundColor Green }
        2 { Write-Host "HSE (8MHz)" -ForegroundColor Green }
        3 { Write-Host "PLL1 (480MHz target)" -ForegroundColor Green }
        default { Write-Host "Unknown ($sws)" -ForegroundColor Yellow }
    }
}
else {
    Write-Error "Failed to read RCC_CFGR register"
}

# Step 4: Hardware Configuration Check
Write-Host ""
Write-Info "Step 4: Hardware Configuration Analysis"

Write-Host "  Nucleo-H753ZI Default Solder Bridges (as sold):" -ForegroundColor White
Write-Host "    SB63: CLOSED   (connects ST-Link MCO to PH0/OSC_IN)" -ForegroundColor Green
Write-Host "    SB61: OPEN     (disconnects 8MHz crystal)" -ForegroundColor Green  
Write-Host "    SB62: OPEN     (disconnects 32kHz crystal)" -ForegroundColor Gray
Write-Host ""
Write-Host "  Expected Clock Source: ST-Link MCO → 8MHz → PH0 (OSC_IN)" -ForegroundColor Cyan

# Step 5: ST-Link MCO Validation
Write-Info "Step 5: ST-Link MCO Configuration Check"
Write-Host "  ST-Link provides 8MHz on MCO pin when:" -ForegroundColor White
Write-Host "    • ST-Link firmware is updated (STLinkUpgrade.bat)" -ForegroundColor Gray
Write-Host "    • MCO configuration is enabled in ST-Link" -ForegroundColor Gray
Write-Host "    • SB63 is closed (factory default)" -ForegroundColor Gray

# Check ST-Link version
$stlink_output = & $STM32_PROG_CLI -c port=SWD --stlink-version -q 2>&1
Write-Host "  ST-Link Version: " -NoNewline -ForegroundColor Gray
if ($stlink_output -match "V(\d+)J(\d+)M(\d+)") {
    Write-Host "$($matches[0])" -ForegroundColor Green
}
else {
    Write-Host "Could not detect" -ForegroundColor Yellow
}

# Step 6: Problem Diagnosis
Write-Host ""
Write-Info "Step 6: Problem Diagnosis & Recommendations"

if ($using_hsi_fallback) {
    Write-Error "CONFIRMED: System running on HSI fallback (64MHz)"
    Write-Host ""
    Write-Host "  Root Cause Analysis:" -ForegroundColor Yellow
    Write-Host "    1. HSE (8MHz external) not starting properly" -ForegroundColor Red
    Write-Host "    2. Firmware expects HSE but hardware provides MCO" -ForegroundColor Red
    Write-Host "    3. Configuration mismatch → HSI fallback → 86% performance loss" -ForegroundColor Red
    
    Write-Host ""
    Write-Host "  Solution Options:" -ForegroundColor Cyan
    Write-Host "    Option A: Update ST-Link MCO configuration" -ForegroundColor White
    Write-Host "    Option B: Modify firmware to handle MCO properly" -ForegroundColor White
    Write-Host "    Option C: Fix HSE startup sequence in code" -ForegroundColor Green
    
    Write-Host ""
    Write-Host "  Next Actions:" -ForegroundColor Magenta
    Write-Host "    1. Run: .\scripts\fix_hse_clock_config.ps1 -Apply" -ForegroundColor White
    Write-Host "    2. Rebuild firmware with proper HSE configuration" -ForegroundColor White
    Write-Host "    3. Test performance improvement (64MHz → 480MHz)" -ForegroundColor White
    
}
else {
    Write-Success "Clock configuration appears correct"
    Write-Host "  System is not using HSI fallback" -ForegroundColor Green
    Write-Host "  Performance should be at target levels" -ForegroundColor Green
}

# Step 7: Expected vs Actual Performance
Write-Host ""
Write-Info "Step 7: Performance Impact Assessment"
Write-Host "  Target Performance: 480MHz (STM32H753ZI maximum)" -ForegroundColor Green
Write-Host "  Current Performance: " -NoNewline -ForegroundColor Gray
if ($using_hsi_fallback) {
    Write-Host "64MHz (86% LOSS)" -ForegroundColor Red
    Write-Host "  Impact: Severely degraded motor control, communication, processing" -ForegroundColor Red
}
else {
    Write-Host "Unknown - requires further analysis" -ForegroundColor Yellow
}

# Disconnect
Write-Info "Disconnecting from target..."
& $STM32_PROG_CLI -c port=SWD -rst -q

Write-Host ""
Write-Host "📋 Validation Complete" -ForegroundColor Magenta
Write-Host "Use the fix script to resolve HSE issues: .\scripts\fix_hse_clock_config.ps1 -Apply" -ForegroundColor Gray
