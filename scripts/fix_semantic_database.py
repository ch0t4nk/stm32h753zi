#!/usr/bin/env python3
"""
Fix Semantic Database Script - FTR-014 Phase 2 Implementation
Refreshes semantic vector database to current codebase state and implements git-friendly storage

This script addresses critical issues:
1. Database staleness (last updated Aug 7, missing 3 days of progress)
2. Git repository bloat (917.71MB database file)
3. Manual maintenance overhead

Usage:
    python scripts/fix_semantic_database.py --action refresh
    python scripts/fix_semantic_database.py --action optimize-storage
    python scripts/fix_semantic_database.py --action full-rebuild
"""

import os
import sys
import json
import shutil
import hashlib
import subprocess
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Optional, Tuple
import argparse
import logging

# Add project root to path for imports
sys.path.append(str(Path(__file__).parent.parent))

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='[%(asctime)s] %(levelname)s: %(message)s',
    datefmt='%H:%M:%S'
)
logger = logging.getLogger(__name__)

class SemanticDatabaseFixer:
    """Fixes semantic database issues for FTR-014 implementation"""
    
    def __init__(self):
        self.project_root = Path(__file__).parent.parent
        self.db_path = self.project_root / "docs" / "semantic_vector_db"
        self.backup_path = self.project_root / "docs" / "semantic_backups"
        self.venv_python = self.project_root / ".venv" / "bin" / "python"
        
        # Ensure paths exist
        self.backup_path.mkdir(exist_ok=True)
        
    def check_database_status(self) -> Dict:
        """Check current database status and health"""
        logger.info("🔍 Checking semantic database status...")
        
        status = {
            'exists': False,
            'size_mb': 0,
            'file_count': 0,
            'last_modified': None,
            'health': 'unknown'
        }
        
        if not self.db_path.exists():
            logger.warning("❌ Semantic database directory not found")
            return status
            
        status['exists'] = True
        
        # Calculate total size
        total_size = 0
        file_count = 0
        latest_mtime = 0
        
        for file_path in self.db_path.rglob('*'):
            if file_path.is_file():
                file_size = file_path.stat().st_size
                total_size += file_size
                file_count += 1
                latest_mtime = max(latest_mtime, file_path.stat().st_mtime)
        
        status['size_mb'] = total_size / 1024 / 1024
        status['file_count'] = file_count
        status['last_modified'] = datetime.fromtimestamp(latest_mtime)
        
        # Check main database file
        main_db = self.db_path / "chroma.sqlite3"
        if main_db.exists():
            main_size = main_db.stat().st_size / 1024 / 1024
            if main_size > 100:
                status['health'] = 'oversized'
                logger.warning(f"⚠️  Main database file is {main_size:.1f}MB (exceeds 100MB GitHub limit)")
            else:
                status['health'] = 'healthy'
        else:
            status['health'] = 'missing_main_db'
            
        logger.info(f"📊 Database status: {status['size_mb']:.1f}MB, {file_count} files")
        logger.info(f"📅 Last modified: {status['last_modified']}")
        
        return status
    
    def create_backup(self) -> Path:
        """Create timestamped backup of current database"""
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        backup_dir = self.backup_path / f"semantic_db_backup_{timestamp}"
        
        logger.info(f"💾 Creating backup: {backup_dir}")
        
        if self.db_path.exists():
            shutil.copytree(self.db_path, backup_dir)
            logger.info(f"✅ Backup created successfully")
        else:
            logger.warning("⚠️  No existing database to backup")
            
        return backup_dir
    
    def check_environment(self) -> bool:
        """Check if environment is ready for database operations"""
        logger.info("🔍 Checking environment readiness...")
        
        # Check virtual environment
        if not self.venv_python.exists():
            logger.error("❌ Virtual environment not found")
            return False
        
        # Check required packages
        try:
            result = subprocess.run([
                str(self.venv_python), "-c",
                "import chromadb, requests, instructor"
            ], capture_output=True, text=True, check=True)
            logger.info("✅ Required packages available")
        except subprocess.CalledProcessError:
            logger.error("❌ Required packages missing (chromadb, requests, instructor)")
            return False
        
        # Check semantic search script
        search_script = self.project_root / "scripts" / "stm32_semantic_search.py"
        if not search_script.exists():
            logger.error("❌ Semantic search script not found")
            return False
            
        logger.info("✅ Environment ready for database operations")
        return True
    
    def rebuild_database(self) -> bool:
        """Rebuild semantic database with current content"""
        logger.info("🔄 Rebuilding semantic database with current content...")
        
        # Create backup first
        backup_dir = self.create_backup()
        
        try:
            # Remove existing database
            if self.db_path.exists():
                logger.info("🗑️  Removing existing database...")
                shutil.rmtree(self.db_path)
            
            # Rebuild database using semantic search script
            logger.info("🏗️  Rebuilding database (this may take several minutes)...")
            result = subprocess.run([
                str(self.venv_python),
                str(self.project_root / "scripts" / "stm32_semantic_search.py"),
                "concept", "test", "--rebuild"
            ], capture_output=True, text=True, check=True)
            
            logger.info("✅ Database rebuild completed successfully")
            
            # Verify new database
            status = self.check_database_status()
            if status['exists'] and status['health'] in ['healthy', 'oversized']:
                logger.info(f"✅ New database verified: {status['size_mb']:.1f}MB, {status['file_count']} files")
                return True
            else:
                logger.error("❌ Database rebuild verification failed")
                return False
                
        except subprocess.CalledProcessError as e:
            logger.error(f"❌ Database rebuild failed: {e}")
            logger.error(f"Output: {e.stdout}")
            logger.error(f"Error: {e.stderr}")
            
            # Restore backup
            logger.info("🔄 Restoring backup...")
            if self.db_path.exists():
                shutil.rmtree(self.db_path)
            shutil.copytree(backup_dir, self.db_path)
            
            return False
    
    def optimize_storage(self) -> bool:
        """Optimize database storage for git-friendly format"""
        logger.info("🎯 Optimizing database storage for git compatibility...")
        
        status = self.check_database_status()
        if not status['exists']:
            logger.error("❌ No database found to optimize")
            return False
        
        main_db = self.db_path / "chroma.sqlite3"
        if not main_db.exists():
            logger.warning("⚠️  Main database file not found, skipping optimization")
            return True
        
        main_size_mb = main_db.stat().st_size / 1024 / 1024
        
        if main_size_mb <= 100:
            logger.info(f"✅ Database size ({main_size_mb:.1f}MB) is already git-friendly")
            return True
        
        logger.warning(f"⚠️  Database size ({main_size_mb:.1f}MB) exceeds GitHub limits")
        logger.info("💡 Recommended actions:")
        logger.info("   1. Add docs/semantic_vector_db/ to .gitignore")
        logger.info("   2. Use external storage for production database")
        logger.info("   3. Implement delta update system (FTR-014 Phase 3)")
        
        # For now, ensure database is excluded from git
        gitignore_path = self.project_root / ".gitignore"
        gitignore_content = ""
        
        if gitignore_path.exists():
            gitignore_content = gitignore_path.read_text()
        
        exclusions_needed = [
            "# Exclude large semantic vector database files",
            "docs/semantic_vector_db/",
            "*.sqlite3"
        ]
        
        updated = False
        for exclusion in exclusions_needed:
            if exclusion not in gitignore_content:
                gitignore_content += f"\n{exclusion}"
                updated = True
        
        if updated:
            gitignore_path.write_text(gitignore_content)
            logger.info("✅ Updated .gitignore to exclude semantic database")
        
        return True
    
    def validate_search_functionality(self) -> bool:
        """Validate that semantic search is working with current database"""
        logger.info("🧪 Validating semantic search functionality...")
        
        test_queries = [
            ("concept", "GPIO configuration", "STM32H7"),
            ("function", "emergency_stop", "all"),
            ("concept", "optimization telemetry", "all")
        ]
        
        for query_type, query, scope in test_queries:
            try:
                logger.info(f"🔍 Testing: {query_type} '{query}' --scope {scope}")
                result = subprocess.run([
                    str(self.venv_python),
                    str(self.project_root / "scripts" / "stm32_semantic_search.py"),
                    query_type, query, "--scope", scope
                ], capture_output=True, text=True, check=True, timeout=30)
                
                if "No results found" in result.stdout:
                    logger.warning(f"⚠️  No results for query: {query}")
                else:
                    logger.info(f"✅ Query successful: {query}")
                    
            except subprocess.CalledProcessError as e:
                logger.error(f"❌ Query failed: {query} - {e}")
                return False
            except subprocess.TimeoutExpired:
                logger.error(f"❌ Query timeout: {query}")
                return False
        
        logger.info("✅ Semantic search validation completed")
        return True
    
    def generate_report(self) -> Dict:
        """Generate comprehensive status report"""
        logger.info("📋 Generating semantic database status report...")
        
        status = self.check_database_status()
        
        report = {
            'timestamp': datetime.now().isoformat(),
            'database_status': status,
            'environment_ready': self.check_environment(),
            'git_friendly': status['size_mb'] <= 100 if status['exists'] else True,
            'recommendations': []
        }
        
        # Generate recommendations
        if not status['exists']:
            report['recommendations'].append("Database not found - run rebuild operation")
        elif status['health'] == 'oversized':
            report['recommendations'].append("Database exceeds GitHub limits - implement storage optimization")
        
        if status['exists'] and status['last_modified']:
            days_old = (datetime.now() - status['last_modified']).days
            if days_old > 1:
                report['recommendations'].append(f"Database is {days_old} days old - consider refresh")
        
        if not report['environment_ready']:
            report['recommendations'].append("Environment issues detected - check dependencies")
        
        return report

def main():
    parser = argparse.ArgumentParser(description="Fix semantic database issues for FTR-014")
    parser.add_argument('--action', choices=['status', 'refresh', 'optimize', 'rebuild', 'validate'], 
                       default='status', help='Action to perform')
    parser.add_argument('--force', action='store_true', help='Force operation without prompts')
    parser.add_argument('--backup', action='store_true', help='Create backup before operations')
    
    args = parser.parse_args()
    
    fixer = SemanticDatabaseFixer()
    
    logger.info("🚀 Semantic Database Fixer - FTR-014 Implementation")
    logger.info("=" * 60)
    
    if args.action == 'status':
        report = fixer.generate_report()
        print(json.dumps(report, indent=2, default=str))
        
        if report['recommendations']:
            logger.info("💡 Recommendations:")
            for rec in report['recommendations']:
                logger.info(f"   • {rec}")
    
    elif args.action == 'refresh':
        logger.info("🔄 Refreshing semantic database with current content...")
        
        if not fixer.check_environment():
            logger.error("❌ Environment not ready")
            sys.exit(1)
        
        if args.backup or not args.force:
            fixer.create_backup()
        
        if fixer.rebuild_database():
            logger.info("✅ Database refresh completed successfully")
            fixer.validate_search_functionality()
        else:
            logger.error("❌ Database refresh failed")
            sys.exit(1)
    
    elif args.action == 'optimize':
        logger.info("🎯 Optimizing database storage...")
        
        if fixer.optimize_storage():
            logger.info("✅ Storage optimization completed")
        else:
            logger.error("❌ Storage optimization failed")
            sys.exit(1)
    
    elif args.action == 'rebuild':
        logger.info("🏗️  Full database rebuild...")
        
        if not fixer.check_environment():
            logger.error("❌ Environment not ready")
            sys.exit(1)
        
        if not args.force:
            confirm = input("⚠️  This will completely rebuild the database. Continue? (y/N): ")
            if confirm.lower() != 'y':
                logger.info("Operation cancelled")
                sys.exit(0)
        
        fixer.create_backup()
        
        if fixer.rebuild_database():
            logger.info("✅ Database rebuild completed successfully")
            fixer.optimize_storage()
            fixer.validate_search_functionality()
        else:
            logger.error("❌ Database rebuild failed")
            sys.exit(1)
    
    elif args.action == 'validate':
        logger.info("🧪 Validating semantic search functionality...")
        
        if fixer.validate_search_functionality():
            logger.info("✅ Validation successful")
        else:
            logger.error("❌ Validation failed")
            sys.exit(1)
    
    logger.info("🎉 Operation completed successfully!")

if __name__ == "__main__":
    main()
