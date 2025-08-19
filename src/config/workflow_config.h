/**
 * @file workflow_config.h
 * @brief Development Workflow Configuration (SSOT)
 *
 * Restored from archive as a workflow SSOT header. This file centralizes
 * workflow-related configuration flags used by scripts and tooling. It is a
 * non-destructive restore intended to satisfy SSOT validation and tooling.
 */

#ifndef WORKFLOW_CONFIG_H
#define WORKFLOW_CONFIG_H

// Minimal, standard, thorough, debug modes
typedef enum {
    WORKFLOW_MODE_MINIMAL = 0,
    WORKFLOW_MODE_STANDARD = 1,
    WORKFLOW_MODE_THOROUGH = 2,
    WORKFLOW_MODE_DEBUG = 3,
    WORKFLOW_MODE_CUSTOM = 4
} WorkflowMode_t;

#ifndef WORKFLOW_MODE
#define WORKFLOW_MODE WORKFLOW_MODE_STANDARD
#endif

// Basic flags (defaults chosen for safety and speed)
#ifndef WORKFLOW_VALIDATE_VENV
#define WORKFLOW_VALIDATE_VENV 1
#endif

#ifndef WORKFLOW_VALIDATE_PYTHON
#define WORKFLOW_VALIDATE_PYTHON 1
#endif

#ifndef WORKFLOW_DETECT_PLATFORM
#define WORKFLOW_DETECT_PLATFORM 1
#endif

#ifndef WORKFLOW_VALIDATE_BUILD_ENV
#define WORKFLOW_VALIDATE_BUILD_ENV 0
#endif

#ifndef WORKFLOW_LOAD_FUNCTIONS
#define WORKFLOW_LOAD_FUNCTIONS 1
#endif

#ifndef WORKFLOW_CREATE_ALIASES
#define WORKFLOW_CREATE_ALIASES 1
#endif

#ifndef WORKFLOW_AUTO_ACTIVATE_VENV
#define WORKFLOW_AUTO_ACTIVATE_VENV 0
#endif

#ifndef WORKFLOW_VALIDATE_SSOT
#define WORKFLOW_VALIDATE_SSOT 0
#endif

#ifndef WORKFLOW_VERBOSE_LOGGING
#define WORKFLOW_VERBOSE_LOGGING 0
#endif

#ifndef WORKFLOW_CONTINUE_ON_ERROR
#define WORKFLOW_CONTINUE_ON_ERROR 1
#endif

#define WORKFLOW_MODE_ENV_VAR "STM32_WORKFLOW_MODE"

#endif // WORKFLOW_CONFIG_H
