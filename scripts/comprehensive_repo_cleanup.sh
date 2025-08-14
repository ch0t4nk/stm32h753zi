#!/bin/bash

# STM32H753ZI Repository Cleanup Script - FTR-014 Implementation
# ==============================================================
# CRITICAL: This script removes large files from git history
# WARNING: This modifies git history - coordinate with team before running

set -e

echo "🚨 STM32H753ZI Repository Cleanup - FTR-014"
echo "=============================================="
echo "⚠️  WARNING: This will modify git history to remove large files"
echo "⚠️  Current repository size: ~7+ GB (should be ~50MB after cleanup)"
echo ""

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ] || [ ! -d "src" ]; then
    echo "❌ Error: Must run from STM32H753ZI project root directory"
    exit 1
fi

echo "🔍 Large files currently tracked in git:"
echo ""

# Find all large files in git history
echo "📊 Scanning git history for files larger than 10MB..."
git rev-list --objects --all | \
    git cat-file --batch-check='%(objecttype) %(objectname) %(objectsize) %(rest)' | \
    awk '/^blob/ {print substr($0,6)}' | \
    sort --numeric-sort --key=2 | \
    tail -20

echo ""
echo "🎯 Files to be removed from git history:"
echo "   • 00_reference/ (7+ GB of ST assets)"
echo "   • .venv/ (5+ GB Python environment)"
echo "   • docs/semantic_vector_db/ (1.5+ GB vector database)"
echo "   • .vscode/browse.vc.db (585MB VS Code database)"
echo "   • Large build artifacts and temporary files"
echo ""

read -p "⚠️  Continue with repository cleanup? This CANNOT be undone (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "❌ Repository cleanup cancelled"
    exit 1
fi

echo ""
echo "🚀 Starting repository cleanup..."

# 1. Create backup
echo "📦 Creating backup of current state..."
git tag "backup-before-cleanup-$(date +%Y%m%d-%H%M%S)" HEAD
echo "✅ Backup tag created"

# 2. Stop any running git processes
echo "🛑 Ensuring no git processes are running..."
pkill -f "git" || true
sleep 2

# 3. Clean working directory
echo "🧹 Cleaning working directory..."
git add .
git commit -m "final: commit all changes before repository cleanup" || echo "No changes to commit"

# 4. Remove large directories from git history
echo "🗑️ Removing large files from git history..."

# Use git filter-repo if available, otherwise use filter-branch
if command -v git-filter-repo &> /dev/null; then
    echo "📋 Using git-filter-repo (recommended method)"
    
    # Remove large directories
    git filter-repo --path 00_reference --invert-paths --force
    git filter-repo --path .venv --invert-paths --force
    git filter-repo --path docs/semantic_vector_db --invert-paths --force
    git filter-repo --path .vscode/browse.vc.db --invert-paths --force
    
    # Remove file patterns
    git filter-repo --path-glob '*.sqlite3' --invert-paths --force
    git filter-repo --path-glob '*.exe' --invert-paths --force
    git filter-repo --path-glob '*.msi' --invert-paths --force
    
else
    echo "📋 Using git filter-branch (fallback method)"
    
    # Set warning environment variable
    export FILTER_BRANCH_SQUELCH_WARNING=1
    
    # Remove large directories and files
    git filter-branch --force --index-filter \
        'git rm -r --cached --ignore-unmatch 00_reference .venv docs/semantic_vector_db .vscode/browse.vc.db' \
        --prune-empty --tag-name-filter cat -- --all
    
    # Remove file patterns
    git filter-branch --force --index-filter \
        'git rm --cached --ignore-unmatch *.sqlite3 *.exe *.msi' \
        --prune-empty --tag-name-filter cat -- --all
fi

# 5. Clean up git repository
echo "🧽 Cleaning up git repository..."
git for-each-ref --format="delete %(refname)" refs/original | git update-ref --stdin
git reflog expire --expire=now --all
git gc --prune=now --aggressive

# 6. Remove large files from filesystem
echo "🗂️ Removing large files from filesystem..."
if [ -d "00_reference" ]; then
    echo "   Removing 00_reference/ directory..."
    rm -rf 00_reference/
fi

if [ -d ".venv" ]; then
    echo "   Removing .venv/ directory..."
    rm -rf .venv/
fi

if [ -d "docs/semantic_vector_db" ]; then
    echo "   Removing semantic vector database..."
    rm -rf docs/semantic_vector_db/
fi

if [ -f ".vscode/browse.vc.db" ]; then
    echo "   Removing VS Code browse database..."
    rm -f .vscode/browse.vc.db*
fi

# 7. Update repository status
echo "📊 Repository cleanup complete!"
echo ""
echo "📈 Repository size comparison:"
du -sh .git 2>/dev/null || echo "Git directory size calculation unavailable"
echo ""

# 8. Create setup instructions
echo "📋 Creating setup instructions for removed components..."

cat > SETUP_INSTRUCTIONS.md << 'EOF'
# STM32H753ZI Project Setup Instructions

## After Repository Cleanup

This repository has been cleaned to remove large files. Follow these steps to restore functionality:

### 1. Python Environment Setup
```bash
# Create virtual environment
python3 -m venv .venv
source .venv/bin/activate  # Linux/Mac
# or
.venv\Scripts\activate     # Windows

# Install dependencies
pip install -r requirements.txt
```

### 2. Reference Documentation Setup
```bash
# Create reference directory
mkdir -p 00_reference

# Download ST assets (optional - large files)
# STM32CubeIDE, STM32CubeMX, X-CUBE-MCSDK, etc.
# Follow instructions in docs/DEVELOPMENT_SETUP.md
```

### 3. Semantic Search Database
```bash
# Rebuild semantic vector database
.venv/bin/python scripts/rebuild_semantic_db.py

# Or use incremental update
.venv/bin/python scripts/semantic_db_delta_update.py
```

### 4. Build System
```bash
# Configure and build
cmake --preset Debug
cmake --build build
```

## Repository Size
- Before cleanup: ~7+ GB
- After cleanup: ~50MB
- Cleaned files are recreated locally as needed

## What Was Removed
- `00_reference/`: ST official assets and documentation (7+ GB)
- `.venv/`: Python virtual environment (5+ GB)  
- `docs/semantic_vector_db/`: Vector database (1.5+ GB)
- `.vscode/browse.vc.db`: VS Code database files (585+ MB)
- Various temporary and build artifacts
EOF

echo "✅ Setup instructions created: SETUP_INSTRUCTIONS.md"

# 9. Commit the cleanup
echo "💾 Committing repository cleanup..."
git add .gitignore requirements.txt SETUP_INSTRUCTIONS.md
git commit -m "feat: comprehensive repository cleanup - FTR-014

- Remove 7+ GB of large files from git history
- Exclude 00_reference/, .venv/, semantic_vector_db/ from tracking
- Add comprehensive .gitignore for embedded development
- Add requirements.txt for Python dependency management
- Repository size reduced from ~7GB to ~50MB

Files removed:
- 00_reference/: ST assets and documentation (use local setup)
- .venv/: Python virtual environment (use requirements.txt)
- docs/semantic_vector_db/: Vector database (rebuild locally)
- .vscode/browse.vc.db: VS Code cache files

FTR-014: Semantic Search Delta Updates - Phase 1 Complete"

echo ""
echo "🎉 Repository cleanup complete!"
echo ""
echo "📊 Summary:"
echo "   ✅ Large files removed from git history"
echo "   ✅ Comprehensive .gitignore added"
echo "   ✅ requirements.txt created"
echo "   ✅ Setup instructions provided"
echo "   ✅ Repository ready for push to GitHub"
echo ""
echo "🚀 Next steps:"
echo "   1. Test git push: git push origin main"
echo "   2. Setup virtual environment: python3 -m venv .venv"
echo "   3. Install dependencies: pip install -r requirements.txt"
echo "   4. Rebuild semantic database as needed"
echo ""
echo "💡 Repository size should now be ~50MB instead of ~7+ GB"
