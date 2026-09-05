# Preferred Rockwell GAME.FNT

This restores the previously used `ROCK.TTF`-based font at 14 logical pixels.
It is the sharper face preferred by the user after reviewing Rockwell Extra
Bold in the pause menu.

Build and install from the repository root:

```text
python tools/recompone-v8-2/build_rockwell_regular_font.py --deploy
```

The source TTF is the locally installed `C:/Windows/Fonts/ROCK.TTF`; it is not
bundled. `build.json` records its hash and compilation settings.

Deployment updates the existing `V8_2_LOOSE/mods/ttf_game_font` override and
the `SHARED/GAME.FNT` whole-file entry in the Enhanced texture pack. The
renderer resolves the font by file provenance, so the same replacement covers
every pause-menu line regardless of position. Missing sidecars still fall back
to the original asset, and the cyan SLOGAN font remains unchanged.

The modal panel, menu logic, selection, input, and transitions are separate
from this asset package and remain untouched.
