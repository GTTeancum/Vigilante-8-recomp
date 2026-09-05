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
        "--options-source-image",
        type=Path,
        default=MOD / "source" / "options_left.png",
        help="editable 224x480 Options left-panel artwork",
    )
    parser.add_argument(
        "--options-table-template",
        type=Path,
        default=ROOT / "V8_2_LOOSE" / "SHELL" / "OPTTABLE.TBL",
        help="retail V8:2 Options table used as the native asset template",
    )
    parser.add_argument(
        "--options-table-output",
        type=Path,
        default=MOD / "files" / "SHELL" / "OPTTABLE.TBL",
        help="runtime loose-file Options table override",
    )
    parser.add_argument(
        "--psxavenc",
        type=Path,
        required=True,
        help="psxavenc v0.3.1+ executable used for native MDEC BS v2",
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


def validate_manifest(image_outputs: list[Path]) -> tuple[int, int]:
    manifest_path = MOD / "manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    groups = manifest.get("vramImages")
    if not isinstance(groups, list) or not groups:
        raise ValueError("classic-menu manifest has no VRAM-image groups")

    generated: dict[str, tuple[int, int]] = {}
    for output in image_outputs:
        relative = output.resolve().relative_to(MOD.resolve()).as_posix()
        with Image.open(output) as image:
            generated[relative] = image.size
    upload_count = 0
    identities: set[tuple[str, int, int, int, int]] = set()
    for group in groups:
        image_relative = group.get("image")
        if image_relative not in generated:
            raise ValueError(
                f"manifest image {image_relative!r} was not generated"
            )
        image_width, image_height = generated[image_relative]
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
                or source_x + width > image_width
                or source_y + height > image_height
            ):
                raise ValueError(
                    f"manifest crop escapes {image_relative} "
                    f"({image_width}x{image_height})"
                )
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


def build_options_table(
    source: Path, template: Path, output: Path, psxavenc: Path
) -> None:
    builder = (
        ROOT / "tools" / "recompone-v8-2" / "build_v8_options_backplate.py"
    )
    subprocess.run(
        [
            sys.executable,
            str(builder),
            "--source",
            str(source),
            "--template",
            str(template),
            "--output",
            str(output),
            "--psxavenc",
            str(psxavenc),
        ],
        cwd=ROOT,
        check=True,
    )


def main() -> None:
    args = parse_args()
    build_background(args.source_image, args.image_output)
    build_options_table(
        args.options_source_image,
        args.options_table_template,
        args.options_table_output,
        args.psxavenc,
    )
    uploads, identities = validate_manifest([args.image_output])
    build_cursor(args.cursor_output)
    report = {
        "source_image": str(args.source_image),
        "source_image_sha256": sha256(args.source_image),
        "runtime_image": str(args.image_output),
        "runtime_image_sha256": sha256(args.image_output),
        "options_source_image": str(args.options_source_image),
        "options_source_image_sha256": sha256(args.options_source_image),
        "options_runtime_table": str(args.options_table_output),
        "options_runtime_table_sha256": sha256(args.options_table_output),
        "cursor": str(args.cursor_output),
        "cursor_sha256": sha256(args.cursor_output),
        "manifest_uploads": uploads,
        "manifest_distinct_identities": identities,
    }
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
