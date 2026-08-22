param(
    [string] $RunnableRoot = "V8_2_LOOSE"
)

$ErrorActionPreference = "Stop"
$workspace = (Resolve-Path -LiteralPath ".").Path
$resolvedRoot = (Resolve-Path -LiteralPath $RunnableRoot).Path
$separator = [IO.Path]::DirectorySeparatorChar

if (-not $resolvedRoot.StartsWith(
        $workspace + $separator,
        [StringComparison]::OrdinalIgnoreCase)) {
    throw "Runnable root escapes workspace: $resolvedRoot"
}

$targets = @(
    Get-ChildItem -LiteralPath $resolvedRoot -File |
        Where-Object {
            $_.Name -match "^v8(?:_|-).*\.log$" -or $_.Name -eq "v8.log"
        }
)
foreach ($target in $targets) {
    if (-not $target.FullName.StartsWith(
            $resolvedRoot + $separator,
            [StringComparison]::OrdinalIgnoreCase)) {
        throw "Log target escapes runnable root: $($target.FullName)"
    }
    Remove-Item -LiteralPath $target.FullName -Force
}

[pscustomobject]@{
    RemovedFiles = $targets.Count
    RemovedBytes = ($targets | Measure-Object Length -Sum).Sum
    RunnableRoot = $resolvedRoot
} | ConvertTo-Json
