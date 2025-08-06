#!/usr/bin/env python3
"""
Enhanced Instruction Manager for STM32H753ZI Project
Manages comprehensive instruction file system with validation,
cross-referencing, and metadata tracking.
"""

import argparse
import re
from pathlib import Path

# SSOT Documentation Configuration - matches documentation_config.h
WORKSPACE_ROOT = Path("/workspaces/code")
INSTRUCTION_ROOT_DIR = WORKSPACE_ROOT / ".github" / "instructions"


def get_actual_instruction_files():
    """Get list of actual instruction files using SSOT path"""
    instruction_dir = INSTRUCTION_ROOT_DIR
    actual_files = {}

    for file in instruction_dir.glob("*.instructions.md"):
        # Convert filename.instructions.md to filename.md for matching
        base_name = file.stem.replace(".instructions", ".md")
        actual_files[base_name] = str(file)

    return actual_files


def get_instruction_references():
    """Find all instruction file references in source code"""
    references = {}
    src_dir = Path("src")

    pattern = r"\.instructions/([^/\s]+\.md)"

    for src_file in src_dir.rglob("*.[ch]"):
        with open(src_file, "r") as f:
            content = f.read()

        matches = re.findall(pattern, content)
        if matches:
            references[str(src_file)] = matches

    return references


def get_consolidation_mapping():
    """Map missing instruction files to existing consolidated files"""
    mapping = {
        # Hardware-related files -> hardware-pins.instructions.md
        "hardware.md": "hardware-pins.instructions.md",
        "mcu-config.md": "hardware-pins.instructions.md",
        "peripheral-config.md": "hardware-pins.instructions.md",
        "pin-mapping.md": "hardware-pins.instructions.md",
        "gpio-config.md": "hardware-pins.instructions.md",
        # Motor-related files -> l6470-registers.instructions.md
        "motor.md": "l6470-registers.instructions.md",
        "stepper.md": "l6470-registers.instructions.md",
        "motor-control.md": "l6470-registers.instructions.md",
        "motion-profiles.md": "l6470-registers.instructions.md",
        "l6470.md": "l6470-registers.instructions.md",
        "stepper-driver.md": "l6470-registers.instructions.md",
        "driver-config.md": "l6470-registers.instructions.md",
        # Safety-related files -> safety-systems.instructions.md
        "safety.md": "safety-systems.instructions.md",
        "watchdog.md": "safety-systems.instructions.md",
        "fault-detection.md": "safety-systems.instructions.md",
        "emergency-stop.md": "safety-systems.instructions.md",
        "fail-safe.md": "safety-systems.instructions.md",
        "error-recovery.md": "safety-systems.instructions.md",
        "diagnostics.md": "safety-systems.instructions.md",
        "monitoring.md": "safety-systems.instructions.md",
        "limits.md": "safety-systems.instructions.md",
        "protection.md": "safety-systems.instructions.md",
        "fault-handling.md": "safety-systems.instructions.md",
        "system-health.md": "safety-systems.instructions.md",
        "safety-checks.md": "safety-systems.instructions.md",
        # Communication-related files -> comm-protocols.instructions.md
        "communication.md": "comm-protocols.instructions.md",
        "protocols.md": "comm-protocols.instructions.md",
        "uart.md": "comm-protocols.instructions.md",
        "can.md": "comm-protocols.instructions.md",
        "spi.md": "comm-protocols.instructions.md",
        "i2c.md": "comm-protocols.instructions.md",
        "ethernet.md": "comm-protocols.instructions.md",
        "messaging.md": "comm-protocols.instructions.md",
        "networking.md": "comm-protocols.instructions.md",
        # Error handling files -> error-handling.instructions.md
        "error.md": "error-handling.instructions.md",
        "errors.md": "error-handling.instructions.md",
        "exception.md": "error-handling.instructions.md",
        "exceptions.md": "error-handling.instructions.md",
        "fault.md": "error-handling.instructions.md",
        # Data type files -> data-types.instructions.md
        "types.md": "data-types.instructions.md",
        "datatypes.md": "data-types.instructions.md",
        "type-safety.md": "data-types.instructions.md",
        "type-definitions.md": "data-types.instructions.md",
        "constants.md": "data-types.instructions.md",
        "enums.md": "data-types.instructions.md",
    }
    return mapping


def find_instruction_mapping():
    """Map referenced files to actual files"""
    actual_files = get_actual_instruction_files()
    references = get_instruction_references()
    consolidation_mapping = get_consolidation_mapping()

    # Create mapping from referenced names to actual files
    mapping = {}
    missing = set()

    all_referenced = set()
    for file_refs in references.values():
        all_referenced.update(file_refs)

    for ref_file in all_referenced:
        if ref_file in actual_files:
            # Direct match
            mapping[ref_file] = actual_files[ref_file]
        elif ref_file in consolidation_mapping:
            # Consolidation match
            target_file = consolidation_mapping[ref_file]
            if target_file in actual_files.values():
                mapping[ref_file] = f".github/instructions/{target_file}"
            else:
                missing.add(ref_file)
        else:
            # Try to find closest match
            base_name = ref_file.replace(".md", "")
            candidates = []

            for actual_name, actual_path in actual_files.items():
                actual_base = actual_name.replace(".md", "")
                if base_name in actual_base or actual_base in base_name:
                    candidates.append((actual_name, actual_path))

            if candidates:
                # Use first candidate
                mapping[ref_file] = candidates[0][1]
            else:
                missing.add(ref_file)

    return mapping, missing, references, actual_files


def analyze_references(verbose=False):
    """Analyze instruction file references and generate report"""
    mapping, missing, references, actual_files = find_instruction_mapping()

    print("📋 Instruction Reference Analysis")
    print("=" * 50)

    if verbose:
        print(f"\n📁 Actual instruction files found: {len(actual_files)}")
        for name, path in actual_files.items():
            print(f"   ✅ {name} → {path}")

    print(
        f"\n🔗 Total unique references found: "
        f"{len(set().union(*references.values()))}"
    )

    print(f"\n✅ References that can be mapped: {len(mapping)}")
    if verbose:
        for ref, actual in mapping.items():
            print(f"   {ref} → {actual}")

    print(f"\n❌ Missing instruction files: {len(missing)}")
    for ref in sorted(missing):
        print(f"   {ref}")

    if verbose:
        print(f"\n📝 Files with references: {len(references)}")
        for src_file, refs in references.items():
            print(f"\n   📄 {src_file}:")
            for ref in refs:
                if ref in mapping:
                    print(f"      🔧 {ref} → {mapping[ref]}")
                else:
                    print(f"      ❌ {ref} (MISSING)")

    return mapping, missing, references, actual_files


def fix_references():
    """Fix references in source files"""
    mapping, missing, references, actual_files = find_instruction_mapping()

    print("\n🔧 Fixing source file references...")

    for src_file, refs in references.items():
        print(f"\n📄 Processing {src_file}...")

        with open(src_file, "r") as f:
            content = f.read()

        modified = False
        for ref in refs:
            old_pattern = f".instructions/{ref}"

            if ref in mapping:
                # Replace with correct path using SSOT
                new_path = mapping[ref]
                content = content.replace(old_pattern, new_path)
                print(f"   ✅ Fixed: {old_pattern} → {new_path}")
                modified = True
            else:
                print(f"   ❌ Cannot fix: {old_pattern} (no mapping found)")

        if modified:
            with open(src_file, "w") as f:
                f.write(content)
            print(f"   💾 Updated {src_file}")


def create_missing_files():
    """Create placeholder instruction files for missing references"""
    mapping, missing, references, actual_files = find_instruction_mapping()

    if not missing:
        print("✅ No missing instruction files to create")
        return

    print(f"\n📝 Creating {len(missing)} missing instruction files...")

    instruction_dir = INSTRUCTION_ROOT_DIR

    for ref_file in sorted(missing):
        base_name = ref_file.replace(".md", "")
        new_file = instruction_dir / f"{base_name}.instructions.md"

        content = f"""---
applyTo: "TBD"
description: "TODO: Add description for {base_name} guidance"
---

# {base_name.replace('-', ' ').title()} Instructions

## Overview
TODO: This instruction file was auto-generated to resolve a broken reference.
Please add appropriate content and update the applyTo field.

## Referenced From
This instruction file is referenced from:
"""

        # Find which source files reference this
        for src_file, refs in references.items():
            if ref_file in refs:
                content += f"- {src_file}\n"

        content += f"""
## TODO Items
- [ ] Add comprehensive guidance for {base_name}
- [ ] Update applyTo field with correct source file patterns
- [ ] Add SSOT references where applicable
- [ ] Add cross-references to related instruction files
- [ ] Add validation and testing guidelines
"""

        with open(new_file, "w") as f:
            f.write(content)

        print(f"   ✅ Created {new_file}")


def validate_references():
    """Validate that instruction file references in source code are valid"""
    errors = []

    # Pattern for instruction file references
    pattern = r"\.github/instructions/([^/\s]+\.instructions\.md)"

    # Scan source files for instruction references
    for src_file in Path("src").rglob("*.[ch]"):
        try:
            with open(src_file, "r", encoding="utf-8", errors="ignore") as f:
                content = f.read()
        except BaseException:
            continue

        matches = re.findall(pattern, content)
        for match in matches:
            instruction_file = INSTRUCTION_ROOT_DIR / match
            if not instruction_file.exists():
                errors.append(
                    f"Broken instruction reference in {src_file}: {match}"
                )

    if errors:
        print(f"\n❌ Found {len(errors)} broken instruction references:")
        for error in errors:
            print(f"   {error}")
    else:
        print("✅ All instruction references are valid")

    return errors


def main():
    """Main function with command-line interface"""
    parser = argparse.ArgumentParser(description="Instruction Management Tool")
    parser.add_argument(
        "action",
        choices=["analyze", "fix", "create", "validate"],
        help="Action to perform",
    )
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="Enable verbose output"
    )

    args = parser.parse_args()

    print("🔍 Instruction Management Tool")
    print("=" * 40)

    if args.action == "analyze":
        analyze_references(args.verbose)
    elif args.action == "fix":
        fix_references()
    elif args.action == "create":
        create_missing_files()
    elif args.action == "validate":
        validate_references()


if __name__ == "__main__":
    main()
