#!/usr/bin/env python3
"""
STM32CubeH7 Manual Migration Script
Downloads and prepares STM32CubeH7 V1.12.1 for RTOS-compatible HAL upgrade
"""

import os
import sys
import json
from pathlib import Path

def create_migration_plan():
    """Create detailed migration plan following STM32CubeH7 standards"""
    
    plan = {
        "migration_type": "STM32CubeH7_V1.12.1_RTOS_Compatible",
        "target_versions": {
            "cmsis_core": "v5.4.0",
            "cmsis_device": "v1.11.x",
            "hal_driver": "v1.12.x",
            "firmware_package": "STM32CubeH7_V1.12.1"
        },
        "backup_strategy": {
            "current_drivers": "drivers_backup_pre_rtos/",
            "current_config": "config_backup_pre_rtos/",
            "git_branch": "pre-rtos-upgrade"
        },
        "migration_steps": [
            {
                "step": 1,
                "action": "Backup Current State",
                "commands": [
                    "git checkout -b pre-rtos-upgrade",
                    "git add -A && git commit -m 'Backup before RTOS HAL upgrade'",
                    "cp -r drivers/ drivers_backup_pre_rtos/",
                    "cp -r Core/Inc/ config_backup_pre_rtos/"
                ]
            },
            {
                "step": 2,
                "action": "Download STM32CubeH7 V1.12.1",
                "source": "https://www.st.com/en/embedded-software/stm32cubeh7.html",
                "extract_components": [
                    "Drivers/CMSIS/",
                    "Drivers/STM32H7xx_HAL_Driver/",
                    "Projects/NUCLEO-H743ZI/Templates/"
                ]
            },
            {
                "step": 3,
                "action": "Replace HAL Components",
                "file_mapping": {
                    "STM32CubeH7/Drivers/CMSIS/": "drivers/CMSIS/",
                    "STM32CubeH7/Drivers/STM32H7xx_HAL_Driver/": "drivers/STM32H7xx_HAL_Driver/"
                }
            },
            {
                "step": 4,
                "action": "Update Configuration",
                "config_changes": [
                    "Core/Inc/stm32h7xx_hal_conf.h: Set USE_RTOS=1",
                    "Core/Inc/FreeRTOSConfig.h: Verify RTOS configuration",
                    "CMakeLists.txt: Update HAL source files if needed"
                ]
            },
            {
                "step": 5,
                "action": "Validate Build",
                "validation_commands": [
                    "cmake --build build --target clean",
                    "cmake --build build",
                    "python scripts/check_rtos_compatibility.py"
                ]
            }
        ],
        "critical_files_to_update": [
            "drivers/CMSIS/Include/cmsis_version.h",
            "drivers/CMSIS/Device/ST/STM32H7xx/Include/stm32h7xx.h",
            "drivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal_def.h",
            "Core/Inc/stm32h7xx_hal_conf.h"
        ],
        "verification_tests": [
            "Check CMSIS Core version = v5.4.0",
            "Verify HAL has no USE_RTOS constraint",
            "Confirm FreeRTOS builds successfully",
            "Test RTOS task scheduling"
        ]
    }
    
    return plan

def save_migration_plan():
    """Save migration plan to file"""
    plan = create_migration_plan()
    
    plan_file = "scripts/rtos_migration_plan.json"
    with open(plan_file, 'w') as f:
        json.dump(plan, f, indent=2)
    
    print(f"✓ Migration plan saved to: {plan_file}")
    
    # Create human-readable instructions
    instructions_file = "scripts/RTOS_MIGRATION_INSTRUCTIONS.md"
    with open(instructions_file, 'w') as f:
        f.write("# STM32H7 RTOS Migration Instructions\n\n")
        f.write("## Following STM32CubeH7 V1.12.1 Version Consistency Standards\n\n")
        
        f.write("### Target Versions\n")
        for component, version in plan["target_versions"].items():
            f.write(f"- **{component.replace('_', ' ').title()}**: {version}\n")
        
        f.write("\n### Migration Steps\n\n")
        for step_info in plan["migration_steps"]:
            f.write(f"#### Step {step_info['step']}: {step_info['action']}\n\n")
            
            if "commands" in step_info:
                f.write("```bash\n")
                for cmd in step_info["commands"]:
                    f.write(f"{cmd}\n")
                f.write("```\n\n")
            
            if "source" in step_info:
                f.write(f"Download from: {step_info['source']}\n\n")
            
            if "file_mapping" in step_info:
                f.write("File mapping:\n")
                for src, dst in step_info["file_mapping"].items():
                    f.write(f"- `{src}` -> `{dst}`\n")
                f.write("\n")
            
            if "config_changes" in step_info:
                f.write("Configuration changes:\n")
                for change in step_info["config_changes"]:
                    f.write(f"- {change}\n")
                f.write("\n")
        
        f.write("### Critical Validation\n\n")
        f.write("After migration, verify:\n")
        for test in plan["verification_tests"]:
            f.write(f"- [ ] {test}\n")
        
        f.write(f"\n### Emergency Rollback\n")
        f.write("```bash\n")
        f.write("git checkout pre-rtos-upgrade\n")
        f.write("cp -r drivers_backup_pre_rtos/* drivers/\n")
        f.write("cp -r config_backup_pre_rtos/* Core/Inc/\n")
        f.write("```\n")
    
    print(f"✓ Human-readable instructions saved to: {instructions_file}")

def main():
    print("STM32CubeH7 RTOS Migration Planner")
    print("=" * 50)
    
    # Change to project root
    os.chdir(os.path.dirname(os.path.abspath(__file__)) + "/..")
    
    save_migration_plan()
    
    print("\n" + "="*50)
    print("NEXT STEPS")
    print("="*50)
    print("1. Review the generated migration plan:")
    print("   - scripts/rtos_migration_plan.json (detailed)")
    print("   - scripts/RTOS_MIGRATION_INSTRUCTIONS.md (readable)")
    
    print("\n2. Choose migration approach:")
    print("   A) STM32CubeMX (Recommended - Safest)")
    print("   B) Manual following generated plan")
    
    print("\n3. For STM32CubeMX approach:")
    print("   - Create new project: STM32H753ZI + FreeRTOS")
    print("   - Use STM32CubeH7 firmware package V1.12.1")
    print("   - Migrate your application code")
    
    print("\n4. Validate version consistency:")
    print("   python scripts/check_rtos_compatibility.py")

if __name__ == "__main__":
    main()
