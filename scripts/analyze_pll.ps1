# Simple PLL Configuration Analysis using STM32CubeProgrammer CLI
# This script uses the same tool we've been using for register analysis

Write-Host "🔧 STM32H753ZI PLL Configuration Analysis" -ForegroundColor Cyan
Write-Host "========================================`n"

# Resolve STM32 CubeProgrammer CLI via env override -> JSON helper -> fallback
$STM32_PROGRAMMER = $null
if ($env:STM32_PROGRAMMER_CLI) { $STM32_PROGRAMMER = $env:STM32_PROGRAMMER_CLI }
else {
    $psHelper = Join-Path $PSScriptRoot "Get-WorkflowToolchain.ps1"
    if (Test-Path $psHelper) {
        try { $candidate = & $psHelper "stm32_programmer_cli_candidates" 2>$null; if ($candidate -and (Test-Path $candidate)) { $STM32_PROGRAMMER = $candidate } }
        catch { }
    }
}
if (-not $STM32_PROGRAMMER) { $STM32_PROGRAMMER = "C:\\ST\\STM32CubeCLT_1.19.0\\STM32CubeProgrammer\\bin\\STM32_Programmer_CLI.exe" }

# Check if tool exists
if (-not (Test-Path $STM32_PROGRAMMER)) {
    Write-Host "❌ STM32CubeProgrammer not found at: $STM32_PROGRAMMER" -ForegroundColor Red
    exit 1
}

Write-Host "✅ STM32CubeProgrammer found" -ForegroundColor Green

# Check if firmware is built and flash it
$ELF_FILE = "build\Debug\stm32h753_ihm02a1.elf"
$HEX_FILE = "build\Debug\stm32h753_ihm02a1.hex"

if (-not (Test-Path $HEX_FILE)) {
    Write-Host "❌ Firmware not found: $HEX_FILE" -ForegroundColor Red
    Write-Host "💡 Run build task first: Build (CMake)" -ForegroundColor Yellow
    exit 1
}

Write-Host "✅ Firmware found: $HEX_FILE" -ForegroundColor Green

# Function to read and decode registers
function Read-Register {
    param($Address, $Name)
    $result = & $STM32_PROGRAMMER -c port=SWD -r32 $Address 1 2>$null | Select-String "0x[0-9A-F]+.*: [0-9A-F]+"
    if ($result) {
        $value = $result.Line.Split(': ')[1]
        Write-Host "$Name (0x$($Address.ToString('X8'))): 0x$value" -ForegroundColor White
        return [convert]::ToUInt32($value, 16)
    }
    return 0
}

function Decode-RCC-Registers {
    Write-Host "`n=== RCC Register Analysis ===" -ForegroundColor Cyan
    
    $rcc_cr = Read-Register 0x58024400 "RCC_CR"
    $rcc_cfgr = Read-Register 0x58024404 "RCC_CFGR" 
    $rcc_pllcfgr = Read-Register 0x58024408 "RCC_PLLCFGR"
    
    Write-Host "`n📊 Decoded Values:" -ForegroundColor Yellow
    
    # RCC_CR analysis
    $hsion = $rcc_cr -band 0x1
    $hsirdy = ($rcc_cr -shr 2) -band 0x1
    $pll1on = ($rcc_cr -shr 24) -band 0x1
    $pll1rdy = ($rcc_cr -shr 25) -band 0x1
    
    Write-Host "  HSION: $hsion, HSIRDY: $hsirdy"
    Write-Host "  PLL1ON: $pll1on, PLL1RDY: $pll1rdy"
    
    # RCC_CFGR analysis
    $sw = $rcc_cfgr -band 0x7
    $sws = ($rcc_cfgr -shr 3) -band 0x7
    $hpre = ($rcc_cfgr -shr 8) -band 0xF
    
    Write-Host "  SW (Clock Source): $sw, SWS (Status): $sws"
    Write-Host "  HPRE (AHB Prescaler): $hpre"
    
    # RCC_PLLCFGR analysis
    $pllsrc = $rcc_pllcfgr -band 0x3
    $pllm = ($rcc_pllcfgr -shr 4) -band 0x3F
    $plln = ($rcc_pllcfgr -shr 8) -band 0x7F
    $pllpen = ($rcc_pllcfgr -shr 16) -band 0x1
    $pllp = ($rcc_pllcfgr -shr 17) -band 0x1
    
    Write-Host "  PLLSRC: $pllsrc, PLLM: $pllm, PLLN: $plln"
    Write-Host "  PLLPEN: $pllpen, PLLP: $pllp"
    
    # Expected values
    Write-Host "`n🎯 Expected vs Actual:" -ForegroundColor Yellow
    Write-Host "  PLLSRC: Expected=2(HSI), Actual=$pllsrc $(if($pllsrc -eq 2){'✅'}else{'❌'})"
    Write-Host "  PLLM: Expected=4, Actual=$pllm $(if($pllm -eq 4){'✅'}else{'❌'})"
    Write-Host "  PLLN: Expected=60, Actual=$plln $(if($plln -eq 60){'✅'}else{'❌'})"  
    Write-Host "  PLLPEN: Expected=1, Actual=$pllpen $(if($pllpen -eq 1){'✅'}else{'❌'})"
    Write-Host "  PLLP: Expected=0, Actual=$pllp $(if($pllp -eq 0){'✅'}else{'❌'})"
}

function Decode-PWR-Registers {
    Write-Host "`n=== PWR Register Analysis ===" -ForegroundColor Cyan
    
    $pwr_cr3 = Read-Register 0x58024C0C "PWR_CR3"
    $pwr_csr1 = Read-Register 0x58024C04 "PWR_CSR1"
    
    Write-Host "`n📊 Decoded Values:" -ForegroundColor Yellow
    
    # PWR analysis
    $vosrdy = ($pwr_csr1 -shr 13) -band 0x1
    Write-Host "  VOSRDY (Voltage Ready): $vosrdy $(if($vosrdy -eq 1){'✅'}else{'❌'})"
}

# Main analysis
Write-Host "🔗 Connecting to STM32H753ZI..." -ForegroundColor Cyan
$connect_result = & $STM32_PROGRAMMER -c port=SWD 2>$null | Select-String "Device name"
if ($connect_result) {
    Write-Host "✅ Connected: $($connect_result.Line.Trim())" -ForegroundColor Green
}
else {
    Write-Host "❌ Connection failed - check ST-LINK connection" -ForegroundColor Red
    exit 1
}

Write-Host "`n💾 Flashing latest firmware..." -ForegroundColor Cyan
$flash_result = & $STM32_PROGRAMMER -c port=SWD -w $HEX_FILE -v -rst 2>$null
if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Firmware flashed successfully" -ForegroundColor Green
}
else {
    Write-Host "❌ Flash failed" -ForegroundColor Red
    exit 1
}

Write-Host "`n⏳ Waiting for system startup..." -ForegroundColor Yellow
Start-Sleep -Seconds 2

# Perform register analysis
Decode-RCC-Registers
Decode-PWR-Registers

Write-Host "`n🔄 Analysis complete!" -ForegroundColor Green
Write-Host "If PLL values are incorrect, this confirms HAL_RCC_OscConfig() corruption." -ForegroundColor Yellow
