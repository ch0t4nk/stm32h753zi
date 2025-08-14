#!/usr/bin/env python3
"""
STM32CubeMX CLI Project Generator
Generates STM32H753ZI NUCLEO project with required peripherals
"""

import os
import subprocess
from pathlib import Path


def create_cubemx_script():
    """Create STM32CubeMX script file for CLI generation"""

    cubemx_script = """# STM32CubeMX CLI Script for STM32H753ZI Project
# Generated for X-CUBE-SPN2 + MCSDK integration

# Load STM32H753ZI MCU
load part STM32H753ZITx

# Project configuration
project name stm32h753zi_stepper_control
project path /workspaces/code/cubemx_generated
project toolchain Makefile

# Clock configuration
clock HCLK 480
clock APB1 120
clock APB2 120
clock APB4 120

# SPI1 configuration (for L6470 daisy chain)
mode SPI1 Full-Duplex_Master
param SPI1 Data_Size SPI_DATASIZE_8BIT
param SPI1 Prescaler SPI_BAUDRATEPRESCALER_64
param SPI1 NSS Software

# I2C1 configuration (for AS5600 encoder 1)
mode I2C1 I2C
param I2C1 ClockSpeed 400000

# I2C2 configuration (for AS5600 encoder 2)
mode I2C2 I2C
param I2C2 ClockSpeed 400000

# USART3 configuration (Virtual COM Port)
mode USART3 Asynchronous
param USART3 BaudRate 115200
param USART3 WordLength UART_WORDLENGTH_8B
param USART3 Parity UART_PARITY_NONE
param USART3 StopBits UART_STOPBITS_1

# FDCAN1 configuration
mode FDCAN1 Activate
param FDCAN1 NominalBitRate 1000000

# ETH configuration
mode ETH RMII
param ETH PHY_Address 0

# GPIO configuration for LEDs (board support)
mode PB0 Output  # LED_GREEN
mode PB7 Output  # LED_BLUE
mode PB14 Output # LED_RED

# GPIO configuration for User Button
mode PC13 EXTI13

# Generate code
generate code
exit
"""

    script_path = Path("/workspaces/code/scripts/cubemx_project.script")
    with open(script_path, "w") as f:
        f.write(cubemx_script)

    return script_path


def run_cubemx_cli():
    """Run STM32CubeMX in CLI mode"""

    script_path = create_cubemx_script()

    # Check if STM32CubeMX is available
    cubemx_paths = [
        "/opt/STMicroelectronics/STM32CubeMX/STM32CubeMX",
        "/usr/local/STMicroelectronics/STM32CubeMX/STM32CubeMX",
        "STM32CubeMX",  # If in PATH
    ]

    cubemx_exe = None
    for path in cubemx_paths:
        if (
            os.path.exists(path)
            or subprocess.run(
                ["which", path.split("/")[-1]], capture_output=True
            ).returncode
            == 0
        ):
            cubemx_exe = path
            break

    if not cubemx_exe:
        print(
            "❌ STM32CubeMX not found. Please install or use manual approach."
        )
        print("📝 Script created at:", script_path)
        print("💡 You can run this script manually in STM32CubeMX CLI mode")
        return False

    try:
        print(f"🚀 Running STM32CubeMX CLI with script: {script_path}")
        result = subprocess.run(
            [cubemx_exe, "-q", str(script_path)],
            capture_output=True,
            text=True,
            timeout=300,
        )

        if result.returncode == 0:
            print("✅ STM32CubeMX project generated successfully!")
            print("📁 Project location: /workspaces/code/cubemx_generated/")
            return True
        else:
            print(f"❌ STM32CubeMX failed: {result.stderr}")
            return False

    except subprocess.TimeoutExpired:
        print("⏰ STM32CubeMX CLI timed out")
        return False
    except Exception as e:
        print(f"❌ Error running STM32CubeMX: {e}")
        return False


def main():
    """Main function"""
    print("🎯 STM32H753ZI CubeMX Project Generator")
    print("=" * 50)

    # Create output directory
    os.makedirs("/workspaces/code/cubemx_generated", exist_ok=True)

    # Try CLI generation
    success = run_cubemx_cli()

    if not success:
        print("\n💡 Alternative: Manual CubeMX Project Creation")
        print("1. Open STM32CubeMX GUI")
        print("2. Select NUCLEO-H753ZI board")
        print("3. Configure peripherals as shown in the script")
        print("4. Generate code to /workspaces/code/cubemx_generated/")
        print("5. We'll integrate X-CUBE-SPN2 + MCSDK manually")


if __name__ == "__main__":
    main()
