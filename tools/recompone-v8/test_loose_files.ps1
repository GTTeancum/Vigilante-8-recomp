param(
    [string]$Exe = "reference/generated/recompiled/bin/Release/net10.0/Vigilante8PC.exe",
    [string]$Cue = "BINCUE/Vigilante 8 (USA).cue",
    [string]$OutputRoot = "reference/generated/loose-tests"
)

$ErrorActionPreference = "Stop"
$repo = (Resolve-Path (Join-Path $PSScriptRoot "../..")).Path
$exePath = (Resolve-Path (Join-Path $repo $Exe)).Path
$cuePath = (Resolve-Path (Join-Path $repo $Cue)).Path
$outputBase = Join-Path $repo $OutputRoot
$runRoot = Join-Path $outputBase (Get-Date -Format "yyyyMMdd-HHmmss")
New-Item -ItemType Directory -Path $runRoot -Force | Out-Null

function Get-Sha256([byte[]]$Data) {
    $sha = [Security.Cryptography.SHA256]::Create()
    try {
        $hash = $sha.ComputeHash($Data)
        return [BitConverter]::ToString($hash).Replace("-", "")
    } finally {
        $sha.Dispose()
    }
}

function Assert-Equal($Actual, $Expected, [string]$Label) {
    if ($Actual -ne $Expected) {
        throw "$Label expected '$Expected', observed '$Actual'"
    }
}

function Invoke-Probe([string]$Label, [string]$DiscPath, [string]$LooseRoot = "") {
    $arguments = @($cuePath)
    if ($LooseRoot.Length -eq 0) {
        $arguments += "--no-loose"
    } else {
        $arguments += @("--loose", $LooseRoot)
    }
    $arguments += @("--probe-file", $DiscPath)
    $output = (& $exePath @arguments 2>&1 | Out-String)
    if ($LASTEXITCODE -ne 0) {
        throw "$Label probe failed with exit code $LASTEXITCODE`n$output"
    }
    [IO.File]::WriteAllText((Join-Path $runRoot "$Label.log"), $output)

    $sourceLine = ($output -split "`r?`n" |
        Where-Object { $_.StartsWith("[SourceProbe]") } | Select-Object -Last 1)
    $fileLine = ($output -split "`r?`n" |
        Where-Object { $_.StartsWith("[SourceProbeFile]") } | Select-Object -Last 1)
    if ($null -eq $sourceLine -or $null -eq $fileLine) {
        throw "$Label probe did not emit both source records"
    }

    $values = @{}
    foreach ($match in [regex]::Matches($fileLine, "(?<key>[A-Za-z0-9]+)=(?<value>[^ ]+)")) {
        $values[$match.Groups["key"].Value] = $match.Groups["value"].Value
    }
    $overrideMatch = [regex]::Match($sourceLine, "overrides=(?<count>[0-9]+)")
    if (!$overrideMatch.Success) { throw "$Label probe omitted the override count" }

    return [PSCustomObject]@{
        Label = $Label
        Overrides = [int]$overrideMatch.Groups["count"].Value
        Size = [int64]$values["size"]
        Bytes = [int64]$values["bytes"]
        Sha256 = $values["sha256"]
        Sector2048 = $values["sector2048"]
        Sector2352 = $values["sector2352"]
    }
}

function Write-Pattern([string]$Path, [int]$Length, [int]$Seed) {
    $parent = Split-Path $Path
    New-Item -ItemType Directory -Path $parent -Force | Out-Null
    $bytes = [byte[]]::new($Length)
    for ($i = 0; $i -lt $bytes.Length; $i++) {
        $bytes[$i] = [byte](($i * 37 + $Seed) -band 0xFF)
    }
    [IO.File]::WriteAllBytes($Path, $bytes)
    return ,$bytes
}

$results = [Collections.Generic.List[object]]::new()
$baselineSystem = Invoke-Probe "baseline-system" "SYSTEM.CNF"
$baselineVideo = Invoke-Probe "baseline-video" "VIDEO/ACTLOGO.STR"
$results.Add($baselineSystem)
$results.Add($baselineVideo)

$exactRoot = Join-Path $runRoot "exact"
$exactPath = Join-Path $exactRoot "SYSTEM.CNF"
[byte[]]$exactBytes = Write-Pattern $exactPath 97 11
$exact = Invoke-Probe "exact-mutated-size" "SYSTEM.CNF" $exactRoot
$results.Add($exact)
Assert-Equal $exact.Overrides 1 "exact override count"
Assert-Equal $exact.Size 97 "exact loose size"
Assert-Equal $exact.Bytes 97 "exact loose read length"
Assert-Equal $exact.Sha256 (Get-Sha256 $exactBytes) "exact loose content"
$padded = [byte[]]::new(2048)
[Array]::Copy($exactBytes, $padded, $exactBytes.Length)
Assert-Equal $exact.Sector2048 (Get-Sha256 $padded) "exact cooked-sector override"
Assert-Equal $exact.Sector2352 $baselineSystem.Sector2352 "exact raw-sector fallback"

Remove-Item -LiteralPath $exactPath
$deleted = Invoke-Probe "deleted-fallback" "SYSTEM.CNF" $exactRoot
$results.Add($deleted)
Assert-Equal $deleted.Overrides 0 "deleted override count"
Assert-Equal $deleted.Size $baselineSystem.Size "deleted disc size fallback"
Assert-Equal $deleted.Sha256 $baselineSystem.Sha256 "deleted disc content fallback"

$singleRoot = Join-Path $runRoot "single-basename"
$singlePath = Join-Path $singleRoot "nested/SYSTEM.CNF"
[byte[]]$singleBytes = Write-Pattern $singlePath 71 23
$single = Invoke-Probe "single-basename" "SYSTEM.CNF" $singleRoot
$results.Add($single)
Assert-Equal $single.Overrides 1 "single-basename override count"
Assert-Equal $single.Sha256 (Get-Sha256 $singleBytes) "single-basename content"

$ambiguousRoot = Join-Path $runRoot "ambiguous-basename"
[void](Write-Pattern (Join-Path $ambiguousRoot "a/SYSTEM.CNF") 73 31)
[void](Write-Pattern (Join-Path $ambiguousRoot "b/SYSTEM.CNF") 79 47)
$ambiguous = Invoke-Probe "ambiguous-basename" "SYSTEM.CNF" $ambiguousRoot
$results.Add($ambiguous)
Assert-Equal $ambiguous.Overrides 0 "ambiguous-basename override count"
Assert-Equal $ambiguous.Sha256 $baselineSystem.Sha256 "ambiguous-basename disc fallback"

$rawRoot = Join-Path $runRoot "raw-sector"
$rawPath = Join-Path $rawRoot "VIDEO/ACTLOGO.STR"
[byte[]]$rawBytes = Write-Pattern $rawPath 4133 59
$raw = Invoke-Probe "raw-sector" "VIDEO/ACTLOGO.STR" $rawRoot
$results.Add($raw)
Assert-Equal $raw.Overrides 1 "raw-file override count"
Assert-Equal $raw.Size 4133 "raw-file changed size"
Assert-Equal $raw.Sha256 (Get-Sha256 $rawBytes) "raw-file loose content"
$rawFirstSector = [byte[]]::new(2048)
[Array]::Copy($rawBytes, $rawFirstSector, 2048)
Assert-Equal $raw.Sector2048 (Get-Sha256 $rawFirstSector) "raw-file cooked-sector override"
Assert-Equal $raw.Sector2352 $baselineVideo.Sector2352 "raw-file 2352-byte disc fallback"

$summaryPath = Join-Path $runRoot "summary.json"
[IO.File]::WriteAllText(
    $summaryPath,
    ($results | ConvertTo-Json -Depth 4))
Write-Host "[loose-test] PASS mutation size deletion fallback basename ambiguity and raw-sector boundary"
Write-Host "[loose-test] evidence=$summaryPath"
