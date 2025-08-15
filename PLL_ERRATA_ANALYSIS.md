/**
 * STM32H753ZI PLL Configuration Analysis and Errata Summary
 * 
 * SILICON REVISION: Rev V (SUPPORTS 480 MHz)
 * ERRATA 2.2.20: "480 MHz maximum CPU frequency not available"
 * - Status: Rev Y, W = Partial workaround (limited to 400 MHz)  
 * - Status: Rev X, V = Partial workaround (480 MHz AVAILABLE)
 * 
 * CURRENT PROBLEM: Direct register programming not working correctly
 */

/* EXPECTED CONFIGURATION (from clock_config.h) */
#define EXPECTED_HSI_M  4    // HSI/4 = 16MHz PLL input
#define EXPECTED_HSI_N  60   // 16MHz*60 = 960MHz VCO
#define EXPECTED_HSI_P  2    // 960MHz/2 = 480MHz SYSCLK

/* ACTUAL HARDWARE REGISTERS (from live analysis) */
#define ACTUAL_DIVM1    32   // WRONG: Should be 4
#define ACTUAL_DIVN1    128  // WRONG: Should be 59 (60-1)
#define ACTUAL_DIVP1    1    // WRONG: Should be 1 (2-1)

/* ACTUAL FREQUENCY CALCULATION */
// Current: HSI(64MHz)/32 * 129 / 2 = 129 MHz
// Target:  HSI(64MHz)/4  * 60  / 2 = 480 MHz

/**
 * ROOT CAUSE ANALYSIS:
 * 
 * 1. Our constants are correct in clock_config.h
 * 2. Our direct register programming logic looks correct
 * 3. But the hardware shows completely different values
 * 
 * POSSIBLE CAUSES:
 * - Direct register programming being overridden by HAL
 * - Incorrect register bit positioning/masking
 * - Hardware register reset after our configuration
 * - Compile-time constant issue
 */

/**
 * ERRATA IMPACT ASSESSMENT:
 * 
 * Since we have Revision V silicon:
 * - 480 MHz operation SHOULD be supported
 * - VOS0 voltage scaling SHOULD be available
 * - The limitation only affects Rev Y/W devices
 * 
 * Therefore, our 129 MHz limitation is NOT due to the errata.
 * This is a configuration/software issue, not a hardware limitation.
 */
