# STM32H753ZI Development Workflow Script (PowerShell)
param(
    [Parameter(Mandatory=$true, Position=0)]
    [ValidateSet("before", "during", "after")]
    [string]$Phase,
    [string]$FeatureId = "",
    [string]$Notes = ""
)

Write-Host "[START] STM32H753ZI Workflow - Phase: $Phase" -ForegroundColor Cyan

# Change to project root
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$ProjectRoot = Split-Path -Parent $ScriptDir
Set-Location $ProjectRoot

# Activate virtual environment
if (Test-Path ".venv\Scripts\Activate.ps1") {
    & ".\.venv\Scripts\Activate.ps1"
    Write-Host "[OK] Virtual environment activated" -ForegroundColor Green
}

switch ($Phase) {
    "before" {
        Write-Host "[INFO] Starting work session..." -ForegroundColor Blue
        & python scripts/auto_update_status.py --verbose
        & python scripts/feature_tracker.py report
    }
    "during" {
        Write-Host "[WORK] Development work in progress..." -ForegroundColor Blue
        if ($FeatureId) {
            & python scripts/feature_tracker.py update $FeatureId --status IN_PROGRESS
        }
        if (Test-Path "scripts\fix_cmake.ps1") {
            & ".\scripts\fix_cmake.ps1"
        } else {
            & bash "./scripts/fix_cmake.sh"
        }
        & python scripts/auto_update_status.py --source build --verbose
    }
    "after" {
        Write-Host "[DONE] Completing work session..." -ForegroundColor Blue
        if ($FeatureId) {
            & python scripts/feature_tracker.py update $FeatureId --status COMPLETE
        }
        & python scripts/auto_update_status.py --source build --verbose
        & git status --porcelain
        Write-Host "[OK] Ready for commit" -ForegroundColor Green
    }
}

Write-Host "[OK] Workflow phase '$Phase' completed" -ForegroundColor Green
