# Durable mid-function hooks (item 4, phase 0)

## Why

`reference-v8-2/generated/` is gitignored and rebuilt from the disc. Anything
hand-edited into it is one regeneration away from gone.

The PC controls page -- the whole precedent for native in-game menus -- lived
entirely as two hand edits inside `func_8010A5BC`, the retail Options routine
that draws the page and services its input in one body. There is no function
boundary there, and `prepare_reference.py` could only express `pre`, `post` and
`replace` at *function* addresses. So the feature had no durable
representation at all, and neither would any menu page added next to it.

## What changed

A new `inline` patch mode places a call beside a named **instruction**:

```python
{
    "overlay": "SHELL_SHELL",
    "address": "8010AE8C",       # instruction, not function start
    "target": "V82NativeControlOptions.TryDraw",
    "mode": "inline",
    "position": "before",        # or "after"
    "branchTo": "8010B1EC",      # optional; target returns bool
}
```

With `branchTo` the emitted line becomes `if (target(c, m)) goto LXXXXXXXX;`,
which is how a hook takes over the rest of a routine rather than merely
observing it. Implemented across `PatchEntry` (config), `InlineHook` +
`MipsFunction.InlineHooks` (model), `OverlayWriter.ApplyPatches` (matches on
address containment rather than function start) and `FunctionEmmiter`
(emits either side of the instruction, after any label).

## Audit

Comparing every mid-function SDK call in the generated tree against the
registered patch list found **nine** hooks with no durable representation, not
the two that were known:

| hook | now registered as |
|------|-------------------|
| `V82NativeControlOptions.TryDraw` | inline before `8010AE8C`, branch `8010B1EC` |
| `V82NativeControlOptions.UpdateState` | inline before `8010B210` |
| `BeginLatePass` / `EndLatePass` | pre / post on `8001C910` |
| `BeginSkyPass` / `EndSkyPass` | pre / post on `8003150C` |
| `BeginSceneryPass` / `EndSceneryPass` | pre / post on `80050B38` |
| `RecordPoolLink` | pre on `8001D414` |
| `RecordObjectVisibility` | inline after `8002E2E0` |

Most sat at a function boundary and were only ever hand edits because nobody
registered them; each host function was checked for a single `return` before
converting an exit hook to `post`, so the semantics match.

`BeginTextureDecode` is deliberately left out: it captures a local
(`v82TextureDecodeCallerSp`) across the call and is not expressible as a hook
call. It remains a hand edit and is the one thing a regeneration would still
lose.

Addresses were verified against the generated output: `8010AE8C` is the
instruction immediately after label `L8010AE8C`, and `8010B210` is the return
address of the `func_80015540` call the hook follows.

## Regenerating without the disc image

The retail CUE/BIN is gone. The recompiler only ever reads files out of the
disc, and `CueFs.OpenLoose` already existed, so the extracted disc root at
`V8_2_WORK/disc` is a complete stand-in. Two things blocked it, both runtime
concerns the recompiler has no use for: standalone STR/XA files must be in
2336-byte Mode 2 sector form, and loose CDDA tracks must be present.
`CueFs.OpenLooseCodeOnly` relaxes exactly those two and nothing else.

```
python tools/recompone-v8-2/prepare_reference.py --loose-disc V8_2_WORK/disc --output <dir>
dotnet .../recompone.dll <dir>/v82.recompone.json
```

Verified: 131 of 132 patches applied, and all eight newly registered hooks
re-emit from their declarations -- the three inline seams as
`if (V82NativeControlOptions.TryDraw(c, m)) goto L8010B1EC;`,
`V82NativeControlOptions.UpdateState(c, m)` and the visibility record, the
four entry hooks as `PreHook.Run(...)`, and the three exit hooks as post
wrappers.

One pre-existing warning is unrelated to this work: `RestoreTerrainFrustum`
declares address `8001C89C`, which matches no function start.

## Remaining exposure

Comparing hooks in the working tree against a regenerated tree, after
registering the widescreen terrain-traversal chain and the selector slot, four
are still hand edits with no declaration:

| hook | why it is not declared |
|------|------------------------|
| `BeginTextureDecode` | captures a local (`v82TextureDecodeCallerSp`) across the call |
| `RestoreTerrainFrustum` | its declared address `8001C89C` matches no function start, and the real seam is mid-routine with no nearby label or call to anchor on |
| `RepairObjectTerrainQuery` | same, no derivable anchor |
| `OverrideNativeSelectorText` | same |

The inline mechanism anchors on an instruction address, and an address can only
be read back out of generated C# where a label or a return address appears.
These four sit where neither does, so pinning them needs the disassembly rather
than the generated text. That is a separate task; the widescreen terrain work
that previously vanished on a regeneration is now safe.

Worth noting the terrain chain could not be `pre` hooks: six calls stack at the
entry of `func_8001BECC`, and `PreHookTarget` is a single field, so each would
have clobbered the last. Inline hooks are a list and emit in declaration order.

## Phase 1 spike: adding a row to the retail Options list (in progress)

The goal is a real VIDEO entry in Options rather than hiding video settings
under CONTROLLER.

What is known so far. The Options page is `func_8010A5BC` in SHELL_SHELL; it
draws the page and services its input in one body, and both existing PC-page
seams live inside it. It is data driven: the draw path reads a descriptor
through `S7`, taking the item at `[S7 + 0x10]` into `S0` and skipping the row
when that is null (`SHELL_SHELL.cs:11231`). Row strings are drawn as text --
GAME STATUS, MEMORY CARD, DIFFICULTY, CONTROLLER, AUDIO, BACK STORY, CREDITS --
and are what `V82Compat.TraceNativeOptionsText` keys its stage names on.

Not yet found: the table those seven entries come from. The strings do not
appear as literals in the overlay's generated C#, so they are data rather than
inline constants, and the descriptor `S7` points at needs tracing back to its
producer before a row can be appended.

Dumped it (`RECOMPONE_V82_TRACE_OPTIONS_TABLE=1`, one shot from
`V82NativeControlOptions.UpdateState`, driven by
`input-scripts/native_options_discovery.txt`):

```
[OptTable] S7=0x80775FF8
[OptTable] [S7-12] 0x80775FEC = 0x80776158
[OptTable] [S7+16] 0x80776008 = 0x80775F48   <== [S7+0x10] row item
[OptTable] [S7+64] 0x80776038 = 0x80775FD0
```

**The rows are not a static table.** `S7` is at `0x807xxxxx`, in the heap
rather than overlay data, and the words around it are pointers into
neighbouring heap objects. The Options list is a UI object tree built at
runtime, in the same shape as the rest of the shell's objects.

That is the better outcome for item 4: a fixed-size table in overlay data would
have been awkward to extend, but a runtime-built list can simply be given one
more row by calling the same construction path once more. The fallback of
turning the CONTROLLER page into a tabbed PC SETUP is probably not needed.

Next step is to find the producer -- the code that builds these row objects --
by breaking on writes to `[S7 + 0x10]` or by walking back from the shell UI
root, then adding a VIDEO row alongside the existing seven. The inline patch
mode from this document is what such a call would be registered with.
