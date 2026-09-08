# V8:2 all-level visual smoke review — 2026-09-08

All19 levels visually sampled, not an exhaustive object audit. No clear repeated inside-out building defect in covered views. Dreamland flicker not reproduced in limited sequences. Visual candidates and SandFactory runtime crash remain unresolved.

Staged executable SHA256: `B14CFCCC39684ECA96D5C6C0A9BEF3FE84F7A34D2EF50E6281328B78E178866E`. No source or executable changes during this inspection.

Method: game-native hidden/silent process-local capture. Reviewed three full 1280x720 frames per slot0..17 (polls601,801,1001), plus two consecutive48-frame Dreamland sequences, each frame individually and sequentially. No desktop control. This is a quick visual smoke; routes sometimes collide, overturn or face terrain. Timing gates from the capture harness are not a performance result.

## Findings requiring follow-up

- Airplane Graveyard: solid black ground quadrilateral visible at801 and1001. Cause unestablished.
- Florida Launch: triangular blue water seams and brown polygon patches. Need isolated water/terrain check.
- V8:2 Oilfield: thin jagged dark slivers around steel framework. Need temporal/source check.
- Secret Base: diagonal texture bend in hangar header at1001; possible UV discontinuity, unconfirmed.
- Sand Factory:600-tick multi-map capture crashed after its three images with unmapped call0xFFFFF800 in func_80100100_LEVELS_V8_SANDFACT. Earlier360-tick smoke did not crash. Logs preserved; no causal attribution to renderer.
- Super Dreamland64: no object flicker reproduced in48-frame clear scenery sequence. Trees, fences, signs and props remain solid. Whole-camera change at004 is separate and unexplained. Limited views do not clear movement-dependent flicker throughout the level.

## Coverage

| Slot | Level | Observations |
|---|---|---|
| 0 | Route66 | Observatory walls, tower and porch retain outward faces. First frame obscured by combat flash; latter two unobstructed. Fine terrain mesh seams visible; no open holes confirmed. |
| 1 | Olympic | Lodge facade, trees, slope present across all views; no clear inverted faces. Fine dark terrain seams visible. |
| 2 | Bayou | Mansion columns, facade and balcony visible from outside at601. Later views face terrain; no open terrain holes seen. Water not visible in checkpoints. |
| 3 | Launch | Watchtower, fencing and mast retain solid faces. Water has conspicuous triangular blue seams and brown polygon patches near camera in601/1001; flag for follow-up rather than declare clean. Could involve submerged terrain/debris, not yet isolated. |
| 4 | SteelMill | Interior roof/walls, vats and nearby column retain intended faces in601/1001.801 is collision/steep closeup and weak coverage. No clear inverted surface or missing facade. |
| 5 | Nuclear | Pond/bank, fencing and distant buildings rendered; no obvious reversed surfaces. Very dark scene and limited building closeup coverage. |
| 6 | Oilfield | Near steel framework is outward-facing overall, but thin jagged black slivers visible around members and right edge; needs focused temporal/source check. Explosions obscure part of601. |
| 7 | Harbor | Warehouse facade, STOP gantry and retaining wall present; latter views face bank. No obvious reversed building faces; no water coverage. |
| 8 | SecretBase | Hangar exterior/interior faces retained. Upper corrugation strip at1001 has a diagonal texture bend near left edge of header; flag possible mapping kink pending source/view confirmation. |
| 9 | SandFactory | Bank, billboards and near wall visible. Billboard damage progresses with combat, so holes cannot be called culling defects from these frames. No clearly reversed wall; broad scenery coverage limited by bank-facing route. |
| 10 | V8_Oilfield | Terrain cliff and vehicles retain visible faces. No scenic buildings in these views; object-culling coverage insufficient. |
| 11 | AirplaneGraveyard | Solid black quadrilateral on valley floor at801 and1001 is conspicuous and requires investigation. Aircraft debris at601 during combat; insufficient intact-aircraft coverage. Cause of black patch not established. |
| 12 | WildWest | Terrain banks remain present; vehicle overturns during combat. No buildings in these views; object-culling coverage insufficient. |
| 13 | HooverDam | Facility exterior, fence, road and near tower faces remain visible.801 is steep overhead combat view;1001 close wall obscures much of scene. No clear repeated inside-out wall defect. |
| 14 | ValleyFarm | Bridge arch, retaining walls, trees and crates visible. Water meets concrete bank without obvious missing edge in these views. No clear inverted faces. |
| 15 | CasinoCity | Building exterior, canopy support, street lamps and distant fuel station visible.801 near wall obstructs view;1001 clear street. No clear inverted faces. |
| 16 | Canyonlands | Bridge truss and utility poles present at601; later terrain and small bridge remain present.801 combat flash obscures right side. No clear inverted faces. |
| 17 | SkiResort | Lift pole and cables present. Snow terrain has fine visible seams; no open holes confirmed. Camera faces slope in all checkpoints, so lodge/object coverage insufficient. |
| 18 | SuperDreamland64 | All 48 consecutive full native frames individually reviewed in order. Trees, fence, signs, benches and props present. No isolated object disappearance/reappearance reproduced. Whole camera changes at frame004; cause not established. Mostly fixed camera after that, so this does not clear view-dependent flicker elsewhere. |

## Native visual evidence

- [Airplane Graveyard black patch](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-all-levels-visual-review-remaining/recompone_present_aftergameplay2_1001_1280x720_fxaa.png)
- [Florida water](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-all-levels-visual-review/recompone_present_aftergameplay4_1001_1280x720_fxaa.png)
- [Oilfield framework](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-all-levels-visual-review/recompone_present_aftergameplay7_0801_1280x720_fxaa.png)
- [Secret Base header](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-all-levels-visual-review/recompone_present_aftergameplay9_1001_1280x720_fxaa.png)
- [Dreamland clear scenery](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-dreamland-flicker-held/recompone_present_gameplay_0601_033_1280x720_fxaa.png)
- [Detailed review record](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-all-levels-visual-review/review.json)

## Follow-up repair status

See `notes/v82_visual_issue_fixes.md` for the current build, repaired terrain/UV/viewport/depth/overlay faults, native proofs and remaining observation limits. The unresolved labels above describe the original smoke build, not the latest repair state.
