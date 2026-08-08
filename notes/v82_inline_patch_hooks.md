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

## Not yet verified

End to end, by regenerating. `prepare_reference.py` requires the retail CUE and
`V8_2_BINCUE/` is absent from this tree -- only the extracted disc root
(`V8_2_WORK/disc`) and the loose files are present. The recompiler builds and
the patch list loads, but nothing has re-emitted `SHELL_SHELL.cs` from these
declarations. That check should run before item 4 builds anything further on
this mechanism.
