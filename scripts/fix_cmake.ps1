# STM32H753ZI CMake Quick Fix Script (Windows PowerShell)
# Purpose: Rapidly recover from CMake configuration issues on Windows
# Usage: .\scripts\fix_cmake.ps1 [-BuildType <ARM|Host|Both|WSL2>] [-Force] [-Verbose]

param(
    [ValidateSet("ARM", "Host", "Both", "WSL2")]
    [string]$BuildType = "ARM",
    [switch]$Force,
    [switch]$Verbose
)

$ErrorActionPr        if ($LASTEXITCODE -eq 0) {
    Write-Host "WSL2 host test build successful!" -ForegroundColor Green
            
    Write-Host "Running WSL2 host tests..." -ForegroundColor Yellowence = "Stop"

    Write-Host "🔧 STM32H753ZI CMake Quick Fix (Windows)" -ForegroundColor Cyan
    Write-Host "=======================================" -ForegroundColor Cyan
    Write-Host "Build Type: $BuildType (ARM=Firmware, Host=Unit Tests, Both=Dual Build, WSL2=Host Tests in WSL2)" -ForegroundColor Yellow

    # 0. OS Detection - Auto-redirect to bash version if on WSL/Linux
    if ($env:WSL_DISTRO_NAME -or $env:SHELL -or (Test-Path "/usr/bin/bash")) {
        Write-Host "🐧 WSL/Linux environment detected - redirecting to bash version..." -ForegroundColor Yellow
        if (Test-Path "scripts/fix_cmake.sh") {
            & bash "scripts/fix_cmake.sh"
            exit $LASTEXITCODE
        }
        else {
            Write-Host "❌ bash version not found at scripts/fix_cmake.sh" -ForegroundColor Red
            exit 1
        }
    }

    Write-Host "🪟 Windows environment confirmed" -ForegroundColor Green

    # 1. CRITICAL: Ensure virtual environment is activated
    Write-Host "🐍 Checking Python virtual environment..." -ForegroundColor Yellow
    if (-not $env:VIRTUAL_ENV) {
        Write-Host "❌ Virtual environment not activated!" -ForegroundColor Red
        Write-Host "📋 Activating virtual environment..." -ForegroundColor Yellow
    
        if (Test-Path ".venv\Scripts\Activate.ps1") {
            & ".\.venv\Scripts\Activate.ps1"
            Write-Host "✅ Virtual environment activated: $env:VIRTUAL_ENV" -ForegroundColor Green
        }
        elseif (Test-Path ".venv\bin\activate") {
            # WSL/Git Bash style paths in Windows
            Write-Host "📋 Found Unix-style venv, trying alternative activation..."
            $env:VIRTUAL_ENV = (Resolve-Path ".venv").Path
            $env:PATH = "$env:VIRTUAL_ENV\Scripts;$env:PATH"
            Write-Host "✅ Virtual environment activated: $env:VIRTUAL_ENV" -ForegroundColor Green
        }
        else {
            Write-Host "❌ Virtual environment not found at .venv/" -ForegroundColor Red
            Write-Host "📋 Creating virtual environment..." -ForegroundColor Yellow
            python -m venv .venv
            & ".\.venv\Scripts\Activate.ps1"
            Write-Host "✅ Virtual environment created and activated: $env:VIRTUAL_ENV" -ForegroundColor Green
        }
    }
    else {
        Write-Host "✅ Virtual environment already active: $env:VIRTUAL_ENV" -ForegroundColor Green
    }

    # Ensure Python dependencies are installed
    Write-Host "📦 Checking Python dependencies..." -ForegroundColor Yellow
    try {
        python -c "import chromadb, requests" 2>$null
        Write-Host "✅ Python dependencies available" -ForegroundColor Green
    }
    catch {
        Write-Host "📦 Installing required Python packages..." -ForegroundColor Yellow
        pip install --quiet chromadb requests ollama
        Write-Host "✅ Python dependencies installed" -ForegroundColor Green
    }

    # Setup Ollama service and model
    Write-Host "🤖 Checking Ollama service and model setup..." -ForegroundColor Yellow
    $ollamaProcess = Get-Process -Name "ollama" -ErrorAction SilentlyContinue
    if (-not $ollamaProcess) {
        Write-Host "🚀 Starting Ollama service..." -ForegroundColor Yellow
        Start-Process -FilePath "ollama" -ArgumentList "serve" -WindowStyle Hidden
        Start-Sleep -Seconds 3
        Write-Host "✅ Ollama service started" -ForegroundColor Green
    }
    else {
        Write-Host "✅ Ollama service already running" -ForegroundColor Green
    }

    # Check if required model is available
    Write-Host "📥 Checking for mxbai-embed-large model..." -ForegroundColor Yellow
    $modelList = ollama list 2>$null
    if ($modelList -and ($modelList -like "*mxbai-embed-large*")) {
        Write-Host "✅ mxbai-embed-large model already available" -ForegroundColor Green
    }
    else {
        Write-Host "⬇️  Downloading mxbai-embed-large model (this may take a few minutes)..." -ForegroundColor Yellow
        ollama pull mxbai-embed-large
        Write-Host "✅ Model downloaded successfully" -ForegroundColor Green
    }

    # Test Ollama connection
    Write-Host "🔍 Testing Ollama connection..." -ForegroundColor Yellow
    try {
        $response = Invoke-RestMethod -Uri "http://localhost:11434/api/tags" -TimeoutSec 5
        Write-Host "✅ Ollama API accessible" -ForegroundColor Green
    }
    catch {
        Write-Host "⚠️  Ollama API not accessible - semantic search will use mock embeddings" -ForegroundColor Yellow
    }

    # 2. Verify required tools based on build type
    Write-Host "📋 Checking tools for build type: $BuildType..." -ForegroundColor Yellow

    $STM32CubeCLT_Base = "C:\ST\STM32CubeCLT_1.19.0"
    $RequiredTools = @{}
    $MissingTools = @()
    $AvailableTools = @()
    $HostCompiler = $null

    if ($BuildType -eq "ARM" -or $BuildType -eq "Both") {
        Write-Host "🔧 ARM Firmware Build Requirements:" -ForegroundColor Cyan
        $RequiredTools += @{
            "cmake"                = "$STM32CubeCLT_Base\CMake\bin\cmake.exe"
            "arm-none-eabi-gcc"    = "$STM32CubeCLT_Base\GNU-tools-for-STM32\bin\arm-none-eabi-gcc.exe"
            "ninja"                = "$STM32CubeCLT_Base\Ninja\bin\ninja.exe"
            "STM32_Programmer_CLI" = "$STM32CubeCLT_Base\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
        }
    }

    if ($BuildType -eq "Host" -or $BuildType -eq "Both") {
        Write-Host "🧪 Host Testing Build Requirements:" -ForegroundColor Cyan
    
        # Check for available compilers (prefer Visual Studio on Windows)
        $hostMsvc = Get-Command cl -ErrorAction SilentlyContinue
        $hostGcc = Get-Command gcc -ErrorAction SilentlyContinue
    
        if ($hostMsvc) {
            Write-Host "✅ Visual Studio compiler (cl): Found at $($hostMsvc.Source)" -ForegroundColor Green
            $AvailableTools += "cl"
            $HostCompiler = "MSVC"
        }
        elseif ($hostGcc) {
            Write-Host "✅ GCC compiler (gcc): Found at $($hostGcc.Source)" -ForegroundColor Green
            $AvailableTools += "gcc"
            $HostCompiler = "GCC"
        }
        else {
            Write-Host "❌ No suitable compiler found for host testing" -ForegroundColor Red
            Write-Host "   Need either Visual Studio (cl.exe) or GCC (gcc.exe)" -ForegroundColor Yellow
            $MissingTools += "compiler"
        }
    
        $hostCmake = Get-Command cmake -ErrorAction SilentlyContinue
        if (-not $hostCmake) {
            $RequiredTools += @{
                "cmake" = "$STM32CubeCLT_Base\CMake\bin\cmake.exe"
            }
        }
    }

    foreach ($tool in $RequiredTools.GetEnumerator()) {
        if (Test-Path $tool.Value) {
            $AvailableTools += $tool.Key
            Write-Host "✅ $($tool.Key): $($tool.Value)" -ForegroundColor Green
        }
        else {
            $MissingTools += $tool.Key
            Write-Host "❌ $($tool.Key): NOT FOUND at $($tool.Value)" -ForegroundColor Red
        }
    }

    # Check tools in PATH as fallback
    foreach ($missingTool in $MissingTools.Clone()) {
        $pathTool = Get-Command $missingTool -ErrorAction SilentlyContinue
        if ($pathTool) {
            Write-Host "✅ ${missingTool}: Found in PATH at $($pathTool.Source)" -ForegroundColor Green
            $MissingTools = $MissingTools | Where-Object { $_ -ne $missingTool }
            $AvailableTools += $missingTool
        }
    }

    if ($MissingTools.Count -gt 0) {
        Write-Host "❌ Missing critical tools: $($MissingTools -join ', ')" -ForegroundColor Red
        Write-Host "💡 Please install STM32CubeCLT or ensure tools are in PATH" -ForegroundColor Yellow
    
        # Check for ninja specifically - can use Make as fallback
        if ($MissingTools -contains "ninja") {
            $MissingTools = $MissingTools | Where-Object { $_ -ne "ninja" }
            Write-Host "⚠️  Ninja not available - will use Make generator as fallback" -ForegroundColor Yellow
            $UseMakeFallback = $true
        }
    
        if ($MissingTools.Count -gt 0) {
            Write-Host "💀 FATAL: Critical tools missing: $($MissingTools -join ', ')" -ForegroundColor Red
            Write-Host "📋 Please install STM32CubeCLT from: https://www.st.com/en/development-tools/stm32cubeclt.html" -ForegroundColor Yellow
            exit 1
        }
    }
    else {
        Write-Host "✅ All required tools available" -ForegroundColor Green
        $UseMakeFallback = $false
    }

    # 3. Verify workspace files
    Write-Host "📋 Checking workspace configuration..." -ForegroundColor Yellow
    $RequiredFiles = @(
        "cmake\gcc-arm-none-eabi.cmake",
        "CMakePresets.json",
        "CMakeLists.txt"
    )

    foreach ($file in $RequiredFiles) {
        if (Test-Path $file) {
            Write-Host "✅ $file" -ForegroundColor Green
        }
        else {
            Write-Host "❌ Required file missing: $file" -ForegroundColor Red
            Write-Host "   Please ensure you're in the correct workspace directory" -ForegroundColor Yellow
            exit 1
        }
    }
    Write-Host "✅ All required workspace files present" -ForegroundColor Green

    # 4. Clean build directories based on build type
    Write-Host "🧹 Cleaning build directories for: $BuildType..." -ForegroundColor Yellow

    if ($BuildType -eq "ARM" -or $BuildType -eq "Both") {
        if (Test-Path "build") {
            Remove-Item -Recurse -Force "build"
            Write-Host "✅ ARM build directory cleaned (build/)" -ForegroundColor Green
        }
        else {
            Write-Host "✅ ARM build directory already clean" -ForegroundColor Green
        }
    }

    if ($BuildType -eq "Host" -or $BuildType -eq "Both") {
        if (Test-Path "build_host_tests") {
            Remove-Item -Recurse -Force "build_host_tests"
            Write-Host "✅ Host test build directory cleaned (build_host_tests/)" -ForegroundColor Green
        }
        else {
            Write-Host "✅ Host test build directory already clean" -ForegroundColor Green
        }
    }

    # 5. Remove VS Code CMake cache (if present)
    if (Test-Path ".vscode\cmake-tools-kits.json") {
        Write-Host "🧹 Cleaning VS Code CMake cache..." -ForegroundColor Yellow
        Remove-Item ".vscode\cmake-tools-kits.json" -Force
        Write-Host "✅ VS Code CMake cache cleaned" -ForegroundColor Green
    }

    # 6. Configure and build based on build type
    Write-Host "⚙️ Configuring and building: $BuildType..." -ForegroundColor Yellow

    $BuildSuccess = $true

    # ARM Firmware Build
    if ($BuildType -eq "ARM" -or $BuildType -eq "Both") {
        Write-Host "🔧 === ARM FIRMWARE BUILD ===" -ForegroundColor Cyan
    
        if ($UseMakeFallback) {
            Write-Host "📋 Using Make generator (ninja not available)..." -ForegroundColor Yellow
            $configResult = cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✅ ARM CMake configuration (Make) successful" -ForegroundColor Green
                $ArmBuildDir = "build"
            }
            else {
                Write-Host "❌ ARM CMake configuration failed" -ForegroundColor Red
                $BuildSuccess = $false
            }
        }
        else {
            Write-Host "📋 Trying Ninja generator first..." -ForegroundColor Yellow
            $configResult = cmake --preset Debug
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✅ ARM CMake configuration (Ninja) successful" -ForegroundColor Green
                $ArmBuildDir = "build\Debug"
            }
            else {
                Write-Host "❌ Ninja preset failed, trying Make fallback..." -ForegroundColor Yellow
                $configResult = cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
                if ($LASTEXITCODE -eq 0) {
                    Write-Host "✅ ARM CMake configuration (Make fallback) successful" -ForegroundColor Green
                    $ArmBuildDir = "build"
                }
                else {
                    Write-Host "❌ ARM configuration failed" -ForegroundColor Red
                    $BuildSuccess = $false
                }
            }
        }
    
        if ($BuildSuccess -and $ArmBuildDir) {
            Write-Host "� Building ARM firmware..." -ForegroundColor Yellow
            $buildResult = cmake --build $ArmBuildDir
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✅ ARM firmware build successful!" -ForegroundColor Green
            
                # Show build artifact
                $elfFile = "$ArmBuildDir\stm32h753_ihm02a1.elf"
                if (Test-Path $elfFile) {
                    Write-Host "📦 Generated ARM firmware: $elfFile" -ForegroundColor Green
                    if (Get-Command arm-none-eabi-size -ErrorAction SilentlyContinue) {
                        arm-none-eabi-size $elfFile
                    }
                }
            }
            else {
                Write-Host "❌ ARM firmware build failed" -ForegroundColor Red
                $BuildSuccess = $false
            }
        }
    }

    # Host Testing Build  
    if ($BuildType -eq "Host" -or $BuildType -eq "Both" -or $BuildType -eq "WSL2") {
    
        # WSL2 Host Testing (Preferred for GCC compatibility)
        if ($BuildType -eq "WSL2") {
            Write-Host "=== WSL2 HOST TESTING BUILD ===" -ForegroundColor Cyan
        
            # Check WSL2 availability
            $wslCheck = wsl --list --quiet 2>$null
            if ($LASTEXITCODE -ne 0) {
                Write-Host "❌ WSL2 not available on this system" -ForegroundColor Red
                $BuildSuccess = $false
                return
            }
        
            Write-Host "Setting up WSL2 host testing environment..." -ForegroundColor Yellow
        
            # Ensure build tools are installed in WSL2
            Write-Host "Installing build tools in WSL2..." -ForegroundColor Yellow
            wsl -d Ubuntu -- bash -c "sudo apt update && sudo apt install -y build-essential cmake ninja-build"
        
            if ($LASTEXITCODE -ne 0) {
                Write-Host "❌ Failed to install build tools in WSL2" -ForegroundColor Red
                $BuildSuccess = $false
                return
            }
        
            # Create WSL2 native build environment
            Write-Host "Setting up WSL2 build environment..." -ForegroundColor Yellow
            wsl -d Ubuntu -- bash -c @"
cd ~ 
rm -rf nucleo_test 
mkdir -p nucleo_test 
cd nucleo_test
echo 'Copying project files...'
cp -r '/mnt/c/repos/Nucleo-H753ZI Project/code/host_tests' .
cp -r '/mnt/c/repos/Nucleo-H753ZI Project/code/src' .
cp -r '/mnt/c/repos/Nucleo-H753ZI Project/code/tests' .
cp -r '/mnt/c/repos/Nucleo-H753ZI Project/code/external' .
echo 'WSL2 environment ready'
"@
        
            if ($LASTEXITCODE -ne 0) {
                Write-Host "❌ Failed to setup WSL2 environment" -ForegroundColor Red
                $BuildSuccess = $false
                return
            }
        
            # Build and run tests in WSL2
            Write-Host "Building host tests in WSL2..." -ForegroundColor Yellow
            wsl -d Ubuntu -- bash -c @"
cd ~/nucleo_test/host_tests
mkdir -p build
cd build
cmake ..
make -j`$(nproc)
"@
        
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✅ WSL2 host test build successful!" -ForegroundColor Green
            
                Write-Host "Running WSL2 host tests..." -ForegroundColor Yellow
                wsl -d Ubuntu -- bash -c @"
cd ~/nucleo_test/host_tests/build
echo 'Available test executables:'
ls -la *host
echo ''
echo 'Running Phase 1.3 validation test:'
./test_phase1_3_validation_host
"@
            
                if ($LASTEXITCODE -eq 0) {
                    Write-Host "WSL2 host tests completed!" -ForegroundColor Green
                }
                else {
                    Write-Host "WSL2 host tests failed" -ForegroundColor Red
                    $BuildSuccess = $false
                }
            }
            else {
                Write-Host "WSL2 host test build failed" -ForegroundColor Red
                $BuildSuccess = $false
            }
        
            return
        }
    
        # Windows Host Testing (MSVC/MinGW - Legacy support)
        Write-Host "=== WINDOWS HOST TESTING BUILD ===" -ForegroundColor Cyan
    
        if (Test-Path "host_tests\CMakeLists.txt") {
            Write-Host "📋 Configuring host tests..." -ForegroundColor Yellow
            Set-Location "host_tests"
        
            # Choose appropriate generator and compiler based on availability
            if ($HostCompiler -eq "MSVC") {
                Write-Host "📋 Using Visual Studio generator for host tests..." -ForegroundColor Yellow
                $hostConfigResult = cmake -S . -B "..\build_host_tests" -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 17 2022" -A x64
            }
            elseif ($HostCompiler -eq "GCC") {
                Write-Host "📋 Using MinGW Makefiles generator for host tests..." -ForegroundColor Yellow
                $hostConfigResult = cmake -S . -B "..\build_host_tests" -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
            }
            else {
                Write-Host "❌ No suitable compiler configured for host tests" -ForegroundColor Red
                Set-Location ".."
                $BuildSuccess = $false
                return
            }
        
            Set-Location ".."
        
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✅ Host test configuration successful" -ForegroundColor Green
            
                Write-Host "🔨 Building host tests..." -ForegroundColor Yellow
                $hostBuildResult = cmake --build "build_host_tests"
            
                if ($LASTEXITCODE -eq 0) {
                    Write-Host "✅ Host test build successful!" -ForegroundColor Green
                
                    Write-Host "🏃 Running host tests..." -ForegroundColor Yellow
                    Set-Location "build_host_tests"
                    $testResult = ctest --output-on-failure --verbose
                    Set-Location ".."
                
                    if ($LASTEXITCODE -eq 0) {
                        Write-Host "✅ All host tests passed!" -ForegroundColor Green
                    }
                    else {
                        Write-Host "❌ Some host tests failed" -ForegroundColor Red
                        $BuildSuccess = $false
                    }
                }
                else {
                    Write-Host "❌ Host test build failed" -ForegroundColor Red
                    $BuildSuccess = $false
                }
            }
            else {
                Write-Host "❌ Host test configuration failed" -ForegroundColor Red
                $BuildSuccess = $false
            }
        }
        else {
            Write-Host "❌ Host test CMakeLists.txt not found at host_tests/" -ForegroundColor Red
            $BuildSuccess = $false
        }
    }

    Write-Host ""
    if ($BuildSuccess) {
        Write-Host "🎉 Dual build system successfully configured and working!" -ForegroundColor Green
        Write-Host "🐍 Virtual environment: $env:VIRTUAL_ENV" -ForegroundColor Cyan
        if ($BuildType -eq "ARM" -or $BuildType -eq "Both") {
            Write-Host "📁 ARM build directory: $ArmBuildDir" -ForegroundColor Cyan
        }
        if ($BuildType -eq "Host" -or $BuildType -eq "Both") {
            Write-Host "� Host test directory: build_host_tests" -ForegroundColor Cyan
        }
    }
    else {
        Write-Host "❌ Build system setup failed!" -ForegroundColor Red
        Write-Host "📋 Check output above for specific errors" -ForegroundColor Yellow
        exit 1
    }

    Write-Host ""
    Write-Host "💡 Dual Build System Usage:" -ForegroundColor Yellow
    Write-Host "   ARM Firmware:  .\scripts\fix_cmake.ps1 -BuildType ARM" -ForegroundColor White
    Write-Host "   Host Testing:  .\scripts\fix_cmake.ps1 -BuildType Host" -ForegroundColor White
    Write-Host "   WSL2 Testing:  .\scripts\fix_cmake.ps1 -BuildType WSL2  (RECOMMENDED)" -ForegroundColor Green
    Write-Host "   Both:          .\scripts\fix_cmake.ps1 -BuildType Both" -ForegroundColor White

    if ($BuildType -eq "ARM" -or $BuildType -eq "Both") {
        Write-Host ""
        Write-Host "� ARM Firmware Commands:" -ForegroundColor Yellow
        if ($ArmBuildDir) {
            Write-Host "   cmake --build $ArmBuildDir           # Quick ARM build" -ForegroundColor White
            Write-Host "   STM32_Programmer_CLI.exe -c port=SWD -w $ArmBuildDir\stm32h753_ihm02a1.elf -v -rst" -ForegroundColor White
        }
    }

    if ($BuildType -eq "Host" -or $BuildType -eq "Both") {
        Write-Host ""
        Write-Host "🧪 Windows Host Testing Commands:" -ForegroundColor Yellow
        Write-Host "   cmake --build build_host_tests       # Build host tests" -ForegroundColor White
        Write-Host "   ctest --test-dir build_host_tests     # Run host tests" -ForegroundColor White
    }

    if ($BuildType -eq "WSL2") {
        Write-Host ""
        Write-Host "🐧 WSL2 Host Testing Commands:" -ForegroundColor Yellow
        Write-Host "   .\scripts\fix_cmake.ps1 -BuildType WSL2  # Recommended: Full WSL2 build & test" -ForegroundColor Green
        Write-Host "   wsl -d Ubuntu -- bash -c 'cd ~/nucleo_test/host_tests/build && ./test_phase1_3_validation_host'" -ForegroundColor White
    }
