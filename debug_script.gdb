# GDB script for debugging Clock_Init()
target remote localhost:3333
monitor reset
load
break Clock_Init
break Clock_ConfigurePLL_HSI
break Clock_ConfigureSystemClocks
break Error_Handler
continue
