# Guest-loading VRAM correction — 2026-09-16

Status: completed and closed by the user on 2026-09-16. This corrects the native PS1
texture allocation failure; it was not host GPU memory exhaustion or an installer
content mismatch.

## Cause and correction

The earlier existing-install and clean-install smokes chose different random
opponents. With Molo (guest type 74) against retail types 1,8,6,7, both installs
produced 52 native `Out of VRAM` errors. The other observed failing roster was
8,13,12,2. The installed mod files matched between installations.

Two general corrections preserve image fidelity:

- The light-mask converter consolidates identical `(BGR555 color, protected-light
  classification)` entries when the resulting palette fits 16 slots. This keeps
  45 previously expanded textures in 4bpp, preserving every texel's original
  color and its independent light/body classification. Textures requiring more
  classes remain 8bpp; there is no color quantization or resolution reduction.
- The native bulk bank loader (`func_8001F3C0`, allocation call at `0x8001F5C8`)
  now shares byte-identical immutable index images with matching dimensions,
  format and compression. Each texture's CLUT remains separately allocated and
  paintable. References retain the backing rectangle until its final owner
  releases it; whole-tree retirement clears the cache. This does not pin unrelated
  allocations or alter the allocator's bounds, placement algorithm or framebuffer.

For the primary failing roster, 76 image reuses saved 19,959 16-bit VRAM words
(39,918 bytes). Palette compaction alone reduced the failures but did not solve
them; the combined correction produced zero errors. There are no character/map
identity branches in the correction.

## Validation

Final staged executable SHA256 (both runnable aliases):
`56672649DA4ED36CD9B220F743E0B89321C5DD561554E2BF932ABFDB7DACC112`.

| Coverage | Result |
|---|---|
| Molo vs 1,8,6,7 — existing install | 52 errors before; zero after, normal exit |
| Same roster — clean GUI-imported install | 52 errors before; zero after, normal exit |
| Molo vs 8,13,12,2 | Zero errors after; roster/log/capture assertions passed |
| Consecutive Canyonlands native type 8 → Dreamland guest Houston type 71 | Both loads, gameplay, defeat and menu returns completed in one process; zero VRAM errors |
| Six light-mask tests | Passed, including every stock texel in both regenerated native banks |
| Guest/native texture ownership contract | Passed, including last-owner free, coordinate reuse and whole-tree invalidation |
| Existing 48 Quest allocation fixtures | Passed; bounds, page constraints, overlap and native teardown checked |

The continuous test was stopped at the next location selector after both matches:
its original exit marker expected another shell overlay notification, but the
resident shell returned directly to the menu. This was a harness exit-condition
mismatch, not a game crash; its saved script now exits at `choose_location_3`.

Visually inspected the primary Molo gameplay captures from both installations,
the alternate-roster Molo capture, and Houston in Dreamland after the level change.
Vehicle paint/lights, attachments, world textures and HUD remained rendered.
Canyonlands was checked through runtime/transition logs; its captures were
overwritten by the second match and are not claimed as visual proof. The enhanced
texture mod remained enabled. This is a targeted loading/cleanup regression check,
not a new all-roster, all-map or performance certification.

Evidence under `artifacts/vram-20260916/`:

- `verification.json`: before/after counts, actual rosters and executable hash.
- `final-roster/guest-baseline/` and `final-roster/guest-relaunch/`: native captures
  and logs for the identical failing roster on each installation.
- `alternate-roster/result.json`: executable identity and automated checks.
- `continuous/guest-baseline/`: both consecutive matches and final Dreamland proof.
- `packing-regression.log`: historical 48-case allocation replay results.

## Reproduction and deployment

Run the focused hidden, silent, process-local reproduction with a fresh output
directory (no desktop input, isolated settings):

```powershell
python tools/recompone-v8-2/run_vram_guest_smoke.py --output artifacts/vram-retest
python tools/recompone-v8-2/run_vram_guest_smoke.py --output artifacts/vram-retest-other --roster 8,13,12,2
```

`--install` can point at an already imported clean package. Inspect the two native
gameplay captures as well as the generated result; frame production alone is not
visual acceptance.

Source-asset regeneration:

```powershell
python tools/recompone-v8-2/build_vehicle_light_masks.py --output artifacts/vehicle-light-masks
$env:V82_LIGHT_MASK_OUTPUT="$pwd/artifacts/vehicle-light-masks"
python -m unittest discover -s tools/recompone-v8-2/tests -p test_vehicle_light_masks.py -v
dotnet run --project tools/recompone-v8-2/tests/GuestTextureContract -c Release -- V8_2_LOOSE/mods/v8_to_v82_guest_roster/CUSTOM.EXP
```

The generator also updates embedded `VehicleLightPalettes.json`; rebuild the host
when regenerating these assets. Deploy the generated guest `CUSTOM.EXP` and
`VEHICLES.V8R` together, plus `SHELL-VEHICLES.EXP` and `SHARED-COMMON.EXP` to their
corresponding enhanced-mod `files/SHELL/VEHICLES.EXP` and `files/SHARED/COMMON.EXP`
overrides. All four assets and both executable aliases are staged in
`V8_2_LOOSE`. Existing loose native-bank copies also match the overrides.

No user settings were changed. The user accepted the fix and requested commit, push and closure on 2026-09-16.

Temporary PPM captures were converted and removed by the capture harnesses.
Tool policy rejected cleanup of obsolete temporary test-build directories, even
with resolved explicit paths; those build directories remain under this task's
artifact folder. The protected runnable directory was not targeted.
