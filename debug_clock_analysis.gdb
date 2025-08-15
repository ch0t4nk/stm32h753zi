# GDB Script for Clock_Init() Debugging
# STM32H753ZI Clock Configuration Analysis

# Connect to ST-LINK GDB server
target extended-remote localhost:3333

# Load symbols from ELF file
file build/Debug/stm32h753_ihm02a1.elf

# Reset and halt the target
monitor reset halt

# Set breakpoints for clock debugging
break Clock_Init
break Clock_ConfigureHSI
break Clock_ConfigurePLL_HSI  
break Clock_ConfigureSystemClocks
break Clock_UpdateSystemClock
break Error_Handler

# Print initial register state
printf "\n=== INITIAL REGISTER STATE ===\n"
printf "RCC_CR:     0x%08x\n", *(unsigned int*)0x58024400
printf "RCC_CFGR:   0x%08x\n", *(unsigned int*)0x58024410
printf "RCC_PLLCFGR: 0x%08x\n", *(unsigned int*)0x58024428
printf "RCC_PLL1DIVR: 0x%08x\n", *(unsigned int*)0x5802442C

# Continue execution
continue

# When Clock_Init() is hit, step through the process
printf "\n=== ENTERED Clock_Init() ===\n"

# Continue to HSI configuration
continue
printf "\n=== AFTER Clock_ConfigureHSI() ===\n"
printf "RCC_CR:     0x%08x\n", *(unsigned int*)0x58024400
printf "HSI Ready:  %s\n", (*(unsigned int*)0x58024400 & 0x400) ? "YES" : "NO"

# Continue to PLL configuration
continue  
printf "\n=== AFTER Clock_ConfigurePLL_HSI() ===\n"
printf "RCC_CR:     0x%08x\n", *(unsigned int*)0x58024400
printf "PLL Ready:  %s\n", (*(unsigned int*)0x58024400 & 0x2000000) ? "YES" : "NO"
printf "RCC_PLLCFGR: 0x%08x\n", *(unsigned int*)0x58024428
printf "RCC_PLL1DIVR: 0x%08x\n", *(unsigned int*)0x5802442C

# Continue to system clock configuration
continue
printf "\n=== ENTERING Clock_ConfigureSystemClocks() ===\n"
printf "Current SYSCLK source (RCC_CFGR[2:0]): %d\n", (*(unsigned int*)0x58024410 >> 3) & 0x7

# Step through the system clock configuration
step 20
printf "RCC_CFGR after config attempt: 0x%08x\n", *(unsigned int*)0x58024410
printf "SYSCLK source after config: %d\n", (*(unsigned int*)0x58024410 >> 3) & 0x7

# Continue to SystemCoreClock update
continue
printf "\n=== AFTER Clock_UpdateSystemClock() ===\n"
printf "SystemCoreClock: %d Hz\n", SystemCoreClock

# Print final state
printf "\n=== FINAL REGISTER STATE ===\n"
printf "RCC_CR:     0x%08x\n", *(unsigned int*)0x58024400
printf "RCC_CFGR:   0x%08x\n", *(unsigned int*)0x58024410
printf "SYSCLK Source: %d (0=HSI, 1=CSI, 2=HSE, 3=PLL)\n", (*(unsigned int*)0x58024410 >> 3) & 0x7
printf "PLL Ready:  %s\n", (*(unsigned int*)0x58024400 & 0x2000000) ? "YES" : "NO"
printf "HSI Ready:  %s\n", (*(unsigned int*)0x58024400 & 0x400) ? "YES" : "NO"
printf "HSE Ready:  %s\n", (*(unsigned int*)0x58024400 & 0x20000) ? "YES" : "NO"

# Print fault status registers
printf "\n=== FAULT STATUS ===\n"
printf "CFSR: 0x%08x\n", *(unsigned int*)0xE000ED28
printf "HFSR: 0x%08x\n", *(unsigned int*)0xE000ED2C
printf "DFSR: 0x%08x\n", *(unsigned int*)0xE000ED30

# Continue execution to see if any errors occur
continue

printf "\n=== DEBUGGING COMPLETE ===\n"
quit
