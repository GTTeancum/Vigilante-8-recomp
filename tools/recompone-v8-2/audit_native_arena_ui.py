#!/usr/bin/env python3
"""Audit a registered V8:2 mod arena's selector and loading-card assets."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import sys


REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO / "tools"))
sys.path.insert(0, str(REPO / "tools" / "blender_addons"))

from vigilante8_vehicle_tools import iff
import port_dreamland_assets
import v82_arena_registry
import v82_native_selector_table


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def transparent_pixels(tim: bytes) -> int:
    import struct

    clut_size = struct.unpack_from("<I", tim, 8)[0]
    clut_width = struct.unpack_from("<H", tim, 16)[0]
    palette = struct.unpack_from(f"<{clut_width}H", tim, 20)
    image_offset = 8 + clut_size
    width, height = v82_native_selector_table.tim_dimensions(tim)
    indices = tim[image_offset + 12:image_offset + 12 + width * height]
    if len(indices) != width * height:
        raise ValueError("selector TIM pixel payload is truncated")
    return sum(palette[index] == 0 for index in indices)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mod = REPO / "V8_2_LOOSE" / "mods" / "v82_n64_super_dreamland"
    parser.add_argument(
        "--retail-table", type=Path,
        default=REPO / "V8_2_LOOSE" / "SHELL" / "LEVELSEL.TBL",
    )
    parser.add_argument(
        "--mod-table", type=Path,
        default=mod / "files" / "SHELL" / "LEVELSEL.TBL",
    )
    parser.add_argument(
        "--registry", type=Path, default=mod / "ARENAS.V8R",
    )
    parser.add_argument(
        "--arena-exp", type=Path,
        default=mod / "files" / "LEVELS" / "N64" / "DREAMLND.EXP",
    )
    parser.add_argument(
        "--arena-dll", type=Path,
        default=mod / "files" / "LEVELS" / "N64" / "DREAMLND.DLL",
    )
    parser.add_argument(
        "--selector-source", type=Path,
        default=REPO / "PS1 game" / "SHELL" / "LOCATNS.BIN",
    )
    parser.add_argument("--template-index", type=int, default=8)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    retail = v82_native_selector_table.split_resource_table(
        args.retail_table.read_bytes()
    )
    converted = v82_native_selector_table.split_resource_table(
        args.mod_table.read_bytes()
    )
    entries = v82_arena_registry.parse_registry(args.registry.read_bytes())
    if len(entries) != 1:
        raise ValueError("Dreamland audit expects exactly one arena registry entry")
    primary_export = v82_arena_registry.validate_primary_export(
        entries[0], args.arena_dll.read_bytes()
    )
    retail_previews = len(retail) - 1
    expected_count = retail_previews + len(entries) + 1
    if len(converted) != expected_count:
        raise ValueError(
            f"selector has {len(converted)} resources, expected {expected_count}"
        )
    if converted[:retail_previews] != retail[:retail_previews]:
        raise ValueError("conversion modified one or more retail previews")
    if converted[-1] != retail[-1]:
        raise ValueError("conversion modified or displaced the retail background")

    inserted = converted[retail_previews:-1]
    for index, (entry, preview) in enumerate(
        zip(entries, inserted, strict=True), start=retail_previews
    ):
        if entry.preview_index != index:
            raise ValueError(
                f"registry preview {entry.preview_index} does not match slot {index}"
            )
        if v82_native_selector_table.tim_dimensions(preview) != (220, 74):
            raise ValueError(f"slot {index} is not a native 220x74 CI8 TIM")

    template_index = args.template_index
    if not 0 <= template_index < retail_previews:
        raise ValueError("template index is outside the retail preview set")
    rebuilt_preview = port_dreamland_assets._selector_preview_tim(
        args.selector_source, retail[template_index]
    )
    if len(inserted) != 1 or inserted[0] != rebuilt_preview:
        raise ValueError(
            "inserted preview is not the exact full-frame conversion using "
            f"retail template {template_index}"
        )
    if inserted[0] in retail[:retail_previews]:
        raise ValueError("inserted preview duplicates a retail map preview")

    document = iff.parse(args.arena_exp.read_bytes())
    terrain = list(document.forms(b"TERR"))
    if len(terrain) != 1:
        raise ValueError("arena EXP does not contain exactly one TERR form")
    loading = [node.payload for node in terrain[0].children if node.tag == b"XLSC"]
    if len(loading) != 1:
        raise ValueError(
            f"arena EXP contains {len(loading)} loading chunks, expected exactly one"
        )
    card = loading[0]
    if len(card) < 16:
        raise ValueError("loading chunk is truncated")
    width = int.from_bytes(card[4:6], "little")
    height = int.from_bytes(card[6:8], "little")
    if (width, height) != (320, 96):
        raise ValueError(f"loading card is {width}x{height}, expected 320x96")
    if card[10:12] != b"\x00\x38" or card[14:16] != b"\x02\x00":
        raise ValueError("loading card is not a native PlayStation MDEC BS v2 frame")

    files = sorted(
        str(path.relative_to(mod)).replace("\\", "/")
        for path in mod.rglob("*") if path.is_file()
    )
    expected_files = {
        "ARENAS.V8R",
        "mod.json",
        "files/LEVELS/N64/DREAMLND.DLL",
        "files/LEVELS/N64/DREAMLND.EXP",
        "files/SHELL/LEVELSEL.TBL",
    }
    unexpected = sorted(set(files) - expected_files)
    if unexpected:
        raise ValueError(
            "mod contains presentation overlay files outside the native asset set: "
            + ", ".join(unexpected)
        )

    report = {
        "schema": 1,
        "status": "PASS",
        "selector": {
            "resourceCount": len(converted),
            "retailPreviewsPreserved": retail_previews,
            "insertedPreviews": len(inserted),
            "backgroundCount": 1,
            "backgroundPosition": len(converted) - 1,
            "backgroundByteExact": True,
            "slot": entries[0].preview_index,
            "dimensions": [220, 74],
            "format": "PlayStation TIM CI8",
            "resize": "full source frame to 220x74",
            "crop": "none",
            "templateIndex": template_index,
            "templatePurpose": "transparent silhouette and native VRAM origins only",
            "transparentPixels": transparent_pixels(inserted[0]),
            "sha256": digest(inserted[0]),
        },
        "loading": {
            "chunkCount": 1,
            "dimensions": [width, height],
            "format": "PlayStation MDEC BS v2",
            "bytes": len(card),
            "sha256": digest(card),
        },
        "registry": [entry.__dict__ for entry in entries],
        "primaryArenaCallbackContract": {
            "pathStem": Path(entries[0].path).stem,
            "dllFirstExport": primary_export,
            "caseSensitiveExactMatch": True,
        },
        "nativeCaptionContract": {
            "displayNameMaxAsciiCharacters": v82_arena_registry.NATIVE_NAME_MAX,
            "subtitleMaxAsciiCharacters": v82_arena_registry.NATIVE_SUBTITLE_MAX,
            "entries": [
                {
                    "slot": entry.preview_index,
                    "displayName": entry.name,
                    "displayNameCharacters": len(entry.name),
                    "subtitle": entry.subtitle,
                    "subtitleCharacters": len(entry.subtitle),
                }
                for entry in entries
            ],
        },
        "modFiles": files,
        "presentationOverlayFiles": [],
    }
    rendered = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(rendered, encoding="utf-8")
    print("[NativeArenaUiAudit] PASS")
    print(
        f"[NativeArenaUiAudit] selector resources={len(converted)} "
        f"retail-previews={retail_previews} inserted=1 background=1(last,byte-exact)"
    )
    print(
        f"[NativeArenaUiAudit] preview slot={entries[0].preview_index} "
        f"format=CI8 dimensions=220x74 crop=none full-frame=1 "
        f"template={template_index} bytes={len(inserted[0])} "
        f"transparent-pixels={report['selector']['transparentPixels']}"
    )
    print(
        f"[NativeArenaUiAudit] loading chunks=1 format=MDEC-BS-v2 "
        f"dimensions={width}x{height} bytes={len(card)}"
    )
    for entry in entries:
        print(
            f"[NativeArenaUiAudit] native-caption slot={entry.preview_index} "
            f"name={entry.name!r} chars={len(entry.name)}/"
            f"{v82_arena_registry.NATIVE_NAME_MAX} subtitle={entry.subtitle!r} "
            f"chars={len(entry.subtitle)}/"
            f"{v82_arena_registry.NATIVE_SUBTITLE_MAX}"
        )
    print("[NativeArenaUiAudit] presentation-overlay-files=0")
    print(
        f"[NativeArenaUiAudit] primary-callback path-stem={primary_export!r} "
        f"dll-first-export={primary_export!r} exact-case=1"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
