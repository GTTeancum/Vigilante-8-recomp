# Continuous Quest harness — September 11

Requested: every new Quest in one game process, without closing the game.

Run: `artifacts/continuous-quests-20260911`, PID 25072, staged build 9AC53BE8.
No game restart or termination was performed. The harness has no automatic game
shutdown on success, assertions or timeout. Wins are explicit state-machine
fixtures after 600 gameplay frames; they are not combat completion evidence.

Results:

- Chassey: four source missions, all map/spawn/player checks pass; ending plays,
  progress saves as 31, native menu returns.
- Clyde: four source missions, all map/spawn/player checks pass; ending plays,
  progress saves as 31, native menu returns.
- Entering Sheila's campaign: the native selector requests 286,848 bytes of
  emulated audio memory and fails with `Out of SPU RAM` (caller 0x8001DF04).
  The game remains in its native fatal-error loop. Remaining 40 missions are
  not validated by this run.

The final log audit also found recovered `Out of VRAM` errors while loading
Chassey mission 4 and Clyde missions 3–4 (12 failures total). The game continues
after those errors by declining the texture allocation, so successful map/spawn
and transition checks are not clean resource/fidelity passes. Both the texture
allocation failures and the later audio exhaustion must be resolved.

Source evidence points to missing imported gameplay bank cleanup:

- `V82VehicleRegistry.AttachSoundBank` allocates each imported SND through the
  native SPU allocator and stores its sample handle at runtime +0xC.
- `ResetRuntimeForMatch` clears `BodyRuntime` / `TransformRuntime` and other
  gameplay pointers without releasing their audio allocations.
- The registry's explicit release helper owns selector BIN/ANM/runtime CPU
  allocations only; it has no corresponding gameplay SPU release path.
- Run logs show newly attached gameplay SND addresses advancing up to 0x787D0
  before the subsequent selector allocation fails. The selector correctly
  refuses the old blanket allocator reset, which would invalidate live banks.

Repair should give gameplay banks explicit ownership and release their CPU
and SPU allocations at a safe native match-teardown boundary. Increasing the
SPU budget or resetting the live allocator would hide the lifecycle problem.
That diagnosis is source/log evidence; a repaired full-run test is still needed.

## Repair and verification in progress

Imported gameplay banks now use the retail bank destructor after all match
objects have been retired. This releases textures, sound handles/SPU memory,
BIN/ANM data and runtime allocations. Stats and upgrade tables are also freed.

The texture repair shares byte-identical immutable imported XOBF image/palette
payloads. Keys include dimensions, page constraints, bit depth, compression
mode and the complete encoded payload hash. Every runtime load must match its
source reservation. Each shared rectangle remains owned by the match until
all native bank destructors finish, then its native backing rectangle and
synthetic descriptor are freed once. Native arenas, menu textures and the
native split/free logic retain their existing paths. No texture sizes, palette data,
VRAM budget or SPU budget changed.

The read-only source audit covers all 48 mission rosters. Clyde mission four
has 536 requested rectangles but only 318 unique payloads: 108,849 texture
words reduce to 77,600. Across the full matrix, sharing saves 26% of requested
texture words. See artifacts/quest-texture-sharing-audit.json.

Reservation ordering alone reduced failures but did not eliminate them. A
best-fit allocator experiment also failed full-campaign validation and was
removed; it is not in the final repair.

The opt-in forced-win fixture now defeats opponents through native lethal
damage (including deferred arrivals). Leaving opponents alive behind a fake
win allowed control attacks to interfere with result acceptance. This is a
state-machine fixture, not proof of natural combat/objective completion.

The shared-content build passed the first fourteen missions without resource errors.
User then requested console-style cleanup of other level-owned data: scene packet
ownership, host object tracking, texture lookup cache capacity and terrain anchors
now clear at the same native end-of-level boundary. Shared display configuration
and immutable catalogs remain available.

The level-cleanup diagnostic reached 26 missions, but later log review found
additional recovered texture failures as well as repeated stale-backing warnings.
Retail full-tree destruction (0x80020754,
recursing through 0x80020700) retires all native allocation nodes, including shared
ones. Host coordinate/backing ownership now expires at that exact boundary;
CPU descriptors remain owned until their bank/match cleanup. Direct descriptor
frees also respect shared ownership while the backing tree is alive. Contract
tests cover both paths. The previous run was deliberately stopped to stage this
repair, not counted as a complete pass.

The tree-lifetime diagnostic reached 20 missions with no stale-backing warnings,
but Torque mission 3 (Air Graveyard) still failed a 64x256 texture allocation.
The isolated trace reproduces it. The 1,133 recorded requests are replayed through
the generated native allocator by `tests/VramPackingContract`: original placement
fails once. Short-side-fit placement fixed Torque but failed the subsequent
continuous test at Clyde mission 4. Both traces are now regression fixtures;
closest-width-column placement (then least spare height) fits every request in
both, matching the allocator's vertical-first image splits. Tests also verify
framebuffer/VRAM bounds, native page-origin rules,
no overlaps, native coordinate frees, and zero live nodes after full teardown.
Only imported-match leaf selection changes; original split/free/upload behavior
and both memory budgets remain intact. Simple request reordering was insufficient.

The harness now rejects stale-backing warnings and PC-heap exhaustion as well as
SPU/VRAM allocation failures and blanket audio allocator resets. A complete
48-mission run on each candidate is required. Coverage starts from mission one;
results are not combined across builds.

The column-packing build (863d7a7d) subsequently completed all 48 missions and
12 correct ending movies in PID 33252. All 48 bank, scene and lookup releases
occurred. It is a failed candidate: Houston mission 4 logged one recovered VRAM
failure, and Molo mission 1 displayed the preceding arena's loading card even
though the terrain was correctly Oilfield. The original monitor stopped on that
card assertion; the game continued unchanged through all remaining campaigns.
`artifacts/continuous-quests-column-packing-20260911/completed-audit.json`
preserves the full post-run audit; no partial runs were combined.

All 48 native allocation sequences are now a durable regression matrix. Sorting
guest reservations by descending width, putting CLUTs into the shortest strips,
and filling image columns left-to-right fits every recorded request. The native
split, page constraints, uploads and frees remain unchanged. Contract replay
also verifies nonoverlap and complete native allocator-node teardown. Earlier
leaf-only heuristics and split-orientation experiments are not production code.

The presentation layer now takes the incoming native level-load path before
overlay execution can update its last-level identity. That identity clears at
match teardown. A consecutive-path contract catches stale-card reuse.

## Completed validation, September 11

The staged width-order build passed the full continuous harness in PID 33444,
without closing or restarting the game: 48 missions, 12 correct ending movies
with decoded video/XA frames, all 12 campaign save records complete, and 12
returns to the main menu. Duration: 1,722.19 seconds (28 minutes 42 seconds).
Every mission released its imported banks, scene tracking and texture lookup
cache. There were zero SPU/VRAM exhaustion errors, stale-backing warnings,
CPU-heap exhaustion errors, blanket audio allocator resets, or loading-card
mismatches. The process remained running after completion.

Staged executable: `V8_2_LOOSE/Vigilante82PC.exe`, SHA-256
`64c194bd4c8cc97f6a8e469b15734749985e1292ed380714c6aea597f476c94b`.
Evidence: `artifacts/continuous-quests-width-order-20260911/proof.json` and
`checkpoint-audit.json`. Runtime sharing totals matched the source audit on
every load. The 48-case generated-native allocator contract, shared-content
ownership/lifetime contract, incoming loading-path contract and patcher
idempotency checks passed. The six `review-*.jpg` sheets were inspected: one
native gameplay capture at frame 450 per mission, not every rendered frame.
These checks address the repaired resource/transition failures; broader visual
release review remains open separately.

The user accepted the observed shared objective-trigger event as sufficient
coverage. Progression in this run used the explicit win fixture after 600
gameplay frames; it did not naturally complete every objective independently.

The broader resource-lifetime audit and five-hour soak remain TO-DO
10/V82-OPEN-026. Post-teardown PC allocation counts rose from 211 after the first
mission to 1,760 after the last, with fluctuations between maps and campaigns.
Counts alone do not distinguish retained shared data from leaks, nor measure
bytes retained. Preserve this finding for allocation-owner/size tracking and
the soak; this pass does not claim all long-term leaks are resolved.
