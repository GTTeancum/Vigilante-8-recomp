#!/usr/bin/env python3
"""Capture, reconstruct, and upscale every V8:2 arena loading card."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys

from PIL import Image


REPO = Path(__file__).resolve().parents[2]
DEFAULT_EXE = REPO / "V8_2_LOOSE" / "Vigilante82PC.exe"
DEFAULT_LOOSE_ROOT = REPO / "V8_2_LOOSE"
DEFAULT_OUTPUT = REPO / "artifacts" / "v82-loading-cards"
DEFAULT_ASSET_DIR = (
    DEFAULT_LOOSE_ROOT / "mods" / "enhanced_textures_2x" / "loading_cards"
)
DEFAULT_RUNNER = Path(__file__).with_name("run_reference_soak.py")
DEFAULT_UPSCALER = REPO / "build" / "realesrgan" / "bin" / "realesrgan-ncnn-vulkan.exe"

ARENAS = [
    "LEVELS_ROUTE66",
    "LEVELS_OLYMPIC",
    "LEVELS_BAYOU",
    "LEVELS_LAUNCH",
    "LEVELS_STEELMIL",
    "LEVELS_NUCLEAR",
    "LEVELS_OILFIELD",
    "LEVELS_HARBOR",
    "LEVELS_V8_SCRTBASE",
    "LEVELS_V8_SANDFACT",
    "LEVELS_V8_OILFIELD",
    "LEVELS_V8_AIRGRAVE",
    "LEVELS_V8_WILDWEST",
    "LEVELS_V8_HOOVRDAM",
    "LEVELS_V8_VALLYFRM",
    "LEVELS_V8_CASNOCTY",
    "LEVELS_V8_CANYNLND",
    "LEVELS_V8_SKIRESRT",
]

CARD_HEIGHTS = {
    "LEVELS_V8_SCRTBASE": 112,
    "LEVELS_V8_CANYNLND": 112,
}

STRIP_RE = re.compile(
    r"^vram_load_\d+_(?P<x>\d+)_(?P<y>\d+)_16x(?P<h>\d+)_"
    r"[0-9A-F]{8}\.ppm$"
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--loose-root", type=Path, default=DEFAULT_LOOSE_ROOT)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--asset-dir", type=Path, default=DEFAULT_ASSET_DIR)
    parser.add_argument("--runner", type=Path, default=DEFAULT_RUNNER)
    parser.add_argument("--upscaler", type=Path, default=DEFAULT_UPSCALER)
    parser.add_argument("--model", default="realesrgan-x4plus-anime")
    parser.add_argument("--tile-size", type=int, default=64)
    parser.add_argument(
        "--maps",
        default="all",
        help="comma-separated zero-based arena slots, or 'all'",
    )
    parser.add_argument(
        "--reuse-captures",
        action="store_true",
        help="rebuild from existing native strip directories without launching the game",
    )
    parser.add_argument(
        "--skip-upscale",
        action="store_true",
        help="capture and reconstruct native cards without running the upscaler",
    )
    return parser.parse_args()


def selected_slots(spec: str) -> list[int]:
    if spec.strip().lower() == "all":
        return list(range(len(ARENAS)))
    result = [int(value.strip()) for value in spec.split(",")]
    for slot in result:
        if slot < 0 or slot >= len(ARENAS):
            raise SystemExit(f"map slot {slot} is outside 0-{len(ARENAS) - 1}")
    return result


def arena_stem(arena: str) -> str:
    return arena.removeprefix("LEVELS_").lower()


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def capture_strips(args: argparse.Namespace, slot: int, strip_dir: Path) -> None:
    strip_dir.mkdir(parents=True, exist_ok=True)
    run_dir = args.output / "runs" / arena_stem(ARENAS[slot])
    run_dir.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    env.update(
        {
            "RECOMPONE_DUMP_VRAM_LOADS": "1",
            "RECOMPONE_VRAM_LOAD_DUMP_SIZE": (
                f"16x{CARD_HEIGHTS.get(ARENAS[slot], 96)}"
            ),
            "RECOMPONE_VRAM_LOAD_DUMP_DIR": str(strip_dir.resolve()),
        }
    )
    command = [
        sys.executable,
        str(args.runner.resolve()),
        "--exe",
        str(args.exe.resolve()),
        "--loose-root",
        str(args.loose_root.resolve()),
        "--output",
        str(run_dir.resolve()),
        "--maps",
        str(slot),
        "--frames",
        "360",
        "--stop-on-failure",
    ]
    subprocess.run(command, cwd=REPO, env=env, check=True)


def reconstruct_card(
    strip_dir: Path, output: Path, height: int
) -> list[Path]:
    groups: dict[int, dict[int, Path]] = {}
    for path in strip_dir.glob("*.ppm"):
        match = STRIP_RE.match(path.name)
        if match is None or int(match.group("h")) != height:
            continue
        y = int(match.group("y"))
        x = int(match.group("x"))
        by_x = groups.setdefault(y, {})
        if x in by_x:
            raise RuntimeError(
                f"duplicate loading-card strip x={x}, y={y}: {strip_dir}"
            )
        by_x[x] = path

    expected_x = list(range(0, 320, 16))
    candidates = [by_x for by_x in groups.values() if sorted(by_x) == expected_x]
    if len(candidates) != 1:
        raise RuntimeError(
            f"loading-card strips in {strip_dir} produced {len(candidates)} "
            f"complete {height}-pixel groups, expected one"
        )
    by_x = candidates[0]

    card = Image.new("RGB", (320, height))
    ordered = [by_x[x] for x in expected_x]
    for x, path in zip(expected_x, ordered, strict=True):
        with Image.open(path) as strip:
            if strip.size != (16, height):
                raise RuntimeError(f"unexpected strip dimensions {strip.size}: {path}")
            card.paste(strip.convert("RGB"), (x, 0))
    output.parent.mkdir(parents=True, exist_ok=True)
    card.save(output)
    return ordered


def upscale_card(
    upscaler: Path,
    model: str,
    tile_size: int,
    native_path: Path,
    asset_path: Path,
    work_dir: Path,
) -> None:
    input_png = work_dir / "native.png"
    output_png = work_dir / "upscaled.png"
    with Image.open(native_path) as native:
        expected_size = (native.width * 4, native.height * 4)
        native.convert("RGB").save(input_png)
    subprocess.run(
        [
            str(upscaler.resolve()),
            "-i",
            str(input_png.resolve()),
            "-o",
            str(output_png.resolve()),
            "-n",
            model,
            "-s",
            "4",
            "-t",
            str(tile_size),
            "-f",
            "png",
        ],
        cwd=upscaler.resolve().parent,
        check=True,
    )
    with Image.open(output_png) as upscaled:
        if upscaled.size != expected_size:
            raise RuntimeError(
                f"upscaler returned {upscaled.size}, expected {expected_size}"
            )
        asset_path.parent.mkdir(parents=True, exist_ok=True)
        upscaled.convert("RGB").save(asset_path)


def main() -> int:
    args = parse_args()
    for required in (args.exe, args.loose_root, args.runner, args.upscaler):
        if not required.exists():
            raise SystemExit(f"required input does not exist: {required}")

    args.output.mkdir(parents=True, exist_ok=True)
    args.asset_dir.mkdir(parents=True, exist_ok=True)
    records: list[dict[str, object]] = []
    for slot in selected_slots(args.maps):
        arena = ARENAS[slot]
        stem = arena_stem(arena)
        native_height = CARD_HEIGHTS.get(arena, 96)
        work_dir = args.output / "native" / stem
        strip_dir = work_dir / "strips"
        if not args.reuse_captures:
            capture_strips(args, slot, strip_dir)
        native_path = work_dir / f"{stem}_loading_card_native.ppm"
        strips = reconstruct_card(strip_dir, native_path, native_height)
        asset_path = args.asset_dir / f"{stem}_loading_card_4x.ppm"
        if not args.skip_upscale:
            upscale_card(
                args.upscaler,
                args.model,
                args.tile_size,
                native_path,
                asset_path,
                work_dir,
            )
        record = {
            "slot": slot,
            "arena": arena,
            "native": str(native_path),
            "nativeSha256": digest(native_path),
            "nativeDimensions": [320, native_height],
            "asset": str(asset_path) if asset_path.is_file() else None,
            "assetSha256": digest(asset_path) if asset_path.is_file() else None,
            "strips": [str(path) for path in strips],
        }
        records.append(record)
        if asset_path.is_file():
            print(
                f"[loading-card] {arena} -> {asset_path.name} "
                f"sha256={record['assetSha256']}",
                flush=True,
            )
        else:
            print(f"[loading-card] captured {arena}", flush=True)

    recorded_arenas = {str(record["arena"]) for record in records}
    for slot, arena in enumerate(ARENAS):
        if arena in recorded_arenas:
            continue
        asset_path = args.asset_dir / f"{arena_stem(arena)}_loading_card_4x.ppm"
        if not asset_path.is_file():
            continue
        with Image.open(asset_path) as asset:
            if asset.size not in ((1280, 384), (1280, 448)):
                raise RuntimeError(
                    f"existing asset has dimensions {asset.size}: {asset_path}"
                )
        records.append(
            {
                "slot": slot,
                "arena": arena,
                "native": None,
                "nativeSha256": None,
                "asset": str(asset_path),
                "assetSha256": digest(asset_path),
                "strips": [],
                "origin": "preexisting",
            }
        )
    records.sort(key=lambda record: int(record["slot"]))

    manifest = {
        "schema": "v82-loading-cards-v1",
        "model": args.model,
        "tileSize": args.tile_size,
        "nativeDimensions": [[320, 96], [320, 112]],
        "assetDimensions": [[1280, 384], [1280, 448]],
        "cards": records,
    }
    manifest_path = args.output / "manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"[loading-card] wrote {manifest_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
