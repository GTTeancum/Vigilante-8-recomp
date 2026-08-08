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

## Correction to the phase 0 premise

The claim that the controls page was one regeneration from being lost was
**wrong**, and the commits b83ee34 / 7671d2b overstate it. `Vigilante82PC.csproj`
runs Python scripts before every compile that re-apply patches to the generated
sources, and they are idempotent (`if new in text: return 0`):

```
apply_vehicle_engine_patches.py    main.cs          (BeginTextureDecode)
apply_native_selector_patches.py   SHELL_SHELL.cs   (ResolveNativeSelectorSlot)
apply_native_options_patches.py    both             (TryDraw, UpdateState,
                                                     OverrideNativeSelectorText)
```

So five hooks were already protected. The phase 0 work is still net correct for
the rest: the terrain-traversal chain, the late/sky/scenery pass brackets,
`RecordPoolLink` and `RecordObjectVisibility` appear in no script and genuinely
had no durable representation. Declaring hooks in the manifest also beats text
substitution at build time. And because the scripts are idempotent, the overlap
between the two mechanisms does not double-apply.

## Phase 1: the VIDEO row draws

Done and verified. `V82NativeVideoOption.AppendRow` is an inline hook at
`0x80108C90`, the loop exit of `func_80108B48`, where the text object, layout
rect and selection index are all still live. It writes a "Video" string to
scratch at `0x8011AE80`, mirrors the loop's own colour selection so the row
highlights and greys like the retail seven, and draws through the same
`func_8001A3B0` call.

Verified by regeneration (the hook lands exactly at the loop exit) and at
runtime: the row list now reads `Game Status, Memory Card, Difficulty,
Controllers, Audio, Back Story, Credits, Video`.

Note both `Vigilante82PC.csproj` files needed the new file added -- the staged
one and the host template in `reference-host/`, which uses a different relative
path.

## Row plates are a separate draw -- correction

An earlier version of this document, and commit 2958f56, claimed the row plate
and its text come from one call and that the eighth row's plate was merely
clipped. **That is wrong.** Plate and text are drawn independently.

What misled it: the retail list starts at Y=172 with a stride of 34, and the
first attempt lifted the start to 138. `172 - 34 == 138`, exactly one stride,
so every text row landed on the *previous* row's plate. Rows two through eight
looked perfect, `Video` sat on the `Credits` plate, and only `Game Status` had
no plate above it -- which reads exactly like a clipped top edge. Setting
`RECOMPONE_V82_OPTIONS_START_Y=158`, which is not offset by a whole stride,
separates them visibly: text high, plates low, every row struck through.

So the true state is **seven plates at fixed positions and eight text rows**.
An eighth plate has to be drawn.

When debugging this, keep the start Y off a multiple of the stride or the
coincidence hides the problem again.

## Where the plates come from

Not yet identified. The row loop issues exactly one call per row
(`func_8001A3B0`, the text), so the plates are emitted before it, by one of:

```
func_80019294   func_8001AAFC   func_8002DE5C   func_8002DE84 (x3)
```

The three `func_8002DE84` calls take colour-like arguments
(`0x00FF0000|0xFFFF`, `0x003F0000|0x3F3F`, `0x00400000|0x4080`) and pointers at
`0x801006DC` and `0x801009B8`, so they are the strongest candidates. Whichever
draws the plates will have its own count of seven, in the same shape as the row
loop's, and extending it is the same kind of change as `AppendRow`.

## Layout levers

The row loop's layout rect sits on the stack and is written just before the
loop: X at `[SP+0x10]` (40), start Y at `[SP+0x12]` (172), a width-ish field
at `[SP+0x14]` (16) and the row stride at `[SP+0x16]` (34).
`V82NativeVideoOption.AdjustLayout`, an inline hook at `0x80108C1C`, rewrites
the start Y and stride once before the loop. Both are tunable without a
rebuild:

```
RECOMPONE_V82_OPTIONS_START_Y      default 138 (retail 172)
RECOMPONE_V82_OPTIONS_ROW_STRIDE   default 34  (retail 34)
```

At 138/34 all eight text rows sit inside the panel, but see the correction
above -- that particular value aligns text against the wrong plates. The user
intends to shrink the V8:2 logo to make room, so the start Y can rise once the
art changes and an eighth plate exists. Tightening the stride is the
alternative if the logo is kept: seven rows spanned 238px, so 29-30 fits eight
in the retail span.

## Scope of this screen

`Video` is the only new row needed. The curated set maps onto existing rows
otherwise: audio settings extend the retail `Audio` row, bindings extend
`Controllers` (already built as `V82NativeControlOptions`). The one thing that
could force a ninth row is the retail Audio page turning out to be unusable for
master volume and mute -- it has not been examined.

When the Video sub-menu is built, present its contents as a bulleted list
first; the user intends to trim it.

## Next step

The row now draws, so what remains is **selection**: find the **selection** handler that maps the chosen row index to a page,
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
