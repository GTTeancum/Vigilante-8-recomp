#!/usr/bin/env python3
"""Build Dreamland's complete native V8 shell presentation.

The N64 location-wheel object is decoded to semantic geometry, collision, and
texture fields, then independently compiled and appended after all twelve
retail PS1 wheel objects. The N64 JPEG loading art is decoded to pixels,
letterboxed to the PS1 shell's 320x112 layout, and encoded as native MDEC BS
v2. No ROM range, donor object, or opaque source payload is copied to either
game asset.

The loading image encoder is the open-source psxavenc command-line tool:
https://github.com/WonderfulToolchain/psxavenc
"""

from __future__ import annotations

import argparse
from dataclasses import asdict
from hashlib import sha256
from io import BytesIO
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
sys.path.insert(0, str(ROOT / "tools" / "blender_addons"))

from v8_n64_level import (  # noqa: E402
    Chunk,
    V8N64Rom,
    convert_xobf_bin,
    iff_form,
    root_children,
)
from vigilante8_vehicle_tools import (  # noqa: E402
    compiler,
    conversion,
    iff,
    project,
    registry,
    stats,
)


def _digest(data: bytes) -> str:
    return sha256(data).hexdigest()


def _decode_bank(data: bytes) -> project.ObjectBank:
    form = iff.IffChunk(
        tag=b"FORM",
        form_type=b"XOBF",
        children=[iff.IffChunk(tag=b"BIN ", payload=data)],
    )
    return registry._decode_bank(form, "V8")


def _compile_bank(bank: project.ObjectBank) -> bytes:
    values = {
        field.name: field.default
        for field in stats.PROFILES["V8"].authoring_fields
        if field.name != "vehicle_type"
    }
    authoring_project = project.VehicleProject(
        schema_version=project.SCHEMA_VERSION,
        stable_id="n64.super_dreamland_64.location_icon",
        display_name="Super Dreamland 64 Location Icon",
        game="V8",
        groups=bank.groups,
        slots=bank.slots,
        collisions=bank.collisions,
        textures=bank.textures,
        animations=bank.animations,
        stats=values,
        body_kind=0,
    )
    return compiler.compile_model(authoring_project)


def _bank_shape(bank: project.ObjectBank) -> dict[str, int]:
    return {
        "groups": len(bank.groups),
        "slots": len(bank.slots),
        "collisions": len(bank.collisions),
        "textures": len(bank.textures),
        "animations": len(bank.animations),
    }


def decode_n64_location_record(shell_data: bytes) -> dict[str, object]:
    """Recover Dreamland's native record and its intended California marker.

    The shipped N64 Dreamland record says California but accidentally reuses
    Ski Resort's Colorado marker.  The ordinary Valley Farms record uses the
    same California subtitle and supplies the native California map point in
    both PS1 and N64 SHELL data, so use that point for the repaired entry.
    """
    strings = {
        "path": b"Terrain\\DreamLnd.exp\0",
        "subtitle": b"California\0",
        "title": b"Super Dreamland 64\0",
    }
    offsets: dict[str, int] = {}
    for name, value in strings.items():
        offset = shell_data.find(value)
        if offset < 0:
            raise ValueError(f"N64 SHELL is missing Dreamland {name}")
        offsets[name] = offset

    candidates: list[tuple[int, tuple[int, int, int, int, int]]] = []
    for offset in range(0, len(shell_data) - 20 + 1, 4):
        words = struct.unpack_from(">IIIII", shell_data, offset)
        if (
            words[1] == offsets["title"]
            and words[2] == offsets["subtitle"]
            and words[4] == offsets["path"]
        ):
            candidates.append((offset, words))
    if len(candidates) != 1:
        raise ValueError(
            "expected one native N64 Dreamland selector record, found "
            f"{len(candidates)}"
        )

    record_offset, words = candidates[0]
    model, _title, _subtitle, packed_marker, _path = words
    source_marker_x = packed_marker >> 16
    source_marker_y = packed_marker & 0xFFFF
    if model != 12 or (source_marker_x, source_marker_y) != (270, 182):
        raise ValueError(
            "unexpected native Dreamland selector values: "
            f"model={model} marker=({source_marker_x},{source_marker_y})"
        )

    california_records: list[
        tuple[int, tuple[int, int, int, int, int]]
    ] = []
    for offset in range(0, len(shell_data) - 20 + 1, 4):
        candidate = struct.unpack_from(">IIIII", shell_data, offset)
        if candidate[0] == 7 and candidate[2] == offsets["subtitle"]:
            california_records.append((offset, candidate))
    if len(california_records) != 1:
        raise ValueError(
            "expected one native model-7 California selector record, found "
            f"{len(california_records)}"
        )
    california_offset, california_words = california_records[0]
    california_marker = california_words[3]
    marker_x = california_marker >> 16
    marker_y = california_marker & 0xFFFF
    if (marker_x, marker_y) != (84, 222):
        raise ValueError(
            "unexpected native California marker: "
            f"({marker_x},{marker_y})"
        )

    return {
        "source": "N64 SHELL.DLL native selector record",
        "source_record_offset": record_offset,
        "native_model_index": model,
        "title": strings["title"][:-1].decode("ascii"),
        "subtitle": strings["subtitle"][:-1].decode("ascii"),
        "source_marker": {
            "x": source_marker_x,
            "y": source_marker_y,
        },
        "marker": {"x": marker_x, "y": marker_y},
        "marker_basis": (
            "N64 model-7 California selector record "
            f"at decoded SHELL offset 0x{california_offset:X}"
        ),
        "source_marker_note": (
            "The shipped Dreamland record duplicates Ski Resort's Colorado "
            "point despite naming California; display marker repaired from "
            "the shared native California record."
        ),
        "location_path": strings["path"][:-1].decode("ascii"),
    }


def build_location_bank(
    retail_data: bytes, n64_data: bytes
) -> tuple[bytes, dict[str, object]]:
    retail = _decode_bank(retail_data)
    retail_shape = _bank_shape(retail)
    if retail_shape == {
        "groups": 13,
        "slots": 13,
        "collisions": 13,
        "textures": 13,
        "animations": 0,
    }:
        retail, root_map = conversion.extract_roots(retail, set(range(12)))
        if root_map != {index: index for index in range(12)}:
            raise ValueError(
                f"could not recover retail roots from merged bank: {root_map}"
            )
    elif retail_shape != {
        "groups": 12,
        "slots": 12,
        "collisions": 12,
        "textures": 12,
        "animations": 0,
    }:
        raise ValueError(
            f"retail location bank has unexpected shape: {retail_shape}"
        )

    converted, conversion_report, _palettes = convert_xobf_bin(n64_data)
    n64 = _decode_bank(converted)
    dream, root_map = conversion.extract_roots(n64, {12})
    if root_map != {12: 0}:
        raise ValueError(f"unexpected Dreamland root map {root_map}")
    if _bank_shape(dream) != {
        "groups": 1,
        "slots": 1,
        "collisions": 1,
        "textures": 1,
        "animations": 0,
    }:
        raise ValueError(
            f"Dreamland location icon is not independent: {_bank_shape(dream)}"
        )

    merged, slot_bases = conversion.merge_banks((retail, dream))
    if slot_bases != (0, 12):
        raise ValueError(f"Dreamland did not append as native slot 12: {slot_bases}")
    compiled = _compile_bank(merged)
    reparsed = _decode_bank(compiled)

    expected_shape = {
        "groups": 13,
        "slots": 13,
        "collisions": 13,
        "textures": 13,
        "animations": 0,
    }
    if _bank_shape(reparsed) != expected_shape:
        raise ValueError(
            f"compiled location bank has wrong shape: {_bank_shape(reparsed)}"
        )
    if (
        reparsed.groups[:12] != retail.groups
        or reparsed.slots[:12] != retail.slots
        or reparsed.collisions[:12] != retail.collisions
        or reparsed.textures[:12] != retail.textures
    ):
        raise ValueError("one or more retail location-wheel objects changed")

    return compiled, {
        "n64_converted_bank": asdict(conversion_report),
        "dreamland_dependency_closure": _bank_shape(dream),
        "compiled_bank": _bank_shape(reparsed),
        "retail_objects_preserved": 12,
        "dreamland_native_model_index": 12,
        "source_sha256": _digest(n64_data),
        "output_sha256": _digest(compiled),
    }


def _trim_sbs_frame(data: bytes) -> bytes:
    # psxavenc's SBS container pads each encoded frame with zeroes to the
    # requested alignment. A BS frame itself is four-byte aligned and ends in
    # non-zero VLC termination bits, so discard only container padding.
    end = len(data.rstrip(b"\0"))
    end = (end + 3) & ~3
    frame = data[:end]
    if len(frame) < 8 or frame[2:4] != b"\x00\x38":
        raise ValueError("psxavenc output is not a PlayStation BS frame")
    if frame[6:8] != b"\x02\x00":
        raise ValueError("psxavenc output is not MDEC BS version 2")
    return frame


def build_loading_chunk(
    n64_xlsc: bytes, psxavenc: Path, preview_path: Path
) -> tuple[bytes, dict[str, object]]:
    if len(n64_xlsc) < 8 or n64_xlsc[4:6] != b"\xFF\xD8":
        raise ValueError("Dreamland XLSC does not contain its native JPEG")
    source = Image.open(BytesIO(n64_xlsc[4:])).convert("RGB")
    if source.size != (320, 100):
        raise ValueError(f"unexpected Dreamland loading art size {source.size}")

    canvas = Image.new("RGB", (320, 112), (0, 0, 0))
    canvas.paste(source, (0, 6))
    preview_path.parent.mkdir(parents=True, exist_ok=True)
    canvas.save(preview_path)

    with tempfile.TemporaryDirectory(
        prefix="v8_dreamland_mdec_", dir=preview_path.parent
    ) as temp_name:
        temp = Path(temp_name)
        # The official Windows psxavenc build's FFmpeg input set accepts BMP
        # consistently, while some builds omit the standalone PNG demuxer.
        bitmap = temp / "dreamland_load_320x112.bmp"
        sbs = temp / "dreamland_load.sbs"
        canvas.save(bitmap)
        subprocess.run(
            [
                str(psxavenc),
                "-q",
                "-t",
                "sbs",
                "-v",
                "v2",
                "-s",
                "320x112",
                "-I",
                "-a",
                "32768",
                str(bitmap),
                str(sbs),
            ],
            check=True,
        )
        bs = _trim_sbs_frame(sbs.read_bytes())

    payload = n64_xlsc[:4] + struct.pack("<HH", 320, 112) + bs
    return payload, {
        "source_dimensions": [320, 100],
        "native_ps1_dimensions": [320, 112],
        "letterbox_rows": {"top": 6, "bottom": 6},
        "codec": "PlayStation MDEC BS v2",
        "mdec_bytes": len(bs),
        "xlsc_bytes": len(payload),
        "output_sha256": _digest(payload),
    }


def install_loading_chunk(exp_data: bytes, xlsc_payload: bytes) -> bytes:
    children = [child for child in root_children(exp_data) if child.tag != b"XLSC"]
    encoded = [Chunk(tag=b"XLSC", payload=xlsc_payload).encode()]
    encoded.extend(child.encode() for child in children)
    result = iff_form(b"TERR", encoded)
    reparsed = root_children(result)
    if reparsed[0].tag != b"XLSC" or reparsed[0].payload != xlsc_payload:
        raise ValueError("native Dreamland loading chunk did not round-trip")
    return result


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--rom", type=Path, default=ROOT / "Vigilante 8 (U) (!).n64"
    )
    parser.add_argument(
        "--retail-locations",
        type=Path,
        default=ROOT / "PS1 game" / "SHELL" / "LOCATNS.BIN",
    )
    parser.add_argument(
        "--dreamland-exp",
        type=Path,
        default=ROOT / "PS1 game" / "TERRAIN" / "DREAMLND.EXP",
    )
    parser.add_argument(
        "--output-locations",
        type=Path,
        default=ROOT / "PS1 game" / "SHELL" / "LOCATNS.BIN",
    )
    parser.add_argument(
        "--output-exp",
        type=Path,
        default=ROOT / "PS1 game" / "TERRAIN" / "DREAMLND.EXP",
    )
    parser.add_argument("--psxavenc", type=Path, required=True)
    parser.add_argument(
        "--artifacts",
        type=Path,
        default=ROOT / "artifacts" / "n64_arena_port" / "native_shell_ui",
    )
    args = parser.parse_args()

    rom = V8N64Rom(args.rom)
    native_selector = decode_n64_location_record(
        rom.decoded("SHELL.DLL")
    )
    locations_data = rom.decoded("LOCATNS.N64")
    dreamland_source = rom.decoded("DREAMLND.EXP")
    source_loading = [
        child.payload
        for child in root_children(dreamland_source)
        if child.tag == b"XLSC"
    ]
    if len(source_loading) != 1:
        raise ValueError(
            f"expected one N64 Dreamland loading image, found {len(source_loading)}"
        )

    output_locations, location_report = build_location_bank(
        args.retail_locations.read_bytes(), locations_data
    )
    preview = args.artifacts / "dreamland_loading_art_320x112.png"
    xlsc, loading_report = build_loading_chunk(
        source_loading[0], args.psxavenc.resolve(), preview
    )
    output_exp = install_loading_chunk(args.dreamland_exp.read_bytes(), xlsc)

    args.output_locations.parent.mkdir(parents=True, exist_ok=True)
    args.output_exp.parent.mkdir(parents=True, exist_ok=True)
    args.artifacts.mkdir(parents=True, exist_ok=True)
    args.output_locations.write_bytes(output_locations)
    args.output_exp.write_bytes(output_exp)
    report = {
        "format": "v8-dreamland-native-shell-ui-v1",
        "source": {
            "rom": str(args.rom.resolve()),
            "rom_sha256": _digest(args.rom.read_bytes()),
        },
        "location_selector": {
            "native_index": 10,
            **native_selector,
            **location_report,
        },
        "loading_screen": loading_report,
        "outputs": {
            "locations": str(args.output_locations.resolve()),
            "locations_sha256": _digest(output_locations),
            "dreamland_exp": str(args.output_exp.resolve()),
            "dreamland_exp_sha256": _digest(output_exp),
        },
    }
    (args.artifacts / "native_shell_ui_report.json").write_text(
        json.dumps(report, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps(report, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
