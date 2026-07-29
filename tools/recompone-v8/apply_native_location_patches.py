#!/usr/bin/env python3
"""Durably append the eleventh native V8 location-selector/loading slot.

RecompOne patches whole functions, while this one source-level seam changes
the retail loop bound from ten records to eleven and routes LOAD's two
loading-card caption lookups through the append-only arena registry. All native
selector drawing, wheel motion, marker placement, input, acceptance, and
loading-card rendering remain the original implementations.
"""

from __future__ import annotations

import argparse
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_SOURCE = ROOT / "reference" / "generated" / "recompiled" / "SHELL.cs"
DEFAULT_LOAD_SOURCE = (
    ROOT / "reference" / "generated" / "recompiled" / "LOAD.cs"
)

OLD_SHELL = """        c.S1 = c.S1 + 0x1u;
        c.V0 = (int)c.S1 < 10 ? 1u : 0u;
"""
NEW_SHELL = """        c.S1 = c.S1 + 0x1u;
        c.V0 = (int)c.S1 < 11 ? 1u : 0u;
"""
OLD_LOAD = """        c.A1 = m.ReadU32(c.V0);
        c.A0 = c.S0 + 0u;
"""
NEW_LOAD = """        c.A1 = RecompOne.Runtime.Sdk.V8ArenaRegistry.ResolveNativeLoadingTitle(
            c, m, m.ReadU32(c.V0));
        c.A0 = c.S0 + 0u;
"""
OLD_LOAD_CAPTURE = """        L80101EB8: ;
        c.S2 = 0x80060000u;
        c.RA = 0x80101EC0u;
        Vigilante8PC.FUN_80015a00(c, m);
"""
NEW_LOAD_CAPTURE = """        L80101EB8: ;
        c.S2 = 0x80060000u;
        RecompOne.Runtime.Sdk.V8ArenaRegistry.CaptureNativeLoadingCard(c, m);
        c.RA = 0x80101EC0u;
        Vigilante8PC.FUN_80015a00(c, m);
"""


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument(
        "--load-source", type=Path, default=DEFAULT_LOAD_SOURCE
    )
    args = parser.parse_args()

    shell = args.source.read_text(encoding="utf-8")
    load = args.load_source.read_text(encoding="utf-8")

    shell_done = NEW_SHELL in shell
    load_done = load.count(NEW_LOAD) == 2
    capture_done = load.count(NEW_LOAD_CAPTURE) == 1
    if args.check:
        if not shell_done:
            raise SystemExit(
                "V8 native location selector still has 10 slots"
            )
        if not load_done:
            raise SystemExit(
                "V8 native loading captions do not resolve appended slot"
            )
        if not capture_done:
            raise SystemExit(
                "V8 native loading-card proof seam is absent"
            )
        print("V8 native location selector/loading slot 10 present")
        return 0

    if not shell_done:
        count = shell.count(OLD_SHELL)
        if count != 1:
            raise SystemExit(
                f"expected one V8 native location loop bound, found {count}"
            )
        shell = shell.replace(OLD_SHELL, NEW_SHELL, 1)
        args.source.write_text(shell, encoding="utf-8")

    if not load_done:
        count = load.count(OLD_LOAD)
        if count != 2:
            raise SystemExit(
                f"expected two V8 native loading caption lookups, found {count}"
            )
        load = load.replace(OLD_LOAD, NEW_LOAD)

    if not capture_done:
        count = load.count(OLD_LOAD_CAPTURE)
        if count != 1:
            raise SystemExit(
                f"expected one V8 native loading-card seam, found {count}"
            )
        load = load.replace(OLD_LOAD_CAPTURE, NEW_LOAD_CAPTURE, 1)

    args.load_source.write_text(load, encoding="utf-8")

    print("V8 native location selector/loading: appended slot 10")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
