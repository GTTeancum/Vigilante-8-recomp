# Vigilante 8: 2nd Offense PC — Open issues

Last updated: 2026-08-01
Test target: `V8_2_LOOSE`
Reported build SHA-256: `67A272D9DEC7E6891DF702E4BB5044EEFEE6B396D6D36956FEC2D76B8269D53A`
Current staged SHA-256: `67A272D9DEC7E6891DF702E4BB5044EEFEE6B396D6D36956FEC2D76B8269D53A`

Unchecked items below are open and required for acceptance.

## Crash and flow

- [x] **V82-OPEN-001 — Arcade defeat/quit crash (critical regression)**
  - Reproduction: lose an Arcade match, then choose to quit the match.
  - Observed: the game crashes.
  - Pass condition: the defeat-to-quit path returns through the native front-end flow without a crash, hang, stale match state, or corrupted UI.
  - Root cause and fix: the guest runtime vehicle correctly retained identity through destruction, but `func_80036C2C` had also assigned the custom callback to its native source object without registering that source. The global event-4 quit sweep therefore reached a valid custom callback with no registry entry. Identity is now registered on both callback-owning objects at assignment time and retired only at their actual allocator lifetime boundary.
  - Verification status: fresh muted loose-files runs exercise the authentic `func_80039DCC → func_80038C40 → func_80038870` lethal path for both guest and stock vehicles, wait through the native 301-tick result lock, press the documented X-to-quit action, reload `SHELL_SHELL`, and accept Up/Down input on the returned main menu. The exact staged executable completes with no crash, hang, stale match state, or corrupted return UI.

- [ ] **V82-OPEN-002 — Quest defeat return has side artifacts**
  - Reproduction: lose a Quest match and press X to return.
  - Observed: expected black bars appear, but the exposed sides contain visual artifacts during the menu transition.
  - Pass condition: authored 4:3 content remains cleanly pillarboxed throughout the complete transition, with black side bars and no stale framebuffer/VRAM content.

## Guest vehicle integration

- [x] **V82-OPEN-003 — V8 character portraits are broken (current-build regression)**
  - Observed: imported character portrait art is absent/corrupt; only the lower name remains on the left panel.
  - Pass condition: every imported V8 character uses the V8:2 selector lifecycle and displays the complete approved left-side banner without seams, corruption, overlap, or persistence into enemy selection.
  - Evidence: `codex-clipboard-5124ca47-f170-4c82-930a-81e7fa9c3d4c.png`.
  - Regression found 2026-07-30: selecting Sid Burn by wrapping left crashed in `func_8002CB74` after the full-size raw portrait was copied through SHELL's fixed VLC scratch address and overwrote the live object-bank table.
  - Fix: all 12 complete original V8 banners are uploaded as exact RGB555 pixels into the V8:2 selector's native VRAM target; the imported pose layer is skipped because the completed banner already contains it, and context zero is excluded so enemy selection remains native.
  - Verification status: a fresh full-roster loose-files sweep shows all 12 complete banners without seams, corruption, overlap, or clipping. A separate fresh acceptance run shows the native Select Enemies screen with no imported portrait persistence.

- [ ] **V82-OPEN-004 — V8 vehicle window/material artifacts**
  - Suspected area: guest-vehicle texture transparency/material interpretation.
  - Observed: window polygons render as holes in both selector previews and gameplay.
  - Pass condition: every imported V8 vehicle retains its original window geometry/materials and correct transparency without making opaque bodywork transparent.
  - Evidence: Palamino selector preview in `codex-clipboard-5124ca47-f170-4c82-930a-81e7fa9c3d4c.png`.
  - Candidate fix: converted glass now uses V8:2's retail `flags=0x10, environment=0x7FFE/0x8080` material; reflection overlays use the separate retail `flags=0, environment=0x3FFF/0x8080` family.
  - Reopened 2026-08-01: the current staged build still shows substantial window artifacts on original V8 vehicles in the native character-select menus, especially after scrolling through multiple vehicles. The earlier isolated package tests and limited selector capture did not prove repeated native menu lifecycle behavior.
  - Revised pass condition: repeatedly cycle the complete imported roster through the unchanged V8:2 selector path for multiple full loops and inspect every vehicle from changing preview angles. Glass must remain correctly textured, ordered, and translucent after VRAM churn and object teardown/recreation; no one-pass or isolated-model proof is sufficient.

- [ ] **V82-OPEN-005 — V8 character voiceovers are missing**
  - Observed: imported characters do not play their original V8 selection/gameplay voice assets.
  - Pass condition: each imported character is mapped into the existing V8:2 voice-event system using the corresponding original V8 voice assets, with no replacement of shared V8:2 voices.

## Rendering and graphics settings

- [ ] **V82-OPEN-006 — Severe terrain jagged spasms/flicker remain**
  - Observed: terrain geometry exhibits frequent jagged, spasmodic movement/flicker during gameplay.
  - Pass condition: moving-camera smoke tests on every map show stable terrain edges and surfaces, with no turn-dependent polygon popping, projected-vertex spasms, or ordering-table corruption.

- [ ] **V82-OPEN-007 — Enhanced settings UI does not match active rendering**
  - Observed: settings report the Enhanced preset while gameplay visibly uses PS1-style settings.
  - Pass condition: the displayed preset and every individual graphics toggle reflect the actual renderer state; Enhanced applies and persists its intended non-PS1 settings after startup, menu transitions, and match loads.
  - Fix: V8:2 identity is configured before preset reconciliation; named presets are authoritative; the active HLE renderer, native-resolution mode, internal scale, VRAM texture, render targets, and shader scale update together without restart. The original port's accelerated/software dither gates remain authoritative, and a V8:2-only enhanced-gameplay presentation cleanup removes the residual ordered pattern without altering menus, fidelity mode, or the accepted original V8 renderer.
  - Reopened 2026-08-01: the current staged build still visibly exhibits the same PS1-style rendering defects while the UI and configuration probe report Enhanced 3x. The previous proof established selected values and two limited scenes, but did not establish that every active primitive, fallback, menu-to-match transition, and map renderer actually consumes those values.
  - Revised pass condition: trace each Enhanced option from the saved setting through runtime application to every world/model/UI draw path, including affine/projective fallback, native-resolution framebuffer copies, overlay reloads, and menu-to-match transitions. Prove sustained motion on every map; configuration logs or a clean isolated angle do not close this item.

- [ ] **V82-OPEN-008 — Textures are too low-resolution**
  - Observed: visible low-resolution texture presentation across the game, especially loading screens.
  - Pass condition: upscale source textures without stretching or changing authored layout; world/model textures remain at or below 512×512 and menu/loading UI may use up to 1024×1024, per the established limits.
  - Evidence: `codex-clipboard-f6a192f7-fc42-46c0-8bdc-0bb619c1befa.png`.

## UI and HUD

- [ ] **V82-OPEN-009 — Defeat/game-over text spacing is corrupt**
  - Observed: “PLAYER 1 destroyed!” has a large mid-word gap (`PL    AYER`).
  - Pass condition: all defeat/game-over strings use correct glyph advance, kerning, and centering at every supported resolution and aspect ratio.
  - Evidence: `codex-clipboard-5cf6fd17-27ca-4fb0-95f0-8d48f18e7105.png`.

- [ ] **V82-OPEN-010 — Pause and objective menus are not centered**
  - Observed: pause/objective overlays and text are offset; objective text is split or displaced inside the panel.
  - Pass condition: the complete authored overlay, heading, body text, and control prompts are centered as one 4:3 UI composition without stretching.
  - Evidence: `codex-clipboard-3dfa69ad-7a6a-48e7-a480-58b5ad1e20f8.png`.

- [ ] **V82-OPEN-011 — HUD backing/content regression**
  - Observed: the weapon panel has displaced/duplicated translucent backing pieces and incorrect alignment.
  - Pass condition: all measured SVG HUD backings align exactly with their live contents and retain the accepted colors, bevels, spacing, left anchor, opacity, and two-digit ammunition layout.
  - Evidence: `codex-clipboard-09809a90-e84d-4505-9fdc-b39a6d4bf9a5.png`.

## Audio

- [ ] **V82-OPEN-012 — Default music volume is excessively loud**
  - Observed: in-match music is dramatically louder than the rest of the mix at default settings.
  - Pass condition: a clean/default configuration starts with a balanced music level, preserves user changes, and does not clip or overpower voice/SFX.

- [ ] **V82-OPEN-013 — Menu music does not play**
  - Observed: menus are silent while in-match CD music does play.
  - Pass condition: the correct converted OGG tracks start, loop, transition, and resume through the existing menu audio state machine.

## Required verification

- Reproduce and close every item against the exact staged `V8_2_LOOSE` executable.
- Exercise original and imported vehicles, Arcade and Quest defeat/return paths, pause/objective UI, and multiple maps.
- Keep audio-muted visual tests separate from audio acceptance tests.
- Do not mark an item complete from logs or static inspection alone when its pass condition is visual or audible.
- Four-fix candidate code gate: runtime build passes with zero errors; 41 relevant native-format/conversion tests pass with 2 optional-archive skips; exact candidate package, selector lifecycle, defeat/quit lifetime, and Enhanced-configuration probes pass. The 16,564,033-byte staged single-file candidate has SHA-256 `049D328108FB5748054E732FE2E50FB918E9BC4C5DFD812D8A16977AEB4A5C2E` and zero adjacent DLLs. OPEN-001/003/004 have fresh runtime and visual proof; OPEN-007 has fresh exact-build Enhanced GL motion proof.
