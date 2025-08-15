#!/usr/bin/env python3
"""
STM32H753ZI RCC Register Analysis and Validation
=================================================

Complete analysis of RCC register dump against STM32H753ZI manufacturer specifications.
This script decodes register bit fields and validates against expected configurations.

Based on:
- STM32H753ZI Reference Manual (RM0433)
- STM32H753ZI datasheet specifications
- CMSIS register definitions from stm32h753xx.h

Author: AI Assistant
Date: 2024
"""

import sys
import json
from typing import Dict, Any, Optional


class STM32H7RCCAnalyzer:
    """Comprehensive RCC register analyzer for STM32H753ZI"""

    def __init__(self):
        """Initialize with STM32H753ZI RCC register specifications"""

        # Register offsets and addresses
        self.RCC_BASE = 0x58024400
        self.register_offsets = {
            "CR": 0x00,
            "HSICFGR": 0x04,
            "CRRCR": 0x08,
            "CSICFGR": 0x0C,
            "CFGR": 0x10,
            "D1CFGR": 0x18,
            "D2CFGR": 0x1C,
            "D3CFGR": 0x20,
            "PLLCKSELR": 0x28,
            "PLLCFGR": 0x2C,
            "PLL1DIVR": 0x30,
            "PLL1FRACR": 0x34,
            "PLL2DIVR": 0x38,
            "PLL2FRACR": 0x3C,
            "PLL3DIVR": 0x40,
            "PLL3FRACR": 0x44,
        }

        # Clock source definitions
        self.clock_sources = {
            "HSI": 64_000_000,  # 64 MHz internal RC oscillator
            "CSI": 4_000_000,  # 4 MHz low-power internal oscillator
            "HSE": 8_000_000,  # 8 MHz external crystal (Nucleo-144 default)
            "LSE": 32_768,  # 32.768 kHz external crystal
            "LSI": 32_000,  # 32 kHz internal RC oscillator
        }

        # Expected Nucleo-H753ZI configuration
        self.expected_config = {
            "HSE_VALUE": 8_000_000,  # 8 MHz crystal on Nucleo board
            "VCO_INPUT_MIN": 1_000_000,  # 1 MHz minimum VCO input
            "VCO_INPUT_MAX": 16_000_000,  # 16 MHz maximum VCO input
            "VCO_OUTPUT_MIN": 150_000_000,  # 150 MHz minimum VCO output
            "VCO_OUTPUT_MAX": 836_000_000,  # 836 MHz maximum VCO output
            "SYSCLK_MAX": 480_000_000,  # 480 MHz maximum system clock
            "HCLK_MAX": 240_000_000,  # 240 MHz maximum AHB clock
            "PCLK_MAX": 120_000_000,  # 120 MHz maximum APB clock
        }

    def parse_register_dump(self, dump_text: str) -> Dict[str, int]:
        """Parse STM32CubeProgrammer register dump output"""
        registers = {}
        lines = dump_text.strip().split("\n")

        for line in lines:
            if ":" in line and "0x58024" in line:
                # Parse format: "0x58024400: 00004025 4000091E 0000014F 20000096"
                parts = line.split(":")
                if len(parts) == 2:
                    addr_str = parts[0].strip()
                    values_str = parts[1].strip()

                    try:
                        base_addr = int(addr_str, 16)
                        values = values_str.split()

                        for i, value_str in enumerate(values):
                            addr = base_addr + (i * 4)
                            value = int(value_str, 16)

                            # Map to register names
                            offset = addr - self.RCC_BASE
                            for (
                                reg_name,
                                reg_offset,
                            ) in self.register_offsets.items():
                                if offset == reg_offset:
                                    registers[reg_name] = value
                                    break
                    except ValueError:
                        continue

        return registers

    def analyze_rcc_cr(self, value: int) -> Dict[str, Any]:
        """Analyze RCC_CR register (Clock Control Register)"""
        analysis = {
            "raw_value": f"0x{value:08X}",
            "fields": {},
            "status": "UNKNOWN",
            "warnings": [],
            "compliance": True,
        }

        # RCC_CR bit fields (from RM0433)
        fields = {
            "HSION": (value >> 0) & 0x1,  # HSI oscillator ON
            "HSIKERON": (value >> 1) & 0x1,  # HSI kernel oscillator ON
            "HSIRDY": (value >> 2) & 0x1,  # HSI oscillator ready
            "HSIDIV": (value >> 3) & 0x3,  # HSI oscillator divider
            "HSIDIVF": (value >> 5) & 0x1,  # HSI divider flag
            "CSION": (value >> 7) & 0x1,  # CSI oscillator ON
            "CSIRDY": (value >> 8) & 0x1,  # CSI oscillator ready
            "CSIKERON": (value >> 9) & 0x1,  # CSI kernel oscillator ON
            "RC48ON": (value >> 12) & 0x1,  # RC48 oscillator ON
            "RC48RDY": (value >> 13) & 0x1,  # RC48 oscillator ready
            "D1CKRDY": (value >> 14) & 0x1,  # D1 domain clocks ready
            "D2CKRDY": (value >> 15) & 0x1,  # D2 domain clocks ready
            "HSEON": (value >> 16) & 0x1,  # HSE oscillator ON
            "HSERDY": (value >> 17) & 0x1,  # HSE oscillator ready
            "HSEBYP": (value >> 18) & 0x1,  # HSE oscillator bypass
            "HSECSSON": (value >> 19) & 0x1,  # HSE clock security system ON
            "PLL1ON": (value >> 24) & 0x1,  # PLL1 ON
            "PLL1RDY": (value >> 25) & 0x1,  # PLL1 ready
            "PLL2ON": (value >> 26) & 0x1,  # PLL2 ON
            "PLL2RDY": (value >> 27) & 0x1,  # PLL2 ready
            "PLL3ON": (value >> 28) & 0x1,  # PLL3 ON
            "PLL3RDY": (value >> 29) & 0x1,  # PLL3 ready
        }

        analysis["fields"] = fields

        # Analyze clock source status
        if fields["HSEON"] and not fields["HSERDY"]:
            analysis["warnings"].append(
                "HSE enabled but not ready - clock startup failure"
            )
            analysis["compliance"] = False

        if fields["PLL1ON"] and not fields["PLL1RDY"]:
            analysis["warnings"].append(
                "PLL1 enabled but not ready - PLL lock failure"
            )
            analysis["compliance"] = False

        # Determine primary clock source status
        if fields["HSERDY"]:
            analysis["status"] = "HSE_READY"
        elif fields["HSIRDY"]:
            analysis["status"] = "HSI_READY"
        elif fields["CSIRDY"]:
            analysis["status"] = "CSI_READY"
        else:
            analysis["status"] = "NO_CLOCK_READY"
            analysis["warnings"].append(
                "No oscillator ready - critical failure"
            )
            analysis["compliance"] = False

        return analysis

    def analyze_rcc_cfgr(self, value: int) -> Dict[str, Any]:
        """Analyze RCC_CFGR register (Clock Configuration Register)"""
        analysis = {
            "raw_value": f"0x{value:08X}",
            "fields": {},
            "system_clock_source": "UNKNOWN",
            "warnings": [],
            "compliance": True,
        }

        # RCC_CFGR bit fields
        fields = {
            "SW": (value >> 0) & 0x7,  # System clock switch
            "SWS": (value >> 3) & 0x7,  # System clock switch status
            "STOPWUCK": (value >> 6) & 0x1,  # Wake-up clock selection
            "STOPKERWUCK": (value >> 7)
            & 0x1,  # Kernel wake-up clock selection
            "RTCPRE": (value >> 8) & 0x3F,  # RTC prescaler
            "HRTIMSEL": (value >> 14) & 0x1,  # HRTIM prescaler selection
            "TIMPRE": (value >> 15) & 0x1,  # Timer prescaler selection
            "MCO1PRE": (value >> 18) & 0xF,  # MCO1 prescaler
            "MCO1SEL": (value >> 22) & 0x7,  # MCO1 clock source selection
            "MCO2PRE": (value >> 25) & 0xF,  # MCO2 prescaler
            "MCO2SEL": (value >> 29) & 0x7,  # MCO2 clock source selection
        }

        analysis["fields"] = fields

        # Decode system clock source
        sw_sources = {0: "HSI", 1: "CSI", 2: "HSE", 3: "PLL1"}

        analysis["system_clock_source"] = sw_sources.get(
            fields["SW"], f"UNKNOWN({fields['SW']})"
        )
        analysis["system_clock_status"] = sw_sources.get(
            fields["SWS"], f"UNKNOWN({fields['SWS']})"
        )

        # Check for clock switch mismatch
        if fields["SW"] != fields["SWS"]:
            analysis["warnings"].append(
                f"Clock switch mismatch: SW={fields['SW']}, SWS={fields['SWS']}"
            )
            analysis["compliance"] = False

        return analysis

    def analyze_pllckselr(self, value: int) -> Dict[str, Any]:
        """Analyze RCC_PLLCKSELR register (PLL Clock Source Selection)"""
        analysis = {
            "raw_value": f"0x{value:08X}",
            "fields": {},
            "pll_sources": {},
            "warnings": [],
            "compliance": True,
        }

        # RCC_PLLCKSELR bit fields
        fields = {
            "PLLSRC": (value >> 0) & 0x3,  # PLL clock source selection
            "DIVM1": (value >> 4) & 0x3F,  # PLL1 prescaler
            "DIVM2": (value >> 12) & 0x3F,  # PLL2 prescaler
            "DIVM3": (value >> 20) & 0x3F,  # PLL3 prescaler
        }

        analysis["fields"] = fields

        # Decode PLL source
        pll_sources = {0: "HSI", 1: "CSI", 2: "HSE", 3: "No clock"}

        pll_source_name = pll_sources.get(fields["PLLSRC"], "UNKNOWN")
        analysis["pll_sources"]["source"] = pll_source_name

        # Calculate VCO input frequencies
        if pll_source_name in self.clock_sources:
            source_freq = self.clock_sources[pll_source_name]

            for pll_num, divm_field in [
                (1, "DIVM1"),
                (2, "DIVM2"),
                (3, "DIVM3"),
            ]:
                divm = fields[divm_field]
                if divm == 0:
                    analysis["warnings"].append(
                        f"PLL{pll_num} prescaler DIVM{pll_num}=0 (division by zero)"
                    )
                    analysis["compliance"] = False
                    vco_input = 0
                else:
                    vco_input = source_freq / divm

                analysis["pll_sources"][f"PLL{pll_num}_VCO_input"] = vco_input

                # Validate VCO input frequency range
                if (
                    vco_input < self.expected_config["VCO_INPUT_MIN"]
                    or vco_input > self.expected_config["VCO_INPUT_MAX"]
                ):
                    analysis["warnings"].append(
                        f"PLL{pll_num} VCO input {vco_input/1e6:.1f} MHz outside valid range "
                        f"({self.expected_config['VCO_INPUT_MIN']/1e6:.1f}-{self.expected_config['VCO_INPUT_MAX']/1e6:.1f} MHz)"
                    )
                    analysis["compliance"] = False

        return analysis

    def analyze_pllcfgr(self, value: int) -> Dict[str, Any]:
        """Analyze RCC_PLLCFGR register (PLL Configuration Register)"""
        analysis = {
            "raw_value": f"0x{value:08X}",
            "fields": {},
            "pll_config": {},
            "warnings": [],
            "compliance": True,
        }

        # RCC_PLLCFGR bit fields
        fields = {
            "PLL1FRACEN": (value >> 0) & 0x1,  # PLL1 fractional divider enable
            "PLL1VCOSEL": (value >> 1) & 0x1,  # PLL1 VCO selection
            "PLL1RGE": (value >> 2) & 0x3,  # PLL1 input frequency range
            "PLL2FRACEN": (value >> 4) & 0x1,  # PLL2 fractional divider enable
            "PLL2VCOSEL": (value >> 5) & 0x1,  # PLL2 VCO selection
            "PLL2RGE": (value >> 6) & 0x3,  # PLL2 input frequency range
            "PLL3FRACEN": (value >> 8) & 0x1,  # PLL3 fractional divider enable
            "PLL3VCOSEL": (value >> 9) & 0x1,  # PLL3 VCO selection
            "PLL3RGE": (value >> 10) & 0x3,  # PLL3 input frequency range
            "DIVP1EN": (value >> 16) & 0x1,  # PLL1 DIVP divider output enable
            "DIVQ1EN": (value >> 17) & 0x1,  # PLL1 DIVQ divider output enable
            "DIVR1EN": (value >> 18) & 0x1,  # PLL1 DIVR divider output enable
            "DIVP2EN": (value >> 19) & 0x1,  # PLL2 DIVP divider output enable
            "DIVQ2EN": (value >> 20) & 0x1,  # PLL2 DIVQ divider output enable
            "DIVR2EN": (value >> 21) & 0x1,  # PLL2 DIVR divider output enable
            "DIVP3EN": (value >> 22) & 0x1,  # PLL3 DIVP divider output enable
            "DIVQ3EN": (value >> 23) & 0x1,  # PLL3 DIVQ divider output enable
            "DIVR3EN": (value >> 24) & 0x1,  # PLL3 DIVR divider output enable
        }

        analysis["fields"] = fields

        # Decode input frequency ranges
        pll_ranges = {0: "1-2 MHz", 1: "2-4 MHz", 2: "4-8 MHz", 3: "8-16 MHz"}

        # Decode VCO ranges
        vco_ranges = {
            0: "Wide VCO (150-836 MHz)",
            1: "Medium VCO (150-420 MHz)",
        }

        for pll in [1, 2, 3]:
            pll_cfg = {}
            pll_cfg["input_range"] = pll_ranges.get(
                fields[f"PLL{pll}RGE"], "UNKNOWN"
            )
            pll_cfg["vco_range"] = vco_ranges.get(
                fields[f"PLL{pll}VCOSEL"], "UNKNOWN"
            )
            pll_cfg["fractional_enabled"] = bool(fields[f"PLL{pll}FRACEN"])
            pll_cfg["outputs_enabled"] = {
                "P": bool(fields[f"DIVP{pll}EN"]),
                "Q": bool(fields[f"DIVQ{pll}EN"]),
                "R": bool(fields[f"DIVR{pll}EN"]),
            }
            analysis["pll_config"][f"PLL{pll}"] = pll_cfg

        return analysis

    def analyze_pll1divr(
        self, value: int, vco_input_freq: Optional[float] = None
    ) -> Dict[str, Any]:
        """Analyze RCC_PLL1DIVR register (PLL1 Dividers Configuration)"""
        analysis = {
            "raw_value": f"0x{value:08X}",
            "fields": {},
            "output_frequencies": {},
            "warnings": [],
            "compliance": True,
        }

        # RCC_PLL1DIVR bit fields
        fields = {
            "DIVN1": (value >> 0) & 0x1FF,  # PLL1 multiplication factor (N)
            "DIVP1": (value >> 9) & 0x7F,  # PLL1 DIVP division factor
            "DIVQ1": (value >> 16) & 0x7F,  # PLL1 DIVQ division factor
            "DIVR1": (value >> 24) & 0x7F,  # PLL1 DIVR division factor
        }

        analysis["fields"] = fields

        # Calculate frequencies if VCO input is known
        if vco_input_freq:
            # VCO frequency = VCO_input * N (where N = DIVN1 + 1)
            n_multiplier = fields["DIVN1"] + 1
            vco_frequency = vco_input_freq * n_multiplier

            analysis["output_frequencies"]["VCO"] = vco_frequency

            # Validate VCO frequency range
            if (
                vco_frequency < self.expected_config["VCO_OUTPUT_MIN"]
                or vco_frequency > self.expected_config["VCO_OUTPUT_MAX"]
            ):
                analysis["warnings"].append(
                    f"PLL1 VCO frequency {vco_frequency/1e6:.1f} MHz outside valid range "
                    f"({self.expected_config['VCO_OUTPUT_MIN']/1e6:.1f}-{self.expected_config['VCO_OUTPUT_MAX']/1e6:.1f} MHz)"
                )
                analysis["compliance"] = False

            # Calculate output frequencies
            for div_name, div_field in [
                ("P", "DIVP1"),
                ("Q", "DIVQ1"),
                ("R", "DIVR1"),
            ]:
                div_value = fields[div_field]
                if div_value == 0:
                    analysis["warnings"].append(
                        f"PLL1 DIV{div_name} = 0 (division by zero)"
                    )
                    analysis["compliance"] = False
                else:
                    # For DIVP: actual divisor = DIVP1 + 1
                    # For DIVQ, DIVR: actual divisor = field value
                    if div_name == "P":
                        actual_divisor = div_value + 1
                        # DIVP must be even for STM32H7
                        if actual_divisor % 2 != 0:
                            analysis["warnings"].append(
                                f"PLL1 DIVP={actual_divisor} is odd (must be even for STM32H7)"
                            )
                            analysis["compliance"] = False
                    else:
                        actual_divisor = div_value

                    output_freq = vco_frequency / actual_divisor
                    analysis["output_frequencies"][
                        f"PLL1_{div_name}"
                    ] = output_freq

                    # Validate system clock frequency (PLL1_P is typically system clock)
                    if (
                        div_name == "P"
                        and output_freq > self.expected_config["SYSCLK_MAX"]
                    ):
                        analysis["warnings"].append(
                            f"PLL1_P frequency {output_freq/1e6:.1f} MHz exceeds maximum system clock "
                            f"({self.expected_config['SYSCLK_MAX']/1e6:.1f} MHz)"
                        )
                        analysis["compliance"] = False

        return analysis

    def generate_comprehensive_report(
        self, registers: Dict[str, int]
    ) -> Dict[str, Any]:
        """Generate comprehensive analysis report of all RCC registers"""

        report = {
            "timestamp": "2024-12-19",
            "device": "STM32H753ZI",
            "board": "NUCLEO-H753ZI",
            "rcc_base_address": f"0x{self.RCC_BASE:08X}",
            "registers_found": list(registers.keys()),
            "analysis": {},
            "overall_status": "UNKNOWN",
            "critical_issues": [],
            "warnings": [],
            "compliance_summary": {
                "registers_compliant": 0,
                "registers_total": 0,
                "overall_compliant": False,
            },
        }

        # Analyze each register
        analyses = {}

        if "CR" in registers:
            analyses["CR"] = self.analyze_rcc_cr(registers["CR"])

        if "CFGR" in registers:
            analyses["CFGR"] = self.analyze_rcc_cfgr(registers["CFGR"])

        if "PLLCKSELR" in registers:
            analyses["PLLCKSELR"] = self.analyze_pllckselr(
                registers["PLLCKSELR"]
            )

        if "PLLCFGR" in registers:
            analyses["PLLCFGR"] = self.analyze_pllcfgr(registers["PLLCFGR"])

        if "PLL1DIVR" in registers:
            # Get VCO input frequency from PLLCKSELR analysis if available
            vco_input = None
            if (
                "PLLCKSELR" in analyses
                and "PLL1_VCO_input" in analyses["PLLCKSELR"]["pll_sources"]
            ):
                vco_input = analyses["PLLCKSELR"]["pll_sources"][
                    "PLL1_VCO_input"
                ]
            analyses["PLL1DIVR"] = self.analyze_pll1divr(
                registers["PLL1DIVR"], vco_input
            )

        report["analysis"] = analyses

        # Collect all warnings and issues
        all_warnings = []
        critical_issues = []
        compliant_count = 0
        total_count = len(analyses)

        for reg_name, analysis in analyses.items():
            if "warnings" in analysis:
                for warning in analysis["warnings"]:
                    all_warnings.append(f"{reg_name}: {warning}")

            if "compliance" in analysis:
                if analysis["compliance"]:
                    compliant_count += 1
                else:
                    critical_issues.append(f"{reg_name} non-compliant")

        report["warnings"] = all_warnings
        report["critical_issues"] = critical_issues
        report["compliance_summary"] = {
            "registers_compliant": compliant_count,
            "registers_total": total_count,
            "overall_compliant": compliant_count == total_count
            and total_count > 0,
        }

        # Determine overall status
        if critical_issues:
            report["overall_status"] = "CRITICAL_ISSUES"
        elif all_warnings:
            report["overall_status"] = "WARNINGS"
        elif total_count > 0:
            report["overall_status"] = "COMPLIANT"
        else:
            report["overall_status"] = "NO_DATA"

        return report

    def print_formatted_report(self, report: Dict[str, Any]):
        """Print human-readable analysis report"""

        print("=" * 80)
        print("STM32H753ZI RCC Register Analysis Report")
        print("=" * 80)
        print(f"Device: {report['device']}")
        print(f"Board: {report['board']}")
        print(f"RCC Base Address: {report['rcc_base_address']}")
        print(f"Analysis Date: {report['timestamp']}")
        print(f"Overall Status: {report['overall_status']}")
        print()

        # Compliance Summary
        print("COMPLIANCE SUMMARY")
        print("-" * 40)
        comp = report["compliance_summary"]
        print(f"Registers Analyzed: {comp['registers_total']}")
        print(f"Compliant Registers: {comp['registers_compliant']}")
        print(
            f"Overall Compliant: {'✅ YES' if comp['overall_compliant'] else '❌ NO'}"
        )
        print()

        # Critical Issues
        if report["critical_issues"]:
            print("🚨 CRITICAL ISSUES")
            print("-" * 40)
            for issue in report["critical_issues"]:
                print(f"❌ {issue}")
            print()

        # Warnings
        if report["warnings"]:
            print("⚠️  WARNINGS")
            print("-" * 40)
            for warning in report["warnings"]:
                print(f"⚠️  {warning}")
            print()

        # Detailed Register Analysis
        print("DETAILED REGISTER ANALYSIS")
        print("-" * 40)

        for reg_name, analysis in report["analysis"].items():
            print(f"\n📋 {reg_name} Register Analysis")
            print(f"   Raw Value: {analysis['raw_value']}")

            if "status" in analysis:
                print(f"   Status: {analysis['status']}")

            if "system_clock_source" in analysis:
                print(
                    f"   System Clock Source: {analysis['system_clock_source']}"
                )
                print(
                    f"   System Clock Status: {analysis['system_clock_status']}"
                )

            if "fields" in analysis:
                print("   Key Bit Fields:")
                for field, value in analysis["fields"].items():
                    if isinstance(value, int):
                        print(f"     {field}: {value} (0x{value:X})")
                    else:
                        print(f"     {field}: {value}")

            if "output_frequencies" in analysis:
                print("   Calculated Frequencies:")
                for freq_name, freq_value in analysis[
                    "output_frequencies"
                ].items():
                    if freq_value > 1e6:
                        print(f"     {freq_name}: {freq_value/1e6:.2f} MHz")
                    elif freq_value > 1e3:
                        print(f"     {freq_name}: {freq_value/1e3:.2f} kHz")
                    else:
                        print(f"     {freq_name}: {freq_value:.2f} Hz")

            compliance_icon = (
                "✅" if analysis.get("compliance", False) else "❌"
            )
            print(f"   Compliance: {compliance_icon}")

        print("\n" + "=" * 80)


def main():
    """Main analysis function"""

    # Register dump from STM32CubeProgrammer (from previous session)
    register_dump = """0x58024400: 00004025 4000091E 0000014F 20000096
0x58024410: 00000000 00000000 00000000 00000000
0x58024420: 00000000 24003508 000003C8 00000000
0x58024430: 00000000 00000000 00000000 00000000
0x58024440: 00000000 00000000 00000000 00000000
0x58024450: 00000000 00000000 00000000 00000000
0x58024460: 00000000 00000000 00000000 00000000
0x58024470: 00000000 00000000 00000000 00000000"""

    # Initialize analyzer
    analyzer = STM32H7RCCAnalyzer()

    # Parse register values
    registers = analyzer.parse_register_dump(register_dump)

    if not registers:
        print("❌ ERROR: No valid registers found in dump")
        print("Expected registers at RCC base address 0x58024400")
        return 1

    print(f"📊 Found {len(registers)} RCC registers:")
    for reg_name, value in registers.items():
        print(f"   {reg_name}: 0x{value:08X}")
    print()

    # Generate comprehensive analysis
    report = analyzer.generate_comprehensive_report(registers)

    # Print formatted report
    analyzer.print_formatted_report(report)

    # Save report to JSON file
    output_file = "rcc_analysis_report.json"
    try:
        with open(output_file, "w") as f:
            json.dump(report, f, indent=2)
        print(f"\n💾 Detailed report saved to: {output_file}")
    except Exception as e:
        print(f"\n❌ Failed to save report: {e}")

    # Return exit code based on compliance
    return 0 if report["compliance_summary"]["overall_compliant"] else 1


if __name__ == "__main__":
    sys.exit(main())
