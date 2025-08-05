#!/usr/bin/env python3
"""
STM32H753ZI Project Structure Generator
Follows file-organization.instructions.md and ssot-config.instructions.md
Integrates directly into our main src/ structure
"""

import os
import shutil
from pathlib import Path


def create_hal_layer():
    """Create STM32H7 HAL integration layer in src/drivers/"""

    print("🔧 Creating STM32H7 HAL integration layer...")

    # Create driver directories following our instructions
    hal_dirs = [
        "src/drivers/hal",
        "src/drivers/bsp",
        "src/drivers/l6470",
        "src/drivers/as5600",
        "src/drivers/adaptation",
    ]

    for dir_path in hal_dirs:
        os.makedirs(dir_path, exist_ok=True)
        print(f"✅ Created: {dir_path}")

    # Create basic HAL configuration header
    hal_config = """/* STM32H7 HAL Configuration for Stepper Motor Project
 * Generated following hardware-config.instructions.md
 * SSOT compliance: hardware_config.h integration
 */

#ifndef STM32H7XX_HAL_CONF_H
#define STM32H7XX_HAL_CONF_H

// TODO: See .github/instructions/hardware-pins.instructions.md for pin mapping details
// TODO: See .github/instructions/stm32h7-spi-l6470.instructions.md for SPI configuration

#include "config/hardware_config.h"  // SSOT hardware definitions

/* STM32H7 HAL Module Configuration */
#define HAL_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED      // L6470 communication
#define HAL_I2C_MODULE_ENABLED      // AS5600 encoders
#define HAL_UART_MODULE_ENABLED     // Debug/control interface
#define HAL_FDCAN_MODULE_ENABLED    // CAN communication
#define HAL_ETH_MODULE_ENABLED      // Ethernet communication
#define HAL_GPIO_MODULE_ENABLED     // GPIO control

/* Clock Configuration (from hardware_config.h) */
#define HSE_VALUE               ((uint32_t)SYSTEM_CLOCK_HSE_HZ)
#define HSI_VALUE               ((uint32_t)16000000U)
#define CSI_VALUE               ((uint32_t)4000000U)
#define HSE_STARTUP_TIMEOUT     ((uint32_t)100U)
#define HSI_STARTUP_TIMEOUT     ((uint32_t)5000U)
#define CSI_STARTUP_TIMEOUT     ((uint32_t)5000U)

/* SPI Configuration for L6470 */
#define HAL_SPI_TIMEOUT_DEFAULT_VALUE   5000U

/* I2C Configuration for AS5600 */
#define HAL_I2C_TIMEOUT_DEFAULT_VALUE   10000U

#endif /* STM32H7XX_HAL_CONF_H */
"""

    with open("src/drivers/hal/stm32h7xx_hal_conf.h", "w") as f:
        f.write(hal_config)

    print("✅ Created: src/drivers/hal/stm32h7xx_hal_conf.h")


def create_main_application():
    """Create main application following our structure"""

    print("🚀 Creating main application structure...")

    os.makedirs("src/application", exist_ok=True)

    main_c = """/* STM32H753ZI Stepper Motor Control - Main Application
 * Generated following file-organization.instructions.md
 * SSOT compliance: All configs from src/config/
 */

#include "config/hardware_config.h"
#include "config/motor_config.h" 
#include "config/comm_config.h"
#include "config/safety_config.h"
#include "config/build_config.h"

// TODO: See .github/instructions/stm32h7-spi-l6470.instructions.md for L6470 integration
// TODO: See .github/instructions/stm32h7-i2c-as5600.instructions.md for AS5600 integration

/* HAL Headers */
#include "stm32h7xx_hal.h"
#include "drivers/hal/stm32h7xx_hal_conf.h"

/* Driver Headers (to be implemented) */
// #include "drivers/l6470/l6470_driver.h"
// #include "drivers/as5600/as5600_driver.h"

/**
 * @brief System Clock Configuration
 * @note Follows SSOT hardware_config.h definitions
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    /* Configure main PLL from SSOT config */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    
    /* TODO: Use SSOT clock configuration from hardware_config.h */
    RCC_OscInitStruct.PLL.PLLM = 4;      // To be moved to SSOT
    RCC_OscInitStruct.PLL.PLLN = 240;    // To be moved to SSOT  
    RCC_OscInitStruct.PLL.PLLP = 2;      // To be moved to SSOT
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    
    /* Configure clock domains from SSOT */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 |
                                  RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    
    /* TODO: Use SSOT APB clock divisions from hardware_config.h */
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief Error Handler
 * @note Follows safety-systems.instructions.md
 */
void Error_Handler(void) {
    /* TODO: Implement safety shutdown following safety_config.h */
    __disable_irq();
    while (1) {
        /* Emergency stop - all motors halt */
    }
}

/**
 * @brief Main function
 * @note Entry point following our project architecture
 */
int main(void) {
    /* Initialize HAL */
    HAL_Init();
    
    /* Configure system clock from SSOT */
    SystemClock_Config();
    
    /* TODO: Initialize peripherals following instruction files:
     * - SPI1 for L6470 (stm32h7-spi-l6470.instructions.md)
     * - I2C1/I2C2 for AS5600 (stm32h7-i2c-as5600.instructions.md)  
     * - UART3 for debug (stm32h7-uart-protocol.instructions.md)
     * - Safety systems (safety-systems.instructions.md)
     */
    
    /* Main control loop */
    while (1) {
        /* TODO: Implement stepper control loop */
        HAL_Delay(1);
    }
}
"""

    with open("src/application/main.c", "w") as f:
        f.write(main_c)

    print("✅ Created: src/application/main.c")


def copy_reference_assets():
    """Copy essential STM32H7 HAL drivers from reference"""

    print("📂 Copying STM32H7 HAL drivers from reference...")

    # Define source paths in our reference materials
    hal_source = Path(
        "00_reference/STM32Cube_FW_H7_V1.12.0/Drivers/STM32H7xx_HAL_Driver"
    )
    cmsis_source = Path("00_reference/STM32Cube_FW_H7_V1.12.0/Drivers/CMSIS")

    if hal_source.exists():
        print(f"✅ Found HAL drivers at: {hal_source}")

        # Copy essential HAL files to our structure
        hal_dest = Path("src/drivers/hal/STM32H7xx_HAL_Driver")
        os.makedirs(hal_dest, exist_ok=True)

        # Copy key include files
        if (hal_source / "Inc").exists():
            shutil.copytree(hal_source / "Inc", hal_dest / "Inc", dirs_exist_ok=True)
            print("✅ Copied HAL include files")
    else:
        print("⚠️  HAL drivers not found in reference - will need manual download")

    if cmsis_source.exists():
        print(f"✅ Found CMSIS at: {cmsis_source}")

        # Copy CMSIS device files
        cmsis_dest = Path("src/drivers/hal/CMSIS")
        os.makedirs(cmsis_dest, exist_ok=True)

        if (cmsis_source / "Device/ST/STM32H7xx").exists():
            shutil.copytree(
                cmsis_source / "Device/ST/STM32H7xx",
                cmsis_dest / "Device/ST/STM32H7xx",
                dirs_exist_ok=True,
            )
            print("✅ Copied CMSIS device files")
    else:
        print("⚠️  CMSIS not found in reference - will need manual download")


def cleanup_temp_folders():
    """Remove any temporary CubeMX folders"""

    print("🧹 Cleaning up temporary folders...")

    temp_folders = [
        "cubemx_generated",
        "cubemx_basic",
        "mcsdk_generated",
        "spn2_mcsdk_generated",
    ]

    for folder in temp_folders:
        if os.path.exists(folder):
            shutil.rmtree(folder)
            print(f"🗑️  Removed: {folder}")


def update_cmake_integration():
    """Update CMakeLists.txt to include our new structure"""

    print("🔨 Updating CMake integration...")

    # Check if CMakeLists.txt exists
    if os.path.exists("CMakeLists.txt"):
        print("ℹ️  CMakeLists.txt exists - will need manual integration")
        print("💡 Add the following directories to your CMakeLists.txt:")
        print("   - src/drivers/hal")
        print("   - src/drivers/l6470")
        print("   - src/drivers/as5600")
        print("   - src/application")
    else:
        print("ℹ️  No CMakeLists.txt found - following build-system.instructions.md")


def main():
    """Main function following our project instructions"""

    print("🎯 STM32H753ZI Project Structure Generator")
    print(
        "📋 Following file-organization.instructions.md and ssot-config.instructions.md"
    )
    print("=" * 70)

    # Clean up any temporary folders first
    cleanup_temp_folders()

    # Create our proper structure
    create_hal_layer()
    create_main_application()
    copy_reference_assets()
    update_cmake_integration()

    print("\n✅ Project structure created following our instructions!")
    print("\n📁 Structure created:")
    print("   src/drivers/hal/          - STM32H7 HAL integration")
    print("   src/drivers/l6470/        - L6470 stepper drivers (pending)")
    print("   src/drivers/as5600/       - AS5600 encoder drivers (pending)")
    print("   src/drivers/adaptation/   - Integration layer (pending)")
    print("   src/application/          - Main application")

    print("\n🎯 Next steps:")
    print("1. Review generated src/application/main.c")
    print("2. Follow stm32h7-spi-l6470.instructions.md for L6470 integration")
    print("3. Follow stm32h7-i2c-as5600.instructions.md for AS5600 integration")
    print("4. Update SSOT configs in src/config/ as needed")
    print("5. Build using existing CMake system")


if __name__ == "__main__":
    main()
