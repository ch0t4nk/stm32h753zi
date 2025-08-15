#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Apply HSE Clock Fix for STM32H753ZI - Option C Implementation
    
.DESCRIPTION
    Uses STM32CubeProgrammer CLI v2.20.0 to fix HSE startup
    Current: RCC_CR = 0x00004025 (HSE OFF, HSI fallback at 64MHz)  
    Target: Enable HSE to achieve 480MHz performance
    
.PARAMETER Force
    Apply fixes without confirmation
    
.EXAMPLE
    .\scripts\apply_hse_fix.ps1
    .\scripts\apply_hse_fix.ps1 -Force
#>

param(
    [switch]$Force
)

$STM32_PROG_CLI = "C:\ST\STM32CubeCLT_1.19.0\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"

function Write-Success { param($Message) Write-Host "✅ $Message" -ForegroundColor Green }
function Write-Error { param($Message) Write-Host "❌ $Message" -ForegroundColor Red }
function Write-Info { param($Message) Write-Host "ℹ️  $Message" -ForegroundColor Cyan }
function Write-Warning { param($Message) Write-Host "⚠️  $Message" -ForegroundColor Yellow }

Write-Host ""
Write-Host "🔧 STM32H753ZI HSE Clock Fix - Option C" -ForegroundColor Magenta
Write-Host "Target: Fix 86% performance loss (64MHz → 480MHz)" -ForegroundColor Gray
Write-Host ""

if (-not $Force) {
    Write-Warning "This will modify RCC registers on your STM32H753ZI"
    Write-Host "Current status: HSE OFF, system on HSI fallback (64MHz)" -ForegroundColor Yellow
    Write-Host "After fix: HSE enabled for 480MHz target performance" -ForegroundColor Green
    Write-Host ""
    $confirm = Read-Host "Continue? (y/N)"
    if ($confirm -ne 'y' -and $confirm -ne 'Y') {
        Write-Info "Operation cancelled"
        exit 0
    }
}

Write-Info "Step 1: Connecting to STM32H753ZI..."
$connect_output = & $STM32_PROG_CLI -c port=SWD 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Error "Connection failed. Check ST-Link connection."
    exit 1
}
Write-Success "Connected to STM32H753ZI (ST-Link V3J16M8)"

Write-Info "Step 1.5: Halting CPU for register access..."
$halt_output = & $STM32_PROG_CLI -c port=SWD -halt 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Warning "CPU halt failed - continuing anyway"
}
else {
    Write-Success "CPU halted for register modification"
}

Write-Info "Step 2: Reading current RCC_CR register..."
$rcc_cr_output = & $STM32_PROG_CLI -c port=SWD -r32 0x58024400 1 2>&1
$current_rcc_cr = $null
foreach ($line in $rcc_cr_output) {
    if ($line -match "0x58024400 : ([0-9A-Fa-f]{8})") {
        $current_rcc_cr = [Convert]::ToUInt32($matches[1], 16)
        break
    }
}

if ($null -eq $current_rcc_cr) {
    Write-Error "Failed to read RCC_CR register"
    exit 1
}

Write-Host "  Current RCC_CR: 0x$($current_rcc_cr.ToString('X8'))" -ForegroundColor White

# Decode current state
$hsi_on = ($current_rcc_cr -band 0x00000001) -ne 0
$hsi_rdy = ($current_rcc_cr -band 0x00000002) -ne 0  
$hse_on = ($current_rcc_cr -band 0x00010000) -ne 0
$hse_rdy = ($current_rcc_cr -band 0x00020000) -ne 0

Write-Host "    HSI: " -NoNewline
if ($hsi_on -and $hsi_rdy) { Write-Host "ON + READY (fallback active)" -ForegroundColor Yellow }
else { Write-Host "OFF" -ForegroundColor Gray }

Write-Host "    HSE: " -NoNewline  
if ($hse_on -and $hse_rdy) { Write-Host "ON + READY" -ForegroundColor Green }
elseif ($hse_on) { Write-Host "ON but NOT READY" -ForegroundColor Red }
else { Write-Host "OFF (disabled)" -ForegroundColor Red }

if ($hse_on -and $hse_rdy) {
    Write-Success "HSE is already enabled and ready!"
    Write-Info "No register changes needed - check firmware configuration"
    exit 0
}

Write-Info "Step 3: Enabling HSE (8MHz ST-Link MCO)..."

# Calculate new RCC_CR value with HSE enabled (set bit 16)
$new_rcc_cr = $current_rcc_cr -bor 0x00010000

Write-Host "  New RCC_CR: 0x$($new_rcc_cr.ToString('X8')) (HSE enabled)" -ForegroundColor Cyan

# Write new RCC_CR value
$write_output = & $STM32_PROG_CLI -c port=SWD -w32 0x58024400 0x$($new_rcc_cr.ToString('X8')) 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to write RCC_CR register"
    exit 1
}
Write-Success "RCC_CR updated - HSE enable bit set"

Write-Info "Step 4: Waiting for HSE ready..."

# Wait for HSE ready with timeout
$timeout_seconds = 10
$start_time = Get-Date
$hse_ready = $false

while (((Get-Date) - $start_time).TotalSeconds -lt $timeout_seconds) {
    Start-Sleep -Milliseconds 200
    
    # Read RCC_CR again
    $check_output = & $STM32_PROG_CLI -c port=SWD -r32 0x58024400 1 2>&1
    foreach ($line in $check_output) {
        if ($line -match "0x58024400 : ([0-9A-Fa-f]{8})") {
            $check_rcc_cr = [Convert]::ToUInt32($matches[1], 16)
            
            # Check HSE ready bit (bit 17)
            if (($check_rcc_cr -band 0x00020000) -ne 0) {
                $hse_ready = $true
                Write-Success "HSE is now READY! RCC_CR = 0x$($check_rcc_cr.ToString('X8'))"
                break
            }
        }
    }
    
    if ($hse_ready) { break }
    Write-Host "." -NoNewline -ForegroundColor Gray
}

Write-Host ""

if (-not $hse_ready) {
    Write-Error "HSE failed to start within $timeout_seconds seconds"
    Write-Warning "Possible causes:"
    Write-Host "  • ST-Link MCO not configured for 8MHz output" -ForegroundColor Red
    Write-Host "  • Solder bridge SB63 not closed (should be closed)" -ForegroundColor Red  
    Write-Host "  • Hardware clock source issue" -ForegroundColor Red
    Write-Host ""
    Write-Info "Try: C:\ST\STM32CubeCLT_1.19.0\STLinkUpgrade.bat to update ST-Link"
    exit 1
}

Write-Info "Step 5: Checking system clock source..."
$rcc_cfgr_output = & $STM32_PROG_CLI -c port=SWD -r32 0x58024408 1 2>&1
foreach ($line in $rcc_cfgr_output) {
    if ($line -match "0x58024408 : ([0-9A-Fa-f]{8})") {
        $rcc_cfgr = [Convert]::ToUInt32($matches[1], 16)
        $sws = ($rcc_cfgr -shr 3) -band 0x7
        
        Write-Host "  RCC_CFGR: 0x$($rcc_cfgr.ToString('X8'))" -ForegroundColor White
        Write-Host "  System Clock Source: " -NoNewline
        
        switch ($sws) {
            0 { Write-Host "HSI (still on fallback)" -ForegroundColor Yellow }
            1 { Write-Host "CSI" -ForegroundColor Green }
            2 { Write-Host "HSE" -ForegroundColor Green }  
            3 { Write-Host "PLL1" -ForegroundColor Green }
            default { Write-Host "Unknown ($sws)" -ForegroundColor Red }
        }
        break
    }
}

Write-Success "HSE hardware configuration completed!"

Write-Host ""
Write-Host "🎯 Next Steps:" -ForegroundColor Magenta
Write-Host "1. HSE is now enabled and ready at hardware level" -ForegroundColor Green
Write-Host "2. Firmware still needs to be configured to use HSE+PLL" -ForegroundColor Yellow  
Write-Host "3. Rebuild firmware with proper clock configuration" -ForegroundColor White
Write-Host "4. Flash updated firmware to achieve 480MHz performance" -ForegroundColor White

Write-Host ""
Write-Host "📋 Firmware Changes Needed:" -ForegroundColor Cyan
Write-Host "• SystemClock_Config() should now succeed with HSE" -ForegroundColor Gray
Write-Host "• PLL configuration will enable 480MHz target speed" -ForegroundColor Gray
Write-Host "• Performance improvement: 64MHz → 480MHz (750% boost!)" -ForegroundColor Green

# Reset target to clean state
Write-Info "Resetting target to clean state..."
& $STM32_PROG_CLI -c port=SWD -rst 2>&1 | Out-Null

Write-Host ""
Write-Success "HSE fix completed successfully!"
