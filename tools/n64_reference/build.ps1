[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath($PSScriptRoot)
$source = Join-Path $root 'src'
$bin = Join-Path $root 'bin'
$compiler = 'C:\msys64\clang64\bin\clang++.exe'

if (-not (Test-Path -LiteralPath $compiler)) {
    throw "clang++ is required at $compiler"
}

New-Item -ItemType Directory -Path $bin -Force | Out-Null
$common = @(
    '-std=c++20',
    '-O2',
    '-Wall',
    '-Wextra',
    '-Werror',
    '-static-libgcc',
    '-static-libstdc++',
    "-I$source"
)

& $compiler @common -shared `
    (Join-Path $source 'input_script_plugin.cpp') `
    '-o' (Join-Path $bin 'v8_n64_input.dll')
if ($LASTEXITCODE -ne 0) {
    throw "input plugin build failed: $LASTEXITCODE"
}

& $compiler @common -shared `
    (Join-Path $source 'gfx_trace_proxy.cpp') `
    '-o' (Join-Path $bin 'v8_n64_gfx_trace.dll')
if ($LASTEXITCODE -ne 0) {
    throw "GFX proxy build failed: $LASTEXITCODE"
}

& $compiler @common -shared `
    (Join-Path $source 'audio_trace_proxy.cpp') `
    '-o' (Join-Path $bin 'v8_n64_audio_trace.dll')
if ($LASTEXITCODE -ne 0) {
    throw "audio proxy build failed: $LASTEXITCODE"
}

& $compiler @common -shared `
    (Join-Path $source 'audio_null_plugin.cpp') `
    '-o' (Join-Path $bin 'v8_n64_audio_null.dll')
if ($LASTEXITCODE -ne 0) {
    throw "null audio plugin build failed: $LASTEXITCODE"
}

& $compiler @common -shared `
    (Join-Path $source 'rsp_trace_proxy.cpp') `
    '-o' (Join-Path $bin 'v8_n64_rsp_trace.dll')
if ($LASTEXITCODE -ne 0) {
    throw "RSP proxy build failed: $LASTEXITCODE"
}

& $compiler @common `
    (Join-Path $source 'headless_frontend.cpp') `
    '-o' (Join-Path $bin 'v8_n64_headless.exe') `
    '-luser32'
if ($LASTEXITCODE -ne 0) {
    throw "headless frontend build failed: $LASTEXITCODE"
}

$sourceFiles = @(
    (Join-Path $source 'm64p_min.h'),
    (Join-Path $source 'input_script_plugin.cpp'),
    (Join-Path $source 'gfx_trace_proxy.cpp'),
    (Join-Path $source 'audio_trace_proxy.cpp'),
    (Join-Path $source 'audio_null_plugin.cpp'),
    (Join-Path $source 'rsp_trace_proxy.cpp'),
    (Join-Path $source 'headless_frontend.cpp')
)
$binaryFiles = @(
    (Join-Path $bin 'v8_n64_input.dll'),
    (Join-Path $bin 'v8_n64_gfx_trace.dll'),
    (Join-Path $bin 'v8_n64_audio_trace.dll'),
    (Join-Path $bin 'v8_n64_audio_null.dll'),
    (Join-Path $bin 'v8_n64_rsp_trace.dll'),
    (Join-Path $bin 'v8_n64_headless.exe')
)
$manifest = [ordered]@{
    schema = 'v8.n64-reference-build.v1'
    generated_at = (Get-Date -Format o)
    compiler = (& $compiler --version | Select-Object -First 1)
    compiler_path = $compiler
    flags = $common
    sources = @(
        $sourceFiles | ForEach-Object {
            $item = Get-Item -LiteralPath $_
            [ordered]@{
                path = $item.FullName
                bytes = $item.Length
                sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $_).Hash
            }
        }
    )
    binaries = @(
        $binaryFiles | ForEach-Object {
            $item = Get-Item -LiteralPath $_
            [ordered]@{
                path = $item.FullName
                bytes = $item.Length
                sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $_).Hash
            }
        }
    )
}
$manifest | ConvertTo-Json -Depth 5 |
    Set-Content -LiteralPath (Join-Path $bin 'build_manifest.json') -Encoding UTF8

Get-ChildItem -LiteralPath $bin -File |
    Select-Object Name,Length,LastWriteTime
