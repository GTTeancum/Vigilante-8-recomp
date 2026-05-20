#!/usr/bin/env python3
"""Triage: starting from a set of anchor addresses, walk the call graph and
mark every reachable game-specific function as in-scope. Then produce a
dependency-ordered (leaf-first) work queue.

usage: triage.py <analysis-dir>
output:
    analysis/<...>/triage_queue.txt   leaves-first order
    analysis/<...>/triage_skipped.txt runtime/library calls that bound the walk
"""
import json, sys
from pathlib import Path
from collections import defaultdict, deque

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
    "strcpy", "strcmp", "strcat", "strlen", "atoi", "DMACallback",
    "GetVideoMode", "SetVideoMode", "PADCMD_", "PADPORTD_", "PADSEQD_",
    "FntLoad", "FntOpen", "FntPrint", "FntFlush", "MargePrim", "GetRCnt",
    "SetRCnt", "ClearOTagR", "SetDefDrawEnv", "SetDefDispEnv", "SetDrawEnv",
)

def is_runtime(name: str) -> bool:
    return any(name.startswith(p) for p in PSYQ_PREFIXES) and not name.startswith("FUN_")

# Anchors: main, init, the entry point.
ANCHORS = [
    "0x80015098",  # main()
    "0x80013cac",  # main loop
    "0x80015e8c",  # init: CdInit + heap
]

def main():
    adir = Path(sys.argv[1])
    funcs = {f["address"]: f for f in json.loads((adir / "functions.json").read_text())}
    xrefs = json.loads((adir / "xrefs.json").read_text())

    in_scope = set()
    skipped = set()
    queue = deque(ANCHORS)
    while queue:
        a = queue.popleft()
        if a in in_scope: continue
        if a not in funcs: continue
        name = funcs[a]["name"]
        if is_runtime(name):
            skipped.add(a)
            continue
        in_scope.add(a)
        for callee in xrefs.get(a, {}).get("callees", []):
            if callee not in in_scope:
                queue.append(callee)

    # Now order leaves-first (post-order DFS)
    order = []
    visited = set()
    def visit(a):
        if a in visited: return
        visited.add(a)
        for c in xrefs.get(a, {}).get("callees", []):
            if c in in_scope:
                visit(c)
        order.append(a)
    for a in ANCHORS:
        if a in in_scope:
            visit(a)
    # Any in-scope not reached by DFS (shouldn't happen) -> append
    for a in in_scope:
        if a not in visited:
            order.append(a)

    out_q = adir / "triage_queue.txt"
    with open(out_q, "w") as fp:
        for a in order:
            f = funcs[a]
            fp.write(f"{a}\t{f['name']}\tsize={f['size']}\n")
    with open(adir / "triage_skipped.txt", "w") as fp:
        for a in sorted(skipped):
            fp.write(f"{a}\t{funcs[a]['name']}\n")
    print(f"in-scope functions: {len(in_scope)}")
    print(f"runtime/library boundary: {len(skipped)}")
    print(f"triage_queue.txt has {len(order)} entries (leaves first)")

if __name__ == "__main__":
    main()
