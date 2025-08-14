#!/usr/bin/env python3
"""
Workflow Configuration Manager

This tool provides easy management of workflow complexity settings via SSOT configuration.
Allows switching between modes and customizing individual features.

Usage:
    python scripts/workflow_config.py mode minimal      # Set to minimal mode
    python scripts/workflow_config.py mode thorough     # Set to thorough mode
    python scripts/workflow_config.py show              # Show current configuration
    python scripts/workflow_config.py set validate_venv 0  # Disable venv validation
    python scripts/workflow_config.py reset             # Reset to defaults
"""

import argparse
import json
import os
import sys
from pathlib import Path
from typing import Dict, Any, Optional


class WorkflowConfigManager:
    """Manages workflow configuration settings and modes."""

    # Workflow modes and their default settings
    MODES = {
        "minimal": {
            "validate_venv": False,
            "validate_python": False,
            "detect_platform": False,
            "validate_build_env": False,
            "load_functions": False,
            "create_aliases": False,
            "auto_activate_venv": False,
            "check_dependencies": False,
            "validate_git_hooks": False,
            "validate_ssot": False,
            "verbose_logging": False,
            "continue_on_error": True,
            "show_timing": False,
            "validation_timeout_ms": 100,
        },
        "standard": {
            "validate_venv": True,
            "validate_python": True,
            "detect_platform": True,
            "validate_build_env": False,  # Skip for speed
            "load_functions": True,
            "create_aliases": True,
            "auto_activate_venv": True,
            "check_dependencies": False,
            "validate_git_hooks": False,
            "validate_ssot": False,
            "verbose_logging": False,
            "continue_on_error": True,
            "show_timing": False,
            "validation_timeout_ms": 500,
        },
        "thorough": {
            "validate_venv": True,
            "validate_python": True,
            "detect_platform": True,
            "validate_build_env": True,
            "load_functions": True,
            "create_aliases": True,
            "auto_activate_venv": True,
            "check_dependencies": True,
            "validate_git_hooks": True,
            "validate_ssot": True,
            "verbose_logging": True,
            "continue_on_error": False,  # Fail fast
            "show_timing": True,
            "validation_timeout_ms": 2000,
        },
        "debug": {
            "validate_venv": True,
            "validate_python": True,
            "detect_platform": True,
            "validate_build_env": True,
            "load_functions": True,
            "create_aliases": True,
            "auto_activate_venv": True,
            "check_dependencies": True,
            "validate_git_hooks": True,
            "validate_ssot": True,
            "verbose_logging": True,
            "continue_on_error": False,
            "show_timing": True,
            "validation_timeout_ms": 5000,
        },
    }

    def __init__(self, workspace_root: Optional[Path] = None):
        """Initialize the configuration manager."""
        if workspace_root is None:
            # Auto-detect workspace root by looking for STATUS.md
            current = Path.cwd()
            workspace_root = current
            while workspace_root.parent != workspace_root:
                if (workspace_root / "STATUS.md").exists():
                    break
                workspace_root = workspace_root.parent
            else:
                workspace_root = current

        self.workspace_root = workspace_root
        self.config_file = workspace_root / ".workflow_config"
        self.env_var = "STM32_WORKFLOW_MODE"

    def get_current_config(self) -> Dict[str, Any]:
        """Get the current workflow configuration."""
        # Check environment variable first
        env_mode = os.environ.get(self.env_var, "").lower()
        if env_mode in self.MODES:
            config = self.MODES[env_mode].copy()
            config["mode"] = env_mode
            config["source"] = "environment_variable"
            return config

        # Check config file
        if self.config_file.exists():
            try:
                with open(self.config_file, "r") as f:
                    file_config = json.load(f)

                mode = file_config.get("mode", "custom")
                if mode in self.MODES:
                    config = self.MODES[mode].copy()
                    # Override with any custom settings
                    for key, value in file_config.items():
                        if key != "mode":
                            config[key] = value
                else:
                    config = file_config

                config["mode"] = mode
                config["source"] = "config_file"
                return config
            except (json.JSONDecodeError, OSError):
                pass

        # Default to standard mode
        config = self.MODES["standard"].copy()
        config["mode"] = "standard"
        config["source"] = "default"
        return config

    def set_mode(self, mode: str) -> bool:
        """Set the workflow mode."""
        if mode not in self.MODES:
            print(f"❌ Unknown mode: {mode}")
            print(f"Available modes: {', '.join(self.MODES.keys())}")
            return False

        config = self.MODES[mode].copy()
        config["mode"] = mode

        try:
            with open(self.config_file, "w") as f:
                json.dump(config, f, indent=2)
            print(f"✅ Workflow mode set to: {mode}")
            return True
        except OSError as e:
            print(f"❌ Failed to save configuration: {e}")
            return False

    def set_setting(self, key: str, value: Any) -> bool:
        """Set an individual setting."""
        config = self.get_current_config()

        # Convert string values to appropriate types
        if isinstance(value, str):
            if value.lower() in ("true", "1", "yes", "on"):
                value = True
            elif value.lower() in ("false", "0", "no", "off"):
                value = False
            elif value.isdigit():
                value = int(value)

        config[key] = value
        if config.get("mode") in self.MODES:
            config["mode"] = "custom"  # Switch to custom mode when modifying

        try:
            with open(self.config_file, "w") as f:
                json.dump(config, f, indent=2)
            print(f"✅ Set {key} = {value}")
            return True
        except OSError as e:
            print(f"❌ Failed to save configuration: {e}")
            return False

    def reset_config(self) -> bool:
        """Reset configuration to defaults."""
        try:
            if self.config_file.exists():
                self.config_file.unlink()
            print("✅ Configuration reset to defaults (standard mode)")
            return True
        except OSError as e:
            print(f"❌ Failed to reset configuration: {e}")
            return False

    def show_config(self) -> None:
        """Display current configuration."""
        config = self.get_current_config()
        mode = config.get("mode", "unknown")
        source = config.get("source", "unknown")

        print(f"🔧 Current Workflow Configuration")
        print(f"Mode: {mode} (from {source})")
        print()

        # Group settings by category
        categories = {
            "Environment Validation": [
                "validate_venv",
                "validate_python",
                "detect_platform",
                "validate_build_env",
            ],
            "Convenience Features": [
                "load_functions",
                "create_aliases",
                "auto_activate_venv",
            ],
            "Validation Depth": [
                "check_dependencies",
                "validate_git_hooks",
                "validate_ssot",
            ],
            "Error Handling": [
                "verbose_logging",
                "continue_on_error",
                "show_timing",
            ],
            "Performance": ["validation_timeout_ms"],
        }

        for category, settings in categories.items():
            print(f"📋 {category}:")
            for setting in settings:
                if setting in config:
                    value = config[setting]
                    status = "✅" if value else "❌"
                    if isinstance(value, bool):
                        print(f"  {status} {setting}")
                    else:
                        print(f"  📊 {setting}: {value}")
            print()

        # Show usage examples
        if mode != "custom":
            print("💡 Quick Mode Changes:")
            for mode_name in self.MODES.keys():
                if mode_name != mode:
                    print(
                        f"  python scripts/workflow_config.py mode {mode_name}"
                    )
            print()

        print("⚙️  Individual Settings:")
        print("  python scripts/workflow_config.py set validate_venv false")
        print("  python scripts/workflow_config.py set verbose_logging true")

    def export_env_vars(self) -> None:
        """Export configuration as environment variables."""
        config = self.get_current_config()

        print("# Workflow Configuration Environment Variables")
        print(f"export STM32_WORKFLOW_MODE={config.get('mode', 'standard')}")

        for key, value in config.items():
            if key not in ("mode", "source"):
                env_key = f"WORKFLOW_{key.upper()}"
                env_value = (
                    "1"
                    if value is True
                    else "0" if value is False else str(value)
                )
                print(f"export {env_key}={env_value}")

    def validate_config(self) -> bool:
        """Validate current configuration for common issues."""
        config = self.get_current_config()
        issues = []

        # Check for common conflicts
        if not config.get("validate_venv") and config.get(
            "auto_activate_venv"
        ):
            issues.append(
                "auto_activate_venv is enabled but validate_venv is disabled"
            )

        if not config.get("validate_python") and config.get(
            "check_dependencies"
        ):
            issues.append(
                "check_dependencies is enabled but validate_python is disabled"
            )

        if config.get("validation_timeout_ms", 0) < 50:
            issues.append(
                "validation_timeout_ms is very low, may cause timeouts"
            )

        if issues:
            print("⚠️  Configuration Issues Found:")
            for issue in issues:
                print(f"  - {issue}")
            return False
        else:
            print("✅ Configuration validation passed")
            return True


def main():
    """Main entry point for the workflow configuration manager."""
    parser = argparse.ArgumentParser(
        description="Manage STM32 workflow configuration complexity settings",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python scripts/workflow_config.py show
  python scripts/workflow_config.py mode minimal
  python scripts/workflow_config.py mode thorough
  python scripts/workflow_config.py set validate_venv false
  python scripts/workflow_config.py set verbose_logging true
  python scripts/workflow_config.py reset
  python scripts/workflow_config.py validate
  python scripts/workflow_config.py export
        """,
    )

    subparsers = parser.add_subparsers(dest="command", help="Commands")

    # Show command
    subparsers.add_parser("show", help="Show current configuration")

    # Mode command
    mode_parser = subparsers.add_parser("mode", help="Set workflow mode")
    mode_parser.add_argument(
        "mode_name", choices=["minimal", "standard", "thorough", "debug"]
    )

    # Set command
    set_parser = subparsers.add_parser("set", help="Set individual setting")
    set_parser.add_argument("key", help="Setting key")
    set_parser.add_argument("value", help="Setting value")

    # Other commands
    subparsers.add_parser("reset", help="Reset to default configuration")
    subparsers.add_parser("validate", help="Validate current configuration")
    subparsers.add_parser("export", help="Export as environment variables")

    args = parser.parse_args()

    if not args.command:
        parser.print_help()
        return 1

    manager = WorkflowConfigManager()

    if args.command == "show":
        manager.show_config()
    elif args.command == "mode":
        success = manager.set_mode(args.mode_name)
        return 0 if success else 1
    elif args.command == "set":
        success = manager.set_setting(args.key, args.value)
        return 0 if success else 1
    elif args.command == "reset":
        success = manager.reset_config()
        return 0 if success else 1
    elif args.command == "validate":
        success = manager.validate_config()
        return 0 if success else 1
    elif args.command == "export":
        manager.export_env_vars()

    return 0


if __name__ == "__main__":
    sys.exit(main())
