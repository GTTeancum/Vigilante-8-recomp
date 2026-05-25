#!/usr/bin/env python3
"""
check_phase9.py -- verify both player (red-orange) and AI (blue) vehicles
are visible in a Phase 9 screenshot.

Usage: python check_phase9.py path.png

The player vehicle is rendered in red-orange (R>180, G<160, B<80).
The AI vehicle is rendered in blue (B>100, R<150, B>R+50).

Exits 0 on pass, 1 on fail.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from check_screenshot import load_ppm, load_png


def is_player_color(r, g, b):
    """Red-orange tint (same as check_centered.py)."""
    return r > 180 and g < 160 and b < 80


def is_ai_color(r, g, b):
    """Blue tint: AI box rendered with tint (0, 0, 5) on red-orange vertices.
    vertex B=0.2 * tint B=5.0 = 1.0 -> saturated blue B=255.
    R and G channels are zeroed by 0-tint.
    After MSAA blending: B > 120, R < 80, G < 80.
    """
    return b > 120 and r < 80 and g < 80


def check(path: str) -> int:
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
            print("FAIL: unknown image format")
            return 1
    except Exception as e:
        print(f"FAIL: {e}")
        return 1

    player_pixels = 0
    ai_pixels = 0

    for y in range(h):
        for x in range(w):
            off = (y * w + x) * 3
            r, g, b = pixels[off], pixels[off+1], pixels[off+2]
            if is_player_color(r, g, b):
                player_pixels += 1
            elif is_ai_color(r, g, b):
                ai_pixels += 1

    total = w * h
    print(f"Image: {w}x{h} ({total} pixels)")
    print(f"Player (red-orange) pixels: {player_pixels} ({100*player_pixels/total:.2f}%)")
    print(f"AI (blue) pixels: {ai_pixels} ({100*ai_pixels/total:.2f}%)")

    ok = True
    if player_pixels < 10:
        print("FAIL: no player vehicle pixels found")
        ok = False
    if ai_pixels < 10:
        print("FAIL: no AI vehicle pixels found (blue box not rendered or off-screen)")
        ok = False

    if ok:
        print("PASS: both player and AI vehicles visible in screenshot")
    return 0 if ok else 1


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} image.png")
        sys.exit(2)
    sys.exit(check(sys.argv[1]))
