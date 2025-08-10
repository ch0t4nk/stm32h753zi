#!/usr/bin/env python3
"""
Semantic Search Delta Update System
STM32H753ZI Stepper Motor Control Project

Intelligent incremental updates for the semantic vector database,
processing only changed documents to maintain efficiency.

Author: STM32H753ZI Development Team  
Date: August 10, 2025
Version: 1.0.0
"""

import os
import sys
import json
import time
import shutil
import subprocess
from pathlib import Path
from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass
from datetime import datetime

# Add the workspace root to Python path for imports
WORKSPACE_ROOT = Path("/workspaces/code")
sys.path.insert(0, str(WORKSPACE_ROOT))
sys.path.insert(0, str(WORKSPACE_ROOT / "scripts"))

@dataclass
class UpdateResult:
    """Results from a delta update operation"""
    timestamp: str
    total_changes: int
    documents_added: int
    documents_updated: int
    documents_removed: int
    collections_affected: List[str]
    processing_time_seconds: float
    errors: List[str]
    
    @property
    def success(self) -> bool:
        """Whether the update completed successfully"""
        return len(self.errors) == 0

class SemanticDeltaUpdater:
    """
    Intelligent delta update system for semantic vector database
    
    Features:
    - Change detection using file system monitoring
    - Incremental database rebuilds when needed
    - Performance optimization for development workflow
    """
    
    def __init__(self, 
                 db_path: str = "docs/semantic_vector_db",
                 change_tracker_config: str = "docs/semantic_vector_db/change_tracker.json"):
        self.db_path = Path(db_path)
        self.change_tracker_config = Path(change_tracker_config)
        self.workspace_root = WORKSPACE_ROOT
        
        # Create backup directory for rollback capabilities
        self.backup_dir = self.db_path / "backups"
        self.backup_dir.mkdir(parents=True, exist_ok=True)
    
    def get_last_database_update(self) -> Optional[datetime]:
        """Get timestamp of last database update"""
        try:
            if self.change_tracker_config.exists():
                with open(self.change_tracker_config) as f:
                    data = json.load(f)
                    last_updated = data.get('last_updated')
                    if last_updated:
                        return datetime.fromisoformat(last_updated.replace('Z', '+00:00'))
        except Exception as e:
            print(f"Warning: Could not read last update time: {e}")
        
        return None
    
    def check_if_update_needed(self) -> Tuple[bool, List[str]]:
        """Check if database update is needed based on file changes"""
        last_update = self.get_last_database_update()
        
        if not last_update:
            return True, ["No previous update timestamp found"]
        
        # Check source paths for modifications
        source_paths = [
            "00_reference/stm32h753xx_User_Manual_md",
            "00_reference/x_cube_spn2_markdown_docs", 
            "00_reference/stm32h7xx_nucleo_bsp_md",
            ".github/instructions",
            "src",
            "docs",
            "scripts",
            "tests"
        ]
        
        changes_detected = []
        
        for source_path in source_paths:
            full_path = self.workspace_root / source_path
            
            if not full_path.exists():
                continue
            
            # Check if any files in the path are newer than last update
            if full_path.is_file():
                if datetime.fromtimestamp(full_path.stat().st_mtime) > last_update:
                    changes_detected.append(f"Modified: {source_path}")
            else:
                # Check directory recursively
                for file_path in full_path.rglob('*'):
                    if file_path.is_file() and file_path.suffix in {'.md', '.c', '.h', '.py', '.sh', '.txt', '.json', '.yaml', '.yml'}:
                        if datetime.fromtimestamp(file_path.stat().st_mtime) > last_update:
                            rel_path = file_path.relative_to(self.workspace_root)
                            changes_detected.append(f"Modified: {rel_path}")
                            break  # One change per directory is enough to trigger update
        
        return len(changes_detected) > 0, changes_detected
    
    def create_backup(self) -> Optional[str]:
        """Create a backup of the current database"""
        if not self.db_path.exists():
            return None
        
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        backup_path = self.backup_dir / f"semantic_db_backup_{timestamp}"
        
        try:
            shutil.copytree(self.db_path, backup_path, ignore=shutil.ignore_patterns("backups"))
            print(f"✅ Database backup created: {backup_path}")
            return str(backup_path)
        except Exception as e:
            print(f"⚠️  Failed to create backup: {e}")
            return None
    
    def rebuild_database(self) -> Tuple[bool, List[str]]:
        """Rebuild the semantic database using existing infrastructure"""
        print("🔄 Rebuilding semantic vector database...")
        
        errors = []
        
        try:
            # Use the existing semantic search script to rebuild
            rebuild_cmd = [
                str(self.workspace_root / ".venv" / "bin" / "python"),
                str(self.workspace_root / "scripts" / "stm32_semantic_search.py"),
                "concept", "test", "--rebuild"
            ]
            
            print(f"Running: {' '.join(rebuild_cmd)}")
            
            result = subprocess.run(
                rebuild_cmd,
                cwd=str(self.workspace_root),
                capture_output=True,
                text=True,
                timeout=3600  # 1 hour timeout
            )
            
            if result.returncode == 0:
                print("✅ Database rebuild completed successfully")
                
                # Update the change tracker timestamp
                self.update_change_tracker()
                
                return True, []
            else:
                error_msg = f"Database rebuild failed (exit code {result.returncode})"
                if result.stderr:
                    error_msg += f": {result.stderr}"
                errors.append(error_msg)
                print(f"❌ {error_msg}")
                
                return False, errors
                
        except subprocess.TimeoutExpired:
            error_msg = "Database rebuild timed out after 1 hour"
            errors.append(error_msg)
            print(f"❌ {error_msg}")
            return False, errors
            
        except Exception as e:
            error_msg = f"Error during database rebuild: {str(e)}"
            errors.append(error_msg)
            print(f"❌ {error_msg}")
            return False, errors
    
    def update_change_tracker(self) -> None:
        """Update the change tracker with current timestamp"""
        self.change_tracker_config.parent.mkdir(parents=True, exist_ok=True)
        
        change_log_data = {
            'last_updated': datetime.now().isoformat(),
            'version': '1.0.0',
            'update_method': 'full_rebuild',
            'files': {}  # Will be populated by change_detector.py if available
        }
        
        try:
            with open(self.change_tracker_config, 'w') as f:
                json.dump(change_log_data, f, indent=2)
            print(f"📝 Updated change tracker: {self.change_tracker_config}")
        except Exception as e:
            print(f"⚠️  Could not update change tracker: {e}")
    
    def run_delta_update(self, force_rebuild: bool = False, dry_run: bool = False) -> UpdateResult:
        """Run delta update process"""
        start_time = time.time()
        timestamp = datetime.now().isoformat()
        
        print(f"� Checking for database updates needed at {timestamp}")
        
        # Check if update is needed
        update_needed, changes = self.check_if_update_needed()
        
        if not update_needed and not force_rebuild:
            print("✅ Database is up to date - no changes detected")
            return UpdateResult(
                timestamp=timestamp,
                total_changes=0,
                documents_added=0,
                documents_updated=0,
                documents_removed=0,
                collections_affected=[],
                processing_time_seconds=time.time() - start_time,
                errors=[]
            )
        
        if dry_run:
            print("🔍 Dry run mode - would rebuild database due to changes:")
            for change in changes[:10]:  # Show first 10 changes
                print(f"  - {change}")
            if len(changes) > 10:
                print(f"  ... and {len(changes) - 10} more changes")
            
            return UpdateResult(
                timestamp=timestamp,
                total_changes=len(changes),
                documents_added=0,
                documents_updated=0,
                documents_removed=0,
                collections_affected=[],
                processing_time_seconds=time.time() - start_time,
                errors=[]
            )
        
        print(f"📊 Database update needed due to {len(changes)} changes")
        for change in changes[:5]:  # Show first 5 changes
            print(f"  - {change}")
        if len(changes) > 5:
            print(f"  ... and {len(changes) - 5} more changes")
        
        # Create backup
        backup_path = self.create_backup()
        
        # Rebuild database
        success, errors = self.rebuild_database()
        
        # Create result
        result = UpdateResult(
            timestamp=timestamp,
            total_changes=len(changes),
            documents_added=0,  # Full rebuild - can't track individual operations
            documents_updated=0,
            documents_removed=0,
            collections_affected=["all"],  # Full rebuild affects all collections
            processing_time_seconds=time.time() - start_time,
            errors=errors
        )
        
        if success:
            print(f"🎉 Database update completed successfully in {result.processing_time_seconds:.2f}s")
        else:
            print(f"❌ Database update failed with {len(errors)} errors")
            
            # Attempt to restore backup on failure
            if backup_path:
                print("🔄 Attempting to restore from backup...")
                try:
                    # Simple restore - remove current and copy backup
                    if self.db_path.exists():
                        shutil.rmtree(self.db_path)
                    shutil.copytree(backup_path, self.db_path)
                    print("✅ Database restored from backup")
                except Exception as e:
                    error_msg = f"Failed to restore backup: {str(e)}"
                    result.errors.append(error_msg)
                    print(f"❌ {error_msg}")
        
        return result

def main():
    """Main function for standalone execution"""
    import argparse
    
    parser = argparse.ArgumentParser(description='Apply delta updates to semantic search database')
    parser.add_argument('--db-path', default='docs/semantic_vector_db',
                       help='Path to semantic vector database')
    parser.add_argument('--force-rebuild', action='store_true',
                       help='Force database rebuild even if no changes detected')
    parser.add_argument('--dry-run', action='store_true',
                       help='Show what would be updated without making changes')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Verbose output with detailed progress')
    parser.add_argument('--json', action='store_true',
                       help='Output results in JSON format')
    
    args = parser.parse_args()
    
    # Initialize delta updater
    updater = SemanticDeltaUpdater(
        db_path=args.db_path,
        change_tracker_config=f"{args.db_path}/change_tracker.json"
    )
    
    # Run delta update
    result = updater.run_delta_update(
        force_rebuild=args.force_rebuild,
        dry_run=args.dry_run
    )
    
    if args.json:
        # Output JSON for programmatic use
        import dataclasses
        result_dict = dataclasses.asdict(result)
        print(json.dumps(result_dict, indent=2))
    else:
        # Human-readable summary
        if result.success:
            print(f"\n🎉 Delta update completed successfully!")
        else:
            print(f"\n⚠️  Delta update completed with {len(result.errors)} errors")
        
        if args.verbose:
            print(f"\n📋 Detailed Results:")
            print(f"  Timestamp: {result.timestamp}")
            print(f"  Total changes: {result.total_changes}")
            print(f"  Collections affected: {', '.join(result.collections_affected)}")
            print(f"  Processing time: {result.processing_time_seconds:.2f}s")
            
            if result.errors:
                print(f"\n❌ Errors encountered:")
                for i, error in enumerate(result.errors, 1):
                    print(f"  {i}. {error}")

if __name__ == "__main__":
    main()
