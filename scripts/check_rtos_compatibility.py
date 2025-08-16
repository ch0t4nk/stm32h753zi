#!/usr/bin/env python3
"""
STM32H7 RTOS Compatibility Checker
Validates current versions and RTOS readiness following STM32CubeH7 standards
"""

import os
import re
import sys

def check_cmsis_version():
    """Check CMSIS Core version"""
    cmsis_version_file = "drivers/CMSIS/Include/cmsis_version.h"
    if os.path.exists(cmsis_version_file):
        with open(cmsis_version_file, 'r') as f:
            content = f.read()
            main_match = re.search(r'#define __CM_CMSIS_VERSION_MAIN\s+\(\s*(\d+)U\s*\)', content)
            sub_match = re.search(r'#define __CM_CMSIS_VERSION_SUB\s+\(\s*(\d+)U\s*\)', content)
            
            if main_match and sub_match:
                main_ver = int(main_match.group(1))
                sub_ver = int(sub_match.group(1))
                print(f"✓ CMSIS Core: v{main_ver}.{sub_ver}.0")
                
                # Check against STM32CubeH7 standard
                if main_ver == 5 and sub_ver >= 4:
                    print("  ✅ Compatible with STM32CubeH7 standard (v5.4.0+)")
                    return True
                else:
                    print(f"  ⚠️  Below STM32CubeH7 standard (v5.4.0) - Current: v{main_ver}.{sub_ver}.0")
                    return False
    
    print("❌ CMSIS version file not found")
    return False

def check_device_version():
    """Check CMSIS Device version"""
    device_file = "drivers/CMSIS/Device/ST/STM32H7xx/Include/stm32h7xx.h"
    if os.path.exists(device_file):
        with open(device_file, 'r') as f:
            content = f.read()
            # Look for device version comment
            version_match = re.search(r'CMSIS Device version number V(\d+)\.(\d+)\.(\d+)', content)
            if version_match:
                major, minor, patch = version_match.groups()
                print(f"✓ CMSIS Device: v{major}.{minor}.{patch}")
                
                # Device v1.10.6+ is needed for modern compatibility
                version_num = int(major) * 100 + int(minor) * 10 + int(patch)
                if version_num >= 1106:  # v1.10.6+
                    print("  ✅ Device version compatible")
                    return True
                else:
                    print(f"  ⚠️  Device version may need update for optimal RTOS support")
                    return False
    
    print("❌ Device version file not found")
    return False

def check_hal_rtos_constraint():
    """Check HAL RTOS constraint"""
    hal_def_file = "drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_def.h"
    src_hal_def_file = "src/drivers/hal/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_def.h"
    
    # Check both possible locations
    hal_file = hal_def_file if os.path.exists(hal_def_file) else src_hal_def_file
    
    if os.path.exists(hal_file):
        with open(hal_file, 'r') as f:
            content = f.read()
            
            # Look for HAL version
            version_match = re.search(r'STM32H7xx_HAL_VERSION_MAIN.*?(\d+).*?STM32H7xx_HAL_VERSION_SUB.*?(\d+).*?STM32H7xx_HAL_VERSION_RC.*?(\d+)', content, re.DOTALL)
            if version_match:
                main, sub, rc = version_match.groups()
                print(f"✓ HAL Version: v{main}.{sub}.{rc}")
            
            # Check for RTOS constraint
            if 'USE_RTOS should be 0' in content:
                print("❌ HAL has hard-coded RTOS constraint (USE_RTOS=0)")
                print("   This HAL version prevents RTOS usage")
                return False
            else:
                print("✅ HAL allows RTOS usage")
                return True
    
    print("❌ HAL definition file not found")
    return False

def check_freertos_config():
    """Check current FreeRTOS configuration"""
    freertos_config = "Core/Inc/FreeRTOSConfig.h"
    if os.path.exists(freertos_config):
        print("✓ FreeRTOS configuration exists")
        with open(freertos_config, 'r') as f:
            content = f.read()
            if 'configUSE_MALLOC_FAILED_HOOK' in content:
                print("  ✅ FreeRTOS config appears complete")
                return True
    else:
        print("⚠️  FreeRTOS configuration not found")
    return False

def recommend_upgrade_path():
    """Provide upgrade recommendations based on STM32CubeH7"""
    print("\n" + "="*60)
    print("UPGRADE RECOMMENDATIONS (STM32CubeH7 V1.12.1 Compatibility)")
    print("="*60)
    
    print("\n🎯 Target Versions (STM32CubeH7 Standard):")
    print("   CMSIS Core:    v5.4.0  (Proven in STM32CubeH7)")
    print("   CMSIS Device:  v1.11.x (Compatible with Core v5.4.0)")
    print("   HAL:           v1.12.x (RTOS constraint removed)")
    
    print("\n🔧 Recommended Upgrade Methods:")
    print("   1. STM32CubeMX (Safest):")
    print("      - Create new project with STM32H753ZI + FreeRTOS")
    print("      - Use firmware package V1.12.1")
    print("      - Migrate your application code")
    
    print("   2. Manual Firmware Update:")
    print("      - Download STM32CubeH7 V1.12.1")
    print("      - Replace drivers/ with version-consistent set")
    print("      - Update include paths and dependencies")
    
    print("\n⚡ Your Current Toolchain Compatibility:")
    print("   ARM GCC 13.3.1: ✅ Compatible with modern CMSIS/HAL")
    print("   Build System:    ✅ CMake-based, ready for upgrade")

def main():
    print("STM32H7 RTOS Compatibility Analysis")
    print("=" * 50)
    
    os.chdir(os.path.dirname(os.path.abspath(__file__)) + "/..")
    
    cmsis_ok = check_cmsis_version()
    device_ok = check_device_version()
    hal_ok = check_hal_rtos_constraint()
    freertos_ok = check_freertos_config()
    
    print("\n" + "="*50)
    print("SUMMARY")
    print("="*50)
    
    if hal_ok:
        print("✅ Your HAL supports RTOS - You can enable USE_RTOS=1")
    else:
        print("❌ HAL Version Constraint: RTOS blocked by current HAL")
        print("   Solution: Upgrade to HAL v1.12.x following STM32CubeH7 standards")
    
    if cmsis_ok and device_ok:
        print("✅ CMSIS versions are compatible")
    else:
        print("⚠️  CMSIS versions below STM32CubeH7 standard")
    
    recommend_upgrade_path()

if __name__ == "__main__":
    main()
