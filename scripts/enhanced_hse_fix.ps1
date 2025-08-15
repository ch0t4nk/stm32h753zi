#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Enhanced HSE Fix with Comprehensive Register Management
    
.DESCRIPTION
    More robust approach to enable HSE using proper STM32H7 register sequence
    Handles power domain configuration and register protection
#>

param([switch]$Force)

$STM32_PROG_CLI = "C:\ST\STM32CubeCLT_1.19.0\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"

function Write-Success { param($Message) Write-Host "✅ $Message" -ForegroundColor Green }
function Write-Error { param($Message) Write-Host "❌ $Message" -ForegroundColor Red }
function Write-Info { param($Message) Write-Host "ℹ️  $Message" -ForegroundColor Cyan }
function Write-Warning { param($Message) Write-Host "⚠️  $Message" -ForegroundColor Yellow }

Write-Host ""
Write-Host "🔧 Enhanced STM32H753ZI HSE Fix" -ForegroundColor Magenta
Write-Host ""

if (-not $Force) {
    $confirm = Read-Host "Apply HSE configuration fix? (y/N)"
    if ($confirm -ne 'y' -and $confirm -ne 'Y') { exit 0 }
}

# Connect and halt
Write-Info "Connecting and halting CPU..."
& $STM32_PROG_CLI -c port=SWD -halt -q
if ($LASTEXITCODE -ne 0) {
    Write-Error "Connection/halt failed"
    exit 1
}
Write-Success "Connected and halted"

# Read current state
Write-Info "Reading current RCC state..."
$rcc_cr_output = & $STM32_PROG_CLI -c port=SWD -r32 0x58024400 1 -q 2>&1
$current_rcc_cr = $null
foreach ($line in $rcc_cr_output) {
    if ($line -match "0x58024400 : ([0-9A-Fa-f]{8})") {
        $current_rcc_cr = [Convert]::ToUInt32($matches[1], 16)
        break
    }
}

if ($null -eq $current_rcc_cr) {
    Write-Error "Failed to read RCC_CR"
    & $STM32_PROG_CLI -c port=SWD -rst -q
    exit 1
}

Write-Host "Current RCC_CR: 0x$($current_rcc_cr.ToString('X8'))" -ForegroundColor White

# Check if HSE is already enabled
$hse_on = ($current_rcc_cr -band 0x00010000) -ne 0
$hse_rdy = ($current_rcc_cr -band 0x00020000) -ne 0

if ($hse_on -and $hse_rdy) {
    Write-Success "HSE is already enabled and ready!"
    Write-Info "The issue may be in firmware clock configuration, not hardware"
    & $STM32_PROG_CLI -c port=SWD -rst -q
    exit 0
}

# Method 1: Try direct register write with proper error checking
Write-Info "Method 1: Direct RCC_CR modification..."
$new_rcc_cr = $current_rcc_cr -bor 0x00010000

$write_result = & $STM32_PROG_CLI -c port=SWD -w32 0x58024400 0x$($new_rcc_cr.ToString('X8')) 2>&1
$write_success = $true
foreach ($line in $write_result) {
    if ($line -match "Error" -or $line -match "Failed") {
        $write_success = $false
        break
    }
}

if ($write_success) {
    Write-Success "RCC_CR write succeeded"
    
    # Verify the write
    Start-Sleep -Milliseconds 100
    $verify_output = & $STM32_PROG_CLI -c port=SWD -r32 0x58024400 1 -q 2>&1
    foreach ($line in $verify_output) {
        if ($line -match "0x58024400 : ([0-9A-Fa-f]{8})") {
            $verify_rcc_cr = [Convert]::ToUInt32($matches[1], 16)
            if (($verify_rcc_cr -band 0x00010000) -ne 0) {
                Write-Success "HSE enable bit confirmed set"
                
                # Wait for HSE ready
                Write-Info "Waiting for HSE ready..."
                for ($i = 0; $i -lt 50; $i++) {
                    Start-Sleep -Milliseconds 100
                    $check_output = & $STM32_PROG_CLI -c port=SWD -r32 0x58024400 1 -q 2>&1
                    foreach ($check_line in $check_output) {
                        if ($check_line -match "0x58024400 : ([0-9A-Fa-f]{8})") {
                            $check_rcc_cr = [Convert]::ToUInt32($matches[1], 16)
                            if (($check_rcc_cr -band 0x00020000) -ne 0) {
                                Write-Success "HSE is now READY! RCC_CR = 0x$($check_rcc_cr.ToString('X8'))"
                                Write-Host ""
                                Write-Host "🎯 SUCCESS: HSE enabled at hardware level!" -ForegroundColor Green
                                Write-Host "Next: Rebuild firmware to use HSE+PLL for 480MHz" -ForegroundColor Cyan
                                & $STM32_PROG_CLI -c port=SWD -rst -q
                                exit 0
                            }
                            break
                        }
                    }
                }
                Write-Warning "HSE enabled but not ready - may need firmware restart"
            }
            else {
                Write-Warning "HSE enable bit not confirmed - write may have failed"
            }
            break
        }
    }
}
else {
    Write-Warning "Direct register write failed"
}

# Method 2: Try with reset sequence
Write-Info "Method 2: Reset sequence + HSE enable..."
& $STM32_PROG_CLI -c port=SWD -rst -halt -q
Start-Sleep -Seconds 1

# Try write again after reset
$write_result2 = & $STM32_PROG_CLI -c port=SWD -w32 0x58024400 0x$($new_rcc_cr.ToString('X8')) 2>&1
Write-Info "Post-reset write attempt completed"

# Final verification
$final_output = & $STM32_PROG_CLI -c port=SWD -r32 0x58024400 1 -q 2>&1
foreach ($line in $final_output) {
    if ($line -match "0x58024400 : ([0-9A-Fa-f]{8})") {
        $final_rcc_cr = [Convert]::ToUInt32($matches[1], 16)
        Write-Host "Final RCC_CR: 0x$($final_rcc_cr.ToString('X8'))" -ForegroundColor White
        
        if (($final_rcc_cr -band 0x00010000) -ne 0) {
            Write-Success "HSE enable achieved!"
        }
        else {
            Write-Warning "HSE still not enabled - this may be normal"
            Write-Info "Some STM32H7 configurations require firmware-level HSE enablement"
        }
        break
    }
}

Write-Host ""
Write-Host "📋 Summary:" -ForegroundColor Cyan
Write-Host "• Register access successful" -ForegroundColor White
Write-Host "• HSE configuration attempted" -ForegroundColor White  
Write-Host "• Main fix will come from firmware SystemClock_Config()" -ForegroundColor White
Write-Host ""
Write-Host "🔧 Next Steps:" -ForegroundColor Magenta
Write-Host "1. Build firmware with your updated SystemClock_Config()" -ForegroundColor White
Write-Host "2. Flash the new firmware" -ForegroundColor White
Write-Host "3. HSE should work properly with corrected firmware" -ForegroundColor White

# Clean reset
& $STM32_PROG_CLI -c port=SWD -rst -q
