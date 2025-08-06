#!/usr/bin/env python3
"""
API Contract Audit Tool
Identifies missing functions and types between test expectations and implementations.
"""

import os
import re
import json
from pathlib import Path

def scan_test_files_for_missing_apis():
    """Scan test files to find all function calls and types that don't exist in implementation."""
    
    test_dir = Path("tests/unit")
    src_dir = Path("src")
    
    # Patterns to find function calls and types in tests
    function_call_pattern = r'(\w+)\s*\('
    type_declaration_pattern = r'(\w+_t)\s+\w+'
    enum_usage_pattern = r'([A-Z_]+[A-Z_]*)\s*[,\);]'
    
    missing_apis = {
        "functions": set(),
        "types": set(), 
        "enums": set()
    }
    
    # Scan all test files
    for test_file in test_dir.glob("test_*.c"):
        print(f"📄 Scanning {test_file.name}")
        
        try:
            with open(test_file, 'r') as f:
                content = f.read()
                
            # Find function calls
            functions = re.findall(function_call_pattern, content)
            for func in functions:
                if not func.startswith(('TEST_', 'UNITY_', 'Mock', 'setUp', 'tearDown')):
                    missing_apis["functions"].add(func)
            
            # Find type declarations  
            types = re.findall(type_declaration_pattern, content)
            for type_name in types:
                if not type_name.startswith('Test'):
                    missing_apis["types"].add(type_name)
            
            # Find enum usage
            enums = re.findall(enum_usage_pattern, content)
            for enum_val in enums:
                if len(enum_val) > 3 and enum_val.isupper() and '_' in enum_val:
                    missing_apis["enums"].add(enum_val)
                    
        except Exception as e:
            print(f"❌ Error scanning {test_file}: {e}")
    
    # Remove Unity/test framework items
    unity_items = {'TEST_ASSERT_EQUAL', 'TEST_ASSERT_TRUE', 'TEST_ASSERT_FALSE', 
                   'TEST_ASSERT_GREATER_THAN', 'UNITY_BEGIN', 'UNITY_END', 'RUN_TEST'}
    missing_apis["functions"] -= unity_items
    
    return missing_apis

def check_implementation_exists(api_name, api_type):
    """Check if API exists in source code."""
    src_dir = Path("src")
    
    if api_type == "function":
        pattern = rf'\b{api_name}\s*\('
    elif api_type == "type":
        pattern = rf'typedef\s+.*\s+{api_name}\s*;'
    elif api_type == "enum":
        pattern = rf'\b{api_name}\b'
    else:
        return False
        
    for src_file in src_dir.rglob("*.h"):
        try:
            with open(src_file, 'r') as f:
                content = f.read()
                if re.search(pattern, content):
                    return True
        except:
            continue
            
    return False

def generate_api_analysis():
    """Generate comprehensive API analysis."""
    
    print("🔍 STM32H753ZI API Contract Audit Tool")
    print("=" * 50)
    
    missing_apis = scan_test_files_for_missing_apis()
    
    analysis = {
        "missing_functions": [],
        "missing_types": [],
        "missing_enums": [],
        "total_missing": 0
    }
    
    # Check functions
    print(f"\n📋 Checking {len(missing_apis['functions'])} functions...")
    for func in sorted(missing_apis['functions']):
        if not check_implementation_exists(func, "function"):
            analysis["missing_functions"].append(func)
            print(f"❌ Missing function: {func}()")
    
    # Check types
    print(f"\n📋 Checking {len(missing_apis['types'])} types...")
    for type_name in sorted(missing_apis['types']):
        if not check_implementation_exists(type_name, "type"):
            analysis["missing_types"].append(type_name)
            print(f"❌ Missing type: {type_name}")
    
    # Check enums
    print(f"\n📋 Checking {len(missing_apis['enums'])} enums...")
    for enum_val in sorted(missing_apis['enums']):
        if not check_implementation_exists(enum_val, "enum"):
            analysis["missing_enums"].append(enum_val)
            print(f"❌ Missing enum: {enum_val}")
    
    analysis["total_missing"] = (len(analysis["missing_functions"]) + 
                                len(analysis["missing_types"]) + 
                                len(analysis["missing_enums"]))
    
    # Generate summary
    print(f"\n📊 API CONTRACT AUDIT SUMMARY")
    print("=" * 40)
    print(f"Missing Functions: {len(analysis['missing_functions'])}")
    print(f"Missing Types: {len(analysis['missing_types'])}")
    print(f"Missing Enums: {len(analysis['missing_enums'])}")
    print(f"TOTAL MISSING: {analysis['total_missing']}")
    
    # Save detailed report
    report_file = "docs/API_AUDIT_REPORT.json"
    with open(report_file, 'w') as f:
        json.dump(analysis, f, indent=2)
    
    print(f"\n💾 Detailed report saved to: {report_file}")
    
    # Generate stub header template
    generate_stub_headers(analysis)
    
    return analysis

def generate_stub_headers(analysis):
    """Generate stub header file to make tests compile."""
    
    stub_content = '''/**
 * @file test_api_stubs.h
 * @brief Stub declarations to make tests compile
 * 
 * AUTO-GENERATED by api_audit_tool.py
 * TODO: Implement these functions properly
 */

#ifndef TEST_API_STUBS_H
#define TEST_API_STUBS_H

#include "config/hardware_config.h"
#include "common/system_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Missing Type Definitions (TODO: Move to proper headers)                  */
/* ========================================================================== */

'''
    
    # Add missing types
    for type_name in analysis["missing_types"]:
        if "State" in type_name:
            stub_content += f"typedef enum {{\n    {type_name.upper().replace('_T', '')}_UNKNOWN = 0\n}} {type_name};\n\n"
        elif "Info" in type_name or "Stats" in type_name:
            stub_content += f"typedef struct {{\n    uint32_t placeholder;\n}} {type_name};\n\n"
        else:
            stub_content += f"typedef uint32_t {type_name}; // TODO: Define proper type\n\n"
    
    # Add missing enums
    stub_content += "/* Missing Enum Values (TODO: Move to proper headers) */\n"
    for enum_val in analysis["missing_enums"]:
        stub_content += f"#define {enum_val} 0x{hash(enum_val) & 0xFFFF:04X} // TODO: Define proper value\n"
    
    stub_content += '''
/* ========================================================================== */
/* Missing Function Declarations (TODO: Implement properly)                 */
/* ========================================================================== */

'''
    
    # Add missing functions
    for func in analysis["missing_functions"]:
        if "get" in func.lower() and "state" in func.lower():
            stub_content += f"uint32_t {func}(void); // TODO: Implement\n"
        elif "is_" in func:
            stub_content += f"bool {func}(void); // TODO: Implement\n"
        elif "process" in func or "init" in func:
            stub_content += f"SystemError_t {func}(void); // TODO: Implement\n"
        else:
            stub_content += f"SystemError_t {func}(void); // TODO: Implement with proper signature\n"
    
    stub_content += '''
#ifdef __cplusplus
}
#endif

#endif /* TEST_API_STUBS_H */
'''
    
    # Save stub header
    stub_file = "tests/test_api_stubs.h"
    with open(stub_file, 'w') as f:
        f.write(stub_content)
    
    print(f"💾 Stub header generated: {stub_file}")
    print("📝 Include this in test files to make them compile")

if __name__ == "__main__":
    try:
        analysis = generate_api_analysis()
        
        print(f"\n🎯 NEXT STEPS:")
        print("1. Include 'tests/test_api_stubs.h' in failing test files")
        print("2. Fix HAL mock conflicts (conditional compilation)")
        print("3. Implement missing APIs in proper source files")
        print("4. Remove stubs as real implementations are added")
        
    except Exception as e:
        print(f"❌ Error during analysis: {e}")
        import traceback
        traceback.print_exc()
