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

Done. The first version of this was `V82NativeVideoOption.AppendRow`, an inline
hook at the loop exit `0x80108C90` that drew an eighth row below the retail
seven. Phase 3 replaced it with the scrolling window described below, which
puts every row on a plate, so `AppendRow` and its patch entry are gone.

Note both `Vigilante82PC.csproj` files needed the new file added -- the staged
one and the host template in `reference-host/`. They used *different* relative
paths, which turned out to be a bug in its own right; see below.

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
  the exit until the button is seen released -- otherwise the same press does
  both, and the screen drops to the main menu.

  Arming has to watch `Controller.State`, **not** the processed word: leaving
  editing consumes that word, so it reads clear on the next frame while the
  button is still down, the exit arms, and the frame after that the still-held
  press fires it. That failure looked exactly like "UP exits Options" and cost
  a while to pin on TRIANGLE.

Its own row list rebuild runs every frame (`func_80108B48` direct, not
`func_80108D1C`, which would also reset the footer). The outer loop only draws
the list once per iteration, and this page holds the frame for many.

Sixteen rows, six visible, scrolled with an "N more below" hint. Verified
headless with `input-scripts/native_options_video_row.txt`.

## Row plates: eight rows through seven plates

The plates are art, not code -- see the evidence section below. There is no
eighth plate and none can be produced from the shell. Eight rows do not need
eight plates, though: the row loop draws seven rows from wherever S2 points, so
`AdjustLayout` points it into an eight-entry table of its own and slides the
window by one when the cursor reaches the appended row.

Rows zero through six render **byte-identical to stock** -- verified by
differing the row strip of all seven retail sub-page captures against a stock
build, zero pixels. On row seven the list scrolls a single step: Game Status
leaves the top and Video occupies the last plate, every row on a plate at its
retail position, the logo untouched.

`RECOMPONE_V82_OPTIONS_START_Y` and `_ROW_STRIDE` now default to the retail
172/34 and are only debugging levers.

### The panel is never repainted

This cost the most time after the frame loop. Nothing redraws the Options panel
between builds: the shell draws it once and leaves it, which is why the retail
seven can be written over themselves forever without anyone noticing. A window
that changes which label sits in which slot *is* noticed -- the old text stays
under the new one, and no amount of rebuilding clears it, because
`func_80019294` mallocs a transient object and `func_80019320` frees it again.

So `ErasePlateText` repaints the plate interiors with `func_8001ADF8` first,
the same primitive every page repaints its content field with. The box is
X=39..207, Y=181..201 stepping 34, in colour 0x182018, all measured off a stock
capture: the gold border sits at X=28..31 and 212..217 with a bevel either
side, and the corners are rounded, so the fill is inset to clear them. It runs
only when the window has actually moved, which is what keeps the retail rows
bit-exact.

## Row order: Video sits under Audio

Decided with the user. The list is Game Status, Memory Card, Difficulty,
Controllers, Audio, **Video**, Back Story, Credits -- the two audio/video pages
together, the informational rows last.

Splicing it in at five needs the page dispatch remapped as well as the strings.
`func_8010EA88` dispatches with `v0 = [s3 + s1*4]`, so `WidenDispatch` biases
S3 by one entry for indices above five: six and seven then read the retail
entries five and six, Back Story and Credits. Nothing is copied and nothing is
written to guest memory, so the entries stay whatever the loader relocated them
to. It also has to admit the eighth index -- `sltiu v0, s1, 7` at 0x8010EC58
would otherwise drop Credits entirely.

The scroll rule is unchanged, but it now bites on Credits rather than Video:
`top` is one only at index seven. Sitting on Video the list does not scroll at
all, so the plate erase never runs there.

Note the retail row strip is **no longer byte-identical to stock** -- it cannot
be, because two of the seven visible labels genuinely changed. The differing
pixel count is 2411 and it is entirely the two relabelled slots.

## The deaf-text-page defect, and its fix

Leaving the Video page used to leave the retail text page the cursor landed on
unable to take any input: frames kept presenting, but DOWN, UP and TRIANGLE
were all ignored and the only way out was to quit. It reproduced in the build
before the reorder too -- UP from Video landed on Credits and the next press
did nothing -- and had been missed only because no fixture pressed anything
after the return.

**Fixed** by `V82NativeTextPageInput.SupplyPad`, an inline hook at 0x8010E9D4,
the instruction after `jal func_800117C0` in `func_8010E854`. That body is
shared by Back Story and Credits and leaves only when the returned word meets
`0x50900000`. The hook supplies that word from the runtime's own pad in the
shell's bit order, and only on frames where the retail read came back empty, so
a healthy pipeline is untouched and the hook does not participate at all. It
writes 0x8006B4EC as well, because the Options loop that regains control reads
the row-cursor bits out of memory and those are just as empty.

### What the fault actually is, and what it is not

Narrowed hard before fixing, so the next person does not redo it:

- **Not the pad sample ring.** `func_80015540` consumes an eight-slot ring and
  advances its write cursor only while it differs from the read cursor, which
  looked like a perfect fit. Probed directly, the cursors were healthy the
  whole time. Worse, the cursor addresses are `gp`-relative and **the shell
  overlay's `gp` is not main's**, so cursor readings taken from inside a shell
  function are meaningless -- that cost a round.
- **Not IRQ delivery.** `BiosB.IntrEnvInInterruptAddr` stayed at 0x800643BA
  with handler 0x80055574 and the in-interrupt flag at 0, so the guest VBlank
  handler kept running every frame.
- **Not idling.** Sitting on Game Status for 400 polls and then pressing DOWN
  works. The pipeline does not rot on its own.
- **Not the whole screen.** Going UP from Video to Audio works, and Audio keeps
  responding afterwards. Every settings page is fine; only the shared text-page
  body is affected.
- **Not the processed-word consumption.** Disabling the Video page's
  `ConsumeNativePadWord` entirely changes nothing.
- Re-running `func_80021C24`, and forcing the outer loop's full re-init path by
  setting S0, both failed to repair it.

So what remains is that `func_80015540` returns zero to that one call site while
`Controller.State` is correct and every observable link in the chain is intact.
The underlying cause is still unknown; the hook compensates for it precisely
where it bites, and is written so that it disappears the moment the underlying
behaviour is fixed.

## The rows really do change the settings

Worth stating because it was asked and, until it was, only the *rendering* and
the navigation had been verified -- no fixture had ever pressed LEFT or RIGHT
on a row. `input-scripts/native_options_video_changes.txt` now does, and with
`RECOMPONE_TRACE_V82_VIDEO_OPTIONS=1` every change is logged:

```
Resolution: '1920x1080' -> '2560x1440' -> '1920x1080'   (both directions)
Fullscreen: 'Off' -> 'On' -> 'Off'    Widescreen: 'On' -> 'Off'
Render scale: '3x' -> '4x'            Anti-aliasing: 'FXAA' -> 'Off'
MSAA: '2x' -> '4x'                    Anisotropic: '4x' -> '8x'
Texture smoothing: 'On' -> 'Off'      Mipmaps: 'On' -> 'Off'
```

and every one of them lands in interface.ini, including `GraphicsPreset`
flipping to `Custom` once an individual setting is touched.

### Two real bugs that only this test could find

- **Pressing RIGHT hung the game.** `ConfigManager.SaveView` calls
  `ImGui.SaveIniSettingsToMemory()`, and the in-game menus save from contexts
  with no ImGui context at all -- headless, or any launch where the host
  window never came up. That takes the process down inside native code with no
  managed exception, so it looked like a freeze rather than a crash. Guarded
  on `ImGui.GetCurrentContext()`.
- **The first version of that guard silently wiped the window layout.**
  Writing an empty ImGui section meant anyone changing a setting from the
  in-game menus lost their whole docking layout. `ReadStoredImGuiLayout`
  carries the layout already on disk across instead.

Live application of resolution and fullscreen is still unverified: `HostWindow`
is null headless, so those two calls return immediately. The config side of
both is confirmed.

## Correction: the earlier "missing plate" readings

An earlier version of this document, and commit 2958f56, claimed the row plate
and its text come from one call and that the eighth row's plate was merely
clipped. That is wrong -- plate and text are drawn independently. A later
version then said the missing plate was `Game Status`'s. Also wrong, in the
sense that mattered: there was never a missing plate, only a list one row too
long for the seven that exist.

What misled the first reading: the retail list starts at Y=172 with a stride of
34, and the first attempt lifted the start to 138. `172 - 34 == 138`, exactly
one stride, so every text row landed on the *previous* row's plate. Rows two
through eight looked perfect and only `Game Status` had no plate above it,
which reads exactly like a clipped top edge. When debugging anything in this
area, keep the start Y off a multiple of the stride or the coincidence hides
the problem again.

## Where the plates come from -- settled

Nowhere in code. Ruled out, in order:

- The two `func_8002DE84` calls, the strongest earlier candidates, are
  **lights**: their pointer arguments hold 4.12 unit vectors (`0x801006DC` is
  `(0, 0, -4096)`, `0x801009B8` is `(0, -4096, 0)`) and the third argument is
  the colour.
- No table of Y values stepping 34 from 172 exists anywhere in SHELL.DLL, at
  any stride -- scanned exhaustively as halfwords.
- No seven-iteration draw loop exists besides the row loop. The only other
  `< 7` comparisons in the overlay are the jump-table bound, the pages' own
  loops, and a glyph-class helper.
- A GPU primitive trace over the row strip shows the panel arriving as a mesh
  of small skewed textured quads, and moving the loop's layout leaves the
  plates where they were. They are geometry on the transformed panel.

Incidental: the jump table at `0x80101180` is immediately followed by the
string `Shell\Cursor.PSX`, so there was no room to extend it in place -- the
hook was the only option.

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

## Next step

**Trim the curated option set** (below) -- the only thing still waiting on the
user. Everything else on this screen is done: the row draws, selects, sits
under Audio, has a plate, scrolls when it needs to, and every neighbouring
retail page navigates correctly.

Worth doing at some point, but not blocking: find why `func_80015540` returns
zero inside `func_8010E854` after a hook-owned page has held the frame, and
delete `V82NativeTextPageInput` once it does not. And the retail Audio page has
a mute toggle and two volume sliders already drawn on it, so master volume and
mute very likely fit there without a ninth row -- worth confirming before
building anything.

## Curated option set

Fourteen rows: Resolution, Fullscreen, Widescreen, Anti-aliasing, MSAA,
Anisotropic, Texture smoothing, Mipmaps, Level of detail, Draw distance, Fog,
Shadows, Particles, HUD anchoring. Audio page (not built): master volume,
mute.

Every row maps onto an existing `ViewConfig` property and saves through
`ConfigManager.SaveView`, so a change made here and one made in the ImGui
Display section take the identical path. Resolution and fullscreen apply live
through `HostWindow.SetOutputResolution` / `SetFullscreen`, reached from the
recompiled project through public wrappers on `V82Compat` because `HostWindow`
is internal.

**No graphics preset row.** Dropped at the user's request: the Original preset
exists to put the PS1 software renderer back and this port is not going back to
it, so in a player-facing menu it could only ever make things worse. The host
ImGui panel still has it, and `GraphicsPreset` still resolves to `Custom` once
any individual setting is touched.

**No render-scale row either, and the scale is pinned at 4x.** What was
"Internal 3D" is `InternalResolutionScale`: the 3D scene rasterises at that
multiple of the PS1's own 320x240 into the enhanced GL targets and is then
presented into the window. It has nothing to do with the Resolution row, which
is the window itself, and the old label read like a second output setting
sitting right underneath one.

It was briefly renamed "Render scale" with 1x dropped, then removed entirely.
The reasoning, and the measurement that settled it:

- The scale is hard-clamped to 4 in three places (`ViewConfig`,
  `EnhancedGlBackend.ApplyResolutionScale`, `GlVram.ReinitializeScale`), so the
  3D area tops out at **1280x960** -- below every output resolution the menu
  offers except 720p. 4x is therefore the right answer essentially always.
- The obvious objection is that it is the performance knob and removing it
  strands weak hardware. It is not much of one. Fill rate at 1280x960 is
  nothing, and **VRAM readback does not scale with it at all**: `ReadRect`
  blits from the scaled texture down to `_stageFbo`, which is created at the
  unscaled 1024x512, and reads from *that*. The CPU transfer is the same size
  at 1x and 4x. What does scale is texture memory (32 MB for the VRAM texture
  at 4x, plus render targets, multiplied again by MSAA samples).
- MSAA stays in the menu and is the knob left for a slow GPU.

`ConfigManager.Load` pins it: for V8:2 it sets `InternalResolutionScale = 4`
and `HighResolution3D = true` after reading interface.ini, so existing configs
sitting at the old default of 3 are moved up. The host ImGui panel can still
change it for the rest of a session.

**The ceiling is still the real limitation.** Pinning at 4x does not remove
it: at 1080p and above the picture is upscaled from 1280x960 no matter what.
Raising the cap above 4 is the actual improvement, and is not done here --
it is renderer work with a memory cost, and the GL path cannot be exercised
headlessly to check it.

### Render scale and Resolution are independent, in both directions

Nothing clamps one against the other, and the interesting case is the one that
looks like a mistake. At 4x the 3D area rasterises at 1280x960 (wider with
Widescreen on, which grows `w1x`), and the smallest output the Resolution row
offers is 1280x720 -- so that pairing renders *above* the output and the final
sample is a **downscale**. That is real supersampling and the best-looking
combination on the page, not an error. `_presentTex` is Linear-filtered
whenever `NativeResolution` is false, which it now always is, so the reduction
is filtered rather than point-sampled. There are no mipmaps, so a reduction
much beyond 2x would start to alias, but nothing the menu can select gets
there.

Every other pairing upscales: 1080p and above are all larger than 1280x960.

**Gotcha worth fixing at some point:** `SetOutputResolution` returns early when
`View.Fullscreen` is set, and `SetFullscreen(true)` just takes the desktop
resolution. So while fullscreen is on, the Resolution row changes the stored
value and nothing else -- it is inert, with no indication in the menu. Greying
it, or applying it as a fullscreen mode change, would both be improvements.

Not verified at runtime: headless runs report `hle=False`, so the enhanced GL
backend never initialises and none of the present path above executes. All of
this is read from the code, not observed.

Out (diagnostics): dithering, geometry correction, precise culling,
perspective-correct textures/colours, enhanced depth buffer. Borderline and
unconfirmed: true-colour output, vector fonts, vector icons. High-resolution 3D
is not a row: it is driven implicitly by Render scale, and with 1x gone it is
now always on.

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
