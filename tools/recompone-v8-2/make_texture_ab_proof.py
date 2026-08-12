#!/usr/bin/env python3
"""Create labeled, pixel-faithful A/B texture proof images and metrics."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageStat


def edge_energy(image: Image.Image) -> float:
    edges = image.convert("L").filter(ImageFilter.FIND_EDGES)
    if edges.width > 2 and edges.height > 2:
        edges = edges.crop((1, 1, edges.width - 1, edges.height - 1))
    return float(ImageStat.Stat(edges).mean[0])


def labeled_pair(
    before: Image.Image, after: Image.Image,
    left: str, right: str, scale: int = 1,
) -> Image.Image:
    if before.size != after.size:
        raise ValueError("A/B inputs have different dimensions")
    if scale != 1:
        before = before.resize(
            (before.width * scale, before.height * scale),
            Image.Resampling.NEAREST,
        )
        after = after.resize(
            (after.width * scale, after.height * scale),
            Image.Resampling.NEAREST,
        )
    header = 48
    result = Image.new(
        "RGB", (before.width * 2, before.height + header), "black"
    )
    result.paste(before, (0, header))
    result.paste(after, (before.width, header))
    draw = ImageDraw.Draw(result)
    draw.text((16, 15), left, fill="white")
    draw.text((before.width + 16, 15), right, fill="white")
    draw.line((before.width, 0, before.width, result.height), fill="white")
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--off", type=Path, required=True)
    parser.add_argument("--on", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--stem", default="route66")
    parser.add_argument(
        "--left-label", default="HIGH-RES TEXTURES OFF (retail source)",
    )
    parser.add_argument(
        "--right-label", default="HIGH-RES TEXTURES ON (4x terrain DDS)",
    )
    parser.add_argument(
        "--detail-left-label", default="ROAD DETAIL: OFF (2x display zoom)",
    )
    parser.add_argument(
        "--detail-right-label",
        default="ROAD DETAIL: ON / 4x DDS (2x display zoom)",
    )
    parser.add_argument(
        "--roi", default="260,300,650,700",
        help="left,top,right,bottom detail crop",
    )
    args = parser.parse_args()
    before = Image.open(args.off).convert("RGB")
    after = Image.open(args.on).convert("RGB")
    if before.size != after.size:
        raise ValueError("A/B inputs have different dimensions")
    roi = tuple(int(value) for value in args.roi.split(","))
    if len(roi) != 4:
        raise ValueError("ROI must contain four comma-separated integers")
    before_crop = before.crop(roi)
    after_crop = after.crop(roi)
    args.output.mkdir(parents=True, exist_ok=True)
    full_path = args.output / f"{args.stem}_high_res_off_vs_4x_on.png"
    detail_path = args.output / f"{args.stem}_detail_off_vs_4x_on.png"
    labeled_pair(
        before, after,
        args.left_label,
        args.right_label,
    ).save(full_path)
    labeled_pair(
        before_crop, after_crop,
        args.detail_left_label,
        args.detail_right_label,
        scale=2,
    ).save(detail_path)
    difference = ImageChops.difference(before_crop, after_crop)
    metrics = {
        "off": str(args.off.resolve()),
        "on": str(args.on.resolve()),
        "size": list(before.size),
        "roi": list(roi),
        "roiMeanAbsoluteDifference": sum(
            ImageStat.Stat(difference).mean
        ) / 3.0,
        "offRoiEdgeEnergy": edge_energy(before_crop),
        "onRoiEdgeEnergy": edge_energy(after_crop),
        "fullProof": str(full_path.resolve()),
        "detailProof": str(detail_path.resolve()),
    }
    (args.output / "metrics.json").write_text(
        json.dumps(metrics, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps(metrics, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
