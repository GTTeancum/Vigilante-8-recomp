"""Durable native seams for local multiplayer and its process-local harness."""
from pathlib import Path
import argparse


def patch(path: Path) -> None:
    text = path.read_text(encoding="utf-8-sig")
    anchor = "        L800135C8: ;\n"
    insertion = "        if (SplitHarnessLaunch(c, m)) goto L80013640;\n"
    if insertion not in text:
        if text.count(anchor) != 1:
            raise RuntimeError("native match-entry seam changed")
        text = text.replace(anchor, anchor + insertion)
    trace = "        SplitHarnessTrace(c, m);\n"
    if trace not in text:
        text = text.replace("        L80013DF0: ;\n", "        L80013DF0: ;\n" + trace)
    prepare = "        SplitHarnessPrepare(c, m);\n"
    if prepare not in text:
        text = text.replace("        c.RA = 0x8001393Cu;\n        Vigilante82PC.func_80033550(c, m);\n",
                            "        c.RA = 0x8001393Cu;\n        Vigilante82PC.func_80033550(c, m);\n" + prepare)
    release = "        SplitRelease(c, m);\n"
    release_anchor = "        RecompOne.Runtime.Sdk.V82VehicleRegistry.ReleaseMatchRuntimes(c, m);\n"
    if release not in text:
        if text.count(release_anchor) != 1: raise RuntimeError("native split teardown seam changed")
        text = text.replace(release_anchor, release_anchor + release)
    skip_wait = "        if (SplitHarnessPlayers != 0) goto L80013A00;\n"
    if skip_wait not in text:
        text = text.replace("        L800139E4: ;\n", "        L800139E4: ;\n" + skip_wait)
    render = "        if (SplitHarnessRender(c, m)) goto L8001473C;\n"
    if render not in text:
        text = text.replace("        L80013E80: ;\n", "        L80013E80: ;\n" + render)
    input_hook = "        SplitHarnessInput(c, m);\n"
    if input_hook not in text:
        start = text.index("    public static void func_80015540(")
        end = text.index("    public static void func_80015AA8(", start)
        region = text[start:end]
        region = region.replace("        c.SP = c.SP + 0x38u;\n", "        c.SP = c.SP + 0x38u;\n" + input_hook)
        text = text[:start] + region + text[end:]
    for original in (
        "        c.LoadWord(2, m, (c.V0 + 0x8u));\n        c.V0 = c.V0 & 0x0100u;",
        "        c.S2 = c.A0 + c.V0;\n        c.V1 = (uint)(short)m.ReadU16((c.S1 + 0x16u));",
    ):
        patched = (original.replace("(c.V0 + 0x8u)", "(SplitInputAddress(c.V0) + 0x8u)")
                   if "LoadWord" in original else original.replace("c.S2 = c.A0 + c.V0", "c.S2 = SplitInputAddress(c.A0 + c.V0)"))
        if patched not in text:
            if text.count(original) != 1: raise RuntimeError("player input seam changed")
            text = text.replace(original, patched)
    # Only redirect proven indexed state accesses, never the neighboring
    # native globals occupying the same numeric addresses.
    def seam(source, function, original, replacement):
        start = source.index(f"    public static void func_{function}(")
        end = source.find("    public static void func_", start + 30)
        if end < 0: end = len(source)
        region = source[start:end]
        if replacement not in region:
            if region.count(original) != 1:
                raise RuntimeError(f"split state seam changed: {function}")
            region = region.replace(original, replacement)
        return source[:start] + region + source[end:]

    text = seam(text, "8003CCD4", "        c.A0 = c.A0 + c.V0;",
                "        c.A0 = SplitMatrixAddress(c.A0 + c.V0);")
    text = seam(text, "80038870", "        c.StoreWord(2, m, (c.GP + 0xC74u));",
                "        if (!SplitVersus(c, m)) c.StoreWord(2, m, (c.GP + 0xC74u));")
    text = seam(text, "800132CC", "        L80013C14: ;\n",
                "        L80013C14: ;\n        SplitCheckRound(c, m);\n")
    for result_draw in ('80013FC0', '8001465C'):
        text = seam(text, "800132CC", f"        c.RA = 0x{result_draw}u;\n",
                    f"        SplitNativeResultContext(c, m);\n        c.RA = 0x{result_draw}u;\n")
    text = seam(text, "800132CC", "        L80013640: ;\n",
                "        L80013640: ;\n        SplitRestoreRematch(c, m);\n")
    text = seam(text, "800132CC", "        c.RA = 0x800147C8u;\n",
                "        SplitCapturePlayerRecords(c, m);\n        c.RA = 0x800147C8u;\n")
    text = seam(text, "80012930", "        L80012CDC: ;\n",
                "        L80012CDC: ;\n        if (SplitVersus(c, m)) goto L80012E44;\n")
    text = seam(text, "80012930", "        c.V0 = (int)c.S2 < 2 ? 1u : 0u;",
                "        c.V0 = (int)c.S2 < (SplitVersus(c, m) ? SplitPlayers : 2) ? 1u : 0u;")
    text = seam(text, "80012930", "        c.V0 = c.V0 + c.T4;",
                "        c.V0 = SplitAwardAddress(c, c.V0 + c.T4);")
    text = seam(text, "80012930", "        c.V0 = c.T2 + c.V0;",
                "        c.V0 = SplitUpgradeAddress(c.T2 + c.V0);")
    text = text.replace("        c.V0 = SplitPlayerProgress(c, m, (int)c.S2, c.V0);\n", "")
    text = seam(text, "80012930", "        c.A3 = c.V0 + 0x6u;\n",
                "        c.A3 = c.V0 + 0x6u;\n        c.A3 = SplitPlayerProgress(c, m, (int)c.S2, c.A3 - 6) + 6;\n")
    text = seam(text, "80049D54", "        c.V0 = c.V0 + c.V1;\n        c.V1 = m.ReadU8(c.V0);\n        c.V1 = c.V1 + c.A0;",
                "        c.V0 = SplitAwardAddress(c, c.V0 + c.V1);\n        c.V1 = m.ReadU8(c.V0);\n        c.V1 = c.V1 + c.A0;")
    # This routine alone interprets the proven human-pointer and score tables.
    # In particular CC4 remains the destroyed-car counter everywhere else.
    import re
    start = text.index("    public static void func_80012930(")
    end = text.index("    public static void func_", start + 30)
    result = text[start:end]
    result = re.sub(r'c\.(LoadWord|StoreWord)\((\d+), m, ([^;\n]+)\);',
                    lambda match: match.group(0) if 'SplitResultAddress' in match[3] else
                    f'c.{match[1]}({match[2]}, m, SplitResultAddress(c, m, {match[3]}));', result)
    result = result.replace('c.V0 = (int)c.S2 < 4 ? 1u : 0u;',
                            'c.V0 = (int)c.S2 < (SplitVersus(c, m) ? SplitPlayers : 4) ? 1u : 0u;')
    if '        SplitResultFormat(c, m);' not in result:
        result = result.replace('        c.RA = 0x80012F44u;', '        SplitResultFormat(c, m);\n        c.RA = 0x80012F44u;')
    if '        SplitResultDrawText(c, m);' not in result:
        result = result.replace('        L800131E8: ;', '        L800131E8: ;\n        SplitResultDrawText(c, m);')
        result = result.replace('        c.LoadWord(3, m, SplitResultAddress(c, m, (c.GP + 0x83Cu)));',
                                '        if (SplitVersus(c, m) && c.S2 >= SplitPlayers) goto L80013294;\n        c.LoadWord(3, m, SplitResultAddress(c, m, (c.GP + 0x83Cu)));')
    text = text[:start] + result + text[end:]
    text = seam(text, "80012930", "        L800131F4: ;\n",
                "        L800131F4: ;\n        SplitResultFooter(c, m);\n")
    text = text.replace("        SplitResultFontState(c, m);\n", "")
    text = seam(text, "80012930", "        Vigilante82PC.func_800198B8(c, m);",
                "        if (!SplitCreateResultPackets(c, m)) Vigilante82PC.func_800198B8(c, m);")
    text = seam(text, "80011C58", "        c.LoadWord(22, m, c.V0);",
                "        c.LoadWord(22, m, SplitPauseInputAddress(c, c.V0));")
    text = seam(text, "80011C58", "        c.V0 = (uint)(short)m.ReadU16(c.V1);",
                "        c.V0 = (uint)(short)m.ReadU16(SplitInputAddress(c.V1));")
    text = seam(text, "80011C58", "        L80011D54: ;\n",
                "        L80011D54: ;\n        if (SplitPlayers >= 3) { m.WriteU8(c.GP + 0xC11, (byte)(m.ReadU8(c.GP + 0xC11) & ~2)); m.WriteU16(c.GP + 0x342, 60); }\n")
    merge = "        SplitMergeMenuInput(c, m);\n"
    anchor = "        c.StoreWord(17, m, (c.GP + 0xCFCu));\n"
    if merge not in text:
        if text.count(anchor) != 1: raise RuntimeError("split menu input aggregate seam changed")
        text = text.replace(anchor, merge + anchor)
    owner = "        SplitResolvePauseOwner(c, m);\n"
    if owner not in text:
        text = text.replace("        L80013DF0: ;\n", "        L80013DF0: ;\n" + owner)
    text = seam(text, "80036C2C", "        c.V0 = c.V1 + c.S3;\n        c.LoadWord(2, m, c.V0);",
                "        c.V0 = c.V1 + c.S3;\n        c.LoadWord(2, m, SplitPlayerModelAddress(c.V0));")
    for function in ("80015AA8", "80015ADC", "80015B00"):
        text = seam(text, function, "        c.V0 = c.V0 + c.V1;",
                    "        c.V0 = SplitRumbleAddress(c, c.V0 + c.V1);")
    text = seam(text, "8003AC84_Impl", "        c.LoadWord(3, m, c.V1);\n        c.LoadWord(4, m, (c.S3 + 0x100u));",
                "        c.LoadWord(3, m, SplitCameraAddress(c.V1));\n        c.LoadWord(4, m, (c.S3 + 0x100u));")
    for function in ("80036558", "800365E0"):
        text = seam(text, function, "        c.A0 = c.A0 + c.V0;",
                    "        c.A0 = SplitUpgradeAddress(c.A0 + c.V0);")
    # The remaining upgrade accesses have multiple arithmetic sites: anchor
    # the final table addition by the following operation.
    for original in (
        "        c.A0 = c.A0 + c.V0;\n        c.A0 = RecompOne.Runtime.Sdk.V82VehicleRegistry.UpgradeStatusForObject(c, m, c.S5, c.A0);",
        "        c.A0 = c.A0 + c.V0;\n        c.A2 = c.A0 + 0x6u;",
        "        c.A0 = c.A0 + c.A1;\n        c.A0 = RecompOne.Runtime.Sdk.V82VehicleRegistry.UpgradeStatusForObject(c, m, c.S5, c.A0);",
        "        c.A0 = c.A0 + c.A1;\n        c.V0 = m.ReadU8((c.A0 + 0x6u));",
    ):
        first, rest = original.split("\n", 1)
        replacement = first.replace("c.A0 = ", "c.A0 = SplitUpgradeAddress(").replace(";", ");") + "\n" + rest
        # The shared vehicle/quest adapter can insert an ownership lookup
        # immediately after the split table address. Keep reruns composable.
        import re
        without_owner_lookup = re.sub(
            r"^        c\.A0 = RecompOne\.Runtime\.Sdk\.V82VehicleRegistry\.UpgradeStatusForObject\([^\n]+\n",
            "", text, flags=re.MULTILINE)
        if replacement not in text and replacement not in without_owner_lookup:
            if text.count(original) != 1: raise RuntimeError("upgrade table seam changed")
            text = text.replace(original, replacement)
    shell = path.with_name("SHELL_LOAD.cs")
    source = shell.read_text(encoding="utf-8-sig")
    source = seam(source, "80103D94", "        c.StoreWord(2, m, c.V1);",
                  "        c.StoreWord(2, m, SplitCameraAddress(c.V1));")
    for required in (insertion, trace, prepare, skip_wait, render, input_hook):
        if text.count(required) != 1:
            raise RuntimeError(f"missing or duplicated split harness seam: {required.strip()}")
    path.write_text(text, encoding="utf-8")
    shell.write_text(source, encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, required=True)
    patch(parser.parse_args().source)
