# Pass 1 -> Pass 2 Backlog -- CLOSED

All 7 large in-scope gameplay functions originally deferred from
pass 1 have been hand-cleaned. This file is preserved as the
historical record of the deferred set.

| Address    | Hand-cleaned file                                | Confidence |
|------------|--------------------------------------------------|------------|
| 0x80013cac | src/gameplay/main_loop.c   (V8_MainLoop)         | MED        |
| 0x800120d4 | src/skipped/pad_input.c    (Pad_Tick; controls)  | seam doc   |
| 0x80012a90 | src/gameplay/pause_menu.c  (Menu_Pause)          | MED        |
| 0x8001356c | src/gameplay/insert_original_cd.c                | HIGH-MED   |
| 0x800136c4 | src/gameplay/match_score.c (MatchScore_AppendLine) | MED-HIGH |
| 0x8001392c | src/gameplay/result_screen.c (ResultScreen_Build)| MED        |
| 0x8003e2fc | src/physics/camera_build.c (Camera_BuildMatrix)  | MED        |

## Closure summary

Pass 2 successfully promoted every entry above plus all per-level
destructible auto-stubs across the 10 level DLLs. The
`src/gameplay/auto/` directory has been removed entirely.

The Vehicle struct evidence (now in `notes/struct_evidence.md`) and
the cross-DLL function resolution (via tools/resolve_xrefs.py)
provided sufficient context for the cleanups -- the original
deferral rationale ("LOW-confidence renaming risk") no longer
applies once each field has cross-call evidence.

The original pass-2 workflow notes are still applicable for any
future hand-cleaning rounds:

  1. Read Ghidra pseudo-C from `analysis/<dll>/decomp/<addr>.c`.
  2. Rewrite using named helpers from src/{assets,gameplay,physics}/.
  3. Cite Vehicle/MatchState fields by name (no more `+0x38` indices).
  4. Place under src/<subsystem>/<descriptive_name>.c.
  5. Delete the auto-stub.
