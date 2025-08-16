#!/usr/bin/env python3
"""
STM32H753ZI Real-Time CubeProgrammer Debugging Session
Interactive debugging with register monitoring
"""

import subprocess
import time
import sys

class STM32Debugger:
    def __init__(self):
        self.cube_prog = r"C:\ST\STM32CubeCLT_1.19.0\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
        self.connected = False
        
    def run_command(self, args):
        """Execute CubeProgrammer command"""
        cmd = [self.cube_prog] + args
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=10)
            return result.stdout, result.stderr, result.returncode
        except subprocess.TimeoutExpired:
            return "", "Command timeout", -1
        except Exception as e:
            return "", str(e), -1
    
    def connect(self):
        """Connect to STM32"""
        print("🔌 Connecting to STM32H753ZI...")
        stdout, stderr, code = self.run_command(["-c", "port=SWD", "freq=4000"])
        if code == 0:
            self.connected = True
            print("✅ Connected successfully")
            return True
        else:
            print(f"❌ Connection failed: {stderr}")
            return False
    
    def halt(self):
        """Halt the CPU"""
        if not self.connected:
            return False
        stdout, stderr, code = self.run_command(["-c", "port=SWD", "-halt"])
        return code == 0
    
    def reset_and_halt(self):
        """Reset and immediately halt"""
        if not self.connected:
            return False
        print("🔄 Resetting and halting CPU...")
        stdout, stderr, code = self.run_command(["-c", "port=SWD", "-rst", "-halt"])
        return code == 0
    
    def read_register(self, address, count=1):
        """Read memory/register"""
        if not self.connected:
            return None
        stdout, stderr, code = self.run_command(["-c", "port=SWD", "-r32", f"0x{address:08X}", str(count)])
        if code == 0:
            # Parse the output to extract the value
            lines = stdout.split('\n')
            for line in lines:
                if f"0x{address:08X}" in line and ":" in line:
                    parts = line.split(':')
                    if len(parts) > 1:
                        hex_part = parts[1].strip()
                        if hex_part:  # Make sure we have data
                            hex_vals = hex_part.split()
                            if hex_vals:  # Make sure we have at least one value
                                return int(hex_vals[0], 16)
        return None
    
    def monitor_clock_registers(self):
        """Continuously monitor clock registers"""
        print("\n🔍 Starting real-time clock register monitoring...")
        print("Press Ctrl+C to stop\n")
        
        registers = {
            'RCC_CR': 0x58024400,
            'RCC_CFGR': 0x58024410, 
            'RCC_PLLCKSELR': 0x58024420,
            'RCC_PLLCFGR': 0x58024428,
            'RCC_PLL1DIVR': 0x5802442C,
            'PWR_SRDCR': 0x58024818
        }
        
        previous_values = {}
        
        try:
            while True:
                print(f"\n⏰ {time.strftime('%H:%M:%S')} - Register Status:")
                print("-" * 50)
                
                for name, addr in registers.items():
                    value = self.read_register(addr)
                    if value is not None:
                        changed = ""
                        if name in previous_values and previous_values[name] != value:
                            changed = " 🔄 CHANGED!"
                        print(f"{name:15}: 0x{value:08X}{changed}")
                        previous_values[name] = value
                    else:
                        print(f"{name:15}: ERROR reading")
                
                # Decode key status
                rcc_cr = previous_values.get('RCC_CR', 0)
                rcc_cfgr = previous_values.get('RCC_CFGR', 0)
                
                hsi_ready = (rcc_cr >> 1) & 1
                pll1_ready = (rcc_cr >> 25) & 1
                clock_source = rcc_cfgr & 0x7
                
                sources = {0: "HSI", 1: "CSI", 2: "HSE", 3: "PLL1"}
                
                print(f"\n📊 Status: HSI={'✅' if hsi_ready else '❌'} | PLL1={'✅' if pll1_ready else '❌'} | CLK={sources.get(clock_source, 'UNK')}")
                
                time.sleep(2)  # Update every 2 seconds
                
        except KeyboardInterrupt:
            print("\n🛑 Monitoring stopped")
    
    def step_through_clock_init(self):
        """Step-by-step debugging of clock initialization"""
        print("\n🐛 Step-by-step clock initialization debugging")
        
        # Reset and halt at start
        if not self.reset_and_halt():
            print("❌ Failed to reset and halt")
            return
        
        print("✅ System halted at reset")
        
        # Check initial state
        print("\n📋 Initial register state:")
        rcc_cr = self.read_register(0x58024400)
        print(f"RCC_CR: 0x{rcc_cr:08X}")
        
        input("\nPress Enter to resume execution and monitor...")
        
        # Resume and monitor
        stdout, stderr, code = self.run_command(["-c", "port=SWD", "-rst"])
        if code == 0:
            print("✅ System resumed")
            time.sleep(1)  # Let it run briefly
            self.halt()
            self.monitor_clock_registers()

def main():
    print("🚀 STM32H753ZI Real-Time Debugging Session")
    print("=" * 50)
    
    debugger = STM32Debugger()
    
    if not debugger.connect():
        sys.exit(1)
    
    while True:
        print("\n📋 Choose debugging option:")
        print("1. Monitor clock registers in real-time")
        print("2. Step through clock initialization")
        print("3. Reset and halt system")
        print("4. Read current register snapshot")
        print("5. Exit")
        
        choice = input("\nEnter choice (1-5): ").strip()
        
        if choice == '1':
            debugger.halt()
            debugger.monitor_clock_registers()
        elif choice == '2':
            debugger.step_through_clock_init()
        elif choice == '3':
            if debugger.reset_and_halt():
                print("✅ System reset and halted")
            else:
                print("❌ Reset failed")
        elif choice == '4':
            print("\n📊 Current register snapshot:")
            registers = {
                'RCC_CR': 0x58024400,
                'RCC_CFGR': 0x58024410,
                'PWR_SRDCR': 0x58024818
            }
            for name, addr in registers.items():
                value = debugger.read_register(addr)
                if value is not None:
                    print(f"{name}: 0x{value:08X}")
        elif choice == '5':
            break
        else:
            print("Invalid choice")
    
    print("👋 Debugging session ended")

if __name__ == "__main__":
    main()
