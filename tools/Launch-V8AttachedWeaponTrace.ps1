param(
    [string]$PcsxPath = $env:PCSX_REDUX_EXE,
    [string]$CuePath = "C:\Programming\GitHub\Vigilante 8 recomp\Vigilante 8 (USA).cue",
    [string]$TraceScript = "C:\Programming\GitHub\Vigilante 8 recomp\tools\pcsx_v8_attached_weapon_trace.lua",
    [switch]$Debugger,
    [switch]$Interpreter,
    [switch]$FastBoot,
    [switch]$NoRun
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($PcsxPath)) {
    $candidates = @(
        "C:\Programming\GitHub\OpenNFS\tools\pcsx-redux\pcsx-redux.exe",
        "C:\Programming\GitHub\OpenNFS\tools\pcsx-redux\pcsx-redux-msvc.exe",
        "C:\Programming\GitHub\OpenNFS\tools\pcsx-redux\build\pcsx-redux.exe",
        "C:\Programming\GitHub\OpenNFS\tools\pcsx-redux\vsprojects\x64\Release\pcsx-redux.exe",
        "C:\Programming\GitHub\OpenNFS\tools\pcsx-redux\vsprojects\x64\Debug\pcsx-redux.exe",
        "C:\Programming\GitHub\OpenNFS\pcsx-redux.exe",
        "$env:LOCALAPPDATA\Microsoft\WinGet\Packages\GrumpyCoders.PCSX-Redux_Microsoft.Winget.Source_8wekyb3d8bbwe\pcsx-redux.exe"
    )
    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate) {
            $PcsxPath = $candidate
            break
        }
    }
}

if ([string]::IsNullOrWhiteSpace($PcsxPath) -or
    -not (Test-Path -LiteralPath $PcsxPath)) {
    throw "PCSX-Redux executable not found. Set PCSX_REDUX_EXE or pass -PcsxPath."
}
if (-not (Test-Path -LiteralPath $CuePath)) {
    throw "Vigilante 8 CUE not found: $CuePath"
}
if (-not (Test-Path -LiteralPath $TraceScript)) {
    throw "Trace script not found: $TraceScript"
}

function Quote-Arg([string]$Value) {
    '"' + ($Value -replace '"', '\"') + '"'
}

$analysis = "C:\Programming\GitHub\Vigilante 8 recomp\analysis"
New-Item -ItemType Directory -Force -Path $analysis | Out-Null
Remove-Item -LiteralPath (Join-Path $analysis "pcsx_v8_attached_weapon_trace.log") -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath (Join-Path $analysis "pcsx_v8_attached_weapon_trace_status.txt") -Force -ErrorAction SilentlyContinue

$args = @()
if ($Debugger) {
    $args += "-debugger"
}
if ($Interpreter) {
    $args += "-interpreter"
}
if ($FastBoot) {
    $args += "-fastboot"
}
$args += @(
    "-iso", (Quote-Arg $CuePath),
    "-dofile", (Quote-Arg $TraceScript),
    "-openglgpu"
)
if (-not $NoRun) {
    $args += "-run"
}

Write-Host "Launching PCSX-Redux attached weapon trace:"
Write-Host "  exe: $PcsxPath"
Write-Host "  cue: $CuePath"
Write-Host "  lua: $TraceScript"
$argumentLine = $args -join " "
Start-Process -FilePath $PcsxPath -ArgumentList $argumentLine -WorkingDirectory (Split-Path -Parent $PcsxPath)
