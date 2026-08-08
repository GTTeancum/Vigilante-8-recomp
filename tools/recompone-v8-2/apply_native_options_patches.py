#!/usr/bin/env python3
"""Apply durable seams for the in-shell V8:2 PC controls page."""

from __future__ import annotations

import argparse
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_SHELL = (
    ROOT / "reference-v8-2" / "generated" / "recompiled" /
    "SHELL_SHELL.cs"
)
DEFAULT_MAIN = (
    ROOT / "reference-v8-2" / "generated" / "recompiled" / "main.cs"
)

SHELL_OLD = """        c.CopyRegister(4, 19);
        c.RA = 0x8010A8ECu;
        Vigilante82PC.func_80019320(c, m);
        c.T3 = 0x80110000u;
        c.LoadWord(4, m, (c.T3 + 0x6730u));
"""
SHELL_EARLY_SEAM = """        c.CopyRegister(4, 19);
        c.RA = 0x8010A8ECu;
        Vigilante82PC.func_80019320(c, m);
        if (V82NativeControlOptions.TryDraw(c, m))
            goto L8010B1EC;
        c.T3 = 0x80110000u;
        c.LoadWord(4, m, (c.T3 + 0x6730u));
"""

SHELL_LOOP_OLD = """        L8010AE8C: ;
        c.LoadWord(16, m, (c.S7 + 0x10u));
"""
SHELL_LOOP_NEW = """        L8010AE8C: ;
        if (V82NativeControlOptions.TryDraw(c, m))
            goto L8010B1EC;
        c.LoadWord(16, m, (c.S7 + 0x10u));
"""

SHELL_FRAME_OLD = """        c.RA = 0x8010B210u;
        Vigilante82PC.func_80015540(c, m);
        c.LoadWord(12, m, (c.SP + 0xF0u));
"""
SHELL_FRAME_NEW = """        c.RA = 0x8010B210u;
        Vigilante82PC.func_80015540(c, m);
        V82NativeControlOptions.UpdateState(c, m);
        c.LoadWord(12, m, (c.SP + 0xF0u));
"""

LEGACY_FRAME_SEAM = """        L8010B1EC: ;
        V82NativeControlOptions.UpdateState(c, m);
        c.CopyRegister(4, 0);
"""
RETAIL_FRAME_SEAM = """        L8010B1EC: ;
        c.CopyRegister(4, 0);
"""

MAIN_OLD = """    public static void func_8001A3B0(CpuContext c, IMemory m)
    {
        if (!RecompOne.Runtime.Context.PreHook.Run(RecompOne.Runtime.Sdk.V82VehicleRegistry.OverrideNativeSelectorText, c, m)) return;
"""
MAIN_NEW = """    public static void func_8001A3B0(CpuContext c, IMemory m)
    {
        if (!RecompOne.Runtime.Context.PreHook.Run(RecompOne.Runtime.Sdk.V82VehicleRegistry.OverrideNativeSelectorText, c, m)) return;
        if (!RecompOne.Runtime.Context.PreHook.Run(RecompOne.Runtime.Sdk.V82Compat.TraceNativeOptionsText, c, m)) return;
"""


def patch_once(path: Path, old: str, new: str, label: str) -> int:
    text = path.read_text(encoding="utf-8")
    if new in text:
        return 0
    count = text.count(old)
    if count != 1:
        raise RuntimeError(
            f"expected exactly one {label} seam in {path}, found {count}"
        )
    path.write_text(text.replace(old, new, 1), encoding="utf-8")
    return 1


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--shell-source", type=Path, default=DEFAULT_SHELL)
    parser.add_argument("--main-source", type=Path, default=DEFAULT_MAIN)
    args = parser.parse_args()

    shell_source = args.shell_source.resolve()
    shell_text = shell_source.read_text(encoding="utf-8")
    if SHELL_EARLY_SEAM in shell_text:
        shell_source.write_text(
            shell_text.replace(SHELL_EARLY_SEAM, SHELL_OLD, 1),
            encoding="utf-8",
        )
        shell_text = shell_source.read_text(encoding="utf-8")
    if LEGACY_FRAME_SEAM in shell_text:
        shell_source.write_text(
            shell_text.replace(LEGACY_FRAME_SEAM, RETAIL_FRAME_SEAM, 1),
            encoding="utf-8",
        )

    changed = patch_once(
        shell_source, SHELL_LOOP_OLD, SHELL_LOOP_NEW,
        "native controller-loop",
    )
    changed += patch_once(
        shell_source, SHELL_FRAME_OLD, SHELL_FRAME_NEW,
        "native controller-frame",
    )
    changed += patch_once(
        args.main_source.resolve(), MAIN_OLD, MAIN_NEW,
        "native options text trace",
    )
    print(f"V8:2 native options seams ready ({changed} applied)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
