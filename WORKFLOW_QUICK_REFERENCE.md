# Development Workflow Quick Reference

## 🔄 **Daily Development Workflow**

### **Before Starting Work (Mandatory)**
```bash
# 1. Update STATUS.md from current state
source .venv/bin/activate
python scripts/auto_update_status.py --verbose

# 2. Check feature status to understand current context
python scripts/feature_tracker.py list --status IN_PROGRESS
python scripts/feature_tracker.py show FTR-009  # Replace with your feature ID

# 3. Validate Copilot has current context (STATUS.md should reflect latest changes)
```

### **During Work (Per Development Session)**
```bash
# Update feature status when making progress
python scripts/feature_tracker.py update FTR-009 --status IN_PROGRESS --notes "Updated motor control algorithm"

# Build and validate changes
./scripts/fix_cmake.sh

# Update STATUS.md to reflect build progress
python scripts/auto_update_status.py --source build --verbose
```

### **After Work (Before Committing)**
```bash
# 1. Final feature status update
python scripts/feature_tracker.py update FTR-009 --status COMPLETE --notes "Motor efficiency optimization implemented and tested"

# 2. Update STATUS.md with final state
python scripts/auto_update_status.py --source build --verbose

# 3. Commit with descriptive message
git add .
git commit -m "feat(motor): implement efficiency optimization algorithm

- Enhanced PID control loop for reduced power consumption
- Added adaptive step rate adjustment based on load
- Implemented predictive torque control
- Validates against hardware constraints

Completes: FTR-009"

# 4. Verify git hooks updated STATUS.md automatically
# (Should happen automatically via post-commit hook)
```

## 📊 **Feature Tracking Commands**

### **List Features**
```bash
python scripts/feature_tracker.py list --status COMPLETE     # Show completed features
python scripts/feature_tracker.py list --status IN_PROGRESS # Show active work
python scripts/feature_tracker.py list --status PLANNED     # Show future work
```

### **Feature Details**
```bash
python scripts/feature_tracker.py show FTR-009              # Show specific feature
python scripts/feature_tracker.py report                    # Project completion report
python scripts/feature_tracker.py dependencies FTR-008     # Show dependencies
```

### **Update Features**
```bash
python scripts/feature_tracker.py update FTR-009 --status IN_PROGRESS --notes "Starting implementation"
python scripts/feature_tracker.py update FTR-009 --status COMPLETE --notes "Implementation validated"
```

## 🎯 **STATUS.md Integration Commands**

### **Manual Updates**
```bash
python scripts/auto_update_status.py --verbose              # Quick update
python scripts/auto_update_status.py --source build --verbose # After build
python scripts/auto_update_status.py --dry-run --verbose    # Preview changes
python scripts/auto_update_status.py --force --verbose      # Force full update
```

### **VS Code Tasks**
- `Ctrl+Shift+P` → "Tasks: Run Task" → "STATUS: Quick Update"
- `Ctrl+Shift+P` → "Tasks: Run Task" → "STATUS: Build + Update"
- `Ctrl+Shift+P` → "Tasks: Run Task" → "STATUS: Preview Changes"

## 🛡️ **Workflow Validation**

### **Before Each Session**
```bash
# Ensure environment is ready
./scripts/validate_build_env.sh

# Verify SSOT compliance
python scripts/validate_ssot.py

# Check feature registry integrity
python scripts/feature_tracker.py validate
```

### **Quality Checks**
```bash
# Build validation
./scripts/fix_cmake.sh

# Documentation validation
python scripts/link_validator.py

# SSOT validation
python scripts/validate_ssot.py
```

## 🎯 **Success Indicators**

### **Workflow Working Correctly:**
1. ✅ STATUS.md automatically updates after commits (git hooks)
2. ✅ Feature status changes are reflected in STATUS.md
3. ✅ Copilot has accurate context about current work
4. ✅ No "lost context" between development sessions
5. ✅ Build status matches feature completion status

### **Common Anti-Patterns to Avoid:**
- ❌ Committing without updating feature status
- ❌ Working without checking current STATUS.md
- ❌ Forgetting to validate build environment
- ❌ Manual STATUS.md edits without using automation
- ❌ Starting work without understanding feature dependencies

## 🚀 **Next Session Checklist**

**Before starting any work:**
1. [ ] Check STATUS.md for current project state
2. [ ] Review feature tracker for active work: `python scripts/feature_tracker.py list --status IN_PROGRESS`
3. [ ] Validate build environment: `./scripts/validate_build_env.sh`
4. [ ] Update STATUS.md: `python scripts/auto_update_status.py --verbose`
5. [ ] Confirm Copilot has accurate context

**This ensures every development session starts with accurate, current context!**
