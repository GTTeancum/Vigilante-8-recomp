#!/usr/bin/env python3
"""Apply durable custom-vehicle seams to generated V8:2 engine code.

RecompOne supports whole-function hooks, but these sites need decoded pointer
selection inside two retail functions. The replacements preserve the retail
path and select independent custom stats, upgrade state, transformation table,
transformation bank, and wheel bank only for registered custom vehicle objects.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path


REPO = Path(__file__).resolve().parents[2]
DEFAULT_SOURCE = REPO / "reference-v8-2" / "generated" / "recompiled" / "main.cs"

MIGRATIONS = (
    (
        "        c.S0 = RecompOne.Runtime.Sdk.V82Compat.SelectVramFreeLeaf(c, m, c.S0);\n",
        "",
    ),
    (
        "        RecompOne.Runtime.Sdk.V82VehicleRegistry.UpdateSpecialTurretAim(c, m, c.A0);\n",
        "",
    ),
    (
        """        c.A0 = c.S3 | c.S2;
        c.RA = 0x80013770u;
""",
        """        c.A0 = c.S3 | c.S2;
        c.A0 = RecompOne.Runtime.Sdk.V82VehicleRegistry.SpecialResourceMask(m, c.A0);
        c.RA = 0x80013770u;
""",
    ),
    (
        """        func_8003AC84_Impl(c, m);
        RecompOne.Runtime.Sdk.V82VehicleRegistry.ApplyControllerPhysics(c, m);
""",
        """        func_8003AC84_Impl(c, m);
        // The capability pre-hook applies movement before native actions.
""",
    ),
    (
        """        RecompOne.Runtime.Sdk.V82Compat.EndTextureDecode(
            c, v82TextureDecodeCallerSp);
""",
        """        RecompOne.Runtime.Sdk.V82Compat.EndTextureDecode(
            c, m, v82TextureDecodeCallerSp);
""",
    ),
)

REPLACEMENTS = (
    (
        "    public static void func_80033550(CpuContext c, IMemory m)\n    {\n",
        "    public static void func_80033550(CpuContext c, IMemory m)\n    {\n"
        "        RecompOne.Runtime.Sdk.V82ArenaRegistry.CaptureLevelLoad(c, m);\n",
    ),
    (
        """        c.LoadWord(16, m, (c.GP + 0xE80u));
        c.StoreWord(20, m, (c.SP + 0x20u));
""",
        """        c.LoadWord(16, m, (c.GP + 0xE80u));
        c.S0 = RecompOne.Runtime.Sdk.V82Compat.SelectMatchVramFreeLeaf(c, m, c.S0);
        c.StoreWord(20, m, (c.SP + 0x20u));
""",
    ),
    (
        """    public static void func_80020754(CpuContext c, IMemory m)
    {
""",
        """    public static void func_80020754(CpuContext c, IMemory m)
    {
        RecompOne.Runtime.Sdk.V82Compat.RetireVramBackingTree(c, m);
""",
    ),
    (
        """    public static void func_80020F5C(CpuContext c, IMemory m)
    {
""",
        """    public static void func_80020F5C(CpuContext c, IMemory m)
    {
        if (!RecompOne.Runtime.Context.PreHook.Run(RecompOne.Runtime.Sdk.V82Compat.RetainSharedGuestTexture, c, m)) return;
""",
    ),
    (
        """        L80041D5C: ;
        c.V1 = m.ReadU16((c.S4 + 0xB4u));
""",
        """        L80041D5C: ;
        if (RecompOne.Runtime.Sdk.V82VehicleRegistry.ApplyOriginalControllerPhysics(c, m, c.S4)) {
            c.S1 = 0u;
            goto L80041E04;
        }
        c.V1 = m.ReadU16((c.S4 + 0xB4u));
""",
    ),
    (
        """        L80041EF8: ;
        c.V0 = (uint)(short)m.ReadU16((c.S4 + 0xAu));
""",
        """        L80041EF8: ;
        if (RecompOne.Runtime.Sdk.V82VehicleRegistry.UsesFlyingController(m, c.S4)) goto L80041FB0;
        c.V0 = (uint)(short)m.ReadU16((c.S4 + 0xAu));
""",
    ),
    (
        """        L8004AF0C: ;
        if (c.S0 != 0u) {
""",
        """        L8004AF0C: ;
        c.S1 = RecompOne.Runtime.Sdk.V82VehicleRegistry.SpecialCallbackForObject(m, c.S2, c.S1);
        if (c.S0 != 0u) {
""",
    ),
    (
        '    public static void func_80054EEC(CpuContext c, IMemory m)\n    {\n',
        '''    public static void func_80054EEC(CpuContext c, IMemory m)
    {
        uint caller = c.RA, requested = c.A0;
        func_80054EEC_Impl(c, m);
        RecompOne.Runtime.Sdk.V82Compat.TraceDisplayCallbackChange(c, m, caller, requested);
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void func_80054EEC_Impl(CpuContext c, IMemory m)
    {
''',
    ),
    (
        '''        func_80021F70_Impl(c, m);
        RecompOne.Runtime.Sdk.V82Compat.EndImportedRenderGroup(c, m);
''',
        '''        float previousScale = RecompOne.Runtime.Sdk.V82MeshClipCompat.BeginMeshDepth(m, c.A0);
        var previousOffsets = RecompOne.Runtime.Sdk.V82JunctionAttachments.BeginMesh(c.A0);
        try { func_80021F70_Impl(c, m); }
        finally
        {
            RecompOne.Runtime.Gte.PreciseViewScale = previousScale; RecompOne.Runtime.Sdk.V82JunctionAttachments.EndMesh(previousOffsets);
            RecompOne.Runtime.Sdk.V82Compat.EndImportedRenderGroup(c, m);
        }
''',
    ),
    (
        '    public static void func_80021FA8(CpuContext c, IMemory m)\n    {\n',
        '''    public static void func_80021FA8(CpuContext c, IMemory m)
    {
        float previousScale = RecompOne.Runtime.Sdk.V82MeshClipCompat.BeginMeshDepth(m, c.A0);
        var previousOffsets = RecompOne.Runtime.Sdk.V82JunctionAttachments.BeginMesh(c.A0);
        try { func_80021FA8_Impl(c, m); }
        finally { RecompOne.Runtime.Gte.PreciseViewScale = previousScale; RecompOne.Runtime.Sdk.V82JunctionAttachments.EndMesh(previousOffsets); }
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void func_80021FA8_Impl(CpuContext c, IMemory m)
    {
''',
    ),
    (
        """    public static void func_80041B0C(CpuContext c, IMemory m)
    {
""",
        """    public static void func_80041B0C(CpuContext c, IMemory m)
    {
        RecompOne.Runtime.Sdk.V82AutoWaterski.BeforeVehiclePhysics(c, m);
""",
    ),
    (
        """    public static void func_80049D54(CpuContext c, IMemory m)
    {
""",
        """    public static void func_80049D54(CpuContext c, IMemory m)
    {
        if (!RecompOne.Runtime.Context.PreHook.Run(
                RecompOne.Runtime.Sdk.V82VehicleRegistry.RejectUnsupportedTransformationPickup,
                c, m)) return;
""",
    ),
    (
        """    public static void func_8003E32C(CpuContext c, IMemory m)
    {
""",
        """    public static void func_8003E32C(CpuContext c, IMemory m)
    {
        if (!RecompOne.Runtime.Context.PreHook.Run(
                RecompOne.Runtime.Sdk.V82VehicleRegistry.RejectUnsupportedTransformationActivation,
                c, m)) return;
""",
    ),
    (
        """    public static void func_8003AC84(CpuContext c, IMemory m)
    {
        if (!RecompOne.Runtime.Context.PreHook.Run(RecompOne.Runtime.Sdk.V82VehicleRegistry.BeginControllerPhysics, c, m)) return;
        c.SP = c.SP - 0x20u;
""",
        """    public static void func_8003AC84(CpuContext c, IMemory m)
    {
        if (!RecompOne.Runtime.Context.PreHook.Run(RecompOne.Runtime.Sdk.V82VehicleRegistry.BeginControllerPhysics, c, m)) return;
        func_8003AC84_Impl(c, m);
        // The capability pre-hook applies movement before native actions.
    }
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.NoInlining)]
    public static void func_8003AC84_Impl(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0x20u;
""",
    ),
    (
        """        L8003C584: ;
        c.CopyRegister(18, 0);
        c.CopyRegister(23, 20);
        L8003C58C: ;
""",
        """        L8003C584: ;
        c.CopyRegister(18, 0);
        c.CopyRegister(23, 20);
        if (RecompOne.Runtime.Sdk.V82VehicleRegistry.SkipWheelConstructionForObject(m, c.S4)) {
            c.S2 = 6u;
            goto L_V82FlyingNoWheels;
        }
        L8003C58C: ;
""",
    ),
    (
        """        c.S7 = c.S7 + 0x4u;
        c.A0 = c.S4 + 0xA8u;
        c.CopyRegister(5, 0);
""",
        """        c.S7 = c.S7 + 0x4u;
        L_V82FlyingNoWheels: ;
        c.A0 = c.S4 + 0xA8u;
        c.CopyRegister(5, 0);
""",
    ),
    (
        """    public static void func_80022C54(CpuContext c, IMemory m)
    {
        c.At = RecompOne.Runtime.Gte.ReadControl(31);
""",
        """    public static void func_80022C54(CpuContext c, IMemory m)
    {
        RecompOne.Runtime.Sdk.V82Compat.ObserveVehicleReflectionPacket(c, m);
        c.At = RecompOne.Runtime.Gte.ReadControl(31);
""",
    ),
    (
        """    public static void func_80021064(CpuContext c, IMemory m)
    {
        c.SP = c.SP - 0xC70u;
""",
        """    public static void func_80021064(CpuContext c, IMemory m)
    {
        uint v82TextureDecodeCallerSp =
            RecompOne.Runtime.Sdk.V82Compat.BeginTextureDecode(c, m);
        c.SP = c.SP - 0xC70u;
""",
    ),
    (
        """        c.LoadWord(16, m, (c.SP + 0xC58u));
        c.SP = c.SP + 0xC70u;
        return;
""",
        """        c.LoadWord(16, m, (c.SP + 0xC58u));
        c.SP = c.SP + 0xC70u;
        RecompOne.Runtime.Sdk.V82Compat.EndTextureDecode(
            c, m, v82TextureDecodeCallerSp);
        return;
""",
    ),
    (
        """        c.LoadWord(4, m, (c.T3 + 0x48u));
        c.A3 = 0x00000008u;
""",
        """        c.LoadWord(4, m, (c.T3 + 0x48u));
        c.A0 = RecompOne.Runtime.Sdk.V82VehicleRegistry.WheelBankForObject(m, c.S4, c.A0);
        c.A3 = 0x00000008u;
""",
    ),
    (
        """        c.LoadWord(3, m, (c.T4 + 0x48u));
        c.A0 = c.A0 - c.S0;
""",
        """        c.LoadWord(3, m, (c.T4 + 0x48u));
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
        c.LoadWord(19, m, (c.V0 + 0x61C0u));
""",
        """        c.A0 = 0x80060000u;
        c.A0 = c.A0 + 0x3F74u;
        c.A0 = RecompOne.Runtime.Sdk.V82VehicleRegistry.TransformTableForObject(m, c.S5, c.A0);
        c.V1 = c.S6 << 1;
        c.LoadWord(19, m, (c.V0 + 0x61C0u));
        c.S3 = RecompOne.Runtime.Sdk.V82VehicleRegistry.TransformBankForObject(m, c.S5, c.S3);
""",
    ),
    (
        """        c.V1 = c.V1 + c.A0;
        c.LoadWord(3, m, c.V1);
        if (c.V1 == 0u) {
""",
        """        c.V1 = c.V1 + c.A0;
        c.LoadWord(3, m, c.V1);
        c.V1 = RecompOne.Runtime.Sdk.V82VehicleRegistry.SpecialBehaviorForObject(m, c.S2, c.V1);
        if (c.V1 == 0u) {
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


# apply_split_screen_patches appends its own release after the vehicle
# release. Validate both effects in their actual order on incremental builds.
TEARDOWN_CALL = (
    "        c.RA = 0x800147C8u;\n"
    "        Vigilante82PC.func_800333D0(c, m);\n"
)
TEARDOWN_END = "        c.RA = 0x800147D0u;\n"
VEHICLE_RELEASE = "        RecompOne.Runtime.Sdk.V82VehicleRegistry.ReleaseMatchRuntimes(c, m);\n"
SPLIT_RELEASE = "        SplitRelease(c, m);\n"


def patch_match_teardown(text: str) -> str:
    if text.count(TEARDOWN_CALL) != 1 or text.count(TEARDOWN_END) != 1:
        raise RuntimeError("missing or duplicate native match teardown boundary")
    start = text.index(TEARDOWN_CALL) + len(TEARDOWN_CALL)
    end = text.index(TEARDOWN_END)
    body = text[start:end]
    if end < start or body not in ("", VEHICLE_RELEASE, SPLIT_RELEASE, VEHICLE_RELEASE + SPLIT_RELEASE):
        raise RuntimeError("unknown or duplicated hooks in native match teardown")
    if body.startswith(VEHICLE_RELEASE):
        return text
    return text[:start] + VEHICLE_RELEASE + text[start:]


def patch_ground_engine(source: str) -> str:
    """Route only the native vehicle-engine key-on through the guest helpers.

    The original a0 is the vehicle. By the time 8001E098 is called the
    register contains the SPU voice instead. Capture the object on entry,
    not from a register that happens to survive a particular call path.
    """
    header = "    public static void func_80038408(CpuContext c, IMemory m)\n    {\n"
    if source.count(header) != 1:
        raise RuntimeError("native vehicle engine startup routine changed")
    start = source.index(header)
    end = source.index("\n    }", start)
    body = source[start:end]
    capture = "        uint groundEngineVehicle = c.A0;\n"
    seam = "c.A2 = RecompOne.Runtime.Sdk.V82VehicleRegistry.GroundEngineSample(m, groundEngineVehicle, c.A2);"
    # RecompOne uses a qualified direct call when crossing a generated unit.
    target = r"^( +)((?:Vigilante82PC\.)?func_8001E098)\(c, m\);$"
    calls = list(re.finditer(target, body, re.MULTILINE))
    if len(calls) != 1:
        raise RuntimeError("native ground/transform engine key-on changed")
    call = calls[0]
    indent, native = call.groups()
    hook = "RecompOne.Runtime.Sdk.V82VehicleRegistry.TryPlayLocalOriginalEngine"
    route = f"if (!{hook}(c, m, groundEngineVehicle, {native}))"
    if hook in body:
        if (not body.startswith(header + capture) or body.count(capture) != 1
                or body.count(seam) != 1 or body.count(hook) != 1
                or not body[:call.start()].endswith(indent[:-4] + seam + "\n" + indent[:-4] + route + "\n")):
            raise RuntimeError("incomplete or misplaced local engine key-on")
        return source
    if capture in body or seam in body:
        if (not body.startswith(header + capture) or body.count(capture) != 1
                or body.count(seam) != 1 or not body[:call.start()].endswith(indent + seam + "\n")):
            raise RuntimeError("incomplete or misplaced vehicle engine mapping")
        prefix = ""
    else:
        prefix = indent + seam + "\n"
    body = (body[:call.start()] + prefix + indent + route + "\n" +
            indent + "    " + native + "(c, m);" + body[call.end():])
    if capture not in body:
        body = body.replace(header, header + capture, 1)
    return source[:start] + body + source[end:]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    args = parser.parse_args()

    source = args.source.resolve()
    text = source.read_text(encoding="utf-8")
    patched = patch_match_teardown(text)
    changed = int(patched != text)
    text = patched
    for old, new in MIGRATIONS:
        if new and new in text:
            continue
        count = text.count(old)
        if count > 1:
            raise RuntimeError(
                f"expected at most one generated engine migration, found "
                f"{count}: {old.splitlines()[0].strip()}"
            )
        if count == 1:
            text = text.replace(old, new, 1)
            changed += 1
    for old, new in REPLACEMENTS:
        split_new = new.replace("c.A0 = c.A0 + c.V0;", "c.A0 = SplitUpgradeAddress(c.A0 + c.V0);")
        split_new_a1 = new.replace("c.A0 = c.A0 + c.A1;", "c.A0 = SplitUpgradeAddress(c.A0 + c.A1);")
        if new in text or split_new in text or split_new_a1 in text:
            continue
        count = text.count(old)
        if count != 1:
            raise RuntimeError(
                f"expected exactly one generated engine seam, found {count}: "
                f"{old.splitlines()[0].strip()}"
            )
        text = text.replace(old, new, 1)
        changed += 1

    start = text.index("public static void func_80021F70_Impl(")
    end = text.index("public static void func_80021FA8(", start)
    body = text[start:end]
    seam = "        RecompOne.Runtime.Sdk.V82ModelBounds.SkipInvisibleFaces(c);\n"
    if seam not in body:
        marker = "        L8002215C: ;"
        if body.count(marker) != 1:
            raise RuntimeError("model face traversal boundary changed")
        body = body.replace(marker, seam + marker, 1)
        text = text[:start] + body + text[end:]
        changed += 1

    mapped = patch_ground_engine(text)
    if mapped != text:
        text = mapped
        changed += 1

    if changed:
        source.write_text(text, encoding="utf-8")
    print(f"V8:2 custom-vehicle engine seams ready ({changed} applied)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
