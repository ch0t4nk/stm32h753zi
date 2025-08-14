# Development Workflow Quick Reference

## 🔄 **Daily Development Workflow (SSOT Configuration)**

### **Configure Workflow Complexity (First Time Setup)**
```powershell
# Choose workflow mode based on your needs
python scripts/workflow_config.py mode minimal    # Fast, no validation (CI/CD)
python scripts/workflow_config.py mode standard   # Balanced (daily development) - RECOMMENDED
python scripts/workflow_config.py mode thorough   # Full validation (critical work)
python scripts/workflow_config.py mode debug      # Maximum verbosity (troubleshooting)

# View current configuration
python scripts/workflow_config.py show
```

### **Before Starting Work (Mandatory)**
```powershell
# 1. Update STATUS.md from current state (SSOT-aware)
scripts/run_python_configurable.ps1 scripts/auto_update_status.py --verbose

# 2. Check feature status to understand current context
scripts/run_python_configurable.ps1 scripts/feature_tracker.py list --status IN_PROGRESS
scripts/run_python_configurable.ps1 scripts/feature_tracker.py show FTR-009  # Replace with your feature ID

# 3. Validate Copilot has current context (STATUS.md should reflect latest changes)
```

### **During Work (Per Development Session)**
```powershell
# Update feature status when making progress
scripts/run_python_configurable.ps1 scripts/feature_tracker.py update FTR-009 --status IN_PROGRESS --notes "Updated motor control algorithm"

# Build and validate changes (cross-platform)
powershell -ExecutionPolicy Bypass -File scripts/fix_cmake.ps1

# Update STATUS.md to reflect build progress (SSOT-aware)
scripts/run_python_configurable.ps1 scripts/auto_update_status.py --source build --verbose
```

### **After Work (Before Committing)**
```powershell
# 1. Final feature status update
scripts/run_python_configurable.ps1 scripts/feature_tracker.py update FTR-009 --status COMPLETE --notes "Motor efficiency optimization implemented and tested"

# 2. Update STATUS.md with final state (SSOT-aware)
scripts/run_python_configurable.ps1 scripts/auto_update_status.py --source build --verbose

# 3. Commit with descriptive message
git add .
git commit -m "feat(motor): implement efficiency optimization algorithm

- Enhanced PID control loop for reduced power consumption
- Added adaptive step rate adjustment based on load
- Implemented predictive torque control
- Validates against hardware constraints

Completes: FTR-009"

# 4. Verify git hooks updated STATUS.md automatically
# (Should happen automatically via cross-platform post-commit hook)
```

## 📊 **Feature Tracking Commands (SSOT-Aware)**

### **List Features**
```powershell
scripts/run_python_configurable.ps1 scripts/feature_tracker.py list --status COMPLETE     # Show completed features
scripts/run_python_configurable.ps1 scripts/feature_tracker.py list --status IN_PROGRESS # Show active work
scripts/run_python_configurable.ps1 scripts/feature_tracker.py list --status PLANNED     # Show future work
```

### **Feature Details**
```powershell
scripts/run_python_configurable.ps1 scripts/feature_tracker.py show FTR-009              # Show specific feature
scripts/run_python_configurable.ps1 scripts/feature_tracker.py report                    # Project completion report
scripts/run_python_configurable.ps1 scripts/feature_tracker.py dependencies FTR-008     # Show dependencies
```

### **Update Features**
```powershell
scripts/run_python_configurable.ps1 scripts/feature_tracker.py update FTR-009 --status IN_PROGRESS --notes "Starting implementation"
scripts/run_python_configurable.ps1 scripts/feature_tracker.py update FTR-009 --status COMPLETE --notes "Implementation validated"
```

## 🎯 **STATUS.md Integration Commands (SSOT-Aware)**

### **Manual Updates**
```powershell
scripts/run_python_configurable.ps1 scripts/auto_update_status.py --verbose              # Quick update
scripts/run_python_configurable.ps1 scripts/auto_update_status.py --source build --verbose # After build
scripts/run_python_configurable.ps1 scripts/auto_update_status.py --dry-run --verbose    # Preview changes
scripts/run_python_configurable.ps1 scripts/auto_update_status.py --force --verbose      # Force full update
```

### **VS Code Tasks**
- `Ctrl+Shift+P` → "Tasks: Run Task" → "STATUS: Quick Update"
- `Ctrl+Shift+P` → "Tasks: Run Task" → "STATUS: Build + Update"
- `Ctrl+Shift+P` → "Tasks: Run Task" → "STATUS: Preview Changes"

## 🛡️ **Workflow Validation (SSOT-Aware)**

### **Before Each Session**
```powershell
# Configure workflow complexity first
python scripts/workflow_config.py show

# Ensure environment is ready (SSOT-aware)
scripts/run_python_configurable.ps1 scripts/validate_build_env.py

# Verify SSOT compliance
scripts/run_python_configurable.ps1 scripts/validate_ssot.py

# Check feature registry integrity
scripts/run_python_configurable.ps1 scripts/feature_tracker.py validate
```

### **Quality Checks (SSOT-Aware)**
```powershell
# Build validation (cross-platform)
powershell -ExecutionPolicy Bypass -File scripts/fix_cmake.ps1

# Documentation validation
scripts/run_python_configurable.ps1 scripts/link_validator.py

# SSOT validation
scripts/run_python_configurable.ps1 scripts/validate_ssot.py

# Workflow configuration validation
python scripts/workflow_config.py validate
```

## 🎯 **Success Indicators**

### **Workflow Working Correctly:**
1. ✅ STATUS.md automatically updates after commits (cross-platform git hooks)
2. ✅ Feature status changes are reflected in STATUS.md
3. ✅ Copilot has accurate context about current work
4. ✅ No "lost context" between development sessions
5. ✅ Build status matches feature completion status
6. ✅ SSOT workflow configuration enables appropriate complexity level

### **Common Anti-Patterns to Avoid:**
- ❌ Committing without updating feature status
- ❌ Working without checking current STATUS.md
- ❌ Forgetting to validate build environment
- ❌ Manual STATUS.md edits without using automation
- ❌ Starting work without understanding feature dependencies
- ❌ Using wrong workflow complexity mode for the task

## 🎛️ **SSOT Workflow Modes Reference**

### **Quick Mode Selection**
```powershell
# Choose based on your development scenario:
python scripts/workflow_config.py mode minimal    # CI/CD, experienced developers, fast iteration
python scripts/workflow_config.py mode standard   # Daily development, balanced validation
python scripts/workflow_config.py mode thorough   # Critical features, comprehensive validation
python scripts/workflow_config.py mode debug      # Troubleshooting, investigation, maximum info
```

### **Mode Characteristics**
| Mode | Validation | Speed | Use Case |
|------|-----------|-------|----------|
| **minimal** | None | ⚡ Fastest | CI/CD, expert developers |
| **standard** | Core | 🏃 Fast | Daily development |
| **thorough** | Full | 🚶 Moderate | Critical development |
| **debug** | Maximum | 🐌 Detailed | Troubleshooting |

## 🚀 **Next Session Checklist (SSOT-Aware)**

**Before starting any work:**
1. [ ] Configure workflow mode: `python scripts/workflow_config.py mode standard`
2. [ ] Check STATUS.md for current project state
3. [ ] Review feature tracker: `scripts/run_python_configurable.ps1 scripts/feature_tracker.py list --status IN_PROGRESS`
4. [ ] Validate build environment: `powershell -ExecutionPolicy Bypass -File scripts/fix_cmake.ps1`
5. [ ] Update STATUS.md: `scripts/run_python_configurable.ps1 scripts/auto_update_status.py --verbose`
6. [ ] Confirm Copilot has accurate context

**This ensures every development session starts with accurate, current context and appropriate workflow complexity!**
