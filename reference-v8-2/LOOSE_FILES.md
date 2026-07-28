# Vigilante 8: 2nd Offense standalone loose files

`Vigilante82PC.exe` supports the sequel without opening a BIN/CUE. The deployed
layout is:

```text
V8_2_LOOSE/
  Vigilante82PC.exe
  SYSTEM.CNF
  SLUS_008.68
  SHELL/
  SHARED/
  LEVELS/
  MOVIES/
  music/
```

With no command-line arguments, the executable uses this directory as its
loose root. The standalone deployment does not require or probe a neighboring
BIN/CUE.

The loose tree is also the asset-modding surface. Replace a file at its exact
retail path and relaunch; no executable or disc-image rebuild is needed.
Ordinary assets retain their logical file bytes. `.STR` and `.XA` files retain
2336 bytes per Mode 2 sector so movies and streamed speech keep their sector
subheaders. `music/track02.ogg` through `music/track17.ogg` provide the Red Book
tracks at 44.1 kHz.

Standalone mode validates every required path at startup and never falls back
to a CUE. Useful launch forms are:

```text
Vigilante82PC.exe
Vigilante82PC.exe <loose-directory>
Vigilante82PC.exe --loose <loose-directory>
Vigilante82PC.exe <disc.cue>
Vigilante82PC.exe --no-loose
Vigilante82PC.exe --probe-source
Vigilante82PC.exe --probe-file <retail-path>
```

To regenerate the complete tree from a legally obtained USA disc:

```powershell
python tools/recompone-v8/prepare_loose_media.py `
  --cue "V8_2_BINCUE/Vigilante 8 - 2nd Offensive [U] [SLUS-00868].cue" `
  --manifest tools/recompone-reference/RecompOne.Runtime/Cdrom/V82LooseManifest.json `
  --loose-root "V8_2_LOOSE"
```

The preparation step requires `ffmpeg` to encode the 16 CD-audio tracks.
