#!/bin/bash

# STM32H753ZI Development Workflow Script
# Implements the systematic development workflow from development-workflow.instructions.md

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Emojis for visual feedback
CHECK="✅"
WORK="🔄"
WARN="⚠️"
INFO="ℹ️"
ROCKET="🚀"
FINISH="🏁"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Ensure we're in the project root
cd "$PROJECT_ROOT"

# Activate virtual environment
if [ -f ".venv/bin/activate" ]; then
    source .venv/bin/activate
    echo -e "${GREEN}${CHECK} Virtual environment activated${NC}"
else
    echo -e "${RED}${WARN} Virtual environment not found at .venv/bin/activate${NC}"
    exit 1
fi

show_usage() {
    echo -e "${CYAN}STM32H753ZI Development Workflow${NC}"
    echo ""
    echo -e "${YELLOW}Usage:${NC} $0 <phase> [options]"
    echo ""
    echo -e "${PURPLE}Phases:${NC}"
    echo -e "  ${GREEN}before${NC}    - Start work session (update context, check features)"
    echo -e "  ${BLUE}during${NC}    - Development work (build, test, update status)"
    echo -e "  ${GREEN}after${NC}     - End work session (final validation, prepare commit)"
    echo ""
    echo -e "${PURPLE}Options:${NC}"
    echo -e "  --feature-id ID  - Specify feature ID for during/after phases"
    echo -e "  --notes \"text\"   - Add notes when updating feature status"
    echo -e "  --dry-run        - Preview changes without executing"
    echo -e "  --help           - Show this help message"
    echo ""
    echo -e "${CYAN}Examples:${NC}"
    echo -e "  $0 before"
    echo -e "  $0 during --feature-id FTR-009"
    echo -e "  $0 after --feature-id FTR-009 --notes \"Implementation complete\""
}

validate_environment() {
    echo -e "${BLUE}${INFO} Validating development environment...${NC}"
    
    # Check ARM GCC
    if ! command -v arm-none-eabi-gcc &> /dev/null; then
        echo -e "${RED}${WARN} ARM GCC not found${NC}"
        return 1
    fi
    
    # Check Python tools (warn but don't fail on SSOT issues)
    if ! python scripts/validate_ssot.py &> /dev/null; then
        echo -e "${YELLOW}${WARN} SSOT validation found issues (see: python scripts/validate_ssot.py)${NC}"
    fi
    
    echo -e "${GREEN}${CHECK} Environment validated${NC}"
    return 0
}

before_work() {
    echo -e "${CYAN}${ROCKET} Starting work session...${NC}"
    echo ""
    
    # 1. Validate environment
    if ! validate_environment; then
        echo -e "${RED}${WARN} Environment validation failed. Run: ./scripts/validate_build_env.sh${NC}"
        exit 1
    fi
    
    # 2. Update STATUS.md from current state
    echo -e "${BLUE}${INFO} Updating STATUS.md with current project state...${NC}"
    python scripts/auto_update_status.py --verbose
    
    # 3. Show current feature status
    echo -e "${BLUE}${INFO} Current project status:${NC}"
    echo ""
    
    echo -e "${PURPLE}📊 Project Completion:${NC}"
    python scripts/feature_tracker.py report | grep -E "(Total Features|Completed|In Progress|Planned)" | sed 's/^/  /'
    
    echo ""
    echo -e "${PURPLE}🔄 Active Work:${NC}"
    if python scripts/feature_tracker.py list --status IN_PROGRESS | grep -q "FTR-"; then
        python scripts/feature_tracker.py list --status IN_PROGRESS | tail -n +2 | sed 's/^/  /'
    else
        echo -e "  ${YELLOW}No features currently in progress${NC}"
    fi
    
    echo ""
    echo -e "${PURPLE}📋 Next Planned:${NC}"
    if python scripts/feature_tracker.py list --status PLANNED | grep -q "FTR-"; then
        python scripts/feature_tracker.py list --status PLANNED | head -3 | tail -n +2 | sed 's/^/  /'
    else
        echo -e "  ${YELLOW}No planned features${NC}"
    fi
    
    echo ""
    echo -e "${GREEN}${CHECK} Work session initialized${NC}"
    echo -e "${CYAN}${INFO} Ready to start development. Use: ./scripts/workflow.sh during --feature-id <ID>${NC}"
}

during_work() {
    local feature_id="$1"
    local notes="$2"
    
    echo -e "${CYAN}${WORK} Development work in progress...${NC}"
    echo ""
    
    if [ -n "$feature_id" ]; then
        # Update feature status
        if [ -n "$notes" ]; then
            echo -e "${BLUE}${INFO} Updating feature $feature_id status with notes...${NC}"
            python scripts/feature_tracker.py update "$feature_id" --status IN_PROGRESS --notes "$notes"
        else
            echo -e "${BLUE}${INFO} Marking feature $feature_id as in progress...${NC}"
            python scripts/feature_tracker.py update "$feature_id" --status IN_PROGRESS
        fi
        echo ""
    fi
    
    # Build project
    echo -e "${BLUE}${INFO} Building project...${NC}"
    ./scripts/fix_cmake.sh
    
    # Update STATUS.md with build results
    echo -e "${BLUE}${INFO} Updating STATUS.md with build results...${NC}"
    python scripts/auto_update_status.py --source build --verbose
    
    echo ""
    echo -e "${GREEN}${CHECK} Development cycle complete${NC}"
    
    if [ -n "$feature_id" ]; then
        echo -e "${CYAN}${INFO} Feature $feature_id updated and build validated${NC}"
    fi
}

after_work() {
    local feature_id="$1"
    local notes="$2"
    
    echo -e "${CYAN}${FINISH} Preparing to complete work session...${NC}"
    echo ""
    
    if [ -n "$feature_id" ]; then
        # Final feature status update
        if [ -n "$notes" ]; then
            echo -e "${BLUE}${INFO} Finalizing feature $feature_id with notes...${NC}"
            python scripts/feature_tracker.py update "$feature_id" --status COMPLETE --notes "$notes"
        else
            echo -e "${BLUE}${INFO} Marking feature $feature_id as complete...${NC}"
            python scripts/feature_tracker.py update "$feature_id" --status COMPLETE
        fi
        echo ""
    fi
    
    # Final build validation
    echo -e "${BLUE}${INFO} Final build validation...${NC}"
    ./scripts/fix_cmake.sh
    
    # Update STATUS.md with final state
    echo -e "${BLUE}${INFO} Updating STATUS.md with final state...${NC}"
    python scripts/auto_update_status.py --source build --verbose
    
    # Show git status
    echo -e "${BLUE}${INFO} Current git status:${NC}"
    git status --porcelain | sed 's/^/  /'
    
    echo ""
    echo -e "${GREEN}${CHECK} Work session ready for commit${NC}"
    
    if [ -n "$feature_id" ]; then
        echo -e "${CYAN}${INFO} Suggested commit message:${NC}"
        echo -e "${YELLOW}git commit -m \"feat: [description] - Completes: $feature_id\"${NC}"
    else
        echo -e "${CYAN}${INFO} Remember to include feature ID in commit message${NC}"
    fi
    
    echo ""
    echo -e "${PURPLE}Next steps:${NC}"
    echo -e "  1. Review changes: ${YELLOW}git diff${NC}"
    echo -e "  2. Add changes: ${YELLOW}git add .${NC}"
    echo -e "  3. Commit: ${YELLOW}git commit -m \"descriptive message\"${NC}"
    echo -e "  4. Git hooks will automatically update STATUS.md"
}

# Parse command line arguments
PHASE=""
FEATURE_ID=""
NOTES=""
DRY_RUN=false

while [[ $# -gt 0 ]]; do
    case $1 in
        before|during|after)
            PHASE="$1"
            shift
            ;;
        --feature-id)
            FEATURE_ID="$2"
            shift 2
            ;;
        --notes)
            NOTES="$2"
            shift 2
            ;;
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        --help|-h)
            show_usage
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            show_usage
            exit 1
            ;;
    esac
done

# Validate required arguments
if [ -z "$PHASE" ]; then
    echo -e "${RED}Error: Phase required${NC}"
    show_usage
    exit 1
fi

# Execute the appropriate phase
case $PHASE in
    before)
        before_work
        ;;
    during)
        during_work "$FEATURE_ID" "$NOTES"
        ;;
    after)
        after_work "$FEATURE_ID" "$NOTES"
        ;;
    *)
        echo -e "${RED}Error: Invalid phase: $PHASE${NC}"
        show_usage
        exit 1
        ;;
esac

echo ""
echo -e "${GREEN}${CHECK} Workflow phase '$PHASE' completed successfully${NC}"
