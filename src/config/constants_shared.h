/**
 * @file constants_shared.h
 * @brief Small SSOT header for widely-used constants to avoid duplication
 *
 * This header intentionally contains only macro definitions (no types)
 * so it can be included broadly without creating circular dependencies.
 */
#ifndef CONSTANTS_SHARED_H
#define CONSTANTS_SHARED_H

#include <stdint.h>

/* Universal message magic used for message header validation */
#define MESSAGE_MAGIC 0xDEADBEEF

/* Base values for error code ranges (mirrors existing values) */
#define ERROR_CODE_SUCCESS 0x0000
#define ERROR_CODE_BASE 0x1000

#endif /* CONSTANTS_SHARED_H */
