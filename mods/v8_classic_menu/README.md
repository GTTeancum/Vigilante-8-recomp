# Vigilante 8 Classic Menu

This loose V8: 2nd Offense mod replaces both native front-end backplates with
the supplied 640x480 Vigilante 8 Classic artwork: the title/save-check image
and the actual `1 PLAYER / 2 PLAYER / OPTIONS` menu image.

The runtime matches the decoded native image uploads by content, dimensions,
and VRAM destination. If the expected retail image is not present, the mod
does nothing and the original image remains intact.

## Editable sources and reproducible build

`source/main_menu.png` is the editable 640x480 source artwork. Rebuild both
runtime assets from the repository root with:

```powershell
python tools/recompone-v8-2/build_v8_classic_menu_mod.py
```

The builder converts the PNG to the uncompressed RGBA
`images/main_menu.dds`, validates every manifest crop, and invokes the shared
cursor converter to regenerate `files/SHELL/CURSOR.PSX`. The cursor conversion
reads original V8 `PS1 game/SHELL/CURSOR.BIN` and uses retail
`V8_2_LOOSE/SHELL/CURSOR.PSX` only as the V8:2 shell-layout template.

`files/SHELL/CURSOR.PSX` is a direct loose-file override of V8:2's shared
front-end selector model. It contains the original V8 tire cursor converted
once into V8:2's native render-packet dialect. The converted file preserves
V8:2's two-slot shared-shell contract and supplies the tire to both selector
roles, so every menu that loads the shared cursor asset receives the
replacement without a per-menu hook.
