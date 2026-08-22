$ErrorActionPreference = "Stop"

$workspace = (Resolve-Path -LiteralPath ".").Path
$separator = [IO.Path]::DirectorySeparatorChar
$targets = @(
    "V8_2_LOOSE\mods\v82_n64_super_dreamland\ui\n64_dreamlnd_selector_preview.ppm",
    "V8_2_LOOSE\mods\v82_n64_super_dreamland\loading_cards\n64_dreamlnd_loading_card_4x.ppm",
    "V8_2_LOOSE\mods\v82_n64_super_dreamland\loading_cards\n64_dreamlnd_loading_card_native_320x112.ppm",
    "V8_2_LOOSE\mods\v8_to_v82_guest_roster\SELECTOR_00.PPM",
    "V8_2_LOOSE\mods\v8_to_v82_guest_roster\SELECTOR_01.PPM",
    "V8_2_LOOSE\mods\v8_to_v82_guest_roster\SELECTOR_02.PPM"
)

$removed = 0
$bytes = 0L
foreach ($target in $targets) {
    $absolute = [IO.Path]::GetFullPath((Join-Path $workspace $target))
    if (-not $absolute.StartsWith(
            $workspace + $separator,
            [StringComparison]::OrdinalIgnoreCase)) {
        throw "Delete target escapes workspace: $absolute"
    }
    if (-not (Test-Path -LiteralPath $absolute -PathType Leaf)) {
        continue
    }
    $bytes += (Get-Item -LiteralPath $absolute).Length
    Remove-Item -LiteralPath $absolute -Force
    $removed++
}

foreach ($relative in @(
        "V8_2_LOOSE\mods\v82_n64_super_dreamland\ui",
        "V8_2_LOOSE\mods\v82_n64_super_dreamland\loading_cards")) {
    $directory = [IO.Path]::GetFullPath((Join-Path $workspace $relative))
    if ((Test-Path -LiteralPath $directory -PathType Container) -and
        -not (Get-ChildItem -LiteralPath $directory -Force)) {
        Remove-Item -LiteralPath $directory -Force
    }
}

[pscustomobject]@{
    RemovedFiles = $removed
    RemovedBytes = $bytes
} | ConvertTo-Json
