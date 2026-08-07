#!/usr/bin/env python3
"""Build a before/after sheet for the backdrop fix from paired captures.

Runs are deterministic once the game's saved state is restored beforehand
(0.99% pixel difference between identical runs, against 64.68% without the
reset), so the same dump index is the same moment in both arms and the images
are directly comparable.

Picks the frames where the metric says the panorama fell furthest short in the
"before" arm, and puts the two images side by side with the affected strip
marked.
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

from PIL import Image, ImageDraw

sys.path.insert(0, str(Path(__file__).resolve().parent))
from measure_backdrop_edge import backdrop_edges, FRAME_LEFT

PAIR = re.compile(r"V82GeometryDump\] frame=(\d+) index=(\d+)")
FRAME_SPAN = 428.0


def pairing(run: Path):
    out = {}
    for log in run.glob("*.stdout.log"):
        for frame, index in PAIR.findall(log.read_text(errors="replace")):
            out[int(frame)] = index
    return out


def shot(run: Path, index: str):
    hits = sorted(run.glob(f"*gameplay_{index}_*.ppm"))
    return hits[0] if hits else None


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--before", type=Path, required=True)
    ap.add_argument("--after", type=Path, required=True)
    ap.add_argument("--output", type=Path, required=True)
    ap.add_argument("--rows", type=int, default=3)
    args = ap.parse_args()

    before_pairs, after_pairs = pairing(args.before), pairing(args.after)
    scored = []
    for dump in args.before.glob("recompone_geometry_frame*.txt"):
        digits = "".join(ch for ch in dump.stem if ch.isdigit())
        if not digits:
            continue
        left, _ = backdrop_edges(dump, 3000.0, 3600.0, 2000.0)
        if left is not None and left > FRAME_LEFT:
            scored.append((left, int(digits)))
    if not scored:
        print("no frames in the before arm where the panorama fell short")
        return 1
    scored.sort(reverse=True)

    rows = []
    for left, frame in scored:
        bi, ai = before_pairs.get(frame), after_pairs.get(frame)
        if not bi or not ai:
            continue
        b, a = shot(args.before, bi), shot(args.after, ai)
        if not (b and a):
            continue
        # Runs only stay in step for a few hundred frames. Verify the two
        # images really are the same moment before showing them as a pair:
        # outside the strip the fix touches they should be near identical.
        import numpy as np
        ia = np.asarray(Image.open(b).convert("RGB")).astype(int)
        ib = np.asarray(Image.open(a).convert("RGB")).astype(int)
        if ia.shape != ib.shape:
            continue
        # The paired-capture request can race the presentation readback and
        # return VRAM garbage (~3% of captures). Such a frame is unusable as
        # evidence, so drop it rather than show it.
        def corrupt(img):
            d = np.abs(np.diff(img, axis=1)).mean(axis=2)
            return (d > 60).mean() > 0.05
        if corrupt(ia) or corrupt(ib):
            print(f"  frame {frame}: capture corrupted by readback race "
                  f"- skipped")
            continue
        keep = int(ia.shape[1] * 0.35)
        diff = (np.abs(ia[:, keep:] - ib[:, keep:]).sum(axis=2) > 40).mean()
        if diff > 0.10:
            print(f"  frame {frame}: runs diverged here ({100*diff:.0f}% "
                  f"differ outside the fixed strip) - skipped")
            continue
        rows.append((left, frame, b, a))
        if len(rows) >= args.rows:
            break
    if not rows:
        print("no paired captures for the worst frames")
        return 1

    w, h = 620, 349
    sheet = Image.new("RGB", (w * 2 + 30, h * len(rows) + 30 * len(rows)),
                      (18, 18, 20))
    d = ImageDraw.Draw(sheet)
    for i, (left, frame, b, a) in enumerate(rows):
        y = i * (h + 30) + 26
        for j, path in enumerate((b, a)):
            im = Image.open(path).convert("RGB").resize((w, h))
            x = j * (w + 20)
            sheet.paste(im, (x, y))
            # mark the strip the panorama failed to reach
            edge = (left + 54.0) / FRAME_SPAN * w
            d.rectangle([x, y, x + max(edge, 2), y + h],
                        outline=(255, 60, 60), width=3)
        d.text((0, y - 20),
               f"frame {frame}: panorama stopped {left - FRAME_LEFT:.0f}px "
               f"inside the frame   |   left: before      right: after",
               fill=(235, 235, 235))
    sheet.save(args.output)
    print(f"wrote {args.output} ({len(rows)} frames)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
