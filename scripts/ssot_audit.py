#!/usr/bin/env python3
"""
SSOT Audit Script

Produces a machine-readable report of all hardcoded-value violations
and include-dependency issues discovered by the project's
`scripts/validate_ssot.py` functions. Writes JSON to the path
provided by --output (default: reports/ssot_audit.json).

This script is intentionally small and non-invasive — it imports
the validator functions and calls them to produce an audit that can
be used to plan staged migrations of constants into SSOT headers.
"""

import argparse
import json
import os
from pathlib import Path

# Try to import the validation helpers from the existing script. The
# script defines functions at module level, so importing usually works
# when run from the repository root. If that fails (for example when
# running as a loose script), fall back to executing the module with
# runpy and extracting the functions.
try:
    import scripts.validate_ssot as validator
except Exception:
    try:
        from importlib import import_module

        validator = import_module("scripts.validate_ssot")
    except Exception:
        import runpy

        module_globals = runpy.run_path("scripts/validate_ssot.py")

        class _ValidatorProxy:
            pass

        validator = _ValidatorProxy()

        # Copy expected functions into proxy if present
        for name in (
            "find_hardcoded_values",
            "check_include_dependencies",
            "validate_ssot_structure",
            "validate_config_consistency",
        ):
            if name in module_globals:
                setattr(validator, name, module_globals[name])


def collect_violations(search_paths):
    all_violations = []
    for sp in search_paths:
        p = Path(sp)
        if not p.exists():
            continue
        for f in p.rglob("*.*"):
            if f.suffix.lower() not in [".c", ".h", ".cpp", ".hpp"]:
                continue
            try:
                file_violations = validator.find_hardcoded_values(str(f))
            except Exception as e:
                print(f"Warning: validator.find_hardcoded_values failed for {f}: {e}")
                file_violations = []
            if file_violations:
                all_violations.extend(file_violations)
    return all_violations


def main():
    parser = argparse.ArgumentParser(description="Produce SSOT audit report")
    parser.add_argument(
        "--output",
        "-o",
        default="reports/ssot_audit.json",
        help="Output JSON path",
    )
    parser.add_argument(
        "--search-paths",
        "-s",
        nargs="*",
        default=["src", "Core"],
        help="Top-level search paths to scan",
    )
    args = parser.parse_args()

    out_path = Path(args.output)
    out_path.parent.mkdir(parents=True, exist_ok=True)

    print("🔎 Running SSOT audit scan...")
    violations = collect_violations(args.search_paths)
    include_violations = []
    try:
        include_violations = validator.check_include_dependencies()
    except Exception as e:
        print(f"Warning: include dependency check failed: {e}")

    missing_files = []
    try:
        missing_files = validator.validate_ssot_structure()
    except Exception as e:
        print(f"Warning: validate_ssot_structure failed: {e}")

    inconsistencies = []
    try:
        inconsistencies = validator.validate_config_consistency()
    except Exception as e:
        print(f"Warning: validate_config_consistency failed: {e}")

    report = {
        "summary": {
            "violations_count": len(violations),
            "include_issues_count": len(include_violations),
            "missing_files_count": len(missing_files),
            "inconsistencies_count": len(inconsistencies),
        },
        "violations": violations,
        "include_violations": include_violations,
        "missing_files": missing_files,
        "inconsistencies": inconsistencies,
    }

    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(report, f, indent=2)

    print(f"✅ Audit complete. Report written to {out_path}")
    print(
        f"Summary: {report['summary']['violations_count']} violations, {report['summary']['include_issues_count']} include issues, {report['summary']['missing_files_count']} missing files"
    )


if __name__ == "__main__":
    main()
