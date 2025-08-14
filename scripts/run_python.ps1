# Cross-platform Python script runner for VS Code tasks (PowerShell version)
# Usage: ./scripts/run_python.ps1 script_name.py [args...]

param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$ScriptPath,
    
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$Arguments
)

# Get the workspace root
$WorkspaceRoot = Split-Path -Parent $PSScriptRoot

# Find Python executable using built-in automatic variables
if ($IsWindows -or $env:OS -eq "Windows_NT") {
    $PythonPaths = @(
        Join-Path $WorkspaceRoot ".venv\Scripts\python.exe"
        Join-Path $WorkspaceRoot ".venv\bin\python.exe"
        Join-Path $WorkspaceRoot ".venv\bin\python"
    )
}
else {
    $PythonPaths = @(
        Join-Path $WorkspaceRoot ".venv/bin/python"
        Join-Path $WorkspaceRoot ".venv/Scripts/python.exe"
    )
}

# Find the first existing Python executable
$PythonExe = $null
foreach ($PythonPath in $PythonPaths) {
    if (Test-Path $PythonPath) {
        $PythonExe = $PythonPath
        break
    }
}

# Better fallback using full paths
if (-not $PythonExe) {
    # Try to find Python in common locations
    $SystemPythonPaths = @()
    if ($IsWindows -or $env:OS -eq "Windows_NT") {
        $SystemPythonPaths = @(
            "C:\Python\python.exe"
            "C:\Python311\python.exe"
            "C:\Python312\python.exe"
            "C:\Python313\python.exe"
            "$env:LOCALAPPDATA\Programs\Python\Python311\python.exe"
            "$env:LOCALAPPDATA\Programs\Python\Python312\python.exe"
            "$env:LOCALAPPDATA\Programs\Python\Python313\python.exe"
        )
    }
    else {
        $SystemPythonPaths = @(
            "/usr/bin/python3"
            "/usr/bin/python"
            "/usr/local/bin/python3"
            "/usr/local/bin/python"
        )
    }
    
    foreach ($SystemPath in $SystemPythonPaths) {
        if (Test-Path $SystemPath) {
            $PythonExe = $SystemPath
            break
        }
    }
    
    # Final fallback
    if (-not $PythonExe) {
        $PythonExe = if ($IsWindows -or $env:OS -eq "Windows_NT") { "python.exe" } else { "python3" }
    }
}

# Run the Python script with all arguments
$AllArgs = @($ScriptPath) + $Arguments
& $PythonExe @AllArgs
