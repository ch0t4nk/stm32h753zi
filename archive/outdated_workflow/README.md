# Outdated Workflow Files Archive

This directory contains workflow-related files that have been superseded by the new SSOT Configuration System (August 2025).

## Archived Files

### PowerShell Modules (Superseded)
- **STM32Workflow_original.psm1**: Original PowerShell workflow module with complex initialization patterns
  - **Replacement**: SSOT configuration system with selective complexity control
  - **Migration Path**: Use `scripts/workflow_config.py` for configuration management
  - **New System**: `scripts/run_python_configurable.ps1` with mode-aware execution

### Instructions (Outdated)
- **phase3-advanced.instructions.md**: Empty placeholder file for advanced phase 3 features
  - **Status**: Never implemented, superseded by comprehensive instruction system
  - **Replacement**: Domain-based instruction files in `.github/instructions/`

## New SSOT Workflow System (August 2025)

The workflow system has been completely redesigned with SSOT configuration principles:

### Core Components
- **SSOT Header**: `src/config/workflow_config.h` - Centralized configuration flags
- **Python Manager**: `scripts/workflow_config.py` - Mode switching and settings management  
- **PowerShell Integration**: `scripts/run_python_configurable.ps1` - Cross-platform execution
- **Configuration File**: `.workflow_config` - Runtime settings storage

### Workflow Modes Available
- **minimal**: Fast execution, no validation (CI/CD, experienced developers)
- **standard**: Balanced validation, good performance (daily development)
- **thorough**: Full validation, comprehensive checks (critical development)
- **debug**: Maximum verbosity, all diagnostics (troubleshooting)

### Migration Guide

#### Old System
```powershell
# Complex initialization with fixed validation
Import-Module .\scripts\STM32Workflow.psm1
workflow-before  # Always full validation
```

#### New System
```powershell
# Configurable complexity with SSOT control
python scripts/workflow_config.py mode minimal  # Fast mode
python scripts/workflow_config.py mode debug    # Full validation
scripts/run_python_configurable.ps1 scripts/auto_update_status.py --verbose
```

#### Benefits of New System
✅ **Selective Complexity**: Enable/disable any workflow pattern individually  
✅ **User Experience**: New users get thorough guidance, experts get speed  
✅ **SSOT Compliance**: All configuration centralized in one header  
✅ **Cross-Platform**: Works identically on Windows/Linux/macOS  
✅ **Zero Breaking Changes**: All existing workflows continue to work  
✅ **Intelligent Defaults**: Modes provide sensible preset combinations  

## Related Documentation
- **Main Instructions**: `.github/instructions/development-workflow.instructions.md`
- **SSOT Configuration**: `.github/instructions/ssot-config.instructions.md`
- **Feature Management**: `.github/instructions/feature-management.instructions.md`

---
**Archive Date**: August 14, 2025  
**Superseded By**: SSOT Configuration System with selective workflow complexity control
