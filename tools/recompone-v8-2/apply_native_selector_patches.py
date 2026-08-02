#!/usr/bin/env python3
"""Apply durable packaged-V8-roster seams inside V8:2's native selector.

RecompOne's regular patches operate on whole functions. These two generated
sites are deliberately narrower: the native carousel's current FP slot and
the native four-row stat-pointer calculation. All drawing, preview animation,
input repeat, transitions, and acceptance logic remain in SHELL.DLL.
"""

from __future__ import annotations

import argparse
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_SOURCE = (
    ROOT / "reference-v8-2" / "generated" / "recompiled" / "SHELL_SHELL.cs"
)

REPLACEMENTS = (
    (
        """        L80106800: ;
        c.RA = 0x80106808u;
""",
        """        L80106800: ;
        c.FP = RecompOne.Runtime.Sdk.V82VehicleRegistry.ResolveNativeSelectorSlot(c, m);
        c.RA = 0x80106808u;
""",
    ),
    (
        """        c.V0 = c.FP + c.V0;
        c.V0 = m.ReadU8(c.V0);
        c.LoadWord(11, m, (c.SP + 0x1E8u));
""",
        """        c.V0 = c.FP + c.V0;
        c.V0 = m.ReadU8(c.V0);
        c.V0 = RecompOne.Runtime.Sdk.V82VehicleRegistry.NativeSelectorVariant(c.V0);
        c.LoadWord(11, m, (c.SP + 0x1E8u));
""",
    ),
    (
        """        c.V0 = c.V0 + c.T2;
        c.V1 = c.SP + 0x80u;
        c.A1 = c.V1 + c.S1;
        c.A0 = m.ReadU8((c.V0 + 0x2Cu));
""",
        """        c.V0 = c.V0 + c.T2;
        c.V0 = RecompOne.Runtime.Sdk.V82VehicleRegistry.NativeSelectorStatsPointer(c, m, c.V0, c.S2);
        c.V1 = c.SP + 0x80u;
        c.A1 = c.V1 + c.S1;
        c.A0 = m.ReadU8((c.V0 + 0x2Cu));
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
                f"expected exactly one native-selector seam, found {count}: "
                f"{old.splitlines()[0].strip()}"
            )
        text = text.replace(old, new, 1)
        changed += 1

    if changed:
        source.write_text(text, encoding="utf-8")
    print(f"V8:2 native packaged-roster selector seams ready ({changed} applied)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
