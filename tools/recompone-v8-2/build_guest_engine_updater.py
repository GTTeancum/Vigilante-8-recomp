#!/usr/bin/env python3
"""Bundle the optional, source-only engine-roster updater (no media or build outputs)."""
from __future__ import annotations

import argparse
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile, ZipInfo


TOOLS = Path(__file__).resolve().parents[1]
PACKAGE = "vigilante8_vehicle_tools"
MODULES = ("__init__", "binary", "compiler", "guest_audio", "iff", "project", "registry", "stats", "xobf")


def build(output: Path) -> None:
    sources = {
        "__main__.py": TOOLS / "recompone-v8-2/upgrade_guest_engine_audio.py",
        "extract_psx_iso.py": TOOLS / "extract_psx_iso.py",
        **{f"{PACKAGE}/{module}.py": TOOLS / "blender_addons" / PACKAGE / f"{module}.py"
           for module in MODULES},
    }
    # Source text only, deterministic order, metadata and line endings.
    content = {name: path.read_text(encoding="utf-8").encode("utf-8") for name, path in sources.items()}
    with ZipFile(output, "x") as archive:
        for name, data in content.items():
            info = ZipInfo(name, (1980, 1, 1, 0, 0, 0))
            info.create_system = 3
            info.external_attr = 0o100644 << 16
            info.compress_type = ZIP_DEFLATED
            archive.writestr(info, data)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path, help="new .pyz file (never overwrites)")
    args = parser.parse_args()
    build(args.output)
