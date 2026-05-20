# Setup — First-Run Bootstrap

Run these steps once at the start of the project. Mark complete in `progress.log` and do not repeat on subsequent sessions.

## 1. Verify Inputs

Check `input/` for:
- The Vigilante 8 PS1 main executable (typically `SLUS_007.??` or similar, extracted from the ISO).
- Any overlay files (`*.OVL`, `*.OVR`).
- Disc data files (`*.BIN`, `*.DAT`, `*.PAK`, etc.) — anything that isn't audio/video.

If the main EXE is not present, log to `progress.log` as `[SETUP] [BLOCKER] [missing main executable in input/]` and stop. This is one of the three valid stop conditions.

If only the EXE is present, proceed. Log missing-but-expected files only if the binary's code references files not in `input/` later in analysis.

## 2. Install Ghidra

Download the latest stable Ghidra release from the official GitHub releases:
```
https://github.com/NationalSecurityAgency/ghidra/releases
```

Pick the latest `_PUBLIC` release zip. Extract to `ghidra/ghidra_install/`. Verify by running:
```
ghidra/ghidra_install/support/analyzeHeadless -help
```

If that succeeds, Ghidra is ready.

## 3. Install PSX Loader Plugin

Ghidra does not natively recognize the PS-X EXE format. Install the PSX loader:
```
https://github.com/lab313ru/ghidra_psx_ldr
```

Download the latest release zip matching the installed Ghidra version. Install via:
```
ghidra/ghidra_install/support/analyzeHeadless ... -preScript InstallExtension.java <plugin.zip>
```

Or place the extension zip in `ghidra/ghidra_install/Extensions/Ghidra/` and accept on first launch.

Verify by checking that "Playstation PS-X EXE" appears in the loader list:
```
ghidra/ghidra_install/support/analyzeHeadless . tmpproj -import input/SLUS_007.??  -loader PsxLoader -noanalysis
```

If the loader is recognized, plugin is installed. Delete the tmpproj after.

## 4. Create Working Project

Create the Ghidra project:
```
ghidra/ghidra_install/support/analyzeHeadless ghidra/projects v8 -import input/<main_exe> -loader PsxLoader -noanalysis
```

This creates `ghidra/projects/v8.gpr` and `ghidra/projects/v8.rep/` without running auto-analysis yet (that's the next phase).

For overlay files: import them separately with the same loader. Some may need manual base-address specification — log to `decisions.log` if a base address is chosen heuristically.

## 5. Run Auto-Analysis (IMPORT/ANALYZE phase)

This is the long-running step. Run:
```
ghidra/ghidra_install/support/analyzeHeadless ghidra/projects v8 -process <program_name> -postScript ExportAllToDisk.java analysis/
```

`ExportAllToDisk.java` is a custom post-script that exports:
- `analysis/functions.json` — every function: address, name (placeholder), size, parameter count
- `analysis/strings.txt` — all strings with addresses
- `analysis/xrefs.json` — cross-references for every function
- `analysis/decomp/<address>.c` — Ghidra's pseudo-C for every function
- `analysis/mips/<address>.s` — raw MIPS disassembly for every function

If `ExportAllToDisk.java` does not yet exist, write it as the first action of the ANALYZE phase. It uses Ghidra's Java scripting API (FlatProgramAPI, DecompInterface).

## 6. Verify Analysis Output

After analysis completes:
- Confirm `analysis/functions.json` has hundreds or thousands of entries (V8 will have many).
- Confirm `analysis/strings.txt` is non-empty.
- Spot-check 3 random `analysis/decomp/*.c` files for non-empty content.

If any of these fail, retry analysis once. If it fails again, log `[SETUP] [BLOCKER] [Ghidra analysis failed]` and stop.

## 7. Mark Setup Complete

Append to `progress.log`:
```
[ISO timestamp] [SETUP] [COMPLETE] [Ghidra installed, PSX loader installed, project analyzed, N functions exported]
```

Proceed to the CLASSIFY phase per `CLAUDE.md`.

## Notes

- Network access required for steps 2 and 3. Both GitHub. If network is restricted, log and stop.
- Ghidra analysis is single-threaded for some phases and can take 10-60 minutes on a PS1-sized binary. This is normal. Do not interrupt.
- If the EXE has overlays loaded at the same VA, treat each as a separate Ghidra program and analyze independently. Track which functions belong to which overlay in `analysis/overlay_map.json`.
