# Handoff — item 4, in-game menus

Agreed shape (decided with the user): **retail-native styling**, a **curated
player-facing** option set, reachable from the **main-menu Options**, with
resolution and fullscreen **applying live, no confirmation timer**.

## Done

**Phase 0, complete and verified.** Generated code is gitignored and rebuilt
from the disc, so hand edits to it do not survive. A new `inline` patch mode
places a call beside a named *instruction* (with an optional `branchTo` that
emits `if (target(c, m)) goto L...;` so a hook can take over a routine).
Fourteen hooks that existed only as hand edits are now declared, including the
whole PC controls page and the widescreen terrain-traversal chain.

**Regeneration without the disc image.** The retail CUE/BIN is gone. The
recompiler only reads files out of the disc, so it now accepts an extracted
disc directory:

```
python tools/recompone-v8-2/prepare_reference.py --loose-disc V8_2_WORK/disc --output <dir>
dotnet tools/recompone-reference/RecompOne.Recompiler/bin/Release/net10.0/recompone.dll <dir>/v82.recompone.json
```

138 of 139 patches apply. `CueFs.OpenLooseCodeOnly` relaxes two runtime-only
requirements the recompiler has no use for (STR/XA sector form, loose CDDA).

## The Options list, located

Rows are **not** a static table in overlay data and **not** a runtime object
tree either -- that was a wrong turn worth not repeating. `S7` in the Controller
page is heap, but the list itself is simpler:

| what | where |
|------|-------|
| draw loop | `func_80108B48` (SHELL_SHELL) |
| row strings | pointer array at **`0x80115E60`**, 7 entries, stride 4, walked by `S2` |
| row count | hard-coded `c.S0 < 0x00000007u` at the loop tail |
| text call | `func_8001A3B0`, `A3 = 8`, return address `0x80108C70` |

Rows are `Game Status, Memory Card, Difficulty, Controllers, Audio, Back Story,
Credits`. Note the sub-page *titles* are drawn by different functions and in
different casing (`CONTROLLER` vs `Controllers`); do not confuse the two --
`TraceNativeOptionsText` sees the titles, not the rows.

## Next step

The row count is a compiled constant, so an inline hook cannot extend it.
Use `"mode": "replace"` on `func_80108B48` and reimplement the loop in C#:
draw the seven retail strings from `0x80115E60` plus a `Video` row. The
existing page is only ~120 lines of generated code and the text call it uses is
already reachable.

Then find the **selection** handler that maps the chosen row index to a page,
and route index 7 to a new video page. Model that page on
`V82NativeControlOptions` (`tools/recompone-v8-2/V82NativeControlOptions.cs`),
which already does retail-native drawing, cursor, footer prompts and pad input
for the controls page -- it is the working precedent and its seams are now
declared, so it survives a regeneration.

## Curated option set (draft, user to confirm the borderline ones)

In: graphics preset, output resolution, fullscreen, widescreen, internal 3D
resolution, anti-aliasing + MSAA, anisotropic filtering, texture smoothing,
mipmaps, level of detail, extended draw distance, fog, shadows, particles,
HUD anchoring. Audio page: master volume, mute.

Out (diagnostics): dithering, geometry correction, precise culling,
perspective-correct textures/colours, enhanced depth buffer. Borderline and
unconfirmed: true-colour output, vector fonts, vector icons, high-resolution 3D.

## Known risk

Applying resolution and fullscreen live means recreating the window and GL
context while the game holds VRAM state; internal-resolution scale reallocates
render targets. This is the riskiest part of item 4, not the menu drawing.

## Still undeclared (four hooks)

`BeginTextureDecode` captures a local across the call.
`RestoreTerrainFrustum`, `RepairObjectTerrainQuery` and
`OverrideNativeSelectorText` sit where no label or return address appears in
the generated C#, so their addresses must come from the disassembly. A
regeneration still drops these four. `RestoreTerrainFrustum` also declares
address `8001C89C`, which matches no function start -- pre-existing.

## Probes available

`RECOMPONE_V82_TRACE_OPTIONS_TABLE=1` dumps the Options descriptor.
`RECOMPONE_TRACE_NATIVE_OPTIONS=1` logs every native UI string with its caller,
object and layout -- that is what located the list loop.
Drive either with `input-scripts/native_options_discovery.txt`.
