#!/usr/bin/env python3
"""Bulk-emit pass-1 stub C files from Ghidra pseudo-C for every in-scope
function not already decompiled by hand.

Each emitted file:
  - Lives in src/<subsystem>/auto/<addr>.c
  - Contains the Ghidra pseudo-C as a comment header followed by a
    placeholder body. (We do NOT use the raw pseudo-C as runnable code
    because it references undefined4 etc.)
  - Carries an explicit `// LOW confidence: pass-1 auto-stub` header.

Pass 2 walks src/<subsystem>/auto/ and promotes functions one by one.

usage: bulk_emit.py <analysis-dir>
"""
import json, sys, re
from pathlib import Path

# Hand-written functions already in src/. Don't overwrite.
HANDWRITTEN = {
    "0x80044fbc",  # Heap_Init
    "0x80045004",  # Heap_Alloc
    "0x80045088",  # Heap_Free
    "0x80045134",  # Heap_Realloc
    "0x80044efc",  # V8_MemSet
    "0x80044c44",  # V8_MemCopy
    "0x80044d9c",  # V8_MemMove
    "0x800154f4",  # V8_CdReadSectors
    "0x8001714c",  # V8_SeedRng
}

# Subsystem assignment by address range (rough; refine in pass 2).
def subsystem_for(addr_int, name, refs):
    # First 256 KiB of the VA range is generally the C runtime / library.
    # Game-specific code lives 0x80011000 .. 0x80044fbc-ish (heap helpers),
    # asset loaders cluster around the directory reader 0x80015c68.
    if any(s in (refs or "") for s in (".exp", ".dll", ".tbl", "Common.exp", "Shell\\")):
        return "assets"
    if "wrecked" in (refs or "") or "PU_WeaponUpgrade" in (refs or ""):
        return "gameplay"
    # Default split by address: stub everything as gameplay for now.
    if 0x80011000 <= addr_int < 0x80015000:
        return "gameplay"
    if 0x80015000 <= addr_int < 0x80022000:
        return "assets"
    if 0x80022000 <= addr_int < 0x80030000:
        return "gameplay"
    return "gameplay"

def main():
    adir = Path(sys.argv[1])
    funcs = json.loads((adir / "functions.json").read_text())
    queue = (adir / "triage_queue.txt").read_text().splitlines()
    in_scope = set()
    for line in queue:
        if not line.strip(): continue
        addr = line.split("\t")[0]
        in_scope.add(addr)

    repo = adir.parent.parent
    written = 0
    skipped = 0
    for line in queue:
        if not line.strip(): continue
        addr, name, *_ = line.split("\t")
        if addr in HANDWRITTEN: continue
        addr_int = int(addr, 16)
        decomp_file = adir / "decomp" / (addr[2:] + ".c")
        if not decomp_file.exists():
            skipped += 1
            continue
        pseudo = decomp_file.read_text(errors="ignore")
        sub = subsystem_for(addr_int, name, pseudo)
        out_dir = repo / "src" / sub / "auto"
        out_dir.mkdir(parents=True, exist_ok=True)
        out_path = out_dir / (addr[2:] + ".c")
        if out_path.exists():
            skipped += 1
            continue

        # Strip Ghidra "WARNING" comments to keep body lean
        body = re.sub(r"^/\* WARNING:.*?\*/\n", "", pseudo, flags=re.M)
        out_path.write_text(
            f"// LOW confidence: pass-1 auto-stub for {name} @ {addr}\n"
            f"// Subsystem: {sub}.  Will be cleaned in DECOMP_PASS_2.\n"
            f"// Original Ghidra pseudo-C follows, untouched. Do NOT compile.\n"
            f"#if 0\n"
            f"{body}\n"
            f"#endif\n"
        )
        written += 1

    print(f"wrote {written} auto-stubs; skipped {skipped} (handwritten/no-decomp/exists)")

if __name__ == "__main__":
    main()
