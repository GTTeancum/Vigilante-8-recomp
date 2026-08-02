param(
    [string]$ArtifactRoot =
        "artifacts\renderer-rewrite-final-staged",
    [string]$Executable =
        "V8_2_LOOSE\Vigilante82PC.exe",
    [string]$InterfaceConfig =
        "V8_2_LOOSE\interface.ini"
)

$ErrorActionPreference = "Stop"

function Read-Json([string]$Path) {
    Get-Content -LiteralPath $Path -Raw | ConvertFrom-Json
}

function Require([bool]$Condition, [string]$Message) {
    if (-not $Condition) {
        throw $Message
    }
}

function Has-All([object[]]$Actual, [object[]]$Expected) {
    foreach ($item in $Expected) {
        if ($Actual -notcontains $item) {
            return $false
        }
    }
    return $true
}

$artifactPath = (Resolve-Path -LiteralPath $ArtifactRoot).Path
$executablePath = (Resolve-Path -LiteralPath $Executable).Path
$configPath = (Resolve-Path -LiteralPath $InterfaceConfig).Path
$looseRoot = Split-Path -Parent $executablePath
$executableItem = Get-Item -LiteralPath $executablePath
$executableHash =
    (Get-FileHash -LiteralPath $executablePath -Algorithm SHA256).Hash

$matrix = Read-Json (Join-Path $artifactPath "renderer-matrix\acceptance.json")
$transitions = Read-Json (Join-Path $artifactPath "transitions\acceptance.json")
$selector = Read-Json (Join-Path $artifactPath "selector\acceptance.json")
$probeReport = Read-Json (Join-Path $artifactPath "probes\acceptance.json")
$ui = Read-Json (Join-Path $artifactPath "ui\acceptance.json")
$media = Read-Json (Join-Path $artifactPath "boot-media\acceptance.json")
$harbor = Read-Json (Join-Path $artifactPath "harbor-long\summary.json")
$harborRenderer =
    Read-Json (Join-Path $artifactPath "harbor-long\renderer-acceptance.json")
$stock = Read-Json (Join-Path $artifactPath "stock-fallback\summary.json")

$config = Get-Content -LiteralPath $configPath -Raw
$bootStdout =
    Get-Content -LiteralPath (Join-Path $artifactPath "boot-media\stdout.log") -Raw
$bootStderr =
    Get-Content -LiteralPath (Join-Path $artifactPath "boot-media\stderr.log") -Raw
$stockLogPath = Join-Path $artifactPath `
    "stock-fallback\01_c00_00_levels_route66.stdout.log"
$stockStdout = Get-Content -LiteralPath $stockLogPath -Raw
$matrixCases = [string]$matrix.cases_passed + "/" +
    [string]$matrix.cases_total
$transitionCases = [string]$transitions.cases_passed + "/" +
    [string]$transitions.cases_total
$probeCases = [string]$probeReport.cases_passed + "/" +
    [string]$probeReport.cases_total
$harborVisibleWorldFallback = @(
    $harborRenderer.renderer_logs)[0].maximum_visible_world_fallback_percent

Require $matrix.passed "Enhanced renderer matrix failed."
Require ($matrix.cases_passed -eq 30 -and $matrix.cases_total -eq 30) `
    "Enhanced renderer matrix is incomplete."
Require ($matrix.stock_cases -eq 18 -and $matrix.guest_cases -eq 12) `
    "Enhanced renderer matrix did not cover all stock maps and guests."
Require ($matrix.executable_sha256 -eq $executableHash) `
    "Enhanced renderer matrix was not run against the staged executable."

Require $transitions.passed "Match transition sweep failed."
Require ($transitions.cases_passed -eq 13 -and
         $transitions.cases_total -eq 13) `
    "Match transition sweep is incomplete."
Require ($transitions.executable_sha256 -eq $executableHash) `
    "Transition sweep was not run against the staged executable."

Require $selector.passed "Native selector lifecycle failed."
Require ($selector.captures.Count -eq 12) `
    "Native selector did not capture all imported vehicles."
Require ($selector.enemy_stage -and $selector.enemy_capture -and
         $selector.clean_exit) `
    "Native enemy-selector handoff or teardown failed."
Require ($selector.executable_sha256 -eq $executableHash) `
    "Selector sweep was not run against the staged executable."

Require $probeReport.passed "Deterministic subsystem probes failed."
Require ($probeReport.cases_passed -eq 6 -and
         $probeReport.cases_total -eq 6) `
    "Deterministic subsystem probe set is incomplete."
Require ($probeReport.executable_sha256 -eq $executableHash) `
    "Subsystem probes were not run against the staged executable."

Require $ui.passed "Pause/objective UI acceptance failed."
Require ($ui.overlays.Count -eq 2) "Pause/objective UI coverage is incomplete."
Require (($ui.overlays | Where-Object { -not $_.passed }).Count -eq 0) `
    "A pause/objective UI overlay failed."
Require (($ui.overlays |
    Where-Object { $_.tall_neutral_gray_columns -ne 0 }).Count -eq 0) `
    "A pause/objective UI overlay contains a gray-column regression."

Require $media.passed "Boot-media temporal acceptance failed."
Require ($media.frames -eq 300 -and $media.unique_frames -ge 200) `
    "Boot-media temporal proof is incomplete."
Require ($media.one_frame_reversions.Count -eq 0) `
    "Boot-media proof contains a one-frame reversion."
Require ($bootStdout -match "MOVIES/ACTLOGO\.STR.*frames=226") `
    "Activision logo stream did not complete."
Require ($bootStdout -match "MOVIES/LUXOFLUX\.STR.*frames=90") `
    "Luxoflux logo stream did not complete."
Require ($bootStdout -match "MOVIES/INTRO\.STR.*frames=167") `
    "Intro stream did not play before the scripted skip."
Require ($bootStdout -match "\[CDDA\] loose track=2.*track02\.ogg") `
    "Loose-files main-menu OGG did not start."
Require ($bootStdout -notmatch "(?i)fatal|unmapped call|unhandled exception") `
    "Boot-media stdout contains a fatal marker."
Require ($bootStderr -notmatch "(?i)fatal|unmapped call|unhandled exception") `
    "Boot-media stderr contains a fatal marker."

Require ($harbor.totals.passed -eq 1 -and $harbor.totals.failed -eq 0) `
    "Long Harbor coverage run failed."
Require ($harbor.requestedFrames -eq 2700) `
    "Long Harbor coverage run is shorter than required."
Require (Has-All @($harbor.totals.weaponKindsArmed) @(1, 2, 3, 4, 5, 6, 7)) `
    "Long Harbor coverage did not arm every weapon kind."
Require (Has-All @($harbor.totals.weaponKindsFired) @(1, 2, 3, 4, 5, 6, 7)) `
    "Long Harbor coverage did not fire every weapon kind."
Require ($harbor.totals.specialCommands.Count -eq 18) `
    "Long Harbor coverage did not execute all special commands."
Require ($harbor.totals.collisionStreamRejections -eq 0) `
    "Long Harbor coverage rejected a collision stream."
Require $harborRenderer.passed "Long Harbor renderer acceptance failed."

Require ($stock.totals.passed -eq 1 -and $stock.totals.failed -eq 0) `
    "Stock fallback smoke failed."
Require ($stock.requestedFrames -eq 720) `
    "Stock fallback smoke is shorter than required."
Require ($stockStdout -match "hle=False") `
    "Stock fallback did not use the preserved software renderer."

$requiredConfig = @(
    "GraphicsPreset=Enhanced",
    "PerspectiveCorrectTextures=True",
    "EnhancedDepthBuffer=True",
    "TrueColor=True",
    "GeometryCorrection=True",
    "PreciseCulling=True",
    "EnhancedFog=True",
    "LevelOfDetail=Maximum"
)
foreach ($line in $requiredConfig) {
    Require ($config -match "(?m)^$([regex]::Escape($line))\r?$") `
        "Shipping config is missing '$line'."
}

$proofFiles = @(
    "boot-media\boot_media_5s.mp4",
    "boot-media\boot_stage_0480_mid.bmp",
    "boot-media\boot_stage_0540_mid.bmp",
    "boot-media\boot_stage_0900_mid.bmp",
    "selector\guest_selector.bmp",
    "selector\enemy_selector.bmp",
    "ui\proofs\pause.bmp",
    "ui\proofs\objective.bmp"
)
foreach ($file in $proofFiles) {
    Require (Test-Path -LiteralPath (Join-Path $artifactPath $file)) `
        "Compact proof '$file' is missing."
}

$dllCount = @(
    Get-ChildItem -LiteralPath $looseRoot -Filter "*.dll" -File
).Count
Require ($dllCount -eq 0) "Loose-files package contains adjacent DLLs."

$report = [ordered]@{
    schema = 1
    passed = $true
    generated_utc = [DateTime]::UtcNow.ToString("o")
    staged_executable = [ordered]@{
        path = $executablePath
        size_bytes = $executableItem.Length
        sha256 = $executableHash
        adjacent_dlls = $dllCount
    }
    shipping_default = [ordered]@{
        preset = "Enhanced"
        config = $configPath
        stock_renderer_retained = $true
        stock_renderer_smoke_frames = $stock.requestedFrames
    }
    acceptance = [ordered]@{
        enhanced_renderer = [ordered]@{
            passed = $matrix.passed
            cases = $matrixCases
            stock_maps = $matrix.stock_cases
            imported_vehicles = $matrix.guest_cases
            frames_per_case = $matrix.frames_per_case
            maximum_visible_world_fallback_percent =
                $matrix.maximum_visible_world_fallback_percent
        }
        match_transitions = [ordered]@{
            passed = $transitions.passed
            cases = $transitionCases
        }
        native_selector = [ordered]@{
            passed = $selector.passed
            imported_captures = $selector.captures.Count
            native_enemy_stage = $selector.enemy_stage
            clean_exit = $selector.clean_exit
        }
        deterministic_probes = [ordered]@{
            passed = $probeReport.passed
            cases = $probeCases
        }
        pause_objective_ui = [ordered]@{
            passed = $ui.passed
            overlays = $ui.overlays.Count
        }
        boot_video_and_menu_audio = [ordered]@{
            passed = $media.passed
            video_frames = $media.frames
            unique_frames = $media.unique_frames
            streams = @("ACTLOGO", "LUXOFLUX", "INTRO")
            menu_track = "music/track02.ogg"
        }
        long_harbor_coverage = [ordered]@{
            passed = $true
            frames = $harbor.requestedFrames
            weapons_armed = $harbor.totals.weaponKindsArmed.Count
            weapons_fired = $harbor.totals.weaponKindsFired.Count
            special_commands = $harbor.totals.specialCommands.Count
            powerups = $harbor.totals.powerups.Count
            collision_stream_rejections =
                $harbor.totals.collisionStreamRejections
            maximum_visible_world_fallback_percent =
                $harborVisibleWorldFallback
        }
    }
    compact_visual_proofs = $proofFiles
    source_reports = @(
        "renderer-matrix/acceptance.json",
        "transitions/acceptance.json",
        "selector/acceptance.json",
        "probes/acceptance.json",
        "ui/acceptance.json",
        "boot-media/acceptance.json",
        "harbor-long/summary.json",
        "harbor-long/renderer-acceptance.json",
        "stock-fallback/summary.json"
    )
}

$outputPath = Join-Path $artifactPath "acceptance.json"
$report | ConvertTo-Json -Depth 8 |
    Set-Content -LiteralPath $outputPath -Encoding UTF8
Write-Output $outputPath
