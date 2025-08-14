# STM32H753ZI Workflow Function Loader
# Run this script to enable workflow commands: workflow-before, workflow-during, workflow-after

# Import the workflow module
$ModulePath = Join-Path $PSScriptRoot "STM32Workflow_Clean.psm1"

if (Test-Path $ModulePath) {
    Import-Module $ModulePath -Force
}
else {
    Write-Error "Workflow module not found at: $ModulePath"
}

# Create convenient aliases
Set-Alias -Name wf-before -Value workflow-before -Force
Set-Alias -Name wf-during -Value workflow-during -Force
Set-Alias -Name wf-after -Value workflow-after -Force
Set-Alias -Name wf-help -Value workflow-help -Force
Set-Alias -Name wf-status -Value workflow-status -Force

Write-Host "Short aliases available: wf-before, wf-during, wf-after, wf-help, wf-status" -ForegroundColor DarkGreen
