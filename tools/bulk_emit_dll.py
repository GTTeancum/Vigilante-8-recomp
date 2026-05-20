#!/usr/bin/env python3
"""bulk_emit_dll.py -- per-DLL pass-1 auto-stub emitter.

For every DLL function not already hand-cleaned in src/, write an
auto-stub under src/physics/<level>/auto/<addr>.c (terrain) or
src/gameplay/shell_auto/<addr>.c (SHELL/LOAD).

Satisfies the charter's "every in-scope function has corresponding C
in src/" rule at pass-1 quality for the DLL layer.
"""
import json, re, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

TERRAIN_LEVELS = {
    "AIRGRAVE": "airgrave",
    "CANYNLND": "canynlnd",
    "CASNOCTY": "casnocty",
    "HOOVRDAM": "hoovrdam",
    "OILFIELD": "oilfield",
    "SANDFACT": "sandfact",
    "SCRTBASE": "scrtbase",
    "SKIRESRT": "skiresrt",
    "VALLYFRM": "vallyfrm",
    "WILDWEST": "wildwest",
}

# Already-promoted addresses we should not stub over.
HANDWRITTEN = {
    "8010068c",   # AGTracker_Tick
    "8010059c",   # AGProjectile_Tick
    "80100be8",   # Boulder_Tick (CANYNLND)
    "80100c6c",   # SF_ConveyorTick (SANDFACT)
}

# Pull every hand-cleaned address from rename_log.md too.
rename = (ROOT / "notes/rename_log.md").read_text()
for m in re.finditer(r"0x([0-9a-fA-F]{8})", rename):
    HANDWRITTEN.add(m.group(1).lower())

written = 0
for dll_dir in (ROOT / "analysis/dll").iterdir():
    if not dll_dir.is_dir(): continue
    name = dll_dir.name
    if name in TERRAIN_LEVELS:
        out = ROOT / "src/physics" / TERRAIN_LEVELS[name] / "auto"
    elif name in ("SHELL", "LOAD"):
        out = ROOT / "src/gameplay" / f"{name.lower()}_auto"
    else:
        continue
    out.mkdir(parents=True, exist_ok=True)

    for c in (dll_dir / "decomp").glob("*.c"):
        addr = c.stem
        if addr in HANDWRITTEN: continue
        body = c.read_text(errors="ignore")
        # Drop Ghidra WARNING comments
        body = re.sub(r"^/\* WARNING:.*?\*/\n", "", body, flags=re.M)
        target = out / (addr + ".c")
        if target.exists(): continue
        target.write_text(
            f"// LOW confidence: pass-1 auto-stub for {name}.DLL FUN_{addr}.\n"
            f"// Will be promoted in DECOMP_PASS_2/3.\n"
            f"// Original Ghidra pseudo-C below (gated out of build).\n"
            f"#if 0\n{body}\n#endif\n"
        )
        written += 1

print(f"wrote {written} DLL auto-stubs")
