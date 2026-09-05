"""Persist the common modal seam and reserve arrow gutters on native choice rows."""
from pathlib import Path
import argparse


def patch(source: Path):
    text = source.read_text(encoding="utf-8")
    marker = "public static void func_80011BB8(CpuContext c, IMemory m)\n    {"
    hook = "\n        if (!RecompOne.Runtime.Context.PreHook.Run(RecompOne.Runtime.Sdk.V82ModalStyle.DrawPanel, c, m)) return;"
    if marker + hook not in text:
        assert text.count(marker) == 1
        text = text.replace(marker, marker + hook)
    for caller in ("8001230C", "80012404"):
        original = f"c.RA = 0x{caller}u;\n        Vigilante82PC.func_8001A6BC(c, m);"
        replacement = (f"c.RA = 0x{caller}u;\n"
                       "        using (RecompOne.Runtime.Sdk.V82ModalStyle.InsetChoice(c, m))\n"
                       "            Vigilante82PC.func_8001A6BC(c, m);")
        if replacement not in text:
            assert text.count(original) == 1, caller
            text = text.replace(original, replacement)
    source.write_text(text, encoding="utf-8", newline="\n")


if __name__ == "__main__":
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--source", type=Path, required=True)
    patch(p.parse_args().source)
