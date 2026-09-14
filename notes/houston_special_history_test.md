# Houston special: history hypothesis test, 2026-09-09

The September 5 commit `daf2aca` added the incorrect original-Houston → sequel-Houston/tow-truck behavior mapping. It is absent from August 28 `a70eb73` and August 31 `f14b3d0`. The historical comment claiming the weapons are equivalent is incorrect.

## Controlled experiment

Four hidden native game runs use the current identical assets, Utah slot 16, held spawn, scripted process-local special firing, and 360 gameplay ticks. A temporary diagnostic build switches only the special-selection hook between the old raw native table result and the current registry-selected result. This is a causal test of that hook, **not a replay of the entire August build**. Existing renderer, Y, Dreamland and package changes are held constant.

| Player | Old selection | Current selection |
| --- | --- | --- |
| Original V8 Houston, custom type 71 | Raw descriptor zero; callback `0x80049880`, generic fallback | `SHARED_TOWTRUCK`, callback at overlay base + `0x934` |
| Stock V8:2 Houston, type 3 | `SHARED_TOWTRUCK`, callback at overlay base + `0x934` | Same overlay and callback offset; raw descriptor passes through unchanged |

Both guest variants consume ammo 99 → 98. This disproves ammo decrement as evidence that the correct original weapon fires. The generic fallback's event table at `0x80010924` sends event 11 directly to the zero-return path; its event 12 performs generic count/removal work. It does not implement Houston's original special.

Source provenance for the original weapon is independently established from the original PS1 executable: the 13-entry callback table at `0x8005ECB0`, read by `FUN_8003d1e8`, has Houston's index 7 pointing to `0x80035CF8`. This is distinct from both tested sequel callbacks. The original aiming code previously referenced at `0x80035D40` is within this callback; copying aim alone did not port its firing behavior.

## Interpretation and limits

- Confirmed: the September 5 mapping activates the wrong sequel special.
- Not supported: reverting that mapping alone restores a working original special. In this controlled experiment it restores the generic fallback.
- Stock selection and scripted ammo use match across the toggle. This does not establish complete tow-hook attachment, towing, damage, or multiplayer parity.
- The paired guest capture at poll 350 was inspected for both variants. Images alone cannot establish the correct weapon behavior; the native callback trace is the decisive evidence.
- The general map gate is not an acceptance criterion for this experiment: held-spawn runs and multiple scripted captures fail its traversal/image-count requirements. Preserve those raw reports; do not present them as green map gates or performance results.

Reproducible probe scripts, logs, captures and machine-readable results are in `artifacts/houston-history-20260909/`. The probe restores the registry source byte-for-byte after building and the staged executable byte-for-byte after the runs. No gameplay fix is claimed by this experiment.
