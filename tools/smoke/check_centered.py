#!/usr/bin/env python3
"""
check_centered.py -- verify the vehicle box is within the central
60% of the screen in a Phase 4 screenshot.

Usage: python check_centered.py path.png [--margin 0.20]

The vehicle is rendered as a reddish-orange box (R > 180, G < 160, B < 80
for the darker faces; R > 230 for the brighter faces). The check passes
if at least 1 vehicle-colored pixel is found in the central band of the
image (within --margin from center in both axes).

Exits 0 on pass, 1 on fail.
"""
import os
import sys

# Reuse the image loading from check_screenshot.py
sys.path.insert(0, os.path.dirname(__file__))
from check_screenshot import load_ppm, load_png


def is_vehicle_color(r, g, b):
    """Detect the vehicle's red-orange colormap from renderer.c.

    Vertex colors:
      Face base:  (1.0, 0.2, 0.2) -> (255, 51, 51)
      Face roof:  (1.0, 0.6, 0.2) -> (255, 153, 51)
    After OpenGL blending and MSAA we get a range.
    """
    return r > 180 and g < 160 and b < 80


def check(path: str, margin: float = 0.20) -> int:
    if not os.path.exists(path):
        print(f"FAIL: {path} missing")
        return 1

    with open(path, "rb") as f:
        data = f.read()

    try:
        if data.startswith(b"P6"):
            w, h, pixels = load_ppm(data)
        elif data[:8] == b'\x89PNG\r\n\x1a\n':
            w, h, pixels = load_png(data)
        else:
            print(f"FAIL: unknown image format")
            return 1
    except Exception as e:
        print(f"FAIL: {e}")
        return 1

    x_lo = int(w * margin)
    x_hi = int(w * (1.0 - margin))
    y_lo = int(h * margin)
    y_hi = int(h * (1.0 - margin))

    vehicle_in_center = 0
    vehicle_total     = 0

    for y in range(h):
        for x in range(w):
            off = (y * w + x) * 3
            r, g, b = pixels[off], pixels[off+1], pixels[off+2]
            if is_vehicle_color(r, g, b):
                vehicle_total += 1
                if x_lo <= x <= x_hi and y_lo <= y <= y_hi:
                    vehicle_in_center += 1

    print(f"Image: {w}x{h}, center band [{x_lo}..{x_hi}]x[{y_lo}..{y_hi}]")
    print(f"Vehicle pixels total: {vehicle_total}, in center: {vehicle_in_center}")

    if vehicle_total == 0:
        print("FAIL: no vehicle-colored pixels found in screenshot")
        return 1
    frac = vehicle_in_center / vehicle_total
    print(f"Vehicle in center: {100*frac:.1f}%")
    if frac < 0.60:
        print(f"FAIL: only {100*frac:.1f}% of vehicle pixels are in the central band")
        return 1

    print("PASS: vehicle is within central 60% of screen")
    return 0


if __name__ == "__main__":
    import argparse
    p = argparse.ArgumentParser()
    p.add_argument("image")
    p.add_argument("--margin", type=float, default=0.20)
    args = p.parse_args()
    sys.exit(check(args.image, args.margin))
