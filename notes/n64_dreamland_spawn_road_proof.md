# N64 Super Dreamland Spawn-Road Proof

Status: **READY FOR USER REVIEW**. Do not mark the task complete without
explicit user acceptance.

## Reference harness

- The focused RMG/CXD4 harness runs on a private Win32 desktop and receives
  frame-indexed input from its own Mupen64Plus input plugin.
- The port harness optionally intercepts the original V8 PRNG seed call at
  `FUN_8001714c` and substitutes the requested proof seed. This is gated by
  `RECOMPONE_DETERMINISTIC_GAME_RNG_SEED` and is dormant in normal play. The
  final pair records effective seed 1 in both logs.
- The canonical pair uses the deterministic null-audio sink and a forwarding
  RSP proxy. A separate stock-audio trace established that `AI_LEN` varies
  while the visual/RSP graphics/RDP timeline remains exact.
- `tools/n64_reference/bin/build_manifest.json` records the Clang build,
  source hashes, and binary hashes.
- The 2026-08-15 run manifest records matching build, frontend, proxy, input,
  ROM, real GFX/RSP plugin, and portable-runtime identities.
- The run exited normally at frame 5900 and captured 14 requested frame IDs.
  Every frame has one 640x480 framebuffer, matching DMEM/IMEM/RDRAM, one RSP
  identity record, and valid indexed RDP spans. Total RDP spans: 9,371.
- A second hidden run reproduces all 14 framebuffer, DMEM, IMEM, VI,
  display-list sequence, and ordered RDP payload digests exactly. At frame
  5460, the 17,400-byte active RSP display list, all 108 referenced road
  vertices, and the 1,024-byte XRTP-0 texture payload are byte-identical.
- All 4,969 graphics RSP task records are byte-identical. The 5,883 audio task
  records align; 569 differ only in scheduler-selected command-list
  `data_size`. Whole RDRAM differs by 1,444 bytes at frame 5460 and is retained
  as an explicit audio-state diagnostic, not used to claim route differences.

## Geometry and submission

- Source arena: 2 `XRTP` descriptors and 35 `RSEG` records (20 route type 0,
  15 route type 1). Primary XRTP width is 2.0, step is 1.0, texture ID is 1.
- Frame 5460 contains 102 runtime road triangles and 108 unique vertices with
  bounds X -2876..3007, Y -189..108, Z -1728..1648.
- The ordered scan finds 962 primary-road triangles from frame 5100,
  display-list sequence 168238, command 3105 through frame 5760,
  display-list sequence 671066, command 52092.
- Runtime code alters the surface: the recovered route builder calls
  `Terrain_HeightAt` three times for center and edge samples, and captured
  vertex Y values vary from -189 to 108. `RSEG`/`XRTP` provide the authored
  route, while runtime code terrain-conforms its submitted vertices.

## Render state and correction

- N64 XRTP 0 uses texture address `0x0034D998`, combiner
  `FC55FE04/1FFCF3F8`, other-mode low `0x0C193078`, opaque Z mode, Z compare,
  and Z update, with no forced blending.
- The N64 source flag `0x0192` contains platform-specific bit `0x0100`.
  Copying that bit made the PS1 route falsely semitransparent. The converter
  strips it, producing `0x0092`; the deployed converted arena byte-matches the
  verified artifact.
- The Enhanced renderer treats the converted N64 route as opaque and performs
  color rendering, `LEQUAL` depth comparison, and depth update in one draw.
  This preserves N64 RDP fragment ordering inside a route batch; the earlier
  color pass followed by a whole-batch depth replay did not. The behavior is
  restricted to the converted N64 route-color path, so ordinary PS1 geometry
  retains ordering-table visibility. Only source-identified water or actual
  glass enters transparent depth handling.
- Current port trace: 68,915 route triangles, 68,915 route depth tests, 68,915
  route depth writes, 68,915 single-pass depth compare/writes, zero
  route-transparent triangles, 203,430 mapped route-color vertices at the
  tick-480 summary, and zero rejected mappings. Every one of the ten
  route-bearing trace intervals has exact route/opaque/depth-test/depth-write/
  single-pass parity.

## Stability and artifacts

- Requirement-by-requirement goal audit (14/14 checks pass, zero failures,
  `READY_FOR_USER_SIGNOFF`):
  `artifacts/n64_reference/dreamland-spawn-road-goal-audit-end-to-end-final.json`
  (`9863E2801FCFA6349566DD66C00AC58304E8B76AA5A5848467B40B90D2674D6D`)
- Two independent full CUE-to-`prepare_reference.py`-to-RecompOne generations,
  post-patches, clean Release publishes, and the deployed executable are
  byte-identical. Each executable is 16,526,451 bytes with SHA-256
  `59E99E68F89A88D454149DBEDD2613E7D3D3ACD0C6ECD33A5316B42CDA609944`.
  This supersedes the earlier generated-source-only `298F76...` rebuild claim.
  The 34-byte build delta has zero road-region pixel delta across all 20 proof
  frames and does not change any route interval or route-color trace summary.
- Machine-checked full-build provenance:
  `artifacts/n64_reference/dreamland-build-reproducibility-end-to-end-final.json`
  (`BF3B0A594C43187BD4D681C3F5DEA429E6FBFB243BE08059D3F7AB1C5051F9C8`).
  It closes and hashes all 13 CUE tracks, fingerprints the preparation,
  recompiler, runtime, post-patch, generated-config, and generated-C# inputs,
  verifies both post-patch seams, and proves the first executable predates the
  independently regenerated source set while the second executable follows it.
- The deployed original-V8 build reaches Dreamland gameplay tick 360 and
  exits through the game cleanly.
- Twenty consecutive 1824x1026 Enhanced frames pass with zero VRAM fallback
  presents, zero presentation reallocations, 219 native 12-word route packets,
  maximum adjacent road-region luma change 1.698, and maximum motion-compensated
  road temporal residual 1.131 (limit 1.5).
- A second corrected port run with the same proof seed and trace flags
  reproduces all ten route interval tuples and all positive-tick route-color
  summaries exactly. All 20 high-resolution frame pairs are byte-identical:
  maximum whole-frame and road-region mean absolute RGB difference is 0/255.
- A separate run of the exact deployed executable with the proof seed variable
  absent reaches tick 360 and exits cleanly. The seed hook emits zero records,
  while all 68,915 route triangles retain exact opaque/depth-test/depth-write/
  single-pass parity across ten route-bearing intervals. This is the
  normal-play negative control for the deterministic harness.
- Combined proof:
  `artifacts/n64_reference/dreamland-spawn-road-reference-final-a-20260815/spawn-road-proof-end-to-end-final.json`
  (`0B73D4509A54F1676B0112705A80CD098DB1FA256470A1F02C538B8D58FC4F9E`)
- Corrected-port repeatability proof:
  `artifacts/n64_reference/dreamland-route-single-pass-game-seeded-candidate-20260815/end-to-end-repeatability.json`
  (`AF4D09ED774BE823636B520250032B048F05248EF7DC64A07C08A8FB8EA7FA0B`)
- N64/port side-by-side:
  `artifacts/n64_reference/dreamland-spawn-road-reference-final-a-20260815/n64-port-spawn-road-side-by-side-end-to-end-final.png`
  (`CFCB68DBEBBE2A3965F28AA3261FBEEB77605CBD85ED2EA804EF9456CB532EDF`)
- Road-focused visual proof board with full-frame context, outlined crops, and
  proof-derived render-state counters:
  `artifacts/n64_reference/dreamland-spawn-road-reference-final-a-20260815/spawn-road-visual-proof-end-to-end-final.png`
  (`B7D2070707D49A0F213D5B92644FC88570400F1894EEECB3192B05C3FCCD49A1`)
- Port stability sheet:
  `artifacts/n64_reference/dreamland-route-single-pass-game-seeded-candidate-20260815/end-to-end-run-a/road-stability-contact-sheet.png`
  (`5BF170D13DAE15D4B8DB1BB7F02DEC94C7BF7BE093B3A209616BDA554ABF31C9`)
- Static source-to-renderer contract:
  `artifacts/n64_reference/dreamland-route-single-pass-game-seeded-candidate-20260815/surface-contract-end-to-end-final.json`
  (`6B866D1DA67DB21163DBA99BCBECBA16FE3C3EFC2B375ACF7475B3EACCE882F2`)
- Normal-play deterministic-hook dormancy proof:
  `artifacts/n64_reference/dreamland-route-single-pass-game-seeded-candidate-20260815/normal-play-dormancy-end-to-end-final.json`
  (`CD8961751022C0A838E2CCE90DC5433F1DFE20FF43D81B7F0AADBDFDA95B2674`)
- Deployed executable:
  `PS1 game/Vigilante8PC.exe`
  (`59E99E68F89A88D454149DBEDD2613E7D3D3ACD0C6ECD33A5316B42CDA609944`)
- Deployed converted arena:
  `PS1 game/TERRAIN/DREAMLND.EXP`
  (`8000D187F5409F7AB225957E37153ECA678A074FA4509BA47FAA9FDFEFB23917`)
