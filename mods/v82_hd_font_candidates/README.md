# V8:2 HD font candidates

This directory contains review-only 4x whole-file replacement candidates for
the retail V8:2 `.FNT` files selected for HD replacement:

- `SHARED/HUD.FNT` — compact in-match digits and symbols.
- `SHARED/KONG.FNT` — heavy block diagnostic and exception-screen text.

`SHARED/GAME.FNT` and the two font regions inside `SHELL/LOAD.TBL` already have
deliberate HD assets and are not duplicated here. `SHELL/SLOGAN.FNT` is the
cyan font and is intentionally excluded: it keeps its untouched retail atlas.
All replacement candidates are non-cursive and non-italic. Any later font
replacement must follow that same rule.

Each candidate directory contains a transparent 4x PNG, a renderer-ready DDS,
and a full-resolution proof that compares the untouched retail atlas against
the candidate. Nothing in this directory is deployed automatically. Selection
must be explicit, after visual review, and deployment must use the existing
shared filename-provenance hook. Fonts without an approved sidecar continue to
fall back to their untouched retail atlas.

The pause-menu example is primarily `GAME.FNT`, which already has an HD
sidecar. Its small all-caps strings remain visibly coarse because that pause
overlay is presented at a smaller raster size than the larger mixed-case menu
choices; replacing `HUD`, `KONG`, or `SLOGAN` will not by itself correct that
separate presentation-scale issue.

Regenerate all candidates with:

```powershell
python tools/recompone-v8-2/build_remaining_fnt_candidates.py
```

The machine-readable inventory and candidate parameters are in `index.json`.
