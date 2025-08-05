#!/usr/bin/env python3
"""
STATUS.md Real-time Monitor for VS Code Integration
Phase 3: Enhanced VS Code integration with status bar updates

Provides real-time status monitoring and VS Code status bar integration
for the STM32H753ZI project automation system.

Usage:
    python3 scripts/status_monitor.py [options]
    
Options:
    --watch             Watch for file changes and update status
    --status-bar        Output status bar compatible format
    --json              Output JSON format for VS Code integration
    --interval SECONDS  Update interval for watch mode (default: 5)
    
Author: STM32H753ZI Development Team
Date: August 5, 2025
"""

import argparse
import json
import subprocess
import sys
import time
from datetime import datetime
from pathlib import Path
from typing import Dict


class StatusMonitor:
    """Real-time status monitoring for VS Code integration"""
    
    def __init__(self, workspace_root: str = "/workspaces/code"):
        self.workspace = Path(workspace_root).resolve()
        self.status_file = self.workspace / "STATUS.md"
        self.build_dir = self.workspace / "build"
        self.venv_python = self.workspace / ".venv" / "bin" / "python3"
        
        # Cache for status data
        self._cached_status = None
        self._last_update = None
    
    def get_current_status(self) -> Dict:
        """Get current project status"""
        status = {
            "timestamp": datetime.now().isoformat(),
            "project": "STM32H753ZI Stepper Motor Control",
            "phase": "Driver Implementation",
            "build_status": "unknown",
            "flash_usage": "unknown",
            "ram_usage": "unknown",
            "git_status": "unknown",
            "automation_active": self._check_automation_active()
        }
        
        # Check build status
        status["build_status"] = self._get_build_status()
        
        # Get memory usage if build exists
        if status["build_status"] == "passing":
            memory_info = self._get_memory_usage()
            status.update(memory_info)
        
        # Get git status
        status["git_status"] = self._get_git_status()
        
        # Get recent activity
        status["recent_activity"] = self._get_recent_activity()
        
        return status
    
    def _check_automation_active(self) -> bool:
        """Check if automation hooks are active"""
        hooks = [
            self.workspace / ".git" / "hooks" / "post-commit",
            self.workspace / ".git" / "hooks" / "pre-commit"
        ]
        return all(
            hook.exists() and hook.stat().st_mode & 0o111 
            for hook in hooks
        )
    
    def _get_build_status(self) -> str:
        """Get current build status"""
        elf_files = list(self.build_dir.rglob("*.elf"))
        if not elf_files:
            return "not_built"
        
        # Check if build is recent (within last hour)
        latest_elf = max(elf_files, key=lambda f: f.stat().st_mtime)
        age_minutes = (time.time() - latest_elf.stat().st_mtime) / 60
        
        if age_minutes < 60:
            return "passing"
        else:
            return "stale"
    
    def _get_memory_usage(self) -> Dict:
        """Get memory usage from build artifacts"""
        elf_files = list(self.build_dir.rglob("*.elf"))
        if not elf_files:
            return {"flash_usage": "unknown", "ram_usage": "unknown"}
        
        try:
            latest_elf = max(elf_files, key=lambda f: f.stat().st_mtime)
            result = subprocess.run(
                ["arm-none-eabi-size", str(latest_elf)],
                capture_output=True, text=True, check=True
            )
            
            lines = result.stdout.strip().split('\n')
            if len(lines) >= 2:
                # Parse size output: text data bss dec hex filename
                parts = lines[1].split()
                if len(parts) >= 3:
                    text = int(parts[0])
                    data = int(parts[1])
                    bss = int(parts[2])
                    
                    flash_used = text + data
                    ram_used = data + bss
                    
                    # STM32H753ZI has 2MB flash, 1MB RAM
                    flash_total = 2 * 1024 * 1024
                    ram_total = 1024 * 1024
                    
                    flash_percent = (flash_used / flash_total) * 100
                    ram_percent = (ram_used / ram_total) * 100
                    
                    flash_kb = flash_used / 1024
                    ram_kb = ram_used / 1024
                    
                    flash_pct = f"{flash_percent:.1f}%"
                    ram_pct = f"{ram_percent:.1f}%"
                    
                    return {
                        "flash_usage": f"{flash_kb:.1f}KB ({flash_pct})",
                        "ram_usage": f"{ram_kb:.1f}KB ({ram_pct})"
                    }
        except (subprocess.CalledProcessError, ValueError, IndexError):
            pass
        
        return {"flash_usage": "error", "ram_usage": "error"}
    
    def _get_git_status(self) -> str:
        """Get git repository status"""
        try:
            # Check if we have uncommitted changes
            result = subprocess.run(
                ["git", "status", "--porcelain"],
                cwd=self.workspace,
                capture_output=True, text=True, check=True
            )
            
            if result.stdout.strip():
                return "dirty"
            else:
                return "clean"
        except subprocess.CalledProcessError:
            return "error"
    
    def _get_recent_activity(self) -> str:
        """Get description of recent activity"""
        try:
            # Get last commit
            result = subprocess.run(
                ["git", "log", "-1", "--pretty=format:%s"],
                cwd=self.workspace,
                capture_output=True, text=True, check=True
            )
            
            commit_msg = result.stdout.strip()
            if len(commit_msg) > 50:
                commit_msg = commit_msg[:47] + "..."
            
            return commit_msg
        except subprocess.CalledProcessError:
            return "No recent activity"
    
    def format_status_bar(self, status: Dict) -> str:
        """Format status for VS Code status bar"""
        build_icon = {
            "passing": "$(check)",
            "not_built": "$(tools)",
            "stale": "$(clock)",
            "unknown": "$(question)"
        }.get(status["build_status"], "$(question)")
        
        git_icon = {
            "clean": "$(git-commit)",
            "dirty": "$(git-merge)",
            "error": "$(error)"
        }.get(status["git_status"], "$(question)")
        
        automation_icon = "$(pulse)" if status["automation_active"] else "$(circle-slash)"
        
        # Build main status text
        main_text = f"{build_icon} STM32 {automation_icon}"
        
        # Build tooltip
        tooltip_parts = [
            f"Build: {status['build_status']}",
            f"Git: {status['git_status']}",
            f"Automation: {'Active' if status['automation_active'] else 'Inactive'}"
        ]
        
        if status["flash_usage"] != "unknown":
            tooltip_parts.append(f"Flash: {status['flash_usage']}")
        
        if status["recent_activity"]:
            tooltip_parts.append(f"Last: {status['recent_activity']}")
        
        tooltip = " | ".join(tooltip_parts)
        
        return f"{main_text}|{tooltip}"
    
    def format_json(self, status: Dict) -> str:
        """Format status as JSON for VS Code integration"""
        return json.dumps(status, indent=2)
    
    def watch_mode(self, interval: int = 5) -> None:
        """Watch for changes and output status updates"""
        print(f"📡 Starting STATUS monitor (interval: {interval}s)")
        print("   Watching for changes in build/, .git/, and STATUS.md")
        print("   Press Ctrl+C to stop")
        
        try:
            while True:
                status = self.get_current_status()
                
                # Output status in watch format
                timestamp = datetime.now().strftime("%H:%M:%S")
                build_status = status["build_status"]
                git_status = status["git_status"]
                automation = "✅" if status["automation_active"] else "❌"
                
                print(f"[{timestamp}] Build: {build_status} | Git: {git_status} | Auto: {automation}")
                
                if status["flash_usage"] != "unknown":
                    print(f"           Flash: {status['flash_usage']} | RAM: {status['ram_usage']}")
                
                time.sleep(interval)
        except KeyboardInterrupt:
            print("\n📡 STATUS monitor stopped")


def main():
    """Command line interface"""
    parser = argparse.ArgumentParser(
        description="STATUS.md Real-time Monitor for VS Code Integration",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    
    parser.add_argument("--watch", action="store_true",
                       help="Watch for file changes and update status")
    parser.add_argument("--status-bar", action="store_true",
                       help="Output status bar compatible format")
    parser.add_argument("--json", action="store_true",
                       help="Output JSON format for VS Code integration")
    parser.add_argument("--interval", type=int, default=5,
                       help="Update interval for watch mode (default: 5)")
    parser.add_argument("--workspace", default="/workspaces/code",
                       help="Workspace root directory")
    
    args = parser.parse_args()
    
    try:
        monitor = StatusMonitor(workspace_root=args.workspace)
        status = monitor.get_current_status()
        
        if args.watch:
            monitor.watch_mode(interval=args.interval)
        elif args.status_bar:
            print(monitor.format_status_bar(status))
        elif args.json:
            print(monitor.format_json(status))
        else:
            # Default: human-readable status
            print("📊 STM32H753ZI Project Status")
            print("=" * 40)
            print(f"Phase: {status['phase']}")
            print(f"Build: {status['build_status']}")
            print(f"Git: {status['git_status']}")
            print(f"Automation: {'Active' if status['automation_active'] else 'Inactive'}")
            
            if status["flash_usage"] != "unknown":
                print(f"Flash: {status['flash_usage']}")
                print(f"RAM: {status['ram_usage']}")
            
            if status["recent_activity"]:
                print(f"Recent: {status['recent_activity']}")
    
    except Exception as e:
        print(f"❌ Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
