#!/usr/bin/env python3
"""
SSOT Validation Script for STM32H753ZI Project
Validates that configuration is consistent across all files including
documentation.

TODO: See .github/instructions/ssot-config.instructions.md for SSOT
      validation rules
TODO: See .github/instructions/build-validation.instructions.md for build
      integration
"""

import argparse
import os
import re
import sys
from pathlib import Path
from typing import Dict, List, Optional

# SSOT Documentation Configuration - matches documentation_config.h
WORKSPACE_ROOT = Path.cwd()  # Use current working directory instead of hardcoded path
DOC_INDEXES_DIR = WORKSPACE_ROOT / "docs" / "indexes"
DOC_INDEX_STM32H7_FULL = (
    WORKSPACE_ROOT / "docs" / "indexes" / "STM32H7_FULL_INDEX.json"
)
DOC_INDEX_L6470_SEARCH = (
    WORKSPACE_ROOT / "docs" / "indexes" / "L6470_SEARCH_INDEX.json"
)
INSTRUCTION_ROOT_DIR = WORKSPACE_ROOT / ".github" / "instructions"
REFERENCE_ROOT_DIR = WORKSPACE_ROOT / "00_reference"


def find_hardcoded_values(file_path: str) -> List[Dict]:
    """Find potential hardcoded values that should be in SSOT."""
    hardcoded_patterns = [
        # Production-ready SSOT validation - focus only on critical violations
        # Critical: Large hex addresses that should be in config (but exclude small register operations)
        (
            r"(?<!0x[0-9A-Fa-f])0x[0-9A-Fa-f]{4,}(?![0-9A-Fa-f])(?!.*SSOT|.*config\.h|.*L6470_|.*AS5600_|.*CRC16_|.*MSG_|.*INVALID_|.*GPIO_PIN|.*HAL_)",
            "Large hex addresses should be in config headers",
        ),
        # Critical: Communication parameters
        (
            r"[0-9]+\s*[Kk]?[Bb]ps(?!.*SSOT|.*config\.h)",
            "Baud rates should be in comm_config.h",
        ),
        (
            r"192\.168\.\d+\.\d+(?!.*SSOT|.*config\.h)",
            "IP addresses should be in comm_config.h",
        ),
        (
            r"[0-9]+\s*[Rr][Pp][Mm](?!.*SSOT|.*config\.h)",
            "Motor speeds should be in motor_config.h",
        ),
    ]

    violations = []

    try:
        with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
            for line_num, line in enumerate(f, 1):
                # Skip comments and includes
                if line.strip().startswith("//") or line.strip().startswith(
                    "#include"
                ):
                    continue

                # Skip lines that are defining the constants (in config files)
                if "#define" in line and "config" in str(file_path).lower():
                    continue

                # Skip lines that properly use SSOT constants (comprehensive)
                if any(
                    const in line
                    for const in [
                        "MOTOR_",
                        "L6470_",
                        "AS5600_",
                        "CRC16_",
                        "INVALID_DEVICE_ID",
                        "DEMO_TIMER_",
                        "HARDWARE_",
                        "COMM_",
                        "SAFETY_",
                    ]
                ):
                    continue

                # Skip lines with explicit type casting (showing proper SSOT usage)
                if re.search(r"\([a-zA-Z_]+\)", line) and any(
                    const in line
                    for const in ["MOTOR_", "DEMO_", "HARDWARE_", "COMM_"]
                ):
                    continue

                # Skip STM32 HAL constants (these should NOT be moved to SSOT)
                if any(
                    hal_const in line
                    for hal_const in [
                        "GPIO_PIN_",
                        "HAL_",
                        "TIM",
                        "EXTI",
                        "NVIC_",
                        "__HAL_",
                    ]
                ):
                    continue

                # Skip mathematical constants and standard values
                if any(
                    std_val in line
                    for std_val in ["0.0f", "1.0f", "0x00", "0xFF", "NULL"]
                ):
                    continue

                # PRODUCTION: Skip type casting - this is normal in embedded C
                if re.search(r"\(uint\d+_t\)", line):
                    continue

                # PRODUCTION: Skip time values in comments - documentation is acceptable
                if "//" in line and re.search(r"\d+\s*ms|\d+ms", line):
                    continue

                # PRODUCTION: Skip period multipliers - these are mathematical ratios
                if "period_multiplier" in line and re.search(r"[0-9]+", line):
                    continue

                # PRODUCTION: Skip modulo operations - acceptable mathematical operations
                if re.search(r"%\s*\d+", line):
                    continue

                # PRODUCTION: Skip small hex values in driver code (register operations)
                if (
                    "driver" in str(file_path).lower()
                    or "l6470" in str(file_path).lower()
                ) and re.search(r"0x[0-9A-F]{1,3}(?![0-9A-F])", line):
                    continue

                for pattern, description in hardcoded_patterns:
                    matches = re.findall(pattern, line)
                    if matches:
                        violations.append(
                            {
                                "file": str(file_path),
                                "line": line_num,
                                "content": line.strip(),
                                "matches": matches,
                                "description": description,
                            }
                        )
    except Exception as e:
        print(f"Error reading {file_path}: {e}")

    return violations


def validate_ssot_structure() -> List[str]:
    """Validate that SSOT structure exists."""
    # Support both src/ (legacy) and CubeMX (Core/Inc, Drivers/Inc) structures
    ssot_locations = [
        ("hardware_config.h", [
            "src/config/hardware_config.h",
            "Core/Inc/hardware_config.h",
            "Drivers/Inc/hardware_config.h"
        ]),
        ("comm_config.h", [
            "src/config/comm_config.h",
            "Core/Inc/comm_config.h",
            "Drivers/Inc/comm_config.h"
        ]),
        ("motor_config.h", [
            "src/config/motor_config.h",
            "Core/Inc/motor_config.h",
            "Drivers/Inc/motor_config.h"
        ]),
        ("safety_config.h", [
            "src/config/safety_config.h",
            "Core/Inc/safety_config.h",
            "Drivers/Inc/safety_config.h"
        ]),
        ("build_config.h", [
            "src/config/build_config.h",
            "Core/Inc/build_config.h",
            "Drivers/Inc/build_config.h"
        ]),
        ("error_codes.h", [
            "src/common/error_codes.h",
            "Core/Inc/error_codes.h",
            "Drivers/Inc/error_codes.h"
        ]),
        ("system_state.h", [
            "src/common/system_state.h",
            "Core/Inc/system_state.h",
            "Drivers/Inc/system_state.h"
        ]),
        ("data_types.h", [
            "src/common/data_types.h",
            "Core/Inc/data_types.h",
            "Drivers/Inc/data_types.h"
        ]),
    ]

    missing_files = []
    for logical_name, candidates in ssot_locations:
        if not any(os.path.exists(path) for path in candidates):
            missing_files.append(f"(any of) {candidates}")

    return missing_files


def check_include_dependencies() -> List[Dict]:
    """Check that source files include appropriate SSOT headers."""
    violations = []

    # Find .c files that might need SSOT includes in both src/ and Core/Src/
    search_dirs = [Path("src"), Path("Core/Src")]
    for search_dir in search_dirs:
        if not search_dir.exists():
            continue
        for file_path in search_dir.rglob("*.c"):
            if "config" in str(file_path):
                continue
            try:
                with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
                    content = f.read()
            except Exception as e:
                print(f"Error checking includes in {file_path}: {e}")
                continue

            # Check for GPIO usage without hardware_config.h include
            if (
                "GPIO_" in content or "SPI" in content or "I2C" in content
            ) and '#include "config/hardware_config.h"' not in content:
                violations.append(
                    {
                        "file": str(file_path),
                        "issue": (
                            "Uses hardware constants but does not "
                            "include hardware_config.h"
                        ),
                        "recommendation": (
                            "Add #include " '"config/hardware_config.h"'
                        ),
                    }
                )

            # Check for communication usage without comm_config.h include
            if (
                "UART_" in content
                or "CAN_" in content
                or "ETH_" in content
            ) and '#include "config/comm_config.h"' not in content:
                violations.append(
                    {
                        "file": str(file_path),
                        "issue": (
                            "Uses communication constants but does not "
                            "include comm_config.h"
                        ),
                        "recommendation": (
                            'Add #include "config/comm_config.h"'
                        ),
                    }
                )

            # Check for motor usage without motor_config.h include
            if (
                "MOTOR_" in content or "L6470_" in content
            ) and '#include "config/motor_config.h"' not in content:
                violations.append(
                    {
                        "file": str(file_path),
                        "issue": (
                            "Uses motor constants but does not "
                            "include motor_config.h"
                        ),
                        "recommendation": (
                            "Add #include " '"config/motor_config.h"'
                        ),
                    }
                )
def validate_config_consistency() -> List[Dict]:
    """Validate consistency between configuration files."""
    inconsistencies = []

    # Check that motor count is consistent

    # Support both src/ and Core/Inc/ for config/common
    def find_first_existing(paths):
        for p in paths:
            if os.path.exists(p):
                return p
        return None

    motor_config_path = find_first_existing([
        "src/config/motor_config.h",
        "Core/Inc/motor_config.h",
        "Drivers/Inc/motor_config.h"
    ])
    state_config_path = find_first_existing([
        "src/common/system_state.h",
        "Core/Inc/system_state.h",
        "Drivers/Inc/system_state.h"
    ])
    if not motor_config_path or not state_config_path:
        print("Error checking config consistency: motor_config.h or system_state.h not found in supported locations")
        return inconsistencies

    with open(motor_config_path, "r", encoding="utf-8") as f:
        motor_config = f.read()
    with open(state_config_path, "r", encoding="utf-8") as f:
        state_config = f.read()

    # Extract MAX_MOTORS from motor config
    motor_match = re.search(r"#define\s+MAX_MOTORS\s+(\d+)", motor_config)
    if motor_match:
        max_motors = int(motor_match.group(1))

        # Check if system state uses MAX_MOTORS constant (correct approach)
        if "MotorStateInfo_t motors[MAX_MOTORS]" not in state_config:
            inconsistencies.append(
                {
                    "issue": (
                        f"MAX_MOTORS defined as {max_motors} but "
                        "system_state.h does not use MAX_MOTORS "
                        "constant"
                    ),
                    "files": [
                        "src/config/motor_config.h",
                        "src/common/system_state.h",
                    ],
                    "recommendation": (
                        "Use MotorStateInfo_t motors[MAX_MOTORS] "
                        "in system_state.h"
                    ),
                }
            )

        # Check if encoder arrays also use MAX_MOTORS constant
        if "EncoderState_t encoders[MAX_MOTORS]" not in state_config:
            inconsistencies.append(
                {
                    "issue": (
                        f"MAX_MOTORS defined as {max_motors} but "
                        "system_state.h encoders array does not use "
                        "MAX_MOTORS constant"
                    ),
                    "files": [
                        "src/config/motor_config.h",
                        "src/common/system_state.h",
                    ],
                    "recommendation": (
                        "Use EncoderState_t "
                        "encoders[MAX_MOTORS] in "
                        "system_state.h"
                    ),
                }
            )


    return inconsistencies


def generate_report(
    violations: List[Dict],
    missing_files: List[str],
    include_violations: List[Dict],
    inconsistencies: List[Dict],
    doc_structure_errors: Optional[List[str]] = None,
    instruction_ref_errors: Optional[List[str]] = None,
    doc_path_errors: Optional[List[str]] = None,
    workflow_errors: Optional[List[str]] = None,
    include_docs: bool = False,
    include_workflow: bool = False,
) -> None:
    """Generate validation report."""
    print("🔍 SSOT Validation Report for STM32H753ZI Project")
    print("=" * 60)

    # Check SSOT structure
    if missing_files:
        print("❌ Missing SSOT files:")
        for file in missing_files:
            print(f"   📄 {file}")
        print("\n💡 Run: mkdir -p src/config src/common")
        print(
            "💡 TODO: See .github/instructions/"
            "ssot-config.instructions.md to create missing files"
        )
    else:
        print("✅ SSOT structure complete")

    # Documentation structure validation (if requested)
    if include_docs:
        print("\n📚 Documentation Structure Validation:")
        if doc_structure_errors:
            print(
                f"❌ Found {len(doc_structure_errors)} documentation "
                "structure issues:"
            )
            for error in doc_structure_errors:
                print(f"   📁 {error}")
        else:
            print("✅ Documentation structure is valid")

        if instruction_ref_errors:
            print(
                f"\n❌ Found {len(instruction_ref_errors)} broken "
                "instruction references:"
            )
            for error in instruction_ref_errors:
                print(f"   🔗 {error}")
        else:
            print("✅ All instruction references are valid")

        if doc_path_errors:
            print(
                f"\n❌ Found {len(doc_path_errors)} hardcoded "
                "documentation paths:"
            )
            for error in doc_path_errors:
                print(f"   📂 {error}")
        else:
            print("✅ No hardcoded documentation paths found")

    # Workflow SSOT validation (if requested)
    if include_workflow:
        print("\n⚙️ Workflow SSOT Validation:")
        if workflow_errors:
            print(f"❌ Found {len(workflow_errors)} workflow SSOT issues:")
            for error in workflow_errors:
                print(f"   🔧 {error}")
        else:
            print("✅ Workflow SSOT configuration is valid")

    # Check for hardcoded values
    if violations:
        print(f"\n⚠️  Found {len(violations)} potential SSOT violations:")

        # Group by file for better readability
        by_file = {}
        for violation in violations:
            file = violation["file"]
            if file not in by_file:
                by_file[file] = []
            by_file[file].append(violation)

        # Show first 5 files
        for file, file_violations in list(by_file.items())[:5]:
            print(f"\n   📁 {file}:")
            # Show first 3 violations per file
            for violation in file_violations[:3]:
                content_preview = violation["content"][:60]
                print(f"      Line {violation['line']}: {content_preview}...")
                print(f"      💡 {violation['description']}")

        if len(by_file) > 5:
            print(
                f"\n   ... and {len(by_file) - 5} more files with violations"
            )
    else:
        print("✅ No SSOT violations found in source files")

    # Check include dependencies
    if include_violations:
        print(
            f"\n⚠️  Found {len(include_violations)} include dependency "
            "issues:"
        )
        for violation in include_violations[:5]:  # Show first 5
            print(f"   📁 {violation['file']}")
            print(f"      Issue: {violation['issue']}")
            print(f"      💡 {violation['recommendation']}")
    else:
        print("✅ Include dependencies look correct")

    # Check configuration consistency
    if inconsistencies:
        print(
            f"\n⚠️  Found {len(inconsistencies)} configuration "
            "inconsistencies:"
        )
        for inconsistency in inconsistencies:
            print(f"   🔗 {inconsistency['issue']}")
            print(f"      Files: {', '.join(inconsistency['files'])}")
            print(f"      💡 {inconsistency['recommendation']}")
    else:
        print("✅ Configuration consistency checks passed")

    # Summary and recommendations
    total_issues = (
        len(violations)
        + len(missing_files)
        + len(include_violations)
        + len(inconsistencies)
    )

    # Add workflow errors to total if workflow validation was requested
    if include_workflow and workflow_errors:
        total_issues += len(workflow_errors)

    # Add documentation errors to total if doc validation was requested
    if include_docs:
        if doc_structure_errors:
            total_issues += len(doc_structure_errors)
        if instruction_ref_errors:
            total_issues += len(instruction_ref_errors)
        if doc_path_errors:
            total_issues += len(doc_path_errors)

    print("\n📊 Summary:")
    print(f"   • Missing SSOT files: {len(missing_files)}")
    print(f"   • Hardcoded value violations: {len(violations)}")
    print(f"   • Include dependency issues: {len(include_violations)}")
    print(f"   • Configuration inconsistencies: {len(inconsistencies)}")
    if include_workflow:
        workflow_count = len(workflow_errors) if workflow_errors else 0
        print(f"   • Workflow SSOT issues: {workflow_count}")
    if include_docs:
        doc_count = 0
        if doc_structure_errors:
            doc_count += len(doc_structure_errors)
        if instruction_ref_errors:
            doc_count += len(instruction_ref_errors)
        if doc_path_errors:
            doc_count += len(doc_path_errors)
        print(f"   • Documentation issues: {doc_count}")
    print(f"   • Total issues found: {total_issues}")

    if total_issues == 0:
        print(
            "\n🎉 SSOT validation passed! All checks completed successfully."
        )
    else:
        print(f"\n🔧 Action required: {total_issues} issues need attention.")
        print("\n💡 Next steps:")
        if missing_files:
            print("   1. Create missing SSOT configuration files")
        if violations:
            print("   2. Replace hardcoded values with SSOT references")
        if include_violations:
            print("   3. Add missing SSOT header includes")
        if inconsistencies:
            print("   4. Fix configuration inconsistencies")
        print("   5. Re-run validation: python3 scripts/validate_ssot.py")

    print("\n🚀 SSOT validation complete!")


def validate_documentation_structure() -> List[str]:
    """Validate documentation structure matches SSOT configuration"""
    errors = []

    # Check required directories exist
    required_dirs = [
        DOC_INDEXES_DIR,
        INSTRUCTION_ROOT_DIR,
        REFERENCE_ROOT_DIR,
        "docs/design",
        "docs/requirements",
    ]

    for dir_path in required_dirs:
        if not Path(dir_path).exists():
            errors.append(
                f"Missing required documentation directory: {dir_path}"
            )

    # Check search indexes exist
    required_indexes = [
        DOC_INDEX_STM32H7_FULL,
        DOC_INDEX_L6470_SEARCH,
        "docs/indexes/STM32H7_COPILOT_INDEX.json",
        "docs/indexes/STM32H7_COPILOT_INDEX.yaml",
    ]

    for index_path in required_indexes:
        if not Path(index_path).exists():
            errors.append(f"Missing required search index: {index_path}")

    return errors


def validate_instruction_references() -> List[str]:
    """
    Validate that instruction file references in source code are valid.

    Note: Detailed instruction reference management is handled by
    instruction_manager.py
    """
    errors = []

    # Basic validation - detailed work delegated to instruction_manager.py
    pattern = r"\.github/instructions/([^/\s]+\.instructions\.md)"

    # Quick scan for obviously broken references
    for src_file in Path("src").rglob("*.[ch]"):
        try:
            with open(src_file, "r", encoding="utf-8", errors="ignore") as f:
                content = f.read()
        except Exception:
            continue

        matches = re.findall(pattern, content)
        for match in matches:
            instruction_file = Path(INSTRUCTION_ROOT_DIR) / match
            if not instruction_file.exists():
                errors.append(
                    f"Broken instruction reference in " f"{src_file}: {match}"
                )

    if errors:
        errors.append(
            "Run 'python3 scripts/instruction_manager.py validate' "
            "for detailed analysis"
        )

    return errors


def validate_documentation_paths() -> List[str]:
    """Validate hardcoded documentation paths against SSOT"""
    errors = []

    # Check scripts for hardcoded paths that should use SSOT
    if Path("scripts").exists():
        script_files = list(Path("scripts").glob("*.py"))

        hardcoded_patterns = [
            (r'docs/indexes/[^"\']+', "Should use DOC_INDEX_* constants"),
            (
                r'\.github/instructions/[^"\']+',
                "Should use INSTRUCTION_* constants",
            ),  # noqa: E501
            # NOTE: 00_reference is intentionally excluded - it's read-only reference material
            # that should NOT be integrated into our SSOT system
        ]

        for script_file in script_files:
            # Skip the validation script itself
            if script_file.name == "validate_ssot.py":
                continue

            try:
                with open(
                    script_file, "r", encoding="utf-8", errors="ignore"
                ) as f:
                    content = f.read()
            except Exception:
                continue

            for pattern, message in hardcoded_patterns:
                matches = re.findall(pattern, content)
                for match in matches:
                    msg = (
                        f"Hardcoded path in {script_file}: {match} - "
                        f"{message}"
                    )
                    errors.append(msg)

    return errors


def validate_workflow_ssot() -> List[str]:
    """Validate workflow SSOT configuration consistency"""
    errors = []

    # Check if workflow_config.h exists
    workflow_config_path = Path("src/config/workflow_config.h")
    if not workflow_config_path.exists():
        errors.append(
            "Missing workflow_config.h - workflow SSOT header not found"
        )
        return errors

    # Check if workflow_config.py script exists
    workflow_script_path = Path("scripts/workflow_config.py")
    if not workflow_script_path.exists():
        errors.append(
            "Missing scripts/workflow_config.py - workflow management script not found"
        )
        return errors

    # Check if .workflow_config file can be read (basic validation)
    try:
        config_file = Path(".workflow_config")
        if config_file.exists():
            with open(config_file, "r") as f:
                content = f.read().strip()
                # Check if it's valid JSON format
                import json

                try:
                    json.loads(content)
                except json.JSONDecodeError as e:
                    errors.append(
                        f"Invalid JSON in .workflow_config: {str(e)}"
                    )
    except Exception as e:
        errors.append(f"Cannot read workflow configuration: {str(e)}")

    # Check for SSOT-aware scripts
    ssot_aware_script = Path("scripts/run_python_configurable.ps1")
    if not ssot_aware_script.exists():
        errors.append("Missing SSOT-aware PowerShell wrapper script")

    # Validate workflow_config.h has required constants
    try:
        with open(workflow_config_path, "r", encoding="utf-8") as f:
            config_content = f.read()
            required_constants = [
                "WORKFLOW_MODE",
                "WORKFLOW_VALIDATE_VENV",
                "WORKFLOW_VERBOSE_LOGGING",
                "WorkflowMode_t",
            ]
            for constant in required_constants:
                if constant not in config_content:
                    errors.append(
                        f"Missing required constant in workflow_config.h: {constant}"
                    )
    except Exception as e:
        errors.append(f"Cannot validate workflow_config.h: {str(e)}")

    return errors


def main():
    """Main validation function."""
    parser = argparse.ArgumentParser(description="Validate SSOT compliance")
    parser.add_argument(
        "--include-docs",
        action="store_true",
        help="Include documentation structure validation",
    )
    parser.add_argument(
        "--include-workflow",
        action="store_true",
        help="Include workflow SSOT validation",
    )
    args = parser.parse_args()

    if not (os.path.exists("src") or os.path.exists("Core")):
        print("❌ Error: Neither src nor Core directory found. Run from project root of a valid STM32 or legacy project.")
        return 1

    print("🔍 Starting SSOT validation...")

    # Check SSOT structure
    missing_files = validate_ssot_structure()



if __name__ == "__main__":
    exit_code = main()
    sys.exit(exit_code)

# TODO: See .instructions/ssot-validation.md for extending this script with:
# - Configuration value range validation
# - Cross-reference checking between files
# - Automatic fix suggestions
# - Integration with pre-commit hooks
# - CI/CD pipeline integration
