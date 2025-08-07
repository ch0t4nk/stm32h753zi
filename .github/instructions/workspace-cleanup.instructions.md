---
applyTo: "**/*.{md,py,sh,json,yaml,yml}"
description: "Workspace cleanup and README update methodology for STM32H753ZI stepper motor project"
---

# Workspace Cleanup and Documentation Update Instructions

## Overview
This instruction file provides the verified methodology for systematically cleaning up development workspaces and updating documentation to reflect production-ready status. Based on successful implementation during STM32H753ZI project completion.

## Cleanup Principles

### 1. **Preservation Before Removal**
- **Never delete development history** - Archive instead of removing
- **Maintain accessibility** - Archived materials should be findable and documented
- **Document rationale** - Each archive decision should have clear justification

### 2. **Production Focus**
- **Keep essential tools only** - Remove development-specific utilities
- **Update status messaging** - Shift from "development" to "production ready"
- **Simplify navigation** - Reduce cognitive load for new users

### 3. **Reference Integrity**
- **Update all links** - Search and update references to moved files
- **Validate functionality** - Ensure production systems still work after cleanup
- **Test workflows** - Verify essential commands and processes

## Systematic Cleanup Methodology

### Phase 1: Assessment and Planning

#### 1.1 Identify Archive Candidates
Use systematic evaluation to categorize workspace assets:

```bash
# Scan for development-specific files
find . -name "*_SUMMARY.md" -o -name "*_STATUS.md" -o -name "PHASE_*.md"
find . -name "*_EVALUATION.md" -o -name "*_ASSESSMENT.md"
find scripts/ -name "*.py" -o -name "*.sh" | head -20

# Check for scattered documentation
find . -name "README.md" -exec grep -l "development\|TODO\|WIP" {} \;
```

#### 1.2 Analyze Production Dependencies
**Critical Step**: Identify what must remain for production:

```bash
# Check VSCode task references
grep -r "scripts/" .vscode/
grep -r "scripts/" CMakeLists.txt cmake/

# Check documentation references  
grep -r "scripts/" README.md docs/ .github/
grep -r "PHASE\|SUMMARY\|STATUS" README.md docs/

# Validate build system dependencies
./scripts/fix_cmake.sh --dry-run  # if available
```

#### 1.3 Create Archive Structure
**Organized Categories**: Group similar content logically

```bash
# Create organized archive structure
mkdir -p archive/{phase_reports,development_artifacts,outdated_docs,development_scripts}
mkdir -p archive/development_scripts/{cubemx_workflow,code_generation,documentation_processing,testing_tools,project_bootstrap}
```

### Phase 2: Systematic Archival

#### 2.1 Archive by Category
**Phase Reports and Completion Documents:**
```bash
# Move phase completion reports
git mv PHASE_*_COMPLETE.md archive/phase_reports/
git mv *_MILESTONE_*.md archive/phase_reports/
git mv milestone_reports/ archive/phase_reports/ 2>/dev/null || true
```

**Development Artifacts:**
```bash
# Move development summaries and evaluations
git mv *_EVALUATION_SUMMARY.md archive/development_artifacts/
git mv *_READINESS_ASSESSMENT.md archive/development_artifacts/
git mv analysis_reports/ archive/development_artifacts/ 2>/dev/null || true
```

**Outdated Documentation:**
```bash
# Move superseded setup and configuration docs
git mv *_SETUP.md archive/outdated_docs/
git mv TOOLCHAIN_*.md archive/outdated_docs/
git mv *_OPTIMIZATION.md archive/outdated_docs/
```

#### 2.2 Archive Development Scripts
**Critical**: Only archive scripts **not referenced** in production systems:

```bash
# Archive development-phase scripts (VERIFY NO PRODUCTION REFERENCES FIRST)
git mv scripts/create_*.py archive/development_scripts/project_bootstrap/
git mv scripts/generate_*.py archive/development_scripts/code_generation/
git mv scripts/cubemx_*.py scripts/cubemx_*.script archive/development_scripts/cubemx_workflow/
git mv scripts/test_*.sh archive/development_scripts/testing_tools/
git mv scripts/*_docs.py scripts/process_*.py archive/development_scripts/documentation_processing/
```

#### 2.3 Consolidate Existing Archives
**Merge scattered archive directories:**
```bash
# Consolidate docs/archives into main archive structure
git mv docs/archives/milestone_reports archive/phase_reports/
git mv docs/archives/analysis_reports archive/development_artifacts/
git mv docs/archives/* archive/development_artifacts/
rmdir docs/archives  # Remove empty directory
```

### Phase 3: Documentation Updates

#### 3.1 Update Production Status in README Files
**Systematic README updates** to reflect completion:

**Root README.md:**
```markdown
# STM32H753ZI Stepper Motor Control System

**Status**: ✅ **PROJECT COMPLETE - PRODUCTION READY**  
**Deployment**: Ready for hardware validation and field deployment

## Production Capabilities
- Complete dual L6470 stepper motor control system
- AS5600 magnetic encoder closed-loop feedback  
- Safety-critical real-time performance (1kHz, <1ms emergency stop)
- Production-optimized firmware (18.7KB, 0.89% flash usage)

## Quick Start - Production Deployment
[Include deployment instructions, not development setup]
```

**Component README.md Updates:**
```bash
# Update all component README files
find . -name "README.md" -not -path "./archive/*" -exec sed -i 's/development phase/production ready/g' {} \;
find . -name "README.md" -not -path "./archive/*" -exec sed -i 's/work in progress/complete and validated/g' {} \;
```

#### 3.2 Update API Documentation
**Version and status updates:**
```yaml
# api/openapi.yaml
info:
  title: "STM32H753ZI Motor Control API"
  version: "1.0.0"  # Update from 0.x.x
  description: "Production-ready REST API for dual stepper motor control"
  x-status: "PRODUCTION_READY"

# api/can_api.yaml  
can_api:
  version: 1.0.0  # Update from 0.x.x
  title: "STM32H753ZI Motor Control CAN API" 
  description: "Production-ready CAN bus communication interface"
  status: "PRODUCTION READY"
```

#### 3.3 Archive Documentation
**Create comprehensive archive documentation:**

**archive/README.md:**
```markdown
# Archive Directory - [Project Name]

**Archive Date**: [Current Date]
**Project Status**: ✅ **COMPLETE - PRODUCTION READY**
**Reason**: Archiving outdated development artifacts after project completion

## Archive Organization
- `phase_reports/` - Development phase completion reports
- `development_artifacts/` - Analysis, summaries, and development work
- `outdated_docs/` - Superseded setup and configuration documentation
- `development_scripts/` - Development-phase automation tools

## What Remains Active
[List current production documentation and tools]
```

### Phase 4: Reference Updates and Validation

#### 4.1 Update All References
**Systematic reference updating:**
```bash
# Find and update all references to moved files
grep -r "PHASE_.*\.md" . --exclude-dir=archive | while read -r line; do
    # Update references to archived files
    file=$(echo "$line" | cut -d: -f1)
    # Update with archive paths
done

# Update script references
grep -r "scripts/legacy_archive" . --exclude-dir=archive
# Replace with archive/development_artifacts/legacy_archive

# Update documentation references
grep -r "docs/archives" . --exclude-dir=archive  
# Replace with appropriate archive/ paths
```

#### 4.2 Validate Production Systems
**Ensure cleanup didn't break anything:**
```bash
# Test build system
./scripts/fix_cmake.sh

# Test essential scripts
python3 scripts/validate_ssot.py
python3 scripts/auto_update_status.py --dry-run

# Test documentation search
./scripts/stm32_search.sh concept "test" --scope all

# Validate VSCode tasks
# Open VSCode and test each task in tasks.json
```

#### 4.3 Update Git and Documentation
**Commit systematically:**
```bash
# Stage and commit in logical groups
git add archive/
git commit -m "feat: Create organized archive structure with comprehensive documentation"

git add README.md docs/README.md  
git commit -m "feat: Update README files to production-ready status"

git add api/
git commit -m "feat: Update API documentation to v1.0.0 production status"

git add -A  # Catch any remaining reference updates
git commit -m "feat: Update all references to archived files and complete workspace cleanup"
```

## Cleanup Validation Checklist

### ✅ **Archive Structure Validation**
- [ ] All archives properly organized by category
- [ ] Archive README.md documents all contents
- [ ] No empty directories left behind
- [ ] Git history preserved for all moved files

### ✅ **Production System Validation**  
- [ ] Build system works (`./scripts/fix_cmake.sh`)
- [ ] All VSCode tasks functional
- [ ] Essential scripts operational
- [ ] Documentation search works
- [ ] SSOT validation passes

### ✅ **Documentation Validation**
- [ ] All README.md files reflect production status
- [ ] API documentation updated to v1.0.0
- [ ] No broken links to archived files
- [ ] Status language consistent across files

### ✅ **Reference Integrity**
- [ ] All references to moved files updated
- [ ] No hardcoded paths to archived content
- [ ] Documentation examples use current tools
- [ ] Instructions reference active systems

## Anti-Patterns to Avoid

### ❌ **Don't Do This**
```bash
# DON'T: Delete without archiving
rm -rf outdated_directory/

# DON'T: Archive without updating references  
git mv important_script.py archive/
# (Breaks systems that depend on it)

# DON'T: Archive production-critical tools
git mv scripts/fix_cmake.sh archive/
# (Breaks build system)

# DON'T: Mass find-replace without verification
sed -i 's/development/production/g' **/*.md
# (May break specific contexts)
```

### ✅ **Do This Instead**
```bash
# DO: Archive with documentation
mkdir -p archive/category/
git mv outdated_file.md archive/category/
echo "Archived: reason and replacement" >> archive/category/README.md

# DO: Verify references before archiving
grep -r "important_script.py" . --exclude-dir=archive
# Update references first, then archive

# DO: Test after each major change
./scripts/fix_cmake.sh  # Verify build still works
```

## Maintenance Guidelines

### **Post-Cleanup Workflow**
1. **Regular Archive Reviews**: Quarterly review of archive contents
2. **Reference Validation**: Monthly link checking with `scripts/link_validator.py`
3. **Status Updates**: Keep README.md files current with system changes
4. **Archive Hygiene**: Don't let new development artifacts accumulate in main directories

### **Adding New Archives**
```bash
# When archiving new items:
1. Determine appropriate archive category
2. Update archive/README.md with new contents
3. Check for and update all references
4. Test production systems after archival
5. Commit with descriptive message
```

### **Recovery Procedures**
```bash
# If archived file needed again:
cp archive/category/file.ext ./current/location/
# Or restore with git:
git mv archive/category/file.ext ./current/location/

# Update references and test functionality
grep -r "file.ext" . --exclude-dir=archive
# Update any hardcoded paths
```

## Integration with Existing Instructions

### **Related Instruction Files**
- **file-organization.instructions.md**: File placement rules and directory structure
- **ssot-config.instructions.md**: Configuration management during cleanup
- **project-setup.instructions.md**: Post-cleanup workspace setup for new developers

### **SSOT Compliance**
- All archive paths should use SSOT configuration where applicable
- Documentation references must follow centralized path definitions
- Archive organization should align with project file organization principles

## Success Metrics

### **Quantitative Measures**
- **Directory Count Reduction**: Measure root-level directory reduction
- **Script Count Optimization**: Track scripts/ directory size reduction  
- **Documentation Clarity**: Count of development-specific references removed
- **Reference Accuracy**: Zero broken links after cleanup

### **Qualitative Measures**
- **Developer Experience**: New developers can quickly understand current state
- **Maintenance Overhead**: Reduced cognitive load for workspace navigation
- **Production Focus**: Clear distinction between active and historical content

---

**Template Usage**: This instruction file provides the complete methodology used successfully for STM32H753ZI project cleanup. Adapt category names and specific paths for different projects while maintaining the systematic approach and validation steps.

**Key Success Factors**: Systematic evaluation, organized archival, comprehensive reference updates, and thorough validation ensure cleanup improves rather than disrupts production workflows.
