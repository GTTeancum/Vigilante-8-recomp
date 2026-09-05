# Water attachment fit — active goal

Current concise coverage: `notes/v82_water_attachment_verification.md`.

User report: water attachments look detached from the vehicle. Previous
buoyancy and mode-transition results do not establish correct attachment fit.

User-confirmed original-game reference:
`C:/Users/smmel/AppData/Local/Temp/codex-clipboard-4d7a951b-3e38-409f-89ce-512d8f3fb7ce.png`.
The original Florida gameplay shows the support arms joining the body. Treat
continuous arm-to-body connection as the acceptance criterion. Do not dismiss
the port's visible gaps as original/authored behavior. The screenshot depicts
a different vehicle, so it establishes the required connection, not a numeric
Sheila-specific offset.

## Latest matched-water investigation

Diagnostic build `1CD2DD2014F957F80C9D98971560D72F9E67EF943836CB3521A72A15B1296578`
ran All and Auto sequentially in Florida, each exiting 0 with settings restored.
Two full-size captures were individually inspected. All frame270 shows the gap
with mode2/transition0; Auto frame270 is **rejected as a matched water proof**:
the vehicle reached the shore and was retracting (mode0/transition8). Both used
the same initial water position, but subsequent physics diverged. Neither is a
fit pass. All capture FPS60.00, Auto59.87; hashes and frame windows are in their
individual `proof.json` files under `artifacts/water-attachment-fit-20260905/`.

Test fixture tightened: attachment profile now checks a 3x3 deep-water footprint,
places only the player, and captures frame150 only if mode2/transition0. Runtime
physics is not held or healed. Proof metadata now records actual attachment
state at capture, rather than trusting a water-themed filename. Clear-water
diagnostic build has compiled successfully; no shipping attachment correction
has been made. Next validate this fixture without new images before the next
two-image visual comparison. All30 targets remain pending.

Clear-water fixture validation completed without images on deployed diagnostic
`ED4E76C42E55BF13D488684F379C42294B728E015E32AEFE3DE9A7D03DDDD35A`.
`sheila-clear-water-auto-validation` exited0, median/min60.00/59.33FPS. Player
remained mode2 throughout water samples120–330, alive with original callback,
then mode0/transition0 at all dry samples480–600. Frame150 is suitable for the
next visual comparison. This verifies the revised fixture, not attachment fit.
No game remains live; settings restored. Source now additionally traces each
attachment's child meshes, bank slots, rotations, flags and translations to
distinguish missing connector geometry from incorrect placement. That final
trace addition still needs a build. Next: publish, capture two clear-water
frame150 views (All/Auto), inspect their complete part trees against direct
COMMON.EXP assets and the user's original-game reference. Do not rerun the
discarded shore/frame270 setup. No more captures in this turn.

## Scope and roster

Verify every currently installed selectable vehicle except Y the Alien:
**30 targets**, stock types **0–17** and imported types **64–75**. Imported
Y is type 76, stable ID `guest.v8.y_the_alien`, excluded by user request.
`tools/recompone-v8-2/inventory_water_attachment_roster.py` reads the actual
retail stats and installed registry rather than assuming a small sample.
Registry SHA256: `3d21658876939e16434240ec80d139dbf8af4907620922866daa7c0fd62842cc`.

All 30 remain **pending**. No attachments have been moved or approved yet.

Diagnostic candidate `88916642DA2720F387BD7327569B597B9DEDE63D53A57E39F4D4E791BF18E47F`
adds opt-in traces only. `sheila-native-all` exits 0 but its frame-270 capture
is **inconclusive for fit**: native water mode settled at 210, then the original
water handler requested wheels after sustained dry contact (`80040C58..CB0`),
so the capture shows mode 0. Do not present this as water-attachment proof.
The next capture triggers on actual mode 2 plus completed assembly, before
native dry-ground retraction. All four supports are correctly parented to
the vehicle in the recorded settled samples; mesh-to-body fit is not proven.

`sheila-native-all-settled` on diagnostic candidate
`25D56DCEBB6560FD46B638F0CB063550198D09EFCC7B2626116B1B49DEE20C96`
exits 0. Inspected the individual full-size native frame196: mode2, transition0,
all four native parent links resolve to Sheila. The deployed supports are
angled on dry ground and their struts reach the body in this rear view; the
lowest ends are clipped by framing. This is an **All-policy/dry-ground
diagnostic**, not a water-fit pass and not directly camera-matched to the earlier
Auto/Florida capture. FPS preceding/capture61.14/60.11. User settings restored
to FB0C93BD...C9A43E36 and no game remains live.

Next diagnostic: compare native All and Auto on the same Florida water fixture
and settled assembly, logging mount transforms. Do not attribute the difference
to Auto or claim an authored gap from the dry-ground frame alone. Two captures
were made in this turn (one rejected/inconclusive); further captures belong in
the next turn to preserve the user's image limit. No shipping fit change yet.

## Investigation

- Original transition `8003E32C` wraps support nodes `vehicle+104..118` in
  temporary parents and preserves their mount translations.
- Native `8003E774` rebuilds models at animation midpoint through `8003E4A8`,
  then reparents the supports back to the vehicle on completion.
- `8003E4A8` chooses vehicle-authored overrides or the common transformation
  bank. Converted vehicles use the same native path with registry bank/table
  substitutions. Examine both the selected asset and the resulting transforms.
- Node `+14` is parent-or-previous-sibling, not a direct parent pointer.
  Diagnostic parent ownership must use native walker `8002CD78`.
- Added opt-in read-only hierarchy diagnostics and a native All-policy water
  attachment capture. This allows comparison with the Auto water screenshot
  without changing attachment positions or introducing authored exceptions.
- Direct asset pull (no VRAM capture): native mode-2 table is
  `(63,70,59,77,63,70)` in common bank 18. Sheila's immediate body mounts are
  front `(-14419,3821,20873)/(14416,3821,20873)` and rear
  `(-17694,3821,-27590)/(17693,3821,-27590)`; other 0x8000-series keys occur in
  subtrees and must not be mistaken for immediate support mounts. The four
  supports retain these X/Z translations after normal transformations.
  Correct parent links alone do not prove that rendered connector geometry
  reaches the body. Check mesh origins/scales and vehicle-authored overrides.

## Required verification

### Torque inspection viewpoint verified

- Native camera cycle1 produced first-person view in `fit-stock01-camera1`;
  rejected as attachment evidence despite exit0/native water checks.
- Added opt-in inspection camera in the process-local probe only. Native
  `8001277C` supplies camera array0x8006B8D8; `8004AFB8` consumes owner+80,
  follow distance+9C and pitch+90. Owner is verified before widening/elevating
  the camera. Native tracking/callback and vehicle physics remain unchanged.
- `fit-stock01-inspection`, build412E1363...482F2BB, exit0: full-size frame150
  individually inspected; all four supports in frame and visibly connected.
  Complete first/repeated water and dry samples. Capture60.07FPS; run
  median57.99/min43.71. User review still pending; not an all-roster pass.
- 319 focused assertions pass after adding mirrored 180-degree root coverage.
  Original settings restored and no game remains. Two captures this turn.

### Corrected guest proof and Torque framing (09556D24 build)

- `fit-chassey-selected`: normal registry selection now reserves100 native
  rectangles, claims38 at body load and retains62 for later transformation
  loads. NO native allocation failures. Individually inspected full-size
  frame150: clean body/attachment textures and visible connected support arms,
  with water tint partly obscuring the lower assembly. Both water/dry cycles
  have complete valid sample sets. Run was interrupted by concurrent-game
  guard after those samples; clean exit is UNVERIFIED. Proof explicitly records
  interruption, not exit0. Capture window58.46FPS, preceding59.70.
- `fit-stock01-visual` (Torque): exit0; complete first/repeat water and dry
  checks; no native fatal diagnostics; median59.92/min49.85FPS. Full-size
  frame150 was inspected but REJECTED as a complete attachment-fit proof:
  close framing clips lower supports and water obscures mounts. Capture
  window60.87FPS, preceding60.26. Do not pass visual fit from the numeric checks.
- Harness now supports explicit native SELECT camera cycles, without host
  input or shipping camera changes, to improve larger-vehicle framing. Zero
  cycles remains default; test one cycle next. Interrupted runs now preserve
  metadata and the observed conflicting PIDs automatically. Python compiles.
- All user settings restored to FB0C93BD...C9A43E36; no game remained at final
  check. Two images inspected this turn. Fit remains opt-in; all30 final
  confirmations still pending (Sheila/Chassey candidate views are not approval).

### Repeat-cycle integration and corrected selection build

- `fit-repeat-native`, hash `09D9AF8D674B805C286003555FB1A7B7CFBA71C1AA2FD79781ED486132A5C6D4`:
  exit0, two consecutive Florida matches, Sheila then imported Chassey.
  Both produced eight initial water/five dry samples and six repeated-water/
  three repeated-dry samples with valid native modes, health and callbacks.
  This run still had the old guest-selection fixture, so imported texture/fit
  acceptance is INVALID. Performance also fails: minimum26.36FPS at
  absolute frames12181–12240 (imported match), median59.94. No overall pass.
- Corrected selection build published successfully:
  `09556D2429E1E60BCD0E074C19CD25A405AE970735C5320509301571244050E2`.
  All281 focused assertions pass. Next native evidence must confirm reservation
  and claim logs, not merely the selected participant byte. Candidate remains
  opt-in pending clean imported and all-roster visual confirmation.

### Candidate visuals (following turn, two individual captures)

Both runs below used candidate hash
`F07DAD2904531F3A53F9E47C965ED68DAF2C98F1AE4C7D7CB21313CCA75C3379`,
native Florida frame150, Auto Waterski, mode2/transition0. Both exited0;
settings were restored byte-for-byte. Full-size PNGs and exact frame-window
FPS are retained in each run's `proof.json`.

- `fit-sheila-visual`: individually inspected at1920x1080. Visible front and
  rear arm endpoints now continuously join the body; previous rear gaps are
  absent. Inner bracket geometry is visibly extended, so this remains a
  candidate for user review, not blanket native-fidelity approval. Capture
  window60.88FPS, preceding59.21; run median60.01/min38.33. Native water8/land5
  samples pass their scoped checks. Repeat activation and whole-roster fit
  remain pending.
- `fit-chassey-visual` (imported type64): FAIL/INCONCLUSIVE for attachment fit.
  Body/attachment textures are badly corrupted and body visibility is poor.
  Allocation failures begin at runtime.log line502, before gameplay starts
  (line873) or this vehicle's first fit (line1226). The already-open #14
  native texture-allocation problem blocks clean imported visual evidence;
  do not describe this as a pass because water/land telemetry succeeds.
  Capture window60.08FPS, preceding59.92; run median59.98/min50.12.
- Added process-local repeat water630 / dry870 placement and samples through
  1050, with teardown1080, to verify a second build/retraction. No continuous
  holding or healing. Fit-cache signature now ignores unrelated/outboard
  animation while retaining body/root mount changes. New build validation
  pending; no additional images this turn.
- Follow-up: `TraceCommonObjectLoadPre` forced type64 without setting registry
  selection. `ReserveGuestVramForMatch` consequently saw no active guest and
  skipped reservations. Corrected the opt-in fixture to call the same
  `SelectTypeForPlayer` used by the carousel before match reservation; stock
  fixture choices explicitly clear the prior guest selection. Thus Chassey's
  corrupt capture is INVALID fixture evidence, not proof of a shipping fit
  regression. Do not close #14 or claim clean guest visuals without retesting.

### 2026-09-05 — clear-water comparison and candidate status

- `sheila-clear-all-parts` and `sheila-clear-auto-parts` each exited 0 on
  executable `5417D10BE8A672D38AA88807966DAFFF1F380917D1EB608E28EEEC70CCE98BAC`.
  Both full-size frame150 captures were individually inspected and record
  mode2/transition0. Auto gives the clearer body/arm view: rear supports have
  visible gaps. All and Auto use identical support-local poses. Neither is a pass.
- Missing-looking class-C records are native alternate LOD meshes (`node+68`),
  not omitted connector children. Native `8002C7D0` accounts for them.
- `sheila-water-geometry` exited 0 on `F8C5CCCF6843694E5C204B06B3422227DEDE54AD652D10512CC445C7D5637A8B`.
  Direct runtime vertices matched the source body groups exactly. The offline
  analyzer resolves 225 active body triangles and rear endpoint/body gaps of
  roughly 4303–6942 fixed-coordinate units. Geometry confirms the visual gap;
  parent links alone do not establish attachment fit.
- Added a shared render-only candidate fitting body-facing arm endpoints to
  the active body surface. It preserves outboard geometry, native contacts and
  source vertices; per-instance copies are scoped to packet generation. No
  per-vehicle offset table. Candidate remains OFF unless the isolated test sets
  `RECOMPONE_V82_WATER_ATTACHMENT_FIT=1`.
- First candidate native integration (`fit-native-integration`, hash
  `5443650CA23E35E41A193EA334257D2E1D656CA74D97D8F95C2C7EF7ECD66BB7`)
  was deliberately stopped: runtime parsing yielded only two body triangles.
  It is INVALID evidence, not a passing fit. No candidate screenshot was taken.
- Traced loader `8001EA0C..8001EA80`: runtime primitive kind is shifted two
  bits and vertex references are eight-byte offsets, unlike on-disk indices.
  Corrected both, made each mesh parse atomic and the body fit fail closed.
  Added malformed-offset/partial-mesh tests; 281 focused assertions now pass.
  Native parser validation subsequently completed; visual candidate review remains pending.
- Corrected candidate `F07DAD2904531F3A53F9E47C965ED68DAF2C98F1AE4C7D7CB21313CCA75C3379`,
  `fit-parser-native`: one hidden/silent Florida run, no images, exit0 in130s.
  Sheila resolves all225 body triangles (`body-valid=True`), eight primary/LOD
  mesh copies and32 endpoint vertices. Four AI body meshes also parse validly.
  Sheila remains alive/in mode2 at all eight water samples and returns to mode0
  at all five dry samples. Render scope counters balance. Median/minimum sampled
  FPS60.01/48.83. These are integration checks, NOT attachment visual approval or
  all-roster coverage. Candidate remains opt-in. Next: full-size candidate
  screenshots, then expand to the remaining roster only if physical fit looks correct.
- Two captures used in this turn; no more until a following turn. All 30
  vehicle confirmations remain pending. User settings restored byte-for-byte
  after the stopped run (`FB0C93BD...C9A43E36`).

- Determine whether the gap exists in native pickup-driven All as well as Auto.
- Fix the shared cause; no per-vehicle offset table or individual authored fix.
- Per-vehicle native evidence for completed attachment assembly, water behavior,
  return to wheels, and repeat activation. Cover all 30, not a representative
  subset. Keep failures and inconclusive cases explicit.
- Inspect native full-size visuals individually; no sheets. Limit images shown
  in a turn to 1–2. Preserve hashes, vehicle identity, capture point and FPS.
- Single hidden/silent process, Enhanced only, V8_2_LOOSE only, original user
  settings restored. No desktop control/capture. Paused renderer goal stays paused.
- User visual review remains required; buoyancy logs alone cannot pass fit.
