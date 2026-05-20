#!/usr/bin/env python3
"""exp_dump.py -- dump the IFF FORM chunk tree of a V8 .EXP file.

Walks the EA-IFF FORM tree (4cc + BE-u32 size) and prints the chunk
inventory so we can identify which chunk holds the heightmap.

Heightmap signature we expect: chunks of ~0x2000 bytes (8KB) consisting
of (64 rows * 64 u16 cells), with values masked 0x7ff giving valid
terrain heights.

Usage: exp_dump.py path/to/OILFIELD.EXP
"""
import os
import struct
import sys


def be_u32(data: bytes, off: int) -> int:
    return struct.unpack_from(">I", data, off)[0]


def is_printable_4cc(b: bytes) -> bool:
    return all(0x20 <= c <= 0x7e for c in b)


def walk(data: bytes, off: int, end: int, depth: int, chunks: list) -> None:
    """Recursive IFF walker. Pads odd chunk sizes to 16-bit alignment."""
    while off + 8 <= end:
        tag = data[off:off+4]
        if not is_printable_4cc(tag):
            print(f"  {' ' * depth}[junk @ {off:#x}: {tag.hex()}]")
            break
        size = be_u32(data, off + 4)
        body = off + 8
        if body + size > end:
            print(f"  {' ' * depth}[truncated {tag.decode('ascii', 'replace')} "
                  f"@ {off:#x}, size {size:#x}, would end at {body + size:#x} > {end:#x}]")
            break
        if tag == b"FORM":
            ftype = data[body:body+4]
            print(f"{' ' * depth}FORM {ftype.decode('ascii','replace')!r}  "
                  f"@ {off:#x}  size {size:#x} (body {body:#x}..{body + size:#x})")
            walk(data, body + 4, body + size, depth + 2, chunks)
        else:
            print(f"{' ' * depth}{tag.decode('ascii','replace')!r:8}        "
                  f"@ {off:#x}  size {size:#x}  body {body:#x}..{body + size:#x}")
            chunks.append((tag, body, size))
        # IFF pads odd sizes to 16-bit alignment.
        off = body + ((size + 1) & ~1)


def analyze_chunk(data: bytes, tag: bytes, body: int, size: int) -> None:
    if size == 0:
        return
    # Heightmap heuristic: u16-array where every value masked 0x7ff
    # is non-pathological. Sample at most ~256 u16s.
    if size < 0x100:
        return
    n_samples = min(256, size // 2)
    over_11bit = 0
    for i in range(n_samples):
        v = struct.unpack_from("<H", data, body + i * 2)[0]
        if (v >> 11) != 0 and (v >> 11) != 0x1f:
            over_11bit += 1
    if over_11bit < n_samples * 0.2:
        # Most values fit 11-bit; could be heightmap.
        print(f"    -> 11-bit-friendly: {over_11bit}/{n_samples} samples "
              f"exceed 11 bits, size {size} ({size // 0x2000} 8KB chunks)")


def main():
    if len(sys.argv) != 2:
        print("Usage: exp_dump.py path/to/OILFIELD.EXP")
        sys.exit(2)
    path = sys.argv[1]
    if not os.path.exists(path):
        print(f"not found: {path}")
        sys.exit(2)
    with open(path, "rb") as f:
        data = f.read()
    print(f"file size: {len(data):#x} ({len(data)} bytes)")
    chunks = []
    walk(data, 0, len(data), 0, chunks)
    print(f"\nNon-FORM chunks: {len(chunks)}")
    by_tag = {}
    for tag, body, size in chunks:
        by_tag.setdefault(tag, []).append((body, size))
    print("Chunk tag inventory (count, total size):")
    for tag in sorted(by_tag):
        entries = by_tag[tag]
        total = sum(s for _, s in entries)
        print(f"  {tag.decode('ascii', 'replace')!r}: {len(entries):3d} chunk(s)  "
              f"{total:#x} bytes total")
    print("\n11-bit heightmap candidates:")
    for tag, body, size in chunks:
        if 0x1000 <= size <= 0x10000:
            print(f"  {tag.decode('ascii','replace')!r} @ {body:#x} size {size:#x}", end="")
            analyze_chunk(data, tag, body, size)
            print()


if __name__ == "__main__":
    main()
