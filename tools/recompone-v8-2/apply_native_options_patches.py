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

SHELL_TITLE_OLD = """    public static void func_8010EDB0(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x78u;
"""
SHELL_TITLE_NEW = """    public static void func_8010EDB0(CpuContext c, IMemory m)
    {
        RecompOne.Runtime.Sdk.V82Compat.TraceNativeTitleEntry(c, m);
        c.SP = c.SP - 0x78u;
"""

SHELL_PROMPT_OLD = """    public static void func_80103C4C(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x20u;
"""
SHELL_PROMPT_NEW = """    public static void func_80103C4C(CpuContext c, IMemory m)
    {
        RecompOne.Runtime.Sdk.V82Compat.TraceNativeTitlePromptRoutine(c, m);
        c.SP = c.SP - 0x20u;
"""

SHELL_MAIN_MENU_OLD = """    public static void func_80103FA0(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x68u;
"""
SHELL_MAIN_MENU_NEW = """    public static void func_80103FA0(CpuContext c, IMemory m)
    {
        RecompOne.Runtime.Sdk.V82Compat.TraceNativeMainMenuEntry(c, m);
        c.SP = c.SP - 0x68u;
"""

SHELL_MAIN_MENU_EXIT_OLD = """        c.LoadWord(16, m, (c.SP + 0x50u));
        c.SP = c.SP + 0x68u;
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void func_8010458C(CpuContext c, IMemory m)
"""
SHELL_MAIN_MENU_EXIT_NEW = """        c.LoadWord(16, m, (c.SP + 0x50u));
        c.SP = c.SP + 0x68u;
        RecompOne.Runtime.Sdk.V82Compat.TraceNativeMainMenuExit(c, m);
        return;
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void func_8010458C(CpuContext c, IMemory m)
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

    old = "    public static void func_80102F70(CpuContext c, IMemory m)\n    {\n"
    new = old + "        using var controlFooter = RecompOne.Runtime.Sdk.V82ControlPrompts.RewriteFooter(c, m);\n"
    patch_once(args.shell_source.resolve(), old, new, "contextual menu footer")

    old = "        c.RA = 0x801045CCu;\n        Vigilante82PC.func_8001A2B8(c, m);\n"
    new = "        c.RA = 0x801045CCu;\n        using (RecompOne.Runtime.Sdk.V82ControlPrompts.Rewrite(c, m, gameplayPrompt: true))\n            Vigilante82PC.func_8001A2B8(c, m);\n"
    patch_once(args.shell_source.resolve().with_name("SHELL_LOAD.cs"), old, new, "loading tip bindings")

    old = "        L80013A00: ;\n"
    new = old + "        RecompOne.Runtime.Sdk.V82ControlPrompts.EndReadyPrompt();\n"
    patch_once(args.main_source.resolve(), old, new, "ready-prompt input context exit")

    for address in ("8001A3B0", "8001A6BC"):
        # Insert after the existing pre-hooks so stage/context discovery sees
        # original text before physical button labels replace native glyphs.
        path = args.main_source.resolve()
        text = path.read_text(encoding="utf-8")
        start = text.index(f"    public static void func_{address}(CpuContext c, IMemory m)")
        end = text.index("\n    }", start)
        body = text[start:end]
        hook = "        using var controlPrompt = RecompOne.Runtime.Sdk.V82ControlPrompts.Rewrite(c, m);\n"
        if hook not in body:
            marker = "        c.SP = c.SP - "
            offset = body.index(marker)
            updated = body[:offset] + hook + body[offset:]
            path.write_text(text[:start] + updated + text[end:], encoding="utf-8")

    for address, method in (("8001D580", "Refresh"), ("8001D884", "Play")):
        old = f"    public static void func_{address}(CpuContext c, IMemory m)\n    {{\n"
        new = old + f"        if (V82NativeAudio.{method}(c, m)) return;\n"
        patch_once(args.main_source.resolve(), old, new, f"soundtrack {method}")
    old = """        c.V1 = 0x80060000u;
        c.V0 = m.ReadU8((c.GP + 0xC14u));
        c.V1 = c.V1 + 0x39B0u;
        c.V0 = c.V0 << 2;
        c.V0 = c.V0 + c.V1;
        c.LoadWord(5, m, c.V0);
"""
    new = "        if (!V82NativeAudio.TrackTitle(c, m))\n        {\n" + old + "        }\n"
    patch_once(args.main_source.resolve(), old, new, "soundtrack pause title")

    # Main-menu integration remains durable across regenerated shell code.
    old = "        c.SP = c.SP - 0x68u;\n        c.StoreWord(31, m, (c.SP + 0x74u));\n"
    new = "        c.SP = c.SP - 0x68u;\n        using var localMenuScope = V82LocalMenu.Enter(c, m);\n        c.StoreWord(31, m, (c.SP + 0x74u));\n"
    patch_once(args.shell_source.resolve(), old, new, "local multiplayer main-menu scope")
    old = "        c.RA = 0x80104284u;\n        Vigilante82PC.func_8001A3B0(c, m);\n"
    new = "        V82LocalMenu.Label(c, m);\n" + old
    patch_once(args.shell_source.resolve(), old, new, "multiplayer main-menu label")
    old = "        L801044A0: ;\n        c.S3 = 0x00000006u;\n        c.CopyRegister(18, 19);\n        c.S7 = 0x00000008u;\n"
    new = "        L801044A0: ;\n        V82LocalMenu.Join(c, m);\n"
    patch_once(args.shell_source.resolve(), old, new, "multiplayer join page")
    old = "    public static void func_80107AD4(CpuContext c, IMemory m)\n    {\n        func_80107AD4_Impl(c, m);"
    new = "    public static void func_80107AD4(CpuContext c, IMemory m)\n    {\n        if (V82LocalMenu.SelectPlayers(c, m)) return;\n        func_80107AD4_Impl(c, m);"
    patch_once(args.shell_source.resolve(), old, new, "joined player selection")

    # Keep both the native backing rectangle and cursor aligned with the
    # shifted menu text; changing only the text leaves stale pixels on return.
    shell_path = args.shell_source.resolve()
    shell_text = shell_path.read_text(encoding="utf-8")
    start = shell_text.index("    public static void func_80103FA0(")
    end = shell_text.index("    public static void func_8010458C(", start)
    section = shell_text[start:end]
    shifted = section.replace("0x0000005Au", "0x0000003Au").replace("0x0000002Au", "0x0000000Au")
    if shifted != section:
        shell_path.write_text(shell_text[:start] + shifted + shell_text[end:], encoding="utf-8")

    changed = patch_once(
        args.main_source.resolve(), MAIN_OLD, MAIN_NEW,
        "native options text trace",
    )
    changed += patch_once(
        args.shell_source.resolve(), SHELL_TITLE_OLD, SHELL_TITLE_NEW,
        "native title entry trace",
    )
    changed += patch_once(
        args.shell_source.resolve(), SHELL_PROMPT_OLD, SHELL_PROMPT_NEW,
        "native title prompt trace",
    )
    changed += patch_once(
        args.shell_source.resolve(), SHELL_MAIN_MENU_OLD, SHELL_MAIN_MENU_NEW,
        "native main-menu entry trace",
    )
    changed += patch_once(
        args.shell_source.resolve(),
        SHELL_MAIN_MENU_EXIT_OLD,
        SHELL_MAIN_MENU_EXIT_NEW,
        "native main-menu exit trace",
    )
    print(f"V8:2 native options seams ready ({changed} applied)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
