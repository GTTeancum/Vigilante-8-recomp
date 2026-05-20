#!/usr/bin/env python3
"""Classify functions in analysis/<program>/functions.json by subsystem.

Heuristics:
  1. If the symbol name matches a known PSY-Q library name (or library prefix),
     tag as 'runtime'.
  2. If the function references certain strings, tag accordingly (best-effort).
  3. Otherwise, tag as 'unknown' and let later passes refine.

Output:
  analysis/<program>/classification.json    -- {addr: {name, subsystem, reason}}
  analysis/<program>/classification_summary.txt
"""
import json, os, re, sys
from pathlib import Path
from collections import defaultdict

PSYQ_PREFIXES = (
    "Cd", "Spu", "Gs", "Gp", "Gv", "Gn", "Pad", "_pad", "Init", "Reset",
    "VSync", "DrawSync", "DrawOTag", "ClearOTag", "ClearImage", "LoadImage",
    "StoreImage", "MoveImage", "PutDispEnv", "PutDrawEnv", "SetDispMask",
    "SetGraphDebug", "SetGeomScreen", "SetGeomOffset", "RotMatrix",
    "TransMatrix", "ApplyMatrix", "MulMatrix", "RotTrans", "rsin", "rcos",
    "ratan2", "SquareRoot", "InitGeom", "OpenTIM", "ReadTIM", "CloseTIM",
    "InitPAD", "StartPAD", "StopPAD", "PadInit", "PadStart", "PadStop",
    "PadRead", "OpenEvent", "CloseEvent", "EnableEvent", "DisableEvent",
    "WaitEvent", "TestEvent", "DeliverEvent", "VSyncCallback",
    "ResetCallback", "GetPad", "InitHeap", "malloc", "free", "realloc",
    "memset", "memcpy", "bzero", "bcopy", "bcmp", "printf", "sprintf",
    "strcpy", "strcmp", "strcat", "strlen", "atoi", "DMA_", "DMACallback",
    "GetVideoMode", "SetVideoMode", "PADCMD_", "PADPORTD_", "PADSEQD_",
    "FntLoad", "FntOpen", "FntPrint", "FntFlush",
)

PSYQ_EXACT = {
    "main",  # often shadows real main - we'll treat separately
}

# Strings -> subsystem inference
STRING_TAGS = [
    ("physics",  re.compile(r"Vehicle|Physics|Collision|Suspension|Damage|wrecked", re.I)),
    ("assets",   re.compile(r"\.exp|\.dll|\.fnt|\.tbl|\.bin|\.bsd|\.bs|\.snd|\.tim|Common\.exp|Quest\.bin|Shell\\|HUD\\|Terrain\\|Sounds\\|Misc\\", re.I)),
    ("gameplay", re.compile(r"PU_WeaponUpgrade|weapon|Player|Score|Match|Round|Quest|wanted|felon|civil|samaritan|vigilante|legend|coyote|delinquent|greenhorn", re.I)),
    ("audio",    re.compile(r"Spu|Sound|Music|VAG|XA|SND|Track|CD COVER|READING INDEX", re.I)),  # mostly runtime
    ("video",    re.compile(r"\.str|Video|MDEC|Movie|Intro|Credits|Demo", re.I)),
    ("renderer", re.compile(r"GPU|Graph|Draw|VRAM|TIM|prim|polygon", re.I)),
    ("controls", re.compile(r"Pad|controller|buttons|input", re.I)),
]

def main():
    if len(sys.argv) < 2:
        print("usage: classify.py <analysis-dir>")
        sys.exit(2)
    adir = Path(sys.argv[1])
    funcs = json.loads((adir / "functions.json").read_text())
    xrefs = json.loads((adir / "xrefs.json").read_text())

    # Map string addr -> text, function addr -> referenced strings
    strings = {}
    for line in (adir / "strings.txt").read_text(errors="ignore").splitlines():
        if "\t" not in line: continue
        addr, s = line.split("\t", 1)
        strings[addr.strip()] = s

    # For each function, gather strings referenced inside its body via xrefs reverse
    # We need: address -> list of strings referenced. The xrefs.json is calls-only.
    # Use disassembly: scan mips/*.s for li / lui patterns referencing string addrs.
    mips_dir = adir / "mips"
    func_strings = defaultdict(list)
    string_addr_set = set(strings.keys())
    # Build address-as-int -> string text
    str_int = {int(a, 16): t for a, t in strings.items()}
    for f in funcs:
        addr = f["address"][2:]   # strip 0x
        mips_path = mips_dir / (addr + ".s")
        if not mips_path.exists(): continue
        try:
            # Find references to "0x80....." in the mips text
            text = mips_path.read_text(errors="ignore")
        except Exception:
            continue
        for m in re.finditer(r"0x([0-9a-fA-F]{8})", text):
            v = int(m.group(1), 16)
            if v in str_int:
                func_strings[f["address"]].append(str_int[v])

    classification = {}
    counts = defaultdict(int)
    for f in funcs:
        addr = f["address"]
        name = f["name"]
        reason = ""
        sub = "unknown"

        if any(name.startswith(p) for p in PSYQ_PREFIXES) or name in PSYQ_EXACT:
            sub = "runtime"
            reason = "psyq-name"
        elif name.startswith("FUN_") or name.startswith("LAB_"):
            # Use referenced strings
            refs = func_strings.get(addr, [])
            for tag, rx in STRING_TAGS:
                if any(rx.search(r) for r in refs):
                    sub = tag
                    reason = "string-match:" + tag
                    break
        else:
            # Other named symbols -- could be runtime data labels or game funcs
            sub = "runtime"
            reason = "named-symbol"

        classification[addr] = {
            "name": name,
            "subsystem": sub,
            "reason": reason,
            "size": f["size"],
        }
        counts[sub] += 1

    (adir / "classification.json").write_text(json.dumps(classification, indent=1))
    with open(adir / "classification_summary.txt", "w") as fp:
        fp.write(f"total functions: {len(funcs)}\n")
        for k in sorted(counts):
            fp.write(f"  {k:10s}: {counts[k]}\n")
    print("classification.json + classification_summary.txt written")
    print(open(adir / "classification_summary.txt").read())

if __name__ == "__main__":
    main()
