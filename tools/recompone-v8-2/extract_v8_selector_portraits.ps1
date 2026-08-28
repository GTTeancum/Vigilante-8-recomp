param(
    [string]$Source = "artifacts\v8-retail-extract-20260820\SHELL\CHARSEL1.TBL",
    [string]$Destination = "V8_2_LOOSE\SHELL",
    [string]$CaptureDirectory = "artifacts\v8_full_roster_headless"
)

$ErrorActionPreference = "Stop"
$sourcePath = (Resolve-Path -LiteralPath $Source).Path
$data = [IO.File]::ReadAllBytes($sourcePath)
if ($data.Length -lt 8) {
    throw "$sourcePath is too short to be a native table"
}

$count = [BitConverter]::ToUInt32($data, 0)
if ($count -lt 13 -or 4 + $count * 4 -gt $data.Length) {
    throw "$sourcePath has an invalid record table"
}

$offsets = New-Object uint32[] $count
for ($index = 0; $index -lt $count; $index++) {
    $offsets[$index] = [BitConverter]::ToUInt32(
        $data, 4 + $index * 4)
    if (($index -gt 0 -and $offsets[$index] -le $offsets[$index - 1]) -or
        $offsets[$index] -ge $data.Length) {
        throw "$sourcePath has invalid record offsets"
    }
}

$destinationPath = [IO.Path]::GetFullPath($Destination)
[IO.Directory]::CreateDirectory($destinationPath) | Out-Null

# V8's selector is a native two-layer composition. Records 16 and 17 are
# raw 8-bit VLC streams for the Coyotes and Vigilantes faction backings.
# Unlike the positioned character records, the original loader receives
# their 256x480 target rectangle from the selector screen object instead of
# from the table record. Prefix that authored rectangle so V8:2's equivalent
# native record loader can consume the exact same stream.
$backgrounds = @(
    @{ Name = "SELECTOR_BASE_COYOTES.VLC"; Index = 16 },
    @{ Name = "SELECTOR_BASE_VIGILANTES.VLC"; Index = 17 }
)
foreach ($background in $backgrounds) {
    $index = [int]$background.Index
    $start = [int]$offsets[$index]
    $end = if ($index + 1 -lt $count) {
        [int]$offsets[$index + 1]
    } else {
        $data.Length
    }
    $length = $end - $start
    if ($length -lt 12) {
        throw "background record $index is truncated"
    }
    $streamHeader = [BitConverter]::ToUInt32($data, $start)
    $streamMode = [BitConverter]::ToUInt32($data, $start + 4)
    if (($streamHeader -band 0xFF000000) -ne 0x38000000 -or
        $streamMode -ne 0x00020008) {
        throw "record $index is not a native V8 faction backing"
    }

    $record = New-Object byte[] ($length + 4)
    [BitConverter]::GetBytes([uint16]256).CopyTo($record, 0)
    [BitConverter]::GetBytes([uint16]480).CopyTo($record, 2)
    [Array]::Copy($data, $start, $record, 4, $length)
    $output = Join-Path $destinationPath $background.Name
    [IO.File]::WriteAllBytes($output, $record)
    Write-Output (
        "$output`: source=$index bytes=$($record.Length) " +
        "size=256x480 mode=0x00020008")
}

for ($outputIndex = 0; $outputIndex -lt 13; $outputIndex++) {
    $start = [int]$offsets[$outputIndex]
    $end = if ($outputIndex + 1 -lt $count) {
        [int]$offsets[$outputIndex + 1]
    } else {
        $data.Length
    }
    $length = $end - $start
    if ($length -lt 24) {
        throw "record $outputIndex is truncated"
    }

    $width = [BitConverter]::ToUInt16($data, $start)
    $height = [BitConverter]::ToUInt16($data, $start + 2)
    $streamHeader = [BitConverter]::ToUInt32($data, $start + 4)
    $streamMode = [BitConverter]::ToUInt32($data, $start + 8)
    if ($width -lt 1 -or $width -gt 512 -or
        $height -lt 1 -or $height -gt 512 -or
        ($streamHeader -band 0xFF000000) -ne 0x38000000 -or
        $streamMode -ne 0x00020004) {
        throw "record $outputIndex is not a native V8 VLC player banner"
    }

    $record = New-Object byte[] $length
    [Array]::Copy($data, $start, $record, 0, $length)
    $output = Join-Path $destinationPath (
        "SELECTOR_{0:D2}.VLC" -f $outputIndex)
    [IO.File]::WriteAllBytes($output, $record)
    Write-Output (
        "$output`: source=$outputIndex bytes=$length " +
        "size=${width}x${height}")
}

# The original selector composes a faction backing and a character VLC layer
# asynchronously. Capture that native composition after it has settled, then
# preserve the exact authored 260x422 left banner as one durable asset. The
# sequel loads these through its ordinary TIM/image path; no host overlay or
# second front-end renderer is involved.
$capturePath = (Resolve-Path -LiteralPath $CaptureDirectory).Path
for ($outputIndex = 0; $outputIndex -lt 13; $outputIndex++) {
    $poll = 5 + $outputIndex * 20
    $capture = Join-Path $capturePath (
        "recompone_capture_choose_player_{0:D4}.ppm" -f $poll)
    if (-not (Test-Path -LiteralPath $capture)) {
        throw "missing settled V8 selector capture: $capture"
    }

    $ppm = [IO.File]::ReadAllBytes($capture)
    $prefixLength = [Math]::Min(64, $ppm.Length)
    $prefix = [Text.Encoding]::ASCII.GetString(
        $ppm, 0, $prefixLength)
    $match = [regex]::Match(
        $prefix, "^P6\s+(\d+)\s+(\d+)\s+255\s")
    if (-not $match.Success) {
        throw "$capture is not a supported binary P6 image"
    }
    $width = [int]$match.Groups[1].Value
    $height = [int]$match.Groups[2].Value
    $pixelOffset = $match.Length
    if ($width -lt 260 -or $height -lt 422 -or
        $pixelOffset + $width * $height * 3 -ne $ppm.Length) {
        throw "$capture does not contain a complete 260x422 selector banner"
    }

    $header = [Text.Encoding]::ASCII.GetBytes("P6`n260 422`n255`n")
    $banner = New-Object byte[] ($header.Length + 260 * 422 * 3)
    [Array]::Copy($header, 0, $banner, 0, $header.Length)
    for ($row = 0; $row -lt 422; $row++) {
        [Array]::Copy(
            $ppm,
            $pixelOffset + $row * $width * 3,
            $banner,
            $header.Length + $row * 260 * 3,
            260 * 3)
    }

    $output = Join-Path $destinationPath (
        "SELECTOR_{0:D2}.PPM" -f $outputIndex)
    [IO.File]::WriteAllBytes($output, $banner)
    Write-Output (
        "$output`: capture=$([IO.Path]::GetFileName($capture)) " +
        "size=260x422 bytes=$($banner.Length)")
}

# A locked source roster wraps before Y. That failure previously produced a
# syntactically valid thirteen-file set whose late entries were duplicates of
# early characters. Treat duplicate completed banners as an extraction error
# so a locked or mis-timed reference capture can never be packaged silently.
$portraitHashes = @{}
for ($outputIndex = 0; $outputIndex -lt 13; $outputIndex++) {
    $portrait = Join-Path $destinationPath (
        "SELECTOR_{0:D2}.PPM" -f $outputIndex)
    $hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $portrait).Hash
    if ($portraitHashes.ContainsKey($hash)) {
        throw (
            "selector portrait $outputIndex duplicates portrait " +
            "$($portraitHashes[$hash]); the original V8 roster was not " +
            "fully unlocked or the captures did not settle")
    }
    $portraitHashes[$hash] = $outputIndex
}
Write-Output "validated 13 distinct original V8 selector banners"
