param(
    [string]$Version = "0.3.1"
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$source = (Resolve-Path (Join-Path $PSScriptRoot "vigilante8_vehicle_tools")).Path
$artifactDirectory = Join-Path $repoRoot "artifacts"
$archivePath = Join-Path $artifactDirectory "vigilante8_vehicle_tools-$Version.zip"
$stagingRoot = Join-Path ([System.IO.Path]::GetTempPath()) (
    "v8_vehicle_tools_" + [System.Guid]::NewGuid().ToString("N")
)
$stagingPackage = Join-Path $stagingRoot "vigilante8_vehicle_tools"
$productionFiles = @(
    "__init__.py",
    "README.md",
    "authored_scene.py",
    "authored_ui.py",
    "binary.py",
    "compiler.py",
    "conversion.py",
    "iff.py",
    "project.py",
    "registry.py",
    "stats.py",
    "xobf.py"
)

New-Item -ItemType Directory -Path $stagingPackage -Force | Out-Null
New-Item -ItemType Directory -Path $artifactDirectory -Force | Out-Null

try {
    foreach ($fileName in $productionFiles) {
        $sourceFile = Join-Path $source $fileName
        if (-not (Test-Path -LiteralPath $sourceFile -PathType Leaf)) {
            throw "Required production add-on file is missing: $sourceFile"
        }
        Copy-Item -LiteralPath $sourceFile -Destination $stagingPackage
    }
    if (Test-Path -LiteralPath $archivePath) {
        Remove-Item -LiteralPath $archivePath
    }
    Compress-Archive -Path $stagingPackage -DestinationPath $archivePath
}
finally {
    if (Test-Path -LiteralPath $stagingRoot) {
        $resolvedStaging = (Resolve-Path -LiteralPath $stagingRoot).Path
        $resolvedTemp = (Resolve-Path -LiteralPath ([System.IO.Path]::GetTempPath())).Path
        if ($resolvedStaging.StartsWith($resolvedTemp, [System.StringComparison]::OrdinalIgnoreCase)) {
            Remove-Item -LiteralPath $resolvedStaging -Recurse -Force
        }
    }
}

Write-Output $archivePath
