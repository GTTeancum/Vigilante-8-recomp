# Visual issue repairs — 2026-09-08

Current attachment follow-up: the user accepted rigid deployed placement on 2026-09-08. See `notes/v82_rigid_water_attachments.md` for current build C598A424 and verification. The withdrawn stretching and original-gap statements below are historical.

Build status: attachment fit experiment withdrawn at user request; original attachment geometry restored.

Staged executable: `V8_2_LOOSE/Vigilante82PC.exe`.
SHA256: `83FEC2293A6A9EA4B7664B1DE174E98DF6262974F9B6F06925FF0F9F6B23A9DA`.
PS1 source assets retained. No fidelity settings reduced or performance benchmark requested/run for this repair.

## Confirmed repairs

- Water attachments: WITHDRAWN. The shared body-fit rule extended support vertices and is not recovered PS1 animation. Removed its integration from object rendering at the user's request. Original separation remains unresolved. The following describes historical experimental evidence only: Native free-physics water/dry lifecycle completed through tick 1050 without fixture failures. See `artifacts/visual-issues-attachment-default/proof.json` and `notes/v82_water_attachment_verification.md`. This is one fresh default-on vehicle proof plus historical all-30 shared-rule coverage, not a new all-30 run.
- Airplane Graveyard black ground patch: the native coarse corner test discarded a cell whose interior slopes remained visible. Retain rejected cells per native camera traversal and submit their authored samples through enhanced clipping. Matched-pose images in `artifacts/visual-issues-airgrave-fixed-view` (before) and `artifacts/visual-issues-airgrave-terrain-after` (after) individually reviewed; black hole is gone.
- Secret Base warehouse header: a source midpoint UV broke a straight edge. Correct the shared midpoint, including adjacent atlas crops of the same texture. Clean before: `artifacts/visual-issues-secret-clean-before`; accepted after: `artifacts/visual-issues-secret-atlas-after`. Header corrugation is straight. Earlier owner-probe and pre-atlas images are not acceptance evidence because they interrupted batching or left the crop mismatch unresolved.
- Widescreen shoreline alignment: world geometry received the horizontal viewport margin twice while reconstructed water received it once. Removed the duplicate addition. Matching Florida captures show a centered vehicle and aligned land/water projection.
- Water base: the flat underlay no longer writes depth that can reject displaced surface troughs. Actual surface retains depth writes and water retains its intended culling exemption.
- SandFactory transition: validate relocated image identity before using cached overlay callbacks. An address reused for original Oilfield could dispatch SandFactory code. Synthetic reuse regression passes, and the earlier seven-level native transition run exited cleanly.
- Follow-up crash found during final Florida capture: collapsed triangles could match one corner twice and derive an invalid third-corner index in the header correction. Require distinct endpoints on both triangles; added regressions for both orders.

## Validation and limits

526 mesh clipping/mapping assertions; 349 transformation assertions; 3 overlay reuse checks passed. `git diff --check` passed. Build has existing trim warnings.

Florida still has visible faceted wave/shore intersections and a distant underlay edge; this does not establish that all polygonal water artifacts are resolved. Oilfield's large jagged beam outlines match source texture transparency, but the isolated transient sky sliver in the original combat capture was not reproduced or conclusively explained. Dreamland flicker was not reproduced in the earlier 96 individually inspected frames; no claim of proving it impossible.

Native proof images use the game's capture facility. No desktop capture or host input was used. Timing fields automatically recorded by the functional fixture are not treated as a new efficiency result. Held-camera fixtures intentionally fail traversal coverage; functional results are reported separately from those gate expectations.

Final five-map follow-up was interrupted after the user rejected support stretching. It is not a completed smoke pass. The preceding final Florida capture exited cleanly and passed functional checks, while intentionally held camera coverage checks failed; that image contains the now-rejected fit and is not current attachment acceptance.

Source check after removal: native `8003E32C` creates transition parents, `8003E774` animates the 32-tick transition and calls `8003E4A8` to rebuild attachments at midpoint. AutoWaterski requests that routine. This establishes the code path, not rendered equivalence with original hardware. No substitute geometry correction is enabled.

Native unstretched validation: `artifacts/visual-issues-attachments-unstretched/proof.json`, executable 6C1B926F. Exit 0; all initial and repeated water/dry samples pass through tick 1050; no fitting log entries. Individually inspected native frame150: original support geometry, partly obscured by water. This is removal/lifecycle evidence, not proof that the underlying gap is fixed. 349 transformation assertions pass after removal.

## Level finalization after attachment workaround removal

The shared frustum now uses the same symmetric integer margins as the framebuffer: native 320 -> 428 at 16:9, instead of separately rounding to 427. Both default object and terrain/water frusta agree; authored width is restored afterward. Explicit diagnostic overrides remain. 526 mesh assertions pass, including all six 16:9/21:9 x 320/512/640 combinations for both frusta and restoration.

`artifacts/visual-issues-florida-width-final/visual-review.json` records the individually inspected matching native capture. The thin uncovered right edge of distant water is gone. The adjacent surface-only diagnostic identifies that far strip as the separate flat water layer; it is not a deleted near shoreline tile. The authored sampled wave grid still has visible facets. No mesh stretching, extra fading, terrain changes, or geometry suppression was used to hide those facets. Florida native capture run exits cleanly with no fatal or packet pool drops.

`artifacts/visual-issues-final-smoke/visual-functional-review.json` records the completed five-level run and each inspected checkpoint. All functional/transition checks passed, including Sand Factory -> original Oil Fields. Full route/timing/capture-count gate remains false and is not presented as a pass. Hoover exterior faces remain solid. Oilfield's persistent stair-stepped beam contours match the source texture; the separate transient sky streak is not reproduced and remains unclassified, so no speculative correction was applied.

Earlier source-backed Airplane Graveyard terrain and Secret Base warehouse before/after evidence remains valid; their algorithms were not changed by the subsequent attachment removal and viewport rounding repair. This is closure of reproduced faults, not a claim of exhaustive visual perfection in every level.

Final Dreamland check: `artifacts/visual-issues-dreamland-final/visual-review.json`. All 12 frames individually inspected sequentially; trees, castle sign, fence, benches and distant props remained stable. Clean exit, no fatal, deterministic completion, no edge pool drops. Short held view does not establish all-camera flicker absence.
