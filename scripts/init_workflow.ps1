#!/usr/bin/env pwsh
# STM32H753ZI Workflow Initialization Script
# Run this script to load workflow functions into your PowerShell session

param(
    [switch]$Profile,
    [switch]$Help
)

if ($Help) {
    Write-Host ""
    Write-Host "STM32H753ZI Workflow Initialization" -ForegroundColor Magenta
    Write-Host "===================================" -ForegroundColor Magenta
    Write-Host ""
    Write-Host "Usage:"
    Write-Host "  .\scripts\init_workflow.ps1          # Load functions for current session"
    Write-Host "  .\scripts\init_workflow.ps1 -Profile # Add to PowerShell profile permanently"
    Write-Host "  .\scripts\init_workflow.ps1 -Help    # Show this help"
    Write-Host ""
    Write-Host "Functions loaded:"
    Write-Host "  workflow-before   # Prepare for development work"
    Write-Host "  workflow-during   # Build and update during work"
    Write-Host "  workflow-after    # Finalize work and prepare commit"
    Write-Host "  workflow-help     # Show workflow help"
    Write-Host "  workflow-status   # Show current project status"
    Write-Host ""
    Write-Host "Short aliases: wf-before, wf-during, wf-after, wf-help, wf-status"
    Write-Host ""
    return
}

# Ensure we're in the project root
if (-not (Test-Path "scripts\workflow_functions.ps1")) {
    Write-Error "This script must be run from the STM32H753ZI project root directory"
    Write-Host "Current location: $(Get-Location)"
    Write-Host "Expected file: scripts\workflow_functions.ps1"
    return
}

if ($Profile) {
    # Add to PowerShell profile permanently
    $ProfilePath = $PROFILE
    $LoadCommand = ". `"$(Resolve-Path 'scripts\load_workflow.ps1')`""
    
    Write-Host "Adding workflow functions to PowerShell profile..." -ForegroundColor Yellow
    Write-Host "Profile path: $ProfilePath"
    
    # Create profile directory if it doesn't exist
    $ProfileDir = Split-Path $ProfilePath -Parent
    if (-not (Test-Path $ProfileDir)) {
        New-Item -ItemType Directory -Path $ProfileDir -Force | Out-Null
    }
    
    # Check if already added
    if (Test-Path $ProfilePath) {
        $ProfileContent = Get-Content $ProfilePath -Raw
        if ($ProfileContent -like "*load_workflow.ps1*") {
            Write-Host "✅ Workflow functions already in profile!" -ForegroundColor Green
            return
        }
    }
    
    # Add to profile
    Add-Content -Path $ProfilePath -Value "`n# STM32H753ZI Workflow Functions"
    Add-Content -Path $ProfilePath -Value $LoadCommand
    
    Write-Host "✅ Added to PowerShell profile. Restart PowerShell or run:" -ForegroundColor Green
    Write-Host "   . `$PROFILE" -ForegroundColor Gray
}
else {
    # Load for current session only
    Write-Host "Loading workflow functions for current session..." -ForegroundColor Yellow
    . ".\scripts\load_workflow.ps1"
}

Write-Host ""
Write-Host "🎯 Quick Test - Try running:" -ForegroundColor Cyan
Write-Host "   workflow-help" -ForegroundColor White
Write-Host "   workflow-status" -ForegroundColor White
Write-Host ""
