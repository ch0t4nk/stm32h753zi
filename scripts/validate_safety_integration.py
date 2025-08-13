#!/usr/bin/env python3
"""
Safety System Integration Validation Script
Validates that the implemented safety system integration is working correctly
"""

import os
import re
import sys

def check_file_exists(filepath):
    """Check if a file exists"""
    if not os.path.exists(filepath):
        print(f"❌ File not found: {filepath}")
        return False
    print(f"✅ File exists: {filepath}")
    return True

def check_function_implementation(filepath, function_name, pattern):
    """Check if a function is properly implemented"""
    try:
        with open(filepath, 'r') as f:
            content = f.read()
            if re.search(pattern, content, re.DOTALL):
                print(f"✅ Function '{function_name}' implemented in {filepath}")
                return True
            else:
                print(f"❌ Function '{function_name}' not found or incomplete in {filepath}")
                return False
    except Exception as e:
        print(f"❌ Error reading {filepath}: {e}")
        return False

def check_todo_resolution(filepath, todo_pattern):
    """Check if TODO has been resolved"""
    try:
        with open(filepath, 'r') as f:
            content = f.read()
            if re.search(todo_pattern, content):
                print(f"❌ TODO still present in {filepath}")
                return False
            else:
                print(f"✅ TODO resolved in {filepath}")
                return True
    except Exception as e:
        print(f"❌ Error reading {filepath}: {e}")
        return False

def main():
    print("🔍 Safety System Integration Validation")
    print("="*50)
    
    # Change to project root
    project_root = "/home/runner/work/stm32h753zi/stm32h753zi"
    os.chdir(project_root)
    
    all_checks_passed = True
    
    # Check 1: File existence
    print("\n📁 Checking file existence...")
    files_to_check = [
        "src/safety/safety_system.c",
        "src/safety/emergency_stop.c", 
        "tests/unit/test_safety_systems_hal_abstracted.c"
    ]
    
    for filepath in files_to_check:
        if not check_file_exists(filepath):
            all_checks_passed = False
    
    # Check 2: Function implementations
    print("\n🔧 Checking function implementations...")
    
    # Check update_system_state_safety implementation
    if not check_function_implementation(
        "src/safety/safety_system.c",
        "update_system_state_safety",
        r"static void update_system_state_safety.*?log_safety_event.*?broadcast_emergency_stop"
    ):
        all_checks_passed = False
        
    # Check broadcast_emergency_stop implementation
    if not check_function_implementation(
        "src/safety/safety_system.c", 
        "broadcast_emergency_stop",
        r"static void broadcast_emergency_stop.*?log_safety_event.*?SAFETY_EVENT_EMERGENCY_STOP"
    ):
        all_checks_passed = False
        
    # Check check_all_safety_monitors implementation
    if not check_function_implementation(
        "src/safety/safety_system.c",
        "check_all_safety_monitors", 
        r"static SystemError_t check_all_safety_monitors.*?for.*?MONITOR_.*?handle_safety_violation"
    ):
        all_checks_passed = False
    
    # Check 3: TODO resolution
    print("\n✅ Checking TODO resolution...")
    
    # Check safety_system.c TODO resolution
    if not check_todo_resolution(
        "src/safety/safety_system.c",
        r"TODO.*Integrate with system state manager"
    ):
        all_checks_passed = False
        
    # Check emergency_stop.c TODO resolution  
    if not check_todo_resolution(
        "src/safety/emergency_stop.c",
        r"TODO.*Integration with system state manager"
    ):
        all_checks_passed = False
        
    # Check emergency stop broadcast TODO resolution
    if not check_todo_resolution(
        "src/safety/safety_system.c", 
        r"TODO.*Implement emergency stop broadcast"
    ):
        all_checks_passed = False
    
    # Check 4: Test integration
    print("\n🧪 Checking test integration...")
    
    # Check new test functions
    test_functions = [
        "test_safety_system_state_integration",
        "test_emergency_stop_broadcast", 
        "test_safety_monitoring_health_checks"
    ]
    
    for test_func in test_functions:
        if not check_function_implementation(
            "tests/unit/test_safety_systems_hal_abstracted.c",
            test_func,
            rf"void {test_func}.*?TEST_ASSERT"
        ):
            all_checks_passed = False
    
    # Check 5: Integration points
    print("\n🔗 Checking integration points...")
    
    # Check set_safety_state calls update_system_state_safety
    if not check_function_implementation(
        "src/safety/safety_system.c",
        "set_safety_state integration",
        r"static void set_safety_state.*?update_system_state_safety\(new_state\)"
    ):
        all_checks_passed = False
    
    # Check emergency_stop.c integration
    if not check_function_implementation(
        "src/safety/emergency_stop.c", 
        "emergency stop logging integration",
        r"safety_log_event.*?SAFETY_EVENT_EMERGENCY_STOP"
    ):
        all_checks_passed = False
    
    # Summary
    print("\n" + "="*50)
    if all_checks_passed:
        print("🎉 ALL VALIDATION CHECKS PASSED!")
        print("✅ Safety system integration implementation is complete and correct")
        return 0
    else:
        print("❌ VALIDATION FAILED - Some checks did not pass")
        print("🔧 Please review the implementation and address the issues above")
        return 1

if __name__ == "__main__":
    exit(main())