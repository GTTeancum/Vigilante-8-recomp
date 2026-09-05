# Vigilante 8 Classic Menu

This loose V8: 2nd Offense mod replaces both native front-end backplates with
the supplied 640x480 Vigilante 8 Classic artwork: the title/save-check image
and the actual `1 PLAYER / 2 PLAYER / OPTIONS` menu image. It also replaces
the native Options screen's 240x421 left panel with the supplied Vigilante 8
Classic wrench artwork.

The front-end backplates use the generic content-addressed decoded-image
replacement path. The Options panel is an asset-direct override of native
`SHELL/OPTTABLE.TBL` record 58; every other table record remains byte-identical
to retail.

## Editable sources and reproducible build

`source/main_menu.png` is the editable 640x480 source artwork.
`source/options_left.png` preserves the supplied 224x480 Options artwork
verbatim. Rebuild the runtime assets from the repository root with:

```powershell
$env:PSXAVENC = "C:\path\to\psxavenc.exe"
python tools/recompone-v8-2/build_v8_classic_menu_mod.py `
  --psxavenc $env:PSXAVENC
```

The builder converts the main-menu PNG to the uncompressed RGBA
`images/main_menu.dds`, validates every manifest crop, encodes the Options
source as PlayStation MDEC BS v2 inside
`files/SHELL/OPTTABLE.TBL`, and invokes the shared cursor converter to
regenerate `files/SHELL/CURSOR.PSX`. The MDEC encoder is the open-source
`psxavenc` v0.3.1 or newer. The cursor conversion reads original V8
`PS1 game/SHELL/CURSOR.BIN` and uses retail
`V8_2_LOOSE/SHELL/CURSOR.PSX` only as the V8:2 shell-layout template.

`build_v8_options_backplate.py` verifies the retail record number and native
240x421 dimensions, preserves 58 unrelated records exactly, extends the final
visible row through the encoder's hidden macroblock rows, and zero-pads the
encoded record tail to keep the complete table at its retail sector length.
There is no VRAM capture, screen-address match, or runtime Options exception.

`files/SHELL/CURSOR.PSX` is a direct loose-file override of V8:2's shared
front-end cursor bank. Root 0 retains the native V8:2 USA-map marker and its
dependency closure. Root 1 contains the original V8 tire converted once into
V8:2's native render-packet dialect. This preserves the two-slot shared-shell
contract without a per-menu hook, a smiley selector, or an unintended tire on
the location map.
