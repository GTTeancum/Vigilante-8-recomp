# Vigilante 8 PC Reference - Soak and Coverage Goal

Last updated: 2026-07-23 EDT
Status: AUTONOMOUS WORK EXHAUSTED - AWAITING USER SIGN-OFF

## Current goal

Run a five-hour standalone loose-files crash campaign with the following hard
coverage gates:

- [x] Play every match as Y the Alien.
- [x] Exercise all ten retail levels, explicitly including Sand Factory and
      Secret Base.
- [x] Let every launched level reach the original result screen; elapsed-time
      survival or forced pause exits do not count as completed matches.
- [x] Programmatically attach all six retail weapon kinds through the original
      allocation, callback, bone, and weapon-slot routines.
- [x] Fire every attached weapon in selected-weapon pairs and retain exact
      hit-counter WHAMMY confirmation for kinds 1 through 6.
- [x] Enter both original directional-plus-machine-gun special-attack commands
      for each of the five standard weapons and retain callback/ammo-consumption
      proof for all ten attacks.
- [x] Continue launching varied-map coverage for at least five wall-clock hours,
      then let the final bounded match finish.
- [x] Preserve and classify every crash, hang, unmapped call, early exit,
      liveness false positive, and unmet coverage gate.
- [x] Commit coherent harness or source-backed defect corrections frequently;
      publish the final validated executable byte-identically to `PS1 game` and
      `BINCUE` without changing cards or settings.

2026-07-23 post-soak evidence: the five-hour campaign ran 18,013.526 seconds,
completed 75 bounded runs, naturally completed every arena at least four times,
confirmed all six WHAMMY kinds, and proved all ten special attacks. Retained
failures were classified as heap-validator false liveness, old manual weapon
attachment heap-header overwrites, and an Aircraft Graveyard frame-pacing
liveness sensitivity; no exact `Unmapped call` occurred. Focused fixed-binary
regressions passed Ghost Town, Casino City, Ski Resort, and Aircraft Graveyard
to natural result screens, with Aircraft requiring a 60-second liveness window
because its sampled stack was sleeping in `FrameClock.Throttle`.

2026-07-23 closeout: post-soak fixes removed the test-induced manual weapon
allocation path, gated/sampled expensive heap validation, disabled periodic
display probes by default, and cached tiny VRAM reads. The final published
single-file `Vigilante8PC.exe` is deployed byte-identically to `PS1 game` and
`BINCUE` at SHA256
`7628FBFD9E1B701F0D55B6F0BC016D8ACE0425DD5E95FAC6CFCDFB22D61D07DF`. Source
probes confirmed `PS1 game` selects standalone loose files with 69 assets and
does not open disc media, while `BINCUE` selects the direct CUE. Both deployed
copies passed independent Oil Fields gameplay smoke runs with clean in-game
exits.

## Previous soak goal

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

### User-found defect remediation

- [x] Recover the Aircraft Graveyard crash, translate the omitted native
      callback, and pass a sustained arena regression under movement and
      weapon load.
- [x] Verify reverse from original pad input through vehicle motion and make
      the retail Down=brake/reverse and Square=handbrake mapping explicit in
      the host input UI.
- [x] Apply the enhanced-mode dithering default to both accelerated and
      software PS1 raster paths, retaining a live fidelity toggle.
- [x] Reproduce and correct the Canyonlands `FUN_80024998` crash at
      `0x380C110F`. Register capture proved the diagonal-neighbor lookup
      returned null at original return address `0x80024C70`, unlike the four
      axial cases which check null; the compatibility boundary now represents
      that exact missing neighbor with a read-only empty pool cell. Ten fresh
      Canyonlands matches passed through 70,200 aggregate gameplay ticks.
- [x] Classify the Ski Resort post-defeat presentation stall as a
      result-screen automation false positive. With result/title transitions
      excluded from the gameplay framebuffer gate, two fresh matches reached
      3,960 and 6,480 ticks and returned cleanly to the title screen.
- [x] Diagnose Casino City's fragmented emulated-heap exhaustion: allocation
      failure is recoverable pressure, not a fatal condition; a subsequent
      Canyonlands run logged it and continued normally through 9,900 ticks and
      result/title teardown.
- [x] Isolate the intermittent cross-arena Object_PreTick stall. A Ghost Town
      live stack exposed main callback `0x800378D0` event 5 dispatching through
      `0x80037ADC`, clearing the child cursor, and returning continue; the exact
      cleared-child state now aborts the outer animation walk. Casino City,
      Hoover Dam, and Ghost Town subsequently completed long result/title runs.
- [x] Regenerate and regression-test the newly recovered main-executable
      entrypoint `0x8003DFFC`, reached from Hoover Dam gameplay at tick 720.
- [x] Fix Hoover Dam callback `0x80100A18` clearing its animation cursor on
      event 5 while returning success; three independent post-fix matches
      reached 7,380, 5,940, and 4,500 ticks with clean result/title teardown.

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

- [ ] Add widescreen gameplay support by expanding the horizontal 3D view;
      preserve the original aspect ratio and placement of HUD/UI elements, and
      pillarbox intros and menus rather than stretching them.
- [x] Add a PS1 color-dithering toggle and default the enhanced presentation
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

The retained screenshots and controller checklist are collected in
`reference/SIGNOFF.md`.

- [ ] Physical-controller feel: buttons, sticks, triggers, dead zones, hot-plug,
      and rumble on the user's actual controller.
- [ ] Visual acceptance of every arena and both split-screen layouts after the
      automated stability gates pass.

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
  instruction-confirmed one-byte PRNG carry. Comparison against an externally
  instrumented PS1/emulator is explicitly outside this goal; the local
  trace/comparator audit is complete and supplies the later native-port oracle.
- 2026-07-22: Final x64 single-file publication produced byte-identical
  `Vigilante8PC.exe` deployments at SHA256
  `1DFD56EFA33760E6E18072DFFCDB22775AF2520D9215FF7976971E302DA1D9C7`.
  The `BINCUE` copy selected direct CUE mode and the `PS1 game` copy selected
  standalone loose mode with 69 exact-path assets. Each independently completed
  Oil Fields gameplay, two pause/resume cycles, confirmation cancel, and normal
  confirmed quit with zero fatal markers. Cards and settings retained their
  pre-test hashes; all 34 temporary deployment captures were removed.
- 2026-07-22: Windows crash evidence identified Aircraft Graveyard event 17
  dispatching omitted main-executable wrapper `0x80021830`. Raw retail MIPS
  established its exact body, regenerated output now registers it, and the
  corrected arena passed 3,600 gameplay ticks under movement and weapons.
- 2026-07-22: A sustained input trace confirmed Square/X is the retail
  handbrake, while Down (D-pad or left stick) is brake/reverse. Down changed
  the active-low pad word to `0xFFBF` and established strong backward vehicle
  motion; the host input labels now expose both functions without changing
  original gameplay controls.
- 2026-07-22: PS1 color dithering is now gated in both accelerated and
  software rasterizers. Enhanced mode reports dithering Off by default;
  enabling the Display setting restores the original ordered-dither path,
  and both configurations passed independent Oil Fields gameplay runs.
- 2026-07-22: The corrected x64 single-file build is deployed byte-identically
  to both layouts at SHA256
  `6DF36FCEA232032F0CEE6AE9AA22BBDE4637CD44844E1B3B6D03946C0292AEEB`.
  The direct copy selected its CUE; the standalone copy selected exactly 69
  loose assets and did not open disc media. Both reached Oil Fields, reported
  dithering Off, completed 360 gameplay ticks, and exited through the original
  pause confirmation. Cards and settings retained their prior hashes.
- 2026-07-22: The three-hour crash campaign found Hoover Dam direct target
  `0x8003DFFC` inside Ghidra's overlapping `0x8003DFD8` extent. Registering the
  independent entry exposed a second defect: overlay callback `0x80100A18`
  cleared animation cursor `+0x60` on event 5 while returning success, causing
  `Object_PreTick` to spin. The exact cleared-cursor boundary now returns the
  animation walker's abort result, and three fresh matches passed.
- 2026-07-22: Long-soak liveness now captures a live `dotnet-stack` report
  before terminating a stalled process. Natural result/title return is accepted
  without requiring an immediate same-arena relaunch, and heap-allocation
  pressure is recorded rather than treated as fatal by itself.
- 2026-07-22: The three-hour loose-file crash hunt ran from 16:12 through the
  19:12 cutoff and retained 64 bounded cases across nine campaigns, totaling
  376,380 gameplay ticks. The early matrix deliberately remains in the evidence
  set: its failures separate genuine unmapped-call, collision, and animation
  defects from corrected result-transition false positives and recoverable heap
  pressure.
- 2026-07-22: A live Ghost Town stack isolated the common nested-animation
  stall at callback `0x800378D0`; the source-backed cleared-child termination
  fix subsequently completed long Casino City, Hoover Dam, and Ghost Town
  matches with normal result/title teardown.
- 2026-07-22: Fault-time registers reduced Canyonlands address `0x380C110F` to
  the diagonal-only null return at `0x80024C70`. After the non-mutating empty-
  neighbor compatibility fix, ten consecutive Canyonlands matches passed for
  70,200 aggregate gameplay ticks. The closing post-cutoff bounded runs also
  passed Ghost Town at 6,120 ticks and Hoover Dam at 7,380 ticks.
- 2026-07-22: The final self-contained build is deployed byte-identically to
  `BINCUE/Vigilante8PC.exe` and `PS1 game/Vigilante8PC.exe` at SHA256
  `FCA1987F77D0C3151FAC54C6B8386F1D1F9A8EE7863DC0B7E327BC0C8604174A`.
  No stale handoff logs were present, and both memory cards plus all settings
  retained their pre-deployment hashes. Autonomous work is exhausted; only the
  two subjective checks in the user sign-off queue remain.
