#!/usr/bin/env python3
"""check_audio.py -- assert a WAV is non-silent (RMS above threshold)."""
import os
import struct
import sys
import math


def check(path: str) -> int:
    if not os.path.exists(path):
        print(f"FAIL: {path} missing")
        return 1
    with open(path, "rb") as f:
        data = f.read()

    # RIFF header parse (we wrote mono s16 @22050, header is 44 bytes).
    if not data.startswith(b"RIFF") or b"WAVE" not in data[:12]:
        print("FAIL: not a RIFF/WAVE file")
        return 1
    # Skip header.
    pcm = data[44:]
    n = len(pcm) // 2
    if n == 0:
        print("FAIL: empty PCM data")
        return 1

    # RMS in normalized [-1, 1] units.
    accum = 0.0
    for i in range(0, len(pcm), 2):
        s = struct.unpack_from("<h", pcm, i)[0] / 32768.0
        accum += s * s
    rms = math.sqrt(accum / n)
    db  = 20.0 * math.log10(rms + 1e-9)
    print(f"  {path}: {n} samples, RMS={rms:.6f} ({db:.1f} dBFS)")

    if db < -60.0:
        print("FAIL: audio is effectively silent")
        return 1

    print("OK: audio above -60 dBFS")
    return 0


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: check_audio.py path.wav")
        sys.exit(2)
    sys.exit(check(sys.argv[1]))
