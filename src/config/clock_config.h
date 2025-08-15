/**
 * @file clock_config.h
 * @brief Centralized Clock Configuration - Single Source of Truth (SSOT)
 * @author STM32H753ZI Project Team
 * @date 2025-08-15
 *
 * @note This file centralizes ALL clock-related configuration and provides
 * a unified interface for clock management across the entire codebase.
 *
 * @warning NEVER hardcode clock frequencies elsewhere - always reference this
 * SSOT
 *
 * TODO: See .github/instructions/ssot-config.instructions.md for SSOT
 * principles
 * TODO: See .github/instructions/hardware.instructions.md for STM32H7
 * specifics
 */

#ifndef CLOCK_CONFIG_H
#define CLOCK_CONFIG_H

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

/* ==========================================================================
 */
/* Hardware Clock Sources (SSOT)                                             */
/* ==========================================================================
 */

/** @brief HSE crystal frequency on STM32H753ZI Nucleo (CRITICAL: Must match
 * hardware) */
#define HSE_CRYSTAL_FREQUENCY_HZ                                              \
    8000000UL // 8 MHz from ST-LINK MCO on Nucleo-H753ZI

/** @brief HSI internal oscillator frequency (STM32H7 specification) */
#define HSI_FREQUENCY_HZ 64000000UL // 64 MHz HSI

/** @brief CSI internal oscillator frequency (STM32H7 specification) */
#define CSI_FREQUENCY_HZ 4000000UL // 4 MHz CSI

/** @brief LSE crystal frequency (32.768 kHz standard) */
#define LSE_FREQUENCY_HZ 32768UL // 32.768 kHz LSE

/** @brief LSI internal oscillator frequency (STM32H7 specification) */
#define LSI_FREQUENCY_HZ 32000UL // ~32 kHz LSI (approximate)

/* ==========================================================================
 */
/* Voltage Scaling Configuration for 480MHz Operation (SSOT)                */
/* ==========================================================================
 */

/** @brief Voltage scaling configuration for 480MHz operation
 * @note VOS0 (Scale 0) is required for 480MHz operation on Revision V silicon
 * @note This requires STM32H753ZI with Revision V or later silicon
 */
#define VOLTAGE_SCALE_CONFIG PWR_REGULATOR_VOLTAGE_SCALE0 // VOS0 for 480MHz

/** @brief Enable overdrive mode for maximum performance */
#define ENABLE_OVERDRIVE_MODE 1

/** @brief Flash latency configuration for 480MHz @ VOS0 */
#define FLASH_LATENCY_480MHZ FLASH_LATENCY_4 // 4 wait states for 480MHz

/* ==========================================================================
 */
/* Target System Clock Frequencies (SSOT) - UPDATED FOR 480MHz OPERATION     */
/* ==========================================================================
 */

/** @brief Target system clock frequency (SYSCLK) - STM32H753ZI Revision V supports 480MHz */
#define TARGET_SYSCLK_FREQUENCY_HZ                                            \
    480000000UL // 480 MHz (maximum for Revision V with VOS0)

/** @brief Target AHB clock frequency (HCLK) */
#define TARGET_HCLK_FREQUENCY_HZ 240000000UL // 240 MHz (SYSCLK/2 for optimal performance)

/** @brief Target APB1 clock frequency (PCLK1) */
#define TARGET_PCLK1_FREQUENCY_HZ 120000000UL // 120 MHz (HCLK/2)

/** @brief Target APB2 clock frequency (PCLK2) */
#define TARGET_PCLK2_FREQUENCY_HZ 120000000UL // 120 MHz (HCLK/2)

/** @brief Target APB3 clock frequency (PCLK3) */
#define TARGET_PCLK3_FREQUENCY_HZ 120000000UL // 120 MHz (HCLK/2)

/** @brief Target APB4 clock frequency (PCLK4) */
#define TARGET_PCLK4_FREQUENCY_HZ 120000000UL // 120 MHz (HCLK/2)

/* ==========================================================================
 */
/* PLL Configuration (SSOT)                                                  */
/* ==========================================================================
 */

/** @brief PLL input frequency range validation */
#define PLL_INPUT_MIN_HZ 1000000UL  // 1 MHz minimum
#define PLL_INPUT_MAX_HZ 16000000UL // 16 MHz maximum

/** @brief PLL VCO frequency range validation */
#define PLL_VCO_MIN_HZ 192000000UL // 192 MHz minimum
#define PLL_VCO_MAX_HZ 960000000UL // 960 MHz maximum

/** @brief Primary PLL (PLL1) configuration for HSI source (preferred for
 * default Nucleo) */
#define PLL1_HSI_M_DIVIDER 4     // HSI/4 = 16MHz
#define PLL1_HSI_N_MULTIPLIER 60 // 16MHz*60 = 960MHz VCO
#define PLL1_HSI_P_DIVIDER 2     // 960MHz/2 = 480MHz SYSCLK
#define PLL1_HSI_Q_DIVIDER 4     // 960MHz/4 = 240MHz
#define PLL1_HSI_R_DIVIDER 2     // 960MHz/2 = 480MHz

/** @brief Primary PLL (PLL1) configuration for HSE source (requires solder
 * bridge changes) */
#define PLL1_HSE_M_DIVIDER 1     // HSE/1 = 8MHz
#define PLL1_HSE_N_MULTIPLIER 120 // 8MHz*120 = 960MHz VCO
#define PLL1_HSE_P_DIVIDER 2     // 960MHz/2 = 480MHz SYSCLK
#define PLL1_HSE_Q_DIVIDER 4     // 960MHz/4 = 240MHz
#define PLL1_HSE_R_DIVIDER 2     // 960MHz/2 = 480MHz

/* ==========================================================================
 */
/* Timing and Tick Configuration (SSOT)                                      */
/* ==========================================================================
 */

/** @brief System tick frequency (HAL and FreeRTOS must match) */
#define SYSTEM_TICK_FREQUENCY_HZ 1000UL // 1 kHz = 1ms ticks

/** @brief FreeRTOS tick rate (must match SYSTEM_TICK_FREQUENCY_HZ) */
#define FREERTOS_TICK_RATE_HZ 1000UL // 1 kHz = 1ms ticks

/** @brief HAL tick frequency (must match SYSTEM_TICK_FREQUENCY_HZ) */
#define HAL_TICK_FREQUENCY_HZ 1000UL // 1 kHz = 1ms ticks

/** @brief SysTick reload value calculation macro */
#define SYSTICK_RELOAD_VALUE(sysclk_hz)                                       \
    ((sysclk_hz / SYSTEM_TICK_FREQUENCY_HZ) - 1)

/* ==========================================================================
 */
/* Clock Source Priority and Fallback Strategy (SSOT)                       */
/* ==========================================================================
 */

/** @brief Clock source enumeration */
typedef enum {
    CLOCK_SOURCE_HSI = 0, /**< HSI internal oscillator (preferred for Nucleo
                             default config) */
    CLOCK_SOURCE_HSE,     /**< HSE crystal (requires solder bridge changes) */
    CLOCK_SOURCE_CSI,     /**< CSI internal oscillator (emergency) */
    CLOCK_SOURCE_INVALID  /**< Invalid/unknown source */
} ClockSource_t;

/** @brief Clock source priority order (0 = highest priority) */
#define CLOCK_SOURCE_PRIORITY_1                                               \
    CLOCK_SOURCE_HSI // HSI preferred (works with default Nucleo config)
#define CLOCK_SOURCE_PRIORITY_2                                               \
    CLOCK_SOURCE_HSE // HSE fallback (requires solder bridge changes)
#define CLOCK_SOURCE_PRIORITY_3                                               \
    CLOCK_SOURCE_CSI // CSI emergency (minimal performance)

/** @brief Clock startup timeout values */
#define HSE_STARTUP_TIMEOUT_MS 100 // 100ms HSE startup timeout
#define HSI_STARTUP_TIMEOUT_MS 10  // 10ms HSI startup timeout
#define CSI_STARTUP_TIMEOUT_MS 5   // 5ms CSI startup timeout
#define PLL_LOCK_TIMEOUT_MS 50     // 50ms PLL lock timeout

/* ==========================================================================
 */
/* Clock Management Function Declarations                                    */
/* ==========================================================================
 */

/**
 * @brief Initialize clock system with automatic source selection
 * @retval HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef Clock_Init(void);

/**
 * @brief Update SystemCoreClock and all dependent timing systems
 * @note MUST be called after any clock configuration change
 * @retval HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef Clock_UpdateSystemClock(void);

/**
 * @brief Get current active clock source
 * @retval ClockSource_t Active clock source
 */
ClockSource_t Clock_GetActiveSource(void);

/**
 * @brief Get actual system clock frequency
 * @retval uint32_t Frequency in Hz
 */
uint32_t Clock_GetSystemFrequency(void);

/**
 * @brief Get actual AHB clock frequency
 * @retval uint32_t Frequency in Hz
 */
uint32_t Clock_GetAHBFrequency(void);

/**
 * @brief Get actual APB1 clock frequency
 * @retval uint32_t Frequency in Hz
 */
uint32_t Clock_GetAPB1Frequency(void);

/**
 * @brief Get actual APB2 clock frequency
 * @retval uint32_t Frequency in Hz
 */
uint32_t Clock_GetAPB2Frequency(void);

/**
 * @brief Validate clock configuration against specifications
 * @retval HAL_StatusTypeDef HAL_OK if valid
 */
HAL_StatusTypeDef Clock_ValidateConfiguration(void);

/**
 * @brief Print detailed clock status to debug output
 * @note Only available in debug builds
 */
void Clock_PrintStatus(void);

/* ==========================================================================
 */
/* Clock Monitoring and Health Check                                         */
/* ==========================================================================
 */

/** @brief Clock health status */
typedef struct {
    ClockSource_t active_source; /**< Currently active clock source */
    uint32_t sysclk_frequency;   /**< Actual SYSCLK frequency */
    uint32_t hclk_frequency;     /**< Actual HCLK frequency */
    uint32_t pclk1_frequency;    /**< Actual PCLK1 frequency */
    uint32_t pclk2_frequency;    /**< Actual PCLK2 frequency */
    uint32_t systick_reload;     /**< SysTick reload value */
    bool pll_locked;             /**< PLL lock status */
    bool hse_ready;              /**< HSE ready status */
    bool hsi_ready;              /**< HSI ready status */
    uint32_t last_update_tick;   /**< Last health check tick */
} ClockHealth_t;

/**
 * @brief Get current clock health status
 * @param health Pointer to health structure to fill
 * @retval HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef Clock_GetHealth(ClockHealth_t *health);

/**
 * @brief Monitor clock stability and detect issues
 * @retval HAL_StatusTypeDef HAL_OK if stable
 */
HAL_StatusTypeDef Clock_MonitorStability(void);

/* ==========================================================================
 */
/* Compile-Time Validation (SSOT)                                            */
/* ==========================================================================
 */

// Validate HSE frequency matches hal_conf.h
#if defined(HSE_VALUE) && (HSE_VALUE != HSE_CRYSTAL_FREQUENCY_HZ)
#error                                                                        \
    "HSE_VALUE in stm32h7xx_hal_conf.h must match HSE_CRYSTAL_FREQUENCY_HZ in clock_config.h"
#endif

// Validate target frequencies are achievable
#if TARGET_SYSCLK_FREQUENCY_HZ > 480000000UL
#error "Target SYSCLK frequency exceeds STM32H753ZI maximum (480 MHz with VOS0)"
#endif

#if TARGET_HCLK_FREQUENCY_HZ > TARGET_SYSCLK_FREQUENCY_HZ
#error "Target HCLK frequency cannot exceed SYSCLK frequency"
#endif

// Validate PLL configuration produces correct frequencies
#define PLL1_HSE_VCO_FREQ                                                     \
    ((HSE_CRYSTAL_FREQUENCY_HZ / PLL1_HSE_M_DIVIDER) * PLL1_HSE_N_MULTIPLIER)
#define PLL1_HSE_SYSCLK_FREQ (PLL1_HSE_VCO_FREQ / PLL1_HSE_P_DIVIDER)

#if PLL1_HSE_SYSCLK_FREQ != TARGET_SYSCLK_FREQUENCY_HZ
#error "PLL1 HSE configuration does not produce target SYSCLK frequency"
#endif

#if PLL1_HSE_VCO_FREQ < PLL_VCO_MIN_HZ || PLL1_HSE_VCO_FREQ > PLL_VCO_MAX_HZ
#error "PLL1 HSE VCO frequency out of valid range"
#endif

// Validate tick configuration consistency
#if defined(configTICK_RATE_HZ) &&                                            \
    (configTICK_RATE_HZ != FREERTOS_TICK_RATE_HZ)
#error                                                                        \
    "FreeRTOS configTICK_RATE_HZ must match FREERTOS_TICK_RATE_HZ in clock_config.h"
#endif

/* ==========================================================================
 */
/* Clock-Dependent Peripheral Configuration (SSOT)                          */
/* ==========================================================================
 */

/** @brief UART baud rate clock calculation */
#define UART_BAUD_RATE_9600 9600UL
#define UART_BAUD_RATE_115200 115200UL
#define UART_BAUD_RATE_921600 921600UL

/** @brief SPI clock configuration */
#define SPI_MAX_FREQUENCY_HZ (TARGET_PCLK2_FREQUENCY_HZ / 2) // Max SPI freq

/** @brief I2C clock configuration */
#define I2C_STANDARD_MODE_HZ 100000UL   // 100 kHz standard mode
#define I2C_FAST_MODE_HZ 400000UL       // 400 kHz fast mode
#define I2C_FAST_MODE_PLUS_HZ 1000000UL // 1 MHz fast mode plus

/** @brief CAN clock configuration */
#define CAN_BITRATE_125K 125000UL // 125 kbps
#define CAN_BITRATE_250K 250000UL // 250 kbps
#define CAN_BITRATE_500K 500000UL // 500 kbps
#define CAN_BITRATE_1M 1000000UL  // 1 Mbps

/* ==========================================================================
 */
/* Debug and Testing Support                                                 */
/* ==========================================================================
 */

#ifdef DEBUG
/** @brief Enable detailed clock debugging */
#define CLOCK_DEBUG_ENABLED 1

/** @brief Clock debug print macro */
#define CLOCK_DEBUG_PRINT(fmt, ...)                                           \
    printf("[CLOCK] " fmt "\r\n", ##__VA_ARGS__)
#else
#define CLOCK_DEBUG_ENABLED 0
#define CLOCK_DEBUG_PRINT(fmt, ...)
#endif

/** @brief Clock configuration test function */
#ifdef FEATURE_UNIT_TESTS
HAL_StatusTypeDef Clock_RunSelfTest(void);
#endif

#endif /* CLOCK_CONFIG_H */

/**
 * @note Clock Configuration SSOT Rules:
 * 1. ALL clock frequencies MUST be defined here
 * 2. NO hardcoded frequencies anywhere else in the codebase
 * 3. Clock_UpdateSystemClock() MUST be called after any clock change
 * 4. All timing-dependent code MUST reference these definitions
 * 5. Compile-time validation prevents invalid configurations
 * 6. Runtime monitoring ensures clock stability
 */
