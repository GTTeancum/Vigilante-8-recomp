# Codex Handoff - Terrain Visuals / Source-Driven RE

Date: 2026-05-27
Workspace: `C:\Programming\GitHub\Vigilante 8 recomp`

## Read First

At the start of the next session, read these in order:

1. `AGENTS.md`
2. `PROJECT_SCOPE.md`
3. `DECOMP_RULES.md`
4. Tail of `progress.log`
5. Tail of `decisions.log`
6. This file

User mandate still applies:

- Work autonomously.
- Use source-driven RE only. No empirical/placeholder terrain fixes.
- Headless/hidden verification only. No screenshots.
- Always deploy latest runnable build to `PS1 game\`.
- Clean old `PS1 game\v8*.log` files before any deploy intended for user testing.
- Preserve fixed-point/source semantics in gameplay/physics. Renderer is a seam, but asset decoding and source draw interpretation must be backed by the binary.

## Current Good Baseline

Latest known-good deployed exe:

- `PS1 game\v8.exe`
- SHA256: `BEF6756133EC4319C55C66EF3A88C225433B1686BDD28122DE81749EC0CD4206`
- Last verified: WILDWEST 1800-frame hidden headless auto-drive+auto-fire
- Log state:
  - `TINF materials loaded -- 256 records`
  - `XBMP terrain texture meta -- 480x192 depth=1`
  - `TerrainMesh -- uploaded XBMP texture 480x192`
  - `TerrainMesh -- uploaded XBGM sky 256x92`
  - `TerrainMesh -- uploaded 17727 vertices (5909 tris)`
  - `bad_pkts=0`
  - No fatal/assert/runtime-error/heap-corrupt/watchdog-stalled/position-stall markers

Current default level should remain `WILDWEST` unless the user explicitly redirects. The user asked not to jump levels during stress testing.

## Important User-Tested Visual Status

Good / accepted enough:

- Machine-gun bullets are visible and working.
- Pickups look good after the second-bank texture fix.
- Buildings/props are much better than before after removing fabricated terrain-side duplicate triangles and restoring source-like culling.
- WILDWEST is the current visual/physics test level.

Still unresolved:

- Terrain visual correctness is now the active priority.
- Terrain surface is textured, but likely not source-correct yet.
- User expects PS1-exact visual interpretation, especially terrain texture layering/materials/lighting if source proves it.
- Prop/building visuals may still have edge cases, but the immediate pivot is terrain.
- Known prop collision mismatch remains logged in `notes/known_runtime_issues.md`.

## Do Not Trust / Do Not Reintroduce

Do not trust the old Claude handoff or old terrain assumptions.

Do not reintroduce:

- Full solid green terrain diagnostic.
- Wireframe overlay by default.
- Fabricated second triangles for terrain-side raw XOBF packet kinds `4/5/7`.
- Forced two-sided placed XOBF drawing as a general fix.
- Any "scale by eye" change for cars/terrain.
- Starter weapon shortcuts or placeholder projectiles.
- Empirical prop collision AABBs as a substitute for source collision semantics.

Important: a previous attempt to reinterpret all raw XOBF primitive kind bits as `(packet[3] >> 2) & 0xf` regressed badly. It was backed out. Keep the source observation, but do not reapply it blindly until the whole source draw-packet path is mirrored end-to-end.

## Recent Source-Backed Visual Fixes

Recent fixes that should be preserved:

1. `platform/terrain_mesh.c`
   - Terrain-side raw XOBF visual upload no longer fabricates coplanar duplicate second triangles for raw kinds `4/5/7`.
   - Source basis:
     - `Object_BuildFromBin` normalizes raw BIN packet low nibble into source kind.
     - SLUS draw handlers around `8001c22c`, `8001c280`, `8001c2f8` advance one packet primitive and do not emit an extra host-created coplanar triangle.

2. `platform/renderer.c`
   - Placed XOBF visuals now leave GL back-face culling enabled.
   - Source basis:
     - SLUS draw handlers use NCLIP/sign rejection before emitting primitives.
     - Forcing `GL_CULL_FACE` off made hidden backs/duplicate planes visible.

3. XOBF texture bank fix:
   - Second XOBF bank atlas is wired through renderer sampler `uTex3`.
   - This fixed pickup/prop texture sampling from the wrong bank.

## Current Terrain Code Map

Main files:

- `platform/host_terrain.c`
  - EXP IFF walker
  - ZMAP/ZONE runtime chunk construction
  - TINF material loader
  - XBMP metadata
  - Globals:
    - `DAT_800911a0`
    - `g_terrain_material_render`
    - `g_terrain_xbmp_w`
    - `g_terrain_xbmp_h`

- `platform/renderer.c`
  - `terr_sample()`
  - `terr_material_id()`
  - `build_terrain_mesh()`
  - Draws ZONE terrain surface first, then placed XOBF visuals.

- `platform/terrain_mesh.c`
  - Placed XOBF terrain/object visual decode
  - XBMP/XBGM upload
  - XOBF bank texture handling

## Source Evidence For Next Terrain Pass

Most relevant source/decomp files:

- `analysis/dll/LOAD/decomp/80105550.c`
- `analysis/dll/LOAD/mips_corrected/80105550.s`
- `analysis/SLUS_005.10/decomp/8002623c.c`

Key source facts from `LOAD` `FUN_80105550`:

- Builds `DAT_8008f020` as 256 records, 0x20 bytes each.
- Each TINF source record is 0x28 bytes.
- For each material record:
  - `+0x00`, `+0x04`, `+0x08`, `+0x0c`: UV words, `u | (v << 8)`, plus VRAM base `_DAT_80065b4e`.
  - `+0x02`, `+0x06`, `+0x0a`, `+0x0e`: same tpage word from `GetTPage(...)`, or zero if `(*param_1 & 0x1000) != 0`.
  - `+0x10..+0x1c`: seven swapped halfwords used by terrain physics/material behavior.
  - `+0x1e`: diagonal/flip bit from `uVar2 >> 0xb & 1`, where `uVar2` is source word at TINF `+6` as read by the loader.
- Uses `DAT_80106dbc` eight-row UV corner table selected by `(TINF[+6] >> 8) & 7` in the decompiler's word view. Existing host uses an equivalent hardcoded corner table but may not be building the exact source render half.

Key source facts from SLUS terrain draw `8002623c`:

- Reads runtime ZONE height words from `DAT_800911a0`.
- Height is `raw & 0x7ff`.
- The high bits are render/lighting inputs:
  - Source repeatedly does `gte_ldVXY0((heightWord >> 0xb) << 7)` before `gte_ncct()` / `gte_nccs()` / `gte_cdp()`.
  - Current renderer's `terr_sample()` discards those high bits.
- Material id comes from runtime chunk offset `+0x2000`.
- Material record comes from `DAT_8008f020 + material * 0x20`.
- If record `+0x02` tpage is zero, source skips textured terrain primitive.
- Record `+0x1e` selects one of two terrain triangle diagonal/order paths.
- Source emits textured `GT3` primitives using the exact UV/tpage records, not generic height-color terrain.

## Current Terrain Implementation Gap

`platform/host_terrain.c` currently:

- Converts ZONE height words/source material IDs well enough for physics and basic rendering.
- Populates only part of `DAT_8008f020` source material records.
- Exposes `HostTerrainMaterialRender` with only:
  - `valid`
  - `u[4]`
  - `v[4]`
- Does not expose/store:
  - exact UV words
  - exact tpage/source-draw flag
  - `+0x1e` terrain diagonal/flip bit in a renderer-facing struct

`platform/renderer.c` currently:

- `terr_sample()` returns `raw & 0x7ff`; high render bits are thrown away.
- `build_terrain_mesh()` makes generic height-derived colors and multiplies XBMP textures by those colors.
- It does not mirror source material flip/diagonal selection.
- It samples TINF UVs but probably not the exact `DAT_8008f020` render records.

## Recommended Next Step

Start with terrain TINF/ZMAP/XBMP source record correctness.

Suggested narrow patch:

1. Extend `HostTerrainMaterialRender` in both `platform/host_terrain.c` and `platform/renderer.c`:
   - `uint8_t valid;`
   - `uint8_t flip;`
   - `uint16_t uv_word[4];`
   - keep or derive `u[4]`, `v[4]` for renderer convenience.

2. In `load_tinf_materials()`:
   - Build the first half of `DAT_8008f020` closer to `LOAD 80105550`, not just the physics half.
   - Store exact UV words.
   - Store source flip bit from TINF word `+6` into both `DAT_8008f020 + 0x1e` and `HostTerrainMaterialRender.flip`.
   - Keep a log with counts: material records, flip count, tpage-hidden count.
   - Be careful with endian view:
     - Existing source decomp reads words in a confusing PSX/load-byte-swapped way.
     - Quick audit showed WILDWEST/SKIRESRT/OILFIELD have zero `0x1000` hidden records when checking little-endian flag word, but verify before enforcing tpage skips.

3. In `renderer.c`:
   - Add `terr_sample_word()` returning the full runtime height word.
   - Let `terr_sample()` keep returning `& 0x7ff` for camera/height callers.
   - In `build_terrain_mesh()`, sample full words and preserve/expose the high render bits.
   - Do not claim exact PS1 GTE lighting unless actually porting the NCCT/CDP path.
   - For the renderer seam, a source-backed improvement is to use the high bits as the terrain surface's material/lighting index instead of ignoring them. Log that as a renderer approximation if done.

4. Map the source flip/diagonal carefully before using it.
   - Host vertex order is currently:
     - 0 = `(x,z)`
     - 1 = `(x+step,z)`
     - 2 = `(x,z+step)`
     - 3 = `(x+step,z+step)`
   - Current order: `{0,2,1, 1,2,3}`.
   - Source no-flip/flip paths in `8002623c.c` use different local stack vertex combinations. Do not change triangle order until the mapping is written down.

## Verification Pattern

After terrain edits:

1. Build:
   - `cmake --build build --config Debug`

2. Clean logs before deploy/test:
   - `Remove-Item 'PS1 game\v8*.log' -Force -ErrorAction SilentlyContinue`

3. Deploy if build script did not already:
   - Make sure latest `v8.exe` and dependencies are in `PS1 game\`.

4. Hidden/headless WILDWEST smoke:
   - From `PS1 game\`:
     - `.\v8.exe --headless --hidden-render --frames 1800 --auto-drive 1800 --auto-fire 1800 --level WILDWEST`

5. Check:
   - No fatal/assert/runtime-error/heap-corrupt/watchdog-stalled/position-stall markers.
   - `TINF materials loaded` still appears.
   - `XBMP terrain texture meta` still appears.
   - `TerrainMesh -- uploaded XBMP texture` still appears.
   - `bad_pkts=0`.

6. Optional broader hidden sweep only after WILDWEST passes:
   - All ten levels short 120-240 frame hidden headless.
   - Do not pivot the user-facing default off WILDWEST unless requested.

## Known Traps

- `tools/terrain_material_audit.py` may be misleading for TINF because it has previously treated TINF records as 0x20 bytes; actual TINF source records are 0x28 bytes. Fix the tool before trusting its raw dumps.
- Source renderer functions are out of scope for decomp, but their behavior is the best evidence for how to feed the rewritten renderer.
- The renderer may use floats, but asset/runtime decoding must preserve original fixed/source data.
- User is highly sensitive to "looks okay" claims. Say exactly what source path changed and what remains unproven.

## Current Mental Model

Terrain is not "missing" anymore in the early Claude sense. The current build loads:

- ZMAP/ZONE runtime terrain
- TINF materials
- XBMP terrain texture
- XBGM sky
- placed XOBF visual objects/props/buildings
- two XOBF texture banks

The remaining terrain task is source-correct interpretation:

- TINF render record construction
- exact source UV/tpage/flip semantics
- terrain high height-word lighting/color semantics
- possible PS1-style terrain blended/detail layers if the source draw path proves them

Continue from there.
