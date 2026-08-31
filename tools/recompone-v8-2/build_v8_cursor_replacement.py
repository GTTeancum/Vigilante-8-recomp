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

    # The V8 shell cursor contains one tire object, while V8:2's shared shell
    # code indexes two root selector objects.  Copying the one-root V8 bank
    # directly therefore makes the shell follow an invalid second slot.  Keep
    # the target file's complete slot layout and transforms, but make every
    # selector slot reference its own native copy of the converted tire.  This
    # is still a pure shared-asset overwrite: no menu-specific runtime route is
    # needed, and both selector roles receive the same canonical V8 model.
    converted = project.ObjectBank(
        groups=tuple(
            replace(converted.groups[0], name=f"group_{index:03d}")
            for index in range(len(target.slots))
        ),
        slots=tuple(
            replace(
                slot,
                name=f"slot_{index:03d}",
                render_group=index,
                collision=index,
                key=None,
            )
            for index, slot in enumerate(target.slots)
        ),
        collisions=tuple(
            replace(converted.collisions[0], name=f"collision_{index:03d}")
            for index in range(len(target.slots))
        ),
        textures=converted.textures,
        animations=(),
    )

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
    }
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
