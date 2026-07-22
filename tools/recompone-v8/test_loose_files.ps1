param(
    [string]$Exe = "reference/generated/recompiled/bin/Release/net10.0/Vigilante8PC.exe",
    [string]$LooseRoot = "PS1 game",
    [string]$OutputRoot = "reference/generated/loose-tests"
)

$ErrorActionPreference = "Stop"
$repo = (Resolve-Path (Join-Path $PSScriptRoot "../..")).Path
$exePath = (Resolve-Path (Join-Path $repo $Exe)).Path
$sourceRoot = (Resolve-Path (Join-Path $repo $LooseRoot)).Path
$outputBase = Join-Path $repo $OutputRoot
$runRoot = Join-Path $outputBase (Get-Date -Format "yyyyMMdd-HHmmssfff")
$fixtureRoot = Join-Path $runRoot "standalone"
New-Item -ItemType Directory -Path $fixtureRoot -Force | Out-Null
Copy-Item -Path (Join-Path $sourceRoot "*") -Destination $fixtureRoot -Recurse -Force

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

function Invoke-Probe([string]$Label, [string]$GamePath) {
    $arguments = @("--loose", $fixtureRoot, "--probe-file", $GamePath)
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
    $filesMatch = [regex]::Match($sourceLine, "files=(?<count>[0-9]+)")
    if (!$filesMatch.Success) { throw "$Label probe omitted the loose file count" }
    if ($sourceLine -notmatch "mode=standalone-loose") {
        throw "$Label did not run in standalone loose mode"
    }

    return [PSCustomObject]@{
        Label = $Label
        Files = [int]$filesMatch.Groups["count"].Value
        Size = [int64]$values["size"]
        Bytes = [int64]$values["bytes"]
        Sha256 = $values["sha256"]
        Sector2048 = $values["sector2048"]
        Sector2336 = $values["sector2336"]
        Sector2352 = $values["sector2352"]
    }
}

function Invoke-ExpectedFailure(
    [string]$Label,
    [string]$GamePath,
    [string]$RequiredMessage
) {
    $oldPreference = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    try {
        $output = (& $exePath --loose $fixtureRoot --probe-file $GamePath 2>&1 | Out-String)
        $exitCode = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $oldPreference
    }
    [IO.File]::WriteAllText((Join-Path $runRoot "$Label.log"), $output)
    if ($exitCode -eq 0) { throw "$Label unexpectedly succeeded" }
    $normalizedOutput = ($output -replace "\s+", " ")
    $normalizedMessage = ($RequiredMessage -replace "\s+", " ")
    if ($normalizedOutput -notmatch [regex]::Escape($normalizedMessage)) {
        throw "$Label did not report '$RequiredMessage'`n$output"
    }
    return [PSCustomObject]@{
        Label = $Label
        ExpectedFailure = $true
        ExitCode = $exitCode
        Message = $RequiredMessage
    }
}

function New-Pattern([int]$Length, [int]$Seed) {
    $bytes = [byte[]]::new($Length)
    for ($i = 0; $i -lt $bytes.Length; $i++) {
        $bytes[$i] = [byte](($i * 37 + $Seed) -band 0xFF)
    }
    return ,$bytes
}

function Expand-CookedSector([byte[]]$Cooked, [int]$Prefix, [int]$Suffix) {
    $result = [byte[]]::new($Prefix + $Cooked.Length + $Suffix)
    [Array]::Copy($Cooked, 0, $result, $Prefix, $Cooked.Length)
    return ,$result
}

$results = [Collections.Generic.List[object]]::new()
$baselineSystem = Invoke-Probe "baseline-system" "SYSTEM.CNF"
$baselineData = Invoke-Probe "baseline-data" "COMMON.EXP"
$baselineVideo = Invoke-Probe "baseline-video" "VIDEO/ACTLOGO.STR"
$results.Add($baselineSystem)
$results.Add($baselineData)
$results.Add($baselineVideo)
Assert-Equal $baselineSystem.Files 69 "standalone required-file count"

$dataPath = Join-Path $fixtureRoot "COMMON.EXP"
$originalData = [IO.File]::ReadAllBytes($dataPath)
[byte[]]$mutatedData = New-Pattern 4097 11
[IO.File]::WriteAllBytes($dataPath, $mutatedData)
$mutated = Invoke-Probe "exact-mutated-size" "COMMON.EXP"
$results.Add($mutated)
Assert-Equal $mutated.Size 4097 "exact loose size"
Assert-Equal $mutated.Bytes 4097 "exact loose read length"
Assert-Equal $mutated.Sha256 (Get-Sha256 $mutatedData) "exact loose content"
$firstCooked = [byte[]]::new(2048)
[Array]::Copy($mutatedData, $firstCooked, 2048)
Assert-Equal $mutated.Sector2048 (Get-Sha256 $firstCooked) "mutated cooked sector"
Assert-Equal $mutated.Sector2336 (Get-Sha256 (Expand-CookedSector $firstCooked 8 280)) "mutated 2336 sector"
Assert-Equal $mutated.Sector2352 (Get-Sha256 (Expand-CookedSector $firstCooked 24 280)) "mutated 2352 sector"
[IO.File]::WriteAllBytes($dataPath, $originalData)

$decoyPath = Join-Path $fixtureRoot "mods/COMMON.EXP"
New-Item -ItemType Directory -Path (Split-Path $decoyPath) -Force | Out-Null
[IO.File]::WriteAllBytes($decoyPath, (New-Pattern 73 23))
$exactWins = Invoke-Probe "exact-path-beats-decoy" "COMMON.EXP"
$results.Add($exactWins)
Assert-Equal $exactWins.Sha256 $baselineData.Sha256 "exact path authority"

$missingPath = Join-Path $fixtureRoot "missing/COMMON.EXP"
New-Item -ItemType Directory -Path (Split-Path $missingPath) -Force | Out-Null
Move-Item -LiteralPath $dataPath -Destination $missingPath
try {
    $missing = Invoke-ExpectedFailure "missing-required-no-fallback" "SYSTEM.CNF" "Standalone loose install is missing required assets (no BIN/CUE fallback): COMMON.EXP"
    $results.Add($missing)
} finally {
    Move-Item -LiteralPath $missingPath -Destination $dataPath
}

$musicPath = Join-Path $fixtureRoot "music/BOOGIE.ogg"
$missingMusicPath = Join-Path $fixtureRoot "missing/BOOGIE.ogg"
Move-Item -LiteralPath $musicPath -Destination $missingMusicPath
try {
    $missingMusic = Invoke-ExpectedFailure "missing-music-no-fallback" "SYSTEM.CNF" "Loose music track 02 is missing"
    $results.Add($missingMusic)
} finally {
    Move-Item -LiteralPath $missingMusicPath -Destination $musicPath
}

$videoPath = Join-Path $fixtureRoot "VIDEO/ACTLOGO.STR"
[byte[]]$rawStream = New-Pattern (2336 * 2) 59
[IO.File]::WriteAllBytes($videoPath, $rawStream)
$raw = Invoke-Probe "raw-stream-mutated-size" "VIDEO/ACTLOGO.STR"
$results.Add($raw)
Assert-Equal $raw.Size 4096 "raw stream logical size"
Assert-Equal $raw.Bytes 4096 "raw stream logical read length"
$logicalStream = [byte[]]::new(4096)
[Array]::Copy($rawStream, 8, $logicalStream, 0, 2048)
[Array]::Copy($rawStream, 2336 + 8, $logicalStream, 2048, 2048)
Assert-Equal $raw.Sha256 (Get-Sha256 $logicalStream) "raw stream logical content"
$rawFirst = [byte[]]::new(2336)
[Array]::Copy($rawStream, $rawFirst, 2336)
$rawCooked = [byte[]]::new(2048)
[Array]::Copy($rawFirst, 8, $rawCooked, 0, 2048)
Assert-Equal $raw.Sector2048 (Get-Sha256 $rawCooked) "raw stream cooked sector"
Assert-Equal $raw.Sector2336 (Get-Sha256 $rawFirst) "raw stream full sector"
Assert-Equal $raw.Sector2352 (Get-Sha256 (Expand-CookedSector $rawFirst 16 0)) "raw stream 2352 synthesis"

$summaryPath = Join-Path $runRoot "summary.json"
[IO.File]::WriteAllText(
    $summaryPath,
    ($results | ConvertTo-Json -Depth 5))
Write-Host "[loose-test] PASS standalone mutation size exact-path missing-file raw-stream and music cases"
Write-Host "[loose-test] evidence=$summaryPath"
