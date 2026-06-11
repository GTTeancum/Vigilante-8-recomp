# Parked WIP Branch

This branch preserves unstable local work and investigation artifacts from the final weapon/pickup debugging pass.

The code baseline was rolled back to the last good committed runtime before parking. The dirty tracked state from the abandoned pass is archived as:

- `analysis/pre_restore_dirty_state_20260611_081634.patch`

That patch is intentionally not applied here because it no longer applies cleanly to the restored baseline. Treat it as historical evidence and a recovery aid, not as a ready-to-merge patch.

Known unresolved regressions from the abandoned pass:

- Alpha blending regressed on multiple map/object visuals.
- Weapon pickups were sunk into terrain or attached at bad offsets.
- Weapon pickup mesh faces and attached weapon geometry were incomplete.
- Left/right steering regressed.
- AI opponent spawning appeared unreliable.
- Pickup population showed duplicates and limited special weapon variety.

The stable parking branch is `main`; this branch exists only so the investigation trail is not lost.
