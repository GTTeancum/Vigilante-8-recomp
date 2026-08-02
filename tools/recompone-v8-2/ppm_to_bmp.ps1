param(
    [Parameter(Mandatory = $true)][string]$InputPath,
    [Parameter(Mandatory = $true)][string]$OutputPath
)

$bytes = [IO.File]::ReadAllBytes((Resolve-Path -LiteralPath $InputPath))
$offset = 0
function Read-Token {
    while ($script:offset -lt $bytes.Length -and
           [char]::IsWhiteSpace([char]$bytes[$script:offset])) {
        $script:offset++
    }
    $start = $script:offset
    while ($script:offset -lt $bytes.Length -and
           -not [char]::IsWhiteSpace([char]$bytes[$script:offset])) {
        $script:offset++
    }
    [Text.Encoding]::ASCII.GetString(
        $bytes, $start, $script:offset - $start)
}

if ((Read-Token) -ne 'P6') { throw 'Only binary P6 PPM files are supported.' }
$width = [int](Read-Token)
$height = [int](Read-Token)
if ([int](Read-Token) -ne 255) { throw 'Only 8-bit PPM files are supported.' }
while ($offset -lt $bytes.Length -and
       [char]::IsWhiteSpace([char]$bytes[$offset])) {
    $offset++
}

$stride = ($width * 3 + 3) -band -4
$pixelBytes = $stride * $height
$bmp = [byte[]]::new(54 + $pixelBytes)
[Text.Encoding]::ASCII.GetBytes('BM').CopyTo($bmp, 0)
[BitConverter]::GetBytes($bmp.Length).CopyTo($bmp, 2)
[BitConverter]::GetBytes(54).CopyTo($bmp, 10)
[BitConverter]::GetBytes(40).CopyTo($bmp, 14)
[BitConverter]::GetBytes($width).CopyTo($bmp, 18)
[BitConverter]::GetBytes($height).CopyTo($bmp, 22)
[BitConverter]::GetBytes([int16]1).CopyTo($bmp, 26)
[BitConverter]::GetBytes([int16]24).CopyTo($bmp, 28)
[BitConverter]::GetBytes($pixelBytes).CopyTo($bmp, 34)

for ($y = 0; $y -lt $height; $y++) {
    $srcRow = $offset + $y * $width * 3
    $dstRow = 54 + ($height - 1 - $y) * $stride
    for ($x = 0; $x -lt $width; $x++) {
        $src = $srcRow + $x * 3
        $dst = $dstRow + $x * 3
        $bmp[$dst] = $bytes[$src + 2]
        $bmp[$dst + 1] = $bytes[$src + 1]
        $bmp[$dst + 2] = $bytes[$src]
    }
}

[IO.File]::WriteAllBytes(
    [IO.Path]::GetFullPath($OutputPath), $bmp)
