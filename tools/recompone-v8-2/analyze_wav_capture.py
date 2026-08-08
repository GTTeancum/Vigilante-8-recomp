#!/usr/bin/env python3
"""Validate a runtime-captured PCM WAV with no third-party dependencies."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
import wave
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("wav", type=Path)
    parser.add_argument("--json", type=Path)
    parser.add_argument("--minimum-seconds", type=float, default=1.0)
    parser.add_argument("--minimum-peak", type=int, default=256)
    args = parser.parse_args()
    with wave.open(str(args.wav), "rb") as stream:
        channels = stream.getnchannels()
        width = stream.getsampwidth()
        rate = stream.getframerate()
        frames = stream.getnframes()
        pcm = stream.readframes(frames)
    if channels != 2 or width != 2:
        raise ValueError(
            f"{args.wav}: expected stereo S16, got {channels}ch/{width * 8}bit"
        )
    samples = struct.unpack(f"<{len(pcm) // 2}h", pcm)
    peak = max(abs(sample) for sample in samples) if samples else 0
    rms = (
        math.sqrt(sum(sample * sample for sample in samples) / len(samples))
        if samples
        else 0.0
    )
    duration = frames / rate if rate else 0.0
    checks = {
        "duration": duration >= args.minimum_seconds,
        "audible_peak": peak >= args.minimum_peak,
        "non_silent_rms": rms >= 32.0,
        "stereo_pcm_s16": channels == 2 and width == 2,
    }
    report = {
        "passed": all(checks.values()),
        "checks": checks,
        "path": str(args.wav.resolve()),
        "sample_rate": rate,
        "frames": frames,
        "duration_seconds": round(duration, 6),
        "peak": peak,
        "rms": round(rms, 3),
        "pcm_sha256": hashlib.sha256(pcm).hexdigest().upper(),
    }
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(
            json.dumps(report, indent=2) + "\n", encoding="utf-8"
        )
    state = "PASS" if report["passed"] else "FAIL"
    print(
        f"[WavCapture] {state} rate={rate} frames={frames} "
        f"duration={duration:.3f}s peak={peak} rms={rms:.2f} "
        f"sha256={report['pcm_sha256']}"
    )
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
