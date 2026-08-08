param(
    [Parameter(Mandatory = $true)]
    [string]$InputPath,
    [Parameter(Mandatory = $true)]
    [string]$OutputPath,
    [Parameter(Mandatory = $true)]
    [int]$X,
    [Parameter(Mandatory = $true)]
    [int]$Y,
    [Parameter(Mandatory = $true)]
    [int]$Width,
    [Parameter(Mandatory = $true)]
    [int]$Height
)

Add-Type -AssemblyName System.Drawing

$sourcePath = (Resolve-Path -LiteralPath $InputPath).Path
$destination = [System.IO.Path]::GetFullPath($OutputPath)
$source = [System.Drawing.Bitmap]::FromFile($sourcePath)
try {
    if ($X -lt 0 -or $Y -lt 0 -or
        $Width -le 0 -or $Height -le 0 -or
        $X + $Width -gt $source.Width -or
        $Y + $Height -gt $source.Height) {
        throw "Crop rectangle is outside the source bitmap."
    }

    $crop = New-Object System.Drawing.Bitmap $Width, $Height
    try {
        $graphics = [System.Drawing.Graphics]::FromImage($crop)
        try {
            $graphics.DrawImage(
                $source,
                [System.Drawing.Rectangle]::new(0, 0, $Width, $Height),
                [System.Drawing.Rectangle]::new($X, $Y, $Width, $Height),
                [System.Drawing.GraphicsUnit]::Pixel)
        }
        finally {
            $graphics.Dispose()
        }
        $directory = [System.IO.Path]::GetDirectoryName($destination)
        if ($directory) {
            [System.IO.Directory]::CreateDirectory($directory) | Out-Null
        }
        $crop.Save($destination, [System.Drawing.Imaging.ImageFormat]::Bmp)
    }
    finally {
        $crop.Dispose()
    }
}
finally {
    $source.Dispose()
}
