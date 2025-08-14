# STM32H753ZI Development Workflow Functions
# This script defines PowerShell functions for easy workflow command execution

# Set the working directory to the project root
$ProjectRoot = Split-Path -Parent $PSScriptRoot

function workflow-before {
    Push-Location $ProjectRoot
    try {
        Write-Host "🚀 Executing Before Work Workflow..." -ForegroundColor Green
        & "$ProjectRoot\scripts\workflow.ps1" "before"
    }
    finally {
        Pop-Location
    }
}

function workflow-during {
    Push-Location $ProjectRoot
    try {
        Write-Host "🔄 Executing During Work Workflow..." -ForegroundColor Yellow
        & "$ProjectRoot\scripts\workflow.ps1" "during"
    }
    finally {
        Pop-Location
    }
}

function workflow-after {
    Push-Location $ProjectRoot
    try {
        Write-Host "✅ Executing After Work Workflow..." -ForegroundColor Cyan
        & "$ProjectRoot\scripts\workflow.ps1" "after"
    }
    finally {
        Pop-Location
    }
}

function workflow-help {
    Write-Host ""
    Write-Host "STM32H753ZI Development Workflow Commands" -ForegroundColor Magenta
    Write-Host "=========================================" -ForegroundColor Magenta
    Write-Host ""
    Write-Host "🚀 workflow-before   - Check feature status, validate environment" -ForegroundColor Green
    Write-Host "🔄 workflow-during   - Build project, update STATUS.md" -ForegroundColor Yellow
    Write-Host "✅ workflow-after    - Update features, prepare commit context" -ForegroundColor Cyan
    Write-Host "❓ workflow-help     - Display this help" -ForegroundColor White
    Write-Host "📊 workflow-status   - Show current feature status" -ForegroundColor Blue
    Write-Host ""
}

function workflow-status {
    Push-Location $ProjectRoot
    try {
        Write-Host "📊 Current Project Status..." -ForegroundColor Blue
        
        # Show feature status
        if (Test-Path "$ProjectRoot\scripts\feature_tracker.py") {
            Write-Host "`n🔍 Feature Status:" -ForegroundColor Yellow
            & python "$ProjectRoot\scripts\feature_tracker.py" list --status IN_PROGRESS
        }
        
        # Show STATUS.md summary
        if (Test-Path "$ProjectRoot\STATUS.md") {
            Write-Host "`n📋 STATUS.md Summary:" -ForegroundColor Green
            Get-Content "$ProjectRoot\STATUS.md" | Select-Object -First 10
        }
    }
    finally {
        Pop-Location
    }
}

Write-Host "✅ Workflow functions loaded. Type 'workflow-help' for usage." -ForegroundColor Green
