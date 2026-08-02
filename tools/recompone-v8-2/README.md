# Vigilante 8: 2nd Offense RecompOne tools

This directory bootstraps the sequel from the retail USA disc
(`SLUS-00868`) using the proven Vigilante 8 RecompOne runtime.

First extract the Mode 2 data track:

```powershell
python tools/extract_psx_iso.py `
  "V8_2_BINCUE/Vigilante 8 - 2nd Offensive [U] [SLUS-00868].bin" `
  V8_2_WORK/disc --clean
```

Then generate the initial main-executable and overlay configuration:

```powershell
python tools/recompone-v8-2/prepare_reference.py
```

After RecompOne emits the generated C# sources, build or publish the generated
project normally. Its pre-compile target reapplies and validates both narrow
custom-vehicle patch sets:

- gameplay engine seams (wheel bank, stats, upgrades, transformation table and
  transformation bank);
- native selector seams (packaged roster slot, variant and stats).

This validation is intentionally part of every compile so regenerating
`main.cs` or `SHELL_SHELL.cs` cannot silently produce a runnable build with
guest-vehicle integration missing.

The first pass uses call discovery plus linear sweep for the main executable,
shell, loader, 18 arena overlays, and 18 vehicle overlays. Ghidra function maps
will supersede bootstrap discovery as runtime coverage identifies ambiguous
boundaries.

The host-runtime reuse policy is conservative: generic PS1 hardware, BIOS,
GPU, SPU, CD, controller, and GTE support carries forward. Every V8-specific
address patch or state-layout assumption must be re-proven against
`SLUS_008.68` before it is enabled for the sequel.

## Independent native vehicle runtime

V8:2 is the lead vehicle implementation. A package contains an append-only
`VEHICLES.V8R` roster and independently authored native XOBF banks in
`CUSTOM.EXP`; no retail type is replaced and no donor entry or opaque record is
retained. Select an entry by stable identity with:

```powershell
Vigilante82PC.exe --guest-vehicle author.vehicle_id retail-disc.cue
```

Set `RECOMPONE_V82_VEHICLE_PACKAGE` to the directory containing both package
files. The registry scans file-backed bank offsets, loads only the selected
body and transformation banks, and allocates independent stats, powerup, and
four-mode transformation tables.

After the original gameplay VRAM reset, the runtime reserves the selected
banks' native CLUT/image rectangles. The original XOBF texture loader claims
those rectangles during construction, so a guest does not rely on whatever
fragmented space happens to remain after COMMON and arena loading.

## Standalone loose files

The shared `tools/recompone-v8/prepare_loose_media.py` utility now prepares
either game. For the sequel it extracts all 128 logical files, preserves raw
Mode 2 sectors for `.STR`/`.XA`, creates the embedded
`V82LooseManifest.json`, and encodes CD tracks 2-17 to OGG. Deployment and mod
layout details are in `reference-v8-2/LOOSE_FILES.md`.
