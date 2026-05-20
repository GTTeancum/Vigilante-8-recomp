#!/usr/bin/env python3
"""resolve_xrefs.py -- annotate DLL decomp files with main-EXE function names.

When Ghidra disassembles a relocated DLL standalone, calls into the
main EXE appear as `func_0x80XXXXXX(...)`. This tool walks every DLL
decomp file, looks up the target address in
`analysis/SLUS_005.10/functions.json`, and rewrites the call as
`<name> /* func_0x80XXXXXX */`. Pure annotation -- the file content
is otherwise unchanged.

Usage:
    resolve_xrefs.py
"""
import json, os, re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
funcs = json.loads((ROOT / "analysis/SLUS_005.10/functions.json").read_text())
name_by_addr = {int(f["address"], 16): f["name"] for f in funcs}

# Also include the curated rename log entries: parse the markdown table.
rename_md = (ROOT / "notes/rename_log.md").read_text()
for m in re.finditer(r"\|\s*0x([0-9a-fA-F]{8})\s*\|\s*FUN_[0-9a-fA-F]{8}\s*\|\s*`([^`]+)`", rename_md):
    addr = int(m.group(1), 16)
    name_by_addr[addr] = m.group(2)

# Iterate every DLL decomp file and rewrite cross-EXE calls.
total_rewrites = 0
for dll_dir in (ROOT / "analysis/dll").iterdir():
    if not dll_dir.is_dir(): continue
    decomp = dll_dir / "decomp"
    if not decomp.exists(): continue
    for c in decomp.glob("*.c"):
        text = c.read_text(errors="ignore")
        n_local = 0
        def repl(m):
            global n_local
            addr = int(m.group(1), 16)
            name = name_by_addr.get(addr)
            if not name:
                return m.group(0)
            n_local += 1
            return f"{name}/*0x{addr:08x}*/"
        out = re.sub(r"func_0x([0-9a-fA-F]{8})", repl, text)
        if n_local > 0:
            c.write_text(out)
            total_rewrites += n_local
print(f"resolved {total_rewrites} cross-overlay -> main-EXE references")
