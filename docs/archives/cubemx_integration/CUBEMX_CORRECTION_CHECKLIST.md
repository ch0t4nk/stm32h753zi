# STM32CubeMX Correction Checklist

## Critical Issues Found - Immediate Action Required

### 🔴 1. CRITICAL: Clock Configuration Mismatch
**Issue**: CubeMX configured for 8MHz HSE, but STM32H753ZI Nucleo-144 has 25MHz HSE

**Current CubeMX (.ioc file)**:
```
RCC.HSE_VALUE=8000000          ❌ WRONG
RCC.SYSCLKFreq_VALUE=64000000  ❌ SEVERELY UNDERCLOCKED
```

**Required Correction**:
```
RCC.HSE_VALUE=25000000         ✅ Correct for Nucleo-144
RCC.SYSCLKFreq_VALUE=480000000 ✅ Maximum performance
```

**Action**: Re-configure clock tree in CubeMX with correct HSE value

### 🔴 2. CRITICAL: Performance Optimization
**Issue**: Caches disabled, voltage scale too low

**Current CubeMX (.ioc file)**:
```
CORTEX_M7.CPU_DCache=Disabled           ❌ SLOW
CORTEX_M7.CPU_ICache=Disabled           ❌ SLOW  
RCC.PWR_Regulator_Voltage_Scale=SCALE2  ❌ LIMITS TO 400MHz
```

**Required Correction**:
```
CORTEX_M7.CPU_DCache=Enabled            ✅ FAST
CORTEX_M7.CPU_ICache=Enabled            ✅ FAST
RCC.PWR_Regulator_Voltage_Scale=SCALE1  ✅ SUPPORTS 480MHz
```

### 🔴 3. CRITICAL: Missing Required Peripherals
**Issue**: SPI, TIM, IWDG, FDCAN not enabled for stepper motor control

**Current HAL Configuration**:
```c
/* #define HAL_SPI_MODULE_ENABLED   */     ❌ NEEDED FOR L6470
/* #define HAL_TIM_MODULE_ENABLED   */     ❌ NEEDED FOR PWM
/* #define HAL_IWDG_MODULE_ENABLED   */    ❌ NEEDED FOR SAFETY
/* #define HAL_FDCAN_MODULE_ENABLED   */   ❌ NEEDED FOR CAN
```

**Required Addition to stm32h7xx_hal_conf.h**:
```c
#define HAL_SPI_MODULE_ENABLED      ✅ For L6470 drivers
#define HAL_TIM_MODULE_ENABLED      ✅ For PWM and timing  
#define HAL_IWDG_MODULE_ENABLED     ✅ For safety watchdog
#define HAL_FDCAN_MODULE_ENABLED    ✅ For CAN communication
```

### 🔴 4. CRITICAL: Missing Pin Configurations
**Issue**: No SPI2 or I2C pins configured for stepper motors and encoders

**Missing Pin Configurations**:
```
# SPI2 for L6470 Daisy-Chain
PB13 → SPI2_SCK                ❌ MISSING
PB14 → SPI2_MISO               ❌ MISSING  
PB15 → SPI2_MOSI               ❌ MISSING
PA9  → GPIO_Output (CS)        ❌ MISSING

# I2C for AS5600 Encoders
PB6  → I2C1_SCL (Encoder 1)    ❌ MISSING
PB7  → I2C1_SDA (Encoder 1)    ❌ MISSING
PB10 → I2C2_SCL (Encoder 2)    ❌ MISSING
PB11 → I2C2_SDA (Encoder 2)    ❌ MISSING
```

## 🛠️ Step-by-Step Correction Process

### Step 1: Fix Clock Configuration in CubeMX
1. Open `code.ioc` in STM32CubeMX
2. Go to Clock Configuration tab
3. Set HSE to 25MHz (not 8MHz)
4. Configure PLL for 480MHz:
   - PLL Source: HSE
   - DIVM1: 5 (25MHz ÷ 5 = 5MHz)
   - DIVN1: 192 (5MHz × 192 = 960MHz VCO)
   - DIVP1: 2 (960MHz ÷ 2 = 480MHz)
5. Set System Clock Source to PLL
6. Set Voltage Scale to Scale 1

### Step 2: Enable Performance Features
1. In System Core → CORTEX_M7:
   - Enable CPU DCache
   - Enable CPU ICache
2. In System Core → PWR:
   - Set Regulator Voltage Scaling to Scale 1

### Step 3: Add Required Peripherals
1. Enable SPI2:
   - PB13 → SPI2_SCK
   - PB14 → SPI2_MISO  
   - PB15 → SPI2_MOSI
   - PA9 → GPIO_Output (for CS)
2. Enable I2C1 and I2C2:
   - PB6/PB7 → I2C1_SCL/SDA
   - PB10/PB11 → I2C2_SCL/SDA
3. Enable TIM1 for PWM generation
4. Enable IWDG for safety watchdog
5. Enable FDCAN1 for CAN communication

### Step 4: Update HAL Configuration
1. In Project Manager → Advanced Settings:
   - Ensure SPI, TIM, IWDG, FDCAN HAL modules are enabled
2. Generate code

### Step 5: Integration with Our Project
1. **Resolve main.c conflict**:
   - Keep CubeMX main.c in Core/Src/
   - Migrate our application logic from src/application/main.c
   - Update CMakeLists.txt to use Core/Src/main.c

2. **Update SSOT references**:
   - Update hardware_config.h to match CubeMX settings
   - Ensure all peripheral configurations align

## 🧪 Validation Steps

### After Corrections:
1. **Build Test**: `cmake --build build`
2. **Clock Verification**: Measure system clock with debugger
3. **Peripheral Test**: Verify SPI2, I2C1/I2C2 functionality
4. **Performance Test**: Run with caches enabled
5. **Safety Test**: Verify watchdog operation

### Expected Results:
- System Clock: 480MHz (7.5x improvement)
- SPI Clock: Up to 60MHz (sufficient for 5MHz L6470 requirement)
- I2C Clock: Up to 1MHz (exceeds 400kHz requirement)
- Cache Performance: Significant speed improvement
- Safety: Proper watchdog protection

## 📋 Files Requiring Updates

### CubeMX Generated Files:
- `code.ioc` → Primary configuration file
- `Core/Inc/stm32h7xx_hal_conf.h` → HAL module enables
- `Core/Src/main.c` → Clock configuration code
- `Core/Inc/main.h` → Pin definitions

### Our Project Files:
- `src/config/hardware_config.h` → SSOT alignment
- `CMakeLists.txt` → Source file paths
- `src/application/main.c` → Integration/migration

---

**URGENCY**: 🔴 **IMMEDIATE ACTION REQUIRED**  
**IMPACT**: Current configuration will not work for stepper motor control  
**TIMELINE**: Fix before any motor control development begins
