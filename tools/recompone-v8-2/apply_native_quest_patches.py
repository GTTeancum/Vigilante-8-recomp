"""Install shared quest table access seams in generated native code."""
from pathlib import Path
import argparse

ROOT = Path(__file__).resolve().parents[2]

def patch_function(path, address, transforms):
    text = path.read_text()
    start = text.index(f"    public static void func_{address}(CpuContext c, IMemory m)")
    end = text.index("\n    }", start)
    body = text[start:end]
    for old, new in transforms:
        if old == "    {\n":
            old = "(CpuContext c, IMemory m)\n" + old
            new = "(CpuContext c, IMemory m)\n" + new
        if new in body: continue
        if body.count(old) != 1:
            raise ValueError(f"Quest seam {address}: expected one {old!r}, got {body.count(old)}")
        body = body.replace(old, new, 1)
    path.write_text(text[:start] + body + text[end:])

Q = "RecompOne.Runtime.Sdk.V82QuestRegistry"

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", type=Path, default=ROOT / "reference-v8-2/generated/recompiled")
    args = parser.parse_args()
    main = args.source_root / "main.cs"
    shell = args.source_root / "SHELL_SHELL.cs"
    load = args.source_root / "SHELL_LOAD.cs"
    patch_function(shell, "801026C4", [("    {\n", f"    {{\n        {Q}.ProbeNativeProgress(m);\n")])
    patch_function(main, "80038A38", [
        ("        c.A0 = SplitUpgradeAddress(c.A0 + c.V0);\n", "        c.A0 = SplitUpgradeAddress(c.A0 + c.V0);\n        c.A0 = RecompOne.Runtime.Sdk.V82VehicleRegistry.UpgradeStatusForObject(c, m, c.S1, c.A0);\n"),
    ])
    patch_function(main, "80049D54", [
        ("        c.A0 = SplitUpgradeAddress(c.A0 + c.A1);\n", "        c.A0 = SplitUpgradeAddress(c.A0 + c.A1);\n        c.A0 = RecompOne.Runtime.Sdk.V82VehicleRegistry.UpgradeStatusForObject(c, m, c.S4, c.A0 - c.S3) + c.S3;\n"),
    ])
    patch_function(shell, "8010299C", [("    {\n", f"    {{\n        if ({Q}.PrepareEnding(c, m))\n        {{\n            Vigilante82PC.func_8010FF04(c, m);\n            c.SP += 144;\n            return;\n        }}\n")])
    patch_function(main, "80031FCC", [
        ("        Vigilante82PC.func_80036C2C(c, m);\n", f"        Vigilante82PC.func_80036C2C(c, m);\n        {Q}.BindPlayerActor(m, c.S0, c.V0);\n"),
        ("        L80032010: ;\n", "        L80032010: ;\n        if (System.Environment.GetEnvironmentVariable(\"RECOMPONE_TRACE_QUEST\") == \"1\") System.Console.WriteLine($\"[QuestActor] type={m.ReadU8(c.S0)} spawn={(short)m.ReadU16(c.S0 + 2)} found=0x{c.A0:X8}\");\n"),
    ])
    patch_function(main, "80012930", [
        ("        L80012B04: ;\n", f"        L80012B04: ;\n        {Q}.ResultText(c, m);\n"),
        ("    {\n", f"    {{\n        {Q}.ProbeResultEntry(c, m);\n"),
        ("        Vigilante82PC.func_80032340(c, m);\n", f"        Vigilante82PC.func_80032340(c, m);\n        {Q}.ProbeObjectives(c);\n"),
        ("        c.A3 = c.V0 + 0x6u;\n", f"        c.V0 = {Q}.ProgressAddress((int)c.V1, (int)c.S2, c.V0);\n        c.A3 = c.V0 + 0x6u;\n"),
    ])
    for address in ("80036558", "800365E0"):
        patch_function(main, address, [
            ("    {\n", "    {\n        int questType = (int)c.A1, questSlot = (int)c.A0;\n"),
            ("        c.A0 = SplitUpgradeAddress(c.A0 + c.V0);\n", f"        c.A0 = SplitUpgradeAddress(c.A0 + c.V0);\n        c.A0 = {Q}.ProgressAddress(questType, questSlot, c.A0);\n"),
        ])
    patch_function(main, "800324A0", [("    {\n", f"    {{\n        if ({Q}.Load(c, m)) return;\n")])
    patch_function(main, "80032684", [("    {\n", f"    {{\n        {Q}.BeforeFree(c, m);\n")])
    patch_function(shell, "80102610", [
        ("    {\n", "    {\n        int questType = (int)c.A1, questSlot = (int)c.A0;\n"),
        ("        c.A0 = c.A0 + c.A1;\n", f"        c.A0 = c.A0 + c.A1;\n        c.A0 = {Q}.ProgressAddress(questType, questSlot, c.A0);\n"),
        ("        c.V1 = m.ReadU8(c.V0);\n", f"        c.V1 = {Q}.SafeTeam(questType, m.ReadU8(c.V0));\n"),
        ("        c.LoadWord(3, m, (c.V1 + 0x4u));\n", f"        c.LoadWord(3, m, (c.V1 + 0x4u));\n        c.V1 = {Q}.RouteAddress(questType, c.V1);\n"),
    ])
    patch_function(shell, "80104B08", [
        ("        c.LoadWord(4, m, (c.T1 + 0x4Cu));\n", "        c.A0 = RecompOne.Runtime.Sdk.V82ArenaRegistry.NativeLocationBackgroundOffset(m, c.T1);\n"),
        ("    {\n", "    {\n        int questType = (int)c.A0;\n"),
        ("        int questType = (int)c.A0;\n", f"        int questType = (int)c.A0;\n        {Q}.BeginRoute();\n"),
        ("        c.A1 = c.A1 + c.V0;\n", f"        c.A1 = c.A1 + c.V0;\n        c.A1 = {Q}.ProgressAddress(questType, 0, c.A1);\n"),
        ("        c.V0 = m.ReadU8(c.A0);\n", f"        c.V0 = {Q}.SafeTeam(questType, m.ReadU8(c.A0));\n"),
        ("        c.LoadWord(22, m, (c.V1 + 0x4u));\n", f"        c.LoadWord(22, m, (c.V1 + 0x4u));\n        c.S6 = {Q}.RouteAddress(questType, c.S6);\n"),
    ])
    patch_function(shell, "8010EDB0", [
        ("        L8010F2A4: ;\n", f"        L8010F2A4: ;\n        {Q}.SaveProgress(m);\n"),
        ("        c.S4 = c.V0 - 0x4618u;\n", f"        c.S4 = {Q}.ProgressBase({Q}.CurrentType(m), c.V0 - 0x4618u);\n"),
        ("        c.V0 = m.ReadU8(c.V1);\n        c.LoadWord(3, m, (c.A1 - 0x47DCu));\n",
         f"        c.V0 = {Q}.SafeTeam({Q}.CurrentType(m), m.ReadU8(c.V1));\n        c.LoadWord(3, m, (c.A1 - 0x47DCu));\n"),
        ("        c.LoadWord(2, m, (c.V1 + 0x4u));\n        c.LoadWord(21, m, c.V0);\n",
         f"        c.LoadWord(2, m, (c.V1 + 0x4u));\n        c.V0 = {Q}.RouteAddress({Q}.CurrentType(m), c.V0);\n        c.LoadWord(21, m, c.V0);\n"),
        ("        c.CopyRegister(5, 0);\n        c.V1 = m.ReadU8(c.V0);\n        c.V0 = c.A0 << 2;\n",
         f"        c.CopyRegister(5, 0);\n        c.V1 = {Q}.SafeTeam({Q}.CurrentType(m), m.ReadU8(c.V0));\n        c.V0 = c.A0 << 2;\n"),
        ("        c.LoadWord(3, m, (c.V1 + 0x4u));\n        c.V0 = c.V0 << 2;\n",
         f"        c.LoadWord(3, m, (c.V1 + 0x4u));\n        c.V1 = {Q}.RouteAddress({Q}.CurrentType(m), c.V1);\n        c.V0 = c.V0 << 2;\n"),
    ])
    # Remaining progress bases in the quest return/setup function all index the
    # currently selected character. Leave its separate native reset loop alone.
    text = shell.read_text()
    text = text.replace(f"        L8010F2A0: ;\n        {Q}.SaveProgress(m);\n", "        L8010F2A0: ;\n")
    start = text.index("    public static void func_8010EDB0(")
    end = text.index("\n    }", start)
    body = text[start:end]
    for reg in ("V1", "A0"):
        old = f"        c.{reg} = c.{reg} - 0x4618u;"
        body = body.replace(old, f"        c.{reg} = {Q}.ProgressBase({Q}.CurrentType(m), c.{reg} - 0x4618u);")
    shell.write_text(text[:start] + body + text[end:])
    # LOAD entry name is recovered from the enclosing generated function.
    text = load.read_text()
    seam = text.index("        c.S2 = c.S1 - 0x4810u;")
    import re
    address = list(re.finditer(r"public static void func_(\w+)\(", text[:seam]))[-1].group(1)
    patch_function(load, address, [
        ("        c.V0 = m.ReadU8(c.V0);\n        c.LoadWord(3, m, (c.V1 - 0x47DCu));\n",
         f"        c.V0 = {Q}.SafeTeam({Q}.CurrentType(m), m.ReadU8(c.V0));\n        c.LoadWord(3, m, (c.V1 - 0x47DCu));\n"),
        ("        c.LoadWord(3, m, (c.V1 + 0x4u));\n        c.V0 = c.V0 << 2;\n",
         f"        c.LoadWord(3, m, (c.V1 + 0x4u));\n        c.V1 = {Q}.RouteAddress({Q}.CurrentType(m), c.V1);\n        c.V0 = c.V0 << 2;\n"),
    ])
    text = main.read_text()
    old_probe = "        L80013A48: ;\n"
    new_probe = old_probe + f"        {Q}.ProbeGameplay(c, m);\n"
    if new_probe not in text: text = text.replace(old_probe, new_probe)
    old = "        L80013934: ;\n"
    new = old + f"        {Q}.LoadingCard(c, m);\n"
    if new not in text:
        assert text.count(old) == 1
        text = text.replace(old, new)
    main.write_text(text)
    print("Native quest loader, route and eligibility seams installed")

if __name__ == "__main__":
    main()
