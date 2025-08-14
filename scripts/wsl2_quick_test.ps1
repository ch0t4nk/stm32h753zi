# WSL2 Quick Test Runner for STM32H753ZI Project
# Purpose: Run working host tests in WSL2 (bypasses Windows compiler issues)
# Usage: .\scripts\wsl2_quick_test.ps1

param(
    [switch]$Verbose
)

Write-Host "=== WSL2 Quick Host Test Runner ===" -ForegroundColor Cyan
Write-Host "Running Phase 1.3 validation test in WSL2..." -ForegroundColor Yellow

# Check if WSL2 test environment exists
$testExists = wsl -d Ubuntu -- bash -c "test -f ~/nucleo_test/host_tests/build/test_phase1_3_validation_host && echo 'EXISTS' || echo 'MISSING'"

if ($testExists.Trim() -eq "MISSING") {
    Write-Host "WSL2 test environment not found. Running full setup..." -ForegroundColor Yellow
    & .\scripts\build_wsl2_host_tests.ps1
    exit $LASTEXITCODE
}

# Run the existing test
Write-Host "Running existing WSL2 host test..." -ForegroundColor Green
wsl -d Ubuntu -- bash -c "cd ~/nucleo_test/host_tests/build && ./test_phase1_3_validation_host"

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "SUCCESS: WSL2 host tests completed!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Quick Commands:" -ForegroundColor Yellow
    Write-Host "  .\scripts\wsl2_quick_test.ps1                     # Quick test run" -ForegroundColor White
    Write-Host "  .\scripts\build_wsl2_host_tests.ps1               # Full rebuild" -ForegroundColor White
    Write-Host "  wsl -d Ubuntu -- 'cd ~/nucleo_test/host_tests/build && ./test_phase1_3_validation_host'" -ForegroundColor White
}
else {
    Write-Host "ERROR: WSL2 host test failed" -ForegroundColor Red
    exit 1
}
