#!/usr/bin/env powershell
# STM32H753ZI System Fault Debugging Script
# Uses STM32CubeProgrammer CLI to analyze system state and fault conditions

param(
    [switch]$Verbose,
    [switch]$FullAnalysis,
    [string]$LogFile = "debug_fault_analysis.log"
)

$ErrorActionPreference = "Stop"

# STM32CubeCLT paths
$CUBE_PROGRAMMER = "C:\ST\STM32CubeCLT_1.19.0\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
$STLINK_GDB = "C:\ST\STM32CubeCLT_1.19.0\STLink-gdb-server\bin\ST-LINK_gdbserver.exe"

# Define register addresses for STM32H753ZI
$REGISTERS = @{
    # System Control Block (SCB) - Fault Status
    "SCB_CFSR"    = "0xE000ED28"  # Configurable Fault Status Register
    "SCB_HFSR"    = "0xE000ED2C"  # HardFault Status Register  
    "SCB_DFSR"    = "0xE000ED30"  # Debug Fault Status Register
    "SCB_AFSR"    = "0xE000ED3C"  # Auxiliary Fault Status Register
    "SCB_SHCSR"   = "0xE000ED24"  # System Handler Control and State Register
    "SCB_ICSR"    = "0xE000ED04"  # Interrupt Control and State Register
    
    # RCC Clock Control
    "RCC_CR"      = "0x58024400"  # Clock Control Register
    "RCC_CFGR"    = "0x58024404"  # Clock Configuration Register
    "RCC_D1CFGR"  = "0x58024418"  # Domain 1 Clock Configuration Register
    "RCC_D2CFGR"  = "0x5802441C"  # Domain 2 Clock Configuration Register
    "RCC_D3CFGR"  = "0x58024420"  # Domain 3 Clock Configuration Register
    "RCC_PLLCFGR" = "0x58024428"  # PLL Configuration Register
    
    # Core Debug
    "DHCSR"       = "0xE000EDF0"  # Debug Halting Control and Status Register
    "DCRSR"       = "0xE000EDF4"  # Debug Core Register Selector Register
    "DCRDR"       = "0xE000EDF8"  # Debug Core Register Data Register
    "DEMCR"       = "0xE000EDFC"  # Debug Exception and Monitor Control Register
    
    # Stack Pointers
    "MSP"         = "0xE000ED08"  # Main Stack Pointer (from VTOR)
    "PSP"         = "0xE000ED09"  # Process Stack Pointer
}

function Write-DebugLog {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logEntry = "[$timestamp] [$Level] $Message"
    
    if ($Verbose) {
        Write-Host $logEntry -ForegroundColor $(
            switch ($Level) {
                "ERROR" { "Red" }
                "WARN" { "Yellow" } 
                "INFO" { "Cyan" }
                "SUCCESS" { "Green" }
                default { "White" }
            }
        )
    }
    
    Add-Content -Path $LogFile -Value $logEntry
}

function Test-STLinkConnection {
    Write-DebugLog "Testing ST-LINK connection..." "INFO"
    
    try {
        $result = & $CUBE_PROGRAMMER -c port=SWD freq=4000 ap=0 mode=UR 2>&1
        
        if ($LASTEXITCODE -eq 0) {
            Write-DebugLog "ST-LINK connection successful" "SUCCESS"
            return $true
        }
        else {
            Write-DebugLog "ST-LINK connection failed: $result" "ERROR"
            return $false
        }
    }
    catch {
        Write-DebugLog "Exception during ST-LINK test: $_" "ERROR"
        return $false
    }
}

function Read-Register {
    param([string]$Address, [string]$Name = "")
    
    try {
        $result = & $CUBE_PROGRAMMER -c port=SWD -r32 $Address 1 2>&1
        
        if ($LASTEXITCODE -eq 0) {
            # Parse the result to extract the register value
            if ($result -match "0x([0-9A-Fa-f]+)") {
                $value = $matches[1]
                $displayName = if ($Name) { $Name } else { $Address }
                Write-DebugLog "Register $displayName ($Address) = 0x$value" "INFO"
                return "0x$value"
            }
        }
        
        Write-DebugLog "Failed to read register $Address" "ERROR"
        return $null
    }
    catch {
        Write-DebugLog "Exception reading register $Address : $_" "ERROR"
        return $null
    }
}

function Analyze-FaultStatus {
    Write-DebugLog "=== ANALYZING SYSTEM FAULT STATUS ===" "INFO"
    
    # Read critical fault status registers
    $cfsr = Read-Register $REGISTERS.SCB_CFSR "SCB_CFSR (Configurable Fault Status)"
    $hfsr = Read-Register $REGISTERS.SCB_HFSR "SCB_HFSR (HardFault Status)"
    $dfsr = Read-Register $REGISTERS.SCB_DFSR "SCB_DFSR (Debug Fault Status)"
    $shcsr = Read-Register $REGISTERS.SCB_SHCSR "SCB_SHCSR (System Handler Control)"
    
    if ($cfsr) {
        $cfsrVal = [Convert]::ToUInt32($cfsr, 16)
        Write-DebugLog "CFSR Analysis:" "INFO"
        
        # Memory Management Fault Status (bits 0-7)
        if ($cfsrVal -band 0x01) { Write-DebugLog "  - IACCVIOL: Instruction access violation" "WARN" }
        if ($cfsrVal -band 0x02) { Write-DebugLog "  - DACCVIOL: Data access violation" "WARN" }
        if ($cfsrVal -band 0x08) { Write-DebugLog "  - MUNSTKERR: Unstacking error" "WARN" }
        if ($cfsrVal -band 0x10) { Write-DebugLog "  - MSTKERR: Stacking error" "WARN" }
        if ($cfsrVal -band 0x20) { Write-DebugLog "  - MLSPERR: Lazy state preservation error" "WARN" }
        if ($cfsrVal -band 0x80) { Write-DebugLog "  - MMARVALID: MMFAR contains valid address" "INFO" }
        
        # Bus Fault Status (bits 8-15)
        if ($cfsrVal -band 0x0100) { Write-DebugLog "  - IBUSERR: Instruction bus error" "WARN" }
        if ($cfsrVal -band 0x0200) { Write-DebugLog "  - PRECISERR: Precise data bus error" "WARN" }
        if ($cfsrVal -band 0x0400) { Write-DebugLog "  - IMPRECISERR: Imprecise data bus error" "WARN" }
        if ($cfsrVal -band 0x0800) { Write-DebugLog "  - UNSTKERR: Bus fault on unstacking" "WARN" }
        if ($cfsrVal -band 0x1000) { Write-DebugLog "  - STKERR: Bus fault on stacking" "WARN" }
        if ($cfsrVal -band 0x2000) { Write-DebugLog "  - LSPERR: Bus fault on lazy state preservation" "WARN" }
        if ($cfsrVal -band 0x8000) { Write-DebugLog "  - BFARVALID: BFAR contains valid address" "INFO" }
        
        # Usage Fault Status (bits 16-31)
        if ($cfsrVal -band 0x010000) { Write-DebugLog "  - UNDEFINSTR: Undefined instruction" "WARN" }
        if ($cfsrVal -band 0x020000) { Write-DebugLog "  - INVSTATE: Invalid state" "WARN" }
        if ($cfsrVal -band 0x040000) { Write-DebugLog "  - INVPC: Invalid PC" "WARN" }
        if ($cfsrVal -band 0x080000) { Write-DebugLog "  - NOCP: No coprocessor" "WARN" }
        if ($cfsrVal -band 0x1000000) { Write-DebugLog "  - UNALIGNED: Unaligned access" "WARN" }
        if ($cfsrVal -band 0x2000000) { Write-DebugLog "  - DIVBYZERO: Divide by zero" "WARN" }
    }
    
    if ($hfsr) {
        $hfsrVal = [Convert]::ToUInt32($hfsr, 16)
        Write-DebugLog "HFSR Analysis:" "INFO"
        
        if ($hfsrVal -band 0x02) { Write-DebugLog "  - VECTTBL: Vector table hard fault" "ERROR" }
        if ($hfsrVal -band 0x40000000) { Write-DebugLog "  - FORCED: Forced hard fault" "ERROR" }
        if ($hfsrVal -band 0x80000000) { Write-DebugLog "  - DEBUGEVT: Debug event hard fault" "ERROR" }
    }
}

function Analyze-ClockStatus {
    Write-DebugLog "=== ANALYZING CLOCK CONFIGURATION ===" "INFO"
    
    $rcc_cr = Read-Register $REGISTERS.RCC_CR "RCC_CR (Clock Control)"
    $rcc_cfgr = Read-Register $REGISTERS.RCC_CFGR "RCC_CFGR (Clock Configuration)"
    $rcc_d1cfgr = Read-Register $REGISTERS.RCC_D1CFGR "RCC_D1CFGR (Domain 1 Config)"
    $rcc_pllcfgr = Read-Register $REGISTERS.RCC_PLLCFGR "RCC_PLLCFGR (PLL Config)"
    
    if ($rcc_cr) {
        $crVal = [Convert]::ToUInt32($rcc_cr, 16)
        Write-DebugLog "RCC_CR Analysis:" "INFO"
        
        if ($crVal -band 0x01) { Write-DebugLog "  - HSION: HSI oscillator enabled" "INFO" }
        if ($crVal -band 0x02) { Write-DebugLog "  - HSIRDY: HSI oscillator ready" "SUCCESS" }
        if ($crVal -band 0x10000) { Write-DebugLog "  - HSEON: HSE oscillator enabled" "INFO" }
        if ($crVal -band 0x20000) { Write-DebugLog "  - HSERDY: HSE oscillator ready" "SUCCESS" }
        if ($crVal -band 0x1000000) { Write-DebugLog "  - PLL1ON: PLL1 enabled" "INFO" }
        if ($crVal -band 0x2000000) { Write-DebugLog "  - PLL1RDY: PLL1 ready" "SUCCESS" }
    }
    
    if ($rcc_cfgr) {
        $cfgrVal = [Convert]::ToUInt32($rcc_cfgr, 16)
        $sw = $cfgrVal -band 0x07
        $sws = ($cfgrVal -shr 3) -band 0x07
        
        Write-DebugLog "RCC_CFGR Analysis:" "INFO"
        Write-DebugLog "  - SW (System clock switch): $sw" "INFO"
        Write-DebugLog "  - SWS (System clock status): $sws" "INFO"
        
        switch ($sws) {
            0 { Write-DebugLog "  - Current system clock: HSI" "INFO" }
            1 { Write-DebugLog "  - Current system clock: CSI" "INFO" }
            2 { Write-DebugLog "  - Current system clock: HSE" "INFO" }
            3 { Write-DebugLog "  - Current system clock: PLL1" "SUCCESS" }
            default { Write-DebugLog "  - Current system clock: Unknown ($sws)" "WARN" }
        }
    }
}

function Analyze-CoreState {
    Write-DebugLog "=== ANALYZING CORE STATE ===" "INFO"
    
    $dhcsr = Read-Register $REGISTERS.DHCSR "DHCSR (Debug Halting Control)"
    $icsr = Read-Register $REGISTERS.SCB_ICSR "SCB_ICSR (Interrupt Control)"
    
    if ($dhcsr) {
        $dhcsrVal = [Convert]::ToUInt32($dhcsr, 16)
        Write-DebugLog "DHCSR Analysis:" "INFO"
        
        if ($dhcsrVal -band 0x01) { Write-DebugLog "  - C_DEBUGEN: Debug enabled" "INFO" }
        if ($dhcsrVal -band 0x02) { Write-DebugLog "  - C_HALT: Core halted" "WARN" }
        if ($dhcsrVal -band 0x04) { Write-DebugLog "  - C_STEP: Single step" "INFO" }
        if ($dhcsrVal -band 0x10000) { Write-DebugLog "  - S_REGRDY: Register ready" "INFO" }
        if ($dhcsrVal -band 0x20000) { Write-DebugLog "  - S_HALT: Core is halted" "WARN" }
        if ($dhcsrVal -band 0x40000) { Write-DebugLog "  - S_SLEEP: Core is sleeping" "INFO" }
        if ($dhcsrVal -band 0x80000) { Write-DebugLog "  - S_LOCKUP: Core is locked up" "ERROR" }
    }
    
    if ($icsr) {
        $icsrVal = [Convert]::ToUInt32($icsr, 16)
        $vectActive = $icsrVal -band 0x1FF
        $vectPending = ($icsrVal -shr 12) -band 0x1FF
        
        Write-DebugLog "ICSR Analysis:" "INFO"
        Write-DebugLog "  - VECTACTIVE: $vectActive" "INFO"
        Write-DebugLog "  - VECTPENDING: $vectPending" "INFO"
        
        if ($vectActive -ne 0) {
            Write-DebugLog "  - Currently executing exception/interrupt: $vectActive" "WARN"
        }
    }
}

function Generate-DebugReport {
    Write-DebugLog "=== GENERATING DEBUG REPORT ===" "INFO"
    
    $reportFile = "STM32H753ZI_Debug_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').md"
    
    $report = @"
# STM32H753ZI System Fault Debug Report

**Generated**: $(Get-Date)
**Firmware**: stm32h753_ihm02a1.elf
**Debug Tool**: STM32CubeProgrammer CLI v1.19.0

## Current Symptoms
- LD4 (Red LED): Solid ON - indicates system fault/hardfault  
- LD1 (Green LED): OFF - normal operation LED not active
- LD3 (Blue LED): OFF - user LED not toggling
- UART: No serial output detected

## Register Analysis
"@

    # Append the log content to the report
    if (Test-Path $LogFile) {
        $report += "`n`n## Debug Log`n"
        $report += "``````"
        $report += Get-Content $LogFile | Out-String
        $report += "``````"
    }
    
    $report += @"

## Recommended Next Steps

### Immediate Actions
1. **Use VS Code Debugger**: Launch "Debug STM32H753ZI (ST-LINK GDB)" configuration
2. **Check Stack Pointer**: Verify stack hasn't overflowed or corrupted
3. **Review Clock Config**: Analyze if new Clock_Init() is causing issues
4. **Examine Boot Sequence**: Step through SystemInit() and main()

### Investigation Priorities
1. **HardFault Handler**: Check if system is stuck in HardFault_Handler
2. **Clock Stability**: Verify if 240MHz PLL configuration is stable
3. **Memory Access**: Check for invalid memory accesses or stack overflow
4. **FreeRTOS Tasks**: Verify if task scheduler is starting properly

### Tools Available
- **VS Code Tasks**: 🔧 DEBUG: Connect ST-LINK, 🔍 DEBUG: Read System Fault Status
- **ST-LINK GDB**: Full debugging with breakpoints and variable inspection
- **CubeProgrammer**: Register analysis and memory inspection

### Reference Files
- Clock configuration: ``src/config/clock_config.c``
- SSOT headers: ``src/config/freertos_config_ssot.h``
- Main application: ``Core/Src/main.c``
- Enhanced clock: ``src/enhanced_clock_config.c``
"@

    Set-Content -Path $reportFile -Value $report
    Write-DebugLog "Debug report generated: $reportFile" "SUCCESS"
    
    return $reportFile
}

# Main execution
try {
    Write-DebugLog "Starting STM32H753ZI system fault analysis..." "INFO"
    
    # Check if STM32CubeProgrammer is available
    if (-not (Test-Path $CUBE_PROGRAMMER)) {
        Write-DebugLog "STM32CubeProgrammer not found at: $CUBE_PROGRAMMER" "ERROR"
        exit 1
    }
    
    # Test ST-LINK connection
    if (-not (Test-STLinkConnection)) {
        Write-DebugLog "Cannot proceed without ST-LINK connection" "ERROR"
        exit 1
    }
    
    # Perform fault analysis
    Analyze-FaultStatus
    Write-DebugLog "" "INFO"
    
    Analyze-ClockStatus  
    Write-DebugLog "" "INFO"
    
    Analyze-CoreState
    Write-DebugLog "" "INFO"
    
    if ($FullAnalysis) {
        Write-DebugLog "=== FULL REGISTER DUMP ===" "INFO"
        foreach ($reg in $REGISTERS.GetEnumerator()) {
            Read-Register $reg.Value $reg.Key
        }
    }
    
    # Generate comprehensive report
    $reportFile = Generate-DebugReport
    
    Write-DebugLog "Analysis complete. Check report: $reportFile" "SUCCESS"
    Write-DebugLog "Next steps: Use VS Code debugger for deeper analysis" "INFO"
    
}
catch {
    Write-DebugLog "Fatal error during analysis: $_" "ERROR"
    exit 1
}
