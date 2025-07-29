#!/usr/bin/env python3
"""
SSOT Validation Script for STM32H753ZI Project
Validates that configuration is consistent across all files.

TODO: See .instructions/ssot-validation.md for extending validation rules
TODO: See .instructions/build-validation.md for integration with build system
"""

import os
import re
import sys
import json
from pathlib import Path
from typing import List, Dict, Tuple

def find_hardcoded_values(file_path: str) -> List[Dict]:
    """Find potential hardcoded values that should be in SSOT."""
    hardcoded_patterns = [
        (r'GPIO_PIN_\d+', 'GPIO pin numbers should be in hardware_config.h'),
        (r'SPI[1-3]', 'SPI instances should be in hardware_config.h'),
        (r'I2C[1-3]', 'I2C instances should be in hardware_config.h'), 
        (r'USART[1-6]', 'UART instances should be in hardware_config.h'),
        (r'FDCAN[1-2]', 'CAN instances should be in hardware_config.h'),
        (r'0x[0-9A-Fa-f]{2,}', 'Hex addresses should be in config headers'),
        (r'[0-9]+\s*[Kk]?[Bb]ps', 'Baud rates should be in comm_config.h'),
        (r'192\.168\.\d+\.\d+', 'IP addresses should be in comm_config.h'),
        (r'[0-9]+\s*[Rr][Pp][Mm]', 'Motor speeds should be in motor_config.h'),
        (r'[0-9]+\s*[Dd]eg', 'Angles should use angle_deg_t type'),
        (r'[0-9]+\s*[Mm][Ss]', 'Time values should use timestamp_ms_t type'),
    ]
    
    violations = []
    
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            for line_num, line in enumerate(f, 1):
                # Skip comments and includes
                if line.strip().startswith('//') or line.strip().startswith('#include'):
                    continue
                
                # Skip lines that are defining the constants (in config files)
                if '#define' in line and 'config' in str(file_path).lower():
                    continue
                
                for pattern, description in hardcoded_patterns:
                    matches = re.findall(pattern, line)
                    if matches:
                        violations.append({
                            'file': str(file_path),
                            'line': line_num,
                            'content': line.strip(),
                            'matches': matches,
                            'description': description
                        })
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
    
    return violations

def validate_ssot_structure() -> List[str]:
    """Validate that SSOT structure exists."""
    required_files = [
        'src/config/hardware_config.h',
        'src/config/comm_config.h', 
        'src/config/motor_config.h',
        'src/config/safety_config.h',
        'src/config/build_config.h',
        'src/common/error_codes.h',
        'src/common/system_state.h',
        'src/common/data_types.h'
    ]
    
    missing_files = []
    for file_path in required_files:
        if not os.path.exists(file_path):
            missing_files.append(file_path)
    
    return missing_files

def check_include_dependencies() -> List[Dict]:
    """Check that source files include appropriate SSOT headers."""
    violations = []
    
    # Files that should include hardware_config.h
    hardware_dependent_files = []
    
    # Find .c files that might need SSOT includes
    for file_path in Path('src').rglob('*.c'):
        if 'config' in str(file_path):
            continue
            
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                
                # Check for GPIO usage without hardware_config.h include
                if ('GPIO_' in content or 'SPI' in content or 'I2C' in content) and \
                   '#include "config/hardware_config.h"' not in content:
                    violations.append({
                        'file': str(file_path),
                        'issue': 'Uses hardware constants but does not include hardware_config.h',
                        'recommendation': 'Add #include "config/hardware_config.h"'
                    })
                
                # Check for communication usage without comm_config.h include
                if ('UART_' in content or 'CAN_' in content or 'ETH_' in content) and \
                   '#include "config/comm_config.h"' not in content:
                    violations.append({
                        'file': str(file_path),
                        'issue': 'Uses communication constants but does not include comm_config.h',
                        'recommendation': 'Add #include "config/comm_config.h"'
                    })
                
                # Check for motor usage without motor_config.h include
                if ('MOTOR_' in content or 'L6470_' in content) and \
                   '#include "config/motor_config.h"' not in content:
                    violations.append({
                        'file': str(file_path),
                        'issue': 'Uses motor constants but does not include motor_config.h',
                        'recommendation': 'Add #include "config/motor_config.h"'
                    })
                
        except Exception as e:
            print(f"Error checking includes in {file_path}: {e}")
    
    return violations

def validate_config_consistency() -> List[Dict]:
    """Validate consistency between configuration files."""
    inconsistencies = []
    
    # Check that motor count is consistent
    try:
        # Read motor config
        with open('src/config/motor_config.h', 'r') as f:
            motor_config = f.read()
        
        # Read system state
        with open('src/common/system_state.h', 'r') as f:
            state_config = f.read()
        
        # Extract MAX_MOTORS from motor config
        motor_match = re.search(r'#define\s+MAX_MOTORS\s+(\d+)', motor_config)
        if motor_match:
            max_motors = int(motor_match.group(1))
            
            # Check if system state uses MAX_MOTORS constant (correct approach)
            if 'MotorState_t motors[MAX_MOTORS]' not in state_config:
                inconsistencies.append({
                    'issue': f'MAX_MOTORS defined as {max_motors} but system_state.h does not use MAX_MOTORS constant',
                    'files': ['src/config/motor_config.h', 'src/common/system_state.h'],
                    'recommendation': 'Use MotorState_t motors[MAX_MOTORS] in system_state.h'
                })
            
            # Check if encoder arrays also use MAX_MOTORS constant
            if 'EncoderState_t encoders[MAX_MOTORS]' not in state_config:
                inconsistencies.append({
                    'issue': f'MAX_MOTORS defined as {max_motors} but system_state.h encoders array does not use MAX_MOTORS constant',
                    'files': ['src/config/motor_config.h', 'src/common/system_state.h'],
                    'recommendation': 'Use EncoderState_t encoders[MAX_MOTORS] in system_state.h'
                })
    
    except Exception as e:
        print(f"Error checking config consistency: {e}")
    
    return inconsistencies

def generate_report(violations: List[Dict], missing_files: List[str], 
                   include_violations: List[Dict], inconsistencies: List[Dict]) -> None:
    """Generate validation report."""
    print("🔍 SSOT Validation Report for STM32H753ZI Project")
    print("=" * 60)
    
    # Check SSOT structure
    if missing_files:
        print("❌ Missing SSOT files:")
        for file in missing_files:
            print(f"   📄 {file}")
        print(f"\n💡 Run: mkdir -p src/config src/common")
        print("💡 TODO: See .instructions/ssot-setup.md to create missing files")
    else:
        print("✅ SSOT structure complete")
    
    # Check for hardcoded values
    if violations:
        print(f"\n⚠️  Found {len(violations)} potential SSOT violations:")
        
        # Group by file for better readability
        by_file = {}
        for violation in violations:
            file = violation['file']
            if file not in by_file:
                by_file[file] = []
            by_file[file].append(violation)
        
        for file, file_violations in list(by_file.items())[:5]:  # Show first 5 files
            print(f"\n   📁 {file}:")
            for violation in file_violations[:3]:  # Show first 3 violations per file
                print(f"      Line {violation['line']}: {violation['content'][:60]}...")
                print(f"      💡 {violation['description']}")
        
        if len(by_file) > 5:
            print(f"\n   ... and {len(by_file) - 5} more files with violations")
    else:
        print("✅ No SSOT violations found in source files")
    
    # Check include dependencies
    if include_violations:
        print(f"\n⚠️  Found {len(include_violations)} include dependency issues:")
        for violation in include_violations[:5]:  # Show first 5
            print(f"   📁 {violation['file']}")
            print(f"      Issue: {violation['issue']}")
            print(f"      💡 {violation['recommendation']}")
    else:
        print("✅ Include dependencies look correct")
    
    # Check configuration consistency
    if inconsistencies:
        print(f"\n⚠️  Found {len(inconsistencies)} configuration inconsistencies:")
        for inconsistency in inconsistencies:
            print(f"   🔗 {inconsistency['issue']}")
            print(f"      Files: {', '.join(inconsistency['files'])}")
            print(f"      💡 {inconsistency['recommendation']}")
    else:
        print("✅ Configuration consistency checks passed")
    
    # Summary and recommendations
    total_issues = len(violations) + len(missing_files) + len(include_violations) + len(inconsistencies)
    
    print(f"\n📊 Summary:")
    print(f"   • Missing SSOT files: {len(missing_files)}")
    print(f"   • Hardcoded value violations: {len(violations)}")
    print(f"   • Include dependency issues: {len(include_violations)}")
    print(f"   • Configuration inconsistencies: {len(inconsistencies)}")
    print(f"   • Total issues found: {total_issues}")
    
    if total_issues == 0:
        print("\n🎉 SSOT validation passed! All checks completed successfully.")
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

def main():
    """Main validation function."""
    if not os.path.exists('src'):
        print("❌ Error: src directory not found. Run from project root.")
        return 1
    
    print("🔍 Starting SSOT validation...")
    
    # Check SSOT structure
    missing_files = validate_ssot_structure()
    
    # Check for hardcoded values in source files
    all_violations = []
    src_dir = Path('src')
    
    if src_dir.exists():
        for file_path in src_dir.rglob('*.c'):
            # Skip config files (they're allowed to have these values)
            if 'config' in str(file_path):
                continue
                
            violations = find_hardcoded_values(str(file_path))
            all_violations.extend(violations)
    
    # Check include dependencies
    include_violations = check_include_dependencies()
    
    # Check configuration consistency
    inconsistencies = validate_config_consistency()
    
    # Generate report
    generate_report(all_violations, missing_files, include_violations, inconsistencies)
    
    # Return exit code based on findings
    total_issues = len(all_violations) + len(missing_files) + len(include_violations) + len(inconsistencies)
    return 1 if total_issues > 0 else 0

if __name__ == '__main__':
    exit_code = main()
    sys.exit(exit_code)

# TODO: See .instructions/ssot-validation.md for extending this script with:
# - Configuration value range validation
# - Cross-reference checking between files  
# - Automatic fix suggestions
# - Integration with pre-commit hooks
# - CI/CD pipeline integration
