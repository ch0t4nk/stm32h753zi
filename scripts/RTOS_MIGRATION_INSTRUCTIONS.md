# STM32H7 RTOS Migration Instructions

## Following STM32CubeH7 V1.12.1 Version Consistency Standards

### Target Versions
- **Cmsis Core**: v5.4.0
- **Cmsis Device**: v1.11.x
- **Hal Driver**: v1.12.x
- **Firmware Package**: STM32CubeH7_V1.12.1

### Migration Steps

#### Step 1: Backup Current State

```bash
git checkout -b pre-rtos-upgrade
git add -A && git commit -m 'Backup before RTOS HAL upgrade'
cp -r drivers/ drivers_backup_pre_rtos/
cp -r Core/Inc/ config_backup_pre_rtos/
```

#### Step 2: Download STM32CubeH7 V1.12.1

Download from: https://www.st.com/en/embedded-software/stm32cubeh7.html

#### Step 3: Replace HAL Components

File mapping:
- `STM32CubeH7/Drivers/CMSIS/` -> `drivers/CMSIS/`
- `STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/` -> `drivers/STM32H7xx_HAL_Driver/`

#### Step 4: Update Configuration

Configuration changes:
- Core/Inc/stm32h7xx_hal_conf.h: Set USE_RTOS=1
- Core/Inc/FreeRTOSConfig.h: Verify RTOS configuration
- CMakeLists.txt: Update HAL source files if needed

#### Step 5: Validate Build

### Critical Validation

After migration, verify:
- [ ] Check CMSIS Core version = v5.4.0
- [ ] Verify HAL has no USE_RTOS constraint
- [ ] Confirm FreeRTOS builds successfully
- [ ] Test RTOS task scheduling

### Emergency Rollback
```bash
git checkout pre-rtos-upgrade
cp -r drivers_backup_pre_rtos/* drivers/
cp -r config_backup_pre_rtos/* Core/Inc/
```
