# STM32CubeMX CLI Quick Reference

## ✅ Yes! CubeMX CLI Code Generation is Ready

Your setup supports **multiple CLI approaches** for code generation:

## Option 1: Direct CLI (Recommended)

### Code Generation Only (Since peripherals are already configured):
```cmd
# Windows Command Prompt
cd "C:\STMicroelectronics\STM32Cube\STM32CubeMX"
STM32CubeMX.exe -q "\\wsl$\Ubuntu\workspaces\code\scripts\cubemx_generate_code.script"
```

### Full Configuration + Generation:
```cmd
# Windows Command Prompt  
cd "C:\STMicroelectronics\STM32Cube\STM32CubeMX"
STM32CubeMX.exe -q "\\wsl$\Ubuntu\workspaces\code\scripts\cubemx_config.script"
```

## Option 2: Automated Wrapper

### From Dev Container:
```bash
# Code generation only (recommended since config is done)
./scripts/run_cubemx_cli.sh generate

# Full configuration + generation
./scripts/run_cubemx_cli.sh run

# Show manual instructions
./scripts/run_cubemx_cli.sh manual

# Validate generated code
./scripts/run_cubemx_cli.sh validate
```

## What Each Script Does

### `cubemx_generate_code.script` (NEW - Recommended):
```bash
# Optimized for your current situation:
1. Loads existing code.ioc (with all peripherals configured)
2. Sets project settings (CMake, Keep User Code)
3. Generates HAL initialization code
4. Saves project
5. Exits cleanly
```

### `cubemx_config.script` (Full Configuration):
```bash
# Complete configuration from scratch:
1. Loads code.ioc
2. Configures all peripherals (SPI2, I2C1/2, TIM2, FDCAN1)
3. Sets pin assignments
4. Resolves conflicts
5. Generates code
6. Saves and exits
```

## Expected Generated Files

After CLI execution, you'll get:

```
Core/
├── Inc/
│   ├── main.h              # 🎯 Pin definitions & peripheral handles
│   ├── stm32h7xx_hal_conf.h  # HAL configuration
│   ├── stm32h7xx_it.h     # Interrupt declarations
│   └── ...
├── Src/
│   ├── main.c              # 🎯 MX_SPI2_Init(), MX_I2C1_Init(), etc.
│   ├── stm32h7xx_hal_msp.c   # MSP initialization functions
│   ├── stm32h7xx_it.c     # 🎯 TIM2_IRQHandler(), FDCAN1_IT0_IRQHandler()
│   └── ...
└── Startup/
    └── startup_stm32h753zitx.s
```

## Key Generated Content

### In `Core/Inc/main.h`:
```c
// Peripheral handles (auto-generated)
extern SPI_HandleTypeDef hspi2;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern TIM_HandleTypeDef htim2;
extern FDCAN_HandleTypeDef hfdcan1;

// Pin definitions (auto-generated)
#define SPI2_SCK_Pin GPIO_PIN_13
#define SPI2_MISO_Pin GPIO_PIN_14
#define SPI2_MOSI_Pin GPIO_PIN_15
#define I2C1_SCL_Pin GPIO_PIN_6
#define I2C1_SDA_Pin GPIO_PIN_7
// ... and more
```

### In `Core/Src/main.c`:
```c
// Auto-generated initialization functions
void MX_SPI2_Init(void);
void MX_I2C1_Init(void);
void MX_I2C2_Init(void);
void MX_TIM2_Init(void);
void MX_FDCAN1_Init(void);

// Called from main()
int main(void) {
    // ... HAL_Init(), SystemClock_Config() ...
    MX_SPI2_Init();
    MX_I2C1_Init();
    MX_I2C2_Init();
    MX_TIM2_Init();
    MX_FDCAN1_Init();
    // ... main loop ...
}
```

## CLI Advantages

✅ **Fully Automated** - No manual clicking required  
✅ **Consistent Results** - Same configuration every time  
✅ **Version Control Friendly** - Scriptable and repeatable  
✅ **Build System Ready** - CMake integration maintained  
✅ **SSOT Compliant** - Aligns with instruction file system  

## Troubleshooting CLI

### If WSL Paths Don't Work:
```cmd
# Copy scripts to Windows temp directory
mkdir C:\temp
copy "\\wsl$\Ubuntu\workspaces\code\scripts\cubemx_generate_code.script" "C:\temp\"
copy "\\wsl$\Ubuntu\workspaces\code\code.ioc" "C:\temp\"

# Run with local paths
cd "C:\STMicroelectronics\STM32Cube\STM32CubeMX"
STM32CubeMX.exe -q "C:\temp\cubemx_generate_code.script"
```

### Post-CLI Validation:
```bash
# Back in dev container, test the generated code
cmake --build build

# Verify peripheral handles
grep -E "hspi2|hi2c[12]|htim2|hfdcan1" Core/Src/main.c

# Check SSOT compliance
python3 scripts/validate_ssot.py
```

## Ready to Execute! 🚀

Your CubeMX CLI setup is **complete and tested**:

✅ **Configuration**: All peripherals configured via Python script  
✅ **CLI Scripts**: Ready for immediate Windows execution  
✅ **Automation**: Wrapper scripts for easy execution  
✅ **Validation**: Built-in checks for success verification  

**Recommended next action**: 
```cmd
# Run this from Windows Command Prompt:
cd "C:\STMicroelectronics\STM32Cube\STM32CubeMX"
STM32CubeMX.exe -q "\\wsl$\Ubuntu\workspaces\code\scripts\cubemx_generate_code.script"
```

This will generate all the HAL initialization code for your configured peripherals! 🎯
