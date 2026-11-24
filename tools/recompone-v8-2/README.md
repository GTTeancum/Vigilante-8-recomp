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

## Original V8 guest-engine audio

A new host alone does not change an installed original-roster mod. To use the
original engines, **keep a copy of the complete old package**, including its
selector portraits. Generate two updated files in a separate, *new* directory:

```powershell
py -3 tools/recompone-v8-2/upgrade_guest_engine_audio.py --package "C:\V8\original-roster" --disc-bin "D:\V8\Vigilante 8 (USA) (Rev 1) (Track 01).bin" --output "C:\V8\engine-pair"
```

Use Python 3.10+ and the original US V8 game's MODE2/2352 data track, **not**
2nd Offense. Alternatively, pass `--arms "D:\V8-original\HUD\ARMS.EXP"`
instead of `--disc-bin`. The updater reads the installed pair and writes
only `CUSTOM.EXP` and `VEHICLES.V8R` to its output. Copy **both** into the
copy of the complete package and point the **updated host** at that copy via
`RECOMPONE_V82_VEHICLE_PACKAGE`. Do not run the output pair as a new empty
mod, overwrite the media, or mix old and new files. Unknown or non-round-
trippable packages are rejected rather than rewritten.

The source-only distributable updater works without a checkout or Blender:
build it with `python tools/recompone-v8-2/build_guest_engine_updater.py OriginalEngineAudio.pyz`;
on the player's machine, use `py -3 OriginalEngineAudio.pyz` with the same
arguments. The archive contains only Python source, no disc or generated data.

The engine lives in each original driver's HUD/ARMS form, not sample zero of
the COMMON/body bank used for weapons. The converter appends the loop (native
pitch and ADPCM unchanged) and records its index in bits 24–31 of the V8:2
entry flags. Zero retains previous behavior. Only the engine key-on in
`80038408` consults the mapping; stock vehicles and transformed modes keep their
native audio. This change is unrelated to tire squeal and music.
