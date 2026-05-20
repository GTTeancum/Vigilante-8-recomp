# src/gameplay

Match / round logic, weapon firing, AI, vehicle slot management,
shell-to-match transitions, scoring.

## Status

DECOMP_PASS_2: **complete.** All in-scope gameplay functions
hand-cleaned.

## Key recoveries

- **`rng.c`** -- V8_SeedRng + V8_RandNext. The consumer-side xorshift
  was the most critical 1:1 contract; both halves now match the
  original bit pattern. HIGH.
- **`collision_apply.c`** -- Damage_AccumulateOrFire, the universal
  damage arbiter used by every destructible. HIGH.
- **`ai_target.c`** -- Vehicle_TryAcquireTarget, the per-frame target
  picker. HIGH.
- **`main_loop.c`** -- V8_MainLoop, the outer shell->load->play->result
  iteration. MED (structural+control-flow 1:1; GPU env table init
  elided per renderer scope).
- **`pause_menu.c`** -- Menu_Pause with CD-track / split-screen /
  resume + confirmation dialog. MED.
- **`result_screen.c`** -- ResultScreen_Build for YOU WIN/LOSE/TEAM
  with cheat unlock emission. MED.
- **`match_score.c`** -- MatchScore_AppendLine, confirmed Vehicle.+0xba
  (weight) and +0xbb (skill). MED-HIGH.
- **`insert_original_cd.c`** -- the "wrong disc" loop with cover-open
  polling. HIGH-MED.
- **`damage_apply.c`**, **`object_lifecycle.c`**, **`object_tree.c`**,
  **`object_hierarchy.c`** -- the object engine.
- **`level_load.c`**, **`level_teardown.c`** -- per-match setup/teardown.
- **`cheat_code.c`** + **`cheat_decode.c`** -- 14-byte scrambled cheat
  code matcher.

Plus 40+ smaller leaf functions: bone/buffer/handle/match-config/
object-broadcast/object-findbyid/object-list/object-post-update/
object-state/object-teardown/objlist-count/etc.

## Charter completion

Every in-scope gameplay function in `analysis/` has a corresponding
hand-cleaned C file under src/gameplay/. The Vehicle struct fields
used by gameplay (+0x0, +0xc, +0xe, +0x38, +0x3e, +0xba, +0xbb,
+0xe4) are all confidence-tagged in include/structs.h.
