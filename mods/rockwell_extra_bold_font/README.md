# Approved Rockwell Extra Bold font

> Superseded on 2026-09-05 by `mods/rockwell_regular_font`. This package is
> retained as a reviewed candidate and is not the active preferred font.

Source: locally installed `C:/Windows/Fonts/ROCKEB.TTF`, selected by the user.
The source TTF is not bundled. `build.json` records its hash and compilation settings.

Build / install from the repository root:

```
python tools/recompone-v8-2/build_rockwell_extra_bold_font.py --deploy
```

This is a build-source package, not a second competing runtime mod. Installation
updates the existing `V8_2_LOOSE/mods/ttf_game_font` GAME.FNT override and the
matching whole-file entry in `enhanced_textures_2x/manifest.json`. The generated
`fontFiles.json` is that entry, not a complete texture-pack manifest.

The original FNT character table and special-symbol coverage are retained.
The new GAME.FNT uses a 242x90 indexed atlas; its filename-routed 4x DDS is
968x360 with antialiased TTF alpha at the user-requested smaller size
(11 logical pixels). There are no per-glyph texture hashes or
generated VRAM keys. The existing file-font hook falls back to the original
when no replacement is available. Other font entries, including the cyan
SLOGAN font, are not changed.

GAME.FNT is shared, so this changes its users outside the pause menu too.
Full-menu layout review remains open. The original assets are preserved;
the installer backs up the individual active files under
`artifacts/pause-style-20260904/font-backup` before replacing them.

Pause-panel styling is runtime-drawn through the native common modal-builder
seam, separately from this font package. It preserves native text, selection,
input and transitions. See `notes/v82_pause_style.md`.
