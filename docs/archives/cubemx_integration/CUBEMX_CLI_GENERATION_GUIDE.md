# STM32CubeMX CLI Code Generation Guide

## ✅ Yes! Code Generation via CubeMX CLI is Ready

Your configuration is complete and ready for CubeMX CLI code generation.

## Current Status

### ✅ **Configuration Complete**
- **Peripherals configured**: SPI2, I2C1, I2C2, TIM2, FDCAN1 ✅
- **Pin conflicts resolved**: PB14, PB7 automatically handled ✅
- **Project settings**: CMake, Keep User Code ✅
- **CLI scripts ready**: Both configuration and generation scripts ✅

### 📁 **Available CLI Scripts**
1. **`scripts/cubemx_config.script`** - Full configuration + code generation
2. **`scripts/cubemx_generate_code.script`** - Code generation only (new)

## Method 1: Code Generation Only (Recommended)

Since your peripherals are already configured, use the dedicated generation script:

### Windows Command Prompt:
```cmd
cd "C:\STMicroelectronics\STM32Cube\STM32CubeMX"
STM32CubeMX.exe -q "\\wsl$\Ubuntu\workspaces\code\scripts\cubemx_generate_code.script"
```

### Windows PowerShell:
```powershell
cd "C:\STMicroelectronics\STM32Cube\STM32CubeMX"
& ".\STM32CubeMX.exe" -q "\\wsl$\Ubuntu\workspaces\code\scripts\cubemx_generate_code.script"
```

### If WSL Path Doesn't Work:
```cmd
# Copy script to Windows first
copy "\\wsl$\Ubuntu\workspaces\code\scripts\cubemx_generate_code.script" "C:\temp\"

# Then run
cd "C:\STMicroelectronics\STM32Cube\STM32CubeMX"
STM32CubeMX.exe -q "C:\temp\cubemx_generate_code.script"
```

## Method 2: Full Configuration + Generation

If you want to reconfigure and generate in one step:

```cmd
cd "C:\STMicroelectronics\STM32Cube\STM32CubeMX"
STM32CubeMX.exe -q "\\wsl$\Ubuntu\workspaces\code\scripts\cubemx_config.script"
```

## Method 3: Automated Wrapper Script

Use the bash wrapper that handles everything:

```bash
# From your dev container
./scripts/run_cubemx_cli.sh run
```

This will:
- Detect Windows CubeMX installation
- Execute CLI script with proper paths
- Validate generated code
- Run SSOT compliance checks
- Test build system

## Expected CLI Output

### Successful Generation:
```
Starting STM32CubeMX...
Loading project: code.ioc
Project loaded successfully
Generating code...
Code generation completed
Saving project...
Project saved
Exiting STM32CubeMX
```

### Generated Files:
```
Core/
├── Inc/
│   ├── main.h              # Pin definitions & handles
│   ├── stm32h7xx_hal_conf.h
│   ├── stm32h7xx_it.h
│   └── ...
├── Src/
│   ├── main.c              # Peripheral initialization
│   ├── stm32h7xx_hal_msp.c
│   ├── stm32h7xx_it.c
│   └── ...
└── Startup/
    └── startup_stm32h753zitx.s
```

## Validation After Generation

### 1. Check Generated Files
```bash
# Back in dev container, verify files exist
ls -la Core/Inc/main.h
ls -la Core/Src/main.c
```

### 2. Verify Peripheral Handles
```bash
# Check for peripheral handles in main.h
grep -E "extern.*Handle.*h(spi2|i2c[12]|tim2|fdcan1)" Core/Inc/main.h
```

### 3. Test Build System
```bash
# Clean build to test generated code
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
```

### 4. SSOT Compliance Check
```bash
python3 scripts/validate_ssot.py
```

## CLI Script Details

### Code Generation Script (`cubemx_generate_code.script`):
```bash
# What it does:
1. Loads your configured code.ioc
2. Sets project settings (CMake, Keep User Code)
3. Generates all HAL initialization code
4. Saves project
5. Exits cleanly
```

### Project Settings Applied:
- **Target Toolchain**: CMake (matches your build system)
- **Keep User Code**: true (preserves your custom code)
- **Delete Previous**: false (safe generation)
- **Code Location**: Core/Src and Core/Inc (standard structure)

## Troubleshooting CLI Issues

### 1. STM32CubeMX Not Found
```cmd
# Check installation path
dir "C:\STMicroelectronics\STM32Cube\STM32CubeMX\STM32CubeMX.exe"

# Alternative paths to try:
dir "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeMX\STM32CubeMX.exe"
dir "C:\ST\STM32CubeMX\STM32CubeMX.exe"
```

### 2. Script Path Issues
```cmd
# If WSL path fails, copy script to Windows:
mkdir C:\temp
copy "\\wsl$\Ubuntu\workspaces\code\scripts\cubemx_generate_code.script" "C:\temp\"

# Then use local path:
STM32CubeMX.exe -q "C:\temp\cubemx_generate_code.script"
```

### 3. Permission Issues
```cmd
# Run Command Prompt as Administrator
# Ensure STM32CubeMX directory is writable
```

### 4. Project Load Failures
```cmd
# Verify .ioc file is accessible
type "\\wsl$\Ubuntu\workspaces\code\code.ioc"

# If access fails, copy .ioc to Windows:
copy "\\wsl$\Ubuntu\workspaces\code\code.ioc" "C:\temp\"
# Then modify script to use: load project from file "C:\temp\code.ioc"
```

## Success Indicators

### ✅ CLI Execution Success:
- STM32CubeMX starts without errors
- Project loads successfully  
- Code generation completes
- No error messages in output
- Files appear in Core/ directory

### ✅ Generated Code Quality:
- All peripheral handles present in main.h
- Initialization functions in main.c
- Interrupt handlers in stm32h7xx_it.c
- Build system compiles successfully

### ✅ Configuration Preserved:
- All 12 peripherals still configured
- Pin assignments match SSOT
- Project settings maintained

## Advanced CLI Options

### Silent Mode (No GUI):
```cmd
STM32CubeMX.exe -q "script_file"
```

### With Logging:
```cmd
STM32CubeMX.exe -q "script_file" > cubemx_output.log 2>&1
```

### Headless Execution:
```cmd
# For completely automated builds
STM32CubeMX.exe -q "script_file" --headless
```

## Ready to Execute!

Your STM32CubeMX CLI setup is **complete and ready**:

✅ **Configuration**: All peripherals added via Python script  
✅ **CLI Scripts**: Ready for immediate execution  
✅ **Project Settings**: CMake, Keep User Code configured  
✅ **Validation Tools**: SSOT and build system checks ready  

**Next action**: Run the CLI generation command from your Windows machine to generate all the HAL initialization code! 🚀
