"""Durable seams for the opt-in native split-screen development harness."""
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
        if replacement not in text:
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
