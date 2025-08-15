#!/usr/bin/env python3
"""
STM32H753ZI PLL Configuration Analysis
Decode register values to verify direct programming results
"""

# Register values from live hardware
RCC_PLLCKSELR = 0x02020200
RCC_PLLCFGR = 0x01FF0000
RCC_PLL1DIVR = 0x01010280

print("🔍 STM32H753ZI PLL Configuration Analysis")
print("=" * 50)

# Decode RCC_PLLCKSELR (0x58024428)
print(f"\n📍 RCC_PLLCKSELR = 0x{RCC_PLLCKSELR:08X}")
pllsrc = RCC_PLLCKSELR & 0x3
divm1 = (RCC_PLLCKSELR >> 4) & 0x3F
divm2 = (RCC_PLLCKSELR >> 12) & 0x3F
divm3 = (RCC_PLLCKSELR >> 20) & 0x3F

pll_sources = {0: "HSI", 1: "CSI", 2: "HSE", 3: "Reserved"}
print(f"   PLL Source: {pll_sources.get(pllsrc, 'Unknown')} ({pllsrc})")
print(f"   DIVM1: {divm1} (PLL1 prescaler)")
print(f"   DIVM2: {divm2} (PLL2 prescaler)")
print(f"   DIVM3: {divm3} (PLL3 prescaler)")

# Decode RCC_PLLCFGR (0x5802442C)
print(f"\n📍 RCC_PLLCFGR = 0x{RCC_PLLCFGR:08X}")
pll1fracen = (RCC_PLLCFGR >> 0) & 0x1
pll1vcosel = (RCC_PLLCFGR >> 1) & 0x1
pll1rge = (RCC_PLLCFGR >> 2) & 0x3
pll1pen = (RCC_PLLCFGR >> 16) & 0x1
pll1qen = (RCC_PLLCFGR >> 17) & 0x1
pll1ren = (RCC_PLLCFGR >> 18) & 0x1

print(f"   PLL1FRACEN: {pll1fracen} (Fractional divider {'enabled' if pll1fracen else 'disabled'})")
print(f"   PLL1VCOSEL: {pll1vcosel} ({'Wide VCO' if pll1vcosel else 'Medium VCO'})")
print(f"   PLL1RGE: {pll1rge} (PLL1 input frequency range)")
print(f"   PLL1PEN: {pll1pen} (PLL1P output {'enabled' if pll1pen else 'disabled'})")
print(f"   PLL1QEN: {pll1qen} (PLL1Q output {'enabled' if pll1qen else 'disabled'})")
print(f"   PLL1REN: {pll1ren} (PLL1R output {'enabled' if pll1ren else 'disabled'})")

# Decode RCC_PLL1DIVR (0x58024430)
print(f"\n📍 RCC_PLL1DIVR = 0x{RCC_PLL1DIVR:08X}")
divn1 = (RCC_PLL1DIVR >> 0) & 0x1FF
divp1 = (RCC_PLL1DIVR >> 9) & 0x7F
divq1 = (RCC_PLL1DIVR >> 16) & 0x7F
divr1 = (RCC_PLL1DIVR >> 24) & 0x7F

# Convert encoded values to actual divisors (register stores value-1)
actual_n = divn1 + 1
actual_p = divp1 + 1
actual_q = divq1 + 1
actual_r = divr1 + 1

print(f"   DIVN1: {divn1} -> N = {actual_n}")
print(f"   DIVP1: {divp1} -> P = {actual_p}")
print(f"   DIVQ1: {divq1} -> Q = {actual_q}")
print(f"   DIVR1: {divr1} -> R = {actual_r}")

# Calculate frequencies
print(f"\n🧮 Frequency Calculations")
print("=" * 30)

# HSI frequency (internal oscillator)
hsi_freq = 64_000_000  # 64 MHz HSI

if pllsrc == 0:  # HSI selected
    pll_input = hsi_freq / divm1
    vco_freq = pll_input * actual_n
    pll1p_freq = vco_freq / actual_p
    pll1q_freq = vco_freq / actual_q
    pll1r_freq = vco_freq / actual_r
    
    print(f"   HSI frequency: {hsi_freq/1e6:.1f} MHz")
    print(f"   PLL input (HSI/M): {pll_input/1e6:.1f} MHz")
    print(f"   VCO frequency: {vco_freq/1e6:.1f} MHz")
    print(f"   PLL1P output: {pll1p_freq/1e6:.1f} MHz (System Clock)")
    print(f"   PLL1Q output: {pll1q_freq/1e6:.1f} MHz")
    print(f"   PLL1R output: {pll1r_freq/1e6:.1f} MHz")
    
    # Check if this matches our target
    if abs(pll1p_freq - 480_000_000) < 1000:
        print(f"\n✅ SUCCESS: PLL1P = {pll1p_freq/1e6:.1f} MHz (Target: 480 MHz)")
    else:
        print(f"\n⚠️  WARNING: PLL1P = {pll1p_freq/1e6:.1f} MHz (Target: 480 MHz)")
        
    # Validate VCO range
    if 192_000_000 <= vco_freq <= 836_000_000:
        print(f"✅ VCO frequency {vco_freq/1e6:.1f} MHz is within valid range (192-836 MHz)")
    else:
        print(f"❌ VCO frequency {vco_freq/1e6:.1f} MHz is outside valid range (192-836 MHz)")
        
else:
    print(f"   PLL source is not HSI (source={pllsrc})")

print(f"\n🎯 Expected vs Actual Configuration:")
print(f"   Expected: HSI/4 * 60 / 1 = 64/4 * 60 = 16 * 60 = 960 MHz VCO, 480 MHz SYSCLK")
print(f"   Actual: HSI/{divm1} * {actual_n} / {actual_p} = 64/{divm1} * {actual_n} / {actual_p}")

if pllsrc == 0:
    expected_vco = (64_000_000 / divm1) * actual_n
    expected_sysclk = expected_vco / actual_p
    print(f"   Calculated: {expected_vco/1e6:.1f} MHz VCO, {expected_sysclk/1e6:.1f} MHz SYSCLK")
