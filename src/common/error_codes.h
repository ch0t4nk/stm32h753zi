/**
 * @file src/common/error_codes.h
 * @brief Legacy wrapper to include canonical error codes (SSOT)
 *
 * This header preserves legacy macros and prototypes while delegating
 * the actual enum definitions to src/config/error_codes.h (the canonical
 * single source of truth). It must remain lightweight and parseable by C
 * compilers without nested or unterminated comments.
 */

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#include <stdbool.h>
#include <stdint.h>

/* Shared constants (MESSAGE_MAGIC, etc) */
#include "config/constants_shared.h"

/* Include the canonical SSOT for error codes */
#include "config/error_codes.h"

/* No local definition here — canonical ErrorInfo_t is in config/error_codes.h
 */

/* Preserve legacy macros and prototypes (thin wrappers forwarding to
 * the canonical implementations expected in the codebase). These macros
 * intentionally use the canonical function names so no duplicate symbols
 * are introduced.
 */
#define CHECK_ERROR(expr, error_code)                                         \
    do {                                                                      \
        if (!(expr)) {                                                        \
            log_error((error_code), __FILE__, __LINE__, __FUNCTION__);        \
            return (error_code);                                              \
        }                                                                     \
    } while (0)

#define LOG_ERROR(error_code, description)                                    \
    log_error_detailed((error_code), (description), __FILE__, __LINE__,       \
                       __FUNCTION__)

#define RETURN_ERROR(error_code)                                              \
    do {                                                                      \
        LOG_ERROR((error_code), #error_code);                                 \
        return (error_code);                                                  \
    } while (0)

/* Public prototypes (expected by legacy callers) */
const char *error_code_to_string(uint32_t error_code);
ErrorSeverity_t get_error_severity(uint32_t error_code);
bool is_critical_error(uint32_t error_code);
void log_error(uint32_t error_code, const char *file, uint32_t line,
               const char *function);
void log_error_detailed(uint32_t error_code, const char *description,
                        const char *file, uint32_t line, const char *function);

#endif /* ERROR_CODES_H */
