[CmdletBinding()]
param(
    [string]$InputScript = 'input_scripts\boot_probe.csv',
    [int]$StopFrame = 900,
    [int]$CaptureEvery = 60,
    [string]$CaptureFrames = '',
    [switch]$DumpRdram,
    [switch]$DumpRdpCommands,
    [ValidateSet('angrylion-cxd4', 'gliden64-hle')]
    [string]$Profile = 'angrylion-cxd4',
    [ValidateSet('script', 'rmg')]
    [string]$InputMode = 'script',
    [ValidateSet('proxy', 'direct')]
    [string]$GfxMode = 'proxy',
    [ValidateSet('null', 'trace')]
    [string]$AudioMode = 'null',
    [string]$Output
)

$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath($PSScriptRoot)
$repo = [IO.Path]::GetFullPath((Join-Path $root '..\..'))
$vendor = Join-Path $root 'vendor\RMG-Portable-Windows64-v0.9.0'
$bin = Join-Path $root 'bin'
$rom = Join-Path $repo 'Vigilante 8 (U) (!).n64'
$config = Join-Path $root 'state\config'

if ([string]::IsNullOrWhiteSpace($Output)) {
    $Output = Join-Path $repo (
        'artifacts\n64_reference\' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
}
$Output = [IO.Path]::GetFullPath($Output)
$inputPath = [IO.Path]::GetFullPath((Join-Path $root $InputScript))

if ($Profile -eq 'angrylion-cxd4') {
    $realGfx = Join-Path $vendor 'Plugin\GFX\mupen64plus-video-angrylion-plus.dll'
    $rsp = Join-Path $vendor 'Plugin\RSP\mupen64plus-rsp-cxd4.dll'
} else {
    $realGfx = Join-Path $vendor 'Plugin\GFX\mupen64plus-video-GLideN64.dll'
    $rsp = Join-Path $vendor 'Plugin\RSP\mupen64plus-rsp-hle.dll'
}

$required = @(
    $rom,
    (Join-Path $vendor 'Core\mupen64plus.dll'),
    $realGfx,
    $rsp,
    (Join-Path $vendor 'Plugin\Audio\RMG-Audio.dll'),
    (Join-Path $bin 'v8_n64_headless.exe'),
    (Join-Path $bin 'v8_n64_input.dll'),
    (Join-Path $bin 'v8_n64_gfx_trace.dll'),
    (Join-Path $bin 'v8_n64_audio_trace.dll'),
    (Join-Path $bin 'v8_n64_audio_null.dll'),
    (Join-Path $bin 'v8_n64_rsp_trace.dll'),
    $inputPath
)
foreach ($path in $required) {
    if (-not (Test-Path -LiteralPath $path)) {
        throw "Missing N64 reference-harness input: $path"
    }
}

New-Item -ItemType Directory -Path $Output -Force | Out-Null
New-Item -ItemType Directory -Path $config -Force | Out-Null

$env:V8_N64_VENDOR = $vendor
$env:V8_N64_ROM = $rom
$env:V8_N64_CONFIG = $config
$env:V8_N64_INPUT_SCRIPT = $inputPath
$env:V8_N64_TRACE_OUTPUT = $Output
$env:V8_N64_GFX_PROXY = if ($GfxMode -eq 'direct') {
    $realGfx
} else {
    Join-Path $bin 'v8_n64_gfx_trace.dll'
}
$realAudio = Join-Path $vendor 'Plugin\Audio\RMG-Audio.dll'
$audioProxy = Join-Path $bin 'v8_n64_audio_trace.dll'
$audioNull = Join-Path $bin 'v8_n64_audio_null.dll'
$env:V8_N64_AUDIO = if ($AudioMode -eq 'trace') { $audioProxy } else { $audioNull }
$env:V8_N64_REAL_AUDIO = $realAudio
$env:V8_N64_INPUT_PLUGIN = if ($InputMode -eq 'rmg') {
    Join-Path $vendor 'Plugin\Input\RMG-Input.dll'
} else {
    Join-Path $bin 'v8_n64_input.dll'
}
$env:V8_N64_REAL_GFX = $realGfx
$rspProxy = Join-Path $bin 'v8_n64_rsp_trace.dll'
$env:V8_N64_RSP = $rspProxy
$env:V8_N64_REAL_RSP = $rsp
$env:V8_N64_STOP_FRAME = [string]$StopFrame
$env:V8_N64_CAPTURE_EVERY = [string]$CaptureEvery
$env:V8_N64_CAPTURE_FRAMES = $CaptureFrames
$env:V8_N64_DUMP_RDRAM = if ($DumpRdram) { '1' } else { '0' }
$env:V8_N64_DUMP_RDP_COMMANDS = if ($DumpRdpCommands) { '1' } else { '0' }

$romHash = Get-FileHash -Algorithm SHA256 -LiteralPath $rom
$runtimeHash = Get-FileHash -Algorithm SHA256 -LiteralPath (
    Join-Path $vendor '..\RMG-Portable-Windows64-v0.9.0.zip')
$headless = Join-Path $bin 'v8_n64_headless.exe'
$inputPlugin = Join-Path $bin 'v8_n64_input.dll'
$gfxProxy = Join-Path $bin 'v8_n64_gfx_trace.dll'
$buildManifest = Join-Path $bin 'build_manifest.json'
$metadata = [ordered]@{
    schema = 'v8.n64-reference-run.v2'
    generated_at = (Get-Date -Format o)
    profile = $Profile
    input_mode = $InputMode
    gfx_mode = $GfxMode
    audio_mode = $AudioMode
    stop_frame = $StopFrame
    capture_every = $CaptureEvery
    capture_frames = $CaptureFrames
    dump_rdram = [bool]$DumpRdram
    dump_rdp_commands = [bool]$DumpRdpCommands
    rom = $rom
    rom_sha256 = $romHash.Hash
    input_script = $inputPath
    input_script_sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $inputPath).Hash
    portable_runtime_sha256 = $runtimeHash.Hash
    headless_sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $headless).Hash
    input_plugin_sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $inputPlugin).Hash
    gfx_proxy_sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $gfxProxy).Hash
    audio_proxy_sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $audioProxy).Hash
    audio_null_sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $audioNull).Hash
    real_audio_sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $realAudio).Hash
    real_gfx_sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $realGfx).Hash
    rsp_proxy_sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $rspProxy).Hash
    real_rsp_sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $rsp).Hash
    build_manifest_sha256 = if (Test-Path -LiteralPath $buildManifest) {
        (Get-FileHash -Algorithm SHA256 -LiteralPath $buildManifest).Hash
    } else {
        $null
    }
    hidden_desktop = $true
}
$metadata | ConvertTo-Json -Depth 4 |
    Set-Content -LiteralPath (Join-Path $Output 'run_manifest.json') -Encoding UTF8

& (Join-Path $bin 'v8_n64_headless.exe')
if ($LASTEXITCODE -ne 0) {
    throw "Hidden N64 reference run failed: $LASTEXITCODE"
}

Get-ChildItem -LiteralPath $Output -File |
    Select-Object Name,Length,LastWriteTime
