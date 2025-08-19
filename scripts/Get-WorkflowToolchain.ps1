<#
.SYNOPSIS
  Read keys from config/workflow_toolchain.json for PowerShell scripts.
.PARAMETER Key
  The JSON key to return.
.EXAMPLE
  .\scripts\Get-WorkflowToolchain.ps1 arm_gcc_executable
#>
param(
    [Parameter(Mandatory = $true)][string]$Key
)

$JsonPath = Join-Path $PSScriptRoot "..\config\workflow_toolchain.json"
$LocalJsonPath = Join-Path $PSScriptRoot "..\config\workflow_toolchain.local.json"
# Prefer local override if present
if (Test-Path $LocalJsonPath) { $JsonPath = $LocalJsonPath }
if (-Not (Test-Path $JsonPath)) {
    Write-Error "Workflow toolchain JSON not found: $JsonPath"
    exit 1
}

try {
    $j = Get-Content $JsonPath -Raw | ConvertFrom-Json
}
catch {
    Write-Error "Failed to parse JSON: $_"
    exit 1
}

function Expand-Placeholders {
    param(
        [string]$Str,
        [hashtable]$Ctx
    )
    $pattern = [regex]'\$\{([^}]+)\}'
    $evaluator = [System.Text.RegularExpressions.MatchEvaluator] { param($m) 
        $name = $m.Groups[1].Value
        if ($null -ne $name -and $Ctx.ContainsKey($name)) { return $Ctx[$name] } else { return $m.Value }
    }
    return $pattern.Replace($Str, $evaluator)
}

# Build context hashtable from string values for expansion
$ctx = @{}
foreach ($prop in $j.PSObject.Properties) {
    if ($prop.Value -is [string]) { $ctx[$prop.Name] = $prop.Value }
}

# Expand placeholders iteratively in context values
for ($i = 0; $i -lt 10; $i++) {
    $changed = $false
    $keys = $ctx.Keys | ForEach-Object { $_ }
    foreach ($k in $keys) {
        $v = $ctx[$k]
        $new = Expand-Placeholders $v $ctx
        if ($new -ne $v) { $ctx[$k] = $new; $changed = $true }
    }
    if (-not $changed) { break }
}

# Return expanded value if present
if ($j.PSObject.Properties.Name -contains $Key) {
    $val = $j.$Key
    if ($val -is [System.Array]) {
        foreach ($cand in $val) {
            if ($cand -is [string]) {
                $expanded = Expand-Placeholders $cand $ctx
                $expanded = [Environment]::ExpandEnvironmentVariables($expanded)
                if (Test-Path $expanded) { Write-Output $expanded; exit 0 }
            }
        }
        # No existing candidate found - return expanded first
        $first = Expand-Placeholders $val[0] $ctx
        Write-Output ([Environment]::ExpandEnvironmentVariables($first))
        exit 0
    }
    elseif ($val -is [string]) {
        $out = Expand-Placeholders $val $ctx
        Write-Output ([Environment]::ExpandEnvironmentVariables($out))
        exit 0
    }
    else {
        Write-Output $val
        exit 0
    }
}

exit 2
