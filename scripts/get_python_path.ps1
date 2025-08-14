# Cross-platform Python path detector for VS Code tasks
# Returns the correct Python executable path for the current platform

param(
    [string]$WorkspaceRoot = $PWD
)

# Define potential Python paths based on platform (using built-in automatic variables)
if ($IsWindows) {
    $PythonPaths = @(
        Join-Path $WorkspaceRoot ".venv\Scripts\python.exe"
        Join-Path $WorkspaceRoot ".venv\bin\python.exe"     # WSL-style
        Join-Path $WorkspaceRoot ".venv\bin\python"         # Unix-style fallback
    )
}
else {
    $PythonPaths = @(
        Join-Path $WorkspaceRoot ".venv/bin/python"
        Join-Path $WorkspaceRoot ".venv/Scripts/python.exe" # Windows-style fallback
    )
}

# Find the first existing Python executable
foreach ($PythonPath in $PythonPaths) {
    if (Test-Path $PythonPath) {
        Write-Output $PythonPath
        exit 0
    }
}

# Fallback to system Python
if ($IsWindows) {
    Write-Output "python.exe"
}
else {
    Write-Output "python"
}
