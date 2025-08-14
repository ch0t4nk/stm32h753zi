# PowerShell Profile Integration for STM32H753ZI Workflow
# Add this to your PowerShell profile or run manually to load workflow functions

# Get the project root directory
$ScriptPath = $PSScriptRoot
if (-not $ScriptPath) {
    # If running interactively, try to detect project location
    if (Test-Path "scripts\workflow_functions.ps1") {
        $ScriptPath = Get-Location
    }
    elseif (Test-Path "..\scripts\workflow_functions.ps1") {
        $ScriptPath = Resolve-Path ".."
    }
    else {
        Write-Warning "Could not detect STM32H753ZI project location. Please run from project root."
        return
    }
}

# Fix path construction - $PSScriptRoot already points to scripts directory
if ($PSScriptRoot) {
    $WorkflowFunctionsPath = Join-Path $PSScriptRoot "workflow_functions_clean.ps1"
}
else {
    $WorkflowFunctionsPath = Join-Path $ScriptPath "scripts\workflow_functions_clean.ps1"
}

if (Test-Path $WorkflowFunctionsPath) {
    # Dot-source the workflow functions
    . $WorkflowFunctionsPath
    Write-Host "✅ STM32H753ZI workflow functions loaded!" -ForegroundColor Green
    Write-Host "   Available commands: workflow-before, workflow-during, workflow-after, workflow-help, workflow-status" -ForegroundColor Gray
}
else {
    Write-Warning "Workflow functions script not found at: $WorkflowFunctionsPath"
}

# Optional: Add project-specific aliases
Set-Alias -Name wf-before -Value workflow-before -Force
Set-Alias -Name wf-during -Value workflow-during -Force  
Set-Alias -Name wf-after -Value workflow-after -Force
Set-Alias -Name wf-help -Value workflow-help -Force
Set-Alias -Name wf-status -Value workflow-status -Force

Write-Host "   Short aliases: wf-before, wf-during, wf-after, wf-help, wf-status" -ForegroundColor Gray
