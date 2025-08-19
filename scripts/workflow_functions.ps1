# STM32H753ZI Development Workflow Functions
# This script defines PowerShell functions for easy workflow command execution

# Set the working directory to the project root
$ProjectRoot = Split-Path -Parent $PSScriptRoot

function workflow-before {
    <#
    .SYNOPSIS
    Execute "before work" workflow phase
    
    .DESCRIPTION
    Runs the development workflow preparation tasks:
    - Check current feature status
    - Validate environment
    - Display project context
    #>
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
    <#
    .SYNOPSIS
    Execute "during work" workflow phase
    
    .DESCRIPTION
    Runs the development workflow build and update tasks:
    - Build project
    - Update STATUS.md
    - Monitor build status
    #>
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
    <#
    .SYNOPSIS
    Execute "after work" workflow phase
    
    .DESCRIPTION
    Runs the development workflow completion tasks:
    - Update feature status
    - Prepare commit context
    - Finalize STATUS.md
    #>
    Push-Location $ProjectRoot
    try {
        Write-Host "✅ Executing After Work Workflow..." -ForegroundColor Cyan
        & "$ProjectRoot\scripts\workflow.ps1" "after"
    }
    finally {
        Pop-Location
    }
}

# Additional convenience functions
function workflow-help {
    <#
    .SYNOPSIS
    Display workflow function help
    
    .DESCRIPTION
    Shows available workflow commands and their purposes
    #>
    Write-Host ""
    Write-Host "STM32H753ZI Development Workflow Commands" -ForegroundColor Magenta
    Write-Host "=========================================" -ForegroundColor Magenta
    Write-Host ""
    Write-Host "🚀 workflow-before   " -ForegroundColor Green -NoNewline
    Write-Host "- Check feature status, validate environment"
    Write-Host "🔄 workflow-during   " -ForegroundColor Yellow -NoNewline
    Write-Host "- Build project, update STATUS.md"
    Write-Host "✅ workflow-after    " -ForegroundColor Cyan -NoNewline
    Write-Host "- Update features, prepare commit context"
    Write-Host "❓ workflow-help     " -ForegroundColor White -NoNewline
    Write-Host "- Display this help"
    Write-Host "📊 workflow-status   " -ForegroundColor Blue -NoNewline
    Write-Host "- Show current feature status"
    Write-Host ""
    Write-Host "VS Code Tasks (Ctrl+Shift+P -> 'Tasks: Run Task'):" -ForegroundColor Gray
    Write-Host "  🚀 WORKFLOW: Before Work"
    Write-Host "  🔄 WORKFLOW: During Work (Build + Update)"  
    Write-Host "  ✅ WORKFLOW: After Work (Prepare Commit)"
    Write-Host ""
}

function workflow-status {
    <#
    .SYNOPSIS
    Show current feature and project status
    
    .DESCRIPTION
    Displays current feature tracking status and project state
    #>
    Push-Location $ProjectRoot
    try {
        Write-Host "📊 Current Project Status..." -ForegroundColor Blue
        
        # Activate virtual environment if it exists
        if (Test-Path "$ProjectRoot\.venv\Scripts\Activate.ps1") {
            & "$ProjectRoot\.venv\Scripts\Activate.ps1"
        }
        
        # Show feature status
        if (Test-Path "$ProjectRoot\scripts\feature_tracker.py") {
            Write-Host "`n🔍 Feature Status:" -ForegroundColor Yellow
            $RunPython = Join-Path $ProjectRoot "scripts\run_python.ps1"
            & $RunPython "scripts\feature_tracker.py" "list" "--status" "IN_PROGRESS"
        }
        
        # Show STATUS.md summary
        if (Test-Path "$ProjectRoot\STATUS.md") {
            Write-Host "`n📋 STATUS.md Summary:" -ForegroundColor Green
            Get-Content "$ProjectRoot\STATUS.md" | Select-Object -First 20
        }
    }
    finally {
        Pop-Location
    }
}

# Export functions for module loading
Export-ModuleMember -Function workflow-before, workflow-during, workflow-after, workflow-help, workflow-status

# Auto-display help when script is dot-sourced
if ($MyInvocation.InvocationName -eq '.') {
    workflow-help
}

Write-Host "✅ Workflow functions loaded. Type 'workflow-help' for usage." -ForegroundColor Green
