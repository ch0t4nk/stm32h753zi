# STM32H753ZI Development Workflow PowerShell Module
# Simple workflow commands for STM32H753ZI development

# Get the project root from the module location
$ModuleRoot = Split-Path -Parent $PSScriptRoot

function workflow-before {
    <#
    .SYNOPSIS
    Execute "before work" workflow preparation
    #>
    Write-Host "🚀 Executing Before Work Workflow..." -ForegroundColor Green
    Push-Location $ModuleRoot
    try {
        & "$ModuleRoot\scripts\workflow.ps1" "before"
    }
    finally {
        Pop-Location
    }
}

function workflow-during {
    <#
    .SYNOPSIS
    Execute "during work" workflow - build and update
    #>
    Write-Host "🔄 Executing During Work Workflow..." -ForegroundColor Yellow
    Push-Location $ModuleRoot
    try {
        & "$ModuleRoot\scripts\workflow.ps1" "during"
    }
    finally {
        Pop-Location
    }
}

function workflow-after {
    <#
    .SYNOPSIS
    Execute "after work" workflow - finalize and prepare commit
    #>
    Write-Host "✅ Executing After Work Workflow..." -ForegroundColor Cyan
    Push-Location $ModuleRoot
    try {
        & "$ModuleRoot\scripts\workflow.ps1" "after"
    }
    finally {
        Pop-Location
    }
}

function workflow-help {
    <#
    .SYNOPSIS
    Show workflow command help
    #>
    Write-Host ""
    Write-Host "STM32H753ZI Development Workflow Commands" -ForegroundColor Magenta
    Write-Host "=========================================" -ForegroundColor Magenta
    Write-Host ""
    Write-Host "🚀 workflow-before   - Prepare for development work" -ForegroundColor Green
    Write-Host "🔄 workflow-during   - Build project and update status" -ForegroundColor Yellow
    Write-Host "✅ workflow-after    - Finalize work and prepare commit" -ForegroundColor Cyan
    Write-Host "❓ workflow-help     - Show this help" -ForegroundColor White
    Write-Host "📊 workflow-status   - Show current project status" -ForegroundColor Blue
    Write-Host ""
    Write-Host "VS Code Tasks available:" -ForegroundColor Gray
    Write-Host "  🚀 WORKFLOW: Before Work" -ForegroundColor DarkGray
    Write-Host "  🔄 WORKFLOW: During Work (Build + Update)" -ForegroundColor DarkGray
    Write-Host "  ✅ WORKFLOW: After Work (Prepare Commit)" -ForegroundColor DarkGray
    Write-Host ""
}

function workflow-status {
    <#
    .SYNOPSIS
    Show current project status
    #>
    Write-Host "📊 Current Project Status..." -ForegroundColor Blue
    
    if (Test-Path "$ModuleRoot\STATUS.md") {
        Write-Host "`n📋 STATUS.md Summary:" -ForegroundColor Green
        Get-Content "$ModuleRoot\STATUS.md" | Select-Object -First 15
    }
    else {
        Write-Host "⚠️ STATUS.md not found in project root" -ForegroundColor Yellow
    }
}

# Export the functions
Export-ModuleMember -Function workflow-before, workflow-during, workflow-after, workflow-help, workflow-status

# Auto-display help when module is loaded
Write-Host ""
Write-Host "✅ STM32H753ZI workflow functions loaded!" -ForegroundColor Green
Write-Host "   Commands: workflow-before, workflow-during, workflow-after, workflow-help, workflow-status" -ForegroundColor Gray
Write-Host "   Type 'workflow-help' for detailed usage." -ForegroundColor Gray
Write-Host ""
