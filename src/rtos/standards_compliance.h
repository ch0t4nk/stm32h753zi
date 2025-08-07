/**
 * @file standards_compliance.h
 * @brief Standards Compliance and Certification Support Framework
 * @author STM32H753ZI Project Team
 * @date August 2025
 * @version 1.0
 *
 * This module provides comprehensive support for industrial standards
 * compliance including MISRA-C, IEC 61508 SIL-2, ISO 26262, and traceability
 * frameworks for certification-ready embedded systems.
 */

#ifndef STANDARDS_COMPLIANCE_H
#define STANDARDS_COMPLIANCE_H

/* ============================================================================
 */
/* Includes */
/* ============================================================================
 */

#include "common/error_codes.h"
#include "config/freertos_config_ssot.h"

#include <stdbool.h>
#include <stdint.h>

/* ============================================================================
 */
/* Compliance Standards Support */
/* ============================================================================
 */

/**
 * @brief MISRA-C Compliance Support
 * @note MISRA-C:2012 Guidelines for C in Critical Systems
 * @note Automated rule checking and violation tracking
 */

/// @brief MISRA-C Rule Categories
typedef enum {
    MISRA_CATEGORY_REQUIRED,  ///< Required rules (must comply)
    MISRA_CATEGORY_ADVISORY,  ///< Advisory rules (should comply)
    MISRA_CATEGORY_MANDATORY, ///< Mandatory rules (shall comply)
    MISRA_CATEGORY_COUNT
} MisraRuleCategory_t;

/// @brief MISRA-C Rule Violation Record
typedef struct {
    uint16_t rule_number;         ///< MISRA-C rule number (e.g., 15.5)
    MisraRuleCategory_t category; ///< Rule category
    const char *file_name;        ///< Source file name
    uint32_t line_number;         ///< Line number of violation
    const char *description;      ///< Violation description
    bool justified;               ///< Violation is justified/documented
    const char *justification;    ///< Justification text
    uint32_t timestamp;           ///< When violation was recorded
} MisraViolationRecord_t;

/**
 * @brief IEC 61508 Functional Safety Support
 * @note IEC 61508 Safety Integrity Level (SIL) Requirements
 * @note SIL-2 compliance for motor control safety systems
 */

/// @brief Safety Integrity Levels
typedef enum {
    SIL_NONE = 0, ///< No safety requirements
    SIL_1 = 1,    ///< Low safety integrity
    SIL_2 = 2,    ///< Medium safety integrity (target)
    SIL_3 = 3,    ///< High safety integrity
    SIL_4 = 4     ///< Very high safety integrity
} SafetyIntegrityLevel_t;

/// @brief Safety Function Classification
typedef struct {
    const char *function_name;        ///< Safety function name
    SafetyIntegrityLevel_t sil_level; ///< Required SIL level
    uint32_t function_id;             ///< Unique function identifier
    bool implemented;                 ///< Implementation status
    bool verified;                    ///< Verification status
    bool validated;                   ///< Validation status
    const char *requirements_ref;     ///< Requirements reference
    const char *test_case_ref;        ///< Test case reference
} SafetyFunctionRecord_t;

/**
 * @brief ISO 26262 Automotive Safety Support
 * @note ASIL (Automotive Safety Integrity Level) Requirements
 * @note For automotive applications of motor control
 */

/// @brief Automotive Safety Integrity Levels
typedef enum {
    ASIL_NONE = 0, ///< No ASIL requirements
    ASIL_A = 1,    ///< ASIL A (lowest)
    ASIL_B = 2,    ///< ASIL B
    ASIL_C = 3,    ///< ASIL C
    ASIL_D = 4     ///< ASIL D (highest)
} AutomotiveSafetyLevel_t;

/* ============================================================================
 */
/* Traceability and Documentation Framework */
/* ============================================================================
 */

/// @brief Requirement Traceability Record
typedef struct {
    uint32_t requirement_id;         ///< Unique requirement identifier
    const char *requirement_text;    ///< Requirement description
    const char *source_document;     ///< Source document reference
    const char *implementation_file; ///< Implementation file
    uint32_t implementation_line;    ///< Implementation line number
    const char *test_case_ref;       ///< Test case reference
    const char *verification_method; ///< Verification method
    bool implemented;                ///< Implementation status
    bool tested;                     ///< Testing status
    bool verified;                   ///< Verification status
} RequirementTrace_t;

/// @brief Design Review Record
typedef struct {
    uint32_t review_id;          ///< Review identifier
    const char *review_type;     ///< Type of review (code, design, etc.)
    const char *reviewer_name;   ///< Reviewer name/identifier
    const char *file_or_module;  ///< File or module reviewed
    uint32_t timestamp;          ///< Review timestamp
    const char *findings;        ///< Review findings
    const char *recommendations; ///< Recommendations
    bool approved;               ///< Approval status
} DesignReviewRecord_t;

/// @brief Code Quality Metrics
typedef struct {
    uint32_t total_lines_of_code;      ///< Total lines of code
    uint32_t comment_lines;            ///< Comment lines
    uint32_t blank_lines;              ///< Blank lines
    float comment_ratio;               ///< Comment to code ratio
    uint32_t cyclomatic_complexity;    ///< Maximum cyclomatic complexity
    uint32_t function_count;           ///< Total number of functions
    uint32_t average_function_length;  ///< Average function length
    uint32_t max_function_length;      ///< Maximum function length
    uint32_t static_analysis_warnings; ///< Static analysis warnings
    uint32_t misra_violations;         ///< MISRA-C violations
} CodeQualityMetrics_t;

/* ============================================================================
 */
/* Configuration Structures */
/* ============================================================================
 */

/// @brief Compliance Configuration
typedef struct {
    bool misra_checking_enabled;         ///< Enable MISRA-C checking
    bool sil_validation_enabled;         ///< Enable SIL validation
    bool asil_validation_enabled;        ///< Enable ASIL validation
    bool traceability_enabled;           ///< Enable requirement traceability
    bool automated_reporting_enabled;    ///< Enable automated reporting
    SafetyIntegrityLevel_t target_sil;   ///< Target SIL level
    AutomotiveSafetyLevel_t target_asil; ///< Target ASIL level
    const char *project_identifier;      ///< Project identifier for reports
    const char *certification_authority; ///< Certification authority
} ComplianceConfiguration_t;

/* ============================================================================
 */
/* Public API Functions */
/* ============================================================================
 */

/**
 * @brief Initialize standards compliance framework
 * @param config Compliance configuration
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_COMPLIANCE_001: Initialize compliance framework
 * @misra Rule 8.4: External function declarations
 * @sil SIL-2: Initialization with validation
 */
SystemError_t
standards_compliance_init(const ComplianceConfiguration_t *config);

/**
 * @brief Record MISRA-C rule violation
 * @param rule_number MISRA-C rule number
 * @param category Rule category
 * @param file_name Source file name
 * @param line_number Line number
 * @param description Violation description
 * @param justification Optional justification (NULL if none)
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_MISRA_001: Record MISRA violations for analysis
 * @misra Rule 8.4: External function declarations
 */
SystemError_t standards_record_misra_violation(
    uint16_t rule_number, MisraRuleCategory_t category, const char *file_name,
    uint32_t line_number, const char *description, const char *justification);

/**
 * @brief Register safety function for SIL compliance
 * @param function_name Safety function name
 * @param sil_level Required SIL level
 * @param function_id Unique function identifier
 * @param requirements_ref Requirements reference
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_SIL_001: Register safety functions for compliance
 * @sil SIL-2: Safety function registration and tracking
 */
SystemError_t standards_register_safety_function(
    const char *function_name, SafetyIntegrityLevel_t sil_level,
    uint32_t function_id, const char *requirements_ref);

/**
 * @brief Add requirement traceability record
 * @param requirement_id Unique requirement identifier
 * @param requirement_text Requirement description
 * @param source_document Source document reference
 * @param implementation_file Implementation file
 * @param implementation_line Implementation line number
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_TRACE_001: Maintain requirement traceability
 * @misra Rule 8.4: External function declarations
 */
SystemError_t standards_add_requirement_trace(uint32_t requirement_id,
                                              const char *requirement_text,
                                              const char *source_document,
                                              const char *implementation_file,
                                              uint32_t implementation_line);

/**
 * @brief Generate compliance report
 * @param buffer Buffer to write report to
 * @param buffer_size Size of buffer
 * @param report_type Type of report to generate
 * @return Number of bytes written, or 0 on error
 * @req REQ_REPORT_001: Generate compliance reports
 * @misra Rule 8.4: External function declarations
 * @sil SIL-2: Comprehensive reporting for certification
 */
uint32_t standards_generate_compliance_report(char *buffer,
                                              uint32_t buffer_size,
                                              const char *report_type);

/**
 * @brief Validate SIL compliance status
 * @param sil_level Target SIL level to validate
 * @param validation_report Buffer for validation report
 * @param report_size Size of report buffer
 * @return SYSTEM_OK if compliant, error code if non-compliant
 * @req REQ_SIL_002: Validate SIL compliance
 * @sil SIL-2: Compliance validation with detailed reporting
 */
SystemError_t
standards_validate_sil_compliance(SafetyIntegrityLevel_t sil_level,
                                  char *validation_report,
                                  uint32_t report_size);

/**
 * @brief Record design review
 * @param review_type Type of review
 * @param reviewer_name Reviewer identifier
 * @param file_or_module File or module reviewed
 * @param findings Review findings
 * @param recommendations Recommendations
 * @param approved Approval status
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_REVIEW_001: Record design reviews
 * @misra Rule 8.4: External function declarations
 */
SystemError_t standards_record_design_review(const char *review_type,
                                             const char *reviewer_name,
                                             const char *file_or_module,
                                             const char *findings,
                                             const char *recommendations,
                                             bool approved);

/**
 * @brief Update code quality metrics
 * @param metrics Current code quality metrics
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_QUALITY_001: Track code quality metrics
 * @misra Rule 8.4: External function declarations
 */
SystemError_t
standards_update_code_metrics(const CodeQualityMetrics_t *metrics);

/**
 * @brief Export compliance data for external tools
 * @param export_format Export format ("XML", "JSON", "CSV")
 * @param buffer Buffer to write exported data
 * @param buffer_size Size of buffer
 * @return Number of bytes written, or 0 on error
 * @req REQ_EXPORT_001: Export compliance data
 * @misra Rule 8.4: External function declarations
 */
uint32_t standards_export_compliance_data(const char *export_format,
                                          char *buffer, uint32_t buffer_size);

/* ============================================================================
 */
/* Compliance Macros and Utilities                                           */
/* ============================================================================
 */

/**
 * @brief MISRA-C compliance annotation macro
 * @param rule Rule number (e.g., 15.5)
 * @param justification Justification text
 * @note Use to document justified MISRA violations
 * @misra Rule 20.10: Preprocessing directives
 */
#define MISRA_JUSTIFIED(rule, justification)                                  \
    do {                                                                      \
        /* MISRA-C Rule rule justified: justification */                      \
    } while (0)

/**
 * @brief SIL function annotation macro
 * @param level SIL level (1-4)
 * @param function_id Function identifier
 * @note Use to mark safety-critical functions
 * @sil SIL-2: Function marking for traceability
 */
#define SIL_FUNCTION(level, function_id)                                      \
    /* SIL-level Function ID: function_id */

/**
 * @brief Requirement traceability annotation
 * @param req_id Requirement identifier
 * @note Use to link code to requirements
 * @req REQ_TRACE_002: Code-level requirement annotation
 */
#define REQ_TRACE(req_id) /* Requirement: req_id */

/**
 * @brief Safety assertion with SIL compliance
 * @param condition Condition to assert
 * @param sil_level SIL level of assertion
 * @note Use for safety-critical assertions
 * @sil SIL-2: Safety assertions with compliance tracking
 */
#define SIL_ASSERT(condition, sil_level)                                      \
    do {                                                                      \
        if (!(condition)) {                                                   \
            standards_record_safety_violation(__FILE__, __LINE__, #condition, \
                                              sil_level);                     \
            /* Handle safety violation based on SIL level */                  \
        }                                                                     \
    } while (0)

/* ============================================================================
 */
/* Static Analysis Integration */
/* ============================================================================
 */

/**
 * @brief Configure static analysis tools
 * @param enable_misra Enable MISRA-C checking
 * @param enable_polyspace Enable PolySpace analysis
 * @param enable_pc_lint Enable PC-Lint analysis
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_STATIC_001: Configure static analysis tools
 */
SystemError_t standards_configure_static_analysis(bool enable_misra,
                                                  bool enable_polyspace,
                                                  bool enable_pc_lint);

/**
 * @brief Import static analysis results
 * @param tool_name Analysis tool name
 * @param results_file Path to results file
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_STATIC_002: Import analysis results
 */
SystemError_t standards_import_analysis_results(const char *tool_name,
                                                const char *results_file);

/* ============================================================================
 */
/* Certification Support */
/* ============================================================================
 */

/**
 * @brief Generate certification package
 * @param certification_type Type of certification ("IEC61508", "ISO26262",
 * etc.)
 * @param package_directory Directory to create package in
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_CERT_001: Generate certification packages
 * @sil SIL-2: Certification document generation
 */
SystemError_t
standards_generate_certification_package(const char *certification_type,
                                         const char *package_directory);

/**
 * @brief Validate certification readiness
 * @param certification_type Type of certification to validate
 * @param readiness_report Buffer for readiness report
 * @param report_size Size of report buffer
 * @return SYSTEM_OK if ready, error code if not ready
 * @req REQ_CERT_002: Validate certification readiness
 * @sil SIL-2: Certification readiness assessment
 */
SystemError_t
standards_validate_certification_readiness(const char *certification_type,
                                           char *readiness_report,
                                           uint32_t report_size);

/* ============================================================================
 */
/* Debug and Development Support */
/* ============================================================================
 */

/**
 * @brief Print compliance status summary
 * @note Development and debugging function
 * @req REQ_DEBUG_001: Compliance status reporting
 */
void standards_print_compliance_status(void);

/**
 * @brief Validate internal compliance data consistency
 * @return true if consistent, false if corruption detected
 * @req REQ_DEBUG_002: Internal consistency validation
 * @sil SIL-2: Self-diagnostic capability
 */
bool standards_validate_internal_consistency(void);

#endif // STANDARDS_COMPLIANCE_H
