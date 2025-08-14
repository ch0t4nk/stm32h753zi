---
applyTo: "**/*.{c,h,md,json,py,sh,yml,yaml}"
description: "Development workflow guidelines ensuring feature tracking and STATUS.md integration for STM32H753ZI stepper motor project"
---

# Development Workflow Instructions

## Overview

This instruction file provides the **mandatory development workflow** that ensures feature tracking and STATUS.md remain synchronized, providing GitHub Copilot with accurate project context and maintaining comprehensive development continuity.

## 🎯 **CRITICAL WORKFLOW**: Feature-Status Integration

### **The Problem**

Without systematic workflow, features get implemented but:

- Feature tracking status doesn't get updated → Copilot sees outdated project state
- STATUS.md doesn't reflect current work → Context becomes stale
- Development sessions lose continuity → Repeated context gathering
- Progress tracking becomes manual → Error-prone status reports

### **The Solution**: Integrated Development Workflow

## 📋 **MANDATORY Development Workflow**

### **Phase 1: Before Starting Work**

#### **1.1 Check Current Feature Status**

```bash
# ALWAYS start by checking feature status
.venv/bin/python scripts/feature_tracker.py list --status IN_PROGRESS
.venv/bin/python scripts/feature_tracker.py list --status PLANNED

# Check what Copilot sees as current work
head -50 STATUS.md
```

#### **1.2 Select or Create Feature**

```bash
# Option A: Continue existing feature
.venv/bin/python scripts/feature_tracker.py show FTR-XXX

# Option B: Create new feature if needed
# (Follow feature-management.instructions.md for creation)

# Option C: Check dependencies
.venv/bin/python scripts/feature_tracker.py dependencies FTR-XXX
```

#### **1.3 Update Feature Status to IN_PROGRESS**

```bash
# MANDATORY: Update feature status when starting work
.venv/bin/python scripts/feature_tracker.py update FTR-XXX --status IN_PROGRESS

# Add any new blockers or dependencies discovered
.venv/bin/python scripts/feature_tracker.py update FTR-XXX --blockers "Hardware procurement"
```

### **Phase 2: During Development**

#### **2.1 Regular Feature Updates**

```bash
# Update progress regularly (daily for active features)
.venv/bin/python scripts/feature_tracker.py update FTR-XXX --effort-actual 3

# Add implementation files as they're created
.venv/bin/python scripts/feature_tracker.py update FTR-XXX --implementation-files "src/new_module.c,src/new_module.h"

# Update blockers as they're resolved
.venv/bin/python scripts/feature_tracker.py update FTR-XXX --remove-blocker "Hardware procurement"
```

#### **2.2 Continuous Status Awareness**

```bash
# Check feature status impacts on overall project
.venv/bin/python scripts/feature_tracker.py report

# Generate feature summary for STATUS.md integration
.venv/bin/python scripts/feature_status_summary.py
```

### **Phase 3: Completion and Integration**

#### **3.1 Feature Completion Process**

```bash
# MANDATORY: Update feature status when work is complete
.venv/bin/python scripts/feature_tracker.py update FTR-XXX --status COMPLETE

# Add final implementation details
.venv/bin/python scripts/feature_tracker.py update FTR-XXX --implementation-files "complete,file,list"
.venv/bin/python scripts/feature_tracker.py update FTR-XXX --test-files "associated,test,files"
```

#### **3.2 STATUS.md Synchronization**

```bash
# MANDATORY: Update STATUS.md to reflect current feature state
.venv/bin/python scripts/auto_update_status.py --verbose

# Verify Copilot context is current
.venv/bin/python scripts/feature_tracker.py report | head -10
```

#### **3.3 Git Commit Integration**

```bash
# Include feature reference in commit messages
git commit -m "feat: implement motor efficiency optimization

FTR-009: Motor Efficiency Optimization - COMPLETE
- Implemented control loop optimization algorithms
- Added real-time performance monitoring
- Validated 15% efficiency improvement on test hardware

Feature Status: IN_PROGRESS → COMPLETE
Next: FTR-008 (Hardware Integration Testing)"

# Git hooks will automatically update STATUS.md
```

## ⚙️ **SSOT Workflow Configuration System**

### **Selective Workflow Complexity Control**

The workflow system now uses SSOT configuration to allow selective enabling/disabling of complex validation patterns:

#### **Configuration Management**

```powershell
# Quick mode changes for different user types
python scripts/workflow_config.py mode minimal    # Fast, no validation (CI/CD)
python scripts/workflow_config.py mode standard   # Balanced (daily development)
python scripts/workflow_config.py mode thorough   # Full validation (critical work)
python scripts/workflow_config.py mode debug      # Maximum verbosity (troubleshooting)

# Individual setting control
python scripts/workflow_config.py set verbose_logging false
python scripts/workflow_config.py set validate_venv true

# View current configuration
python scripts/workflow_config.py show
```

#### **Cross-Platform Execution**

```powershell
# Use SSOT-aware script for all Python operations
scripts/run_python_configurable.ps1 scripts/auto_update_status.py --verbose
scripts/run_python_configurable.ps1 scripts/feature_tracker.py list --status IN_PROGRESS
```

#### **SSOT Configuration Benefits**

✅ **User Experience**: New users get thorough guidance, experts get speed  
✅ **Selective Complexity**: Enable/disable any workflow pattern individually  
✅ **Cross-Platform**: Works identically on Windows/Linux/macOS  
✅ **Zero Breaking Changes**: All existing workflows continue to work  
✅ **SSOT Compliance**: All configuration centralized in `src/config/workflow_config.h`

## 🤖 **Copilot Integration Guidelines**

### **Making Copilot Aware of Feature Changes**

#### **Always Include Feature Context**

When working with Copilot, provide feature context:

```markdown
Working on FTR-009 (Motor Efficiency Optimization)
Status: IN_PROGRESS → target: COMPLETE this session
Dependencies: FTR-008 (blocked on hardware)
Implementation: src/controllers/motor_efficiency.c
```

#### **Reference Current Feature Status**

```bash
# Show Copilot current feature state
.venv/bin/python scripts/feature_tracker.py show FTR-XXX
.venv/bin/python scripts/feature_tracker.py list --category motor_control
```

#### **Update Copilot Context After Changes**

```bash
# After feature updates, refresh Copilot context
.venv/bin/python scripts/auto_update_status.py --verbose
.venv/bin/python scripts/feature_tracker.py report
```

### **Conversation Continuity Pattern**

```markdown
## Session Context for Copilot

**Current Feature**: FTR-XXX (Name) - STATUS
**Last Updated**: [timestamp from feature tracker]
**Implementation Progress**: [files completed]
**Blockers**: [current blockers if any]
**Next Steps**: [specific next actions]

This ensures Copilot understands exactly where development stands.
```

## 🔄 **STATUS.md Integration Requirements**

### **Automatic Integration Points**

#### **Git Hook Integration**

The git post-commit hook automatically:

1. Runs feature status summary
2. Updates STATUS.md with current feature progress
3. Preserves Copilot context sections
4. Maintains technical continuity markers

#### **Manual Status Updates**

```bash
# Force STATUS.md update with feature integration
.venv/bin/python scripts/auto_update_status.py --source build --verbose

# Preview what Copilot will see
.venv/bin/python scripts/auto_update_status.py --dry-run --verbose
```

### **Feature-Status Synchronization Points**

#### **Daily Development Routine**

1. **Morning**: Check feature status and blockers
2. **During Work**: Update feature progress as work proceeds
3. **Evening**: Complete feature updates and STATUS.md sync
4. **Commit**: Include feature references in commit messages

#### **Weekly Feature Review**

```bash
# Weekly feature health check
.venv/bin/python scripts/feature_tracker.py validate
.venv/bin/python scripts/feature_tracker.py report
.venv/bin/python scripts/auto_update_status.py --force --verbose
```

## 🚨 **Critical Anti-Patterns to Avoid**

### **❌ DON'T: Work Without Feature Context**

```bash
# BAD: Working without feature tracking
git commit -m "fix motor control bug"
# Copilot has no context about what was completed
```

### **✅ DO: Always Include Feature Context**

```bash
# GOOD: Feature-aware development
.venv/bin/python scripts/feature_tracker.py update FTR-009 --status IN_PROGRESS
# ... do work ...
.venv/bin/python scripts/feature_tracker.py update FTR-009 --status COMPLETE
git commit -m "feat: complete motor efficiency optimization (FTR-009)"
```

### **❌ DON'T: Manual STATUS.md Updates Only**

```bash
# BAD: Updating STATUS.md without feature tracking
# STATUS.md shows progress but feature tracking is stale
```

### **✅ DO: Integrated Updates**

```bash
# GOOD: Update features, then STATUS.md automatically syncs
.venv/bin/python scripts/feature_tracker.py update FTR-XXX --status COMPLETE
.venv/bin/python scripts/auto_update_status.py --verbose
```

### **❌ DON'T: Assume Copilot Knows Current State**

```markdown
# BAD: Vague context

"Continue working on the motor control feature"
```

### **✅ DO: Provide Specific Feature Context**

```markdown
# GOOD: Explicit context

"Continue FTR-009 (Motor Efficiency Optimization) - currently IN_PROGRESS
Need to complete src/controllers/motor_efficiency.c implementation
Target: 15% efficiency improvement validation"
```

## 📊 **Workflow Validation**

### **Health Check Commands**

```powershell
# Verify workflow compliance
scripts/run_python_configurable.ps1 scripts/feature_tracker.py validate
scripts/run_python_configurable.ps1 scripts/auto_update_status.py --dry-run

# Check SSOT workflow configuration
python scripts/workflow_config.py show
python scripts/workflow_config.py validate

# Check STATUS.md-Feature alignment (cross-platform)
# Windows PowerShell
Get-Content STATUS.md | Select-String -Pattern "Current Work" -Context 5
scripts/run_python_configurable.ps1 scripts/feature_tracker.py list --status IN_PROGRESS

# Linux/macOS (if available)
# grep -A5 "Current Work" STATUS.md
# .venv/bin/python scripts/feature_tracker.py list --status IN_PROGRESS
```

### **SSOT Configuration Validation**

```powershell
# Validate SSOT workflow configuration integrity
python scripts/workflow_config.py validate

# Test configuration mode switching
python scripts/workflow_config.py mode debug
scripts/run_python_configurable.ps1 scripts/auto_update_status.py --verbose
python scripts/workflow_config.py mode minimal
scripts/run_python_configurable.ps1 scripts/auto_update_status.py --verbose
```

### **Quality Metrics**

- **Feature Tracking Coverage**: All active work has corresponding features
- **Status Synchronization**: STATUS.md reflects current feature state
- **Copilot Context**: Clear development continuity markers
- **Progress Visibility**: Regular feature status updates

## 🎯 **Success Indicators**

### **Workflow Working Correctly When:**

✅ Copilot immediately understands current development context  
✅ Feature status always reflects actual development state  
✅ STATUS.md provides accurate project snapshot  
✅ Development sessions have clear continuity  
✅ Progress tracking is automatic and reliable  
✅ No "lost context" between development sessions

### **Integration Success Metrics**

- **Context Consistency**: Feature tracking ↔ STATUS.md ↔ Copilot awareness
- **Development Velocity**: No time lost on context reconstruction
- **Progress Accuracy**: Real-time development state visibility
- **Conversation Continuity**: Seamless session handoffs with complete context

## 🔗 **Related Instructions**

- **feature-management.instructions.md**: Feature creation and management details
- **status-maintenance.instructions.md**: STATUS.md structure and content requirements
- **project-setup.instructions.md**: Workspace setup and automation tools
- **ssot-config.instructions.md**: Configuration management principles

## 📝 **Workflow Summary Checklist**

### **Before Starting Work:**

- [ ] Configure workflow complexity (`python scripts/workflow_config.py mode <mode>`)
- [ ] Check current feature status
- [ ] Select/create appropriate feature
- [ ] Update feature to IN_PROGRESS
- [ ] Verify no blocking dependencies

### **During Development:**

- [ ] Use SSOT-aware script execution (`scripts/run_python_configurable.ps1`)
- [ ] Update feature progress regularly
- [ ] Add implementation files as created
- [ ] Remove blockers as resolved
- [ ] Provide feature context to Copilot

### **After Completing Work:**

- [ ] Update feature status to COMPLETE
- [ ] Add final implementation details
- [ ] Sync STATUS.md with current state (`scripts/run_python_configurable.ps1 scripts/auto_update_status.py --verbose`)
- [ ] Include feature reference in commit
- [ ] Verify Copilot context accuracy

### **SSOT Configuration Options:**

- [ ] **Minimal Mode**: Fast execution for CI/CD or expert developers
- [ ] **Standard Mode**: Balanced validation for daily development
- [ ] **Thorough Mode**: Full validation for critical development phases
- [ ] **Debug Mode**: Maximum verbosity for troubleshooting and investigation

**Remember**: The goal is ensuring Copilot always has accurate, current context about development state through integrated feature tracking, STATUS.md maintenance, and configurable workflow complexity via SSOT principles.
