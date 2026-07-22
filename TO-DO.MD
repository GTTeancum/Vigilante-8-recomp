# Vigilante 8 PC Reference - Soak and Coverage Goal

Last updated: 2026-07-22 11:40 EDT
Status: ACTIVE

## Goal

Run deterministic soak coverage across multiple retail maps, validate split-screen
gameplay separately, close every actionable reference-handoff gap that does not
require subjective user evaluation, and leave a short explicit sign-off queue.

## Acceptance criteria

- [x] A committed soak runner can select multiple named maps, run them for a
      bounded duration, retain per-map evidence, and fail on crashes, hangs,
      missing gameplay entry, unmapped calls, heap/terrain faults, or stale logs.
- [x] At least one substantial soak pass covers every selectable arena; any
      failing arena is fixed or documented with reproducible evidence.
- [x] Cooperative and Versus split-screen each reach active gameplay in clean,
      independent runs and remain stable under simultaneous two-pad input.
- [ ] Direct BIN/CUE and loose-file deployments remain byte-identical and pass
      focused regressions after any runtime changes.
- [ ] Temporary test processes and handoff logs are cleaned; cards and settings
      are preserved; coherent changes are committed.

## Work queue

### Multi-map soak

- [x] Recover and document the retail arena-selection order and stable menu
      synchronization points.
- [x] Create parameterized/stage-aware fixtures for all arenas.
- [x] Implement a bounded soak runner with timeout, responsiveness, stage,
      framebuffer-change, audio-init, and fatal-marker checks.
- [x] Run an initial all-map reachability matrix.
- [x] Run extended multi-map soak coverage and triage/fix failures.

### Split-screen

- [x] Make Player 2 gamepad defaults usable without manual remapping while
      preserving explicit user bindings.
- [x] Create separate Cooperative and Versus gameplay fixtures.
- [x] Validate simultaneous Player 1/Player 2 movement and weapon inputs.
- [x] Validate split-screen rendering, match progression, teardown, and clean
      relaunch independently from single-player soak results.

### Other autonomous coverage

- [x] Exercise loose-file content mutation, size changes, deletion/fallback,
      ambiguous basename handling, and raw-sector disc fallback in an isolated
      temporary tree.
- [x] Add non-invasive live-controller diagnostics and validate SDL/XInput
      discovery against the attached XInput-compatible device.
- [ ] Exercise hot-plug state handling and rumble calls where automation permits.
- [ ] Test the victory/result path without directly writing gameplay state.
- [ ] Extend gameplay coverage into Quest where deterministic automation permits.
- [ ] Audit and either fix or explicitly disable the unsupported single-file
      C# mod/runtime-detour path.
- [ ] Refresh stale reference documentation and deployed-path instructions.
- [ ] Audit available state-trace/comparison scaffolding and advance the
      PS1-versus-reference fidelity work as far as local inputs permit.

### Future QOL and presentation

- [ ] Add optional perspective-correct texture mapping to eliminate the
      PlayStation's affine texture warping/projection distortion, while
      retaining a fidelity mode for exact reference comparisons.

## Deliberate boundaries

- The reference executable remains Windows x64; the shipping Xbox game is a
  separate 32-bit x86 implementation.
- Loose mode still requires the CUE for stable LBAs, XA/STR raw sectors, CDDA,
  and fallback files.
- Renderer, raw controls, and host audio output are rewrite seams; gameplay
  state and timing remain the fidelity target.

## User sign-off queue

- [ ] Physical-controller feel: buttons, sticks, triggers, dead zones, hot-plug,
      and rumble on the user's actual controller.
- [ ] Visual acceptance of every arena and both split-screen layouts after the
      automated stability gates pass.
- [ ] Any PS1-versus-reference comparison that requires an externally
      instrumented original-game/emulator capture not available locally.

## Evidence log

- 2026-07-22: Goal created from deployed commit `9da3806`; both current
  `Vigilante8PC.exe` copies are byte-identical and the Oil Fields one-player
  vertical slice is the known-good baseline.
- 2026-07-22: `tools/recompone-v8/run_reference_soak.py` now generates
  deterministic stage-aware fixtures for all ten retail arenas and independent
  Arcade, Cooperative, and Versus paths. The current memory-card state exposes
  eight arenas; Sand Factory and Secret Base remain locked bonus selections.
- 2026-07-22: Initial eight-arena reachability exposed four omitted indirect
  callbacks plus a relocated jump-table defect. After source-backed repairs,
  `reference/generated/soak/all-arenas-reachability-fixed/summary.json` records
  8/8 passes with correct overlays, SDL audio, gameplay heartbeats, and changing
  HLE framebuffer hashes.
- 2026-07-22: Separate clean processes proved Cooperative as internal match
  mode 4 and Versus as mode 3. Each run rendered two distinct live viewports,
  changed both player positions under simultaneous pad scripts, initialized
  audio, remained responsive, and quit through the original pause confirmation.
  The host also discovered the attached `Xbox One Controller` through SDL.
- 2026-07-22: Extended coverage now spans all eight selectable arenas. Ski
  Resort, Valley Farms, Hoover Dam, Ghost Town, and Aircraft Graveyard ran the
  full 120-second gameplay window; Canyonlands, Casino City, and Oil Fields
  completed matches, dismissed results normally, reloaded their arenas, and
  resumed physics. Casino City's reproducible scatter-animation spin was fixed
  at its arena-specific callback boundary and passed the corrected 120-second
  gate. Liveness now uses continuous framebuffer probes while wheel-body ticks
  remain diagnostic telemetry.
- 2026-07-22: The reusable isolated loose-file suite passed edited content,
  changed lengths, deletion-to-disc fallback, unique and ambiguous basename
  handling, cooked-sector overrides, and raw 2352-byte disc fallback without
  modifying either handoff data tree.
