---
applyTo: "STATUS.md"
description: "STATUS.md maintenance rules, format requirements, and context preservation for STM32H753ZI stepper motor project"
---

# STATUS.md Maintenance Instructions

## Overview

This instruction file provides comprehensive guidance for maintaining STATUS.md as the authoritative project status document and primary context source for GitHub Copilot conversation continuity. The STATUS.md file serves as both a project dashboard and technical handoff document.

## Critical PURPOSE of STATUS.md

### Primary Functions

1. **Project Dashboard**: Current development state, build health, phase progress
2. **Copilot Context Source**: Technical state for conversation continuity
3. **Handoff Document**: Seamless development context transfer between sessions
4. **Progress Tracking**: Phase completion, metrics, and next priorities
5. **Build System Status**: Dual-build health, performance metrics, validation results

### Target Audiences

- **GitHub Copilot**: Primary consumer for conversation context and development continuity
- **Development Team**: Quick project status and progress overview
- **Future Sessions**: Technical state preservation for "continuing where we left off"
- **Stakeholders**: High-level progress and capability summary

## MANDATORY STATUS.md Structure

### Required Sections (MUST MAINTAIN)

```markdown
# STM32H753ZI Project Status

**Last Updated**: [AUTO-GENERATED DATE]
**Status**: [CURRENT_STATE] - [BRIEF_DESCRIPTION]
**Deployment**: [DEPLOYMENT_READINESS]
**AI Infrastructure**: [SEMANTIC_SEARCH_STATUS]

## 🔧 Setup Summary

[BUILD_COMMANDS_AND_CAPABILITIES]

## ✅ Progress So Far

[COMPLETED_PHASES_AND_ACHIEVEMENTS]

## 🔄 Current Work

[ACTIVE_DEVELOPMENT_STATE]

## 🔄 Current Technical Context for Copilot Continuation

[COMPREHENSIVE_TECHNICAL_STATE]

## ⏭️ Next Steps

[PRIORITY_ROADMAP]

## 🧠 Notes & Observations

[TECHNICAL_INSIGHTS_AND_CONTEXT]

## 📊 Key Metrics

[PERFORMANCE_AND_BUILD_METRICS]

## 🎯 Architecture Status

[SYSTEM_COMPONENT_STATUS]

## 🔗 Quick References

[ESSENTIAL_COMMANDS_AND_DOCUMENTATION]

## 🤖 Enhanced Status Auto-Update Configuration

[AUTO_UPDATE_ENHANCEMENT_GUIDANCE]
```

## Content Requirements and Rules

### 1. Technical Context for Copilot Continuation Section

**CRITICAL**: This section MUST contain comprehensive technical state information for seamless conversation handoff.

**Required Subsections**:

```markdown
**Phase Status**: ✅ **Phase X.X COMPLETE** - [Description]

- **Phase 1.1**: [Status] - [Brief description with key files]
- **Phase 1.2**: [Status] - [Brief description with key files]
- **Phase 1.3**: [Status] - [Brief description with key files]
- **Phase 1.4**: [Status] - [Brief description with key files]

**Active Development State**:

- **Recent Work**: [Last major session focus and achievements]
- **Test Results**: [Current test status with pass/fail counts]
- **Build System**: [Dual-build status and health]
- **Validation Framework**: [Testing infrastructure status]
- **Performance Metrics**: [Current performance against targets]

**Technical Architecture Decisions**:

- **Dual-Build System**: [ARM vs host compilation separation status]
- **HAL Abstraction**: [Platform-independent design status]
- **SSOT Configuration**: [Configuration centralization status]
- **FreeRTOS Integration**: [Real-time OS status and performance]
- **Safety Framework**: [Safety system status and response times]

**Key Implementation Files**:

- **Telemetry**: [File paths and status]
- **Characterization**: [File paths and status]
- **Dual-Build**: [File paths and status]
- **Validation**: [File paths and status]
- **SSOT Config**: [File paths and status]

**Performance Validation Results**:
[TABLE OF METRICS WITH TARGETS AND ACTUAL VALUES]

**Build System Status**:
[ARM FIRMWARE AND HOST TESTING BUILD HEALTH]

**Next Development Priorities**:
[NUMBERED LIST OF IMMEDIATE NEXT STEPS]

**Conversation Continuity Markers**:

- **Last Completed**: [Most recent major achievement]
- **Current State**: [Technical readiness summary]
- **Development Flow**: [Process adherence status]
- **Build Health**: [Build system operational status]
- **Test Coverage**: [Validation and testing status]
```

### 2. Status Indicators and Formatting

**Status Emoji Standards**:

- ✅ **COMPLETE**: Fully implemented and validated
- 🚀 **READY**: Prepared for next phase
- 🔄 **IN_PROGRESS**: Actively being developed
- ⏸️ **PAUSED**: Temporarily on hold
- ❌ **BLOCKED**: Cannot proceed due to dependencies
- 🧪 **TESTING**: Under validation
- 📝 **PLANNED**: Scheduled for future implementation

**Phase Status Format**:

```markdown
**Phase X.X**: [STATUS_EMOJI] **[STATUS_TEXT]** - [Description]
```

**Metrics Format**:

```markdown
**[Metric Name]**: [Value] ([Target]) [STATUS_EMOJI]
```

### 3. Build System Status Requirements

**MANDATORY Build Information**:

- **ARM Firmware Size**: [SIZE] / [TOTAL] ([PERCENTAGE] usage)
- **RAM Usage**: [SIZE] / [TOTAL] ([PERCENTAGE] utilized)
- **FreeRTOS Performance**: [Scheduler details and timing]
- **Control Performance**: [Loop timing and response times]
- **Dual-Build Health**: [ARM vs host build separation status]

**Build Command Format**:

```powershell
# Windows PowerShell (recommended - uses repo .venv)
# Prepare/activate venv (optional) and run helper script for cross-platform consistency
.venv\Scripts\Activate.ps1                     # (optional) activate virtualenv in current session
.\scripts\run_python.ps1 scripts\fix_cmake.ps1           # ARM firmware build (uses venv Python)
.\scripts\run_python.ps1 scripts\run_host_tests.ps1     # Host testing validation (uses venv Python)
.\scripts\run_python.ps1 scripts\auto_update_status.py --verbose  # STATUS.md refresh (recommended)

# Direct PowerShell invocation without helper (explicit)
& 'C:\repos\Nucleo-H753ZI Project\code\.venv\Scripts\python.exe' scripts\auto_update_status.py --verbose
```

### 4. Auto-Update Integration Rules

**Automatic Update Triggers**:

- Build completion (via build system)
- Git commit (via git hooks)
- Manual execution (via scripts)
- Development milestone completion

**Preserved Manual Content**:

- Technical Context for Copilot Continuation section
- Architecture status details
- Next steps priorities
- Conversation continuity markers

**Auto-Generated Content**:

- Timestamps
- Build metrics
- File counts
- Git commit analysis
- Performance measurements

## Content Writing Guidelines

### Technical Accuracy Requirements

- **Specific File References**: Always include exact file paths
- **Quantified Metrics**: Include numbers, percentages, timing data
- **Status Evidence**: Link status claims to verification methods
- **Version Information**: Include firmware sizes, dates, commit references

### Copilot Context Optimization

- **Session Handoff**: Include "where we left off" information
- **Technical State**: Comprehensive architecture and implementation status
- **Next Actions**: Clear prioritized list of immediate next steps
- **Context Markers**: Identifiable technical state indicators

### Language and Tone Requirements

- **Professional Technical**: Engineering documentation style
- **Action-Oriented**: Focus on what's done, what's next
- **Quantitative**: Include measurements and metrics
- **Unambiguous**: Clear status indicators and descriptions

## Update Process Rules

### When to Update STATUS.md

1. **After Major Phase Completion**: Phase 1.1, 1.2, 1.3, 1.4, etc.
2. **After Build System Changes**: CMake updates, toolchain changes
3. **After Test Suite Updates**: New validation tests, framework changes
4. **After Performance Validation**: Metrics verification, benchmark updates
5. **Before Development Session End**: Ensure handoff context is current
6. **After Architecture Changes**: Dual-build, SSOT, safety system updates

### Update Validation Requirements

After any STATUS.md update, verify:

1. **All required sections present** and properly formatted
2. **Technical context comprehensive** for Copilot handoff
3. **Status indicators accurate** and evidence-based
4. **Build commands functional** and up-to-date
5. **File references valid** and paths correct
6. **Metrics current** and properly formatted

## CRITICAL Anti-Patterns to Avoid

### Content Anti-Patterns

❌ **Vague Status**: "Working on motor control" → ✅ **Specific Status**: "Phase 1.3 validation: 10/10 tests passing"
❌ **Missing Context**: No technical handoff information → ✅ **Comprehensive Context**: Full technical state for continuation
❌ **Outdated Metrics**: Old build sizes, performance data → ✅ **Current Metrics**: Latest validation results and measurements
❌ **Broken Commands**: Non-functional build instructions → ✅ **Verified Commands**: Tested and working procedures
❌ **Generic Descriptions**: Basic feature lists → ✅ **Technical Details**: Implementation status and file references

### Structure Anti-Patterns

❌ **Missing Sections**: Incomplete required structure → ✅ **Complete Structure**: All mandatory sections present
❌ **Inconsistent Format**: Mixed status indicators → ✅ **Consistent Format**: Standard emoji and text patterns
❌ **Poor Organization**: Random information order → ✅ **Logical Flow**: Structured information hierarchy
❌ **Context Gaps**: Missing handoff information → ✅ **Seamless Handoff**: Complete technical state preservation

## Validation Checklist

### Pre-Update Checklist

- [ ] Current phase status accurately determined
- [ ] Build system health verified
- [ ] Test results current and accurate
- [ ] Performance metrics up-to-date
- [ ] File references validated

### Post-Update Checklist

- [ ] All mandatory sections present
- [ ] Technical context comprehensive for Copilot
- [ ] Status indicators consistent and evidence-based
- [ ] Build commands tested and functional
- [ ] Conversation continuity information complete
- [ ] Next steps clearly prioritized
- [ ] Metrics formatted correctly

### Copilot Context Validation

- [ ] "Where we left off" clearly documented
- [ ] Technical architecture status comprehensive
- [ ] Implementation file status current
- [ ] Performance validation results included
- [ ] Next development priorities specific
- [ ] Build system status detailed

## Integration with Other Systems

### Git Hook Integration

STATUS.md should be automatically updated via git hooks for:

- Post-commit updates (build metrics, commit analysis)
- Pre-push validation (ensure context is current)
- Branch changes (development state tracking)

### Build System Integration

STATUS.md should reflect build system state:

- ARM firmware compilation success/failure
- Host testing validation results
- Dual-build system health
- Memory usage and optimization metrics

### Testing Framework Integration

STATUS.md should include testing status:

- Validation test pass/fail counts
- Performance benchmark results
- Safety system test outcomes
- Integration test readiness

## Example High-Quality STATUS.md Entry

```markdown
**Phase Status**: ✅ **Phase 1.3 COMPLETE** - Dynamic Motor Tuning validation completed successfully

- **Phase 1.1**: Telemetry Infrastructure - COMPLETE ✅ (optimization_telemetry.h/c with <500µs execution)
- **Phase 1.2**: Motor Characterization Framework - COMPLETE ✅ (motor_characterization.h/c with L6470 optimization)
- **Phase 1.3**: Validation Testing - COMPLETE ✅ (10/10 tests passing, dual-build system validated)
- **Phase 1.4**: Ready for Integration Testing 🚀 (hardware-in-the-loop validation)

**Active Development State**:

- **Recent Work**: Phase 1.3 validation testing with dual-build system compliance
- **Test Results**: 10/10 Phase 1.3 validation tests passing (test_phase1_3_validation.c)
- **Build System**: Dual-build architecture working (ARM firmware: build/, Host testing: build_host_tests/)
- **Validation Framework**: CTest integration successful with Unity testing framework
- **Performance Metrics**: All targets met (Sub-500µs telemetry, <2% CPU overhead, 1kHz sample rate)

**Performance Validation Results**:

- **Sample Collection**: 380µs (target: <500µs) ✅
- **CPU Overhead**: 1.2% (target: <2%) ✅
- **Memory Usage**: 24KB (target: <32KB) ✅
- **Sample Rate**: 1050Hz (target: ≥1000Hz) ✅
- **Time Constant Error**: 7.3% (target: ≤10%) ✅
```

Remember: **STATUS.md is the primary context preservation mechanism for GitHub Copilot conversation continuity - comprehensive technical state documentation is essential for seamless development handoff.**
