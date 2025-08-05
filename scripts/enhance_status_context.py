#!/usr/bin/env python3
"""
Enhanced STATUS.md Context Tracking

Extends the basic automation to track conversation context and technical state
for better AI assistant handoffs between conversations.

Usage:
    python3 scripts/enhance_status_context.py [--update-session-context "desc"]

Features:
    - Session context tracking
    - Technical state snapshots
    - Decision point logging
    - Command history tracking
    - Integration with auto_update_status.py
"""

import argparse
import json
import subprocess
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List


class EnhancedStatusTracker:
    """Enhanced context tracking for STATUS.md"""

    def __init__(self, workspace_root: str = "/workspaces/code",
                 verbose: bool = False):
        self.workspace = Path(workspace_root).resolve()
        self.status_file = self.workspace / "STATUS.md"
        self.context_file = self.workspace / ".status_context.json"
        self.verbose = verbose

    def log(self, message: str, level: str = "INFO"):
        """Log messages with timestamp"""
        if self.verbose or level in ["ERROR", "WARNING"]:
            timestamp = datetime.now().strftime("%H:%M:%S")
            print(f"[{timestamp}] {level}: {message}")

    def read_status_content(self) -> str:
        """Read current STATUS.md content"""
        try:
            return self.status_file.read_text(encoding='utf-8')
        except Exception as e:
            self.log(f"Error reading STATUS.md: {e}", "ERROR")
            return ""

    def write_status_content(self, content: str) -> bool:
        """Write STATUS.md content"""
        try:
            self.status_file.write_text(content, encoding='utf-8')
            return True
        except Exception as e:
            self.log(f"Error writing STATUS.md: {e}", "ERROR")
            return False

    def load_context(self) -> Dict:
        """Load existing context data"""
        try:
            if self.context_file.exists():
                return json.loads(self.context_file.read_text())
            return {}
        except Exception as e:
            self.log(f"Error loading context: {e}", "WARNING")
            return {}

    def save_context(self, context: Dict) -> bool:
        """Save context data"""
        try:
            self.context_file.write_text(json.dumps(context, indent=2))
            return True
        except Exception as e:
            self.log(f"Error saving context: {e}", "ERROR")
            return False

    def get_current_git_state(self) -> Dict:
        """Get current git state for context"""
        try:
            # Get current branch
            branch_result = subprocess.run(
                ["git", "branch", "--show-current"],
                capture_output=True, text=True, cwd=self.workspace,
                check=False
            )
            current_branch = (branch_result.stdout.strip()
                              if branch_result.returncode == 0 else "unknown")

            # Get last commit
            commit_result = subprocess.run(
                ["git", "log", "--oneline", "-1"],
                capture_output=True, text=True, cwd=self.workspace,
                check=False
            )
            last_commit = (commit_result.stdout.strip()
                           if commit_result.returncode == 0 else "unknown")

            # Get status
            status_result = subprocess.run(
                ["git", "status", "--porcelain"],
                capture_output=True, text=True, cwd=self.workspace,
                check=False
            )
            has_changes = (bool(status_result.stdout.strip())
                           if status_result.returncode == 0 else False)

            return {
                "branch": current_branch,
                "last_commit": last_commit,
                "has_uncommitted_changes": has_changes,
                "timestamp": datetime.now().isoformat()
            }

        except Exception as e:
            self.log(f"Error getting git state: {e}", "WARNING")
            return {"error": str(e)}

    def get_build_state(self) -> Dict:
        """Get current build state"""
        try:
            build_state = {
                "artifacts": [],
                "cmake_configured": False,
                "python_env": "",
                "toolchain_status": {},
                "timestamp": datetime.now().isoformat()
            }

            # Check CMake configuration
            cmake_cache = self.workspace / "build" / "CMakeCache.txt"
            if cmake_cache.exists():
                build_state["cmake_configured"] = True
                build_state["cmake_config_time"] = datetime.fromtimestamp(
                    cmake_cache.stat().st_mtime).isoformat()

            # Check build artifacts
            build_dirs = [self.workspace / "build",
                          self.workspace / "build" / "Debug"]
            for build_dir in build_dirs:
                if build_dir.exists():
                    elf_files = list(build_dir.glob("*.elf"))
                    build_state["artifacts"].extend([
                        str(f.relative_to(self.workspace)) for f in elf_files])

            # Check Python environment
            if (self.workspace / ".venv").exists():
                build_state["python_env"] = ".venv/bin/python3 (venv)"

            # Check toolchain
            try:
                gcc_result = subprocess.run(
                    ["arm-none-eabi-gcc", "--version"],
                    capture_output=True, text=True, check=False)
                if gcc_result.returncode == 0:
                    version_line = gcc_result.stdout.split('\n')[0]
                    build_state["toolchain_status"]["gcc"] = version_line
            except Exception:
                build_state["toolchain_status"]["gcc"] = "not available"

            return build_state

        except Exception as e:
            self.log(f"Error getting build state: {e}", "WARNING")
            return {"error": str(e)}

    def update_session_context(self, description: str) -> bool:
        """Update session context with current state"""
        try:
            context = self.load_context()

            # Initialize session tracking if needed
            if "sessions" not in context:
                context["sessions"] = []

            # Create session entry
            session_entry = {
                "timestamp": datetime.now().isoformat(),
                "description": description,
                "git_state": self.get_current_git_state(),
                "build_state": self.get_build_state(),
                "session_id": len(context["sessions"]) + 1
            }

            context["sessions"].append(session_entry)

            # Keep only last 10 sessions
            if len(context["sessions"]) > 10:
                context["sessions"] = context["sessions"][-10:]

            # Update last session reference
            context["last_session"] = session_entry

            return self.save_context(context)

        except Exception as e:
            self.log(f"Error updating session context: {e}", "ERROR")
            return False

    def add_decision_point(self, question: str,
                           options: List[str]) -> bool:
        """Log a decision point for future reference"""
        try:
            context = self.load_context()

            if "decision_points" not in context:
                context["decision_points"] = []

            decision = {
                "timestamp": datetime.now().isoformat(),
                "question": question,
                "options": options,
                "resolved": False
            }

            context["decision_points"].append(decision)
            return self.save_context(context)

        except Exception as e:
            self.log(f"Error adding decision point: {e}", "ERROR")
            return False

    def update_technical_state(self, key: str, value: str) -> bool:
        """Update technical state information"""
        try:
            context = self.load_context()

            if "technical_state" not in context:
                context["technical_state"] = {}

            context["technical_state"][key] = {
                "value": value,
                "timestamp": datetime.now().isoformat()
            }

            return self.save_context(context)

        except Exception as e:
            self.log(f"Error updating technical state: {e}", "ERROR")
            return False

    def show_summary(self) -> None:
        """Show context summary"""
        try:
            context = self.load_context()

            print("=== STATUS.md Context Summary ===")

            # Last session
            if "last_session" in context:
                session = context["last_session"]
                print(f"\nLast Session:")
                print(f"  Description: {session['description']}")
                print(f"  Time: {session['timestamp']}")
                print(f"  Git: {session['git_state']['branch']} - "
                      f"{session['git_state']['last_commit'][:50]}...")

            # Pending decisions
            if "decision_points" in context:
                pending = [d for d in context["decision_points"]
                           if not d["resolved"]]
                if pending:
                    print(f"\nPending Decisions ({len(pending)}):")
                    for decision in pending[-3:]:  # Show last 3
                        print(f"  - {decision['question']}")

            # Technical state
            if "technical_state" in context:
                print(f"\nTechnical State ({len(context['technical_state'])}"
                      " items):")
                for key, data in list(context["technical_state"].items())[-5:]:
                    print(f"  {key}: {data['value']}")

        except Exception as e:
            self.log(f"Error showing summary: {e}", "ERROR")


def main():
    """Main CLI interface"""
    parser = argparse.ArgumentParser(
        description="Enhanced STATUS.md context tracking")
    parser.add_argument("--verbose", action="store_true",
                        help="Verbose output")
    parser.add_argument("--update-session", type=str,
                        help="Update session context with description")
    parser.add_argument("--decision", nargs="+",
                        help="Add decision point: question option1 option2...")
    parser.add_argument("--technical-state", nargs=2,
                        metavar=("KEY", "VALUE"),
                        help="Update technical state")
    parser.add_argument("--show-summary", action="store_true",
                        help="Show context summary")
    parser.add_argument("--update-status", action="store_true",
                        help="Update STATUS.md with current context")

    args = parser.parse_args()

    if len(sys.argv) == 1:
        parser.print_help()
        return

    tracker = EnhancedStatusTracker(verbose=args.verbose)

    if args.update_session:
        success = tracker.update_session_context(args.update_session)
        print("✅ Session context updated" if success
              else "❌ Failed to update session context")

    elif args.decision:
        if len(args.decision) < 3:
            print("❌ Decision requires: question option1 option2 [...]")
            return
        question = args.decision[0]
        options = args.decision[1:]
        success = tracker.add_decision_point(question, options)
        print("✅ Decision point added" if success
              else "❌ Failed to add decision point")

    elif args.technical_state:
        key, value = args.technical_state
        success = tracker.update_technical_state(key, value)
        print("✅ Technical state updated" if success
              else "❌ Failed to update technical state")

    elif args.show_summary:
        tracker.show_summary()

    elif args.update_status:
        print("🔄 STATUS.md integration not yet implemented")

    else:
        parser.print_help()


if __name__ == "__main__":
    main()
