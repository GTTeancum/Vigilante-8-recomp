param(
    [string] $ArtifactRoot = "artifacts",
    [string[]] $PreserveRoots = @("artifacts\v8_to_v82_guest_roster")
)

$ErrorActionPreference = "Stop"
$workspace = (Resolve-Path -LiteralPath ".").Path
$resolvedArtifactRoot = (Resolve-Path -LiteralPath $ArtifactRoot).Path
$separator = [IO.Path]::DirectorySeparatorChar

if (-not $resolvedArtifactRoot.StartsWith(
        $workspace + $separator,
        [StringComparison]::OrdinalIgnoreCase)) {
    throw "Artifact root escapes workspace: $resolvedArtifactRoot"
}

$resolvedPreserveRoots = @(
    foreach ($root in $PreserveRoots) {
        if (-not (Test-Path -LiteralPath $root)) {
            continue
        }
        $resolved = (Resolve-Path -LiteralPath $root).Path
        if (-not $resolved.StartsWith(
                $resolvedArtifactRoot + $separator,
                [StringComparison]::OrdinalIgnoreCase)) {
            throw "Preserve root escapes artifacts: $resolved"
        }
        $resolved
    }
)

$extensions = @(
    ".ppm", ".png", ".jpg", ".jpeg", ".webp", ".bmp", ".gif", ".mp4"
)
$targets = @(
    # Old candidate-runtime trees may preserve hidden attributes from copied
    # game folders. -Force is required for recursive discovery to enter them;
    # without it the cleanup can falsely report zero while raster artifacts
    # remain beneath a hidden directory.
    Get-ChildItem -LiteralPath $resolvedArtifactRoot -Recurse -File -Force |
        Where-Object {
            $candidate = $_.FullName
            $extensions -contains $_.Extension.ToLowerInvariant() -and
                -not ($resolvedPreserveRoots | Where-Object {
                    $candidate.StartsWith(
                        $_ + $separator,
                        [StringComparison]::OrdinalIgnoreCase)
                })
        }
)

foreach ($target in $targets) {
    if (-not $target.FullName.StartsWith(
            $resolvedArtifactRoot + $separator,
            [StringComparison]::OrdinalIgnoreCase)) {
        throw "Delete target escapes artifacts: $($target.FullName)"
    }
}

$bytes = ($targets | Measure-Object Length -Sum).Sum
foreach ($target in $targets) {
    Remove-Item -LiteralPath $target.FullName -Force
}

[pscustomobject]@{
    RemovedFiles = $targets.Count
    RemovedBytes = $bytes
    RemovedGiB = [math]::Round($bytes / 1GB, 3)
    ArtifactRoot = $resolvedArtifactRoot
    PreservedRoots = $resolvedPreserveRoots
} | ConvertTo-Json -Depth 3
