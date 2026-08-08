#!/usr/bin/env python3
"""Apply the one in-shell V8:2 options seam the manifest cannot express.

The controls page's two shell seams -- V82NativeControlOptions.TryDraw at
L8010AE8C and UpdateState after the frame call at 0x8010B210 -- are declared
as inline patches in prepare_reference.py and emitted by the recompiler, so
they are no longer applied here; two mechanisms writing the same line left
neither anchor matching after a regeneration.

What remains is the second pre-hook on func_8001A3B0. PreHookTarget is a
single field, so the manifest can attach only one pre-hook to a function, and
that slot already holds OverrideNativeSelectorText.
"""

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

    changed = patch_once(
        args.main_source.resolve(), MAIN_OLD, MAIN_NEW,
        "native options text trace",
    )
    print(f"V8:2 native options seams ready ({changed} applied)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
