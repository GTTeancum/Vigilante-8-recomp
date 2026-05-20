# DLL Overlay Source Layout

The 12 overlay files in `input/{TERRAIN,SHELL}/*.DLL` are
position-independent PSX MIPS code. After `tools/dll_preproc.py`
relocates them to a synthetic base of `0x80100000` and Ghidra
auto-analyses each, their function lists live under
`analysis/dll/<NAME>/`.

Per the charter, in-scope subsystems from these overlays are:

- **Terrain DLLs (per-level)**: per-object physics + AI for that
  level's destructibles, traps, level-specific events. -> goes to
  `src/physics/<level>/` (eventually).
- **SHELL.DLL**: character select, level select, options menu UI.
  Mostly out-of-scope (UI) but includes the *vehicle constructor*
  (where the Vehicle struct gets allocated and initialized -- pass 2
  must locate it here to finalise the struct).
- **LOAD.DLL**: shared MDEC video player + the per-level loader entry
  for terrain DLL load + the level-select TBL. The video player is
  out-of-scope; the level loader logic is in-scope (asset routing).

## Per-DLL clean C: pass 2 onwards

For now the per-DLL Ghidra output (`analysis/dll/<NAME>/decomp/*.c`)
serves as the pass-1 record. Pass 2 will:

1. Add explicit entry-point hints for the four DLLs whose first-pass
   analysis found zero functions (SHELL, SKIRESRT, VALLYFRM, WILDWEST)
   via `tools/ghidra_scripts/MarkDllEntry.java`.
2. Reroute every cross-overlay `func_0x80XXXXXX` reference to its
   resolved main-EXE / DLL name.
3. Promote AIRGRAVE's `FUN_8010068c` (the homing tracker) into
   `src/physics/airgrave/tracker.c` as the first per-level physics
   exemplar.
4. Find the Vehicle allocation in SHELL.DLL once its functions are
   visible, close the struct size.
