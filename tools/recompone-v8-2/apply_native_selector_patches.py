#!/usr/bin/env python3
"""Apply durable native selector seams to generated V8:2 code.

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
        c.V0 = RecompOne.Runtime.Sdk.V82ArenaRegistry.NativeSelectedLocationRecord(m, c.V0);
        c.LoadWord(2, m, (c.V0 + 0xCu));
        c.V0 = RecompOne.Runtime.Sdk.V82ArenaRegistry.ResolveLaunchPath(c, m, c.V0);
        c.SP = c.SP + 0x78u;
""",
    ),
    (
        """        c.S0 = c.S4 << 4;
        c.S0 = c.S0 + c.V0;
        c.V0 = m.ReadU16((c.S0 + 0x8u));
""",
        """        c.S0 = c.S4 << 4;
        c.S0 = c.S0 + c.V0;
        c.S0 = RecompOne.Runtime.Sdk.V82ArenaRegistry.NativeLocationRecordAddress(m, c.S4, c.S0);
        c.V0 = m.ReadU16((c.S0 + 0x8u));
""",
    ),
    (
        """        c.V1 = c.S0 << 4;
        c.S4 = c.V1 + c.V0;
        c.V0 = m.ReadU16((c.S4 + 0x8u));
""",
        """        c.V1 = c.S0 << 4;
        c.S4 = c.V1 + c.V0;
        c.S4 = RecompOne.Runtime.Sdk.V82ArenaRegistry.NativeLocationRecordAddress(m, c.S0, c.S4);
        c.V0 = m.ReadU16((c.S4 + 0x8u));
""",
    ),
    # The vehicle carousel slot seam at L80106800 is no longer here: it is declared in
    # prepare_reference.py as an inline patch on ApplyNativeSelectorSlot, which
    # the recompiler emits directly. Two mechanisms writing the same line meant
    # neither anchor matched after a regeneration.
    (
        """        c.LoadWord(4, m, (c.S7 + 0x4Cu));
        c.T3 = c.SP + 0x48u;
""",
        """        c.A0 = RecompOne.Runtime.Sdk.V82ArenaRegistry.NativeLocationBackgroundOffset(m, c.S7);
        c.T3 = c.SP + 0x48u;
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

OBSOLETE_DREAMLAND_REPLACEMENTS = (
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
    (
        """        c.LoadWord(16, m, (c.SP + 0x30u));
        c.SP = c.SP + 0x40u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void func_8004DC20(CpuContext c, IMemory m)
""",
        """        c.LoadWord(16, m, (c.SP + 0x30u));
        c.SP = c.SP + 0x40u;
        RecompOne.Runtime.Sdk.V82Compat.TraceNativeModelLifecycleEnd(c, m);
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void func_8004DC20(CpuContext c, IMemory m)
""",
    ),
)

MAIN_MIGRATIONS = tuple(
    (compatibility, native)
    for native, compatibility in OBSOLETE_DREAMLAND_REPLACEMENTS
)

OBSOLETE_MAIN_DIAGNOSTICS = (
    (
        """        c.RA = 0x8002BFCCu;
        Vigilante82PC.func_8001FD18(c, m);
        c.StoreWord(2, m, (c.S0 + 0x40u));
""",
        """        c.RA = 0x8002BFCCu;
        Vigilante82PC.func_8001FD18(c, m);
        c.StoreWord(2, m, (c.S0 + 0x40u));
        RecompOne.Runtime.Sdk.V82Compat.TraceNativeObjectModelAssigned(c, m);
""",
    ),
    (
        """    public static void func_8002D1DC(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x18u;
""",
        """    public static void func_8002D1DC(CpuContext c, IMemory m)
    {
        RecompOne.Runtime.Sdk.V82Compat.TraceNativeObjectExtent(c, m);
        c.SP = c.SP - 0x18u;
""",
    ),
    (
        """    public static void func_8001FD18(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x18u;
""",
        """    public static void func_8001FD18(CpuContext c, IMemory m)
    {
        RecompOne.Runtime.Sdk.V82Compat.TraceNativeModelResolveBegin(c, m);
        c.SP = c.SP - 0x18u;
""",
    ),
    (
        """        c.LoadWord(16, m, (c.SP + 0x10u));
        c.SP = c.SP + 0x18u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void func_8001FE58(CpuContext c, IMemory m)
""",
        """        c.LoadWord(16, m, (c.SP + 0x10u));
        c.SP = c.SP + 0x18u;
        RecompOne.Runtime.Sdk.V82Compat.TraceNativeModelResolveEnd(c, m);
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void func_8001FE58(CpuContext c, IMemory m)
""",
    ),
    (
        """    public static void func_8001FEB8(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x8u;
""",
        """    public static void func_8001FEB8(CpuContext c, IMemory m)
    {
        RecompOne.Runtime.Sdk.V82Compat.TraceNativeModelRelease(c, m);
        c.SP = c.SP - 0x8u;
""",
    ),
    (
        """    public static void func_8004DB00(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x40u;
""",
        """    public static void func_8004DB00(CpuContext c, IMemory m)
    {
        RecompOne.Runtime.Sdk.V82Compat.TraceNativeModelLifecycle(c, m);
        c.SP = c.SP - 0x40u;
""",
    ),
)

# These model-lifecycle probes isolated the malformed converted hierarchy and
# are intentionally absent from production builds now that the converter emits
# native links. Keep only the cleanup patterns so an already-instrumented
# generated tree is normalized on the next patch pass.
MAIN_DIAGNOSTICS: tuple[tuple[str, str], ...] = ()

SOURCE_MIGRATIONS = (
    (
        """        c.LoadWord(4, m, (c.S7 + 0x4Cu));
        c.T3 = c.SP + 0x48u;
""",
        """        c.A0 = RecompOne.Runtime.Sdk.V82ArenaRegistry.NativeLocationBackgroundOffset(m, c.S7);
        c.T3 = c.SP + 0x48u;
""",
    ),
    (
        """        c.A3 = 0x0000002Du;
        c.V0 = RecompOne.Runtime.Sdk.V82ArenaRegistry.NativeLocationArtIndex(c.S4) << 2;
        c.V0 = c.S7 + c.V0;
""",
        """        c.A3 = 0x0000002Du;
        c.V0 = c.S4 << 2;
        c.V0 = c.S7 + c.V0;
""",
    ),
    (
        """        c.A3 = 0x0000002Du;
        c.V0 = RecompOne.Runtime.Sdk.V82ArenaRegistry.ResolveNativeLocationArt(c, m, c.S4, c.S7);
""",
        """        c.A3 = 0x0000002Du;
        c.V0 = c.S4 << 2;
        c.V0 = c.S7 + c.V0;
""",
    ),
    (
        """        c.A3 = 0x0000002Du;
        RecompOne.Runtime.Sdk.V82ArenaRegistry.TrackNativeLocationHighlight(c.S4);
        c.V0 = RecompOne.Runtime.Sdk.V82ArenaRegistry.NativeLocationArtIndex(c.S4) << 2;
        c.V0 = c.S7 + c.V0;
""",
        """        c.A3 = 0x0000002Du;
        c.V0 = c.S4 << 2;
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


def remove_obsolete_main_diagnostics(source: Path) -> int:
    text = source.read_text(encoding="utf-8")
    changed = 0
    obsolete = (*OBSOLETE_MAIN_DIAGNOSTICS, *OBSOLETE_DREAMLAND_REPLACEMENTS)
    for native, instrumented in obsolete:
        if instrumented in text:
            text = text.replace(instrumented, native, 1)
            changed += 1
    lifecycle_end = (
        "        RecompOne.Runtime.Sdk.V82Compat."
        "TraceNativeModelLifecycleEnd(c, m);\n"
    )
    if lifecycle_end in text:
        text = text.replace(lifecycle_end, "")
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
    changed = remove_obsolete_main_diagnostics(main_source)
    changed += apply_replacements(
        source,
        REPLACEMENTS,
        "native-selector",
        SOURCE_MIGRATIONS,
    )
    changed += apply_replacements(
        main_source,
        MAIN_DIAGNOSTICS,
        "native main callback",
        MAIN_MIGRATIONS,
    )
    print(f"V8:2 native selector seams ready ({changed} applied)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
