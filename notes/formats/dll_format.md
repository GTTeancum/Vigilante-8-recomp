# DLL File Format (Vigilante 8 PSX Overlay)

Not a Windows DLL. V8's `.DLL` files are PSX **overlay** modules:
position-independent code + data loaded into a fixed memory region by the
main EXE at level / shell load time.

## Observed headers

### TERRAIN/AIRGRAVE.DLL (6504 bytes)
```
00: c4 17 00 00   0x000017c4   header field 0 (likely total file size or text-segment size)
04: 08 00 00 00   0x00000008
08: 30 00 00 00   0x00000030
0C: ec 00 00 00   0x000000ec
10: 3c 00 00 00   0x0000003c
14: 28 02 00 00   0x00000228
18: 5c 00 00 00   0x0000005c
1C: 70 0c 00 00   0x00000c70
20: b8 00 00 00   0x000000b8
24: d0 14 00 00   0x000014d0
28: 00 00 00 00
2C: 00 00 00 00
30: "AirGrave"
3C: "Cranberry..."
```

After the offset/size table, ASCII identifiers begin (level codename plus
ingame display name). The first u32 is plausibly the offset to the code
section's start (file_size 6504 ~ 0x1968; field 0x17c4 < 0x1968, so it
could be a code-section size). Need cross-reference to the EXE loader to
disambiguate.

### SHELL/LOAD.DLL (33820 bytes)
Pure pointer table at start, no obvious strings until later. Likely a
table of function entry points for the loader overlay.

### SHELL/SHELL.DLL (93588 bytes)
```
00: a0 35 01 00   0x000135a0
04: 84 c7 00 00   0x0000c784
08: "Terrain\\SkiResrt.exp\0"
   ...           further "Terrain\\<name>.exp", level display names, etc.
```
First two u32s look like load-address + size pair. Then a string pool of
file paths and level names referenced from shell code.

## Plan

1. Find the overlay loader function in the EXE (after Ghidra analysis).
   Look for: reads from CD, copies bytes to a fixed VA, then calls a
   function pointer near the head of the loaded region. Likely calls
   `CdRead` / `CdControl`.

2. Once header layout is known, write `tools/dll_dump.py` to extract:
   - load VA
   - text / data / bss sizes
   - relocation table (if present)
   - exported function table

3. Import each DLL into Ghidra as a separate program (PSX raw-binary
   loader with the recovered base VA) for full disassembly.
