#!/bin/bash
# Enhanced post-commit hook with optional changelog generation
# Integrates with existing STATUS.md automation system

# Get workspace root
WORKSPACE_ROOT="$(git rev-parse --show-toplevel)"
cd "$WORKSPACE_ROOT"

# Configuration
ENABLE_CHANGELOG=${ENABLE_CHANGELOG_AUTOMATION:-false}
ENABLE_STATUS=${ENABLE_STATUS_AUTOMATION:-true}

# Logging function
log_automation() {
    echo "[$(date '+%H:%M:%S')] POST-COMMIT: $1" >> logs/automation.log
}

# Main automation sequence
main() {
    log_automation "Starting post-commit automation"
    
    # Always run STATUS.md updates (existing automation)
    if [ "$ENABLE_STATUS" = "true" ]; then
        log_automation "Running STATUS.md automation"
        if [ -f ".venv/bin/python" ]; then
            .venv/bin/python scripts/auto_update_status.py --source commit 2>/dev/null || {
                log_automation "STATUS.md automation failed"
            }
        fi
    fi
    
    # Optional changelog automation
    if [ "$ENABLE_CHANGELOG" = "true" ]; then
        log_automation "Running changelog automation"
        
        # Check if this is a conventional commit
        COMMIT_MSG=$(git log -1 --pretty=format:"%s")
        if [[ "$COMMIT_MSG" =~ ^(feat|fix|docs|perf|refactor|test|ci|build|chore)(\(.+\))?:.+ ]]; then
            log_automation "Conventional commit detected: $COMMIT_MSG"
            
            if [ -f ".venv/bin/python" ]; then
                .venv/bin/python scripts/auto_update_changelog.py --since-hours 1 2>/dev/null || {
                    log_automation "Changelog automation failed"
                }
            fi
        else
            log_automation "Non-conventional commit, skipping changelog: $COMMIT_MSG"
        fi
    fi
    
    log_automation "Post-commit automation complete"
}

# Prevent infinite loops
if [ "$GIT_AUTOMATION_RUNNING" = "true" ]; then
    log_automation "Automation already running, skipping"
    exit 0
fi

export GIT_AUTOMATION_RUNNING=true
main
unset GIT_AUTOMATION_RUNNING
