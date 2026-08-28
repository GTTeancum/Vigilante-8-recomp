#!/usr/bin/env python3
"""Run hidden 1080p acceptance for every high-resolution loading card."""

from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import hashlib
import json
import math
import os
from pathlib import Path
import subprocess
import sys

from PIL import Image, ImageDraw

from build_loading_cards import ARENAS, CARD_HEIGHTS, arena_stem


REPO = Path(__file__).resolve().parents[2]
DEFAULT_EXE = REPO / "V8_2_LOOSE" / "Vigilante82PC.exe"
DEFAULT_LOOSE_ROOT = REPO / "V8_2_LOOSE"
DEFAULT_OUTPUT = REPO / "artifacts" / "v82-loading-card-acceptance"
DEFAULT_RUNNER = Path(__file__).with_name("run_reference_soak.py")
DEFAULT_ASSET_DIR = (
    DEFAULT_LOOSE_ROOT / "mods" / "enhanced_textures_2x" / "loading_cards"
)
ACCEPTANCE_ARENAS = [*ARENAS]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--loose-root", type=Path, default=DEFAULT_LOOSE_ROOT)
    parser.add_argument("--asset-dir", type=Path, default=DEFAULT_ASSET_DIR)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--runner", type=Path, default=DEFAULT_RUNNER)
    parser.add_argument("--workers", type=int, default=4)
    parser.add_argument("--resolution", default="1920x1080")
    parser.add_argument("--frame", type=int, default=2600)
    parser.add_argument(
        "--maps",
        default="all",
        help="comma-separated zero-based arena slots, or 'all'",
    )
    parser.add_argument(
        "--reuse-runs",
        action="store_true",
        help="reuse passing per-arena captures and rerun only missing or failed cases",
    )
    return parser.parse_args()


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def selected_acceptance_slots(spec: str) -> list[int]:
    if spec.strip().lower() == "all":
        return list(range(len(ACCEPTANCE_ARENAS)))
    result = [int(value.strip()) for value in spec.split(",")]
    for slot in result:
        if slot < 0 or slot >= len(ACCEPTANCE_ARENAS):
            raise SystemExit(
                f"map slot {slot} is outside 0-{len(ACCEPTANCE_ARENAS) - 1}")
    return result


def correlation(left: Image.Image, right: Image.Image) -> float:
    a = left.convert("L").tobytes()
    b = right.convert("L").tobytes()
    mean_a = sum(a) / len(a)
    mean_b = sum(b) / len(b)
    numerator = sum((x - mean_a) * (y - mean_b) for x, y in zip(a, b))
    denom_a = math.sqrt(sum((x - mean_a) ** 2 for x in a))
    denom_b = math.sqrt(sum((y - mean_b) ** 2 for y in b))
    return numerator / (denom_a * denom_b) if denom_a and denom_b else 0.0


def verify_pixels(
    capture: Path, asset: Path, arena: str, width: int, height: int
) -> dict[str, float]:
    source_height = 448 if CARD_HEIGHTS.get(arena, 96) == 112 else 384
    rect_height_720 = 288 if source_height == 448 else 240
    rect = (
        0,
        round(height * (210 - rect_height_720 / 2) / 720),
        width,
        round(height * (210 + rect_height_720 / 2) / 720),
    )
    with Image.open(capture) as screen, Image.open(asset) as card:
        screen = screen.convert("RGB")
        if screen.size != (width, height):
            raise RuntimeError(
                f"capture is {screen.size}, expected {(width, height)}: {capture}"
            )
        if card.size != (1280, source_height):
            raise RuntimeError(
                f"asset is {card.size}, expected {(1280, source_height)}: {asset}"
            )
        expected = card.crop((0, 32, 1280, source_height - 32)).resize(
            (rect[2] - rect[0], rect[3] - rect[1]), Image.Resampling.BILINEAR
        )
        actual = screen.crop(rect)

        # The native title is deliberately preserved over the upper-left card
        # area. Compare the lower 70%, where every pixel comes from the asset.
        compare_y = round(actual.height * 0.30)
        expected = expected.crop((4, compare_y, expected.width - 4, expected.height - 4))
        actual = actual.crop((4, compare_y, actual.width - 4, actual.height - 4))
        corr = correlation(actual, expected)
        abs_error = sum(
            abs(a - b)
            for a, b in zip(actual.tobytes(), expected.tobytes())
        ) / (actual.width * actual.height * 3)
        if corr < 0.90 or abs_error > 18.0:
            raise RuntimeError(
                f"live card mismatch for {arena}: correlation={corr:.6f}, "
                f"meanAbsError={abs_error:.6f}"
            )
        return {"correlation": corr, "meanAbsError": abs_error}


def run_slot(args: argparse.Namespace, slot: int, width: int, height: int) -> dict[str, object]:
    arena = ACCEPTANCE_ARENAS[slot]
    result_dir = args.output / "runs" / f"{slot:02d}_{arena_stem(arena)}"
    result_dir.mkdir(parents=True, exist_ok=True)
    summary_path = result_dir / "summary.json"
    captures = list(result_dir.glob(f"*recompone_present_frame_{args.frame:06d}_*.ppm"))
    reuse = False
    if args.reuse_runs and summary_path.is_file() and len(captures) == 1:
        prior = json.loads(summary_path.read_text(encoding="utf-8"))
        reuse = bool(prior["runs"][0]["passed"])
    if not reuse:
        command = [
            sys.executable,
            str(args.runner.resolve()),
            "--exe",
            str(args.exe.resolve()),
            "--loose-root",
            str(args.loose_root.resolve()),
            "--output",
            str(result_dir.resolve()),
            "--maps",
            str(slot),
            "--frames",
            "360",
            "--presentation-resolution",
            args.resolution,
            "--presentation-frames",
            str(args.frame),
            "--stop-on-failure",
        ]
        env = os.environ.copy()
        env["DOTNET_BUNDLE_EXTRACT_BASE_DIR"] = str(
            (result_dir / "dotnet_bundle").resolve()
        )
        completed = subprocess.run(
            command, cwd=REPO, env=env, capture_output=True, text=True, check=False
        )
        (result_dir / "runner.stdout.log").write_text(
            completed.stdout, encoding="utf-8"
        )
        (result_dir / "runner.stderr.log").write_text(
            completed.stderr, encoding="utf-8"
        )
        if completed.returncode != 0:
            raise RuntimeError(
                f"arena runner failed for {arena}: {completed.stdout[-1000:]} "
                f"{completed.stderr[-1000:]}"
            )

    summary = json.loads(summary_path.read_text(encoding="utf-8"))
    run = summary["runs"][0]
    if not run["passed"] or run["actual_overlay"] != arena:
        raise RuntimeError(f"arena result mismatch for {arena}: {run}")
    stdout_path = Path(run["stdout_log"])
    log = stdout_path.read_text(encoding="utf-8", errors="replace")
    marker = f"[TexturePack] selected loading card overlay arena={arena}:"
    if marker not in log:
        raise RuntimeError(f"missing exact loading-card selection marker for {arena}")
    expected_load_marker = (
        f"loaded {len(ACCEPTANCE_ARENAS)} loading card overlays")
    if expected_load_marker not in log:
        raise RuntimeError(f"runtime did not load the complete card set for {arena}")

    captures = list(result_dir.glob(f"*recompone_present_frame_{args.frame:06d}_*.ppm"))
    if len(captures) != 1:
        raise RuntimeError(f"expected one loading capture for {arena}, found {captures}")
    asset = loading_card_asset(args, arena)
    metrics = verify_pixels(captures[0], asset, arena, width, height)
    return {
        "slot": slot,
        "arena": arena,
        "passed": True,
        "capture": str(captures[0]),
        "captureSha256": digest(captures[0]),
        "asset": str(asset),
        "assetSha256": digest(asset),
        **metrics,
    }


def loading_card_asset(args: argparse.Namespace, arena: str) -> Path:
    return args.asset_dir / f"{arena_stem(arena)}_loading_card_4x.ppm"


def write_contact_sheet(results: list[dict[str, object]], output: Path) -> None:
    thumb_size = (480, 270)
    label_height = 24
    columns = 3
    rows = math.ceil(len(results) / columns)
    sheet = Image.new(
        "RGB", (columns * thumb_size[0], rows * (thumb_size[1] + label_height)), "black"
    )
    draw = ImageDraw.Draw(sheet)
    for index, result in enumerate(results):
        x = index % columns * thumb_size[0]
        y = index // columns * (thumb_size[1] + label_height)
        with Image.open(str(result["capture"])) as capture:
            sheet.paste(capture.convert("RGB").resize(thumb_size), (x, y))
        draw.text((x + 6, y + thumb_size[1] + 5), str(result["arena"]), fill="white")
    sheet.save(output)


def main() -> int:
    args = parse_args()
    width, height = (int(value) for value in args.resolution.lower().split("x", 1))
    if args.workers <= 0:
        raise SystemExit("workers must be positive")
    for required in (args.exe, args.loose_root, args.asset_dir, args.runner):
        if not required.exists():
            raise SystemExit(f"required input does not exist: {required}")
    args.output.mkdir(parents=True, exist_ok=True)

    slots = selected_acceptance_slots(args.maps)
    results: list[dict[str, object]] = []
    with ThreadPoolExecutor(max_workers=args.workers) as executor:
        futures = {
            executor.submit(run_slot, args, slot, width, height): slot
            for slot in slots
        }
        for future in as_completed(futures):
            result = future.result()
            results.append(result)
            print(
                f"[loading-card-acceptance] PASS {result['arena']} "
                f"correlation={result['correlation']:.6f}",
                flush=True,
            )
    results.sort(key=lambda result: int(result["slot"]))
    if (args.maps.strip().lower() == "all" and
            len({result["captureSha256"] for result in results}) !=
            len(ACCEPTANCE_ARENAS)):
        raise RuntimeError("loading captures are not unique across all arenas")

    contact_sheet = args.output / "loading_cards_1080p_contact_sheet.png"
    write_contact_sheet(results, contact_sheet)
    report = {
        "schema": "v82-loading-card-acceptance-v1",
        "passed": True,
        "executable": str(args.exe.resolve()),
        "executableSha256": digest(args.exe),
        "resolution": [width, height],
        "captureFrame": args.frame,
        "cardsPassed": len(results),
        "cardsTotal": len(slots),
        "completeCardSetTotal": len(ACCEPTANCE_ARENAS),
        "minimumCorrelation": min(float(result["correlation"]) for result in results),
        "maximumMeanAbsError": max(float(result["meanAbsError"]) for result in results),
        "contactSheet": str(contact_sheet),
        "cards": results,
    }
    report_path = args.output / "acceptance.json"
    report_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(
        f"[loading-card-acceptance] PASS "
        f"{len(results)}/{len(slots)}: {report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
