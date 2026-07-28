#!/usr/bin/env python3
"""Apply durable custom-vehicle seams to generated V8:2 engine code.

RecompOne supports whole-function hooks, but these sites need decoded pointer
selection inside two retail functions. The replacements preserve the retail
path and select independent custom stats, upgrade state, transformation table,
transformation bank, and wheel bank only for registered custom vehicle objects.
"""

from __future__ import annotations

import argparse
from pathlib import Path


REPO = Path(__file__).resolve().parents[2]
DEFAULT_SOURCE = REPO / "reference-v8-2" / "generated" / "recompiled" / "main.cs"

REPLACEMENTS = (
    (
        """        c.A0 = m.ReadU32((c.T3 + 0x48u));
        c.A3 = 0x00000008u;
""",
        """        c.A0 = m.ReadU32((c.T3 + 0x48u));
        c.A0 = RecompOne.Runtime.Sdk.V82VehicleRegistry.WheelBankForObject(m, c.S4, c.A0);
        c.A3 = 0x00000008u;
""",
    ),
    (
        """        c.V1 = m.ReadU32((c.T4 + 0x48u));
        c.A0 = c.A0 - c.S0;
""",
        """        c.V1 = m.ReadU32((c.T4 + 0x48u));
        c.V1 = RecompOne.Runtime.Sdk.V82VehicleRegistry.WheelBankForObject(m, c.S4, c.V1);
        c.A0 = c.A0 - c.S0;
""",
    ),
    (
        """        c.V0 = c.V0 + c.V1;
        c.A0 = c.A0 + c.V0;
        c.V0 = 0u | 0xFFFFu;
""",
        """        c.V0 = c.V0 + c.V1;
        c.A0 = c.A0 + c.V0;
        c.A0 = RecompOne.Runtime.Sdk.V82VehicleRegistry.UpgradeStatusForObject(c, m, c.S5, c.A0);
        c.V0 = 0u | 0xFFFFu;
""",
    ),
    (
        """        c.V0 = c.V0 + 0x3A80u;
        c.A1 = c.A1 + c.V0;
        c.V1 = m.ReadU8((c.A1 + 0x14u));
""",
        """        c.V0 = c.V0 + 0x3A80u;
        c.A1 = c.A1 + c.V0;
        c.A1 = RecompOne.Runtime.Sdk.V82VehicleRegistry.StatsPointerForObject(m, c.S5, c.A1);
        c.V1 = m.ReadU8((c.A1 + 0x14u));
""",
    ),
    (
        """        c.A0 = 0x80060000u;
        c.A0 = c.A0 + 0x3F74u;
        c.V1 = c.S6 << 1;
        c.S3 = m.ReadU32((c.V0 + 0x61C0u));
""",
        """        c.A0 = 0x80060000u;
        c.A0 = c.A0 + 0x3F74u;
        c.A0 = RecompOne.Runtime.Sdk.V82VehicleRegistry.TransformTableForObject(m, c.S5, c.A0);
        c.V1 = c.S6 << 1;
        c.S3 = m.ReadU32((c.V0 + 0x61C0u));
        c.S3 = RecompOne.Runtime.Sdk.V82VehicleRegistry.TransformBankForObject(m, c.S5, c.S3);
""",
    ),
    (
        """        c.V0 = c.V0 << 2;
        c.A0 = c.A0 + c.V0;
        c.A0 = c.A0 + c.A1;
        c.V0 = m.ReadU8((c.A0 + 0x7u));
""",
        """        c.V0 = c.V0 << 2;
        c.A0 = c.A0 + c.V0;
        c.A0 = c.A0 + c.A1;
        c.A0 = RecompOne.Runtime.Sdk.V82VehicleRegistry.UpgradeStatusForObject(c, m, c.S5, c.A0);
        c.V0 = m.ReadU8((c.A0 + 0x7u));
""",
    ),
)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    args = parser.parse_args()

    source = args.source.resolve()
    text = source.read_text(encoding="utf-8")
    changed = 0
    for old, new in REPLACEMENTS:
        if new in text:
            continue
        count = text.count(old)
        if count != 1:
            raise RuntimeError(
                f"expected exactly one generated engine seam, found {count}: "
                f"{old.splitlines()[0].strip()}"
            )
        text = text.replace(old, new, 1)
        changed += 1

    if changed:
        source.write_text(text, encoding="utf-8")
    print(f"V8:2 custom-vehicle engine seams ready ({changed} applied)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
