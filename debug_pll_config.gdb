# GDB script for debugging PLL configuration issue
# STM32H753ZI Clock Configuration Debug Session
#
# Usage: arm-none-eabi-gdb build/Debug/stm32h753_ihm02a1.elf -x debug_pll_config.gdb

# Connect to STLink-gdb-server
target extended-remote localhost:61234

# Load symbols and reset
load
monitor reset halt

# Set breakpoints for PLL configuration debugging
break Clock_Init
break Clock_ConfigurePLL_HSI
break HAL_RCC_OscConfig
break Error_Handler

# STM32H7 Register addresses for manual inspection
set $RCC_BASE = 0x58024400
set $RCC_CR = $RCC_BASE + 0x000
set $RCC_CFGR = $RCC_BASE + 0x004
set $RCC_PLLCFGR = $RCC_BASE + 0x008
set $PWR_BASE = 0x58024800
set $PWR_CR3 = $PWR_BASE + 0x00C
set $PWR_CSR1 = $PWR_BASE + 0x004

# Define helper functions for register inspection
define show_rcc_status
    printf "=== RCC Status ===\n"
    printf "RCC_CR (0x%08x): 0x%08x\n", $RCC_CR, *(uint32_t*)$RCC_CR
    printf "  HSION: %d, HSIRDY: %d\n", (*(uint32_t*)$RCC_CR & 0x1), ((*(uint32_t*)$RCC_CR >> 2) & 0x1)
    printf "  PLL1ON: %d, PLL1RDY: %d\n", ((*(uint32_t*)$RCC_CR >> 24) & 0x1), ((*(uint32_t*)$RCC_CR >> 25) & 0x1)
    printf "RCC_CFGR (0x%08x): 0x%08x\n", $RCC_CFGR, *(uint32_t*)$RCC_CFGR
    printf "  SW: %d, SWS: %d\n", (*(uint32_t*)$RCC_CFGR & 0x7), ((*(uint32_t*)$RCC_CFGR >> 3) & 0x7)
    printf "RCC_PLLCFGR (0x%08x): 0x%08x\n", $RCC_PLLCFGR, *(uint32_t*)$RCC_PLLCFGR
    set $pllcfgr = *(uint32_t*)$RCC_PLLCFGR
    printf "  PLLSRC: %d, PLLM: %d, PLLN: %d\n", ($pllcfgr & 0x3), (($pllcfgr >> 4) & 0x3F), (($pllcfgr >> 8) & 0x7F)
    printf "  PLLPEN: %d, PLLP: %d\n", (($pllcfgr >> 16) & 0x1), (($pllcfgr >> 17) & 0x1)
    printf "==================\n"
end

define show_pwr_status
    printf "=== PWR Status ===\n"
    printf "PWR_CR3 (0x%08x): 0x%08x\n", $PWR_CR3, *(uint32_t*)$PWR_CR3
    printf "PWR_CSR1 (0x%08x): 0x%08x\n", $PWR_CSR1, *(uint32_t*)$PWR_CSR1
    printf "  VOSRDY: %d\n", ((*(uint32_t*)$PWR_CSR1 >> 13) & 0x1)
    printf "==================\n"
end

define debug_pll_step
    printf "\n=== PLL Debug Step ===\n"
    show_rcc_status
    show_pwr_status
    printf "Press Enter to continue...\n"
    shell read
end

# Initial state
printf "Starting PLL configuration debug session...\n"
printf "Breakpoints set at key functions. Use 'continue' to start.\n"
printf "Use 'debug_pll_step' command at any breakpoint for detailed register inspection.\n"
