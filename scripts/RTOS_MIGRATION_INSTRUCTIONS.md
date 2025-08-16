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
