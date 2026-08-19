#!/usr/bin/env python3
"""Apply durable native selector and Dreamland seams to generated V8:2 code.

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
DEFAULT_MAIN_SOURCE = (
    ROOT / "reference-v8-2" / "generated" / "recompiled" / "main.cs"
)

REPLACEMENTS = (
    (
        """        c.S3 = 0x00000012u;
        L801045E0: ;
        c.LoadWord(8, m, (c.SP + 0xB0u));
""",
        """        c.S3 = 0x00000012u;
        L801045E0: ;
        c.S3 = RecompOne.Runtime.Sdk.V82ArenaRegistry.BeginNativeLocationSelector(c, m, c.S3);
        c.LoadWord(8, m, (c.SP + 0xB0u));
""",
    ),
    (
        """        c.CopyRegister(2, 20);
        c.LoadWord(31, m, (c.SP + 0xACu));
""",
        """        c.CopyRegister(2, 20);
        RecompOne.Runtime.Sdk.V82ArenaRegistry.EndNativeLocationSelector(c, m);
        c.LoadWord(31, m, (c.SP + 0xACu));
""",
    ),
    (
        """        c.V0 = c.V0 + c.V1;
        c.LoadWord(2, m, (c.V0 + 0xCu));
        c.SP = c.SP + 0x78u;
""",
        """        c.V0 = c.V0 + c.V1;
        c.LoadWord(2, m, (c.V0 + 0xCu));
        c.V0 = RecompOne.Runtime.Sdk.V82ArenaRegistry.ResolveLaunchPath(c, m, c.V0);
        c.SP = c.SP + 0x78u;
""",
    ),
    # The vehicle carousel slot seam at L80106800 is no longer here: it is declared in
    # prepare_reference.py as an inline patch on ApplyNativeSelectorSlot, which
    # the recompiler emits directly. Two mechanisms writing the same line meant
    # neither anchor matched after a regeneration.
    (
        """        c.A3 = 0x0000002Du;
        c.V0 = c.S4 << 2;
        c.V0 = c.S7 + c.V0;
""",
        """        c.A3 = 0x0000002Du;
        RecompOne.Runtime.Sdk.V82ArenaRegistry.TrackNativeLocationHighlight(c.S4);
        c.V0 = RecompOne.Runtime.Sdk.V82ArenaRegistry.NativeLocationArtIndex(c.S4) << 2;
        c.V0 = c.S7 + c.V0;
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

MAIN_REPLACEMENTS = (
    (
        """        c.RA = 0x8002DDA0u;
        Dispatcher.Call(c, m, c.V0);
""",
        """        c.RA = 0x8002DDA0u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.DispatchObjectEvent(c, m, c.V0);
""",
    ),
    (
        """        c.RA = 0x8002DDE4u;
        Dispatcher.Call(c, m, c.V0);
""",
        """        c.RA = 0x8002DDE4u;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.DispatchObjectEvent(c, m, c.V0);
""",
    ),
    (
        """        c.A1 = 0x00000005u;
        c.LoadWord(2, m, c.A0);
        c.CopyRegister(6, 19);
        c.RA = 0x8002FCBCu;
        Dispatcher.Call(c, m, c.V0);
""",
        """        c.A1 = 0x00000005u;
        c.LoadWord(2, m, c.A0);
        c.CopyRegister(6, 19);
        c.RA = 0x8002FCBCu;
        RecompOne.Runtime.Sdk.V82DreamlandCompat.DispatchAnimationEvent(c, m, c.V0);
""",
    ),
    (
        """    public static void func_8004DB00(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x40u;
""",
        """    public static void func_8004DB00(CpuContext c, IMemory m)
    {
        if (RecompOne.Runtime.Sdk.V82DreamlandCompat.PreserveAnimatedModel(c, m)) return;
        c.SP = c.SP - 0x40u;
""",
    ),
)

SOURCE_MIGRATIONS = (
    (
        """        c.A3 = 0x0000002Du;
        c.V0 = RecompOne.Runtime.Sdk.V82ArenaRegistry.NativeLocationArtIndex(c.S4) << 2;
        c.V0 = c.S7 + c.V0;
""",
        """        c.A3 = 0x0000002Du;
        RecompOne.Runtime.Sdk.V82ArenaRegistry.TrackNativeLocationHighlight(c.S4);
        c.V0 = RecompOne.Runtime.Sdk.V82ArenaRegistry.NativeLocationArtIndex(c.S4) << 2;
        c.V0 = c.S7 + c.V0;
""",
    ),
    (
        """        c.A3 = 0x0000002Du;
        c.V0 = RecompOne.Runtime.Sdk.V82ArenaRegistry.ResolveNativeLocationArt(c, m, c.S4, c.S7);
""",
        """        c.A3 = 0x0000002Du;
        RecompOne.Runtime.Sdk.V82ArenaRegistry.TrackNativeLocationHighlight(c.S4);
        c.V0 = RecompOne.Runtime.Sdk.V82ArenaRegistry.NativeLocationArtIndex(c.S4) << 2;
        c.V0 = c.S7 + c.V0;
""",
    ),
    (
        """        c.S0 = RecompOne.Runtime.Sdk.V82ArenaRegistry.ResolveNativeLocationArt(c, m, c.S0, c.T3);
        c.RA = 0x80104FA0u;
""",
        """        c.S0 = c.S0 << 2;
        c.RA = 0x80104FA0u;
""",
    ),
    (
        """        c.LoadWord(4, m, (c.SP + 0xB4u));
        c.LoadWord(5, m, (c.S0 + 0x4u));
""",
        """        c.LoadWord(4, m, (c.SP + 0xB4u));
        c.S0 = c.T3 + c.S0;
        c.LoadWord(5, m, (c.S0 + 0x4u));
""",
    ),
    (
        """        c.A1 = RecompOne.Runtime.Sdk.V82ArenaRegistry.ResolveNativeLocationArtBank(c, m, c.S4, c.S7) + c.A1;
        c.RA = 0x80104838u;
""",
        """        c.A1 = c.S7 + c.A1;
        c.RA = 0x80104838u;
""",
    ),
    (
        """        c.A1 = RecompOne.Runtime.Sdk.V82ArenaRegistry.ResolveNativeLocationArtBankForEntry(c, m, c.S0, c.T3) + c.A1;
        c.RA = 0x80104FCCu;
""",
        """        c.A1 = c.T3 + c.A1;
        c.RA = 0x80104FCCu;
""",
    ),
)


def apply_replacements(
    source: Path,
    replacements: tuple[tuple[str, str], ...],
    label: str,
    migrations: tuple[tuple[str, str], ...] = (),
) -> int:
    text = source.read_text(encoding="utf-8")
    changed = 0
    for old, new in migrations:
        if new not in text and old in text:
            text = text.replace(old, new, 1)
            changed += 1
    for old, new in replacements:
        if new in text:
            continue
        count = text.count(old)
        if count != 1:
            raise RuntimeError(
                f"expected exactly one {label} seam, found {count}: "
                f"{old.splitlines()[0].strip()}"
            )
        text = text.replace(old, new, 1)
        changed += 1
    if changed:
        source.write_text(text, encoding="utf-8")
    return changed


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--main-source", type=Path, default=DEFAULT_MAIN_SOURCE)
    args = parser.parse_args()

    source = args.source.resolve()
    main_source = args.main_source.resolve()
    changed = apply_replacements(
        source,
        REPLACEMENTS,
        "native-selector",
        SOURCE_MIGRATIONS,
    )
    changed += apply_replacements(main_source, MAIN_REPLACEMENTS, "Dreamland animation")
    print(f"V8:2 native selector and Dreamland seams ready ({changed} applied)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
