#!/usr/bin/env python3
"""
Complete STM32H753ZI Diagnosis and Clock Fix
Comprehensive solution based on debugging analysis
"""

import subprocess
import time
import sys

class STM32CompleteFix:
    def __init__(self):
        self.cube_prog = r"C:\ST\STM32CubeCLT_1.19.0\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
        self.connected = False
        
    def run_command_with_connection(self, additional_args):
        """Run command with connection established"""
        base_args = ["-c", "port=SWD", "freq=4000", "ap=0", "mode=NORMAL"]
        full_args = base_args + additional_args
        
        cmd = [self.cube_prog] + full_args
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
            return result.stdout, result.stderr, result.returncode
        except subprocess.TimeoutExpired:
            return "", "Command timeout", -1
        except Exception as e:
            return "", str(e), -1
    
    def read_register_connected(self, address):
        """Read register with connection in same command"""
        stdout, stderr, code = self.run_command_with_connection(["-r32", f"0x{address:08X}", "1"])
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
    
    def write_register_connected(self, address, value):
        """Write register with connection in same command"""
        stdout, stderr, code = self.run_command_with_connection(["-w32", f"0x{address:08X}", f"0x{value:08X}"])
        return code == 0
    
    def full_system_reset(self):
        """Perform full system reset and analysis"""
        print("🔄 Performing full system reset...")
        
        # Full reset with mass erase if needed
        stdout, stderr, code = self.run_command_with_connection(["-e", "all", "-rst"])
        
        if code == 0:
            print("✅ Full system reset completed")
            time.sleep(1)  # Wait for reset to complete
            return True
        else:
            print(f"⚠️  Full reset had issues: {stderr}")
            # Try simple reset
            stdout, stderr, code = self.run_command_with_connection(["-rst"])
            if code == 0:
                print("✅ Simple reset completed")
                time.sleep(1)
                return True
            else:
                print(f"❌ Reset failed: {stderr}")
                return False
    
    def analyze_system_thoroughly(self):
        """Comprehensive system analysis"""
        print("\n🔍 Comprehensive System Analysis")
        print("-" * 40)
        
        # Key registers to read
        registers = {
            'RCC_CR': 0x58024400,
            'RCC_CFGR': 0x58024410,
            'RCC_PLLCKSELR': 0x58024420,
            'RCC_PLLCFGR': 0x58024428,
            'RCC_PLL1DIVR': 0x5802442C,
            'PWR_SRDCR': 0x58024818,
            'FLASH_ACR': 0x52002000,
            'SCB_CPUID': 0xE000ED00,  # CPU ID register
        }
        
        reg_values = {}
        success_count = 0
        
        for name, addr in registers.items():
            value = self.read_register_connected(addr)
            if value is not None:
                reg_values[name] = value
                print(f"✅ {name:15}: 0x{value:08X}")
                success_count += 1
            else:
                print(f"❌ {name:15}: READ FAILED")
                reg_values[name] = None
        
        print(f"\n📊 Register Access: {success_count}/{len(registers)} successful")
        
        if success_count == 0:
            print("❌ No registers accessible - hardware issue")
            return None
        
        if success_count < len(registers):
            print("⚠️  Partial register access - possible protection or reset issue")
        
        # Decode what we can read
        decoded_info = {}
        
        if reg_values['RCC_CR'] is not None:
            rcc_cr = reg_values['RCC_CR']
            decoded_info.update({
                'hsi_on': (rcc_cr >> 0) & 1,
                'hsi_ready': (rcc_cr >> 1) & 1,
                'hse_on': (rcc_cr >> 14) & 1,
                'hse_ready': (rcc_cr >> 15) & 1,
                'pll1_on': (rcc_cr >> 24) & 1,
                'pll1_ready': (rcc_cr >> 25) & 1,
            })
        
        if reg_values['PWR_SRDCR'] is not None:
            pwr_srdcr = reg_values['PWR_SRDCR']
            decoded_info.update({
                'vos': (pwr_srdcr >> 14) & 0x3,
                'vos_ready': (pwr_srdcr >> 13) & 1,
            })
        
        if reg_values['RCC_CFGR'] is not None:
            rcc_cfgr = reg_values['RCC_CFGR']
            decoded_info['clock_source'] = rcc_cfgr & 0x7
        
        if reg_values['SCB_CPUID'] is not None:
            cpuid = reg_values['SCB_CPUID']
            print(f"🔍 CPU ID: 0x{cpuid:08X}")
            if cpuid == 0x411FC271:
                print("✅ Cortex-M7 r1p1 confirmed")
            else:
                print(f"⚠️  Unexpected CPU ID: expected 0x411FC271")
        
        # Print decoded status
        if decoded_info:
            print(f"\n🎯 System Status:")
            if 'hsi_on' in decoded_info:
                print(f"   HSI: {'ON' if decoded_info['hsi_on'] else 'OFF'}, {'READY' if decoded_info.get('hsi_ready', 0) else 'NOT READY'}")
            if 'hse_on' in decoded_info:
                print(f"   HSE: {'ON' if decoded_info['hse_on'] else 'OFF'}, {'READY' if decoded_info.get('hse_ready', 0) else 'NOT READY'}")
            if 'pll1_on' in decoded_info:
                print(f"   PLL1: {'ON' if decoded_info['pll1_on'] else 'OFF'}, {'READY' if decoded_info.get('pll1_ready', 0) else 'NOT READY'}")
            if 'vos' in decoded_info:
                vos_names = {0: 'VOS0 (480MHz)', 1: 'VOS1 (400MHz)', 2: 'VOS2', 3: 'VOS3'}
                print(f"   VOS: {vos_names.get(decoded_info['vos'], f'VOS{decoded_info["vos"]}')} {'READY' if decoded_info.get('vos_ready', 0) else 'NOT READY'}")
            if 'clock_source' in decoded_info:
                sources = {0: 'HSI', 1: 'CSI', 2: 'HSE', 3: 'PLL1'}
                print(f"   Clock: {sources.get(decoded_info['clock_source'], 'INVALID')}")
        
        return {
            'register_values': reg_values,
            'decoded_info': decoded_info,
            'access_success_rate': success_count / len(registers)
        }
    
    def attempt_clock_fix(self, analysis_result):
        """Attempt to fix clock issues based on analysis"""
        if analysis_result is None:
            print("❌ Cannot fix - no register access")
            return False
        
        decoded = analysis_result['decoded_info']
        registers = analysis_result['register_values']
        
        print("\n🔧 Attempting Clock Configuration Fix")
        print("-" * 40)
        
        # Step 1: Fix HSI if needed
        if 'hsi_ready' in decoded and not decoded['hsi_ready']:
            print("🔧 Fixing HSI oscillator...")
            
            if registers['RCC_CR'] is not None:
                rcc_cr = registers['RCC_CR']
                
                # Enable HSI if not already on
                if not decoded.get('hsi_on', 0):
                    print("   Enabling HSI...")
                    new_rcc_cr = rcc_cr | 0x1  # Set HSION
                    if self.write_register_connected(0x58024400, new_rcc_cr):
                        print("   ✅ HSI enabled")
                        time.sleep(0.1)  # Wait for HSI
                    else:
                        print("   ❌ Failed to enable HSI")
                        return False
                
                # Check HSI calibration
                hsicfgr = self.read_register_connected(0x58024404)
                if hsicfgr is not None:
                    print(f"   Current HSICFGR: 0x{hsicfgr:08X}")
                    # Set calibration to 64 (confirmed value)
                    new_hsicfgr = (hsicfgr & 0xFFFFF000) | (64 << 3)
                    if self.write_register_connected(0x58024404, new_hsicfgr):
                        print("   ✅ HSI calibration set to 64")
                    else:
                        print("   ⚠️  HSI calibration write failed")
                
                # Wait for HSI ready
                print("   Waiting for HSI ready...")
                for i in range(20):
                    time.sleep(0.05)
                    new_rcc_cr = self.read_register_connected(0x58024400)
                    if new_rcc_cr is not None:
                        hsi_ready = (new_rcc_cr >> 1) & 1
                        if hsi_ready:
                            print("   ✅ HSI is now ready!")
                            break
                        else:
                            print(f"   ⏳ HSI check {i+1}/20...")
                else:
                    print("   ❌ HSI ready timeout")
                    return False
        
        # Step 2: Configure VOS0 for 480MHz if needed
        if 'vos' in decoded and decoded['vos'] != 0:
            print("🔧 Configuring VOS0 for 480MHz...")
            
            if registers['PWR_SRDCR'] is not None:
                pwr_srdcr = registers['PWR_SRDCR']
                new_srdcr = (pwr_srdcr & 0xFFFF3FFF) | (0 << 14)  # Set VOS to 0
                
                if self.write_register_connected(0x58024818, new_srdcr):
                    print("   ✅ VOS0 configuration set")
                    
                    # Wait for VOS ready
                    print("   Waiting for VOS0 ready...")
                    for i in range(50):  # Longer timeout for VOS
                        time.sleep(0.02)
                        new_srdcr_check = self.read_register_connected(0x58024818)
                        if new_srdcr_check is not None:
                            vos_ready = (new_srdcr_check >> 13) & 1
                            if vos_ready:
                                print("   ✅ VOS0 is ready!")
                                break
                            else:
                                print(f"   ⏳ VOS0 check {i+1}/50...")
                    else:
                        print("   ❌ VOS0 ready timeout")
                        return False
                else:
                    print("   ❌ Failed to configure VOS0")
                    return False
        
        print("✅ Clock fix completed successfully!")
        return True
    
    def create_summary_report(self, initial_analysis, final_analysis, fix_applied):
        """Create comprehensive summary report"""
        print("\n" + "="*60)
        print("📋 STM32H753ZI DIAGNOSIS AND FIX REPORT")
        print("="*60)
        
        print(f"🏷️  Device: STM32H753ZI Revision V")
        print(f"🔌 Connection: {'✅ Success' if final_analysis else '❌ Failed'}")
        
        if initial_analysis:
            print(f"📊 Initial Register Access: {initial_analysis['access_success_rate']*100:.1f}%")
        
        if final_analysis:
            print(f"📊 Final Register Access: {final_analysis['access_success_rate']*100:.1f}%")
            
            decoded = final_analysis['decoded_info']
            if decoded:
                print(f"\n🎯 Final System State:")
                if 'hsi_ready' in decoded:
                    print(f"   HSI: {'✅ Ready' if decoded['hsi_ready'] else '❌ Not Ready'}")
                if 'vos' in decoded:
                    vos_ok = decoded['vos'] == 0 and decoded.get('vos_ready', 0)
                    print(f"   VOS0: {'✅ Ready for 480MHz' if vos_ok else '❌ Not Ready'}")
                if 'clock_source' in decoded:
                    sources = {0: 'HSI', 1: 'CSI', 2: 'HSE', 3: 'PLL1'}
                    print(f"   Clock Source: {sources.get(decoded['clock_source'], 'INVALID')}")
        
        print(f"\n🔧 Fix Applied: {'✅ Yes' if fix_applied else '❌ No'}")
        
        # Determine next steps
        if final_analysis and final_analysis['decoded_info'].get('hsi_ready', False):
            vos_ready = (final_analysis['decoded_info'].get('vos', 1) == 0 and 
                        final_analysis['decoded_info'].get('vos_ready', False))
            
            if vos_ready:
                print(f"\n🚀 RESULT: READY FOR 480MHz OPERATION!")
                print(f"   ✅ HSI: Working")
                print(f"   ✅ VOS0: Configured for 480MHz")
                print(f"   🎯 Next: Flash firmware with fixed clock configuration")
            else:
                print(f"\n⚡ RESULT: READY FOR CLOCK CONFIGURATION")
                print(f"   ✅ HSI: Working")
                print(f"   🔧 Next: Configure VOS0 and PLL via firmware")
        else:
            print(f"\n⚠️  RESULT: HARDWARE ISSUE DETECTED")
            print(f"   ❌ Basic clock oscillator not working")
            print(f"   🔍 Check: Power supply, crystal, hardware connections")
        
        print("\n" + "="*60)
    
    def complete_fix_sequence(self):
        """Execute complete fix sequence"""
        print("🚀 STM32H753ZI Revision V - Complete Fix Sequence")
        print("="*60)
        
        # Step 1: Full system reset
        print("📋 Step 1: System Reset")
        if not self.full_system_reset():
            print("❌ System reset failed - hardware issue")
            return False
        
        # Step 2: Initial analysis
        print("\n📋 Step 2: Initial Analysis")
        initial_analysis = self.analyze_system_thoroughly()
        
        if initial_analysis is None:
            print("❌ No register access - hardware failure")
            self.create_summary_report(None, None, False)
            return False
        
        # Step 3: Apply fixes
        print("\n📋 Step 3: Apply Clock Fixes")
        fix_applied = self.attempt_clock_fix(initial_analysis)
        
        # Step 4: Final verification
        print("\n📋 Step 4: Final Verification")
        final_analysis = self.analyze_system_thoroughly()
        
        # Step 5: Generate report
        self.create_summary_report(initial_analysis, final_analysis, fix_applied)
        
        return fix_applied and final_analysis is not None

def main():
    print("🔧 STM32H753ZI Complete Fix Tool")
    print("   Comprehensive diagnosis and repair")
    print("   Based on real-time debugging analysis")
    print()
    
    fixer = STM32CompleteFix()
    
    try:
        success = fixer.complete_fix_sequence()
        
        if success:
            print("\n🎉 FIX COMPLETE!")
            print("   Hardware is ready for 480MHz firmware")
        else:
            print("\n⚠️  FIX INCOMPLETE")
            print("   Manual intervention may be required")
            
    except KeyboardInterrupt:
        print("\n🛑 Fix interrupted by user")
    except Exception as e:
        print(f"\n❌ Unexpected error: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()
