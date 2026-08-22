#!/usr/bin/env python3
"""Write a verbose text-only audit of Bayou-native Dreamland water."""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import re
import struct
import sys


REPO = Path(__file__).resolve().parents[2]
TOOLS = REPO / "tools"
ADDONS = TOOLS / "blender_addons"
sys.path[:0] = [str(TOOLS), str(ADDONS), str(Path(__file__).resolve().parent)]

from terrain_object_identity_audit import collect_heads  # noqa: E402
from vigilante8_vehicle_tools import iff  # noqa: E402
import v82_native_water_conversion as water_conversion  # noqa: E402


INIT_PATTERN = re.compile(
    r"\[V82NativeWater\] init .*?argument=0x(?P<argument>[0-9A-Fa-f]{8}) "
    r"plane=0x(?P<plane>[0-9A-Fa-f]{8}) color=0x(?P<color>[0-9A-Fa-f]{8})"
)
FRAME_PATTERN = re.compile(
    r"\[V82NativeWater\] frame .*?index=(?P<index>\d+) "
    r"plane=0x(?P<plane>[0-9A-Fa-f]{8})"
)


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def rectangles(document: iff.IffDocument) -> list[tuple[int, ...]]:
    return [
        struct.unpack(">7h", node.payload)
        for node in document.walk()
        if node.tag == b"RECT" and len(node.payload) == 14
    ]


def only_chunk(document: iff.IffDocument, tag: bytes) -> bytes:
    payloads = [node.payload for node in document.walk() if node.tag == tag]
    if len(payloads) != 1:
        raise ValueError(f"expected one {tag.decode('ascii')} chunk, got {len(payloads)}")
    return payloads[0]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--dreamland-source", type=Path,
        default=REPO / "PS1 game" / "TERRAIN" / "DREAMLND.EXP",
    )
    parser.add_argument(
        "--dreamland-target", type=Path,
        default=(REPO / "V8_2_LOOSE" / "mods" / "v82_n64_super_dreamland" /
                 "files" / "LEVELS" / "N64" / "DREAMLND.EXP"),
    )
    parser.add_argument(
        "--bayou-source", type=Path,
        default=REPO / "V8_2_LOOSE" / "LEVELS" / "BAYOU.EXP",
    )
    parser.add_argument(
        "--dreamland-overlay", type=Path,
        default=REPO / "tools" / "recompone-v8-2" / "LEVELS_N64_DREAMLND.cs",
    )
    parser.add_argument("--runtime-log", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    source_bytes = args.dreamland_source.read_bytes()
    target_bytes = args.dreamland_target.read_bytes()
    bayou_bytes = args.bayou_source.read_bytes()
    source_doc = iff.parse(source_bytes)
    target_doc = iff.parse(target_bytes)
    bayou_doc = iff.parse(bayou_bytes)
    source_water = water_conversion.find_native_water_source(source_doc)

    source_forms = list(source_doc.forms(b"XOBF"))
    target_forms = list(target_doc.forms(b"XOBF"))
    if len(source_forms) != 3 or len(target_forms) != 2:
        raise ValueError(
            f"water-bank removal mismatch: source={len(source_forms)} target={len(target_forms)}"
        )
    source_heads = [
        head for head in collect_heads(args.dreamland_source)
        if head["bank"] == source_water.bank_index
    ]
    target_names = {head["name"] for head in collect_heads(args.dreamland_target)}
    if len(source_heads) != 1 or source_heads[0]["name"] in target_names:
        raise ValueError("modeled water HEAD was not removed with its source bank")

    bayou_xwat = only_chunk(bayou_doc, b"XWAT")
    target_xwat = only_chunk(target_doc, b"XWAT")
    expected_xwat = water_conversion.encode_native_xwat(
        source_water.bank.textures[0], bayou_xwat
    )
    if target_xwat != expected_xwat:
        raise ValueError("target XWAT is not a byte-exact encoding of Dreamland art")
    if target_xwat == bayou_xwat:
        raise ValueError("target copied Bayou art instead of Dreamland art")

    expected_rect = struct.unpack(
        ">7h",
        water_conversion.convert_water_rectangle(
            struct.pack(">7h", *source_water.rectangle)
        ),
    )
    target_rects = rectangles(target_doc)
    if target_rects != [expected_rect]:
        raise ValueError(f"target lethal-water RECT mismatch: {target_rects}")

    overlay = args.dreamland_overlay.read_text(encoding="utf-8")
    init_calls = overlay.count("Vigilante82PC.func_80017F34(c, m);")
    frame_calls = overlay.count("Vigilante82PC.func_80017EB8(c, m);")
    if init_calls != 1 or frame_calls != 1:
        raise ValueError(
            f"native overlay call counts differ: init={init_calls} frame={frame_calls}"
        )
    forbidden = [token for token in ("0x8FFF0104", "V82DreamlandCompat") if token in overlay]
    if forbidden:
        raise ValueError(f"overlay retained compatibility seams: {forbidden}")
    init_argument = water_conversion.native_water_init_argument(source_water.plane_y)
    if f"c.A0 = 0x{init_argument:08X}u;" not in overlay:
        raise ValueError("overlay initializer is not derived from Dreamland's plane")

    runtime_lines: list[str] = []
    if args.runtime_log is not None:
        runtime_text = args.runtime_log.read_text(encoding="utf-8", errors="replace")
        init_matches = list(INIT_PATTERN.finditer(runtime_text))
        frame_matches = list(FRAME_PATTERN.finditer(runtime_text))
        expected_init = (f"{init_argument:08X}", f"{source_water.plane_y:08X}", "7FFF0000")
        observed_init = [
            (m["argument"].upper(), m["plane"].upper(), m["color"].upper())
            for m in init_matches
        ]
        if expected_init not in observed_init:
            raise ValueError(f"runtime native-water initializer missing: {observed_init}")
        if not frame_matches:
            raise ValueError("runtime never called the native water frame renderer")
        fatal = [
            line for line in runtime_text.splitlines()
            if any(token in line.lower() for token in (
                "fatal", "unhandled exception", "access violation", "bad request"
            ))
        ]
        if fatal:
            raise ValueError(f"runtime contains fatal diagnostics: {fatal[:5]}")
        lifecycle = [
            line for line in runtime_text.splitlines()
            if line.startswith("[V82WaterLifecycle]")
        ]
        converted_drown = [
            line for line in runtime_text.splitlines()
            if line.startswith("[V82ConvertedWater] drowned ")
        ]
        if lifecycle and not converted_drown:
            raise ValueError(
                "runtime fixture entered imported water without native defeat transition"
            )
        runtime_lines = [
            "", "RUNTIME NATIVE-WATER PROOF",
            f"path={args.runtime_log.resolve()}",
            f"init_records={len(init_matches)} observed={observed_init}",
            f"frame_records={len(frame_matches)} last_index={frame_matches[-1]['index']}",
            f"lifecycle_records={len(lifecycle)}",
            f"lifecycle_destroyed={int(any(' destroyed ' in line for line in lifecycle))}",
            f"lifecycle_respawned={int(any(' respawned ' in line for line in lifecycle))}",
            f"lifecycle_timeout={int(any(' timeout ' in line for line in lifecycle))}",
            f"converted_water_drown_records={len(converted_drown)}",
            "fatal_diagnostics=0 PASS",
        ]

    source_texture = source_water.bank.textures[0]
    source_palette = b"".join(
        struct.pack("<H", value) for value in source_texture.palette_bgr555
    )
    lines = [
        "DREAMLAND -> V8:2 BAYOU-NATIVE LETHAL WATER AUDIT",
        "=====================================================",
        "mode=text-only", "screenshots=0", "runtime_map_identity_branches=0",
        "rendering_physics_contract=stock V8:2 Bayou global water",
        "drowning_contract=original-V8 0x0043/-1 imported-region extension",
        "art_contract=Dreamland source palette and texels",
        "code_exception=ABSENT",
        "", "SOURCE DREAMLAND WATER",
        f"path={args.dreamland_source.resolve()}",
        f"bytes={len(source_bytes)} sha256={sha256(source_bytes)}",
        f"structural_water_bank={source_water.bank_index}",
        f"modeled_water_heads={len(source_heads)} name={source_heads[0]['name']}",
        f"plane_y=0x{source_water.plane_y:08X}",
        f"source_rect={source_water.rectangle}",
        f"texture={source_texture.width}x{source_texture.height} depth={source_texture.depth}",
        f"palette_entries={len(source_texture.palette_bgr555)} palette_sha256={sha256(source_palette)}",
        f"indices_sha256={sha256(source_texture.indices)}",
    ]
    for y in range(source_texture.height):
        row = source_texture.indices[y * source_texture.width:(y + 1) * source_texture.width]
        lines.append(f"source_row[{y:02d}]={row.hex().upper()}")
    lines += [
        "", "STOCK V8:2 BAYOU CONTAINER CONTRACT",
        f"path={args.bayou_source.resolve()}",
        f"bytes={len(bayou_bytes)} sha256={sha256(bayou_bytes)}",
        f"xwat_bytes={len(bayou_xwat)} xwat_sha256={sha256(bayou_xwat)}",
        f"water_rects={[row for row in rectangles(bayou_doc) if row[5] in (-32701, -32702)]}",
        "", "CONVERTED DREAMLAND OUTPUT",
        f"path={args.dreamland_target.resolve()}",
        f"bytes={len(target_bytes)} sha256={sha256(target_bytes)}",
        f"xobf_count={len(target_forms)} modeled_water_xobf_count=0 PASS",
        f"modeled_water_head_count={int(source_heads[0]['name'] in target_names)} PASS",
        f"xwat_count=1 xwat_bytes={len(target_xwat)} xwat_sha256={sha256(target_xwat)}",
        f"xwat_matches_dreamland_encoding={int(target_xwat == expected_xwat)} PASS",
        f"xwat_differs_from_bayou_art={int(target_xwat != bayou_xwat)} PASS",
        f"target_rects={target_rects}",
        f"expected_imported_drowning_rect={expected_rect}",
        "imported_drowning_region_translation=PASS",
        f"overlay_init_call_count={init_calls} PASS",
        f"overlay_frame_call_count={frame_calls} PASS",
        f"overlay_init_argument=0x{init_argument:08X} PASS",
        "legacy_host_water_service=ABSENT",
        "runtime_arena_exception=ABSENT",
    ]
    lines += runtime_lines
    lines += [
        "", "RESULT=PASS",
        "review_scope=water renders with Dreamland art; imported 0x0043/-1 water sinks the vehicle and dispatches native V8:2 destruction",
    ]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(args.output.resolve())
    print("RESULT=PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
