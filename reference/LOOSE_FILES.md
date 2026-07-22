# Standalone loose-file run and asset mods

The same `Vigilante8PC.exe` is deployed to two independent locations. Neither
copy needs a command script.

- `BINCUE/Vigilante8PC.exe` finds the neighboring Vigilante 8 CUE and runs from
  the original disc tracks.
- `PS1 game/Vigilante8PC.exe` sees the neighboring `SYSTEM.CNF` and runs only
  from the extracted tree. It does not search for, open, or fall back to
  `BINCUE`, even if that directory exists.

The executable files remain byte-identical. Their neighboring data selects the
source mode. A loose launch fails with a list of missing or malformed assets
instead of silently borrowing bytes from a disc image.

## Asset replacement

The loose tree is the asset-modding surface. Replace a file at its exact retail
path, then relaunch the game; rebuilding a BIN or the executable is unnecessary.
Examples include:

```text
COMMON.EXP
HUD/ARMS.EXP
SHELL/VEHICLES.EXP
TERRAIN/OILFIELD.EXP
```

The file's current bytes and logical length are authoritative. Extra files are
ignored unless a later mod system explicitly uses them, and moving a required
asset elsewhere does not satisfy its retail path. Replacing MIPS instructions
inside `SLUS_005.10` or a `.DLL` does not create newly recompiled host code;
their data portions can still be replaced.

`VIDEO/*.STR` and `SOUNDS/*.XA` use 2336-byte Mode 2 sectors so their subheaders
and complete movie/XA payloads remain available without a BIN. Each such file
must have a length divisible by 2336. `music/*.ogg` supplies the twelve Red Book
tracks by name at 44.1 kHz; the zero-byte `REDBOOK/*.RAW` files are retained only
as visible retail-path placeholders.

For a fresh legally obtained extraction, prepare those stream files and the
metadata-only built-in layout with:

```powershell
python tools/recompone-v8/prepare_loose_media.py `
  --cue "BINCUE/Vigilante 8 (USA).cue" `
  --manifest tools/recompone-reference/RecompOne.Runtime/Cdrom/V8LooseManifest.json `
  --loose-root "PS1 game"
```

The CUE is an input to this one-time preparation command. It is not read by the
finished loose deployment.

Optional launch and diagnostic forms are:

```text
Vigilante8PC.exe
Vigilante8PC.exe <loose-directory>
Vigilante8PC.exe --loose <loose-directory>
Vigilante8PC.exe <disc.cue>
Vigilante8PC.exe --probe-source
Vigilante8PC.exe --probe-file <retail-path>
```

The executable defaults Vigilante 8's own Music and Sound Effects sliders to
zero, matching the former launcher behavior without altering Windows volume.
