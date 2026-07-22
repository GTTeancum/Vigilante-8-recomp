#!/usr/bin/env python3
"""Validate a RecompOne reference PCM WAV capture."""

from __future__ import annotations

import argparse
import array
import math
import sys
import wave
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("wav", type=Path)
    parser.add_argument("--min-seconds", type=float, default=1.0)
    parser.add_argument("--min-rms-dbfs", type=float, default=-60.0)
    args = parser.parse_args()

    with wave.open(str(args.wav), "rb") as source:
        channels = source.getnchannels()
        rate = source.getframerate()
        width = source.getsampwidth()
        frames = source.getnframes()
        payload = source.readframes(frames)

    if channels != 2 or rate != 44100 or width != 2:
        print(
            f"FAIL format channels={channels} rate={rate} width={width}",
            file=sys.stderr,
        )
        return 1

    samples = array.array("h")
    samples.frombytes(payload)
    if sys.byteorder != "little":
        samples.byteswap()

    seconds = frames / rate
    peak = max((abs(sample) for sample in samples), default=0)
    square_sum = sum(float(sample) * sample for sample in samples)
    rms = math.sqrt(square_sum / len(samples)) if samples else 0.0
    rms_dbfs = 20.0 * math.log10(rms / 32768.0) if rms else float("-inf")
    clipped = sum(1 for sample in samples if abs(sample) >= 32767)

    print(
        f"seconds={seconds:.3f} frames={frames} peak={peak} "
        f"rms_dbfs={rms_dbfs:.2f} clipped={clipped}"
    )
    if seconds < args.min_seconds:
        print(f"FAIL duration below {args.min_seconds:.3f}s", file=sys.stderr)
        return 1
    if rms_dbfs < args.min_rms_dbfs:
        print(f"FAIL RMS below {args.min_rms_dbfs:.2f} dBFS", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
