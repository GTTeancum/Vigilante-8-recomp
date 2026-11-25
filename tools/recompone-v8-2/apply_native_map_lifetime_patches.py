#!/usr/bin/env python3
"""Keep map-specific lifetime repairs when rebuilding generated native overlays."""
import argparse
from pathlib import Path

DEFAULT_ROOT = Path(__file__).resolve().parents[2] / "reference-v8-2/generated/recompiled"

OIL_SEAM = '        c.RA = 0x80100198u;\n        Vigilante82PC.func_8004EEB0(c, m);'
OIL_PATCH = '        c.RA = 0x80100198u;\n        // Pool actors can return through a core callback with base memory.\n        // Publish the actual arena-owned head address into each actor then.\n        c.A0 = Dispatcher.ResolveLinkedAddress(m, c.A0);\n        Vigilante82PC.func_8004EEB0(c, m);'
VALLEY_SEAM = '    public static void func_8010056C(CpuContext c, IMemory m)\n    {\n        c.SP = c.SP - 0x10u;'
VALLEY_PATCH = '    public static void func_8010056C(CpuContext c, IMemory m)\n    {\n        // Original event 4 dispatched by native object retirement. Test it\n        // before native helpers reuse A1 on their way to the shared epilogue.\n        if (c.A1 == 4u && c.RA == 0x80030724u &&\n            c.A0 != 0u && c.A0 == m.ReadU32(c.GP + 0x1024u))\n        {\n            uint owner = c.A0;\n            uint field = owner + 0x88u;\n            uint table = m.ReadU32(field);\n            if (table != 0u &&\n                RecompOne.Runtime.Sdk.V82Compat.IsPcAllocationLive(table))\n            {\n                m.WriteU32(field, 0u);\n                c.A0 = table;\n                Vigilante82PC.func_800520D8(c, m);\n                c.A0 = owner;\n            }\n        }\n        c.SP = c.SP - 0x10u;'

PATCHES = {
    "LEVELS_V8_OILFIELD.cs": (OIL_SEAM, OIL_PATCH),
    "LEVELS_V8_VALLYFRM.cs": (VALLEY_SEAM, VALLEY_PATCH),
}

def patched_text(path, text):
    original, replacement = PATCHES[path.name]
    count = text.count(replacement)
    if count == 1 and text.count(original) == int(original in replacement):
        return text
    if count != 0 or text.count(original) != 1:
        raise ValueError(f"{path}: unknown or duplicate map lifetime seam")
    return text.replace(original, replacement, 1)

def apply(source_root):
    updates = {}
    for file_name in PATCHES:
        path = source_root / file_name
        if not path.is_file():
            continue
        text = path.read_text(encoding="utf-8")
        output = patched_text(path, text)
        if output != text:
            updates[path] = output
    for path, output in updates.items():
        path.write_text(output, encoding="utf-8")
    return len(updates)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", type=Path, default=DEFAULT_ROOT)
    args = parser.parse_args()
    print(f"Native map lifetime seams ready ({apply(args.source_root)} applied)")
