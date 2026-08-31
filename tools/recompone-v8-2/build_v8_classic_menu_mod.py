#!/usr/bin/env python3
"""Build the editable assets for the Vigilante 8 Classic Menu mod."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

from PIL import Image


ROOT = Path(__file__).resolve().parents[2]
MOD = ROOT / "mods" / "v8_classic_menu"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--source-image",
        type=Path,
        default=MOD / "source" / "main_menu.png",
        help="editable 640x480 menu artwork",
    )
    parser.add_argument(
        "--image-output",
        type=Path,
        default=MOD / "images" / "main_menu.dds",
        help="runtime uncompressed RGBA DDS",
    )
    parser.add_argument(
        "--cursor-output",
        type=Path,
        default=MOD / "files" / "SHELL" / "CURSOR.PSX",
        help="runtime converted shared-selector model",
    )
    return parser.parse_args()


def build_background(source: Path, output: Path) -> None:
    with Image.open(source) as image:
        if image.size != (640, 480):
            raise ValueError(
                f"menu background must be 640x480, got "
                f"{image.width}x{image.height}"
            )
        output.parent.mkdir(parents=True, exist_ok=True)
        image.convert("RGBA").save(output, format="DDS")


def validate_manifest(image_output: Path) -> tuple[int, int]:
    manifest_path = MOD / "manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    groups = manifest.get("vramImages")
    if not isinstance(groups, list) or not groups:
        raise ValueError("classic-menu manifest has no VRAM-image groups")

    image_relative = image_output.resolve().relative_to(MOD.resolve()).as_posix()
    upload_count = 0
    identities: set[tuple[str, int, int, int, int]] = set()
    for group in groups:
        if group.get("image") != image_relative:
            raise ValueError(
                f"manifest image {group.get('image')!r} does not match "
                f"generated asset {image_relative!r}"
            )
        uploads = group.get("uploads")
        if not isinstance(uploads, list) or not uploads:
            raise ValueError("classic-menu manifest group has no uploads")
        for upload in uploads:
            source_x = int(upload["sourceX"])
            source_y = int(upload["sourceY"])
            width = int(upload["width"])
            height = int(upload["height"])
            if (
                width <= 0
                or height <= 0
                or source_x < 0
                or source_y < 0
                or source_x + width > 640
                or source_y + height > 480
            ):
                raise ValueError("manifest crop escapes the 640x480 background")
            identity = (
                str(upload["hash"]).upper(),
                int(upload["destinationX"]),
                int(upload["destinationY"]),
                width,
                height,
            )
            identities.add(identity)
            upload_count += 1
    return upload_count, len(identities)


def build_cursor(output: Path) -> None:
    builder = ROOT / "tools" / "recompone-v8-2" / "build_v8_cursor_replacement.py"
    subprocess.run(
        [sys.executable, str(builder), "--output", str(output)],
        cwd=ROOT,
        check=True,
    )


def main() -> None:
    args = parse_args()
    build_background(args.source_image, args.image_output)
    uploads, identities = validate_manifest(args.image_output)
    build_cursor(args.cursor_output)
    report = {
        "source_image": str(args.source_image),
        "source_image_sha256": sha256(args.source_image),
        "runtime_image": str(args.image_output),
        "runtime_image_sha256": sha256(args.image_output),
        "cursor": str(args.cursor_output),
        "cursor_sha256": sha256(args.cursor_output),
        "manifest_uploads": uploads,
        "manifest_distinct_identities": identities,
    }
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
