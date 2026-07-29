param(
    [string]$OutputRoot = "V8_ARCADE_WORK\vehicle_blends_corrected"
)

$ErrorActionPreference = "Stop"
$blender = "C:\Program Files\Blender Foundation\Blender 4.5\blender.exe"
$vehicles = @(
    "FartyDog",
    "GrooVan",
    "Incarcerator",
    "Jefferson",
    "Leprechaun",
    "Mammoth",
    "Manta",
    "Piranha",
    "Saucer",
    "Stag"
)
$inputs = Get-ChildItem "V8_ARCADE_WORK\decompressed" -Filter "*.ib" |
    Where-Object { $_.BaseName -in $vehicles } |
    Sort-Object Name |
    ForEach-Object { $_.FullName }

& $blender --background --factory-startup `
    --python "tools\v8_arcade\extract_vehicle_blends.py" `
    -- $inputs --out $OutputRoot
if ($LASTEXITCODE -ne 0) {
    throw "Blender vehicle export failed with exit code $LASTEXITCODE"
}
