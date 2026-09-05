#!/usr/bin/env python3
"""Convert the original V8 tire cursor into V8:2's native model dialect."""

from __future__ import annotations

import argparse
from dataclasses import replace
import hashlib
import json
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[2]
ADDONS = ROOT / "tools" / "blender_addons"
if str(ADDONS) not in sys.path:
    sys.path.insert(0, str(ADDONS))

from vigilante8_vehicle_tools import (  # noqa: E402
    compiler,
    conversion,
    iff,
    project,
    registry,
)


def decode_model(data: bytes, game: str) -> project.ObjectBank:
    form = iff.IffChunk(
        tag=b"FORM",
        form_type=b"XOBF",
        children=[iff.IffChunk(tag=b"BIN ", payload=data)],
    )
    return registry._decode_bank(form, game)


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--source",
        type=Path,
        default=ROOT / "PS1 game" / "SHELL" / "CURSOR.BIN",
        help="original V8 CURSOR.BIN",
    )
    parser.add_argument(
        "--target-template",
        type=Path,
        default=ROOT / "V8_2_LOOSE" / "SHELL" / "CURSOR.PSX",
        help="retail V8:2 cursor whose shared shell slot contract is preserved",
    )
    parser.add_argument(
        "--template-archive",
        type=Path,
        default=ROOT / "V8_2_LOOSE" / "CUSTOM.EXP",
        help="a valid V8:2 authored package used only for compiler metadata",
    )
    parser.add_argument(
        "--template-registry",
        type=Path,
        default=ROOT / "V8_2_LOOSE" / "VEHICLES.V8R",
        help="registry paired with --template-archive",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=(
            ROOT
            / "mods"
            / "v8_classic_menu"
            / "files"
            / "SHELL"
            / "CURSOR.PSX"
        ),
        help="converted V8:2 CURSOR.PSX override",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    source_data = args.source.read_bytes()
    source = decode_model(source_data, "V8")
    converted = conversion.v8_bank_to_v82(source)

    target_data = args.target_template.read_bytes()
    target = decode_model(target_data, "V8_2")
    if len(converted.groups) != 1 or len(converted.collisions) != 1:
        raise ValueError(
            "the original V8 cursor must contain exactly one render group and "
            "one collision stream"
        )
    if not target.slots:
        raise ValueError("the V8:2 cursor must contain at least one shell slot")

    if len(converted.slots) != 1 or len(target.slots) != 2:
        raise ValueError(
            "the V8 cursor must have one root slot and the V8:2 cursor must "
            "have its two native shell roots"
        )

    # V8:2's shared cursor bank serves two unrelated roles: root 0 is the
    # raised marker on the USA map, while root 1 is the front-end smiley
    # selector. Replace only root 1 with the converted V8 tire and retain root
    # 0's full native dependency closure. The selector keeps V8:2's authored root
    # transform/flags so this remains a direct model overwrite rather than a
    # menu-specific runtime route.
    tire, _ = conversion.extract_roots(converted, {0})
    tire = replace(
        tire,
        slots=(
            replace(
                target.slots[1],
                name="slot_000",
                render_group=0,
                collision=0,
                parent=None,
                key=None,
            ),
        ),
        animations=(),
    )
    map_marker, _ = conversion.extract_roots(target, {0})
    converted, slot_bases = conversion.merge_banks((map_marker, tire))
    if slot_bases != (0, 1):
        raise AssertionError(f"unexpected merged cursor roots {slot_bases}")

    templates = registry.decompile_package(
        args.template_archive.read_bytes(),
        args.template_registry.read_bytes(),
    )
    if not templates:
        raise ValueError("V8:2 template package contains no vehicles")
    # compile_model uses the template only to validate the V8:2 project
    # contract and select its packet dialect. The emitted model is wholly the
    # decoded-and-converted cursor bank passed as the second argument.
    output = compiler.compile_model(templates[0], converted)
    reparsed = decode_model(output, "V8_2")
    if project._bank_to_dict(reparsed) != project._bank_to_dict(converted):
        raise AssertionError("converted cursor changed during V8:2 round trip")
    reparsed_marker, _ = conversion.extract_roots(reparsed, {0})
    if project._bank_to_dict(reparsed_marker) != project._bank_to_dict(map_marker):
        raise AssertionError("native V8:2 USA-map marker changed")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(output)
    report = {
        "source": str(args.source),
        "source_sha256": sha256(source_data),
        "target_template": str(args.target_template),
        "target_template_sha256": sha256(target_data),
        "output": str(args.output),
        "output_sha256": sha256(output),
        "output_bytes": len(output),
        "groups": len(reparsed.groups),
        "slots": len(reparsed.slots),
        "collisions": len(reparsed.collisions),
        "textures": len(reparsed.textures),
        "faces": sum(len(group.faces) for group in reparsed.groups),
        "selector_root": "converted original V8 tire",
        "usa_map_root": "preserved native V8:2 marker",
    }
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
