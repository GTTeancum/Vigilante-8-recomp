param(
    [string] $ArtifactRoot = "artifacts",
    [string[]] $PreserveRoots = @("artifacts\v8_to_v82_guest_roster"),
    [switch] $IncludeWorkspaceRootCaptures,
    [string[]] $AdditionalCaptureRoots = @(),
    [switch] $PruneObsoleteRuns
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

if ($IncludeWorkspaceRootCaptures) {
    $targets += @(
        Get-ChildItem -LiteralPath $workspace -File -Force |
            Where-Object {
                $_.Name -like "recompone_capture_*.ppm" -or
                    $_.Name -like "recompone_present_*.ppm" -or
                    $_.Name -like "recompone_vram_*.ppm" -or
                    $_.Name -like "vram_load_*.ppm"
            }
    )
}

$resolvedAdditionalCaptureRoots = @(
    foreach ($root in $AdditionalCaptureRoots) {
        if (-not (Test-Path -LiteralPath $root -PathType Container)) {
            continue
        }
        $resolved = (Resolve-Path -LiteralPath $root).Path
        if (-not $resolved.StartsWith(
                $workspace + $separator,
                [StringComparison]::OrdinalIgnoreCase)) {
            throw "Additional capture root escapes workspace: $resolved"
        }
        $resolved
    }
)

foreach ($root in $resolvedAdditionalCaptureRoots) {
    $targets += @(
        Get-ChildItem -LiteralPath $root -File -Force |
            Where-Object {
                $_.Name -like "recompone_capture_*.ppm" -or
                    $_.Name -like "recompone_present_*.ppm" -or
                    $_.Name -like "recompone_vram_*.ppm" -or
                    $_.Name -like "vram_load_*.ppm"
            }
    )
}

foreach ($target in $targets) {
    $isArtifact = $target.FullName.StartsWith(
        $resolvedArtifactRoot + $separator,
        [StringComparison]::OrdinalIgnoreCase)
    $isWorkspaceRootCapture =
        $IncludeWorkspaceRootCaptures -and
        [IO.Path]::GetDirectoryName($target.FullName).Equals(
            $workspace,
            [StringComparison]::OrdinalIgnoreCase)
    $isAdditionalCapture = [bool]($resolvedAdditionalCaptureRoots |
        Where-Object {
            [IO.Path]::GetDirectoryName($target.FullName).Equals(
                $_,
                [StringComparison]::OrdinalIgnoreCase)
        })
    if (-not $isArtifact -and -not $isWorkspaceRootCapture -and
            -not $isAdditionalCapture) {
        throw "Delete target escapes artifacts: $($target.FullName)"
    }
}

$bytes = ($targets | Measure-Object Length -Sum).Sum
if ($null -eq $bytes) {
    $bytes = 0L
}
foreach ($target in $targets) {
    Remove-Item -LiteralPath $target.FullName -Force
}

$pruneTargets = @()
$prunedBytes = 0L
if ($PruneObsoleteRuns) {
    $pruneTargets = @(
        Get-ChildItem -LiteralPath $resolvedArtifactRoot -Force |
            Where-Object {
                $candidate = $_.FullName.TrimEnd($separator)
                -not ($resolvedPreserveRoots | Where-Object {
                    $preserved = $_.TrimEnd($separator)
                    $preserved.Equals(
                        $candidate,
                        [StringComparison]::OrdinalIgnoreCase) -or
                    $preserved.StartsWith(
                        $candidate + $separator,
                        [StringComparison]::OrdinalIgnoreCase)
                })
            }
    )

    foreach ($target in $pruneTargets) {
        $parent = [IO.Path]::GetDirectoryName(
            $target.FullName.TrimEnd($separator))
        if (-not $parent.Equals(
                $resolvedArtifactRoot.TrimEnd($separator),
                [StringComparison]::OrdinalIgnoreCase)) {
            throw "Prune target is not an immediate artifact child: " +
                $target.FullName
        }
        if ($target.PSIsContainer) {
            $prunedBytes += (
                Get-ChildItem -LiteralPath $target.FullName -File -Recurse -Force |
                    Measure-Object Length -Sum
            ).Sum
        }
        else {
            $prunedBytes += $target.Length
        }
    }

    foreach ($target in $pruneTargets) {
        Remove-Item -LiteralPath $target.FullName -Recurse -Force
    }
}

[pscustomobject]@{
    RemovedFiles = $targets.Count
    RemovedBytes = $bytes
    RemovedGiB = [math]::Round($bytes / 1GB, 3)
    PrunedChildren = $pruneTargets.Count
    PrunedBytes = $prunedBytes
    PrunedGiB = [math]::Round($prunedBytes / 1GB, 3)
    ArtifactRoot = $resolvedArtifactRoot
    PreservedRoots = $resolvedPreserveRoots
    AdditionalCaptureRoots = $resolvedAdditionalCaptureRoots
} | ConvertTo-Json -Depth 3
