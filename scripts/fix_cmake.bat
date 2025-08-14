@echo off
REM STM32H753ZI CMake Quick Fix - Universal Windows Launcher
REM This batch file detects the environment and calls the appropriate script

echo 🔧 STM32H753ZI CMake Quick Fix (Universal Launcher)
echo ================================================

REM Check if PowerShell is available and prefer it
where powershell.exe >nul 2>&1
if %errorlevel% == 0 (
    echo 🪟 Using PowerShell version...
    powershell.exe -ExecutionPolicy Bypass -File "scripts\fix_cmake.ps1" %*
    exit /b %errorlevel%
)

REM Check if bash is available (Git Bash, WSL, etc.)
where bash.exe >nul 2>&1
if %errorlevel% == 0 (
    echo 🐧 Using bash version...
    bash.exe "scripts/fix_cmake.sh" %*
    exit /b %errorlevel%
)

REM Check if sh is available
where sh.exe >nul 2>&1
if %errorlevel% == 0 (
    echo 🐚 Using sh version...
    sh.exe "scripts/fix_cmake.sh" %*
    exit /b %errorlevel%
)

echo ❌ No compatible shell found (PowerShell, bash, or sh)
echo 💡 Please install PowerShell or Git Bash
pause
exit /b 1
