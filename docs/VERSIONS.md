# Versions and Tooling (discovered from the repository)

This file records the actual versions and locations discovered in the workspace. Keep these entries in-sync with CI and the SSOT headers when upgrading.

- STM32Cube HAL (embedded driver sources): STM32H7xx HAL version 1.11.5

  - Source: `Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c`
    - Defines: `__STM32H7xx_HAL_VERSION_MAIN   (0x01)` (== 1)
      `__STM32H7xx_HAL_VERSION_SUB1   (0x0B)` (== 11)
      `__STM32H7xx_HAL_VERSION_SUB2   (0x05)` (== 5)
      `__STM32H7xx_HAL_VERSION_RC     (0x00)` (== 0)
  - Interpreted version: 1.11.5 (RC 0)

- STM32Cube FW bundle referenced for RTOS migration: STM32CubeH7 V1.12.1 (used in migration scripts/docs)

  - Source: `scripts/rtos_migration_planner.py`, `scripts/rtos_migration_plan.json`, `scripts/RTOS_MIGRATION_INSTRUCTIONS.md`
  - Note: project contains guidance to migrate to V1.12.1; the embedded HAL currently present is 1.11.5.

- STM32CubeCLT (tool bundle used for flashing/tooling): 1.19.0

  - Source: `src/config/workspace_config.h` and `src/config/workspace_config.generated.h`
    - `#define STM32CUBECLT_VERSION "1.19.0"`
    - Example path: `C:/ST/STM32CubeCLT_1.19.0/...`

- ARM GCC toolchain:

  - Toolchain binary path in generated config: `C:/ST/STM32CubeCLT_1.19.0/GNU-tools-for-STM32/bin/arm-none-eabi-gcc.exe`
    - Source: `src/config/workspace_config.generated.h`
  - Status/notes: `STATUS.md` reports `ARM GCC Cross-Compiler Active (arm-none-eabi-gcc STM32CubeCLT 1.19.0, GNU 13.3.1)` — verify locally with `arm-none-eabi-gcc --version`.

- Other references and CI guidance:
  - Several scripts reference the recommended HAL bundle and migration to `STM32CubeH7_V1.12.1` (see `scripts/rtos_migration_planner.py`).
  - FreeRTOS integration notes are located under the `stm32h753zi_spn2_mcsdk_rtos/` tree and mention package expectations.

How to update these entries

- If you change HAL or toolchain versions, update this file and ensure `src/config/*` SSOT headers and `scripts/*` references are updated.
- Run CI and `scripts/check_rtos_compatibility.py` (or `scripts/validate_build_env.sh`) after upgrades to detect mismatches.

Quick verification commands (run locally in project workspace):

```powershell
# Check ARM GCC version available in PATH
arm-none-eabi-gcc --version

# Confirm HAL version value (example using grep on Windows PowerShell):
Select-String -Path Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c -Pattern "__STM32H7xx_HAL_VERSION"
```

Notes:

- The repository contains guidance and scripts for migrating to STM32CubeH7 V1.12.1 but the embedded HAL currently used in source is 1.11.5; align these intentionally when performing an upgrade.
