#!/usr/bin/env python3
"""
STATUS.md Automation System - Phase 1 Implementation

Automated updater for project status tracking and GitHub Copilot context.
Analyzes build results, git commits, and code changes to maintain current
STATUS.md.

Usage:
    python3 scripts/auto_update_status.py [options]

Options:
    --verbose           Enable detailed output
    --dry-run          Show changes without writing
    --source BUILD     Update triggered by build
    --source GIT       Update triggered by git commit
    --force            Force update even if no changes detected

Author: STM32H753ZI Development Team
Date: August 5, 2025
"""

import argparse
import os
import re
import subprocess
import sys
import json
import urllib.request
import urllib.error
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Tuple

# Fix Windows console encoding for Unicode emojis
if sys.platform == "win32":
    import io
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')
    sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8')


class StatusUpdater:
    """Core STATUS.md automation engine"""

    def __init__(
        self, workspace_root: str | None = None, verbose: bool = False
    ):
        if workspace_root is None:
            # Auto-detect workspace root
            workspace_root = os.getcwd()
            # Look for STATUS.md in current directory or parent directories
            current_path = Path(workspace_root).resolve()
            while current_path.parent != current_path:
                if (current_path / "STATUS.md").exists():
                    workspace_root = str(current_path)
                    break
                current_path = current_path.parent

        self.workspace = Path(workspace_root).resolve()
        self.status_file = self.workspace / "STATUS.md"
        self.verbose = verbose
        self.changes_made = False

        if not self.status_file.exists():
            raise FileNotFoundError(
                f"STATUS.md not found at {self.status_file}"
            )

    def log(self, message: str, level: str = "INFO") -> None:
        """Log message if verbose mode enabled"""
        if self.verbose:
            timestamp = datetime.now().strftime("%H:%M:%S")
            print(f"[{timestamp}] {level}: {message}")

    def read_status_content(self) -> str:
        """Read current STATUS.md content"""
        try:
            return self.status_file.read_text(encoding="utf-8")
        except Exception as e:
            raise RuntimeError(f"Failed to read STATUS.md: {e}")

    def write_status_content(
        self, content: str, dry_run: bool = False
    ) -> None:
        """Write updated content to STATUS.md"""
        if dry_run:
            self.log("DRY RUN: Would update STATUS.md")
            return

        try:
            self.status_file.write_text(content, encoding="utf-8")
            self.changes_made = True
            self.log("STATUS.md updated successfully")
        except Exception as e:
            raise RuntimeError(f"Failed to write STATUS.md: {e}")

    def update_timestamp(self, content: str) -> str:
        """Update the Last Updated timestamp"""
        self.log("Updating timestamp")
        current_time = datetime.now().strftime("%B %d, %Y")

        updated_content = re.sub(
            r"\*\*Last Updated\*\*:\s*.*",
            f"**Last Updated**: {current_time}",
            content,
        )

        if updated_content != content:
            self.log(f"Timestamp updated to: {current_time}")
            return updated_content
        return content

    def update_build_metrics(self, content: str) -> Tuple[str, Dict]:
        """Update build status and metrics from build artifacts"""
        self.log("Analyzing build metrics")
        metrics = {}

        try:
            # Check for ELF file in multiple possible locations
            possible_elf_paths = [
                self.workspace / "build" / "stm32h753_ihm02a1.elf",
                self.workspace / "build" / "Debug" / "stm32h753_ihm02a1.elf",
                self.workspace / "build" / "Release" / "stm32h753_ihm02a1.elf",
            ]

            elf_path = None
            for path in possible_elf_paths:
                if path.exists():
                    elf_path = path
                    break

            if elf_path:
                size_bytes = elf_path.stat().st_size
                size_kb = size_bytes / 1024
                flash_percent = (size_bytes / (2 * 1024 * 1024)) * 100

                # Extract memory usage from size output if available
                try:
                    result = subprocess.run(
                        ["arm-none-eabi-size", str(elf_path)],
                        capture_output=True,
                        text=True,
                        cwd=self.workspace,
                        check=False,
                    )

                    if result.returncode == 0 and result.stdout:
                        # Parse size output for detailed memory info
                        lines = result.stdout.strip().split("\n")
                        if len(lines) >= 2:
                            # Second line contains the actual numbers
                            parts = lines[1].split()
                            if len(parts) >= 3:
                                text_size = int(parts[0])
                                data_size = int(parts[1])
                                bss_size = int(parts[2])

                                flash_used = text_size + data_size
                                ram_used = data_size + bss_size

                                flash_pct = (
                                    flash_used / (2 * 1024 * 1024)
                                ) * 100
                                ram_pct = (ram_used / (1 * 1024 * 1024)) * 100
                                metrics.update(
                                    {
                                        "text_kb": text_size / 1024,
                                        "data_kb": data_size / 1024,
                                        "bss_kb": bss_size / 1024,
                                        "flash_used_kb": flash_used / 1024,
                                        "ram_used_kb": ram_used / 1024,
                                        "flash_percent": flash_pct,
                                        "ram_percent": ram_pct,
                                    }
                                )

                except Exception as e:
                    self.log(
                        f"Could not extract detailed size info: {e}", "WARNING"
                    )

                metrics.update({"size_kb": size_kb, "status": "✅ Passing"})

                # Only update flash_percent if memory parsing failed
                if "flash_percent" not in metrics:
                    metrics["flash_percent"] = (
                        size_bytes / (2 * 1024 * 1024)
                    ) * 100

                # Update build status line with enhanced metrics
                if "flash_used_kb" in metrics:
                    flash_used_pct = metrics["flash_percent"]
                    flash_free_pct = 100 - flash_used_pct

                    status_text = (
                        f"**Build Status**: ✅ Passing "
                        f'({metrics["flash_used_kb"]:.1f}KB firmware, '
                        f"{flash_used_pct:.2f}% flash used)"
                    )
                    content = re.sub(
                        r"\*\*Build Status\*\*:\s*.*",
                        status_text,
                        content,
                    )

                    # Log accurate memory usage from parsed sections
                    log_msg = (
                        f"Build metrics: {metrics['flash_used_kb']:.1f}KB "
                        f"flash used ({flash_used_pct:.2f}% used, "
                        f"{flash_free_pct:.1f}% free)"
                    )
                    self.log(log_msg)

                    # Update metrics section with detailed memory info
                    flash_usage_text = (
                        f"- **Flash Usage**: "
                        f'{metrics["flash_used_kb"]:.1f}KB / 2MB '
                        f"({flash_used_pct:.2f}% used, "
                        f"{flash_free_pct:.2f}% free)"
                    )
                    content = re.sub(
                        r"- \*\*Flash Usage\*\*:\s*.*",
                        flash_usage_text,
                        content,
                    )

                    # Update RAM usage if we have the information
                    content = re.sub(
                        r"- \*\*RAM Usage\*\*:\s*.*",
                        f'- **RAM Usage**: {metrics["ram_used_kb"]:.1f}KB / 1MB ({metrics["ram_percent"]:.2f}%)',  # noqa: E501
                        content,
                    )
                else:
                    # Fallback to basic metrics
                    flash_used_pct = flash_percent
                    flash_free_pct = 100 - flash_used_pct

                    content = re.sub(
                        r"\*\*Build Status\*\*:\s*.*",
                        f"**Build Status**: ✅ Passing ({size_kb:.1f}KB firmware, {flash_used_pct:.2f}% flash used)",  # noqa: E501
                        content,
                    )

                    content = re.sub(
                        r"- \*\*Flash Usage\*\*:\s*.*",
                        f"- **Flash Usage**: {size_kb:.1f}KB / 2MB ({flash_used_pct:.2f}% used, {flash_free_pct:.2f}% free)",  # noqa: E501
                        content,
                    )

                # Check for build artifacts and update build time
                if elf_path.stat().st_mtime:
                    build_time = datetime.fromtimestamp(
                        elf_path.stat().st_mtime
                    )
                    time_diff = datetime.now() - build_time
                    if (
                        time_diff.total_seconds() < 300
                    ):  # Recent build (< 5 minutes)
                        metrics["recent_build"] = True
                        metrics["build_time"] = build_time.strftime("%H:%M:%S")

                        # Update build time if section exists
                        content = re.sub(
                            r"- \*\*Build Time\*\*:\s*.*",
                            f'- **Build Time**: ~{int(time_diff.total_seconds())}s (last: {metrics["build_time"]})',  # noqa: E501
                            content,
                        )

            else:
                self.log("No build artifacts found - skipping build metrics")
                # Update status to show no artifacts
                content = re.sub(
                    r"\*\*Build Status\*\*:\s*.*",
                    "**Build Status**: ⏸️ No artifacts (run build to update)",
                    content,
                )

        except Exception as e:
            self.log(f"Error updating build metrics: {e}", "WARNING")
            metrics["error"] = str(e)

        return content, metrics

    def analyze_recent_commits(self, count: int = 5) -> Dict[str, List[str]]:
        """Analyze recent git commits for project activity"""
        self.log(f"Analyzing last {count} commits")

        try:
            result = subprocess.run(
                ["git", "log", "--oneline", f"-{count}"],
                capture_output=True,
                text=True,
                cwd=self.workspace,
            )

            if result.returncode != 0:
                self.log(f"Git log failed: {result.stderr}", "WARNING")
                return {"features": [], "fixes": [], "docs": [], "tests": []}

            commits = (
                result.stdout.strip().split("\n")
                if result.stdout.strip()
                else []
            )

            # Categorize commits by type
            categories = {"features": [], "fixes": [], "docs": [], "tests": []}

            patterns = {
                "features": [r"feat:", r"implement", r"add", r"create"],
                "fixes": [r"fix:", r"bug", r"correct", r"resolve"],
                "docs": [r"docs:", r"update.*md", r"readme", r"documentation"],
                "tests": [r"test:", r"spec", r"coverage", r"unittest"],
            }

            for commit in commits:
                if not commit.strip():
                    continue

                # Extract commit message (remove hash)
                commit_msg = (
                    commit.split(" ", 1)[1] if " " in commit else commit
                )
                commit_lower = commit_msg.lower()

                # Categorize based on patterns
                categorized = False
                for category, pattern_list in patterns.items():
                    if any(
                        re.search(pattern, commit_lower)
                        for pattern in pattern_list
                    ):
                        categories[category].append(
                            commit_msg[:80] + "..."
                            if len(commit_msg) > 80
                            else commit_msg
                        )
                        categorized = True
                        break

                if not categorized and commit_msg:
                    # Default to features for uncategorized commits
                    categories["features"].append(
                        commit_msg[:80] + "..."
                        if len(commit_msg) > 80
                        else commit_msg
                    )

            self.log(
                f"Commit analysis: {sum(len(cat) for cat in categories.values())} commits categorized"  # noqa: E501
            )
            return categories

        except Exception as e:
            self.log(f"Error analyzing commits: {e}", "WARNING")
            return {"features": [], "fixes": [], "docs": [], "tests": []}

    def analyze_code_status(self) -> Dict:
        """Analyze source code to determine current work status"""
        self.log("Analyzing source code status")

        analysis = {
            "todo_count": 0,
            "todo_items": [],
            "driver_status": {},
            "file_counts": {},
        }

        try:
            # Count files by type
            c_files = list(self.workspace.glob("src/**/*.c"))
            h_files = list(self.workspace.glob("src/**/*.h"))

            analysis["file_counts"] = {
                "c_files": len(c_files),
                "h_files": len(h_files),
                "total": len(c_files) + len(h_files),
            }

            # Analyze TODO comments
            for src_file in c_files + h_files:
                try:
                    content = src_file.read_text(encoding="utf-8")
                    todos = re.findall(
                        r"//\s*TODO:?\s*(.+)", content, re.IGNORECASE
                    )
                    analysis["todo_count"] += len(todos)

                    # Collect interesting TODOs (limit to avoid spam)
                    for todo in todos[:2]:
                        if len(todo.strip()) > 10:  # Skip trivial TODOs
                            analysis["todo_items"].append(
                                f"{src_file.name}: {todo.strip()}"
                            )

                except Exception as e:
                    self.log(f"Error reading {src_file}: {e}", "WARNING")

            # Check driver implementation status
            drivers = {
                "L6470": self.workspace / "src/drivers/l6470/l6470_driver.c",
                "AS5600": self.workspace
                / "src/drivers/as5600/as5600_driver.c",
            }

            for driver_name, driver_path in drivers.items():
                if driver_path.exists():
                    try:
                        content = driver_path.read_text(encoding="utf-8")
                        # Simple heuristic: count implemented functions
                        functions = len(
                            re.findall(
                                r"^[a-zA-Z_][a-zA-Z0-9_]*\s+[a-zA-Z_][a-zA-Z0-9_]*\s*\([^)]*\)\s*{",
                                content,
                                re.MULTILINE,
                            )
                        )
                        analysis["driver_status"][driver_name] = {
                            "exists": True,
                            "functions": functions,
                            "size_kb": driver_path.stat().st_size / 1024,
                            "status": (
                                "🔄 In Progress"
                                if functions > 0
                                else "📝 Started"
                            ),
                        }
                    except Exception as e:
                        analysis["driver_status"][driver_name] = {
                            "exists": True,
                            "error": str(e),
                        }
                else:
                    analysis["driver_status"][driver_name] = {
                        "exists": False,
                        "status": "⏸️ Not Started",
                    }

            self.log(
                f"Code analysis: {analysis['todo_count']} TODOs, {len(analysis['driver_status'])} drivers checked"  # noqa: E501
            )

        except Exception as e:
            self.log(f"Error analyzing code: {e}", "WARNING")
            analysis["error"] = str(e)

        return analysis

    def update_architecture_status(
        self, content: str, code_analysis: Dict
    ) -> str:
        """Update the architecture status section based on code analysis"""
        self.log("Updating architecture status")

        try:
            # Update driver status in architecture section
            if "driver_status" in code_analysis:
                driver_info = code_analysis["driver_status"]

                # Create status indicators
                driver_line = "├── drivers/             "
                if all(d.get("exists", False) for d in driver_info.values()):
                    if all(
                        d.get("functions", 0) > 5
                        for d in driver_info.values()
                        if d.get("exists")
                    ):
                        driver_line += "✅ L6470 + AS5600 drivers complete"
                    else:
                        driver_line += "🔄 L6470 + AS5600 drivers in progress"
                else:
                    driver_line += "🔄 L6470 + AS5600 drivers in progress"

                # Update the architecture status
                content = re.sub(r"├── drivers/\s+.*", driver_line, content)

        except Exception as e:
            self.log(f"Error updating architecture status: {e}", "WARNING")

        return content

    def validate_format(self, content: str) -> bool:
        """Validate STATUS.md format integrity"""
        required_sections = [
            "🔧 Setup Summary",
            "✅ Progress So Far",
            "🔄 Current Work",
            "⏭️ Next Steps",
            "🧠 Notes & Observations",
            "📊 Key Metrics",
            "🎯 Architecture Status",
            "🔗 Quick References",
        ]

        missing_sections = []
        for section in required_sections:
            if section not in content:
                missing_sections.append(section)

        if missing_sections:
            self.log(f"Missing sections: {missing_sections}", "WARNING")
            # Don't fail validation for missing sections in production-ready status
            # Just log the warning and continue

        # Check for basic markdown structure
        if not re.search(
            r"^# STM32H753ZI Project Status", content, re.MULTILINE
        ):
            self.log("Missing main title", "ERROR")
            return False

        self.log("Format validation passed")
        return True

    def run_update(
        self,
        source: str = "manual",
        dry_run: bool = False,
        force: bool = False,
    ) -> Dict:
        """Main update orchestration method"""
        self.log(f"Starting STATUS.md update (source: {source})")

        # Read current content
        content = self.read_status_content()
        original_content = content

        # Always update timestamp
        content = self.update_timestamp(content)

        # Update build metrics if triggered by build or forced
        build_metrics = {}
        if source in ["build", "manual"] or force:
            content, build_metrics = self.update_build_metrics(content)

        # Analyze commits and code
        commit_analysis = self.analyze_recent_commits()
        code_analysis = self.analyze_code_status()

        # Update architecture status
        content = self.update_architecture_status(content, code_analysis)

        # Validate format
        if not self.validate_format(content):
            raise RuntimeError("Format validation failed")

        # Write changes
        if content != original_content or force:
            self.write_status_content(content, dry_run)
        else:
            self.log("No changes detected")

        # Return summary
        return {
            "timestamp_updated": True,
            "build_metrics": build_metrics,
            "commit_analysis": commit_analysis,
            "code_analysis": code_analysis,
            "changes_made": self.changes_made,
            "source": source,
        }


def main():
    """Command line interface"""
    parser = argparse.ArgumentParser(
        description="STATUS.md Automation System",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s --verbose                   # Manual update with detailed output
  %(prog)s --source build              # Update after build
  %(prog)s --dry-run                   # Preview changes
  %(prog)s --force --verbose           # Force update with output
        """,
    )

    parser.add_argument(
        "--verbose", "-v", action="store_true", help="Enable detailed output"
    )
    parser.add_argument(
        "--dry-run", action="store_true", help="Show changes without writing"
    )
    parser.add_argument(
        "--source",
        choices=["manual", "build", "git", "git-commit"],
        default="manual",
        help="Update trigger source",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Force update even if no changes detected",
    )
    parser.add_argument(
        "--workspace",
        default=None,
        help="Workspace root directory (auto-detected if not specified)",
    )

    # New behavior controls: commit locally or trigger CI workflow
    parser.add_argument(
        "--auto-commit",
        action="store_true",
        help="When changes are made, automatically git add/commit and exit",
    )
    parser.add_argument(
        "--trigger-ci",
        action="store_true",
        help="When changes are made, trigger the configured GitHub Actions workflow (uses CI_TRIGGER_TOKEN env)",
    )
    parser.add_argument(
        "--ci-workflow",
        default="zero-touch-ci.yml",
        help="Workflow file name to dispatch when --trigger-ci is used (default: zero-touch-ci.yml)",
    )
    parser.add_argument(
        "--ci-ref",
        default="main",
        help="Git ref to use when dispatching the workflow (default: main)",
    )
    parser.add_argument(
        "--ci-token-env",
        default="CI_TRIGGER_TOKEN",
        help="Environment variable name that contains the PAT used to dispatch workflows",
    )

    # Git hook specific arguments
    parser.add_argument(
        "--commit-hash", help="Git commit hash (for git-commit source)"
    )
    parser.add_argument(
        "--commit-message", help="Git commit message (for git-commit source)"
    )

    args = parser.parse_args()

    try:
        # Create updater and run
        updater = StatusUpdater(
            workspace_root=args.workspace, verbose=args.verbose
        )
        result = updater.run_update(
            source=args.source, dry_run=args.dry_run, force=args.force
        )

        # Post-update actions: commit locally or trigger CI workflow based on flags
        def git_commit_status_file(message: str) -> int:
            try:
                subprocess.run(["git", "add", "STATUS.md"], check=True, cwd=updater.workspace)
                # Ensure pre-commit hooks that run Python can print UTF-8 on Windows
                env = os.environ.copy()
                env.setdefault("PYTHONIOENCODING", "utf-8")
                # Run git commit with environment that enables UTF-8 output for hooks
                subprocess.run(["git", "commit", "-m", message], check=True, cwd=updater.workspace, env=env)
                return 0
            except subprocess.CalledProcessError as e:
                print(f"❌ Git commit failed: {e}")
                # If available, show stderr for diagnostics
                try:
                    print(e.stderr)
                except Exception:
                    pass
                return 1

        def get_repo_owner_and_name() -> Tuple[str, str] | None:
            try:
                r = subprocess.run(["git", "config", "--get", "remote.origin.url"], capture_output=True, text=True, cwd=updater.workspace)
                url = r.stdout.strip()
                if not url:
                    return None
                # support git@github.com:owner/repo.git and https URLs
                if url.startswith("git@"):
                    # git@github.com:owner/repo.git
                    _, path = url.split(":", 1)
                    owner_repo = path.rstrip(".git")
                else:
                    # https://github.com/owner/repo.git
                    parts = url.rstrip(".git").split("/")
                    owner_repo = "/".join(parts[-2:])
                if "/" in owner_repo:
                    owner, repo = owner_repo.split("/", 1)
                    return owner, repo
            except Exception:
                return None
            return None

        def dispatch_github_workflow(owner: str, repo: str, workflow_file: str, ref: str, token: str) -> bool:
            url = f"https://api.github.com/repos/{owner}/{repo}/actions/workflows/{workflow_file}/dispatches"
            data = json.dumps({"ref": ref}).encode("utf-8")
            headers = {
                "Accept": "application/vnd.github+json",
                "Authorization": f"token {token}",
                "User-Agent": "auto-update-status-script",
                "Content-Type": "application/json",
            }
            req = urllib.request.Request(url, data=data, headers=headers, method="POST")
            try:
                with urllib.request.urlopen(req, timeout=15) as resp:
                    if 200 <= resp.status < 300:
                        return True
            except urllib.error.HTTPError as e:
                print(f"❌ Workflow dispatch failed: {e.code} {e.reason}")
                try:
                    print(e.read().decode())
                except Exception:
                    pass
            except Exception as e:
                print(f"❌ Workflow dispatch exception: {e}")
            return False

        # If changes made and user requested post-actions, perform them
        if result.get("changes_made"):
            # prefer commit if requested
            if args.auto_commit:
                commit_msg = args.commit_message or "chore(ssot): update STATUS.md — auto-update"
                rc = git_commit_status_file(commit_msg)
                if rc == 0:
                    print("✅ STATUS.md committed locally")
                else:
                    print("❌ STATUS.md commit failed")

            elif args.trigger_ci:
                token = os.environ.get(args.ci_token_env)
                repo_info = get_repo_owner_and_name()
                if not token:
                    print(f"❌ No CI token found in env '{args.ci_token_env}' - cannot dispatch workflow")
                elif not repo_info:
                    print("❌ Could not determine repo owner/name from git remote - cannot dispatch workflow")
                else:
                    owner, repo = repo_info
                    success = dispatch_github_workflow(owner, repo, args.ci_workflow, args.ci_ref, token)
                    if success:
                        print(f"✅ Dispatched workflow {args.ci_workflow} on {args.ci_ref}")
                    else:
                        print("❌ Workflow dispatch failed")

        else:
            # No changes made - inform user and do nothing unless trigger-ci explicitly requested with --ci-on-no-change
            print("ℹ️  No changes made; nothing to commit or dispatch.")

        # Report results
        if args.verbose or not result["changes_made"]:
            print(f"\n📊 Update Summary:")
            print(f"  Source: {result['source']}")
            print(
                f"  Changes Made: {'Yes' if result['changes_made'] else 'No'}"
            )
            print(
                f"  Build Status: {'Updated' if result['build_metrics'] else 'Skipped'}"
            )
            print(
                f"  Commits Analyzed: {sum(len(cat) for cat in result['commit_analysis'].values())}"
            )
            print(
                f"  TODOs Found: {result['code_analysis'].get('todo_count', 0)}"
            )

        if result["changes_made"]:
            print("✅ STATUS.md updated successfully!")
        else:
            print("ℹ️  No updates needed")

        return 0

    except Exception as e:
        print(f"❌ Error: {e}", file=sys.stderr)
        if args.verbose:
            import traceback

            traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())
