#!/usr/bin/env python3
"""
STM32H753ZI Real-Time Register Analysis
Decodes CubeProgrammer register readings for live debugging
"""

def analyze_rcc_cr(value):
    """Analyze RCC_CR register (0x58024400)"""
    print(f"\n=== RCC_CR (0x58024400) = 0x{value:08X} ===")
    
    hsion = (value >> 0) & 1
    hsirdy = (value >> 1) & 1
    hseon = (value >> 16) & 1
    hserdy = (value >> 17) & 1
    pll1on = (value >> 24) & 1
    pll1rdy = (value >> 25) & 1
    
    print(f"HSI: {'ON' if hsion else 'OFF'}, {'READY' if hsirdy else 'NOT READY'}")
    print(f"HSE: {'ON' if hseon else 'OFF'}, {'READY' if hserdy else 'NOT READY'}")
    print(f"PLL1: {'ON' if pll1on else 'OFF'}, {'LOCKED' if pll1rdy else 'NOT LOCKED'}")
    
    return {
        'hsi_on': hsion, 'hsi_ready': hsirdy,
        'hse_on': hseon, 'hse_ready': hserdy,
        'pll1_on': pll1on, 'pll1_ready': pll1rdy
    }

def analyze_rcc_cfgr(value):
    """Analyze RCC_CFGR register (0x58024410)"""
    print(f"\n=== RCC_CFGR (0x58024410) = 0x{value:08X} ===")
    
    sw = (value >> 0) & 0x7
    sws = (value >> 3) & 0x7
    
    clock_sources = {
        0: "HSI", 1: "CSI", 2: "HSE", 3: "PLL1"
    }
    
    print(f"SW (System Clock Switch): {sw} = {clock_sources.get(sw, 'INVALID')}")
    print(f"SWS (System Clock Status): {sws} = {clock_sources.get(sws, 'INVALID')}")
    
    if sw != sws:
        print("⚠️  WARNING: Clock switch pending!")
    
    return {'sw': sw, 'sws': sws, 'source': clock_sources.get(sws, 'INVALID')}

def analyze_pll_cfgr(value):
    """Analyze RCC_PLLCFGR register (0x58024428)"""
    print(f"\n=== RCC_PLLCFGR (0x58024428) = 0x{value:08X} ===")
    
    divp1en = (value >> 16) & 1
    divq1en = (value >> 17) & 1
    divr1en = (value >> 18) & 1
    pll1rge = (value >> 2) & 0x3
    pll1vcosel = (value >> 1) & 1
    
    print(f"PLL1P Output: {'ENABLED' if divp1en else 'DISABLED'}")
    print(f"PLL1Q Output: {'ENABLED' if divq1en else 'DISABLED'}")
    print(f"PLL1R Output: {'ENABLED' if divr1en else 'DISABLED'}")
    
    rge_meanings = {0: "1-2MHz", 1: "2-4MHz", 2: "4-8MHz", 3: "8-16MHz"}
    print(f"PLL1 Input Range: {rge_meanings.get(pll1rge, 'INVALID')}")
    print(f"PLL1 VCO Range: {'Wide (192-960MHz)' if pll1vcosel else 'Medium (150-420MHz)'}")
    
    return {
        'divp_en': divp1en, 'divq_en': divq1en, 'divr_en': divr1en,
        'input_range': rge_meanings.get(pll1rge, 'INVALID'),
        'vco_wide': pll1vcosel
    }

def analyze_pll1_divr(value):
    """Analyze RCC_PLL1DIVR register (0x5802442C)"""
    print(f"\n=== RCC_PLL1DIVR (0x5802442C) = 0x{value:08X} ===")
    
    divn1 = ((value >> 0) & 0x1FF) + 1   # N divider (bits 8:0) + 1
    divp1 = ((value >> 9) & 0x7F) + 1    # P divider (bits 15:9) + 1
    divq1 = ((value >> 16) & 0x7F) + 1   # Q divider (bits 22:16) + 1
    divr1 = ((value >> 24) & 0x7F) + 1   # R divider (bits 30:24) + 1
    
    print(f"PLL1N (Multiplier): {divn1}")
    print(f"PLL1P (SYSCLK Divider): {divp1}")
    print(f"PLL1Q (Divider): {divq1}")
    print(f"PLL1R (Divider): {divr1}")
    
    return {'n': divn1, 'p': divp1, 'q': divq1, 'r': divr1}

def analyze_pwr_srdcr(value):
    """Analyze PWR_SRDCR register (0x58024818)"""
    print(f"\n=== PWR_SRDCR (0x58024818) = 0x{value:08X} ===")
    
    vos = (value >> 14) & 0x3
    vosrdy = (value >> 13) & 1
    
    vos_levels = {
        0: "VOS0 (480MHz capable)", 
        1: "VOS1 (400MHz max)",
        2: "VOS2 (300MHz max)", 
        3: "VOS3 (200MHz max)"
    }
    
    print(f"VOS (Voltage Scaling): {vos} = {vos_levels.get(vos, 'INVALID')}")
    print(f"VOSRDY (Voltage Ready): {'READY' if vosrdy else 'NOT READY'}")
    
    return {'vos': vos, 'vos_ready': vosrdy, 'level': vos_levels.get(vos, 'INVALID')}

def calculate_frequencies(rcc_cr, pll_divr, clock_source):
    """Calculate actual frequencies based on register values"""
    print(f"\n=== FREQUENCY CALCULATION ===")
    
    hsi_freq = 64000000  # 64 MHz
    hse_freq = 8000000   # 8 MHz (assumed)
    
    if clock_source == 'HSI':
        sysclk = hsi_freq
        print(f"System Clock Source: HSI = {sysclk/1000000:.1f} MHz")
    elif clock_source == 'HSE':
        sysclk = hse_freq
        print(f"System Clock Source: HSE = {sysclk/1000000:.1f} MHz")
    elif clock_source == 'PLL1' and rcc_cr['pll1_ready']:
        # Need to determine PLL input source from PLLCKSELR
        # For now assume HSI input
        pll_input = hsi_freq / 4  # Typical HSI/4 for PLL input
        vco_freq = pll_input * pll_divr['n']
        sysclk = vco_freq / pll_divr['p']
        
        print(f"PLL Input: {pll_input/1000000:.1f} MHz")
        print(f"VCO Frequency: {vco_freq/1000000:.1f} MHz")
        print(f"System Clock: {sysclk/1000000:.1f} MHz")
    else:
        print("⚠️  System clock source unknown or PLL not ready")
        return None
    
    return sysclk

def main():
    """Main analysis function - input register values here"""
    print("🔍 STM32H753ZI Real-Time Register Analysis")
    print("=" * 50)
    
    # Current register readings from CubeProgrammer
    rcc_cr_val = 0x00004025      # From register read
    rcc_cfgr_val = 0x00000000    # From register read  
    pll_cfgr_val = 0x02020200    # From register read
    pll1_divr_val = 0x01FF0000   # From register read
    pwr_srdcr_val = 0x00006000   # From register read
    
    # Analyze each register
    rcc_cr = analyze_rcc_cr(rcc_cr_val)
    rcc_cfgr = analyze_rcc_cfgr(rcc_cfgr_val)
    pll_cfg = analyze_pll_cfgr(pll_cfgr_val)
    pll_divr = analyze_pll1_divr(pll1_divr_val)
    pwr_srdcr = analyze_pwr_srdcr(pwr_srdcr_val)
    
    # Calculate frequencies
    sysclk = calculate_frequencies(rcc_cr, pll_divr, rcc_cfgr['source'])
    
    # Overall analysis
    print(f"\n=== OVERALL SYSTEM STATUS ===")
    print(f"Silicon Revision: V (480MHz capable)")
    print(f"Voltage Scaling: {pwr_srdcr['level']}")
    print(f"Clock Source: {rcc_cfgr['source']}")
    
    if sysclk:
        print(f"System Frequency: {sysclk/1000000:.1f} MHz")
        
        if sysclk > 400000000 and pwr_srdcr['vos'] != 0:
            print("⚠️  WARNING: Frequency >400MHz requires VOS0!")
        elif sysclk <= 64000000:
            print("ℹ️  Running on internal oscillator (fallback mode)")
        else:
            print("✅ Clock configuration appears normal")
    
    # Debug recommendations
    print(f"\n=== DEBUG RECOMMENDATIONS ===")
    
    if not rcc_cr['pll1_ready'] and rcc_cfgr['sw'] == 3:
        print("🚨 CRITICAL: Trying to use PLL1 but PLL is not locked!")
        print("   → Check PLL configuration and input clock")
    
    if rcc_cfgr['sw'] != rcc_cfgr['sws']:
        print("⚠️  Clock switch pending - system may be transitioning")
    
    if pwr_srdcr['vos'] != 0 and sysclk and sysclk > 400000000:
        print("⚠️  High frequency requires VOS0 voltage scaling")
    
    if rcc_cr['hsi_ready'] and not rcc_cr['hse_ready']:
        print("ℹ️  HSE not ready - system using HSI (internal oscillator)")
        print("   → This is normal for default Nucleo configuration")

if __name__ == "__main__":
    main()
