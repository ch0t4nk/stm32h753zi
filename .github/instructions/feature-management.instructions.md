---
applyTo: "features/feature_registry.json,scripts/feature_tracker.py,docs/FEATURE_TRACKING_SYSTEM.md"
description: "Feature creation, management, and tracking guidelines for STM32H753ZI stepper motor project"
---

# Feature Management Instructions

## Overview
This instruction file provides comprehensive guidance for creating, updating, and managing features in the STM32H753ZI stepper motor control project using the centralized feature tracking system.

## Feature Creation Guidelines

### **1. Feature Definition Requirements**

#### **Mandatory Fields**
Every feature must include:
- **Unique ID**: FTR-XXX format (sequential numbering)
- **Name**: Clear, descriptive feature name (50 characters max)
- **Status**: COMPLETE, IN_PROGRESS, or PLANNED
- **Phase**: Development phase (1.1, 1.2, 1.3, 1.4, 2.0, 3.0, etc.)
- **Priority**: CRITICAL, HIGH, MEDIUM, or LOW
- **Category**: One of predefined categories (see registry)
- **Description**: Detailed feature description (1-2 sentences)
- **Acceptance Criteria**: Measurable completion requirements

#### **Optional but Recommended Fields**
- **Implementation Files**: List of source files implementing the feature
- **Test Files**: Associated unit/integration tests
- **Documentation**: Related documentation files
- **Dependencies**: Features this feature depends on
- **Dependents**: Features that depend on this feature
- **Blockers**: External factors preventing implementation
- **Effort Estimates**: Estimated hours for completion

### **2. Feature ID Assignment**

#### **ID Format**: FTR-XXX
- **FTR**: Feature Request prefix
- **XXX**: Sequential 3-digit number (001, 002, 003, etc.)

#### **ID Assignment Process**
1. Check existing features: `.venv/bin/python scripts/feature_tracker.py list`
2. Use next sequential number
3. Reserve ID ranges for different phases if needed

#### **ID Examples**
```bash
FTR-001  # First feature
FTR-010  # Tenth feature
FTR-025  # Twenty-fifth feature
```

### **3. Feature Categories**

#### **Valid Categories** (must match registry definitions)
- **telemetry**: Real-time data collection and monitoring systems
- **motor_control**: Motor control algorithms and coordination
- **safety**: Safety systems and emergency procedures
- **communication**: Inter-system communication protocols
- **rtos**: Real-time operating system integration
- **testing**: Testing infrastructure and validation
- **optimization**: Performance optimization and efficiency
- **integration**: Hardware-software integration
- **documentation**: Documentation systems and tools

#### **Category Selection Rules**
- Choose **primary** category that best describes the feature
- If feature spans multiple categories, choose the most significant one
- Add cross-references in description if multi-category

### **4. Phase Assignment Guidelines**

#### **Phase Definitions**
- **1.1**: Foundation Architecture
- **1.2**: Core Implementation
- **1.3**: Advanced Features
- **1.4**: Hardware Integration
- **2.0**: FreeRTOS Task Migration
- **3.0**: Advanced Capabilities

#### **Phase Selection Criteria**
- **Current Development**: Use current active phase
- **Future Features**: Assign to appropriate future phase
- **Dependencies**: Must respect phase dependencies (can't implement 1.4 before 1.3)

### **5. Priority Guidelines**

#### **Priority Levels**
- **CRITICAL**: System cannot function without this feature
- **HIGH**: Important for core functionality
- **MEDIUM**: Valuable enhancement
- **LOW**: Nice-to-have improvement

#### **Priority Examples**
```json
"CRITICAL": "Safety system emergency stop"
"HIGH": "Motor control telemetry"
"MEDIUM": "Performance optimization"
"LOW": "Documentation improvements"
```

## Feature Creation Process

### **Step 1: Feature Planning**
```bash
# 1. Analyze requirements
# 2. Check existing features for overlap
.venv/bin/python scripts/feature_tracker.py list --category motor_control

# 3. Identify dependencies
.venv/bin/python scripts/feature_tracker.py dependencies FTR-XXX
```

### **Step 2: Feature Registration**
#### **Manual Registration** (Primary Method)
1. Edit `features/feature_registry.json`
2. Add new feature entry following template
3. Validate registry consistency
4. Update metadata

#### **Feature Template**
```json
"FTR-XXX": {
  "id": "FTR-XXX",
  "name": "Feature Name Here",
  "status": "PLANNED",
  "phase": "X.X",
  "priority": "MEDIUM",
  "category": "category_name",
  "description": "Detailed description of what this feature accomplishes",
  "acceptance_criteria": [
    "Specific measurable requirement 1",
    "Specific measurable requirement 2",
    "Specific measurable requirement 3"
  ],
  "implementation_files": [],
  "test_files": [],
  "documentation": [],
  "dependencies": [],
  "dependents": [],
  "blockers": [],
  "completion_date": null,
  "effort_estimate_hours": 0,
  "effort_actual_hours": 0,
  "commits": []
}
```

### **Step 3: Validation**
```bash
# Validate registry consistency
.venv/bin/python scripts/feature_tracker.py validate

# Check feature details
.venv/bin/python scripts/feature_tracker.py show FTR-XXX
```

## Feature Update Guidelines

### **Status Updates**

#### **Status Transitions**
- **PLANNED → IN_PROGRESS**: When starting implementation
- **IN_PROGRESS → COMPLETE**: When all acceptance criteria met
- **Any Status → PLANNED**: When requirements change significantly

#### **Status Update Process**
```bash
# Update status
.venv/bin/python scripts/feature_tracker.py update FTR-XXX --status IN_PROGRESS

# Update effort
.venv/bin/python scripts/feature_tracker.py update FTR-XXX --effort-actual 8
```

### **Completion Requirements**

#### **Before Marking COMPLETE**
- [ ] All acceptance criteria met
- [ ] Implementation files completed
- [ ] Unit tests written and passing
- [ ] Documentation updated
- [ ] Integration tests passing (if applicable)
- [ ] Code review completed
- [ ] No known blockers remaining

#### **Completion Process**
1. Verify all acceptance criteria
2. Update status to COMPLETE
3. Set completion_date
4. Update effort_actual_hours
5. Add relevant commit references

### **Dependency Management**

#### **Adding Dependencies**
```json
"dependencies": ["FTR-001", "FTR-004"]  // Features this depends on
"dependents": ["FTR-008", "FTR-012"]   // Features that depend on this
```

#### **Dependency Rules**
- Dependencies must exist in registry
- No circular dependencies allowed
- Dependencies should be from same or earlier phases
- Update both directions (dependency and dependent)

### **Blocker Management**

#### **Types of Blockers**
- **Technical**: Code dependencies not ready
- **Hardware**: Physical hardware not available
- **External**: Third-party dependencies
- **Resource**: Personnel or time constraints

#### **Blocker Format**
```json
"blockers": [
  "AS5600 encoder procurement",
  "STM32H753ZI hardware setup",
  "Phase 1.3 completion"
]
```

## Integration with Development Workflow

### **Git Integration**

#### **Commit Message Format**
```bash
git commit -m "feat(FTR-XXX): Brief description of changes

- Implementation details
- Related to Feature: FTR-XXX Feature Name"
```

#### **Branch Naming** (if using feature branches)
```bash
git checkout -b feature/FTR-XXX-brief-description
```

### **Documentation Integration**

#### **File References**
```json
"implementation_files": [
  "src/telemetry/optimization_telemetry.h",
  "src/telemetry/optimization_telemetry.c"
],
"test_files": [
  "tests/unit/test_optimization_telemetry.c"
],
"documentation": [
  "docs/PHASE_1_3_COMPLETE.md",
  ".github/instructions/telemetry.instructions.md"
]
```

### **STATUS.md Integration**

#### **Automatic Updates**
Feature tracking is integrated with STATUS.md automation:
- Feature summaries included in status updates
- Progress tracking by phase and category
- Blocker identification in status reports

#### **Manual Feature Status**
```bash
# Get current feature summary
.venv/bin/python scripts/feature_status_summary.py

# Generate comprehensive report
.venv/bin/python scripts/feature_tracker.py report
```

## Quality Assurance

### **Registry Validation**

#### **Automatic Checks**
```bash
# Run all validations
.venv/bin/python scripts/feature_tracker.py validate

# Expected output: "✅ Registry validation passed"
```

#### **Validation Rules**
- All feature IDs are unique
- All dependencies reference existing features
- No circular dependencies
- All required fields present
- Valid category and phase values
- Consistent metadata counts

### **Best Practices**

#### **Feature Naming**
- Use clear, descriptive names
- Avoid technical jargon unless necessary
- Keep under 50 characters
- Use action verbs when appropriate

#### **Acceptance Criteria**
- Make criteria measurable and testable
- Include performance requirements
- Specify integration requirements
- Include success metrics

#### **Effort Estimation**
- Break large features into smaller ones if >40 hours
- Use historical data for estimation
- Include time for testing and documentation
- Account for integration overhead

## Common Anti-Patterns to Avoid

### **❌ Don't Do This**

#### **Vague Descriptions**
```json
"description": "Improve motor control"  // Too vague
```

#### **Missing Acceptance Criteria**
```json
"acceptance_criteria": []  // No measurable requirements
```

#### **Incorrect Dependencies**
```json
"dependencies": ["FTR-999"]  // Non-existent feature
```

#### **Inconsistent Status**
```json
"status": "COMPLETE",
"completion_date": null  // Inconsistent data
```

### **✅ Do This Instead**

#### **Clear Descriptions**
```json
"description": "Real-time telemetry collection infrastructure for performance optimization with 1kHz data collection capability"
```

#### **Measurable Criteria**
```json
"acceptance_criteria": [
  "Telemetry data collection at 1kHz frequency",
  "Integration with FreeRTOS scheduler",
  "Memory-efficient data structures",
  "Unit test coverage >90%"
]
```

#### **Valid Dependencies**
```json
"dependencies": ["FTR-001", "FTR-004"],  // Existing features
"dependents": ["FTR-008"]
```

## Future Enhancements

### **Planned Improvements**
1. **Automated Feature Creation**: CLI command for new features
2. **Dependency Visualization**: Generate dependency graphs
3. **Effort Analysis**: Historical effort tracking and prediction
4. **Git Hook Integration**: Automatic status updates from commits
5. **Progress Dashboards**: Visual progress tracking

### **Extension Points**
- Additional categories as project grows
- Custom validation rules
- Integration with external tools
- Automated testing workflows

## Related Instructions
- **project-setup.instructions.md**: Overall project setup and build system
- **status-maintenance.instructions.md**: STATUS.md maintenance and Copilot context
- **ssot-config.instructions.md**: SSOT configuration management principles

Remember: **Structured feature tracking enables systematic development, clear progress visibility, and maintainable project management.**
