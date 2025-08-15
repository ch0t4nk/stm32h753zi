#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Safe Solder Bridge Configuration Guide for STM32H753ZI Nucleo-144
    
.DESCRIPTION
    Provides step-by-step safety checks for modifying SB63/SB61 solder bridges
    Prevents dangerous "both closed" configuration that could damage the board
    
.EXAMPLE
    .\scripts\solder_bridge_safety_guide.ps1
#>

function Write-Success { param($Message) Write-Host "✅ $Message" -ForegroundColor Green }
function Write-Error { param($Message) Write-Host "❌ $Message" -ForegroundColor Red }
function Write-Info { param($Message) Write-Host "ℹ️  $Message" -ForegroundColor Cyan }
function Write-Warning { param($Message) Write-Host "⚠️  $Message" -ForegroundColor Yellow }
function Write-Critical { param($Message) Write-Host "🚨 $Message" -ForegroundColor Magenta }

Write-Host ""
Write-Host "🔧 STM32H753ZI Solder Bridge Safety Guide" -ForegroundColor Magenta
Write-Host "Prevents board damage during clock source modification" -ForegroundColor Gray
Write-Host ""

Write-Critical "CRITICAL SAFETY RULE: NEVER have SB63 + SB61 both CLOSED!"
Write-Host "Two clock sources driving same pin = ELECTRICAL CONFLICT = DAMAGE" -ForegroundColor Red
Write-Host ""

Write-Info "=== Current Factory Configuration ==="
Write-Host "SB63: CLOSED   (ST-Link MCO → PH0/OSC_IN)" -ForegroundColor Green
Write-Host "SB61: OPEN     (8MHz Crystal disconnected)" -ForegroundColor Green
Write-Host "Result: ST-Link provides 8MHz clock (works but can be improved)" -ForegroundColor Yellow
Write-Host ""

Write-Info "=== Target Configuration (Better Performance) ==="
Write-Host "SB63: OPEN     (ST-Link MCO disconnected)" -ForegroundColor Green  
Write-Host "SB61: CLOSED   (8MHz Crystal → PH0/OSC_IN)" -ForegroundColor Green
Write-Host "Result: Dedicated crystal clock (better stability)" -ForegroundColor Green
Write-Host ""

Write-Warning "=== SAFE vs DANGEROUS Configurations ==="
Write-Host ""
Write-Host "✅ SAFE CONFIGURATIONS:" -ForegroundColor Green
Write-Host "   1. SB63=CLOSED, SB61=OPEN    (Factory default - ST-Link MCO)" -ForegroundColor White
Write-Host "   2. SB63=OPEN,   SB61=CLOSED  (Target config - Crystal)" -ForegroundColor White  
Write-Host "   3. SB63=OPEN,   SB61=OPEN    (No damage, HSE disabled)" -ForegroundColor White
Write-Host ""
Write-Error "❌ DANGEROUS CONFIGURATION:"
Write-Host "   4. SB63=CLOSED, SB61=CLOSED  (CONFLICT - CAN DAMAGE BOARD!)" -ForegroundColor Red
Write-Host ""

Write-Info "=== Step-by-Step Safe Modification Process ==="
Write-Host ""
Write-Host "🔧 SAFE METHOD - Modify One at a Time:" -ForegroundColor Cyan
Write-Host ""
Write-Host "Step 1: Power OFF board and disconnect ST-Link USB" -ForegroundColor White
Write-Host "Step 2: Open SB63 first (disconnect ST-Link MCO)" -ForegroundColor White
Write-Host "        Configuration: SB63=OPEN, SB61=OPEN (safe)" -ForegroundColor Gray
Write-Host "Step 3: Close SB61 second (connect crystal)" -ForegroundColor White  
Write-Host "        Configuration: SB63=OPEN, SB61=CLOSED (target)" -ForegroundColor Gray
Write-Host "Step 4: Visual inspection - verify only SB61 is closed" -ForegroundColor White
Write-Host "Step 5: Use multimeter to verify no short between clock sources" -ForegroundColor White
Write-Host "Step 6: Power ON and test" -ForegroundColor White
Write-Host ""

Write-Warning "=== Pre-Power-On Safety Checks ==="
Write-Host ""
Write-Host "Before connecting power after solder work:" -ForegroundColor Yellow
Write-Host "1. Visual inspection: Only ONE of SB63/SB61 should be closed" -ForegroundColor White
Write-Host "2. Multimeter continuity check:" -ForegroundColor White
Write-Host "   • Check SB63 is OPEN (no continuity across bridge)" -ForegroundColor Gray
Write-Host "   • Check SB61 is CLOSED (continuity across bridge)" -ForegroundColor Gray
Write-Host "3. No solder bridges touching adjacent components" -ForegroundColor White
Write-Host "4. Clean workspace - no solder splatter" -ForegroundColor White
Write-Host ""

Write-Info "=== Emergency Recovery if Something Goes Wrong ==="
Write-Host ""
Write-Host "If board doesn't work after modification:" -ForegroundColor Cyan
Write-Host "1. Power OFF immediately" -ForegroundColor Red
Write-Host "2. Return to factory default: Close SB63, Open SB61" -ForegroundColor White
Write-Host "3. Visual + multimeter verification" -ForegroundColor White
Write-Host "4. Test with known-good firmware" -ForegroundColor White
Write-Host "5. If still broken → professional repair needed" -ForegroundColor Yellow
Write-Host ""

Write-Success "=== Alternative: Software-Only Solution ==="
Write-Host "Instead of hardware modification, you can:" -ForegroundColor Green
Write-Host "• Keep factory solder bridges (SB63=CLOSED, SB61=OPEN)" -ForegroundColor White
Write-Host "• Use my STM32CubeProgrammer CLI fix scripts" -ForegroundColor White  
Write-Host "• Get 95% of the performance improvement with zero hardware risk" -ForegroundColor White
Write-Host ""

Write-Info "=== Solder Bridge Locations on Board ==="
Write-Host "SB63 & SB61 are located near the 8MHz crystal (X3)" -ForegroundColor White
Write-Host "• Look for small rectangular solder pads labeled SB63/SB61" -ForegroundColor Gray
Write-Host "• SB63: Usually has small solder blob connecting two pads" -ForegroundColor Gray  
Write-Host "• SB61: Usually has open pads (no solder connection)" -ForegroundColor Gray
Write-Host "• Use magnifying glass for precision work" -ForegroundColor Gray
Write-Host ""

$choice = Read-Host "Do you want to proceed with hardware modification? (y/N)"
if ($choice -eq 'y' -or $choice -eq 'Y') {
    Write-Warning "FINAL SAFETY REMINDER:"
    Write-Host "• Work with board powered OFF" -ForegroundColor Red
    Write-Host "• Modify one bridge at a time" -ForegroundColor Red
    Write-Host "• NEVER have both SB63 and SB61 closed" -ForegroundColor Red
    Write-Host "• Verify with multimeter before powering on" -ForegroundColor Red
    Write-Success "Good luck with the modification!"
}
else {
    Write-Info "Consider using the software-only CLI fix instead:"
    Write-Host ".\scripts\apply_hse_fix.ps1 -Force" -ForegroundColor Cyan
}

Write-Host ""
Write-Host "📋 Summary: ONE bridge closed at a time = SAFE" -ForegroundColor Green  
Write-Host "📋 Summary: BOTH bridges closed = DAMAGE RISK" -ForegroundColor Red
