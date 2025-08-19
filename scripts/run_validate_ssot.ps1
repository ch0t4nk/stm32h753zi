# Wrapper to run SSOT validator via venv Python to avoid Windows 'Open with' dialogs
param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$Args
)

$WorkspaceRoot = Split-Path -Parent $PSScriptRoot

# Use the centralized runner which prefers the workspace venv Python
$RunPython = Join-Path $WorkspaceRoot "scripts\run_python.ps1"

if (Test-Path $RunPython) {
    & $RunPython "scripts\validate_ssot.py" @Args
    exit $LASTEXITCODE
}
else {
    Write-Host "Runner not found at $RunPython - attempting to locate python on PATH as fallback" -ForegroundColor Yellow
    $PythonCmd = $null
    try {
        $PythonCmd = (Get-Command python -ErrorAction SilentlyContinue).Source
    }
    catch {
        $PythonCmd = $null
    }

    if ($PythonCmd) {
        Write-Host "Using system python: $PythonCmd" -ForegroundColor Gray
        & $PythonCmd (Join-Path $WorkspaceRoot "scripts\validate_ssot.py") @Args
        exit $LASTEXITCODE
    }
    else {
        Write-Host "No python executable found on PATH and runner missing. Please create a venv or install Python." -ForegroundColor Red
        exit 2
    }
}
