#!/usr/bin/env python3
"""
Enhanced Clock Configuration Analysis Tool
Compares before/after register states and validates the 8MHz HSE fix
"""

import subprocess
import json
import time
import sys
from pathlib import Path


def run_stm32_programmer_cmd(cmd_args):
    """Execute STM32CubeProgrammer command and capture output"""
    try:
        cmd = ["STM32_Programmer_CLI"] + cmd_args
        result = subprocess.run(
            cmd, capture_output=True, text=True, timeout=30
        )
        return result.returncode == 0, result.stdout, result.stderr
    except subprocess.TimeoutExpired:
        return False, "", "Command timed out"
    except Exception as e:
        return False, "", str(e)


def analyze_rcc_registers():
    """Read and analyze current RCC register state"""
    print("🔍 Reading current RCC register state...")

    # Key RCC registers to check
    registers = {
        "RCC_CR": 0x58024400,  # Clock control
        "RCC_CFGR": 0x58024410,  # Clock configuration
        "RCC_PLLCKSELR": 0x58024428,  # PLL clock source selection
        "RCC_PLLCFGR": 0x5802442C,  # PLL configuration
        "RCC_PLL1DIVR": 0x58024430,  # PLL1 dividers
        "RCC_CSR": 0x58024474,  # Clock control & status
    }

    register_values = {}

    for reg_name, address in registers.items():
        success, stdout, stderr = run_stm32_programmer_cmd(
            ["-c", "port=SWD", "-r32", hex(address), "1"]
        )
        if success and stdout:
            # Parse register value from output
            lines = stdout.strip().split("\n")
            for line in lines:
                if hex(address).upper() in line.upper():
                    parts = line.split()
                    if len(parts) >= 2:
                        register_values[reg_name] = int(parts[1], 16)
                        break
        if reg_name not in register_values:
            print(f"⚠️ Failed to read {reg_name} at {hex(address)}")
            register_values[reg_name] = None

    return register_values


def analyze_clock_source(rcc_cfgr):
    """Analyze system clock source from RCC_CFGR"""
    if rcc_cfgr is None:
        return "UNKNOWN", "Failed to read RCC_CFGR"

    sws = (rcc_cfgr >> 3) & 0x7  # SWS bits [5:3]
    sw = rcc_cfgr & 0x7  # SW bits [2:0]

    clock_sources = {0: "HSI", 1: "CSI", 2: "HSE", 3: "PLL1"}

    return clock_sources.get(sws, f"UNKNOWN({sws})"), clock_sources.get(
        sw, f"UNKNOWN({sw})"
    )


def analyze_pll_config(rcc_cr, rcc_pllckselr, rcc_pll1divr):
    """Analyze PLL configuration and status"""
    if rcc_cr is None:
        return {"status": "UNKNOWN", "ready": False}

    pll_on = bool(rcc_cr & (1 << 24))  # PLL1ON bit
    pll_ready = bool(rcc_cr & (1 << 25))  # PLL1RDY bit

    config = {
        "enabled": pll_on,
        "ready": pll_ready,
        "status": "ON" if pll_on else "OFF",
    }

    if rcc_pllckselr is not None:
        pll_src = rcc_pllckselr & 0x3  # PLLSRC bits [1:0]
        pll_m = (rcc_pllckselr >> 4) & 0x3F  # DIVM1 bits [9:4]

        pll_sources = {0: "HSI", 1: "CSI", 2: "HSE", 3: "NONE"}
        config["source"] = pll_sources.get(pll_src, f"UNKNOWN({pll_src})")
        config["pllm"] = pll_m

    if rcc_pll1divr is not None:
        pll_n = ((rcc_pll1divr >> 0) & 0x1FF) + 1  # DIVN1 bits [8:0] + 1
        pll_p = ((rcc_pll1divr >> 9) & 0x7F) + 1  # DIVP1 bits [15:9] + 1
        pll_q = ((rcc_pll1divr >> 16) & 0x7F) + 1  # DIVQ1 bits [22:16] + 1
        pll_r = ((rcc_pll1divr >> 24) & 0x7F) + 1  # DIVR1 bits [30:24] + 1

        config.update(
            {"plln": pll_n, "pllp": pll_p, "pllq": pll_q, "pllr": pll_r}
        )

    return config


def calculate_frequencies(register_values):
    """Calculate actual system frequencies"""
    rcc_cr = register_values.get("RCC_CR")
    rcc_cfgr = register_values.get("RCC_CFGR")
    rcc_pllckselr = register_values.get("RCC_PLLCKSELR")
    rcc_pll1divr = register_values.get("RCC_PLL1DIVR")

    # Get clock source
    active_source, selected_source = analyze_clock_source(rcc_cfgr)

    # Base frequencies
    hsi_freq = 64_000_000  # 64 MHz
    csi_freq = 4_000_000  # 4 MHz
    hse_freq = 8_000_000  # 8 MHz (corrected)

    if active_source == "HSI":
        return hsi_freq, "HSI 64MHz"
    elif active_source == "CSI":
        return csi_freq, "CSI 4MHz"
    elif active_source == "HSE":
        return hse_freq, "HSE 8MHz"
    elif active_source == "PLL1":
        # Calculate PLL frequency
        pll_config = analyze_pll_config(rcc_cr, rcc_pllckselr, rcc_pll1divr)

        if not pll_config.get("ready", False):
            return 0, "PLL not ready"

        # Get PLL source frequency
        pll_source = pll_config.get("source", "UNKNOWN")
        if pll_source == "HSI":
            input_freq = hsi_freq
        elif pll_source == "CSI":
            input_freq = csi_freq
        elif pll_source == "HSE":
            input_freq = hse_freq
        else:
            return 0, f"Unknown PLL source: {pll_source}"

        # Calculate VCO and system frequency
        pllm = pll_config.get("pllm", 1)
        plln = pll_config.get("plln", 1)
        pllp = pll_config.get("pllp", 1)

        if pllm == 0:
            return 0, "Invalid PLLM=0"

        vco_freq = (input_freq // pllm) * plln
        sys_freq = vco_freq // pllp

        return (
            sys_freq,
            f"PLL: {pll_source} {input_freq//1000000}MHz → VCO {vco_freq//1000000}MHz → SYS {sys_freq//1000000}MHz",
        )

    return 0, f"Unknown clock source: {active_source}"


def main():
    print("🔧 Enhanced Clock Configuration Analysis")
    print("=" * 60)

    # Check if STM32CubeProgrammer is available
    success, _, _ = run_stm32_programmer_cmd(["-c", "port=SWD"])
    if not success:
        print("❌ Cannot connect to STM32CubeProgrammer")
        print(
            "   Make sure ST-Link is connected and STM32CubeProgrammer is installed"
        )
        return 1

    print("✅ Connected to STM32 via ST-Link")

    # Read current register state
    register_values = analyze_rcc_registers()

    print("\n📊 Current RCC Register Analysis:")
    print("-" * 40)

    for reg_name, value in register_values.items():
        if value is not None:
            print(f"{reg_name:15}: 0x{value:08X} ({value})")
        else:
            print(f"{reg_name:15}: READ_FAILED")

    # Analyze clock configuration
    print("\n🔍 Clock Configuration Analysis:")
    print("-" * 40)

    rcc_cr = register_values.get("RCC_CR")
    if rcc_cr is not None:
        hse_on = bool(rcc_cr & (1 << 16))  # HSEON bit
        hse_ready = bool(rcc_cr & (1 << 17))  # HSERDY bit
        hsi_on = bool(rcc_cr & (1 << 8))  # HSION bit
        hsi_ready = bool(rcc_cr & (1 << 10))  # HSIRDY bit

        print(
            f"HSE Status    : {'ON' if hse_on else 'OFF'} / {'READY' if hse_ready else 'NOT_READY'}"
        )
        print(
            f"HSI Status    : {'ON' if hsi_on else 'OFF'} / {'READY' if hsi_ready else 'NOT_READY'}"
        )

    # Analyze PLL
    pll_config = analyze_pll_config(
        register_values.get("RCC_CR"),
        register_values.get("RCC_PLLCKSELR"),
        register_values.get("RCC_PLL1DIVR"),
    )

    print(
        f"PLL Status    : {pll_config['status']} / {'READY' if pll_config.get('ready', False) else 'NOT_READY'}"
    )
    if "source" in pll_config:
        print(f"PLL Source    : {pll_config['source']}")
    if "pllm" in pll_config:
        print(
            f"PLL Config    : M={pll_config.get('pllm', '?')}, N={pll_config.get('plln', '?')}, P={pll_config.get('pllp', '?')}"
        )

    # System clock analysis
    active_source, selected_source = analyze_clock_source(
        register_values.get("RCC_CFGR")
    )
    sys_freq, freq_desc = calculate_frequencies(register_values)

    print(f"System Clock  : {active_source} (selected: {selected_source})")
    print(f"Frequency     : {freq_desc}")

    # Performance assessment
    print(f"\n🎯 Performance Assessment:")
    print("-" * 40)

    if sys_freq >= 200_000_000:
        print(
            f"✅ EXCELLENT: {sys_freq//1000000}MHz - High performance achieved"
        )
    elif sys_freq >= 64_000_000:
        print(f"✅ GOOD: {sys_freq//1000000}MHz - Acceptable performance")
    elif sys_freq >= 10_000_000:
        print(
            f"⚠️ MODERATE: {sys_freq//1000000}MHz - Below optimal performance"
        )
    else:
        print(
            f"❌ POOR: {sys_freq//1000000}MHz - Severely degraded performance"
        )

    # Recommendations
    print(f"\n💡 Recommendations:")
    print("-" * 40)

    if active_source == "CSI":
        print(
            "❌ System running on CSI (4MHz) - this indicates clock configuration failure"
        )
        print("   → Check HSE crystal connections and HSE_VALUE setting")
        print("   → Verify PLL configuration parameters")
    elif active_source == "HSI" and not pll_config.get("ready", False):
        print("⚠️ System on HSI without PLL - performance can be improved")
        print("   → Enable PLL for higher system clock")
    elif active_source == "HSE" and not pll_config.get("ready", False):
        print(
            "✅ HSE working but PLL disabled - enable PLL for maximum performance"
        )
    elif active_source == "PLL1":
        if pll_config.get("source") == "HSE":
            print("✅ OPTIMAL: HSE + PLL configuration active")
        else:
            print("✅ GOOD: PLL active but could use HSE for better stability")

    return 0


if __name__ == "__main__":
    sys.exit(main())
