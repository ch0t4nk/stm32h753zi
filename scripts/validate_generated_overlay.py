#!/usr/bin/env python3
"""Validate generated workspace overlay header contains required macros.

Usage: python scripts/validate_generated_overlay.py --file build/generated/include/workspace_config.generated.h

Exits with code 0 on success, non-zero on failure with a descriptive message.
"""
import argparse
import sys
from pathlib import Path

REQUIRED_MACROS = [
    'ARM_GCC_PATH',
    'ARM_GPP_PATH',
    'CMAKE_TOOLCHAIN_FILE_PATH',
]


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--file", default="build/generated/include/workspace_config.generated.h")
    args = p.parse_args()

    f = Path(args.file)
    if not f.exists():
        print(f"ERROR: Generated overlay not found: {f}", file=sys.stderr)
        sys.exit(2)

    text = f.read_text(encoding='utf-8')

    missing = []
    for macro in REQUIRED_MACROS:
        if macro not in text:
            missing.append(macro)

    if missing:
        print("ERROR: Generated overlay is missing required macros:", file=sys.stderr)
        for m in missing:
            print(f"  - {m}", file=sys.stderr)
        print("Make sure to run scripts/generate_workspace_config.py before building.", file=sys.stderr)
        sys.exit(3)

    print("Generated overlay validation: OK")
    sys.exit(0)


if __name__ == '__main__':
    main()
