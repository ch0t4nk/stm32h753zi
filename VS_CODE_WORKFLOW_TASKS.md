# 🚀 VS Code Workflow Tasks - Quick Reference

## **Two Ways to Execute Workflow**

### **Method 1: VS Code Tasks (Recommended for VS Code users)**
`Ctrl+Shift+P` → "Tasks: Run Task" → Select workflow task:

#### **🚀 Development Workflow Tasks**
- **🚀 WORKFLOW: Before Work** - Start session (environment check, feature status)
- **🔄 WORKFLOW: During Work** - Development cycle (build + status update)  
- **✅ WORKFLOW: After Work** - End session (validation + commit prep)

#### **📊 Status & Feature Tasks**
- **📋 WORKFLOW: Check Feature Status** - Show in-progress features
- **📊 WORKFLOW: Project Report** - Complete project overview

### **Method 2: Command Line (More flexible)**
```bash
# Development workflow phases
./scripts/workflow.sh before
./scripts/workflow.sh during --feature-id FTR-009 --notes "Working on optimization"
./scripts/workflow.sh after --feature-id FTR-009 --notes "Implementation complete"

# Additional options
./scripts/workflow.sh before --dry-run          # Preview mode
./scripts/workflow.sh --help                    # Show all options
```

## **📋 Daily Development Pattern**

### **🌅 Start of Development Session**
**VS Code**: `Ctrl+Shift+P` → "Tasks: Run Task" → "🚀 WORKFLOW: Before Work"
**CLI**: `./scripts/workflow.sh before`

**What it does**:
- ✅ Validates ARM GCC toolchain
- ✅ Updates STATUS.md with current state
- ✅ Shows project completion status (8/12 features, 66.7%)
- ✅ Lists active work (FTR-009: Motor Efficiency Optimization)
- ✅ Shows next planned features

### **⚙️ During Development Work**
**VS Code**: `Ctrl+Shift+P` → "Tasks: Run Task" → "🔄 WORKFLOW: During Work"
**CLI**: `./scripts/workflow.sh during --feature-id FTR-009`

**What it does**:
- 🔄 Updates feature status to IN_PROGRESS
- 🔨 Builds project with ARM GCC
- 📊 Updates STATUS.md with build metrics
- ✅ Validates all systems working

### **🏁 End of Development Session**
**VS Code**: `Ctrl+Shift+P` → "Tasks: Run Task" → "✅ WORKFLOW: After Work"
**CLI**: `./scripts/workflow.sh after --feature-id FTR-009 --notes "Optimization complete"`

**What it does**:
- ✅ Marks feature as COMPLETE
- 🔨 Final build validation
- 📊 Updates STATUS.md with final state
- 📋 Shows git status for commit
- 💡 Suggests commit message format

## **🎯 Key Benefits**

### **For You:**
- **No more lost context** between development sessions
- **Systematic progress tracking** with feature management
- **Automated STATUS.md updates** keeping documentation current
- **Consistent workflow** preventing mistakes

### **For Copilot:**
- **Always current context** - knows exactly what features are active/complete
- **Accurate project state** - STATUS.md reflects reality
- **Feature tracking integration** - understands development progress
- **No context drift** - every session starts with updated information

## **🔄 Workflow Integration with Existing Tasks**

Your existing tasks still work exactly the same:
- **Build (CMake)** - `Ctrl+Shift+P` → "Tasks: Run Task" → "Build (CMake)"
- **STATUS: Quick Update** - Still available for manual updates
- **All other tasks** - Unchanged and fully compatible

The workflow tasks **enhance** your existing setup by providing:
- **Structured phases** for systematic development
- **Feature tracking integration** for context preservation
- **Automated workflows** reducing manual steps

## **💡 Pro Tips**

### **Starting Work on New Feature**
1. Run "🚀 WORKFLOW: Before Work" to see current state
2. Check if target feature is PLANNED or blocked
3. Use `python scripts/feature_tracker.py show FTR-XXX` for details
4. Start with "🔄 WORKFLOW: During Work"

### **Finishing Feature**
1. Run "✅ WORKFLOW: After Work" for validation
2. Follow suggested commit message format
3. Git hooks will automatically update STATUS.md after commit
4. Next session will reflect completed feature

### **Quick Status Check**
Use "📋 WORKFLOW: Check Feature Status" or "📊 WORKFLOW: Project Report" anytime to understand current project state.

---

**🎯 This workflow ensures Copilot always has accurate, current context about your development progress!**
