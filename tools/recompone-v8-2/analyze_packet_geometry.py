"""Summarize opt-in PacketGeometry diagnostics without loading images."""
import argparse
import json
import re
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument("log", type=Path)
p.add_argument("--max-depth", type=float, default=2000)
p.add_argument("--max-x", type=float, default=20)
p.add_argument("--min-x", type=float, default=-55)
p.add_argument("--min-y", type=float, default=100)
p.add_argument("--limit", type=int, default=30)
a = p.parse_args()
endpoint = re.compile(r"view=([^,]+),([^,]+),([^,]+),projection=([^,]+),([^,]+),([^,]+),exact=(True|False),modern=(True|False),uv=(\d+),(\d+)")
rows = []
for line in a.log.read_text(errors="replace").splitlines():
    if not line.startswith("[PacketGeometry]"):
        continue
    command = int(re.search(r"command=0x([0-9A-F]+)", line)[1], 16)
    if not command & (1 << 26):
        continue
    vertices = []
    for m in endpoint.finditer(line):
        x, y, z, cx, cy, scale = map(float, m.groups()[:6])
        vertices.append(dict(view=[x, y, z], xy=[cx+x*scale/z, cy+y*scale/z] if z else None,
                             exact=m[7] == "True", modern=m[8] == "True", uv=[int(m[9]), int(m[10])]))
    if not vertices or max(v["view"][2] for v in vertices) > a.max_depth:
        continue
    points = [v["xy"] for v in vertices if v["xy"]]
    if not points or min(v[0] for v in points) > a.max_x or max(v[0] for v in points) < a.min_x:
        continue
    if max(v[1] for v in points) < a.min_y:
        continue
    rows.append(dict(packet=re.search(r"packet=(0x[0-9A-F]+)",line)[1],
                     owner=re.search(r'owner="([^"]*)"',line)[1], vertices=vertices))
print(json.dumps(dict(matches=len(rows), packets=rows[:a.limit]), indent=2))
