# Windows STM32CubeMX Code Generation Instructions

## ✅ Status: Configuration Complete!

The Python script has successfully configured your `code.ioc` file with all required peripherals:

### ✅ Peripherals Added:
- **SPI2**: L6470 stepper drivers (PB13/PB14/PB15, PA9)
- **I2C1**: AS5600 encoder 1 (PB6/PB7)  
- **I2C2**: AS5600 encoder 2 (PB10/PB11)
- **TIM2**: 1kHz control loop timer
- **FDCAN1**: MCU communication (PD0/PD1)

### ✅ Conflicts Resolved:
- **PB14**: GPIO_Output → SPI2_MISO
- **PB7**: GPIO_Output → I2C1_SDA

## Next Step: Generate Code with Windows STM32CubeMX

### Method 1: GUI Approach (Recommended)

1. **Open STM32CubeMX** on your Windows machine:
   ```
   C:\STMicroelectronics\STM32Cube\STM32CubeMX\STM32CubeMX.exe
   ```

2. **Load Project**:
   - File → Load Project
   - Navigate to your workspace: `\\wsl$\Ubuntu\workspaces\code\code.ioc`
   - Or use mapped drive if available

3. **Verify Configuration**:
   - Check **Pinout & Configuration** tab
   - Verify **SPI2** is configured (Connectivity)
   - Verify **I2C1 & I2C2** are configured (Connectivity)
   - Verify **TIM2** is configured (Timers)
   - Verify **FDCAN1** is configured (Connectivity)

4. **Generate Code**:
   - Go to **Project Manager** tab
   - Verify settings:
     - Project Name: `code`
     - Project Location: Your workspace path
     - Toolchain/IDE: `CMake`
     - Keep User Code: ✅ **CHECKED**
   - Click **GENERATE CODE**

5. **Success Indicators**:
   - Code generation should complete without errors
   - Check "Keep User Code" was selected
   - New files should appear in `Core/` directory

### Method 2: CLI Approach (Alternative)

If you prefer command line:

```cmd
# Windows Command Prompt
cd "C:\STMicroelectronics\STM32Cube\STM32CubeMX"
STM32CubeMX.exe -q "\\wsl$\Ubuntu\workspaces\code\scripts\cubemx_config.script"
```

## Expected Generated Files

After successful code generation, you should see:

### Core Directory Structure:
```
Core/
├── Inc/
│   ├── main.h              # Pin definitions & peripheral handles
│   ├── stm32h7xx_hal_conf.h  # HAL configuration
│   ├── stm32h7xx_it.h     # Interrupt declarations
│   └── ...
├── Src/
│   ├── main.c              # Main initialization code
│   ├── stm32h7xx_hal_msp.c   # MSP initialization
│   ├── stm32h7xx_it.c     # Interrupt handlers
│   └── ...
└── Startup/
    └── startup_stm32h753zitx.s
```

### Key Generated Content:

#### In `Core/Inc/main.h`:
```c
// SPI2 pin definitions
#define SPI2_SCK_Pin GPIO_PIN_13
#define SPI2_MISO_Pin GPIO_PIN_14
#define SPI2_MOSI_Pin GPIO_PIN_15

// I2C pin definitions  
#define I2C1_SCL_Pin GPIO_PIN_6
#define I2C1_SDA_Pin GPIO_PIN_7
#define I2C2_SCL_Pin GPIO_PIN_10
#define I2C2_SDA_Pin GPIO_PIN_11

// Peripheral handles
extern SPI_HandleTypeDef hspi2;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern TIM_HandleTypeDef htim2;
extern FDCAN_HandleTypeDef hfdcan1;
```

#### In `Core/Src/main.c`:
```c
// Peripheral initialization functions
void MX_SPI2_Init(void);
void MX_I2C1_Init(void);
void MX_I2C2_Init(void);
void MX_TIM2_Init(void);
void MX_FDCAN1_Init(void);
```

## Post-Generation Validation

After code generation, return to your dev container and run:

### 1. Build Test
```bash
cd /workspaces/code
cmake --build build
```

### 2. SSOT Validation
```bash
python3 scripts/validate_ssot.py
```

### 3. Check Generated Pin Assignments
```bash
# Verify peripheral handles
grep -E "hspi2|hi2c[12]|htim2|hfdcan1" Core/Src/main.c

# Verify pin definitions
grep -E "(SPI2|I2C[12]|TIM2|FDCAN1).*Pin" Core/Inc/main.h
```

## Troubleshooting

### Common Issues:

#### 1. Path Access Problems
- Use UNC path: `\\wsl$\Ubuntu\workspaces\code\code.ioc`
- Or copy file to Windows: `C:\temp\code.ioc`

#### 2. Code Generation Errors
- Ensure "Keep User Code" is checked
- Verify project path is accessible
- Check for file permission issues

#### 3. Missing Peripherals
- Reload the .ioc file
- Check Pinout & Configuration tab
- Verify IP count shows 12 (was 6, now 12)

#### 4. Build Failures After Generation
```bash
# Clean rebuild
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
```

## Success Criteria

✅ **Code generation successful** without errors  
✅ **All peripherals visible** in CubeMX Pinout view  
✅ **Build system works** after generation  
✅ **SSOT validation passes**  
✅ **Pin assignments match** hardware configuration  

## Next Development Steps

After successful code generation:

1. **Test peripheral initialization** - Verify all handles initialize properly
2. **Implement driver layers** - Use `.github/instructions/*.instructions.md` files
3. **Add safety systems** - Per `safety-systems.instructions.md`
4. **Integrate motor control** - Per `l6470-registers.instructions.md`

Your CubeMX configuration is now complete and ready for code generation! 🚀
