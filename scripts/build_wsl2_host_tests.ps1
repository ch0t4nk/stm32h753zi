# WSL2 Host Testing Script for STM32H753ZI Project
# Purpose: Build and run host tests in WSL2 Linux environment for GCC compatibility
# Usage: .\scripts\build_wsl2_host_tests.ps1

param(
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

Write-Host "=== WSL2 Host Testing for STM32H753ZI ===" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

# Check WSL2 availability
Write-Host "Checking WSL2 availability..." -ForegroundColor Yellow
$wslCheck = wsl --list --quiet 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: WSL2 not available on this system" -ForegroundColor Red
    Write-Host "Please install WSL2 and Ubuntu distribution first" -ForegroundColor Yellow
    exit 1
}

Write-Host "WSL2 available" -ForegroundColor Green

# Ensure build tools are installed in WSL2
Write-Host "Installing build tools in WSL2..." -ForegroundColor Yellow
wsl -d Ubuntu -- bash -c "sudo apt update && sudo apt install -y build-essential cmake ninja-build"

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to install build tools in WSL2" -ForegroundColor Red
    exit 1
}

Write-Host "Build tools installed" -ForegroundColor Green

# Create WSL2 native build environment
Write-Host "Setting up WSL2 build environment..." -ForegroundColor Yellow

wsl -d Ubuntu -- bash -c "cd ~ && rm -rf nucleo_test && mkdir -p nucleo_test"
wsl -d Ubuntu -- bash -c "cd ~/nucleo_test && cp -r '/mnt/c/repos/Nucleo-H753ZI Project/code/host_tests' ."
wsl -d Ubuntu -- bash -c "cd ~/nucleo_test && cp -r '/mnt/c/repos/Nucleo-H753ZI Project/code/src' ."
wsl -d Ubuntu -- bash -c "cd ~/nucleo_test && cp -r '/mnt/c/repos/Nucleo-H753ZI Project/code/tests' ."
wsl -d Ubuntu -- bash -c "cd ~/nucleo_test && cp -r '/mnt/c/repos/Nucleo-H753ZI Project/code/external' ."

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to setup WSL2 environment" -ForegroundColor Red
    exit 1
}

Write-Host "WSL2 environment ready" -ForegroundColor Green

# Build host tests in WSL2
Write-Host "Building host tests in WSL2..." -ForegroundColor Yellow

wsl -d Ubuntu -- bash -c "cd ~/nucleo_test/host_tests && mkdir -p build"
wsl -d Ubuntu -- bash -c "cd ~/nucleo_test/host_tests/build && cmake .."
wsl -d Ubuntu -- bash -c "cd ~/nucleo_test/host_tests/build && make -j"

if ($LASTEXITCODE -eq 0) {
    Write-Host "WSL2 host test build successful!" -ForegroundColor Green
    
    # Run tests in WSL2
    Write-Host "Running WSL2 host tests..." -ForegroundColor Yellow
    
    wsl -d Ubuntu -- bash -c "cd ~/nucleo_test/host_tests/build && ls -la *host"
    wsl -d Ubuntu -- bash -c "cd ~/nucleo_test/host_tests/build && ./test_phase1_3_validation_host"
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host ""
        Write-Host "SUCCESS: WSL2 host tests completed!" -ForegroundColor Green
        Write-Host ""
        Write-Host "WSL2 Host Testing Commands:" -ForegroundColor Yellow
        Write-Host "  .\scripts\build_wsl2_host_tests.ps1  # Full rebuild and test" -ForegroundColor White
        Write-Host "  wsl -d Ubuntu -- bash -c 'cd ~/nucleo_test/host_tests/build && ./test_phase1_3_validation_host'" -ForegroundColor White
    }
    else {
        Write-Host "ERROR: WSL2 host tests failed" -ForegroundColor Red
        exit 1
    }
}
else {
    Write-Host "ERROR: WSL2 host test build failed" -ForegroundColor Red
    exit 1
}
