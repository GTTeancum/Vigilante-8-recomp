# Vigilante 8 PS1 Decompilation — Operating Charter

You are working on a long-running autonomous decompilation of the Vigilante 8 PlayStation 1 game. Read this file at the start of every session, then read `progress.log` to determine where to resume.

## Mission

Produce a clean, readable, **bit-exact** C decompilation of Vigilante 8's:
- Physics (vehicle dynamics, collision, damage)
- Asset loading (all proprietary formats, textures, models, levels, sounds-as-data)
- Gameplay logic (state machines, AI, weapons, match flow)

**Out of scope** — do not decompile:
- Renderer (will be rewritten)
- Controls / input handling (will be rewritten)
- Audio engine (will be rewritten — but asset loaders for audio data ARE in scope)

## Operating Rules — Read Carefully

### Autonomy
- **Never ask for confirmation.** Decide and proceed.
- **Never ask clarifying questions.** If ambiguous, pick the most reasonable interpretation, log it in `decisions.log`, continue.
- **Never ask permission** to install tools, write files, run shell commands, or modify anything inside the project directory.
- **Never stop to ask "should I continue?"** — yes, always, until the charter is complete or you hit a real blocker.

### Real blockers (the only valid stop conditions)
1. A required input file is missing (EXE, overlay, or disc data file referenced by the binary).
2. An unrecoverable tool error after one retry (Ghidra crash, etc.).
3. The charter is complete (all in-scope subsystems decompiled and refined to satisfaction).

For anything else: log the issue, continue working.

### Progress discipline
- Append to `progress.log` after every major step. Format: `[ISO timestamp] [phase] [action] [result]`.
- Append to `decisions.log` whenever you make a judgment call. Format: `[timestamp] [context] [chose X over Y because Z]`.
- Write intermediate results to disk continuously. Never hold large analysis state only in memory.
- After every build intended for testing or handoff, ensure the latest runnable binary and required dependencies are copied to `PS1 game\`. The user runs from that directory; never leave them on a stale exe.
- When deploying a build for user testing, clean up old run logs in `PS1 game\` so stale `v8_latest.log` / archived run logs do not confuse crash or freeze triage.

### When resuming a session
1. Read `AGENTS.md` (this file).
2. Read `PROJECT_SCOPE.md` for the 1:1 requirements.
3. Read `DECOMP_RULES.md` for naming and confidence conventions.
4. Read the tail of `progress.log` to find where work left off.
5. Resume from the next unfinished phase. Do not restart completed work.

### Style
- No mid-task summaries to the user.
- No "should I proceed?" prompts.
- No asking about coding style — follow `DECOMP_RULES.md`.
- Match the apparent original code style of the binary; idiomatic K&R C otherwise.
- One concise status line per major milestone is fine. Do not narrate every step.

## Project Layout

```
v8_decomp/
├── AGENTS.md              # This file
├── PROJECT_SCOPE.md       # 1:1 match requirements
├── DECOMP_RULES.md        # Naming, confidence, struct conventions
├── SETUP.md               # First-run bootstrap
├── progress.log           # Append-only work log
├── decisions.log          # Append-only judgment-call log
├── input/                 # User-provided files (EXE, disc data) — read-only
├── ghidra/                # Ghidra install + project files
├── analysis/              # Raw Ghidra exports: function lists, MIPS, pseudo-C, xrefs
├── src/
│   ├── physics/           # Decompiled physics (in-scope)
│   ├── assets/            # Decompiled asset loaders (in-scope)
│   ├── gameplay/          # Decompiled gameplay logic (in-scope)
│   ├── data/              # Extracted data tables (vehicle stats, weapons, etc.)
│   └── skipped/           # Stub headers for renderer/controls/audio (out-of-scope, for reference only)
├── include/
│   ├── structs.h          # Recovered structs
│   ├── globals.h          # Recovered global variables
│   ├── fixed.h            # Fixed-point math types and ops
│   └── gte.h              # GTE operation declarations
├── tools/                 # Custom scripts: asset extractors, struct dumpers, verification
└── notes/
    ├── unknowns.md        # Functions/structs that couldn't be RE'd
    ├── ambiguous.md       # Functions tagged AMBIGUOUS (scope unclear)
    └── rename_log.md      # Original address → final name mapping
```

## Phase Order (default execution)

1. **SETUP** — install Ghidra, PSX loader plugin, verify headless mode. See `SETUP.md`.
2. **IMPORT** — headless import of EXE and overlay/data files into a Ghidra project.
3. **ANALYZE** — run full Ghidra auto-analysis. Export functions, strings, xrefs, pseudo-C in bulk to `analysis/`.
4. **CLASSIFY** — tag every function by subsystem (physics / assets / gameplay / renderer / controls / audio / runtime / unknown).
5. **TRIAGE** — for in-scope subsystems, build dependency-ordered work queue (leaf functions first, then callers).
6. **DECOMP_PASS_1** — first decomp pass on every in-scope function. Best-effort naming and struct recovery. Save to `src/`.
7. **STRUCT_PASS** — cluster field accesses across all functions, finalize struct layouts in `include/structs.h`, propagate types.
8. **DECOMP_PASS_2** — refinement pass. Improve names using context from pass 1. Update `rename_log.md` retroactively.
9. **DECOMP_PASS_3** — final pass. Tighten anything still LOW confidence where possible.
10. **VERIFY** — write verification scaffolding (see PROJECT_SCOPE.md "Verification").
11. **DOCUMENT** — write per-subsystem README in each `src/*/` folder explaining architecture and entry points.

Each phase completes fully before the next begins, unless a phase produces zero output (skip and log).

## Charter Complete When

- Every in-scope function in `analysis/` has corresponding cleaned C in `src/`.
- Every recovered struct is in `include/structs.h` with field names and confidence tags.
- All passes 1–3 are complete.
- `notes/unknowns.md` lists every function that couldn't be fully RE'd, with what is known about it.
- Per-subsystem READMEs exist.
- Verification scaffolding is in place (does not need to pass — just exist).

When complete: write a final entry in `progress.log` reading `[timestamp] [CHARTER_COMPLETE] [summary stats: function count, struct count, unknowns count]` and stop.
