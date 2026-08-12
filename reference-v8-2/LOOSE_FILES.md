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
    HUD/
      v82_radar.svg
      v82_status.svg
      v82_health.svg
  LEVELS/
  MOVIES/
  music/
  mods/
    enhanced_textures_2x/
      manifest.json
      README.md
      images/
        <content-key>.dds
        terrain/
          levels/
            <level-name>.dds
        route/
          levels/
            <level-route-texture>.dds
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

Enhanced HUD backings are literal SVG assets under `SHARED/HUD`. They are
loaded at startup and can be edited without rebuilding the executable. The
renderer supports the circles, lines, rectangles, polygons, and linear/radial
gradients used by these three files. If a loose SVG is absent, the executable
uses its embedded copy.

Enhanced texture replacements are ordinary uncompressed 32-bit DDS files
under `mods/enhanced_textures_2x/images`. Ordinary world textures are 2x.
Terrain is 4x and remains one human-readable DDS per level under
`images/terrain/levels`; for example, Route 66's authored 400x160 XBMP is
`route66.dds` at 1600x640. XRTP route/road payloads decoded from level EXP
records are also 4x and live under `images/route/levels`, with names such as
`route66_xrtp00_tex2.dds`.

The terrain material table selects cells from the per-level terrain source at
runtime--the pack does not duplicate its overlapping tile windows into hundreds
of files. Route textures add full-image and quadrant metadata entries so route
template variants can resolve without duplicating the DDS payloads.
`manifest.json` associates decoded retail content with those human-editable
files and preserves indexed-terrain and XRTP route identity; it is metadata,
not an image archive. The renderer validates and loads the individual DDS files
at startup, then builds a private GPU atlas in memory. Replace a DDS and
relaunch to see the change--there is no disc image, executable, or packed
texture archive to rebuild. The stock-looking in-game `Options > Video >
High-res textures` row enables or disables the pack.

Terrain, route, and vehicle source images are 4x. Other world textures remain
2x. Vehicle ownership comes from the stock SHARED banks, the selector's
`SHELL/VEHICLES.EXP`, and `CUSTOM.EXP`; this keeps gameplay and selector models
at 4x while preserving their original STP/transparency mask in the renderer.

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
