/**
 * @file documentation_config.h
 * @brief Documentation Structure Configuration - SSOT for Documentation
 * Management
 *
 * Restored from archive as a documentation SSOT header. Intended as a
 * non-destructive placeholder to satisfy validation and tooling that expect
 * these definitions under src/config/.
 */

#ifndef DOCUMENTATION_CONFIG_H
#define DOCUMENTATION_CONFIG_H

#define DOC_ROOT_DIR "docs"
#define DOC_INDEXES_DIR "docs/indexes"
#define DOC_DESIGN_DIR "docs/design"
#define DOC_REQUIREMENTS_DIR "docs/requirements"

#define INSTRUCTION_ROOT_DIR ".github/instructions"
#define INSTRUCTION_FILE_SUFFIX ".instructions.md"
#define INSTRUCTION_README_FILE "README.md"

#define REFERENCE_ROOT_DIR "00_reference"
#define REFERENCE_STM32H7_DIR "00_reference/stm32h753xx_User_Manual_md"
#define REFERENCE_L6470_DIR "00_reference/x_cube_spn2_markdown_docs"

#define DOC_INDEX_STM32H7_FULL "docs/indexes/STM32H7_FULL_INDEX.json"
#define DOC_INDEX_STM32H7_COPILOT_JSON                                        \
    "docs/indexes/STM32H7_COPILOT_INDEX.json"
#define DOC_INDEX_L6470_SEARCH "docs/indexes/L6470_SEARCH_INDEX.json"

#define DOXYGEN_CONFIG_FILE "docs/Doxyfile"
#define DOXYGEN_OUTPUT_DIR "docs/doxygen"
#define DOC_GENERATED_DIR "docs/generated"

#endif /* DOCUMENTATION_CONFIG_H */
