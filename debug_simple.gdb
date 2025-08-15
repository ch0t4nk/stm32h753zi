# Simple STM32H753ZI Clock Debug Script
# Compatible version for ST-LINK GDB server

# Connect to ST-LINK GDB server
target extended-remote localhost:3333

# Load the program
load

# Soft reset and halt
monitor halt

# Check initial register state
printf "\n=== INITIAL STATE ===\n"
x/1wx 0x58024400
printf "RCC_CR: 0x%08x\n", *(unsigned int*)0x58024400
printf "RCC_CFGR: 0x%08x\n", *(unsigned int*)0x58024410

# Set breakpoints
break Clock_Init
break Clock_ConfigureHSI
break Clock_ConfigurePLL_HSI
break Clock_ConfigureSystemClocks
break Error_Handler

# Start execution
continue
