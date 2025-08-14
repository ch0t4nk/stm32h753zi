#!/usr/bin/env python3
"""
STM32 Development Environment Validation Script
Validates STM32 toolchain, extensions, and configuration setup.
"""

import json
import subprocess
import sys
from pathlib import Path


class STM32Validator:
    def __init__(self):
        self.workspace_root = Path(__file__).parent.parent
        self.errors = []
        self.warnings = []
        self.successes = []

    def run_command(self, cmd, description=""):
        """Run a command and return success status."""
        try:
            result = subprocess.run(
                cmd, shell=True, capture_output=True, text=True
            )
            if result.returncode == 0:
                self.successes.append(f"✅ {description}: {cmd}")
                return True, result.stdout
            else:
                self.errors.append(
                    f"❌ {description}: {cmd} "
                    f"(Exit code: {result.returncode})"
                )
                return False, result.stderr
        except Exception as e:
            self.errors.append(f"❌ {description}: {cmd} (Exception: {e})")
            return False, str(e)

    def check_toolchain(self):
        """Validate ARM GCC toolchain."""
        print("🔧 Checking ARM GCC Toolchain...")

        # Check ARM GCC
        success, output = self.run_command(
            "arm-none-eabi-gcc --version", "ARM GCC Compiler"
        )
        if success:
            version = output.split("\n")[0]
            print(f"   Found: {version}")

        # Check GDB
        success, output = self.run_command(
            "gdb-multiarch --version", "GDB Multiarch Debugger"
        )
        if success:
            version = output.split("\n")[0]
            print(f"   Found: {version}")

        # Check OpenOCD
        success, output = self.run_command(
            "openocd --version 2>&1 | head -1", "OpenOCD"
        )
        if success:
            print(f"   Found: {output.strip()}")

    def check_extensions(self):
        """Check VS Code extensions."""
        print("\n🔌 Checking VS Code Extensions...")

        required_extensions = [
            "stmicroelectronics.stm32-vscode-extension",
            "stmicroelectronics.stm32cube-ide-build-cmake",
            "stmicroelectronics.stm32cube-ide-debug-core",
            "jeandudey.cortex-debug-dp-stm32h7",
            "marus25.cortex-debug",
        ]

        # Read extensions.json
        extensions_file = self.workspace_root / ".vscode" / "extensions.json"
        if extensions_file.exists():
            with open(extensions_file, encoding="utf-8") as f:
                extensions_config = json.load(f)
                recommended = extensions_config.get("recommendations", [])

                for ext in required_extensions:
                    if ext in recommended:
                        self.successes.append(
                            f"✅ Extension recommended: {ext}"
                        )
                        print(f"   ✅ {ext}")
                    else:
                        self.warnings.append(
                            f"⚠️ Extension not in recommendations: {ext}"
                        )
                        print(f"   ⚠️ {ext}")
        else:
            self.errors.append("❌ .vscode/extensions.json not found")

    def check_configuration_files(self):
        """Check configuration files."""
        print("\n⚙️ Checking Configuration Files...")

        config_files = {
            ".vscode/settings.json": "Workspace settings",
            ".vscode/launch.json": "Debug launch configuration",
            ".vscode/tasks.json": "Build tasks configuration",
            "CMakeLists.txt": "CMake build configuration",
        }

        for file_path, description in config_files.items():
            full_path = self.workspace_root / file_path
            if full_path.exists():
                self.successes.append(f"✅ {description}: {file_path}")
                print(f"   ✅ {file_path}")
            else:
                self.errors.append(f"❌ {description}: {file_path} not found")
                print(f"   ❌ {file_path}")

    def check_reference_assets(self):
        """Check reference documentation and assets."""
        print("\n📚 Checking Reference Assets...")

        assets = {
            "00_reference/ST_Assets/stm32h7-svd_2.8/STM32H753.svd": (
                "STM32H753 SVD file"
            ),
            "00_reference/STM32Cube_FW_H7_V1.12.0": (
                "STM32H7 HAL documentation"
            ),
            "00_reference/x_cube_spn2_markdown_docs": "L6470 documentation",
        }

        for asset_path, description in assets.items():
            full_path = self.workspace_root / asset_path
            if full_path.exists():
                self.successes.append(f"✅ {description}: {asset_path}")
                print(f"   ✅ {asset_path}")
            else:
                self.warnings.append(
                    f"⚠️ {description}: {asset_path} not found"
                )
                print(f"   ⚠️ {asset_path}")

    def check_workspace_settings(self):
        """Validate workspace-specific STM32 settings."""
        print("\n🎯 Checking STM32-Specific Settings...")

        settings_file = self.workspace_root / ".vscode" / "settings.json"
        if settings_file.exists():
            with open(settings_file, encoding="utf-8") as f:
                settings = json.load(f)

                required_settings = {
                    "C_Cpp.default.defines": "STM32 preprocessor defines",
                    "cortex-debug.openocdPath": "OpenOCD path for debugging",
                    "stm32cube.cubeMXPath": "STM32CubeMX path",
                    "stm32cube.svdPath": "SVD files path",
                }

                for setting, description in required_settings.items():
                    if setting in settings:
                        self.successes.append(f"✅ {description}: {setting}")
                        print(f"   ✅ {setting}")
                    else:
                        self.warnings.append(
                            f"⚠️ {description}: {setting} not configured"
                        )
                        print(f"   ⚠️ {setting}")

    def run_validation(self):
        """Run complete validation."""
        print("🔍 STM32 Development Environment Validation")
        print("=" * 50)

        self.check_toolchain()
        self.check_extensions()
        self.check_configuration_files()
        self.check_reference_assets()
        self.check_workspace_settings()

        print("\n📊 Validation Summary")
        print("=" * 30)
        print(f"✅ Successes: {len(self.successes)}")
        print(f"⚠️ Warnings: {len(self.warnings)}")
        print(f"❌ Errors: {len(self.errors)}")

        if self.errors:
            print("\n❌ Critical Issues:")
            for error in self.errors:
                print(f"   {error}")

        if self.warnings:
            print("\n⚠️ Recommendations:")
            for warning in self.warnings:
                print(f"   {warning}")

        # Return exit code
        return 1 if self.errors else 0


if __name__ == "__main__":
    validator = STM32Validator()
    sys.exit(validator.run_validation())
