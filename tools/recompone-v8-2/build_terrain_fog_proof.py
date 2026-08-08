#!/usr/bin/env python3
"""Compose before/after proof sheets for the V8:2 widescreen terrain and
distance-fog fixes, and report the objective terrain-hole metric.

Terrain presence is read from real presentation captures taken with
RECOMPONE_V82_DEBUG_TERRAIN_COVERAGE=1, which paints every TerrainRoute
primitive magenta.  A hole is a backdrop-coloured pixel strictly below that
column's terrain silhouette.  Packet-space probes are not used: terrain is
rendered from camera space, so packed coordinates are not where it lands.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

REPO = Path(__file__).resolve().parents[2]
ARENAS = {
    0: ("Route 66", "levels_route66"),
    1: ("Olympic", "levels_olympic"),
    2: ("Bayou", "levels_bayou"),
    5: ("Nuclear", "levels_nuclear"),
}
BACKDROP_TOLERANCE = 10


def capture(directory: Path, stem: str) -> Path:
    matches = sorted(directory.glob(f"*_{stem}.gameplay-presentation.ppm"))
    if not matches:
        raise SystemExit(f"no gameplay capture for {stem} in {directory}")
    return matches[0]


def terrain_silhouette(mask: np.ndarray) -> np.ndarray:
    magenta = (
        (mask[:, :, 0] > 200) & (mask[:, :, 1] < 80) & (mask[:, :, 2] > 200)
    )
    height, width = magenta.shape
    top = np.full(width, height)
    for x in range(width):
        rows = np.nonzero(magenta[:, x])[0]
        if len(rows):
            top[x] = rows.min()
    return top


def backdrop_color(mask_path: Path, mask: np.ndarray) -> np.ndarray:
    """The engine's own backdrop colour, taken from the tallest full-display
    fill the renderer logged for this run.  Falls back to the top rows of the
    capture when the log is unavailable."""
    best = None
    for log in mask_path.parent.glob("*.stderr.log"):
        for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
            if "[EnhancedFog] atmosphere candidate" not in line:
                continue
            rgb = line.split("rgb=(", 1)[1].split(")", 1)[0]
            span = line.rsplit("y=", 1)[1]
            low, high = (float(v) for v in span.split(".."))
            height = high - low
            if best is None or height > best[0]:
                best = (height, [int(v) for v in rgb.split(",")])
    if best is not None:
        return np.array(best[1])
    return np.median(mask[2:6, :, :].reshape(-1, 3), axis=0).astype(int)


def hole_metrics(mask_path: Path, reference: np.ndarray) -> dict:
    mask = np.asarray(Image.open(mask_path).convert("RGB")).astype(int)
    top = terrain_silhouette(mask)
    pale = (np.abs(mask - reference) <= BACKDROP_TOLERANCE).all(axis=2)
    height, width = pale.shape
    rows = np.arange(height)[:, None]
    holes = pale & (rows > top[None, :])
    edge = int(width * 0.08)
    return {
        "backdrop": reference.tolist(),
        "total": int(holes.sum()),
        "outer_left": int(holes[:, :edge].sum()),
        "outer_right": int(holes[:, -edge:].sum()),
        "interior": int(holes[:, edge:-edge].sum()),
    }


def horizon_step(image_path: Path, mask_path: Path) -> dict:
    """Largest colour step across the terrain/sky boundary, averaged over the
    middle half of the frame.  A correct atmospheric fade leaves a small step;
    the previous synthetic haze left a hard band."""
    image = np.asarray(Image.open(image_path).convert("RGB")).astype(int)
    mask = np.asarray(Image.open(mask_path).convert("RGB")).astype(int)
    top = terrain_silhouette(mask)
    height, width = mask.shape[:2]
    steps = []
    for x in range(width // 4, 3 * width // 4, 4):
        y = int(top[x])
        if y < 8 or y > height - 8:
            continue
        sky = image[y - 6:y - 2, x].mean(axis=0)
        ground = image[y + 2:y + 6, x].mean(axis=0)
        steps.append(np.abs(sky - ground).max())
    if not steps:
        return {"samples": 0}
    steps = np.array(steps)
    return {
        "samples": int(len(steps)),
        "mean": round(float(steps.mean()), 1),
        "p90": round(float(np.percentile(steps, 90)), 1),
    }


def label(image: Image.Image, text: str) -> Image.Image:
    draw = ImageDraw.Draw(image)
    draw.rectangle((0, 0, 8 * len(text) + 16, 26), fill=(0, 0, 0))
    draw.text((8, 8), text, fill=(255, 255, 255))
    return image


def sheet(before: Path, after: Path, title: str, out: Path,
          crop: tuple[int, int, int, int] | None = None) -> None:
    a = Image.open(before).convert("RGB")
    b = Image.open(after).convert("RGB")
    if crop:
        a, b = a.crop(crop), b.crop(crop)
    width = 1280
    scale = width / a.width
    size = (width, int(a.height * scale))
    a = label(a.resize(size, Image.LANCZOS), f"{title} - BEFORE")
    b = label(b.resize(size, Image.LANCZOS), f"{title} - AFTER")
    canvas = Image.new("RGB", (width, size[1] * 2 + 6), (24, 24, 24))
    canvas.paste(a, (0, 0))
    canvas.paste(b, (0, size[1] + 6))
    canvas.save(out)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--ab-root", type=Path,
                        default=REPO / "artifacts" / "terrain-fix" / "ab")
    parser.add_argument("--output", type=Path,
                        default=REPO / "artifacts" / "terrain-fix" / "proof")
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)

    report = {}
    for slot, (name, stem) in ARENAS.items():
        try:
            before = capture(args.ab_root / f"before-map{slot}", stem)
            after = capture(args.ab_root / f"after-map{slot}", stem)
            before_mask = capture(args.ab_root / f"before-mask{slot}", stem)
            after_mask = capture(args.ab_root / f"after-mask{slot}", stem)
        except SystemExit as error:
            print(f"skip {name}: {error}")
            continue

        sheet(before, after, name, args.output / f"{slot:02d}-{stem}-full.png")
        sheet(before, after, f"{name} left edge",
              args.output / f"{slot:02d}-{stem}-left.png",
              crop=(0, 300, 760, 620))
        sheet(before, after, f"{name} right edge",
              args.output / f"{slot:02d}-{stem}-right.png",
              crop=(1160, 300, 1920, 620))

        # The backdrop colour is a property of the arena, so take the engine's
        # own value once and score both states against it.
        reference = backdrop_color(
            after_mask, np.asarray(Image.open(after_mask).convert("RGB")))
        report[name] = {
            "backdrop": reference.tolist(),
            "holes_before": hole_metrics(before_mask, reference),
            "holes_after": hole_metrics(after_mask, reference),
            "horizon_step_before": horizon_step(before, before_mask),
            "horizon_step_after": horizon_step(after, after_mask),
        }

    path = args.output / "metrics.json"
    path.write_text(json.dumps(report, indent=2), encoding="utf-8")
    for name, data in report.items():
        hb, ha = data["holes_before"], data["holes_after"]
        sb, sa = data["horizon_step_before"], data["horizon_step_after"]
        # A dark backdrop is indistinguishable from the arena's own shadowed
        # materials, so the hole count stops meaning anything there.
        luminance = float(np.dot(data["backdrop"], [0.299, 0.587, 0.114]))
        reliable = luminance >= 90.0
        data["hole_metric_reliable"] = reliable
        holes = (
            f"holes {hb['total']:6d} -> {ha['total']:6d} "
            f"(outer left {hb['outer_left']:5d} -> {ha['outer_left']:5d}, "
            f"outer right {hb['outer_right']:5d} -> {ha['outer_right']:5d})"
            if reliable
            else f"holes n/a (backdrop luminance {luminance:.0f} is too dark "
                 f"to separate from arena materials)")
        print(f"{name:10s} {holes}   "
              f"horizon step {sb.get('mean')} -> {sa.get('mean')}")
    print(f"\nsheets and metrics in {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
