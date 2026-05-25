#!/usr/bin/env python3
"""
check_screenshot.py -- validate that a screenshot is non-uniform.

Usage:   check_screenshot.py path.ppm
         check_screenshot.py path.png

Returns 0 if the image has >= 2 distinct color buckets above 1% of total
pixels. Returns 1 (fail) if the image is solid color / black.

Supports P6 binary PPM and PNG (using stdlib zlib for PNG decode).
This is the Phase 3 smoke predicate: the renderer drew SOMETHING.
"""
import os
import sys
import struct
import zlib


def load_ppm(data: bytes):
    """Parse P6 binary PPM -> (width, height, pixels_bytes)."""
    if not data.startswith(b"P6"):
        raise ValueError("Not a P6 PPM")
    i = 2
    tokens = []
    while len(tokens) < 3:
        while i < len(data) and data[i:i+1] in (b" ", b"\n", b"\t", b"\r"):
            i += 1
        start = i
        while i < len(data) and data[i:i+1] not in (b" ", b"\n", b"\t", b"\r"):
            i += 1
        tokens.append(data[start:i].decode("ascii"))
    i += 1  # skip the single whitespace after maxval
    width   = int(tokens[0])
    height  = int(tokens[1])
    maxval  = int(tokens[2])
    if maxval != 255:
        raise ValueError(f"Unsupported maxval {maxval}")
    pixels = data[i:]
    expected = width * height * 3
    if len(pixels) != expected:
        raise ValueError(f"Pixel data size {len(pixels)} vs expected {expected}")
    return width, height, pixels


def load_png(data: bytes):
    """Parse PNG -> (width, height, rgb_bytes). Minimal stdlib-only decoder."""
    if data[:8] != b'\x89PNG\r\n\x1a\n':
        raise ValueError("Not a PNG")

    # Read IHDR
    def read_chunk(buf, pos):
        length = struct.unpack_from('>I', buf, pos)[0]
        ctype  = buf[pos+4:pos+8]
        cdata  = buf[pos+8:pos+8+length]
        pos   += 12 + length
        return ctype, cdata, pos

    pos = 8
    width = height = bit_depth = color_type = 0
    idat_chunks = []
    while pos < len(data):
        ctype, cdata, pos = read_chunk(data, pos)
        if ctype == b'IHDR':
            width      = struct.unpack_from('>I', cdata, 0)[0]
            height     = struct.unpack_from('>I', cdata, 4)[0]
            bit_depth  = cdata[8]
            color_type = cdata[9]
        elif ctype == b'IDAT':
            idat_chunks.append(cdata)
        elif ctype == b'IEND':
            break

    if bit_depth != 8:
        raise ValueError(f"Unsupported bit depth {bit_depth}")

    # Supported color types: 2=RGB, 6=RGBA
    if color_type not in (2, 6):
        raise ValueError(f"Unsupported color type {color_type}")

    channels = 3 if color_type == 2 else 4

    raw = zlib.decompress(b''.join(idat_chunks))
    stride = 1 + width * channels  # 1 filter byte + pixel bytes per row

    rgb_out = bytearray(width * height * 3)
    prev_row = bytearray(width * channels)

    for y in range(height):
        row_start = y * stride
        filt = raw[row_start]
        row  = bytearray(raw[row_start+1:row_start+stride])

        # Apply PNG filter
        if filt == 0:  # None
            pass
        elif filt == 1:  # Sub
            for i in range(channels, len(row)):
                row[i] = (row[i] + row[i - channels]) & 0xff
        elif filt == 2:  # Up
            for i in range(len(row)):
                row[i] = (row[i] + prev_row[i]) & 0xff
        elif filt == 3:  # Average
            for i in range(len(row)):
                a = row[i - channels] if i >= channels else 0
                b = prev_row[i]
                row[i] = (row[i] + (a + b) // 2) & 0xff
        elif filt == 4:  # Paeth
            def paeth(a, b, c):
                p  = a + b - c
                pa = abs(p - a); pb = abs(p - b); pc = abs(p - c)
                return a if pa <= pb and pa <= pc else (b if pb <= pc else c)
            for i in range(len(row)):
                a = row[i - channels] if i >= channels else 0
                b = prev_row[i]
                c = prev_row[i - channels] if i >= channels else 0
                row[i] = (row[i] + paeth(a, b, c)) & 0xff

        # Extract RGB
        for x in range(width):
            out_off = (y * width + x) * 3
            src_off = x * channels
            rgb_out[out_off]   = row[src_off]
            rgb_out[out_off+1] = row[src_off+1]
            rgb_out[out_off+2] = row[src_off+2]

        prev_row = row

    return width, height, bytes(rgb_out)


def check(path: str) -> int:
    if not os.path.exists(path):
        print(f"FAIL: {path} missing")
        return 1

    with open(path, "rb") as f:
        data = f.read()

    try:
        if data.startswith(b"P6"):
            width, height, pixels = load_ppm(data)
        elif data[:8] == b'\x89PNG\r\n\x1a\n':
            width, height, pixels = load_png(data)
        else:
            print(f"FAIL: unknown image format (first bytes: {data[:8].hex()})")
            return 1
    except Exception as e:
        print(f"FAIL: could not decode image: {e}")
        return 1

    n = width * height

    # Bucket by quantized color (5 bits per channel -> 32k buckets).
    buckets = {}
    for j in range(0, len(pixels), 3):
        r = pixels[j]   >> 3
        g = pixels[j+1] >> 3
        b = pixels[j+2] >> 3
        key = (r << 10) | (g << 5) | b
        buckets[key] = buckets.get(key, 0) + 1

    # Count significant buckets (>= 1% of pixels).
    threshold  = max(1, n // 100)
    significant = sum(1 for c in buckets.values() if c >= threshold)
    total_distinct = len(buckets)

    print(f"  {path}: {width}x{height}, "
          f"{total_distinct} distinct colors, "
          f"{significant} buckets above {threshold} px (1%)")

    if significant < 2:
        top = sorted(buckets.values(), reverse=True)[:3]
        print(f"FAIL: image is solid/uniform color (top 3 bucket sizes: {top})")
        return 1

    print(f"OK: image has >= 2 significant color regions")
    return 0


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: check_screenshot.py path.ppm|path.png")
        sys.exit(2)
    sys.exit(check(sys.argv[1]))
