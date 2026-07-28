#!/usr/bin/env python3
"""Restore source-faithful edges after a conservative Real-ESRGAN pass."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageEnhance, ImageFilter


def preserve_source_edges(
    source: Image.Image,
    restored: Image.Image,
    scale: int,
    edge_threshold: int,
    edge_strength: float,
) -> Image.Image:
    source = source.convert("RGBA")
    restored = restored.convert("RGBA")
    expected_size = (source.width * scale, source.height * scale)
    if restored.size != expected_size:
        raise ValueError(
            f"restored size {restored.size} does not match {expected_size}"
        )
    if source.width == 1 and source.height == 1:
        return source.resize(expected_size, Image.Resampling.NEAREST)

    source_upscaled = source.resize(expected_size, Image.Resampling.LANCZOS)
    source_edges = source.convert("L").filter(ImageFilter.FIND_EDGES)
    source_edges = source_edges.point(
        lambda value: 255 if value >= edge_threshold else 0
    )
    source_edges = source_edges.filter(ImageFilter.MaxFilter(3))
    edge_mask = source_edges.resize(expected_size, Image.Resampling.BICUBIC)
    edge_mask = edge_mask.filter(ImageFilter.GaussianBlur(1.0))
    edge_mask = ImageEnhance.Brightness(edge_mask).enhance(edge_strength)

    result = Image.composite(source_upscaled, restored, edge_mask)
    # Alpha defines source-authored cards and silhouettes. Never ask the
    # restoration model to reinterpret it.
    result.putalpha(source.getchannel("A").resize(expected_size, Image.Resampling.LANCZOS))
    return result


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--esrgan", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--scale", type=int, default=2)
    parser.add_argument("--edge-threshold", type=int, default=35)
    parser.add_argument("--edge-strength", type=float, default=0.85)
    args = parser.parse_args()

    args.out.mkdir(parents=True, exist_ok=True)
    source_paths = sorted(args.source.glob("*.png"))
    if not source_paths:
        raise SystemExit(f"no source PNG files in {args.source}")
    for source_path in source_paths:
        esrgan_path = args.esrgan / source_path.name
        if not esrgan_path.is_file():
            raise SystemExit(f"missing ESRGAN output: {esrgan_path}")
        with (
            Image.open(source_path) as source,
            Image.open(esrgan_path) as restored,
        ):
            result = preserve_source_edges(
                source,
                restored,
                args.scale,
                args.edge_threshold,
                args.edge_strength,
            )
            result.save(args.out / source_path.name)
    print(
        f"I76_TEXTURE_FIDELITY_OK textures={len(source_paths)} "
        f"scale={args.scale} edge_threshold={args.edge_threshold} "
        f"edge_strength={args.edge_strength} output={args.out}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
