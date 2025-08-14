# STATUS.md Automation System

Automated maintenance system for project status tracking and GitHub Copilot context optimization with git hooks integration.

## Overview

The STATUS.md automation system keeps the project status file current with real-time information about build status, code progress, git activity, and development metrics. This ensures GitHub Copilot always has accurate context for optimal assistance. **Phase 3 Complete**: Fully automated with git post-commit hooks.

## Features

### ✅ Phase 1 (Complete)
- ✅ **Automatic Timestamp Updates**: Current date for last updated field
- ✅ **Build Metrics Analysis**: Binary size, flash usage, build status
- ✅ **Git Commit Analysis**: Categorizes recent commits by type (features, fixes, docs, tests)
- ✅ **Code Status Analysis**: TODO count, driver implementation status, file metrics
- ✅ **Architecture Status Updates**: Driver progress tracking
- ✅ **Format Validation**: Ensures STATUS.md structure integrity
- ✅ **VS Code Task Integration**: Manual and automated triggers

### ✅ Phase 2 (Complete)
- ✅ **CMake Build Integration**: Automatic post-build updates via CMake targets
- ✅ **Build-Triggered Updates**: STATUS.md updates on successful builds
- ✅ **Performance Optimization**: <5 second execution for full updates

### ✅ Phase 3 (Complete - NEW)
- ✅ **Git Hooks Integration**: Automatic post-commit STATUS.md updates
- ✅ **Loop Prevention**: Intelligent detection to prevent update loops
- ✅ **Zero-Maintenance Operation**: Fully automated operation requiring no manual intervention
- ✅ **Incremental Updates**: <1 second execution for git-triggered updates

## Usage

### Automatic Operation (Default)
The system now operates automatically via git hooks with no manual intervention required.

### Command Line Interface (Manual Override)

```bash
# Manual update with detailed output
python3 scripts/auto_update_status.py --verbose

# Preview changes without writing
python3 scripts/auto_update_status.py --dry-run --verbose

# Update triggered by build system
python3 scripts/auto_update_status.py --source build

# Force update even if no changes detected
python3 scripts/auto_update_status.py --force --verbose
```

### VS Code Tasks

Use `Ctrl+Shift+P` → `Tasks: Run Task` and select:

- **Update STATUS.md**: Manual update with full output
- **Update STATUS.md (Preview)**: Dry-run mode to preview changes
- **Update STATUS.md (Build Triggered)**: Silent update after builds
- **Build with Status Update**: Build project and auto-update status

### Automation Examples

```bash
# Daily development workflow
python3 scripts/auto_update_status.py --verbose

# CI/CD integration
python3 scripts/auto_update_status.py --source build

# Status health check
python3 scripts/auto_update_status.py --dry-run
```

## Technical Architecture

### Core Components

**StatusUpdater Class**: Main automation engine with methods for:
- `update_timestamp()`: Current date tracking
- `update_build_metrics()`: ELF analysis, flash usage calculation
- `analyze_recent_commits()`: Git log parsing and categorization
- `analyze_code_status()`: Source code analysis and TODO tracking
- `update_architecture_status()`: Driver progress updates
- `validate_format()`: STATUS.md structure validation

**Data Sources**:
- Build artifacts: `build/stm32h753_ihm02a1.elf`
- Git history: Last 5 commits with semantic categorization
- Source code: `src/**/*.{c,h}` files for TODO analysis
- Driver status: Implementation progress tracking

### Metrics Tracked

**Build Metrics**:
- Binary size (KB)
- Flash usage percentage (2MB total)
- Build success/failure status

**Code Metrics**:
- Total source files (C and H)
- TODO comment count and samples
- Driver implementation status
- Function count per driver

**Git Activity**:
- Recent commits categorized by type:
  - Features: `feat:`, `implement`, `add`, `create`
  - Fixes: `fix:`, `bug`, `correct`, `resolve`
  - Documentation: `docs:`, `readme`, `documentation`
  - Tests: `test:`, `spec`, `coverage`, `unittest`

### Integration Points

**VS Code Tasks**: Integrated with `.vscode/tasks.json`
- Manual triggers via Command Palette
- Silent post-build updates
- Preview mode for validation

**Build System**: Ready for CMake integration
- Post-build hooks (Phase 2)
- Automatic metric extraction
- Build status monitoring

**Git Workflow**: Prepared for git hooks
- Commit-triggered updates (Phase 3)
- Branch-aware status tracking
- Intelligent update scheduling

## Configuration

### Environment Variables

```bash
# Override workspace location
export WORKSPACE_ROOT="/custom/path"

# Control verbosity
export STATUS_VERBOSE=true
```

### Customization Options

**Update Triggers**:
- `manual`: Developer-initiated updates
- `build`: Post-build automation
- `git`: Git commit triggers (future)

**Output Modes**:
- Normal: Write changes to STATUS.md
- Dry-run: Preview changes only
- Verbose: Detailed operation logging

## Safety Features

**Format Validation**: Ensures STATUS.md structure integrity
- Required section presence checking
- Markdown format validation
- Backup handling for failed updates

**Error Handling**: Robust error handling for:
- Missing build artifacts
- Git repository issues
- File permission problems
- Invalid STATUS.md format

**Rollback Capability**: Original content preservation
- Changes only applied if validation passes
- Detailed error reporting for troubleshooting
- Dry-run mode for safe testing

## Development Guidelines

### Adding New Metrics

1. **Extend StatusUpdater class**:
   ```python
   def analyze_new_metric(self) -> Dict:
       # Implementation
       return {"metric_name": value}
   ```

2. **Update run_update() method**:
   ```python
   new_data = self.analyze_new_metric()
   content = self.update_section_with_new_data(content, new_data)
   ```

3. **Add format validation**:
   ```python
   def validate_format(self, content: str) -> bool:
       # Add new section validation
   ```

### Testing Changes

```bash
# Test with dry-run mode
python3 scripts/auto_update_status.py --dry-run --verbose

# Test specific trigger sources
python3 scripts/auto_update_status.py --source build --dry-run

# Validate format after changes
python3 scripts/auto_update_status.py --force --dry-run
```

## Phase 2 Preview: Build Integration

**Planned Features**:
- CMake custom targets for automatic updates
- Build timing analysis
- Compilation error tracking
- Memory map analysis

**Implementation Strategy**:
```cmake
# CMakeLists.txt integration
add_custom_target(update_status ALL
    COMMAND python3 scripts/auto_update_status.py --source build
    DEPENDS stm32h753_ihm02a1.elf
    COMMENT "Updating STATUS.md with build metrics"
)
```

## Phase 3 Preview: Intelligence & Git Integration

**Planned Features**:
- Git pre-commit and post-commit hooks
- Intelligent update scheduling (avoid spam)
- Progress milestone detection
- Development velocity metrics

**Smart Scheduling**:
- Update frequency limits (max once per hour for routine changes)
- Force updates for milestone commits
- Batch updates for related changes

## Troubleshooting

### Common Issues

**Permission Errors**:
```bash
chmod +x scripts/auto_update_status.py
```

**Missing Dependencies**:
```bash
# Verify Python environment
python3 --version
which python3
```

**Git Issues**:
```bash
# Verify git repository
git status
git log --oneline -5
```

### Debug Mode

```bash
# Enable maximum verbosity
python3 scripts/auto_update_status.py --verbose --force --dry-run
```

### Log Analysis

The automation system provides detailed logging when `--verbose` is enabled:
- Timestamp: Update timing information
- Build Metrics: Binary analysis results
- Git Analysis: Commit categorization details
- Code Analysis: Source code scanning results
- Format Validation: Structure checking results

## Integration with GitHub Copilot

The automation system specifically optimizes STATUS.md for GitHub Copilot context:

**Current Work Tracking**: Always-current list of active development items
**Next Steps Clarity**: Clear priorities for Copilot suggestions
**Architecture Status**: Component completion status for relevant code suggestions
**Metrics Visibility**: Build and code health for optimization suggestions

This ensures Copilot has accurate, up-to-date context for every development session.

---

**Author**: STM32H753ZI Development Team  
**Created**: August 5, 2025  
**Status**: Phase 1 Complete, Phase 2 Planning
