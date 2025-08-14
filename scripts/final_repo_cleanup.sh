#!/bin/bash

# Final aggressive repository cleanup
echo "🧹 Final Repository Cleanup - Removing Large Files from History"
echo "============================================================="

# Set environment to suppress filter-branch warnings
export FILTER_BRANCH_SQUELCH_WARNING=1

echo "📋 Current repository size:"
du -sh .git

echo "🔍 Finding largest objects in git history:"
git rev-list --objects --all | git cat-file --batch-check='%(objecttype) %(objectname) %(objectsize) %(rest)' | sort -k 3 -n | tail -10

echo "🗑️ Removing large files from git history using multiple passes..."

# Remove specific large files we identified
echo "   Removing STM32H7_FULL_INDEX.json files..."
git filter-branch -f --index-filter 'git rm --cached --ignore-unmatch "docs/indexes/STM32H7_FULL_INDEX.json"' --tag-name-filter cat -- --all

echo "   Removing semantic log files..."
git filter-branch -f --index-filter 'git rm --cached --ignore-unmatch "logs/semantic_rebuild_fixed.log"' --tag-name-filter cat -- --all

echo "   Removing any remaining large CMSIS files..."
git filter-branch -f --index-filter 'git rm --cached --ignore-unmatch "drivers/CMSIS/DSP/Source/CommonTables/arm_common_tables.c"' --tag-name-filter cat -- --all

echo "   Removing large example data files..."
git filter-branch -f --index-filter 'git rm --cached --ignore-unmatch "drivers/CMSIS/DSP/Examples/ARM/arm_linear_interp_example/arm_linear_interp_data.c"' --tag-name-filter cat -- --all

echo "🧽 Running aggressive garbage collection..."
git for-each-ref --format='delete %(refname)' refs/original | git update-ref --stdin
git reflog expire --expire=now --all
git gc --aggressive --prune=now

echo "📊 Final repository size:"
du -sh .git

echo "✅ Cleanup completed!"
