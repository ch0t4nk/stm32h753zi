# Workflow Configuration Aware PowerShell Script
# Uses SSOT configuration to determine complexity level

param(
    [Parameter(Mandatory = $true)]
    [string]$PythonScript,
    
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$Arguments = @()
)

# Get workflow configuration
$WorkflowMode = $env:STM32_WORKFLOW_MODE
if (-not $WorkflowMode) {
    # Check for config file
    $ConfigFile = ".\.workflow_config"
    if (Test-Path $ConfigFile) {
        try {
            $Config = Get-Content $ConfigFile | ConvertFrom-Json
            $WorkflowMode = $Config.mode
        }
        catch {
            $WorkflowMode = "standard"  # Default fallback
        }
    }
    else {
        $WorkflowMode = "standard"  # Default
    }
}

Write-Host "Workflow Mode: $WorkflowMode" -ForegroundColor Cyan

# Determine validation level based on mode
$ValidateVenv = $true
$ValidatePython = $true
$VerboseLogging = $false
$ValidationTimeout = 500

switch ($WorkflowMode.ToLower()) {
    "minimal" {
        $ValidateVenv = $false
        $ValidatePython = $false
        $VerboseLogging = $false
        $ValidationTimeout = 100
        Write-Host "Fast mode - minimal validation" -ForegroundColor Green
    }
    "standard" {
        $ValidateVenv = $true
        $ValidatePython = $true
        $VerboseLogging = $false
        $ValidationTimeout = 500
        Write-Host "Standard mode - balanced performance" -ForegroundColor Blue
    }
    "thorough" {
        $ValidateVenv = $true
        $ValidatePython = $true
        $VerboseLogging = $true
        $ValidationTimeout = 2000
        Write-Host "Thorough mode - full validation" -ForegroundColor Yellow
    }
    "debug" {
        $ValidateVenv = $true
        $ValidatePython = $true
        $VerboseLogging = $true
        $ValidationTimeout = 5000
        Write-Host "Debug mode - maximum verbosity" -ForegroundColor Magenta
    }
    default {
        Write-Host "Unknown mode '$WorkflowMode', using standard" -ForegroundColor Yellow
    }
}

# Fast path for minimal mode
if ($WorkflowMode -eq "minimal") {
    if ($VerboseLogging) { Write-Host "Minimal mode - skipping all validation" -ForegroundColor Green }
    
    # Try direct Python execution
    $PythonPaths = @(
        ".\.venv\Scripts\python.exe",
        ".\.venv\bin\python",
        "python"
    )
    
    foreach ($PythonPath in $PythonPaths) {
        try {
            if ($VerboseLogging) { Write-Host "Trying: $PythonPath" -ForegroundColor Gray }
            & $PythonPath $PythonScript @Arguments
            if ($LASTEXITCODE -eq 0) {
                exit 0
            }
        }
        catch {
            # Continue to next option
        }
    }
    
    Write-Host "All Python execution attempts failed" -ForegroundColor Red
    exit 1
}

# Standard/Thorough/Debug modes with validation
$StartTime = Get-Date

# Validate virtual environment if enabled
if ($ValidateVenv) {
    if ($VerboseLogging) { Write-Host "Validating virtual environment..." -ForegroundColor Gray }
    
    $VenvPaths = @(".\.venv", ".\venv")
    $VenvFound = $false
    
    foreach ($VenvPath in $VenvPaths) {
        if (Test-Path $VenvPath) {
            if ($VerboseLogging) { Write-Host "Found virtual environment: $VenvPath" -ForegroundColor Green }
            $VenvFound = $true
            break
        }
    }
    
    if (-not $VenvFound) {
        Write-Host "Virtual environment not found" -ForegroundColor Red
        if ($WorkflowMode -eq "thorough" -or $WorkflowMode -eq "debug") {
            Write-Host "Create with: python -m venv .venv" -ForegroundColor Blue
            exit 1
        }
    }
}

# Find Python executable
$PythonExe = $null

if ($ValidatePython) {
    if ($VerboseLogging) { Write-Host "Detecting Python executable..." -ForegroundColor Gray }
    
    $PythonCandidates = @(
        ".\.venv\Scripts\python.exe",  # Windows venv
        ".\.venv\bin\python",          # Linux/macOS venv
        "python",                      # System Python
        "python3"                      # System Python3
    )
    
    foreach ($Candidate in $PythonCandidates) {
        try {
            if (Test-Path $Candidate -ErrorAction SilentlyContinue) {
                $VersionOutput = & $Candidate --version 2>&1
                if ($LASTEXITCODE -eq 0) {
                    $PythonExe = $Candidate
                    if ($VerboseLogging) { 
                        Write-Host "Found Python: $Candidate ($VersionOutput)" -ForegroundColor Green 
                    }
                    break
                }
            }
        }
        catch {
            # Continue to next candidate
        }
    }
    
    if (-not $PythonExe) {
        Write-Host "No working Python executable found" -ForegroundColor Red
        if ($WorkflowMode -eq "thorough" -or $WorkflowMode -eq "debug") {
            Write-Host "Tried: $($PythonCandidates -join ', ')" -ForegroundColor Blue
            exit 1
        }
        else {
            # Fallback to basic python command
            $PythonExe = "python"
        }
    }
}
else {
    # Skip validation - use first available
    $PythonExe = ".\.venv\Scripts\python.exe"
    if (-not (Test-Path $PythonExe)) {
        $PythonExe = ".\.venv\bin\python"
        if (-not (Test-Path $PythonExe)) {
            $PythonExe = "python"
        }
    }
}

# Check validation timeout
$ElapsedMs = ((Get-Date) - $StartTime).TotalMilliseconds
if ($ElapsedMs -gt $ValidationTimeout) {
    Write-Host "WARNING: Validation took ${ElapsedMs}ms (timeout: ${ValidationTimeout}ms)" -ForegroundColor Orange
}

# Execute Python script
if ($VerboseLogging) { 
    Write-Host "EXECUTING: $PythonExe $PythonScript $($Arguments -join ' ')" -ForegroundColor Cyan 
}

try {
    & $PythonExe $PythonScript @Arguments
    $ExitCode = $LASTEXITCODE
    
    if ($VerboseLogging) {
        $TotalMs = ((Get-Date) - $StartTime).TotalMilliseconds
        Write-Host "COMPLETED in ${TotalMs}ms (exit code: $ExitCode)" -ForegroundColor Green
    }
    
    exit $ExitCode
}
catch {
    Write-Host "ERROR: Failed to execute Python script: $_" -ForegroundColor Red
    exit 1
}
