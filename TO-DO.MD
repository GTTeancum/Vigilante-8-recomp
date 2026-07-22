# Vigilante 8 PC Reference - Soak and Coverage Goal

Last updated: 2026-07-22 14:59 EDT
Status: READY FOR USER SIGN-OFF

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
- [x] Direct BIN/CUE and standalone loose-file deployments use the same
      executable and pass focused regressions after any runtime changes; the
      loose deployment must work with the entire `BINCUE` tree unavailable.
- [x] Temporary test processes and handoff logs are cleaned; cards and settings
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

- [x] Replace the CUE-backed override layer with a standalone loose filesystem
      that boots, streams STR/XA, plays loose music, and reaches gameplay with
      no BIN/CUE files available.
- [x] Exercise exact-path content mutation, size changes, missing-file errors,
      ambiguous basename rejection, and cooked/raw stream reads in an isolated
      standalone tree suitable for future mods.
- [x] Add non-invasive live-controller diagnostics and validate SDL/XInput
      discovery against the attached XInput-compatible device.
- [x] Exercise hot-plug state handling and rumble calls where automation permits.
- [x] Test the victory/result path without directly writing gameplay state.
- [x] Extend gameplay coverage into Quest where deterministic automation permits.
- [x] Audit and either fix or explicitly disable the unsupported single-file
      C# mod/runtime-detour path.
- [x] Refresh stale reference documentation and deployed-path instructions.
- [x] Audit available state-trace/comparison scaffolding and advance the
      PS1-versus-reference fidelity work as far as local inputs permit.

### Future QOL and presentation

- [ ] Add a PS1 color-dithering toggle and default the enhanced presentation
      mode to undithered output so flat textures and gradients remain clean;
      retain authentic ordered dithering in the fidelity/reference mode.
- [ ] Add optional perspective-correct texture mapping to eliminate the
      PlayStation's affine texture warping/projection distortion, while
      retaining a fidelity mode for exact reference comparisons.
- [ ] Add a geometry-edge anti-aliasing option (prefer MSAA or an equivalent
      edge-only pass) that preserves crisp source textures and PS1 dithering;
      retain full-frame FXAA and Off as separately selectable modes.

## Deliberate boundaries

- The reference executable remains Windows x64; the shipping Xbox game is a
  separate 32-bit x86 implementation.
- Standalone loose mode must never discover, open, or fall back to BIN/CUE data.
  Its path-stable asset tree is also the supported foundation for future mods.
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
- 2026-07-22: The first isolated loose-file suite proved override mechanics,
  but its deletion/raw-sector cases depended on disc fallback. The clarified
  standalone/modding requirement invalidated that acceptance result; the item
  is reopened until the same executable passes with `BINCUE` wholly absent.
- 2026-07-22: The attached Xbox controller survived an automated SDL
  close/reopen rescan with the same Player 1 assignment. Suppressed motor tests
  routed independent start/stop requests to both controller slots; physical
  vibration strength and real unplug/replug feel remain user sign-off items.
- 2026-07-22: Standalone loose mode now resolves the retail ISO layout from an
  embedded metadata-only manifest, requires all 69 content assets at exact
  paths, streams 23 full-sector STR/XA files, and maps twelve named OGG tracks
  to their original CD positions. With the entire `BINCUE` directory renamed
  away, the published single-file host streamed ACTLOGO, loaded SHELL/LOAD and
  OILFIELD, initialized SDL audio, and sustained active gameplay.
- 2026-07-22: The replacement loose regression suite passed live content and
  length changes, exact-path precedence over decoys, explicit missing data and
  music errors with no fallback, and cooked/full-sector stream hashes. A long
  result run decoded `SOUNDS/DEFEAT.XA` from loose sectors; incremental OGG
  sector decoding then passed the normal 15-second liveness gate for 90 seconds.
- 2026-07-22: Final direct-disc and standalone deployments are byte-identical
  at SHA256 `C8B218A06905FB672443128B827A148495958B3F6899B56E98D30CA90647BFE9`.
  Default probes selected CUE mode in `BINCUE` and standalone mode in `PS1 game`;
  both cards and settings remained byte-identical through media conversion and
  executable deployment.
- 2026-07-22: The loose exact-path tree is confirmed as the deployed mod
  foundation, with no disc fallback. The C# loader now compiles source from
  loaded assembly metadata and a framework-dependent build verified a real
  `main/start` pre-hook. MonoMod declares single-file applications unsupported,
  so the deployed layout now rejects C# source mods with an explicit diagnostic
  instead of reporting inactive hooks as loaded; loose asset mods remain active.
- 2026-07-22: The input-only victory fixture entered `I_WILL_NOT_DIE` and
  `DEADLY_MISSILE` through the original passcode editor, launched a one-enemy
  Oil Fields match, and supplied only ordinary controller input afterward. The
  enemy reached real soft/full destruction at gameplay ticks 11,710/12,010;
  the retail result logic reported the player alive at tick 12,521, rendered
  `YOU WIN!` with `PLAYER 1 vs ENEMIES 0`, and accepted its Circle dismissal.
- 2026-07-22: Quest automation selected its vehicle and route, confirmed the
  original post-load continue wait, loaded Ski Resort, and sustained 900 active
  gameplay ticks with moving position/velocity, acceleration, weapon fire, and
  weapon cycling before quitting through the original pause confirmation.
- 2026-07-22: The reference now emits pointer-neutral per-physics-tick JSONL
  state. The comparator reproduced the 16,260-tick victory trace and 900-tick
  Quest trace exactly against themselves, then correctly found Arcade-versus-
  Quest divergence at tick 1. Native snapshot scaffolding now records the
  instruction-confirmed one-byte PRNG carry; external PS1 capture remains a
  user-sign-off dependency rather than an autonomous local task.
- 2026-07-22: Final x64 single-file publication produced byte-identical
  `Vigilante8PC.exe` deployments at SHA256
  `1DFD56EFA33760E6E18072DFFCDB22775AF2520D9215FF7976971E302DA1D9C7`.
  The `BINCUE` copy selected direct CUE mode and the `PS1 game` copy selected
  standalone loose mode with 69 exact-path assets. Each independently completed
  Oil Fields gameplay, two pause/resume cycles, confirmation cancel, and normal
  confirmed quit with zero fatal markers. Cards and settings retained their
  pre-test hashes; all 34 temporary deployment captures were removed.
