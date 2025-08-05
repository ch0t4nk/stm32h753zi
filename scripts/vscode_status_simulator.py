#!/usr/bin/env python3
"""
VS Code Status Bar Simulator
Shows what the status bar would display and provides manual controls
"""

import os
import sys
import time
import subprocess
from pathlib import Path

def clear_screen():
    os.system('clear' if os.name == 'posix' else 'cls')

def get_status():
    """Get current project status"""
    try:
        result = subprocess.run(
            ['python3', 'scripts/status_monitor.py', '--status-bar'],
            capture_output=True, text=True, cwd='/workspaces/code'
        )
        if result.returncode == 0:
            return result.stdout.strip()
    except Exception as e:
        return f"$(error) STM32|Error: {e}"
    return "$(question) STM32|Status unavailable"

def get_detailed_status():
    """Get detailed status information"""
    try:
        result = subprocess.run(
            ['python3', 'scripts/status_monitor.py'],
            capture_output=True, text=True, cwd='/workspaces/code'
        )
        if result.returncode == 0:
            return result.stdout.strip()
    except Exception as e:
        return f"Error getting detailed status: {e}"
    return "Detailed status unavailable"

def update_status():
    """Update STATUS.md"""
    print("🔄 Updating STATUS.md...")
    try:
        result = subprocess.run(
            ['python3', 'scripts/auto_update_status.py', '--verbose'],
            cwd='/workspaces/code'
        )
        if result.returncode == 0:
            print("✅ STATUS.md updated successfully!")
        else:
            print("❌ STATUS.md update failed")
    except Exception as e:
        print(f"❌ Error updating STATUS.md: {e}")

def build_project():
    """Build project with status update"""
    print("🔨 Building project...")
    try:
        result = subprocess.run(
            ['cmake', '--build', 'build'],
            cwd='/workspaces/code'
        )
        if result.returncode == 0:
            print("✅ Build successful!")
            print("🔄 Updating STATUS.md...")
            subprocess.run(
                ['python3', 'scripts/auto_update_status.py', '--source', 'build', '--verbose'],
                cwd='/workspaces/code'
            )
        else:
            print("❌ Build failed")
    except Exception as e:
        print(f"❌ Error building: {e}")

def show_help():
    """Show command help"""
    print("""
🎯 VS Code Status Bar Simulator Commands:
==========================================

📊 Status Commands:
  s, status    - Show current status bar format
  d, detail    - Show detailed project status
  j, json      - Show JSON status data

🔧 Action Commands:
  u, update    - Update STATUS.md manually
  b, build     - Build project with status update
  w, watch     - Start real-time monitoring

🎮 Control Commands:
  h, help      - Show this help
  q, quit      - Exit simulator
  r, refresh   - Refresh status display

💡 Tips:
- Status bar format: ICON TEXT | TOOLTIP
- Icons: $(check)=✓ $(pulse)=🟢 $(error)=❌
- Real VS Code integration available with extension
""")

def watch_mode():
    """Real-time monitoring mode"""
    print("📡 Starting real-time status monitoring...")
    print("Press Ctrl+C to stop")
    try:
        subprocess.run(
            ['python3', 'scripts/status_monitor.py', '--watch', '--interval', '3'],
            cwd='/workspaces/code'
        )
    except KeyboardInterrupt:
        print("\n📡 Real-time monitoring stopped")

def main():
    """Main interactive loop"""
    print("🎯 VS Code Status Bar Simulator")
    print("================================")
    print("Type 'h' for help, 'q' to quit")
    print()
    
    while True:
        # Show current status
        status_line = get_status()
        if '|' in status_line:
            main_text, tooltip = status_line.split('|', 1)
            print(f"📊 Status Bar: {main_text.strip()}")
            print(f"💬 Tooltip:    {tooltip.strip()}")
        else:
            print(f"📊 Status Bar: {status_line}")
        
        print()
        command = input("Command (h for help): ").lower().strip()
        
        if command in ['q', 'quit', 'exit']:
            print("👋 Goodbye!")
            break
        elif command in ['s', 'status']:
            status = get_status()
            print(f"\n📊 Current Status Bar Format:")
            print(f"   {status}")
        elif command in ['d', 'detail']:
            detail = get_detailed_status()
            print(f"\n📋 Detailed Status:")
            print(detail)
        elif command in ['j', 'json']:
            try:
                result = subprocess.run(
                    ['python3', 'scripts/status_monitor.py', '--json'],
                    capture_output=True, text=True, cwd='/workspaces/code'
                )
                print(f"\n📄 JSON Status:")
                print(result.stdout)
            except Exception as e:
                print(f"❌ Error getting JSON: {e}")
        elif command in ['u', 'update']:
            update_status()
        elif command in ['b', 'build']:
            build_project()
        elif command in ['w', 'watch']:
            watch_mode()
        elif command in ['h', 'help']:
            show_help()
        elif command in ['r', 'refresh']:
            clear_screen()
            print("🔄 Status refreshed")
        else:
            print(f"❓ Unknown command: {command}")
            print("   Type 'h' for help")
        
        print()

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n👋 Goodbye!")
    except Exception as e:
        print(f"\n❌ Error: {e}")
        sys.exit(1)
