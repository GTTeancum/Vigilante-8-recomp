[CmdletBinding(SupportsShouldProcess = $true)]
param(
    [switch]$KeepArtifacts,
    [switch]$SkipGitMaintenance
)

$ErrorActionPreference = 'Stop'

$repoRoot = [IO.Path]::GetFullPath($PSScriptRoot)
$repoPrefix = $repoRoot.TrimEnd('\', '/') + [IO.Path]::DirectorySeparatorChar

if (-not (Test-Path -LiteralPath (Join-Path $repoRoot 'AGENTS.md')) -or
    -not (Test-Path -LiteralPath (Join-Path $repoRoot '.git'))) {
    throw "clean.ps1 must remain in the Vigilante 8 repository root."
}

$script:removedBytes = [int64]0
$script:removedItems = 0
$script:skippedItems = 0

function Get-PathBytes {
    param([Parameter(Mandatory)][string]$LiteralPath)

    if (-not (Test-Path -LiteralPath $LiteralPath)) {
        return [int64]0
    }

    $item = Get-Item -LiteralPath $LiteralPath -Force
    if (-not $item.PSIsContainer) {
        return [int64]$item.Length
    }

    $sum = (Get-ChildItem -LiteralPath $LiteralPath -File -Recurse -Force `
            -ErrorAction SilentlyContinue | Measure-Object Length -Sum).Sum
    if ($null -eq $sum) {
        return [int64]0
    }
    return [int64]$sum
}

function Remove-RepoPath {
    param([Parameter(Mandatory)][string]$LiteralPath)

    if (-not (Test-Path -LiteralPath $LiteralPath)) {
        return
    }

    $fullPath = [IO.Path]::GetFullPath($LiteralPath)
    if ($fullPath -eq $repoRoot -or
        -not $fullPath.StartsWith($repoPrefix, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to remove path outside the repository: $fullPath"
    }

    $bytes = Get-PathBytes -LiteralPath $fullPath
    $relativePath = $fullPath.Substring($repoPrefix.Length)
    if ($PSCmdlet.ShouldProcess($relativePath, 'Remove regenerable output')) {
        try {
            Remove-Item -LiteralPath $fullPath -Recurse -Force
        } catch {
            Write-Warning "Could not completely remove active output '$relativePath': $($_.Exception.Message)"
        }

        $remainingBytes = Get-PathBytes -LiteralPath $fullPath
        $script:removedBytes += [math]::Max(0, $bytes - $remainingBytes)
        if (Test-Path -LiteralPath $fullPath) {
            $script:skippedItems++
        } else {
            $script:removedItems++
        }
    }
}

function Remove-GeneratedChildrenExcept {
    param(
        [Parameter(Mandatory)][string]$Directory,
        [Parameter(Mandatory)][string[]]$Keep
    )

    if (-not (Test-Path -LiteralPath $Directory)) {
        return
    }

    $keepSet = [Collections.Generic.HashSet[string]]::new(
        [StringComparer]::OrdinalIgnoreCase)
    foreach ($name in $Keep) {
        [void]$keepSet.Add($name)
    }

    foreach ($child in Get-ChildItem -LiteralPath $Directory -Force) {
        if (-not $keepSet.Contains($child.Name)) {
            Remove-RepoPath -LiteralPath $child.FullName
        }
    }
}

# Evidence captures, crash dumps, and historical test packages are reproducible
# and are explicitly ignored by Git. Current runnable handoffs live in
# PS1 game/ and V8_2_LOOSE/, neither of which is removed here.
if (-not $KeepArtifacts) {
    Remove-RepoPath -LiteralPath (Join-Path $repoRoot 'artifacts')
}

# Tool downloads, extracted comparison discs, emulator sandboxes, and one-off
# inspection files under .tmp are disposable scratch data.
Remove-RepoPath -LiteralPath (Join-Path $repoRoot '.tmp')

# CMake build trees are wholly regenerable.
foreach ($directory in Get-ChildItem -LiteralPath $repoRoot -Directory -Force |
         Where-Object { $_.Name -eq 'build' -or $_.Name -like 'build-*' }) {
    Remove-RepoPath -LiteralPath $directory.FullName
}

# Retain recompiler source/configuration while dropping old publishes, tests,
# captures, logs, and dumps from the ignored generated trees.
$referenceGenerated = Join-Path $repoRoot 'reference\generated'
Remove-GeneratedChildrenExcept -Directory $referenceGenerated -Keep @(
    'dreamland-generated',
    'dreamland.recompone.json',
    'empty-mods',
    'function-maps',
    'recompiled',
    'three-hour-handoff-preserve-hashes.json',
    'v8.recompone.json'
)

$v82Generated = Join-Path $repoRoot 'reference-v8-2\generated'
Remove-GeneratedChildrenExcept -Directory $v82Generated -Keep @(
    'recompiled',
    'v82.recompone.json'
)

# Generated project roots contain source files at their top level. Any child
# directory is a compiler or publish output and may be regenerated.
foreach ($generatedProject in @(
    (Join-Path $referenceGenerated 'recompiled'),
    (Join-Path $v82Generated 'recompiled')
)) {
    if (Test-Path -LiteralPath $generatedProject) {
        foreach ($childDirectory in Get-ChildItem -LiteralPath $generatedProject `
                 -Directory -Force) {
            Remove-RepoPath -LiteralPath $childDirectory.FullName
        }
        foreach ($transientFile in Get-ChildItem -LiteralPath $generatedProject `
                 -File -Force | Where-Object {
                     $_.Extension -in @('.log', '.obj', '.pdb')
                 }) {
            Remove-RepoPath -LiteralPath $transientFile.FullName
        }
    }
}

# Clean bin/obj beside every checked-out .NET project without touching game
# asset directories that happen to use "bin" in their names.
foreach ($project in Get-ChildItem -LiteralPath $repoRoot -Filter '*.csproj' `
         -File -Recurse -Force -ErrorAction SilentlyContinue) {
    foreach ($outputName in @('bin', 'obj')) {
        Remove-RepoPath -LiteralPath (Join-Path $project.DirectoryName $outputName)
    }
}

foreach ($transientDirectory in @(
    (Join-Path $repoRoot 'reference\traces'),
    (Join-Path $repoRoot 'reference-v8-2\traces'),
    (Join-Path $repoRoot 'tools\smoke\captures'),
    (Join-Path $repoRoot 'PS1 game\Screenshots'),
    (Join-Path $repoRoot 'PS1 game\V8_2'),
    (Join-Path $repoRoot 'V8_2_LOOSE\artifacts')
)) {
    Remove-RepoPath -LiteralPath $transientDirectory
}

foreach ($cacheDirectory in Get-ChildItem -LiteralPath $repoRoot `
         -Directory -Filter '__pycache__' -Recurse -Force `
         -ErrorAction SilentlyContinue) {
    Remove-RepoPath -LiteralPath $cacheDirectory.FullName
}

# Once the portable N64 reference emulator has been extracted, retaining its
# download archive only duplicates the same regenerable oracle in the repo.
Remove-RepoPath -LiteralPath (Join-Path $repoRoot `
    'tools\n64_reference\vendor\RMG-Portable-Windows64-v0.9.0.zip')

foreach ($pattern in @('*.obj', '*.o', '*.pdb')) {
    foreach ($file in Get-ChildItem -LiteralPath $repoRoot -File -Filter $pattern `
             -Force -ErrorAction SilentlyContinue) {
        Remove-RepoPath -LiteralPath $file.FullName
    }
    $verifyDirectory = Join-Path $repoRoot 'tools\verify'
    if (Test-Path -LiteralPath $verifyDirectory) {
        foreach ($file in Get-ChildItem -LiteralPath $verifyDirectory -File `
                 -Filter $pattern -Force -ErrorAction SilentlyContinue) {
            Remove-RepoPath -LiteralPath $file.FullName
        }
    }
}

foreach ($fileName in @('run_compare.exe')) {
    Remove-RepoPath -LiteralPath (Join-Path $repoRoot "tools\verify\$fileName")
}

# Runtime logs are never handoff inputs. The deployed executables and assets
# remain untouched.
foreach ($runtimeDirectory in @(
    (Join-Path $repoRoot 'PS1 game'),
    (Join-Path $repoRoot 'V8_2_LOOSE')
)) {
    if (Test-Path -LiteralPath $runtimeDirectory) {
        foreach ($log in Get-ChildItem -LiteralPath $runtimeDirectory -File `
                 -Filter '*.log' -Recurse -Force -ErrorAction SilentlyContinue) {
            Remove-RepoPath -LiteralPath $log.FullName
        }
    }
}

foreach ($log in Get-ChildItem -LiteralPath $repoRoot -File -Filter '*.log' `
         -Force -ErrorAction SilentlyContinue | Where-Object {
             $_.Name -notin @('progress.log', 'decisions.log')
         }) {
    Remove-RepoPath -LiteralPath $log.FullName
}

# Remove old comparison audio/logs and the framework-dependent sequel build
# that was previously deployed into the original game's handoff directory.
# The current single-file V8 executable and all loose game assets are retained.
foreach ($stalePath in @(
    'reference\audio_match3.wav',
    'reference\audio_match3_stderr.log',
    'reference\audio_match3_stdout.log',
    'reference\cdda_pause_stderr.log',
    'reference\cdda_pause_stdout.log',
    'reference\cdda_test3_stderr.log',
    'reference\cdda_test3_stdout.log',
    'reference\cdda_test4_stderr.log',
    'reference\cdda_test4_stdout.log',
    'reference\deployed_final_stderr.log',
    'reference\deployed_final_stdout.log',
    'PS1 game\Vigilante82PC.exe',
    'PS1 game\Vigilante82PC.dll',
    'PS1 game\Vigilante82PC.deps.json',
    'PS1 game\Vigilante82PC.runtimeconfig.json',
    'PS1 game\VIGILANTE_8_2_README.md',
    'PS1 game\Vigilante8PC_Dreamland_test.exe',
    'PS1 game\RecompOne.Runtime.dll',
    'PS1 game\SDL2.dll',
    'PS1 game\createdump.exe',
    'PS1 game\nfd.lib',
    'PS1 game\nfd.exp'
)) {
    Remove-RepoPath -LiteralPath (Join-Path $repoRoot $stalePath)
}

# Satellite assemblies left by old framework-dependent publishes are not game
# localization assets; the current handoff is a self-contained executable.
foreach ($locale in @(
    'cs', 'de', 'es', 'fr', 'it', 'ja', 'ko',
    'pl', 'pt-BR', 'ru', 'tr', 'zh-Hans', 'zh-Hant'
)) {
    Remove-RepoPath -LiteralPath (Join-Path $repoRoot "PS1 game\$locale")
}

if (-not $SkipGitMaintenance -and
    $PSCmdlet.ShouldProcess('.git', 'Prune unreachable objects and repack Git data')) {
    & git -C $repoRoot gc --prune=now
    if ($LASTEXITCODE -ne 0) {
        throw "git gc failed with exit code $LASTEXITCODE"
    }
}

$removedGiB = [math]::Round($script:removedBytes / 1GB, 2)
$summary = "Cleanup complete: removed $($script:removedItems) targets ($removedGiB GiB)."
if ($script:skippedItems -gt 0) {
    $summary += " Skipped $($script:skippedItems) active/locked targets."
}
Write-Host $summary
