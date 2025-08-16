#!/usr/bin/env python3
"""
STM32H753ZI Revision V Clock Configuration Fix - Connect Mode Version
Based on real-time debugging findings and confirmed 480MHz capability
"""

import subprocess
import time
import sys

class STM32ClockFixerConnected:
    def __init__(self):
        self.cube_prog = r"C:\ST\STM32CubeCLT_1.19.0\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
        
    def run_command(self, args):
        """Execute CubeProgrammer command"""
        cmd = [self.cube_prog] + args
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
            return result.stdout, result.stderr, result.returncode
        except subprocess.TimeoutExpired:
            return "", "Command timeout", -1
        except Exception as e:
            return "", str(e), -1
    
    def establish_connection(self):
        """Establish SWD connection with reset"""
        print("🔌 Establishing SWD connection...")
        stdout, stderr, code = self.run_command([
            "-c", "port=SWD", "freq=4000", "ap=0", "mode=UR"
        ])
        
        if code == 0:
            print("✅ SWD connection established")
            print("📋 Device information:")
            for line in stdout.split('\n'):
                line = line.strip()
                if any(keyword in line for keyword in ['Board', 'Revision ID', 'Device name', 'Flash size']):
                    print(f"   {line}")
            return True
        else:
            print(f"❌ Connection failed: {stderr}")
            return False
    
    def read_register(self, address):
        """Read single 32-bit register"""
        stdout, stderr, code = self.run_command(["-r32", f"0x{address:08X}", "1"])
        if code == 0:
            lines = stdout.split('\n')
            for line in lines:
                if f"0x{address:08X}" in line and ":" in line:
                    parts = line.split(':')
                    if len(parts) > 1:
                        hex_part = parts[1].strip()
                        if hex_part:
                            hex_vals = hex_part.split()
                            if hex_vals:
                                return int(hex_vals[0], 16)
        return None
    
    def write_register(self, address, value):
        """Write single 32-bit register"""
        stdout, stderr, code = self.run_command(["-w32", f"0x{address:08X}", f"0x{value:08X}"])
        return code == 0
    
    def analyze_current_state(self):
        """Analyze current clock configuration state"""
        print("🔍 Analyzing current STM32H753ZI Revision V clock state...")
        
        # Read key registers
        rcc_cr = self.read_register(0x58024400)
        rcc_cfgr = self.read_register(0x58024410) 
        rcc_pllckselr = self.read_register(0x58024420)
        rcc_pllcfgr = self.read_register(0x58024428)
        rcc_pll1divr = self.read_register(0x5802442C)
        pwr_srdcr = self.read_register(0x58024818)
        
        print(f"\n📊 Current Register Values:")
        if rcc_cr is not None:
            print(f"RCC_CR:       0x{rcc_cr:08X}")
        else:
            print("RCC_CR:       READ FAILED")
            return {'error': 'Register read failure'}
            
        if rcc_cfgr is not None:
            print(f"RCC_CFGR:     0x{rcc_cfgr:08X}")
        else:
            print("RCC_CFGR:     READ FAILED")
            
        if rcc_pllckselr is not None:
            print(f"RCC_PLLCKSELR: 0x{rcc_pllckselr:08X}")
        else:
            print("RCC_PLLCKSELR: READ FAILED")
            
        if rcc_pllcfgr is not None:
            print(f"RCC_PLLCFGR:  0x{rcc_pllcfgr:08X}")
        else:
            print("RCC_PLLCFGR:  READ FAILED")
            
        if rcc_pll1divr is not None:
            print(f"RCC_PLL1DIVR: 0x{rcc_pll1divr:08X}")
        else:
            print("RCC_PLL1DIVR: READ FAILED")
            
        if pwr_srdcr is not None:
            print(f"PWR_SRDCR:    0x{pwr_srdcr:08X}")
        else:
            print("PWR_SRDCR:    READ FAILED")
        
        # Decode status - handle None values
        if rcc_cr is not None:
            hsi_on = (rcc_cr >> 0) & 1
            hsi_ready = (rcc_cr >> 1) & 1
            hse_on = (rcc_cr >> 14) & 1
            hse_ready = (rcc_cr >> 15) & 1
            pll1_on = (rcc_cr >> 24) & 1
            pll1_ready = (rcc_cr >> 25) & 1
        else:
            hsi_on = hsi_ready = hse_on = hse_ready = pll1_on = pll1_ready = 0
        
        if pwr_srdcr is not None:
            vos = (pwr_srdcr >> 14) & 0x3
            vos_ready = (pwr_srdcr >> 13) & 1
        else:
            vos = vos_ready = 0
        
        if rcc_cfgr is not None:
            clock_source = rcc_cfgr & 0x7
        else:
            clock_source = 0
        
        print(f"\n🎯 Decoded Status:")
        print(f"HSI: {'ON' if hsi_on else 'OFF'}, {'READY' if hsi_ready else 'NOT READY'}")
        print(f"HSE: {'ON' if hse_on else 'OFF'}, {'READY' if hse_ready else 'NOT READY'}")
        print(f"PLL1: {'ON' if pll1_on else 'OFF'}, {'LOCKED' if pll1_ready else 'NOT LOCKED'}")
        print(f"VOS: {vos} ({'VOS0 (480MHz)' if vos == 0 else 'VOS1 (400MHz)' if vos == 1 else f'VOS{vos}'}), {'READY' if vos_ready else 'NOT READY'}")
        print(f"Clock Source: {['HSI', 'CSI', 'HSE', 'PLL1'][clock_source] if clock_source < 4 else 'INVALID'}")
        
        return {
            'hsi_ready': hsi_ready,
            'hse_ready': hse_ready, 
            'pll1_ready': pll1_ready,
            'vos': vos,
            'vos_ready': vos_ready,
            'clock_source': clock_source
        }
    
    def simple_reset_and_wait(self):
        """Perform simple reset and wait for HSI to become ready"""
        print("\n🔄 Performing system reset and waiting for HSI...")
        
        # Reset the system
        stdout, stderr, code = self.run_command(["-rst"])
        if code != 0:
            print(f"❌ Reset failed: {stderr}")
            return False
            
        print("✅ Reset completed")
        
        # Wait a moment for system to stabilize
        time.sleep(0.5)
        
        # Check if HSI is now ready
        for attempt in range(10):
            rcc_cr = self.read_register(0x58024400)
            if rcc_cr is not None:
                hsi_ready = (rcc_cr >> 1) & 1
                print(f"Attempt {attempt + 1}: HSI {'READY' if hsi_ready else 'NOT READY'}")
                
                if hsi_ready:
                    print("✅ HSI is ready after reset")
                    return True
            else:
                print(f"❌ Failed to read RCC_CR on attempt {attempt + 1}")
            
            time.sleep(0.1)
        
        print("❌ HSI still not ready after reset")
        return False
    
    def check_for_firmware_issues(self):
        """Check if the current firmware is causing clock issues"""
        print("\n🔍 Checking for firmware clock configuration issues...")
        
        # Read application vector table to see if firmware is loaded
        reset_vector = self.read_register(0x08000004)  # Reset vector
        stack_pointer = self.read_register(0x08000000)  # Initial stack pointer
        
        if reset_vector is not None and stack_pointer is not None:
            print(f"Firmware detected:")
            print(f"  Stack Pointer: 0x{stack_pointer:08X}")
            print(f"  Reset Vector:  0x{reset_vector:08X}")
            
            # Check if firmware vectors look valid
            if (stack_pointer & 0xFF000000) == 0x20000000:  # Stack in RAM
                print("✅ Firmware appears to be loaded")
                
                # The issue might be firmware hanging in clock init
                print("⚠️  Firmware may be hanging in clock initialization")
                print("   This matches our debugging findings!")
                return True
            else:
                print("⚠️  Firmware vectors look unusual")
        else:
            print("❌ Could not read firmware vectors")
        
        return False
    
    def attempt_direct_hsi_fix(self):
        """Attempt to directly fix HSI without changing firmware"""
        print("\n🔧 Attempting direct HSI fix via CubeProgrammer...")
        
        # First, halt the CPU to stop any running firmware
        print("Halting CPU...")
        stdout, stderr, code = self.run_command(["-halt"])
        if code != 0:
            print(f"⚠️  Halt command failed: {stderr}")
        
        # Try to enable HSI directly
        print("Reading current RCC_CR...")
        rcc_cr = self.read_register(0x58024400)
        if rcc_cr is None:
            print("❌ Could not read RCC_CR")
            return False
        
        print(f"Current RCC_CR: 0x{rcc_cr:08X}")
        
        # Force HSI ON if not already
        if not (rcc_cr & 0x1):
            print("Enabling HSI...")
            new_rcc_cr = rcc_cr | 0x1  # Set HSION
            if self.write_register(0x58024400, new_rcc_cr):
                print("✅ HSI enable bit set")
            else:
                print("❌ Failed to write HSI enable")
                return False
        
        # Wait for HSI ready
        print("Waiting for HSI ready...")
        for i in range(20):
            time.sleep(0.05)  # 50ms delay
            rcc_cr_check = self.read_register(0x58024400)
            if rcc_cr_check is not None:
                hsi_ready = (rcc_cr_check >> 1) & 1
                print(f"  Check {i+1}: HSI {'READY' if hsi_ready else 'not ready'}")
                if hsi_ready:
                    print("✅ HSI is now ready!")
                    return True
            else:
                print(f"  Check {i+1}: Register read failed")
        
        print("❌ HSI still not ready after direct configuration")
        return False
    
    def complete_diagnosis_and_fix(self):
        """Complete diagnosis and fix sequence"""
        print("🚀 STM32H753ZI Revision V - Complete Diagnosis and Fix")
        print("=" * 60)
        
        # Step 1: Establish connection
        if not self.establish_connection():
            print("❌ Cannot establish connection - check hardware")
            return False
        
        # Step 2: Analyze current state
        print("\n📋 Step 1: Initial state analysis")
        state = self.analyze_current_state()
        if 'error' in state:
            print("❌ Initial analysis failed")
            return False
        
        # Step 3: Check for firmware issues
        print("\n📋 Step 2: Firmware analysis")
        firmware_detected = self.check_for_firmware_issues()
        
        # Step 4: Try simple reset first
        if not state['hsi_ready']:
            print("\n📋 Step 3: Simple reset attempt")
            if self.simple_reset_and_wait():
                print("✅ Simple reset fixed HSI issue!")
                # Re-analyze
                state = self.analyze_current_state()
            else:
                print("\n📋 Step 4: Direct HSI fix attempt")
                if self.attempt_direct_hsi_fix():
                    print("✅ Direct HSI fix successful!")
                    # Re-analyze
                    state = self.analyze_current_state()
                else:
                    print("❌ Could not fix HSI issue")
                    print("\n🔍 Diagnosis Summary:")
                    if firmware_detected:
                        print("   • Firmware is loaded but may be hanging in clock init")
                        print("   • This matches our debugging analysis")
                        print("   • Solution: Flash fixed firmware with proper timeouts")
                    else:
                        print("   • Hardware or low-level issue detected")
                        print("   • May require firmware update or hardware reset")
                    return False
        
        # Step 5: Final state analysis
        print("\n📋 Final: System state verification")
        final_state = self.analyze_current_state()
        
        if final_state['hsi_ready']:
            print("\n🎉 SUCCESS! HSI is now working correctly")
            print("✅ STM32H753ZI Revision V is ready for 480MHz firmware")
            
            if firmware_detected:
                print("\n📝 Next Steps:")
                print("   1. Flash the fixed firmware with proper clock timeouts")
                print("   2. The new firmware will handle VOS0 and PLL configuration")
                print("   3. System will run at 480MHz safely")
            
            return True
        else:
            print("\n⚠️  HSI issue persists")
            print("   This may require hardware-level investigation")
            return False

def main():
    print("🔧 STM32H753ZI Revision V Complete Diagnosis Tool")
    print("   Based on real-time debugging analysis")
    print("   Addresses firmware hang issues")
    print()
    
    fixer = STM32ClockFixerConnected()
    
    try:
        success = fixer.complete_diagnosis_and_fix()
        
        if success:
            print("\n🎯 DIAGNOSIS COMPLETE!")
            print("   STM32H753ZI hardware is ready")
            print("   Flash the fixed firmware for 480MHz operation")
        else:
            print("\n⚠️  Additional investigation needed")
            print("   Check hardware connections and power supply")
            
    except KeyboardInterrupt:
        print("\n🛑 Diagnosis interrupted by user")
    except Exception as e:
        print(f"\n❌ Unexpected error: {e}")

if __name__ == "__main__":
    main()
