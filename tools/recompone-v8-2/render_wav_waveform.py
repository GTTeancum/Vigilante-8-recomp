#!/usr/bin/env python3
"""Render a captured PCM WAV as a compact, dependency-free BMP proof."""

from __future__ import annotations

import argparse
import struct
import wave
from pathlib import Path


def write_bmp(path: Path, width: int, height: int, rgb: bytearray) -> None:
    stride = (width * 3 + 3) & ~3
    pixels = bytearray(stride * height)
    for y in range(height):
        source = y * width * 3
        target = (height - 1 - y) * stride
        for x in range(width):
            src = source + x * 3
            dst = target + x * 3
            pixels[dst : dst + 3] = bytes(
                (rgb[src + 2], rgb[src + 1], rgb[src])
            )
    header = bytearray(54)
    header[:2] = b"BM"
    struct.pack_into("<I", header, 2, 54 + len(pixels))
    struct.pack_into("<I", header, 10, 54)
    struct.pack_into("<I", header, 14, 40)
    struct.pack_into("<i", header, 18, width)
    struct.pack_into("<i", header, 22, height)
    struct.pack_into("<H", header, 26, 1)
    struct.pack_into("<H", header, 28, 24)
    struct.pack_into("<I", header, 34, len(pixels))
    path.write_bytes(header + pixels)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--width", type=int, default=1600)
    parser.add_argument("--height", type=int, default=520)
    parser.add_argument("--start", type=float, default=0.0)
    parser.add_argument("--duration", type=float)
    args = parser.parse_args()

    with wave.open(str(args.input), "rb") as source:
        channels = source.getnchannels()
        sample_width = source.getsampwidth()
        rate = source.getframerate()
        total_frames = source.getnframes()
        if sample_width != 2:
            raise ValueError("only 16-bit PCM WAV input is supported")
        start_frame = min(
            total_frames,
            max(0, round(args.start * rate)),
        )
        end_frame = total_frames
        if args.duration is not None:
            if args.duration <= 0:
                raise ValueError("--duration must be positive")
            end_frame = min(
                total_frames,
                start_frame + round(args.duration * rate),
            )
        frames = end_frame - start_frame
        if frames <= 0:
            raise ValueError("the requested waveform interval is empty")
        source.setpos(start_frame)
        samples = struct.unpack(
            f"<{frames * channels}h", source.readframes(frames)
        )

    width = max(320, args.width)
    height = max(180, args.height)
    rgb = bytearray((12, 17, 21) * (width * height))

    def pixel(x: int, y: int, color: tuple[int, int, int]) -> None:
        if 0 <= x < width and 0 <= y < height:
            offset = (y * width + x) * 3
            rgb[offset : offset + 3] = bytes(color)

    center = height // 2
    for x in range(width):
        pixel(x, center, (58, 80, 86))

    peak = 0
    for x in range(width):
        start = x * frames // width
        end = max(start + 1, (x + 1) * frames // width)
        maximum = 0
        for frame in range(start, min(end, frames)):
            value = max(
                abs(samples[frame * channels + channel])
                for channel in range(channels)
            )
            maximum = max(maximum, value)
        peak = max(peak, maximum)
        amplitude = round(maximum / 32768 * (height * 0.43))
        for y in range(center - amplitude, center + amplitude + 1):
            pixel(x, y, (52, 226, 183))

    # A small machine-readable footer is encoded as bars: duration, sample
    # rate, channel count, and peak are also printed by the tool.
    for x in range(20, width - 20):
        pixel(x, height - 34, (38, 57, 62))
    duration = frames / rate if rate else 0.0
    write_bmp(args.output, width, height, rgb)
    print(
        f"{args.output}: start={start_frame / rate:.3f}s "
        f"duration={duration:.3f}s rate={rate}Hz "
        f"channels={channels} peak={peak}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
