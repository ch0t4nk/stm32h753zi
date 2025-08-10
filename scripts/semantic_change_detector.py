#!/usr/bin/env python3
"""
Semantic Change Detection System for STM32H753ZI Project
========================================================

Part of Feature Request FTR-014: Delta Update System for Semantic Search Database
Detects changes in the codebase and prepares delta updates for semantic search.

Author: STM32H753ZI Development Team
Date: August 10, 2025
"""

import os
import sys
import json
import hashlib
import argparse
from pathlib import Path
from typing import Dict, List, Set, Tuple, Optional, NamedTuple
from dataclasses import dataclass
import subprocess
from datetime import datetime

# Add project root to Python path
sys.path.insert(0, str(Path(__file__).parent.parent))

@dataclass
class FileMetadata:
    """Metadata for tracking file changes"""
    path: str
    size: int
    modified_time: float
    hash_md5: str
    content_type: str
    
class ChangeType:
    """Types of changes detected"""
    NEW = "NEW"
    MODIFIED = "MODIFIED" 
    DELETED = "DELETED"
    RENAMED = "RENAMED"

@dataclass
class FileChange:
    """Represents a detected file change"""
    change_type: str
    file_path: str
    old_path: Optional[str] = None
    metadata: Optional[FileMetadata] = None

class SemanticChangeDetector:
    """Detects changes relevant to semantic search database"""
    
    def __init__(self, workspace_root: str = "/workspaces/code"):
        self.workspace_root = Path(workspace_root)
        self.metadata_file = self.workspace_root / "docs" / "semantic_change_tracking.json"
        self.verbose = False
        
        # Patterns for files relevant to semantic search
        self.relevant_patterns = {
            # Source code
            "*.c", "*.h", "*.cpp", "*.hpp", "*.cc", "*.cxx",
            # Documentation
            "*.md", "*.rst", "*.txt", "*.adoc",
            # Configuration 
            "*.json", "*.yaml", "*.yml", "*.toml", "*.ini",
            # Build files
            "*.cmake", "CMakeLists.txt", "Makefile", "*.mk",
            # Scripts
            "*.py", "*.sh", "*.bash", "*.ps1",
            # Headers and interfaces
            "*.ioc", "*.xml"
        }
        
        # Directories to include in semantic search
        self.relevant_directories = {
            "src", "tests", "docs", "scripts", "Core", "drivers",
            "examples", "00_reference", "middleware", "rtos",
            ".github/instructions"
        }
        
        # Directories to exclude
        self.excluded_directories = {
            ".git", "build", "build_host_tests", ".venv", "node_modules",
            "__pycache__", ".pytest_cache", "logs", "archive",
            "docs/semantic_vector_db"  # Exclude the database itself
        }

    def log(self, message: str, level: str = "INFO"):
        """Log message if verbose mode enabled"""
        if self.verbose:
            timestamp = datetime.now().strftime("%H:%M:%S")
            print(f"[{timestamp}] {level}: {message}")

    def calculate_file_hash(self, file_path: Path) -> str:
        """Calculate MD5 hash of file content"""
        try:
            hash_md5 = hashlib.md5()
            with open(file_path, "rb") as f:
                for chunk in iter(lambda: f.read(4096), b""):
                    hash_md5.update(chunk)
            return hash_md5.hexdigest()
        except Exception as e:
            self.log(f"Error calculating hash for {file_path}: {e}", "WARNING")
            return ""

    def get_file_metadata(self, file_path: Path) -> Optional[FileMetadata]:
        """Get metadata for a file"""
        try:
            stat = file_path.stat()
            content_type = self.determine_content_type(file_path)
            
            return FileMetadata(
                path=str(file_path.relative_to(self.workspace_root)),
                size=stat.st_size,
                modified_time=stat.st_mtime,
                hash_md5=self.calculate_file_hash(file_path),
                content_type=content_type
            )
        except Exception as e:
            self.log(f"Error getting metadata for {file_path}: {e}", "WARNING")
            return None

    def determine_content_type(self, file_path: Path) -> str:
        """Determine content type for semantic categorization"""
        suffix = file_path.suffix.lower()
        name = file_path.name.lower()
        
        # Source code
        if suffix in ['.c', '.h', '.cpp', '.hpp', '.cc', '.cxx']:
            return "source_code"
        elif suffix in ['.py', '.sh', '.bash', '.ps1']:
            return "script"
        elif suffix in ['.md', '.rst', '.txt', '.adoc']:
            return "documentation"
        elif suffix in ['.json', '.yaml', '.yml', '.toml', '.ini']:
            return "configuration"
        elif suffix in ['.cmake'] or name == 'cmakelists.txt':
            return "build_system"
        elif suffix in ['.ioc', '.xml']:
            return "interface_definition"
        else:
            return "other"

    def is_relevant_file(self, file_path: Path) -> bool:
        """Check if file is relevant for semantic search"""
        # Check if in excluded directory
        for excluded in self.excluded_directories:
            if excluded in str(file_path):
                return False
        
        # Check if in relevant directory
        for relevant_dir in self.relevant_directories:
            if str(file_path).startswith(relevant_dir):
                break
        else:
            # Not in any relevant directory, check patterns
            for pattern in self.relevant_patterns:
                if file_path.match(pattern):
                    break
            else:
                return False
        
        return True

    def load_previous_metadata(self) -> Dict[str, FileMetadata]:
        """Load previously tracked file metadata"""
        if not self.metadata_file.exists():
            self.log("No previous metadata found - this is the first run")
            return {}
        
        try:
            with open(self.metadata_file, 'r') as f:
                data = json.load(f)
            
            metadata = {}
            for path, meta_dict in data.get('files', {}).items():
                metadata[path] = FileMetadata(**meta_dict)
            
            self.log(f"Loaded metadata for {len(metadata)} previously tracked files")
            return metadata
            
        except Exception as e:
            self.log(f"Error loading previous metadata: {e}", "WARNING")
            return {}

    def save_current_metadata(self, metadata: Dict[str, FileMetadata]):
        """Save current file metadata"""
        try:
            # Ensure directory exists
            self.metadata_file.parent.mkdir(parents=True, exist_ok=True)
            
            # Convert to serializable format
            data = {
                'last_updated': datetime.now().isoformat(),
                'total_files': len(metadata),
                'files': {
                    path: {
                        'path': meta.path,
                        'size': meta.size,
                        'modified_time': meta.modified_time,
                        'hash_md5': meta.hash_md5,
                        'content_type': meta.content_type
                    }
                    for path, meta in metadata.items()
                }
            }
            
            with open(self.metadata_file, 'w') as f:
                json.dump(data, f, indent=2)
            
            self.log(f"Saved metadata for {len(metadata)} files to {self.metadata_file}")
            
        except Exception as e:
            self.log(f"Error saving metadata: {e}", "ERROR")

    def scan_filesystem(self) -> Dict[str, FileMetadata]:
        """Scan filesystem for relevant files"""
        current_metadata = {}
        
        self.log("Starting filesystem scan...")
        
        for root, dirs, files in os.walk(self.workspace_root):
            # Filter out excluded directories
            dirs[:] = [d for d in dirs if d not in self.excluded_directories]
            
            root_path = Path(root)
            
            for file_name in files:
                file_path = root_path / file_name
                
                if self.is_relevant_file(file_path):
                    metadata = self.get_file_metadata(file_path)
                    if metadata:
                        current_metadata[metadata.path] = metadata
        
        self.log(f"Filesystem scan complete - found {len(current_metadata)} relevant files")
        return current_metadata

    def scan_git_changes(self) -> List[FileChange]:
        """Scan git for changes since last commit"""
        changes = []
        
        try:
            # Get staged and unstaged changes
            result = subprocess.run(
                ['git', 'status', '--porcelain'],
                cwd=self.workspace_root,
                capture_output=True,
                text=True,
                check=True
            )
            
            for line in result.stdout.strip().split('\n'):
                if not line.strip():
                    continue
                
                status = line[:2]
                file_path = line[3:].strip()
                old_path = None  # Initialize for all change types
                
                # Parse git status codes
                if status.startswith('A'):
                    change_type = ChangeType.NEW
                elif status.startswith('M'):
                    change_type = ChangeType.MODIFIED
                elif status.startswith('D'):
                    change_type = ChangeType.DELETED
                elif status.startswith('R'):
                    change_type = ChangeType.RENAMED
                    # Handle renamed files (format: "old_name -> new_name")
                    if ' -> ' in file_path:
                        old_path, file_path = file_path.split(' -> ', 1)
                else:
                    continue  # Skip other status codes
                
                full_path = self.workspace_root / file_path
                if self.is_relevant_file(Path(file_path)):
                    metadata = None
                    if change_type != ChangeType.DELETED and full_path.exists():
                        metadata = self.get_file_metadata(full_path)
                    
                    changes.append(FileChange(
                        change_type=change_type,
                        file_path=file_path,
                        old_path=old_path if change_type == ChangeType.RENAMED else None,
                        metadata=metadata
                    ))
            
            self.log(f"Git scan found {len(changes)} relevant changes")
            return changes
            
        except subprocess.CalledProcessError as e:
            self.log(f"Git command failed: {e}", "ERROR")
            return []
        except Exception as e:
            self.log(f"Error scanning git changes: {e}", "ERROR")
            return []

    def detect_filesystem_changes(self) -> List[FileChange]:
        """Detect changes by comparing current filesystem with previous metadata"""
        previous_metadata = self.load_previous_metadata()
        current_metadata = self.scan_filesystem()
        
        changes = []
        
        # Find new and modified files
        for path, current_meta in current_metadata.items():
            if path not in previous_metadata:
                # New file
                changes.append(FileChange(
                    change_type=ChangeType.NEW,
                    file_path=path,
                    metadata=current_meta
                ))
            else:
                previous_meta = previous_metadata[path]
                # Check if file was modified
                if (current_meta.hash_md5 != previous_meta.hash_md5 or
                    current_meta.modified_time != previous_meta.modified_time):
                    changes.append(FileChange(
                        change_type=ChangeType.MODIFIED,
                        file_path=path,
                        metadata=current_meta
                    ))
        
        # Find deleted files
        for path in previous_metadata:
            if path not in current_metadata:
                changes.append(FileChange(
                    change_type=ChangeType.DELETED,
                    file_path=path
                ))
        
        # Save current metadata for next run
        self.save_current_metadata(current_metadata)
        
        self.log(f"Filesystem comparison found {len(changes)} changes")
        return changes

    def format_changes_report(self, changes: List[FileChange]) -> str:
        """Format changes into a readable report"""
        if not changes:
            return "No relevant changes detected."
        
        # Group changes by type
        by_type = {}
        for change in changes:
            if change.change_type not in by_type:
                by_type[change.change_type] = []
            by_type[change.change_type].append(change)
        
        # Build report
        lines = [
            f"Semantic Search Delta Detection Report",
            f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
            f"Total changes: {len(changes)}",
            "",
        ]
        
        for change_type in [ChangeType.NEW, ChangeType.MODIFIED, ChangeType.DELETED, ChangeType.RENAMED]:
            if change_type not in by_type:
                continue
            
            type_changes = by_type[change_type]
            lines.append(f"{change_type} files ({len(type_changes)}):")
            
            for change in type_changes:
                if change.metadata:
                    size_kb = change.metadata.size / 1024
                    lines.append(f"  {change.change_type}: {change.file_path} ({change.metadata.content_type}, {size_kb:.1f}KB)")
                else:
                    lines.append(f"  {change.change_type}: {change.file_path}")
            
            lines.append("")
        
        return "\n".join(lines)

def main():
    parser = argparse.ArgumentParser(
        description="Detect changes for semantic search database delta updates"
    )
    parser.add_argument(
        "--mode", 
        choices=["git", "filesystem"], 
        default="git",
        help="Detection mode: git (staged/unstaged changes) or filesystem (full comparison)"
    )
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Enable verbose logging"
    )
    parser.add_argument(
        "--output", "-o",
        help="Output file for changes report (default: stdout)"
    )
    parser.add_argument(
        "--format",
        choices=["text", "json"],
        default="text",
        help="Output format"
    )
    
    args = parser.parse_args()
    
    detector = SemanticChangeDetector()
    detector.verbose = args.verbose
    
    # Detect changes
    if args.mode == "git":
        changes = detector.scan_git_changes()
    else:
        changes = detector.detect_filesystem_changes()
    
    # Format output
    if args.format == "json":
        output = json.dumps([{
            'change_type': change.change_type,
            'file_path': change.file_path,
            'old_path': change.old_path,
            'metadata': {
                'content_type': change.metadata.content_type,
                'size': change.metadata.size
            } if change.metadata else None
        } for change in changes], indent=2)
    else:
        output = detector.format_changes_report(changes)
    
    # Output results
    if args.output:
        with open(args.output, 'w') as f:
            f.write(output)
        print(f"Changes report written to {args.output}")
    else:
        print(output)
    
    # Exit with appropriate code
    if changes:
        print(f"\n✅ Detected {len(changes)} changes that may require semantic database updates")
        sys.exit(0)
    else:
        print("\n✅ No changes detected - semantic database is up to date")
        sys.exit(0)

if __name__ == "__main__":
    main()
