#!/usr/bin/env python3
"""
check_screenshot.py -- validate that a PPM screenshot is non-uniform.

Usage:   check_screenshot.py path.ppm

Returns 0 if the image has >= 2 distinct color buckets above 1% of total
pixels. Returns 1 (fail) if the image is solid color / black.

This is the Phase 3 smoke predicate: the renderer drew SOMETHING.
"""
import os
import sys


def check(path: str) -> int:
    if not os.path.exists(path):
        print(f"FAIL: {path} missing")
        return 1

    with open(path, "rb") as f:
        data = f.read()

    # Parse PPM P6 header: "P6\n<width> <height>\n255\n<binary RGB>"
    if not data.startswith(b"P6"):
        print(f"FAIL: {path} not P6 PPM")
        return 1

    # Skip header (3 whitespace-separated tokens after magic).
    i = 2
    tokens = []
    while len(tokens) < 3:
        # skip whitespace
        while i < len(data) and data[i:i+1] in (b" ", b"\n", b"\t", b"\r"):
            i += 1
        # read non-whitespace
        start = i
        while i < len(data) and data[i:i+1] not in (b" ", b"\n", b"\t", b"\r"):
            i += 1
        tokens.append(data[start:i].decode("ascii"))
    i += 1  # skip the single whitespace after maxval
    width   = int(tokens[0])
    height  = int(tokens[1])
    maxval  = int(tokens[2])
    if maxval != 255:
        print(f"FAIL: unsupported maxval {maxval}")
        return 1

    pixels = data[i:]
    n      = width * height
    expected = n * 3
    if len(pixels) != expected:
        print(f"FAIL: pixel data size {len(pixels)} vs expected {expected}")
        return 1

    # Bucket by quantized color (5 bits per channel -> 32k buckets).
    buckets = {}
    for j in range(0, len(pixels), 3):
        r = pixels[j]   >> 3
        g = pixels[j+1] >> 3
        b = pixels[j+2] >> 3
        key = (r << 10) | (g << 5) | b
        buckets[key] = buckets.get(key, 0) + 1

    # Count significant buckets (>= 1% of pixels).
    threshold = max(1, n // 100)
    significant = sum(1 for c in buckets.values() if c >= threshold)
    total_distinct = len(buckets)

    print(f"  {path}: {width}x{height}, "
          f"{total_distinct} distinct colors, "
          f"{significant} buckets above {threshold} px (1%)")

    if significant < 2:
        # Solid color or near-solid.
        top = sorted(buckets.values(), reverse=True)[:3]
        print(f"FAIL: image is solid/uniform color (top 3 bucket sizes: {top})")
        return 1

    print(f"OK: image has >= 2 significant color regions")
    return 0


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: check_screenshot.py path.ppm")
        sys.exit(2)
    sys.exit(check(sys.argv[1]))
