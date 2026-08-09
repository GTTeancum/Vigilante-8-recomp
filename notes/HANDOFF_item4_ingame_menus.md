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

## Phase 2: the row selects, and the Video page runs

Done and verified. `func_8010EA88` is the Options screen's outer loop. It
redraws the row list, dispatches the selected index through a **seven-entry
jump table at `0x80101180`** guarded by `sltiu v0, s1, 7` at `0x8010EC58`, then
reads the pad to move the cursor. `S1` is the selected index throughout.

| index | page |
|-------|------|
| 0 | `func_80108D90` Game Status |
| 1 | `func_8010CF98` Memory Card |
| 2 | `func_80109C60` Difficulty |
| 3 | `func_8010A5BC` Controllers |
| 4 | `func_8010B84C` Audio |
| 5 | `func_8010EA60` Back Story |
| 6 | `func_8010EA38` Credits |

Two compiled constants confine the screen to seven rows, and each needs its own
seam:

- `slti v0, s1, 6` at **`0x8010ED20`** clamps the cursor. That is a *delay
  slot*, and `FunctionEmmiter` skips delay slots in its emit loop, so an inline
  hook declared on it is never emitted. `V82NativeVideoOption.ExtendCursorRange`
  goes on the `and v0, v0, v1` at **`0x8010ED18`** instead, where `V0` is
  non-zero exactly when the next-row input is down. Stepping the cursor onto row
  seven there leaves the retail clamp to decline a second step, so the two never
  both fire -- and it needs no knowledge of which pad bit means "down".
- `sltiu v0, s1, 7` at **`0x8010EC58`** guards the table.
  `V82NativeVideoPage.Dispatch` is an inline hook there with
  `branchTo: 8010ECE0`, claiming the row and skipping the retail dispatch.

### The outer loop is not a frame loop

The thing that cost the most time. `func_8010EA88` contains no VSync: **every
retail sub-page owns its own inner frame loop** and returns only when the pad
carries a bit that moves the row cursor or leaves the screen. Audio's runs
`0x8010B9AC`-`0x8010C17C`; Back Story/Credits' runs inside `func_8010E854` and
exits on `0x50900000`. Branching past the dispatch without supplying a loop
does not crash -- the game simply stops presenting, `HostWindow.Present` is
never reached, scripted input freezes and it looks like a hang.

The minimal pump a settings page turns on is two calls:

```
func_80054C4C(a0 = 0)   VSync -- this is what reaches Runtime.PresentFrame
func_80015540()         per-frame pad service; returns the processed word
```

### Processed pad word bit order

`0x8006B4EC` does **not** use `Controller`'s bit order in either half. Observed
directly: pressing Triangle yields `0x00100010`, so the shell's bit 4 is
Triangle. The masks that matter, in shell order:

| mask | meaning |
|------|---------|
| `0x10000000` | previous row |
| `0x40000000` | next row |
| `0x00900000` | leave Options (bit 4 is Triangle) |
| `0x50900000` | the three together -- what a sub-page exits on |

Read `Controller.State` for the page's own controls, as the controls page does,
and use these only for handing the frame back.

### Shape of the page

`V82NativeVideoPage` browses and edits:

- **Browsing** draws the settings read-only and returns on `0x50900000`, so the
  row cursor behaves exactly as it does on every retail row. CROSS enters
  editing.
- **Editing** owns up/down/left/right, consumes the handled bits out of the
  processed word so the outer loop cannot also act, and TRIANGLE returns to
  browsing. Because TRIANGLE is also the leave-Options bit, returning disarms
  the exit until the mask is seen clear -- otherwise the same press does both.

Sixteen rows, six visible, scrolled with an "N more below" hint. Verified
headless with `input-scripts/native_options_video_row.txt`.

## Row plates: the missing one is at the top

The capture settles it. With `START_Y=138` the eight text rows land at
138,172,...,376 and the seven plates stay at the retail 172,...,376, so rows
two through eight sit on plates one through seven and **`Game Status` is the
row without a plate** -- not `Video`. `Game Status` also overlaps the V8:2 logo
at that start Y. Both go away together if the logo shrinks and the list returns
to its retail span, but an eighth plate is still needed either way.

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

## Two mechanisms were fighting over the same lines

Phase 0 declared three seams in the manifest that the build-time Python scripts
already owned. The scripts anchor on generated text, so once the recompiler
emitted its own version of a line neither anchor matched and the script raised.
That was invisible because the staged `Vigilante82PC.csproj` is gitignored and
was being overwritten on every regeneration by the `reference-host` template,
whose relative paths did not resolve from the staged directory -- so the
pre-build scripts were simply not running. Fixed together:

- Both csproj forms now use `..\..\..\tools\recompone-v8-2\`, which resolves
  from the template's directory *and* from the staged one (both sit three
  levels below the repo root, which is what the `ProjectReference` already
  relied on). The `Exec` paths too.
- `ResolveNativeSelectorSlot` returns the slot and the caller must store it to
  `FP`. An inline patch emits a bare call, so declaring the resolver directly
  **silently discarded the value** and the packaged-roster carousel seam did
  nothing. `ApplyNativeSelectorSlot` is a void wrapper that stores it; the
  manifest points at that and the script no longer touches L80106800.
- `TryDraw` and `UpdateState` are correctly declared, so
  `apply_native_options_patches.py` no longer applies them.
- `PreHookTarget` is a single field: two `pre` patches on `func_8001A3B0`
  meant the second silently replaced the first, and
  `OverrideNativeSelectorText` was being lost. Only that one is declared now;
  the script appends `TraceNativeOptionsText` as the second pre-hook.

### The guard, and the two more it found

`OverlayWriter.ApplyPatches` now warns when a `pre` or `post` patch replaces a
target already set on the same function. It found two more live cases on its
first run:

- **`func_8001BECC`** carried the six widescreen terrain-traversal hooks
  *twice*: once as the inline seams phase 0 added, and once as the older `pre`
  entries it forgot to delete. Five of those six were being discarded and the
  sixth was running an extra time. Behaviour was accidentally correct because
  the survivor, `TraceTerrainTraversalPolygon`, is not in the inline set.
  Duplicates deleted; only the trace stays `pre`.
- **`func_8002D9E0`** had `BeginObjectRender` silently replacing
  `RepairObjectTerrainQuery`, so that hook had not run since phase 0.
  `ApplyLevelOfDetail` has exactly one caller and it is inside
  `RepairObjectTerrainQuery`, so the per-object half of the level-of-detail
  setting was dead -- including for the Video page's own "Level of detail"
  row. `BeginObjectRender` is an inline seam now and both run again.

Both hooks there are `void`, and `PreHook.Run(Action, ...)` always returns
true, so moving one to an inline seam changes nothing except which hooks run.
Not verified in gameplay: neither Debug nor Release headless reached a match
within ten minutes on `native_chassey_gameplay_camera_proof.txt`, which stops
at `choose_enemies`. This restores the pre-phase-0 state rather than
introducing anything, and the shell and selector paths run clean.

## Row plates are art, not code -- settled

The producer is not in the shell overlay. Ruled out:

- The two `func_8002DE84` calls, the strongest earlier candidates, are
  **lights**: their pointer arguments hold 4.12 unit vectors (`0x801006DC` is
  `(0, 0, -4096)`, `0x801009B8` is `(0, -4096, 0)`) and the third argument is
  the colour. Nothing to do with plates.
- No table of Y values stepping 34 from 172 exists anywhere in SHELL.DLL, at
  any stride -- scanned exhaustively as halfwords.
- No seven-iteration draw loop exists other than the row loop itself. The only
  other `< 7` comparisons in the overlay are the jump-table bound, the pages'
  own loops, and a glyph-class helper.
- The plates skew and scale with the panel during the screen transition, so
  they are geometry or texture on the transformed 3D panel, not a 2D backdrop.

So an eighth plate needs an art change or a 2D quad that only lines up when the
panel is at rest. That fits the intent to shrink the V8:2 logo: worth doing in
one pass with the layout rather than bolting on a primitive now.

Incidental: the jump table at `0x80101180` is immediately followed by the
string `Shell\Cursor.PSX`, so there is no room to extend it in place -- the
hook was the only option.

## Next step

**The list layout and the eighth plate, together with the logo art.** The plate
producer is settled (see above): it is art, so the row list, the logo and the
plates want one combined pass rather than a code-side patch. Until then
`RECOMPONE_V82_OPTIONS_START_Y=138` leaves `Game Status` plateless and
overlapping the logo.

Then: **trim the curated option set** (below), and examine the Audio page for
whether master volume and mute fit on it -- still the only thing that could
force a ninth row.

## Curated option set (built as the draft; user still to trim)

The page ships the draft "in" list, sixteen rows in this order: Preset,
Resolution, Fullscreen, Widescreen, Internal 3D, Anti-aliasing, MSAA,
Anisotropic, Texture smoothing, Mipmaps, Level of detail, Draw distance, Fog,
Shadows, Particles, HUD anchoring. Audio page (not built): master volume, mute.

Every row maps onto an existing `ViewConfig` property and saves through
`ConfigManager.SaveView`, so a change made here and one made in the ImGui
Display section take the identical path. Resolution and fullscreen apply live
through `HostWindow.SetOutputResolution` / `SetFullscreen`, reached from the
recompiled project through new public wrappers on `V82Compat` because
`HostWindow` is internal.

Out (diagnostics): dithering, geometry correction, precise culling,
perspective-correct textures/colours, enhanced depth buffer. Borderline and
unconfirmed: true-colour output, vector fonts, vector icons, high-resolution 3D
(the last is currently driven implicitly by Internal 3D > 1x).

Trimming is one edit to the `Rows` array; six rows are visible at a time, so
dropping four would remove one page of scrolling.

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
