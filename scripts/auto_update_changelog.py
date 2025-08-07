#!/usr/bin/env python3
"""
Automatic Changelog Generation System
Integrates with existing STATUS.md automation to provide commit-driven changelog updates.

Usage:
    python scripts/auto_update_changelog.py [--dry-run] [--force] [--verbose]
    
Integration:
    Called automatically from post-commit hook or manual trigger
"""

import os
import sys
import json
import subprocess
import re
import argparse
from datetime import datetime
from typing import Dict, List, Optional, Tuple, Union, Any
from pathlib import Path

# Add project root to path for imports
sys.path.append(str(Path(__file__).parent.parent))

class ChangelogGenerator:
    def __init__(self, workspace_root: str, dry_run: bool = False, verbose: bool = False):
        self.workspace_root = Path(workspace_root)
        self.changelog_path = self.workspace_root / "docs" / "changelog.md"
        self.dry_run = dry_run
        self.verbose = verbose
        
        # Conventional commit patterns
        self.commit_patterns = {
            'feat': ('✅ Added', 'minor'),
            'fix': ('🐛 Fixed', 'patch'),
            'docs': ('📝 Documentation', 'patch'),
            'perf': ('⚡ Performance', 'minor'),
            'refactor': ('🔧 Refactored', 'patch'),
            'test': ('🧪 Testing', 'patch'),
            'ci': ('🔄 CI/CD', 'patch'),
            'build': ('🏗️ Build', 'patch'),
            'chore': ('🧹 Maintenance', 'patch'),
        }
        
        # Version pattern for semantic versioning
        self.version_pattern = r'\[(\d+)\.(\d+)\.(\d+)\]'
        
    def log(self, message: str, level: str = "INFO"):
        """Log message with timestamp and level"""
        if self.verbose or level in ["ERROR", "WARNING"]:
            timestamp = datetime.now().strftime("%H:%M:%S")
            print(f"[{timestamp}] {level}: {message}")

    def run_git_command(self, command: List[str], timeout: int = 5) -> Tuple[bool, str]:
        """Run git command with timeout and error handling"""
        try:
            result = subprocess.run(
                command,
                cwd=self.workspace_root,
                capture_output=True,
                text=True,
                timeout=timeout
            )
            return result.returncode == 0, result.stdout.strip()
        except subprocess.TimeoutExpired:
            self.log(f"Git command timed out: {' '.join(command)}", "WARNING")
            return False, ""
        except Exception as e:
            self.log(f"Git command failed: {e}", "ERROR")
            return False, ""

    def get_recent_commits(self, since_hours: int = 24) -> List[Dict[str, str]]:
        """Get recent commits for changelog generation"""
        since_time = f"{since_hours} hours ago"
        success, output = self.run_git_command([
            'git', 'log', f'--since={since_time}',
            '--pretty=format:%H|%s|%an|%ad|%B',
            '--date=iso'
        ])
        
        if not success or not output:
            self.log("No recent commits found", "INFO")
            return []
        
        commits = []
        current_commit = {}
        
        for line in output.split('\n'):
            if '|' in line and len(line.split('|')) >= 4:
                # New commit header
                if current_commit:
                    commits.append(current_commit)
                
                parts = line.split('|', 4)
                current_commit = {
                    'hash': parts[0][:8],
                    'subject': parts[1],
                    'author': parts[2],
                    'date': parts[3],
                    'body': parts[4] if len(parts) > 4 else ''
                }
            else:
                # Commit body continuation
                if current_commit:
                    current_commit['body'] += '\n' + line
        
        if current_commit:
            commits.append(current_commit)
        
        self.log(f"Found {len(commits)} recent commits", "INFO")
        return commits

    def parse_commit_message(self, commit: Dict[str, str]) -> Dict[str, Union[str, bool]]:
        """Parse conventional commit message"""
        subject = commit['subject']
        
        # Match conventional commit pattern: type(scope): description
        match = re.match(r'^(\w+)(?:\(([^)]+)\))?: (.+)$', subject)
        if not match:
            return {
                'type': 'chore',
                'scope': '',
                'description': subject,
                'breaking': False,
                'category': '🧹 Maintenance',
                'version_impact': 'patch'
            }
        
        commit_type, scope, description = match.groups()
        
        # Check for breaking changes
        breaking = ('BREAKING CHANGE' in commit['body'] or 
                   commit_type.endswith('!') or 
                   '!' in subject)
        
        if breaking:
            commit_type = commit_type.rstrip('!')
        
        # Get category and version impact
        category, version_impact = self.commit_patterns.get(
            commit_type, ('🔧 Other', 'patch')
        )
        
        if breaking:
            version_impact = 'major'
        
        return {
            'type': commit_type,
            'scope': scope or '',
            'description': description,
            'breaking': breaking,
            'category': category,
            'version_impact': version_impact,
            'hash': commit['hash'],
            'author': commit['author'],
            'date': commit['date']
        }

    def get_current_version(self) -> Tuple[int, int, int]:
        """Extract current version from changelog"""
        try:
            with open(self.changelog_path, 'r') as f:
                content = f.read()
            
            match = re.search(self.version_pattern, content)
            if match:
                groups = match.groups()
                if len(groups) == 3:
                    return (int(groups[0]), int(groups[1]), int(groups[2]))
                else:
                    self.log("Invalid version format in changelog", "WARNING")
                    return (1, 0, 0)
            else:
                self.log("No version found in changelog, starting from 1.0.0", "WARNING")
                return (1, 0, 0)
        except FileNotFoundError:
            self.log("Changelog not found, starting from 1.0.0", "WARNING")
            return (1, 0, 0)

    def calculate_next_version(self, current_version: Tuple[int, int, int], 
                             commits: List[Dict[str, Union[str, bool]]]) -> Tuple[int, int, int]:
        """Calculate next version based on commit types"""
        major, minor, patch = current_version
        
        has_breaking = any(c['breaking'] for c in commits)
        has_minor = any(c['version_impact'] == 'minor' for c in commits)
        has_patch = any(c['version_impact'] == 'patch' for c in commits)
        
        if has_breaking:
            return (major + 1, 0, 0)
        elif has_minor:
            return (major, minor + 1, 0)
        elif has_patch:
            return (major, minor, patch + 1)
        else:
            return current_version

    def get_build_metrics(self) -> Dict[str, str]:
        """Get current build metrics for inclusion in changelog"""
        metrics = {}
        
        # Get firmware size if available
        elf_path = self.workspace_root / "build" / "stm32h753_ihm02a1.elf"
        if elf_path.exists():
            success, output = self.run_git_command([
                'arm-none-eabi-size', str(elf_path)
            ])
            if success and output:
                lines = output.strip().split('\n')
                if len(lines) >= 2:
                    # Parse size output: text data bss dec hex filename
                    parts = lines[1].split()
                    if len(parts) >= 4:
                        text, data, bss, total = parts[:4]
                        metrics['flash_usage'] = f"{text} bytes FLASH"
                        metrics['ram_usage'] = f"{int(data) + int(bss)} bytes RAM"
                        
                        # Calculate percentages (STM32H753ZI: 2MB FLASH, 1MB RAM)
                        flash_percent = (int(text) / (2 * 1024 * 1024)) * 100
                        ram_percent = ((int(data) + int(bss)) / (1024 * 1024)) * 100
                        
                        metrics['flash_percent'] = f"{flash_percent:.2f}%"
                        metrics['ram_percent'] = f"{ram_percent:.2f}%"
        
        return metrics

    def group_commits_by_category(self, commits: List[Dict[str, Union[str, bool]]]) -> Dict[str, List[Dict[str, Union[str, bool]]]]:
        """Group commits by category for organized changelog"""
        grouped = {}
        
        for commit in commits:
            category = commit['category']
            if category not in grouped:
                grouped[category] = []
            grouped[category].append(commit)
        
        return grouped

    def generate_mermaid_diagram(self, commits: List[Dict[str, Union[str, bool]]], 
                               version: Tuple[int, int, int]) -> str:
        """Generate Mermaid diagram for significant changes"""
        if len(commits) < 3:  # Only generate for substantial updates
            return ""
        
        diagram = f"""```mermaid
graph TB
    subgraph "Version {version[0]}.{version[1]}.{version[2]} Updates"
"""
        
        # Add nodes for each category with commits
        grouped = self.group_commits_by_category(commits)
        
        node_id = 0
        for category, category_commits in grouped.items():
            if len(category_commits) > 0:
                # Clean category name for node ID
                clean_category = re.sub(r'[^a-zA-Z0-9]', '', category)
                
                commit_summary = f"{len(category_commits)} changes"
                diagram += f'        {clean_category}["{category}<br/>{commit_summary}"]\n'
                
                node_id += 1
        
        diagram += "    end\n```\n"
        
        return diagram

    def format_commit_entry(self, commit: Dict[str, Union[str, bool]]) -> str:
        """Format single commit for changelog"""
        scope_text = f"({commit['scope']})" if commit.get('scope') else ""
        breaking_text = " **BREAKING**" if commit.get('breaking') else ""
        
        entry = f"- **{commit['description']}**{scope_text}{breaking_text}"
        
        # Add hash reference for traceability
        entry += f" [`{commit['hash']}`]"
        
        return entry

    def generate_changelog_entry(self, commits: List[Dict[str, Union[str, bool]]], 
                                version: Tuple[int, int, int]) -> str:
        """Generate complete changelog entry"""
        if not commits:
            return ""
        
        version_str = f"{version[0]}.{version[1]}.{version[2]}"
        date_str = datetime.now().strftime("%Y-%m-%d")
        
        # Determine milestone type
        has_breaking = any(c['breaking'] for c in commits)
        has_major_features = len([c for c in commits if c['type'] == 'feat']) >= 3
        
        if has_breaking:
            milestone_text = "🚨 **BREAKING CHANGES**"
        elif has_major_features:
            milestone_text = "🎉 **MAJOR MILESTONE**"
        else:
            milestone_text = "📦 **UPDATE**"
        
        entry = f"\n## [{version_str}] - {date_str} {milestone_text}\n\n"
        
        # Add build metrics if available
        metrics = self.get_build_metrics()
        if metrics:
            entry += "### 📊 **System Metrics**\n"
            if 'flash_usage' in metrics:
                entry += f"- **Firmware Size**: {metrics['flash_usage']} ({metrics['flash_percent']} of 2MB)\n"
            if 'ram_usage' in metrics:
                entry += f"- **Memory Usage**: {metrics['ram_usage']} ({metrics['ram_percent']} of 1MB)\n"
            entry += "\n"
        
        # Add Mermaid diagram for significant updates
        mermaid_diagram = self.generate_mermaid_diagram(commits, version)
        if mermaid_diagram:
            entry += mermaid_diagram + "\n"
        
        # Group commits by category
        grouped = self.group_commits_by_category(commits)
        
        # Sort categories by importance
        category_order = [
            '✅ Added', '🐛 Fixed', '⚡ Performance', '📝 Documentation',
            '🔧 Refactored', '🧪 Testing', '🔄 CI/CD', '🏗️ Build', '🧹 Maintenance'
        ]
        
        for category in category_order:
            if category in grouped:
                entry += f"### {category}\n"
                for commit in grouped[category]:
                    entry += self.format_commit_entry(commit) + "\n"
                entry += "\n"
        
        # Add any remaining categories
        for category, category_commits in grouped.items():
            if category not in category_order:
                entry += f"### {category}\n"
                for commit in category_commits:
                    entry += self.format_commit_entry(commit) + "\n"
                entry += "\n"
        
        return entry

    def update_changelog(self, new_entry: str) -> bool:
        """Update changelog file with new entry"""
        try:
            # Read existing changelog
            if self.changelog_path.exists():
                with open(self.changelog_path, 'r') as f:
                    existing_content = f.read()
            else:
                existing_content = "# Changelog\n\n"
            
            # Find insertion point (after header, before first version)
            lines = existing_content.split('\n')
            insert_index = 0
            
            for i, line in enumerate(lines):
                if line.startswith('# Changelog'):
                    insert_index = i + 1
                    break
            
            # Insert new entry
            lines.insert(insert_index + 1, new_entry.rstrip())
            updated_content = '\n'.join(lines)
            
            if self.dry_run:
                print("DRY RUN - Changelog update preview:")
                print("=" * 50)
                print(new_entry)
                print("=" * 50)
                return True
            else:
                with open(self.changelog_path, 'w') as f:
                    f.write(updated_content)
                self.log(f"Updated changelog: {self.changelog_path}", "INFO")
                return True
                
        except Exception as e:
            self.log(f"Failed to update changelog: {e}", "ERROR")
            return False

    def generate_changelog_update(self, since_hours: int = 24) -> bool:
        """Main method to generate changelog update"""
        self.log("Starting automatic changelog generation", "INFO")
        
        # Get recent commits
        commits = self.get_recent_commits(since_hours)
        if not commits:
            self.log("No recent commits to process", "INFO")
            return True
        
        # Parse commits
        parsed_commits = [self.parse_commit_message(commit) for commit in commits]
        
        # Filter out maintenance commits that shouldn't be in changelog
        significant_commits = [
            c for c in parsed_commits 
            if c['type'] not in ['ci', 'chore'] or c['breaking']
        ]
        
        if not significant_commits:
            self.log("No significant commits for changelog", "INFO")
            return True
        
        # Calculate version
        current_version = self.get_current_version()
        next_version = self.calculate_next_version(current_version, significant_commits)
        
        if next_version == current_version:
            self.log("No version bump needed", "INFO")
            return True
        
        self.log(f"Version bump: {current_version} -> {next_version}", "INFO")
        
        # Generate changelog entry
        changelog_entry = self.generate_changelog_entry(significant_commits, next_version)
        
        if not changelog_entry:
            self.log("No changelog entry generated", "WARNING")
            return False
        
        # Update changelog
        success = self.update_changelog(changelog_entry)
        
        if success:
            self.log(f"Changelog updated successfully for version {next_version}", "INFO")
        
        return success

def main():
    parser = argparse.ArgumentParser(description="Automatic Changelog Generation")
    parser.add_argument('--dry-run', action='store_true', 
                       help='Preview changes without updating files')
    parser.add_argument('--force', action='store_true',
                       help='Force update even with no significant changes')
    parser.add_argument('--verbose', action='store_true',
                       help='Enable verbose logging')
    parser.add_argument('--since-hours', type=int, default=24,
                       help='Hours to look back for commits (default: 24)')
    
    args = parser.parse_args()
    
    # Get workspace root
    workspace_root = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
    
    # Create changelog generator
    generator = ChangelogGenerator(
        workspace_root=workspace_root,
        dry_run=args.dry_run,
        verbose=args.verbose
    )
    
    try:
        success = generator.generate_changelog_update(args.since_hours)
        sys.exit(0 if success else 1)
    except KeyboardInterrupt:
        print("\nChangelog generation cancelled by user")
        sys.exit(1)
    except Exception as e:
        print(f"ERROR: Changelog generation failed: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
