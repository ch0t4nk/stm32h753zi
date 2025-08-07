/**
 * @file embedding_config.h
 * @brief SSOT Configuration for Embedding Models and Semantic Search
 *
 * Single Source of Truth for embedding model selection, performance tuning,
 * and semantic search configuration for STM32H753ZI project optimization.
 *
 * This configuration enables dynamic model selection based on:
 * - Hardware capabilities (GPU/CPU)
 * - Performance requirements (speed vs quality)
 * - Use case specific optimization (HAL docs, motor control, etc.)
 *
 * @author STM32H753ZI Motor Control Project
 * @date 2025-08-07
 */

#ifndef EMBEDDING_CONFIG_H
#define EMBEDDING_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 */
/* EMBEDDING MODEL CONFIGURATION (SSOT) */
/* ============================================================================
 */

/**
 * @defgroup EMBEDDING_MODELS Embedding Model Identifiers
 * @brief Standardized model identifiers for semantic search configuration
 * @{
 */

/* Primary embedding models with performance characteristics */
#define EMBEDDING_MODEL_INSTRUCTOR_XL_GPU "instructor_xl_gpu"
#define EMBEDDING_MODEL_INSTRUCTOR_XL_CPU "instructor_xl_cpu"
#define EMBEDDING_MODEL_MXBAI_EMBED_LARGE "mxbai_embed_large"
#define EMBEDDING_MODEL_NOMIC_EMBED_TEXT "nomic_embed_text"

/* Model performance tiers for automatic selection */
#define EMBEDDING_TIER_HIGH_QUALITY 1 /* Best accuracy, slower */
#define EMBEDDING_TIER_BALANCED 2     /* Good accuracy, moderate speed */
#define EMBEDDING_TIER_HIGH_SPEED 3   /* Fast inference, lower accuracy */

/**
 * @}
 */

/**
 * @defgroup EMBEDDING_DIMENSIONS Vector Dimensions by Model
 * @brief Embedding vector dimensions for memory allocation and validation
 * @{
 */

#define INSTRUCTOR_XL_DIMENSIONS 768
#define MXBAI_EMBED_LARGE_DIMENSIONS 1024
#define NOMIC_EMBED_TEXT_DIMENSIONS 768
#define MAX_EMBEDDING_DIMENSIONS 1024 /* For buffer allocation */

/**
 * @}
 */

/**
 * @defgroup PERFORMANCE_THRESHOLDS Performance Configuration Thresholds
 * @brief Timing and memory thresholds for embedding model evaluation
 * @{
 */

/* Response time thresholds (seconds) */
#define EMBEDDING_FAST_THRESHOLD_MS 500  /* <0.5s considered fast */
#define EMBEDDING_SLOW_THRESHOLD_MS 2000 /* >2.0s considered slow */

/* Memory usage thresholds (MB) */
#define EMBEDDING_LOW_MEMORY_THRESHOLD_MB 100   /* <100MB low memory */
#define EMBEDDING_HIGH_MEMORY_THRESHOLD_MB 1000 /* >1GB high memory */

/* Quality thresholds (relevance scores 0.0-1.0) */
#define EMBEDDING_EXCELLENT_QUALITY 0.8f /* Excellent performance */
#define EMBEDDING_GOOD_QUALITY 0.6f      /* Good performance */
#define EMBEDDING_MINIMUM_QUALITY 0.4f   /* Minimum acceptable */

/**
 * @}
 */

/**
 * @defgroup DOMAIN_OPTIMIZATION Domain-Specific Optimization
 * @brief Configuration for STM32H753ZI domain-specific embedding optimization
 * @{
 */

/* Domain identifiers for instruction-tuned embeddings */
#define DOMAIN_STM32_HAL "stm32_hal"
#define DOMAIN_MOTOR_CONTROL "motor_control"
#define DOMAIN_SAFETY_SYSTEMS "safety_systems"
#define DOMAIN_BUILD_SYSTEM "build_system"
#define DOMAIN_PROJECT_CODE "project_source"

/* Instruction templates for domain optimization */
#define INSTRUCTION_STM32_HAL                                                 \
    "Represent the STM32 HAL documentation for finding peripheral "           \
    "configuration and driver functions:"
#define INSTRUCTION_MOTOR_CONTROL                                             \
    "Represent the stepper motor control documentation for finding L6470 "    \
    "driver functions and configuration:"
#define INSTRUCTION_SAFETY_SYSTEMS                                            \
    "Represent the safety system documentation for finding fault detection "  \
    "and recovery procedures:"
#define INSTRUCTION_BUILD_SYSTEM                                              \
    "Represent the build configuration for finding CMake setup and "          \
    "compilation options:"
#define INSTRUCTION_PROJECT_CODE                                              \
    "Represent the embedded C source code for finding HAL abstraction "       \
    "functions and drivers:"

/* Batch size optimization by domain complexity */
#define BATCH_SIZE_STM32_HAL_GPU 25     /* Large docs, GPU acceleration */
#define BATCH_SIZE_STM32_HAL_CPU 8      /* Large docs, CPU only */
#define BATCH_SIZE_MOTOR_CONTROL_GPU 30 /* Medium docs, GPU */
#define BATCH_SIZE_MOTOR_CONTROL_CPU 10 /* Medium docs, CPU */
#define BATCH_SIZE_PROJECT_CODE_GPU 20  /* Source code, GPU */
#define BATCH_SIZE_PROJECT_CODE_CPU 6   /* Source code, CPU */

/**
 * @}
 */

/**
 * @defgroup MODEL_SELECTION Automatic Model Selection Configuration
 * @brief Rules for automatic embedding model selection based on requirements
 * @{
 */

/* Selection criteria flags */
#define SELECT_OPTIMIZE_QUALITY (1 << 0) /* Prioritize accuracy */
#define SELECT_OPTIMIZE_SPEED (1 << 1)   /* Prioritize speed */
#define SELECT_OPTIMIZE_MEMORY (1 << 2)  /* Minimize memory usage */
#define SELECT_REQUIRE_GPU (1 << 3)      /* GPU required */
#define SELECT_ALLOW_FALLBACK (1 << 4)   /* Allow fallback models */

/* Default selection for different use cases */
#define SELECT_DEVELOPMENT_DEFAULT                                            \
    (SELECT_OPTIMIZE_QUALITY | SELECT_ALLOW_FALLBACK)
#define SELECT_PRODUCTION_DEFAULT                                             \
    (SELECT_OPTIMIZE_SPEED | SELECT_OPTIMIZE_MEMORY)
#define SELECT_ANALYSIS_DEFAULT (SELECT_OPTIMIZE_QUALITY | SELECT_REQUIRE_GPU)

/**
 * @}
 */

/**
 * @defgroup EVALUATION_CONFIG Evaluation and Benchmarking Configuration
 * @brief Configuration for embedding model evaluation and performance testing
 * @{
 */

/* Test scenario weights for overall scoring */
#define WEIGHT_STM32_HAL_FUNCTIONS 0.25f  /* HAL functions priority */
#define WEIGHT_L6470_MOTOR_CONTROL 0.25f  /* Motor control priority */
#define WEIGHT_SAFETY_SYSTEMS 0.20f       /* Safety systems priority */
#define WEIGHT_BUILD_CONFIGURATION 0.15f  /* Build system priority */
#define WEIGHT_PROJECT_ARCHITECTURE 0.15f /* Architecture priority */

/* Evaluation test configuration */
#define EVALUATION_QUERIES_PER_SCENARIO 5 /* Test queries per domain */
#define EVALUATION_SIMILARITY_SAMPLES 3   /* Similarity test samples */
#define EVALUATION_NOISE_DOCUMENTS 3      /* Noise docs for separation test */

/* Performance monitoring intervals */
#define PERFORMANCE_SAMPLE_INTERVAL_MS 100     /* Performance sampling rate */
#define PERFORMANCE_LOG_SLOW_THRESHOLD_MS 1000 /* Log operations >1s */

/**
 * @}
 */

/**
 * @defgroup CACHE_CONFIG Embedding Cache Configuration
 * @brief Configuration for embedding result caching and optimization
 * @{
 */

/* Cache size limits */
#define EMBEDDING_CACHE_MAX_ENTRIES 10000 /* Max cached embeddings */
#define EMBEDDING_CACHE_MAX_SIZE_MB 500   /* Max cache size in MB */

/* Cache invalidation settings */
#define EMBEDDING_CACHE_TTL_HOURS 24          /* Cache time-to-live */
#define EMBEDDING_CACHE_CLEANUP_INTERVAL 3600 /* Cleanup interval (seconds)   \
                                               */

/* Cache key configuration */
#define CACHE_KEY_MAX_LENGTH 256   /* Max cache key length */
#define CACHE_HASH_ALGORITHM "MD5" /* Hash algorithm for keys */

/**
 * @}
 */

/**
 * @defgroup ERROR_HANDLING Embedding Error Handling Configuration
 * @brief Error codes and fallback configuration for embedding operations
 * @{
 */

/* Embedding-specific error codes */
#define EMBEDDING_ERROR_MODEL_NOT_AVAILABLE 0x3001
#define EMBEDDING_ERROR_DIMENSION_MISMATCH 0x3002
#define EMBEDDING_ERROR_TIMEOUT 0x3003
#define EMBEDDING_ERROR_MEMORY_ALLOCATION 0x3004
#define EMBEDDING_ERROR_NETWORK_FAILURE 0x3005

/* Fallback configuration */
#define EMBEDDING_ENABLE_MOCK_FALLBACK                                        \
    1                           /* Enable mock embeddings as last resort */
#define EMBEDDING_RETRY_COUNT 3 /* Retry attempts for failed operations */
#define EMBEDDING_TIMEOUT_SECONDS 30 /* Operation timeout */

/**
 * @}
 */

/**
 * @defgroup LOGGING_CONFIG Embedding Logging Configuration
 * @brief Logging levels and output configuration for embedding operations
 * @{
 */

/* Logging levels for embedding operations */
#define EMBEDDING_LOG_LEVEL_ERROR 1
#define EMBEDDING_LOG_LEVEL_WARNING 2
#define EMBEDDING_LOG_LEVEL_INFO 3
#define EMBEDDING_LOG_LEVEL_DEBUG 4

/* Default logging configuration */
#define EMBEDDING_DEFAULT_LOG_LEVEL EMBEDDING_LOG_LEVEL_INFO
#define EMBEDDING_LOG_PERFORMANCE 1     /* Enable performance logging */
#define EMBEDDING_LOG_MODEL_SELECTION 1 /* Log model selection decisions */

/**
 * @}
 */

/* ============================================================================
 */
/* STATIC ASSERTIONS FOR CONFIGURATION VALIDATION */
/* ============================================================================
 */

/* Validate dimension configurations */
_Static_assert(INSTRUCTOR_XL_DIMENSIONS <= MAX_EMBEDDING_DIMENSIONS,
               "Instructor-XL dimensions exceed maximum buffer size");
_Static_assert(MXBAI_EMBED_LARGE_DIMENSIONS <= MAX_EMBEDDING_DIMENSIONS,
               "mxbai-embed-large dimensions exceed maximum buffer size");
_Static_assert(NOMIC_EMBED_TEXT_DIMENSIONS <= MAX_EMBEDDING_DIMENSIONS,
               "nomic-embed-text dimensions exceed maximum buffer size");

/* Validate performance thresholds are sensible */
_Static_assert(EMBEDDING_FAST_THRESHOLD_MS < EMBEDDING_SLOW_THRESHOLD_MS,
               "Fast threshold must be less than slow threshold");
_Static_assert(EMBEDDING_LOW_MEMORY_THRESHOLD_MB <
                   EMBEDDING_HIGH_MEMORY_THRESHOLD_MB,
               "Low memory threshold must be less than high memory threshold");

/* Validate quality thresholds are in valid range */
_Static_assert(EMBEDDING_MINIMUM_QUALITY >= 0.0f &&
                   EMBEDDING_MINIMUM_QUALITY <= 1.0f,
               "Minimum quality threshold must be between 0.0 and 1.0");
_Static_assert(EMBEDDING_EXCELLENT_QUALITY >= 0.0f &&
                   EMBEDDING_EXCELLENT_QUALITY <= 1.0f,
               "Excellent quality threshold must be between 0.0 and 1.0");

/* Validate scenario weights sum to approximately 1.0 */
_Static_assert((WEIGHT_STM32_HAL_FUNCTIONS + WEIGHT_L6470_MOTOR_CONTROL +
                WEIGHT_SAFETY_SYSTEMS + WEIGHT_BUILD_CONFIGURATION +
                WEIGHT_PROJECT_ARCHITECTURE) > 0.95f &&
                   (WEIGHT_STM32_HAL_FUNCTIONS + WEIGHT_L6470_MOTOR_CONTROL +
                    WEIGHT_SAFETY_SYSTEMS + WEIGHT_BUILD_CONFIGURATION +
                    WEIGHT_PROJECT_ARCHITECTURE) < 1.05f,
               "Evaluation weights must sum to approximately 1.0");

#ifdef __cplusplus
}
#endif

#endif /* EMBEDDING_CONFIG_H */

/**
 * @example embedding_config_usage.c
 * Example usage of embedding configuration:
 *
 * ```c
 * #include "embedding_config.h"
 *
 * // Select model based on requirements
 * uint32_t selection_flags = SELECT_OPTIMIZE_QUALITY | SELECT_ALLOW_FALLBACK;
 * const char* model = select_best_embedding_model(selection_flags);
 *
 * // Get domain-specific instruction
 * const char* instruction = get_domain_instruction(DOMAIN_STM32_HAL);
 *
 * // Configure batch size based on domain and hardware
 * int batch_size = is_gpu_available() ?
 *                  BATCH_SIZE_STM32_HAL_GPU :
 *                  BATCH_SIZE_STM32_HAL_CPU;
 * ```
 */
