#!/usr/bin/env python3
"""
STM32CubeMX Programmatic Configuration Generator
Generates CubeMX .ioc file modifications for STM32H753ZI motor control project

This script programmatically modifies the CubeMX .ioc file to add all required
peripherals for the stepper motor control system without requiring CubeMX GUI.
"""

import re
import shutil
from datetime import datetime
from pathlib import Path


class CubeMXConfigurator:
    """Programmatic STM32CubeMX configuration for STM32H753ZI motor control
    project"""

    def __init__(self, workspace_root: str):
        self.workspace_root = Path(workspace_root)
        self.ioc_file = self.workspace_root / "code.ioc"
        self.backup_file = (
            self.workspace_root
            / f"code_backup_{datetime.now().strftime('%Y%m%d_%H%M%S')}.ioc"
        )

        # Configuration tracking
        self.ip_count = 6  # Starting count from existing config
        self.pin_assignments = {}
        self.peripheral_configs = {}

    def backup_current_config(self):
        """Create backup of current .ioc file"""
        if self.ioc_file.exists():
            shutil.copy2(self.ioc_file, self.backup_file)
            print(f"✅ Backup created: {self.backup_file}")
        else:
            raise FileNotFoundError(f"IOC file not found: {self.ioc_file}")

    def read_current_config(self):
        """Read current .ioc file configuration"""
        config_lines = []
        with open(self.ioc_file, "r") as f:
            config_lines = f.readlines()

        # Parse existing configuration
        for line in config_lines:
            line = line.strip()
            if line.startswith("Mcu.IPNb="):
                self.ip_count = int(line.split("=")[1])
            elif "=" in line and not line.startswith("#"):
                key, value = line.split("=", 1)
                if key.startswith("Mcu.Pin") and ".Signal=" in key:
                    pin_match = re.match(r"Mcu\.Pin\d+", key)
                    if pin_match:
                        pin_name = pin_match.group(0)
                        self.pin_assignments[pin_name] = value

        return config_lines

    def add_spi2_config(self):
        """Add SPI2 configuration for L6470 stepper drivers"""
        print("🔧 Adding SPI2 configuration for L6470 drivers...")

        spi2_config = {
            # IP configuration
            f"Mcu.IP{self.ip_count}": "SPI2",
            # SPI2 peripheral configuration
            "SPI2.Mode": "SPI_MODE_MASTER",
            "SPI2.Direction": "SPI_DIRECTION_2LINES",
            "SPI2.DataSize": "SPI_DATASIZE_8BIT",
            "SPI2.CLKPolarity": "SPI_POLARITY_HIGH",
            "SPI2.CLKPhase": "SPI_PHASE_2EDGE",
            "SPI2.NSS": "SPI_NSS_SOFT",
            "SPI2.BaudRatePrescaler": "SPI_BAUDRATEPRESCALER_32",
            "SPI2.FirstBit": "SPI_FIRSTBIT_MSB",
            "SPI2.TIMode": "SPI_TIMODE_DISABLE",
            "SPI2.CRCCalculation": "SPI_CRCCALCULATION_DISABLE",
            # STM32H7-specific SPI2 features
            "SPI2.NSSPMode": "SPI_NSS_PULSE_DISABLE",
            "SPI2.NSSPolarity": "SPI_NSS_POLARITY_LOW",
            "SPI2.FifoThreshold": "SPI_FIFO_THRESHOLD_01DATA",
            "SPI2.MasterSSIdleness": "SPI_MASTER_SS_IDLENESS_00CYCLE",
            "SPI2.MasterInterDataIdleness": (
                "SPI_MASTER_INTERDATA_IDLENESS_00CYCLE"
            ),
            "SPI2.MasterReceiverAutoSusp": "SPI_MASTER_RX_AUTOSUSP_DISABLE",
            "SPI2.MasterKeepIOState": "SPI_MASTER_KEEP_IO_STATE_ENABLE",
            "SPI2.IOSwap": "SPI_IO_SWAP_DISABLE",
            # Pin assignments
            "PB13.Mode": "Alternate",
            "PB13.Alternate": "GPIO_AF5_SPI2",
            "PB13.Signal": "SPI2_SCK",
            "PB13.Locked": "true",
            "PB14.Mode": "Alternate",
            "PB14.Alternate": "GPIO_AF5_SPI2",
            "PB14.Signal": "SPI2_MISO",
            "PB14.Locked": "true",
            "PB15.Mode": "Alternate",
            "PB15.Alternate": "GPIO_AF5_SPI2",
            "PB15.Signal": "SPI2_MOSI",
            "PB15.Locked": "true",
            # CS pin (software controlled)
            "PA9.Mode": "Output",
            "PA9.Signal": "GPIO_Output",
            "PA9.GPIO_Label": "MOTOR_SPI_CS",
            "PA9.Locked": "true",
            # L6470 control pins
            "PA10.Mode": "Input",
            "PA10.Signal": "GPIO_Input",
            "PA10.GPIO_Label": "MOTOR_FAULT",
            "PA10.GPIO_PuPd": "GPIO_PULLUP",
            "PA10.Locked": "true",
            "PA11.Mode": "Input",
            "PA11.Signal": "GPIO_Input",
            "PA11.GPIO_Label": "MOTOR_BUSY",
            "PA11.GPIO_PuPd": "GPIO_PULLUP",
            "PA11.Locked": "true",
        }

        self.peripheral_configs.update(spi2_config)
        self.ip_count += 1
        return spi2_config

    def add_i2c_config(self):
        """Add I2C1 and I2C2 configuration for AS5600 encoders"""
        print("🔧 Adding I2C1 & I2C2 configuration for AS5600 encoders...")

        i2c_config = {
            # I2C1 IP configuration
            f"Mcu.IP{self.ip_count}": "I2C1",
            # I2C1 peripheral configuration
            "I2C1.I2C_Mode": "I2C_Fast",
            "I2C1.I2C_Speed_Mode": "I2C_Fast",
            "I2C1.I2C_Analog_Filter": "I2C_ANALOGFILTER_ENABLE",
            "I2C1.I2C_Digital_Filter": "0",
            "I2C1.ClockSpeed": "400000",
            # I2C1 pins
            "PB6.Mode": "Alternate",
            "PB6.Alternate": "GPIO_AF4_I2C1",
            "PB6.Signal": "I2C1_SCL",
            "PB6.Locked": "true",
            "PB7.Mode": "Alternate",
            "PB7.Alternate": "GPIO_AF4_I2C1",
            "PB7.Signal": "I2C1_SDA",
            "PB7.Locked": "true",
        }

        self.peripheral_configs.update(i2c_config)
        self.ip_count += 1

        # I2C2 configuration
        i2c2_config = {
            # I2C2 IP configuration
            f"Mcu.IP{self.ip_count}": "I2C2",
            # I2C2 peripheral configuration
            "I2C2.I2C_Mode": "I2C_Fast",
            "I2C2.I2C_Speed_Mode": "I2C_Fast",
            "I2C2.I2C_Analog_Filter": "I2C_ANALOGFILTER_ENABLE",
            "I2C2.I2C_Digital_Filter": "0",
            "I2C2.ClockSpeed": "400000",
            # I2C2 pins
            "PB10.Mode": "Alternate",
            "PB10.Alternate": "GPIO_AF4_I2C2",
            "PB10.Signal": "I2C2_SCL",
            "PB10.Locked": "true",
            "PB11.Mode": "Alternate",
            "PB11.Alternate": "GPIO_AF4_I2C2",
            "PB11.Signal": "I2C2_SDA",
            "PB11.Locked": "true",
        }

        self.peripheral_configs.update(i2c2_config)
        self.ip_count += 1
        return {**i2c_config, **i2c2_config}

    def add_tim2_config(self):
        """Add TIM2 configuration for 1kHz control loop"""
        print("🔧 Adding TIM2 configuration for control loop...")

        tim2_config = {
            # TIM2 IP configuration
            f"Mcu.IP{self.ip_count}": "TIM2",
            # TIM2 peripheral configuration
            "TIM2.ClockSource": "TIM_CLOCKSOURCE_INTERNAL",
            "TIM2.Prescaler": "12000-1",
            "TIM2.Period": "10-1",
            "TIM2.CounterMode": "TIM_COUNTERMODE_UP",
            "TIM2.ClockDivision": "TIM_CLOCKDIVISION_DIV1",
            "TIM2.AutoReloadPreload": "TIM_AUTORELOAD_PRELOAD_ENABLE",
            # NVIC configuration for TIM2
            "NVIC.TIM2_IRQn": (
                "true\\:1\\:0\\:false\\:false\\:true\\:true\\:true\\:true"
            ),
        }

        self.peripheral_configs.update(tim2_config)
        self.ip_count += 1
        return tim2_config

    def add_fdcan_config(self):
        """Add FDCAN1 configuration for MCU-to-MCU communication"""
        print("🔧 Adding FDCAN1 configuration for inter-MCU communication...")

        fdcan_config = {
            # FDCAN1 IP configuration
            f"Mcu.IP{self.ip_count}": "FDCAN1",
            # FDCAN1 peripheral configuration
            "FDCAN1.CalculateTimeQuantum": "Disabled",
            "FDCAN1.CalculateBitRateNominal": "Disabled",
            "FDCAN1.NominalBitRate": "500000",
            "FDCAN1.NominalSyncJumpWidth": "1",
            "FDCAN1.DataBitRate": "2000000",
            "FDCAN1.DataSyncJumpWidth": "1",
            "FDCAN1.FrameFormat": "FDCAN_FRAME_FD_BRS",
            "FDCAN1.Mode": "FDCAN_MODE_NORMAL",
            "FDCAN1.AutoRetransmission": "ENABLE",
            "FDCAN1.TransmitPause": "DISABLE",
            "FDCAN1.ProtocolException": "ENABLE",
            # FDCAN1 pins
            "PD0.Mode": "Alternate",
            "PD0.Alternate": "GPIO_AF9_FDCAN1",
            "PD0.Signal": "FDCAN1_RX",
            "PD0.Locked": "true",
            "PD1.Mode": "Alternate",
            "PD1.Alternate": "GPIO_AF9_FDCAN1",
            "PD1.Signal": "FDCAN1_TX",
            "PD1.Locked": "true",
            # NVIC configuration for FDCAN1
            "NVIC.FDCAN1_IT0_IRQn": (
                "true\\:1\\:0\\:false\\:false\\:true\\:true\\:true\\:true"
            ),
            "NVIC.FDCAN1_IT1_IRQn": (
                "true\\:1\\:0\\:false\\:false\\:true\\:true\\:true\\:true"
            ),
        }

        self.peripheral_configs.update(fdcan_config)
        self.ip_count += 1
        return fdcan_config

    def resolve_pin_conflicts(self, config_lines):
        """Resolve pin conflicts by removing conflicting assignments"""
        print("🔧 Resolving pin conflicts...")

        # Remove conflicting GPIO assignments for PB14 and PB7
        conflicts_resolved = []
        updated_lines = []

        for line in config_lines:
            line_stripped = line.strip()

            # Remove PB14 GPIO assignment (conflicts with SPI2_MISO)
            pb14_gpio_conflict = (
                line_stripped.startswith("PB14.")
                and "GPIO_Output" in line_stripped
            )
            pb14_pin_conflict = line_stripped.startswith(
                "Mcu.Pin"
            ) and line_stripped.endswith("=PB14")
            if pb14_gpio_conflict or pb14_pin_conflict:
                conflicts_resolved.append(
                    "PB14 GPIO assignment removed (SPI2_MISO)"
                )
                continue

            # Remove PB7 GPIO assignment (conflicts with I2C1_SDA)
            pb7_gpio_conflict = (
                line_stripped.startswith("PB7.")
                and "GPIO_Output" in line_stripped
            )
            pb7_pin_conflict = line_stripped.startswith(
                "Mcu.Pin"
            ) and line_stripped.endswith("=PB7")
            if pb7_gpio_conflict or pb7_pin_conflict:
                conflicts_resolved.append(
                    "PB7 GPIO assignment removed (I2C1_SDA)"
                )
                continue

            updated_lines.append(line)

        if conflicts_resolved:
            print(f"   ✅ Resolved conflicts: {', '.join(conflicts_resolved)}")

        return updated_lines

    def write_updated_config(self):
        """Write updated configuration to .ioc file"""
        print("📝 Writing updated configuration...")

        # Read current config and resolve conflicts
        config_lines = self.read_current_config()
        config_lines = self.resolve_pin_conflicts(config_lines)

        # Add all peripheral configurations
        self.add_spi2_config()
        self.add_i2c_config()
        self.add_tim2_config()
        self.add_fdcan_config()

        # Update IP count
        self.peripheral_configs["Mcu.IPNb"] = str(self.ip_count)

        # Build new configuration
        updated_lines = []
        ip_count_updated = False

        for line in config_lines:
            line_stripped = line.strip()

            # Update IP count
            if line_stripped.startswith("Mcu.IPNb=") and not ip_count_updated:
                updated_lines.append(f"Mcu.IPNb={self.ip_count}\n")
                ip_count_updated = True
            # Skip existing lines that we're replacing
            elif any(
                line_stripped.startswith(key + "=")
                for key in self.peripheral_configs.keys()
            ):
                continue
            else:
                updated_lines.append(line)

        # Add new peripheral configurations
        for key, value in self.peripheral_configs.items():
            if not key.startswith("Mcu.IP") or key == "Mcu.IPNb":
                updated_lines.append(f"{key}={value}\n")

        # Write updated file
        with open(self.ioc_file, "w") as f:
            f.writelines(updated_lines)

        print(f"✅ Configuration written to {self.ioc_file}")

    def validate_configuration(self):
        """Validate the generated configuration"""
        print("🔍 Validating configuration...")

        with open(self.ioc_file, "r") as f:
            content = f.read()

        validations = [
            ("SPI2", "SPI2.Mode=SPI_MODE_MASTER"),
            ("I2C1", "I2C1.I2C_Mode=I2C_Fast"),
            ("I2C2", "I2C2.I2C_Mode=I2C_Fast"),
            ("TIM2", "TIM2.Prescaler=12000-1"),
            ("FDCAN1", "FDCAN1.NominalBitRate=500000"),
            ("Pin PB13", "PB13.Signal=SPI2_SCK"),
            ("Pin PB6", "PB6.Signal=I2C1_SCL"),
            ("Pin PD0", "PD0.Signal=FDCAN1_RX"),
        ]

        validation_results = []
        for name, check in validations:
            if check in content:
                validation_results.append(f"✅ {name}")
            else:
                validation_results.append(f"❌ {name}")

        print("Validation Results:")
        for result in validation_results:
            print(f"   {result}")

        return all("✅" in result for result in validation_results)

    def generate_summary(self):
        """Generate configuration summary"""
        summary = f"""
STM32CubeMX Configuration Summary
Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}

Peripherals Added:
├── SPI2 (L6470 Stepper Drivers)
│   ├── Pins: PB13(SCK), PB14(MISO), PB15(MOSI), PA9(CS)
│   ├── Mode: Master, 8-bit, Mode 3 (CPOL=1, CPHA=1)
│   └── Speed: ~7.5MHz (prescaler 32)
│
├── I2C1 (AS5600 Encoder 1)
│   ├── Pins: PB6(SCL), PB7(SDA)
│   └── Speed: 400kHz Fast Mode
│
├── I2C2 (AS5600 Encoder 2)
│   ├── Pins: PB10(SCL), PB11(SDA)
│   └── Speed: 400kHz Fast Mode
│
├── TIM2 (Control Loop Timer)
│   ├── Frequency: 1kHz
│   ├── Prescaler: 12000-1, Period: 10-1
│   └── Interrupt: Enabled
│
└── FDCAN1 (MCU Communication)
    ├── Pins: PD0(RX), PD1(TX)
    ├── Bitrate: 500kbps nominal, 2Mbps data
    └── Interrupts: Enabled

Pin Conflicts Resolved:
├── PB14: GPIO_Output → SPI2_MISO
└── PB7:  GPIO_Output → I2C1_SDA

Total IPs: {self.ip_count}
Backup: {self.backup_file.name}
"""
        return summary


def main():
    """Main execution function"""
    print("🚀 STM32CubeMX Programmatic Configuration Generator")
    print("   STM32H753ZI Motor Control Project")
    print("=" * 60)

    # Configuration
    workspace_root = "/workspaces/code"

    try:
        # Initialize configurator
        configurator = CubeMXConfigurator(workspace_root)

        # Create backup
        configurator.backup_current_config()

        # Generate updated configuration
        configurator.write_updated_config()

        # Validate results
        if configurator.validate_configuration():
            print("\n🎉 Configuration completed successfully!")
        else:
            print("\n⚠️  Configuration completed with validation warnings")

        # Show summary
        print(configurator.generate_summary())

        print("\n📋 Next Steps:")
        print("1. Open STM32CubeMX and load code.ioc")
        print("2. Verify peripheral configurations")
        print("3. Generate code (keep user code)")
        print("4. Build and test: cmake --build build")
        print("5. Validate SSOT: python3 scripts/validate_ssot.py")

    except Exception as e:
        print(f"❌ Error: {e}")
        return 1

    return 0


if __name__ == "__main__":
    exit(main())
