#!/usr/bin/env python3
"""Programmatic acceptance checks for V8:2 Enhanced-renderer captures.

The checks intentionally operate on the native P6 output written by the host so
they do not depend on Pillow, ImageMagick, or a desktop session.  Selector
regions are derived from the centered 4:3 game canvas rather than the host
window, which keeps the checks valid at every 16:9 presentation resolution.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import re
from dataclasses import asdict, dataclass
from pathlib import Path


@dataclass
class CaptureResult:
    file: str
    sha256: str
    width: int
    height: int
    portrait_black_fraction: float
    portrait_luma_deviation: float
    preview_background_fraction: float
    preview_foreground_fraction: float
    wheel_box_contrast_fraction: float
    neutral_gray_column_fraction: float
    passed: bool
    failures: list[str]


def read_ppm(path: Path) -> tuple[int, int, bytes]:
    with path.open("rb") as stream:
        if stream.readline().strip() != b"P6":
            raise ValueError(f"{path}: expected binary P6 PPM")
        dimensions = stream.readline()
        while dimensions.startswith(b"#"):
            dimensions = stream.readline()
        parts = dimensions.split()
        if len(parts) != 2:
            raise ValueError(f"{path}: invalid dimensions")
        width, height = map(int, parts)
        if stream.readline().strip() != b"255":
            raise ValueError(f"{path}: expected 8-bit RGB")
        pixels = stream.read()
    expected = width * height * 3
    if len(pixels) != expected:
        raise ValueError(
            f"{path}: truncated payload ({len(pixels)} != {expected})"
        )
    return width, height, pixels


def centered_game_canvas(width: int, height: int) -> tuple[int, int, int, int]:
    target_aspect = 4.0 / 3.0
    if width / height >= target_aspect:
        canvas_height = height
        canvas_width = round(height * target_aspect)
        return (width - canvas_width) // 2, 0, canvas_width, canvas_height
    canvas_width = width
    canvas_height = round(width / target_aspect)
    return 0, (height - canvas_height) // 2, canvas_width, canvas_height


def sample_region(
    pixels: bytes,
    width: int,
    canvas: tuple[int, int, int, int],
    normalized: tuple[float, float, float, float],
) -> list[tuple[int, int, int]]:
    cx, cy, cw, ch = canvas
    nx0, ny0, nx1, ny1 = normalized
    x0 = max(0, min(width, cx + round(nx0 * cw)))
    x1 = max(x0, min(width, cx + round(nx1 * cw)))
    y0 = max(0, cy + round(ny0 * ch))
    y1 = max(y0, cy + round(ny1 * ch))
    result = []
    for y in range(y0, y1):
        start = (y * width + x0) * 3
        end = (y * width + x1) * 3
        row = pixels[start:end]
        result.extend(zip(row[0::3], row[1::3], row[2::3]))
    return result


def luma(rgb: tuple[int, int, int]) -> float:
    r, g, b = rgb
    return 0.2126 * r + 0.7152 * g + 0.0722 * b


def median_color(samples: list[tuple[int, int, int]]) -> tuple[int, int, int]:
    channels = sorted(s[0] for s in samples), sorted(
        s[1] for s in samples
    ), sorted(s[2] for s in samples)
    middle = len(samples) // 2
    return channels[0][middle], channels[1][middle], channels[2][middle]


def color_distance(
    left: tuple[int, int, int], right: tuple[int, int, int]
) -> float:
    return math.sqrt(sum((a - b) ** 2 for a, b in zip(left, right)))


def analyze_capture(path: Path) -> CaptureResult:
    width, height, pixels = read_ppm(path)
    canvas = centered_game_canvas(width, height)

    # Retail selector layout: portrait/banner left, 3-D preview upper-right.
    portrait = sample_region(
        pixels, width, canvas, (0.015, 0.04, 0.335, 0.94)
    )
    preview = sample_region(
        pixels, width, canvas, (0.535, 0.245, 0.925, 0.535)
    )
    preview_border_free = sample_region(
        pixels, width, canvas, (0.57, 0.30, 0.90, 0.52)
    )
    wheel_zone = sample_region(
        pixels, width, canvas, (0.56, 0.39, 0.91, 0.535)
    )
    outside_preview = sample_region(
        pixels, width, canvas, (0.34, 0.05, 0.525, 0.92)
    )

    portrait_black = sum(max(rgb) < 12 for rgb in portrait) / len(portrait)
    portrait_lumas = [luma(rgb) for rgb in portrait]
    portrait_mean = sum(portrait_lumas) / len(portrait_lumas)
    portrait_deviation = (
        sum(abs(value - portrait_mean) for value in portrait_lumas)
        / len(portrait_lumas)
    )

    background = median_color(preview)
    preview_background = sum(
        color_distance(rgb, background) <= 18.0 for rgb in preview_border_free
    ) / len(preview_border_free)
    preview_foreground = 1.0 - preview_background

    # The historical wheel regression is a wrongly decoded monochrome glyph
    # page: large wheel quads alternate nearly black and nearly white.  Normal
    # tyres/rims contain gradients, so count only strongly monochrome extremes.
    monochrome_extremes = 0
    for r, g, b in wheel_zone:
        chroma = max(r, g, b) - min(r, g, b)
        value = max(r, g, b)
        if chroma < 14 and (value < 18 or value > 238):
            monochrome_extremes += 1
    wheel_box_fraction = monochrome_extremes / len(wheel_zone)

    # The broken UI-background regression creates large exact neutral-gray
    # slabs outside the authored preview.  Natural portrait/UI pixels rarely
    # have all three channels within two values over a sustained area.
    neutral_gray = sum(
        max(rgb) - min(rgb) <= 2 and 70 <= sum(rgb) / 3 <= 190
        for rgb in outside_preview
    ) / len(outside_preview)

    failures: list[str] = []
    if portrait_black > 0.62:
        failures.append(
            f"portrait/banner mostly absent ({portrait_black:.3f} black)"
        )
    if portrait_deviation < 22.0:
        failures.append(
            f"portrait/banner lacks image detail ({portrait_deviation:.2f})"
        )
    if preview_foreground < 0.075:
        failures.append(
            f"vehicle preview absent ({preview_foreground:.3f} foreground)"
        )
    if wheel_box_fraction > 0.145:
        failures.append(
            f"monochrome wheel-box signature ({wheel_box_fraction:.3f})"
        )
    if neutral_gray > 0.12:
        failures.append(
            f"neutral-gray UI slab signature ({neutral_gray:.3f})"
        )

    return CaptureResult(
        file=str(path),
        sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
        width=width,
        height=height,
        portrait_black_fraction=round(portrait_black, 6),
        portrait_luma_deviation=round(portrait_deviation, 6),
        preview_background_fraction=round(preview_background, 6),
        preview_foreground_fraction=round(preview_foreground, 6),
        wheel_box_contrast_fraction=round(wheel_box_fraction, 6),
        neutral_gray_column_fraction=round(neutral_gray, 6),
        passed=not failures,
        failures=failures,
    )


def analyze_renderer_log(path: Path) -> dict[str, object]:
    text = path.read_text(encoding="utf-8", errors="replace")
    telemetry = []
    for line in text.splitlines():
        if not line.startswith("[EnhancedRenderer]"):
            continue
        fields = dict(
            re.findall(r"([a-z][a-z-]*)=(\d+(?:\.\d+)?%?)", line)
        )
        total = int(fields.get("triangles", "0"))
        if total == 0:
            continue
        direct = int(fields["direct"])
        reconstructed = int(fields["reconstructed"])
        fallback = int(fields["fallback"])
        world = int(fields.get("world", str(total)))
        world_fallback = int(fields.get("world-fallback", str(fallback)))
        visible_world = int(fields.get("visible-world", str(world)))
        visible_world_fallback = int(
            fields.get("visible-world-fallback", str(world_fallback))
        )
        effect_fallback = int(fields.get("effect-fallback", "0"))
        telemetry.append(
            {
                "triangles": total,
                "direct_percent": 100.0 * direct / total,
                "reconstructed_percent": 100.0 * reconstructed / total,
                "fallback_percent": 100.0 * fallback / total,
                "world_triangles": world,
                "world_fallback_percent": (
                    100.0 * world_fallback / world if world else 0.0
                ),
                "visible_world_triangles": visible_world,
                "visible_world_fallback_percent": (
                    100.0 * visible_world_fallback / visible_world
                    if visible_world
                    else 0.0
                ),
                "effect_fallback_primitives": effect_fallback,
                "glass_primitives": int(fields["glass"]),
            }
        )
    fatal_terms = [
        "unhandled exception",
        "shader compile failed",
        "shader link failed",
        "unmapped call:",
        "accessviolationexception",
    ]
    fatals = [term for term in fatal_terms if term in text.lower()]
    return {
        "file": str(path),
        "telemetry_blocks": telemetry,
        "maximum_fallback_percent": (
            max((block["fallback_percent"] for block in telemetry), default=None)
        ),
        "maximum_world_fallback_percent": max(
            (
                block["world_fallback_percent"]
                for block in telemetry
            ),
            default=None,
        ),
        "maximum_visible_world_fallback_percent": max(
            (
                block["visible_world_fallback_percent"]
                for block in telemetry
            ),
            default=None,
        ),
        "maximum_glass_primitives": max(
            (block["glass_primitives"] for block in telemetry), default=0
        ),
        "fatal_markers": fatals,
        "passed": bool(telemetry)
        and not fatals
        and max(
            (
                block["world_fallback_percent"]
                if "visible_world_fallback_percent" not in block
                else block["visible_world_fallback_percent"]
                for block in telemetry
            ),
            default=100.0,
        )
        # The retained source-address/GTE seam covers at least 94% of every
        # visible world interval in the worst dense arena.  A six-percent
        # ceiling is deliberately above the measured 5.51% peak but below
        # the pre-ScreenEffect misclassification (13%+), so that regression
        # remains a hard failure without pretending absent upstream GTE data
        # can be reconstructed exactly.
        <= 6.0,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("captures", nargs="*", type=Path)
    parser.add_argument(
        "--renderer-log",
        action="append",
        type=Path,
        default=[],
        help="renderer stderr log; repeat for a multi-map acceptance matrix",
    )
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    captures: list[Path] = []
    for item in args.captures:
        if item.is_dir():
            captures.extend(sorted(item.glob("*.ppm")))
        else:
            captures.append(item)
    results = [analyze_capture(path.resolve()) for path in captures]
    duplicate_hashes: dict[str, list[str]] = {}
    for result in results:
        duplicate_hashes.setdefault(result.sha256, []).append(result.file)
    duplicates = [
        files for files in duplicate_hashes.values() if len(files) > 1
    ]
    report: dict[str, object] = {
        "captures": [asdict(result) for result in results],
        "duplicate_captures": duplicates,
        "capture_pass_count": sum(result.passed for result in results),
        "capture_fail_count": sum(not result.passed for result in results),
    }
    renderer_logs = [
        analyze_renderer_log(path.resolve()) for path in args.renderer_log
    ]
    if renderer_logs:
        report["renderer_logs"] = renderer_logs
    report["passed"] = (
        all(result.passed for result in results)
        and not duplicates
        and all(bool(renderer_log["passed"]) for renderer_log in renderer_logs)
        and bool(results or renderer_logs)
    )

    encoded = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(encoded, encoding="utf-8")
    print(encoded, end="")
    raise SystemExit(0 if report["passed"] else 1)


if __name__ == "__main__":
    main()
