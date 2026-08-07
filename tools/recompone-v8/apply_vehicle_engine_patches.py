#!/usr/bin/env python3
"""Reapply/check the durable V8 independent-vehicle seam after regeneration."""

from __future__ import annotations

import argparse
from pathlib import Path


PATCHES = (
    (
        "    public static void FUN_80018124(CpuContext c, IMemory m)\n"
        "    {\n",
        "        if (!RecompOne.Runtime.Context.PreHook.Run("
        "RecompOne.Runtime.Sdk.V8Compat.ClaimGuestVramAllocation, c, m)) "
        "return;\n",
    ),
    (
        "    public static void FUN_800183ec(CpuContext c, IMemory m)\n"
        "    {\n",
        "        if (!RecompOne.Runtime.Context.PreHook.Run("
        "RecompOne.Runtime.Sdk.V8Compat.IgnoreSyntheticVramFree, c, m)) "
        "return;\n",
    ),
    (
        "    public static void LAB_80021e5c(CpuContext c, IMemory m)\n"
        "    {\n",
        "        if (!RecompOne.Runtime.Context.PreHook.Run("
        "RecompOne.Runtime.Sdk.V8VehicleRegistry.CreateVehicle, c, m)) return;\n",
    ),
    (
        "    public static void FUN_800227a4(CpuContext c, IMemory m)\n"
        "    {\n",
        "        if (!RecompOne.Runtime.Context.PreHook.Run("
        "RecompOne.Runtime.Sdk.V8Compat.PrepareGuestCommonObjectMask, c, m)) "
        "return;\n",
    ),
)

REPLACEMENTS = (
    (
        (
            """        c.A0 = m.ReadU32((c.FP + 0x34u));
        c.A1 = c.S0 + 0u;
""",
            """        c.LoadWord(4, m, (c.FP + 0x34u));
        c.CopyRegister(5, 16);
""",
        ),
        (
            """        c.A0 = m.ReadU32((c.FP + 0x34u));
        c.A0 = RecompOne.Runtime.Sdk.V8VehicleRegistry.WheelBankForObject(m, c.S3, c.A0);
        c.A1 = c.S0 + 0u;
""",
            """        c.LoadWord(4, m, (c.FP + 0x34u));
        c.A0 = RecompOne.Runtime.Sdk.V8VehicleRegistry.WheelBankForObject(m, c.S3, c.A0);
        c.CopyRegister(5, 16);
""",
        ),
    ),
    (
        (
            """        c.V0 = m.ReadU32((c.FP + 0x34u));
        c.A0 = c.S6 + 0u;
""",
            """        c.LoadWord(2, m, (c.FP + 0x34u));
        c.CopyRegister(4, 22);
""",
        ),
        (
            """        c.V0 = m.ReadU32((c.FP + 0x34u));
        c.V0 = RecompOne.Runtime.Sdk.V8VehicleRegistry.WheelBankForObject(m, c.S3, c.V0);
        c.A0 = c.S6 + 0u;
""",
            """        c.LoadWord(2, m, (c.FP + 0x34u));
        c.V0 = RecompOne.Runtime.Sdk.V8VehicleRegistry.WheelBankForObject(m, c.S3, c.V0);
        c.CopyRegister(4, 22);
""",
        ),
    ),
)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--check", action="store_true", help="verify without modifying"
    )
    parser.add_argument(
        "--file",
        type=Path,
        default=Path("reference/generated/recompiled/main.cs"),
    )
    args = parser.parse_args()
    text = args.file.read_text(encoding="utf-8")
    missing = []
    for signature, hook in PATCHES:
        patched = signature + hook
        if patched in text:
            continue
        missing.append((signature, hook))
    missing_replacements = []
    for old_variants, new_variants in REPLACEMENTS:
        if any(new in text for new in new_variants):
            continue
        matching = [
            (old, new)
            for old, new in zip(old_variants, new_variants)
            if old in text
        ]
        if len(matching) != 1:
            raise SystemExit(
                "V8 wheel-bank patch site is missing or ambiguous"
            )
        missing_replacements.append(matching[0])
    if not missing and not missing_replacements:
        print("V8 independent-vehicle seams: present")
        return 0
    if args.check:
        raise SystemExit(
            f"{len(missing) + len(missing_replacements)} "
            "V8 independent-vehicle seam(s) are missing"
        )
    for signature, hook in missing:
        if text.count(signature) != 1:
            raise SystemExit(
                "V8 patch function signature is missing or ambiguous"
            )
        text = text.replace(signature, signature + hook)
    for old, new in missing_replacements:
        if text.count(old) != 1:
            raise SystemExit("V8 wheel-bank patch site is ambiguous")
        text = text.replace(old, new, 1)
    args.file.write_text(text, encoding="utf-8")
    print(f"V8 independent-vehicle seams: applied {len(missing)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
