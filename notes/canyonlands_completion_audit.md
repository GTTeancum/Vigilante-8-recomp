# Rendering goal: completion evidence audit

Completed implementation and native visual verification: 2026-09-14T09:15:13.995304-04:00.
Staged build: A7AE25FB8B6E7C590480F89B9456276A2BDB5659C7ACA8904D856999B2E3D86C.
The user accepted and closed bridge item4 on September14. Original item2
was also accepted and removed; the to-do list now contains three open items.

| Requirement | Evidence | Limit |
| --- | --- | --- |
| Shared terrain filtering | Per-mip atlas bounds and anisotropy correction; Canyonlands/Route66 comparisons; user accepts remainder September14 | Finite camera coverage |
| Canyonlands road/shoulders | Native XRTP/JUNC ordering, deferred road blending and source attachment endpoints; user accepts remainder September14 | Source-topology adaptation in Enhanced; native gameplay and assets preserved |
| Far bridge approach | Source-matched support at Z3890.9504 overwrote sand at Z1875.962 without depth comparison; controlled compare/update A/B removes artifact | This is the supplied entrance defect, separate from historical near-clip correction |
| Bridge crossing | Final native captures240 and300 show continuous deck and exit; matching entrance yaw3353 clean | Initial120 is settling; later terrain frames are not bridge proof |
| Shared scenery regression | Final Hoover native capture retains exterior tower fronts; existing source winding contracts pass | Bounded control, not every map/camera |
| No map-specific workaround | Coherent opaque WorldObject depth comparison; vehicles/effects/fallbacks retain contracts; no content-name/ID branch | Modern depth composition adapts original polygon-order rendering |
| Texture pack | Enhanced and staged mods active in all final runs; replacement hits recorded | No texture bypass in final validation |
| Tests | 593 MeshClipContract assertions pass | Pixel ownership A/B and inspected captures provide compositing evidence |
| Handoff | Final executable staged individually in protected V8_2_LOOSE; tracker/progress updated | No commit/push requested for this turn |

Evidence manifest: `artifacts/canyonlands-bridge-20260914/visual-verification.json`.
Source analysis and historical investigation: `notes/canyonlands_road_ordering_re.md`.
The previous missing-reproduction blocker was removed by the September14
screenshot; it is no longer current.
