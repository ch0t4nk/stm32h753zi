#!/usr/bin/env pwsh
# Simple cross-platform Python runner
# Just finds Python and runs the command - no complexity

param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$ScriptPath,
    
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$Arguments
)

# Simple platform detection
$IsWindowsOS = $IsWindows -or ($PSVersionTable.PSVersion.Major -lt 6)

# Simple Python path detection - no virtual environment complexity
if ($IsWindowsOS) {
    $PythonPaths = @(
        ".\.venv\Scripts\python.exe",
        ".\.venv\bin\python",
        "python",
        "python3"
    )
}
else {
    $PythonPaths = @(
        "./.venv/bin/python",
        "./.venv/Scripts/python.exe", 
        "python3",
        "python"
    )
}

# Find first working Python
$PythonCmd = $null
foreach ($Path in $PythonPaths) {
    if (Get-Command $Path -ErrorAction SilentlyContinue) {
        $PythonCmd = $Path
        break
    }
}

if (-not $PythonCmd) {
    Write-Error "No Python interpreter found. Tried: $($PythonPaths -join ', ')"
    exit 1
}

# Just run it - no fancy error handling
& $PythonCmd $ScriptPath @Arguments
