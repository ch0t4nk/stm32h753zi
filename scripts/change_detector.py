#!/usr/bin/env python3
"""
Change Detection Engine for Semantic Search Database
STM32H753ZI Stepper Motor Control Project

This module detects changes in documentation and source files to enable
intelligent delta updates for the semantic vector database.

Author: STM32H753ZI Development Team
Date: August 10, 2025
Version: 1.0.0
"""

import os
import json
import hashlib
import time
from pathlib import Path
from typing import Dict, List, Set, Optional
from dataclasses import dataclass, asdict
from datetime import datetime

@dataclass
class FileMetadata:
    """Metadata for tracked files"""
    path: str
    size: int
    mtime: float
    hash: str
    collection: str

@dataclass
class ChangeSet:
    """Represents a set of changes detected in the filesystem"""
    new_files: List[str]
    modified_files: List[str] 
    deleted_files: List[str]
    renamed_files: List[tuple]  # (old_path, new_path)
    unchanged_files: List[str]
    
    def __post_init__(self):
        """Validate changeset consistency"""
        all_files = set(self.new_files + self.modified_files + self.deleted_files + self.unchanged_files)
        for old, new in self.renamed_files:
            all_files.add(old)
            all_files.add(new)
        
    @property
    def has_changes(self) -> bool:
        """Check if any changes were detected"""
        return bool(self.new_files or self.modified_files or self.deleted_files or self.renamed_files)
    
    @property
    def total_changes(self) -> int:
        """Total number of changes"""
        return len(self.new_files) + len(self.modified_files) + len(self.deleted_files) + len(self.renamed_files)

class DocumentChangeDetector:
    """
    Detect changes in documentation and source files for semantic database updates
    
    Tracks file metadata and detects:
    - New files added
    - Existing files modified 
    - Files deleted
    - Files moved/renamed
    """
    
    def __init__(self, config_path: str = "docs/semantic_vector_db/change_tracker.json"):
        self.config_path = Path(config_path)
        self.workspace_root = Path("/workspaces/code")
        
        # Source paths to monitor for changes
        self.source_paths = [
            "00_reference/stm32h753xx_User_Manual_md",
            "00_reference/x_cube_spn2_markdown_docs", 
            "00_reference/stm32h7xx_nucleo_bsp_md",
            ".github/instructions",
            "src",
            "docs",
            "scripts",
            "tests",
            "cmake",
            "Core"
        ]
        
        # File extensions to track
        self.tracked_extensions = {
            '.md', '.c', '.h', '.cpp', '.hpp', '.py', '.sh', '.cmake', 
            '.txt', '.json', '.yaml', '.yml', '.rst', '.ini'
        }
        
        # Collection mapping for different source paths
        self.collection_mapping = {
            "00_reference/stm32h753xx_User_Manual_md": "stm32_hal",
            "00_reference/x_cube_spn2_markdown_docs": "motor_control", 
            "00_reference/stm32h7xx_nucleo_bsp_md": "nucleo_bsp",
            ".github/instructions": "instructions",
            "src": "project_source",
            "docs": "documentation",
            "scripts": "scripts",
            "tests": "validation",
            "cmake": "build_system",
            "Core": "project_source"
        }
        
        self.change_log = self.load_change_log()
    
    def load_change_log(self) -> Dict[str, FileMetadata]:
        """Load previous file metadata from change log"""
        if not self.config_path.exists():
            return {}
        
        try:
            with open(self.config_path, 'r') as f:
                data = json.load(f)
                return {
                    path: FileMetadata(**metadata) 
                    for path, metadata in data.get('files', {}).items()
                }
        except (json.JSONDecodeError, KeyError, TypeError) as e:
            print(f"Warning: Could not load change log from {self.config_path}: {e}")
            return {}
    
    def save_change_log(self, file_metadata: Dict[str, FileMetadata]) -> None:
        """Save current file metadata to change log"""
        self.config_path.parent.mkdir(parents=True, exist_ok=True)
        
        change_log_data = {
            'last_updated': datetime.now().isoformat(),
            'version': '1.0.0',
            'files': {
                path: asdict(metadata)
                for path, metadata in file_metadata.items()
            }
        }
        
        with open(self.config_path, 'w') as f:
            json.dump(change_log_data, f, indent=2)
    
    def calculate_file_hash(self, file_path: Path) -> str:
        """Calculate SHA256 hash of file content"""
        try:
            with open(file_path, 'rb') as f:
                return hashlib.sha256(f.read()).hexdigest()
        except (OSError, IOError) as e:
            print(f"Warning: Could not hash file {file_path}: {e}")
            return ""
    
    def get_collection_for_path(self, file_path: str) -> str:
        """Determine which collection a file belongs to"""
        for prefix, collection in self.collection_mapping.items():
            if file_path.startswith(prefix):
                return collection
        return "project_source"  # Default collection
    
    def scan_files(self) -> Dict[str, FileMetadata]:
        """Scan all tracked files and build current metadata"""
        current_files = {}
        
        for source_path in self.source_paths:
            full_path = self.workspace_root / source_path
            
            if not full_path.exists():
                continue
                
            if full_path.is_file():
                # Single file
                if full_path.suffix in self.tracked_extensions:
                    rel_path = str(full_path.relative_to(self.workspace_root))
                    current_files[rel_path] = self.get_file_metadata(full_path, rel_path)
            else:
                # Directory - scan recursively
                for file_path in full_path.rglob('*'):
                    if file_path.is_file() and file_path.suffix in self.tracked_extensions:
                        rel_path = str(file_path.relative_to(self.workspace_root))
                        current_files[rel_path] = self.get_file_metadata(file_path, rel_path)
        
        return current_files
    
    def get_file_metadata(self, file_path: Path, rel_path: str) -> FileMetadata:
        """Get metadata for a single file"""
        stat = file_path.stat()
        file_hash = self.calculate_file_hash(file_path)
        collection = self.get_collection_for_path(rel_path)
        
        return FileMetadata(
            path=rel_path,
            size=stat.st_size,
            mtime=stat.st_mtime,
            hash=file_hash,
            collection=collection
        )
    
    def detect_changes(self) -> ChangeSet:
        """Detect all changes since last scan"""
        print("Scanning for file changes...")
        current_files = self.scan_files()
        previous_files = self.change_log
        
        current_paths = set(current_files.keys())
        previous_paths = set(previous_files.keys())
        
        # Detect different types of changes
        new_files = list(current_paths - previous_paths)
        deleted_files = list(previous_paths - current_paths)
        potential_modified = list(current_paths & previous_paths)
        
        # Check for actual modifications (content changes)
        modified_files = []
        unchanged_files = []
        
        for path in potential_modified:
            current_meta = current_files[path]
            previous_meta = previous_files[path]
            
            # Compare hash for definitive change detection
            if current_meta.hash != previous_meta.hash:
                modified_files.append(path)
            else:
                unchanged_files.append(path)
        
        # TODO: Implement rename detection using file content similarity
        renamed_files = []
        
        changes = ChangeSet(
            new_files=new_files,
            modified_files=modified_files,
            deleted_files=deleted_files,
            renamed_files=renamed_files,
            unchanged_files=unchanged_files
        )
        
        # Save current state as new baseline
        self.save_change_log(current_files)
        
        return changes
    
    def get_changes_by_collection(self, changes: ChangeSet) -> Dict[str, ChangeSet]:
        """Group changes by database collection"""
        collection_changes = {}
        current_files = self.scan_files()
        
        # Group by collection
        for change_type in ['new_files', 'modified_files', 'deleted_files']:
            files = getattr(changes, change_type)
            for file_path in files:
                if file_path in current_files:
                    collection = current_files[file_path].collection
                else:
                    # File was deleted, try to determine collection from path
                    collection = self.get_collection_for_path(file_path)
                
                if collection not in collection_changes:
                    collection_changes[collection] = ChangeSet([], [], [], [], [])
                
                getattr(collection_changes[collection], change_type).append(file_path)
        
        return collection_changes
    
    def print_change_summary(self, changes: ChangeSet) -> None:
        """Print a human-readable summary of detected changes"""
        if not changes.has_changes:
            print("✅ No changes detected - database is up to date")
            return
        
        print(f"\n📊 Change Detection Summary ({changes.total_changes} changes):")
        print(f"  📄 New files: {len(changes.new_files)}")
        print(f"  📝 Modified files: {len(changes.modified_files)}")
        print(f"  🗑️  Deleted files: {len(changes.deleted_files)}")
        print(f"  📁 Renamed files: {len(changes.renamed_files)}")
        print(f"  ✅ Unchanged files: {len(changes.unchanged_files)}")
        
        # Show collection breakdown
        collection_changes = self.get_changes_by_collection(changes)
        if collection_changes:
            print(f"\n📚 Changes by Collection:")
            for collection, coll_changes in collection_changes.items():
                if coll_changes.has_changes:
                    print(f"  {collection}: {coll_changes.total_changes} changes")
        
        # Show some examples if not too many files
        if len(changes.new_files) <= 5:
            for file_path in changes.new_files:
                print(f"    ➕ {file_path}")
        
        if len(changes.modified_files) <= 5:
            for file_path in changes.modified_files:
                print(f"    📝 {file_path}")
        
        if len(changes.deleted_files) <= 3:
            for file_path in changes.deleted_files:
                print(f"    ❌ {file_path}")

def main():
    """Main function for standalone execution"""
    import argparse
    
    parser = argparse.ArgumentParser(description='Detect changes for semantic database updates')
    parser.add_argument('--config', default='docs/semantic_vector_db/change_tracker.json',
                       help='Path to change tracker configuration file')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Verbose output with detailed change information')
    parser.add_argument('--json', action='store_true', 
                       help='Output changes in JSON format')
    
    args = parser.parse_args()
    
    # Initialize change detector
    detector = DocumentChangeDetector(args.config)
    
    # Detect changes
    changes = detector.detect_changes()
    
    if args.json:
        # Output JSON for programmatic use
        change_data = {
            'has_changes': changes.has_changes,
            'total_changes': changes.total_changes,
            'new_files': changes.new_files,
            'modified_files': changes.modified_files,
            'deleted_files': changes.deleted_files,
            'renamed_files': changes.renamed_files,
            'collection_changes': detector.get_changes_by_collection(changes)
        }
        print(json.dumps(change_data, indent=2))
    else:
        # Human-readable output
        detector.print_change_summary(changes)
        
        if args.verbose and changes.has_changes:
            print(f"\n📋 Detailed Change List:")
            for file_path in changes.new_files:
                print(f"  NEW: {file_path}")
            for file_path in changes.modified_files:
                print(f"  MOD: {file_path}")
            for file_path in changes.deleted_files:
                print(f"  DEL: {file_path}")

if __name__ == "__main__":
    main()
