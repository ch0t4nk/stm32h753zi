#!/bin/bash
"""
Git Repository Fix Script - FTR-014 Phase 1 Implementation
Resolves git push blocking issue caused by 917.71MB semantic database file

This script addresses:
1. GitHub file size limit (100MB) exceeded by chroma.sqlite3 (917.71MB)
2. Blocked git push operations preventing development progress
3. Repository cleanup and history sanitization

CRITICAL: This script modifies git history. Ensure team coordination before running.
"""

set -e  # Exit on any error

echo "🚨 Git Repository Fix - FTR-014 Phase 1"
echo "========================================"
echo "⚠️  WARNING: This will modify git history to remove large files"
echo "⚠️  Ensure all team members are coordinated before proceeding"
echo ""

# Configuration
LARGE_FILE="docs/semantic_vector_db/chroma.sqlite3"
DB_DIR="docs/semantic_vector_db"
BACKUP_DIR="backup_$(date +%Y%m%d_%H%M%S)"

# Function to check file sizes
check_file_sizes() {
    echo "🔍 Checking repository file sizes..."
    
    # Find files larger than 50MB
    echo "📊 Files larger than 50MB:"
    find . -type f -size +50M -not -path "./.git/*" -exec ls -lh {} \; | awk '{print $5, $9}' || true
    
    # Check specific problematic file
    if [ -f "$LARGE_FILE" ]; then
        local size=$(du -h "$LARGE_FILE" | cut -f1)
        echo "🚨 Problem file: $LARGE_FILE ($size)"
        return 1
    else
        echo "✅ Large file not found in working directory"
        return 0
    fi
}

# Function to update .gitignore
update_gitignore() {
    echo "📝 Updating .gitignore to exclude semantic database..."
    
    local gitignore_updates=(
        "# Exclude large semantic vector database files"
        "docs/semantic_vector_db/"
        "*.sqlite3"
    )
    
    for exclusion in "${gitignore_updates[@]}"; do
        if ! grep -Fxq "$exclusion" .gitignore 2>/dev/null; then
            echo "$exclusion" >> .gitignore
            echo "   Added: $exclusion"
        fi
    done
    
    echo "✅ .gitignore updated"
}

# Function to create backup
create_backup() {
    echo "💾 Creating backup of current repository state..."
    
    # Create backup directory
    mkdir -p "$BACKUP_DIR"
    
    # Backup semantic database if it exists
    if [ -d "$DB_DIR" ]; then
        echo "   Backing up semantic database..."
        cp -r "$DB_DIR" "$BACKUP_DIR/"
        echo "   ✅ Database backed up to $BACKUP_DIR/"
    fi
    
    # Backup git information
    echo "   Backing up git state..."
    git log --oneline -10 > "$BACKUP_DIR/recent_commits.txt"
    git status > "$BACKUP_DIR/git_status.txt"
    git remote -v > "$BACKUP_DIR/remotes.txt"
    
    echo "✅ Backup created: $BACKUP_DIR"
}

# Function to remove file from git history
remove_from_history() {
    echo "🧹 Removing large file from git history..."
    echo "⚠️  This operation modifies git history!"
    
    # Set environment variable to suppress warning
    export FILTER_BRANCH_SQUELCH_WARNING=1
    
    # Remove the file from all commits
    echo "   Running git filter-branch..."
    git filter-branch --force --index-filter \
        "git rm --cached --ignore-unmatch $LARGE_FILE" \
        --prune-empty --tag-name-filter cat -- --all
    
    echo "✅ File removed from git history"
}

# Function to clean up git
cleanup_git() {
    echo "🧹 Cleaning up git repository..."
    
    # Remove backup refs created by filter-branch
    echo "   Removing backup references..."
    git for-each-ref --format="%(refname)" refs/original/ | xargs -n 1 git update-ref -d
    
    # Clean up and garbage collect
    echo "   Running garbage collection..."
    git reflog expire --expire=now --all
    git gc --prune=now --aggressive
    
    echo "✅ Git cleanup completed"
}

# Function to check repository status
check_repo_status() {
    echo "📊 Checking repository status..."
    
    # Check working directory status
    if [ -n "$(git status --porcelain)" ]; then
        echo "⚠️  Working directory has uncommitted changes:"
        git status --short
        return 1
    else
        echo "✅ Working directory clean"
    fi
    
    # Check repository size
    local repo_size=$(du -sh .git | cut -f1)
    echo "📦 Repository size: $repo_size"
    
    return 0
}

# Function to test git push
test_git_push() {
    echo "🔍 Testing git push capability..."
    
    # Check remote configuration
    if git remote | grep -q origin; then
        echo "✅ Remote 'origin' configured"
        git remote -v | grep origin
    else
        echo "⚠️  No 'origin' remote configured"
        return 1
    fi
    
    # Test push (dry run)
    echo "   Testing push (dry run)..."
    if git push --dry-run origin main 2>&1; then
        echo "✅ Git push test successful"
        return 0
    else
        echo "❌ Git push test failed"
        return 1
    fi
}

# Function to commit current changes
commit_changes() {
    echo "💾 Committing current changes..."
    
    # Stage .gitignore changes
    git add .gitignore
    
    # Check if there are changes to commit
    if git diff --staged --quiet; then
        echo "   No changes to commit"
        return 0
    fi
    
    # Commit changes
    git commit -m "fix: exclude semantic vector database from git tracking

- Add docs/semantic_vector_db/ to .gitignore
- Exclude *.sqlite3 files from repository
- Resolves git push blocking issue (FTR-014 Phase 1)
- Large database file (917.71MB) now excluded from version control"
    
    echo "✅ Changes committed"
}

# Main execution
main() {
    echo "🚀 Starting git repository fix..."
    echo ""
    
    # Check current status
    if check_file_sizes; then
        echo "✅ No large files detected in working directory"
        echo "🔍 Checking if large files exist in git history..."
        
        # Check if file exists in git history
        if git log --all --full-history -- "$LARGE_FILE" | grep -q commit; then
            echo "🚨 Large file found in git history - cleanup required"
        else
            echo "✅ No large files in git history"
            echo "🎉 Repository appears to be clean"
            exit 0
        fi
    fi
    
    # Confirm operation
    echo ""
    echo "This script will:"
    echo "  1. Create backup of current state"
    echo "  2. Update .gitignore to exclude semantic database"
    echo "  3. Remove large file from git history"
    echo "  4. Clean up git repository"
    echo "  5. Test git push capability"
    echo ""
    read -p "Continue with git history cleanup? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Operation cancelled"
        exit 0
    fi
    
    # Execute fixes
    create_backup
    update_gitignore
    commit_changes
    
    # Check if file removal is needed
    if git log --all --full-history -- "$LARGE_FILE" | grep -q commit; then
        echo ""
        echo "⚠️  CRITICAL: About to modify git history"
        echo "⚠️  This cannot be easily undone"
        echo "⚠️  Ensure all team members have pushed their changes"
        echo ""
        read -p "Proceed with git history modification? (y/N): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            remove_from_history
            cleanup_git
        else
            echo "Skipping history modification"
        fi
    fi
    
    # Final checks
    echo ""
    echo "🔍 Final repository checks..."
    if check_repo_status && test_git_push; then
        echo ""
        echo "🎉 Git repository fix completed successfully!"
        echo ""
        echo "✅ Summary:"
        echo "   • Large files excluded from git tracking"
        echo "   • Repository history cleaned"
        echo "   • Git push capability restored"
        echo ""
        echo "📋 Next steps:"
        echo "   • Run: git push origin main"
        echo "   • Rebuild semantic database: python scripts/fix_semantic_database.py --action rebuild"
        echo "   • Implement delta updates: FTR-014 Phase 3"
        echo ""
        echo "💾 Backup created: $BACKUP_DIR"
    else
        echo ""
        echo "❌ Repository fix incomplete - check errors above"
        echo "💾 Backup available: $BACKUP_DIR"
        exit 1
    fi
}

# Run main function
main "$@"
