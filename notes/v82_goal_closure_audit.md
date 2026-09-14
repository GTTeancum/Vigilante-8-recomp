# V8:2 regression goal — closure audit

**Closed by user instruction on 2026-09-09: "Call it complete."**
The scoped regression items have been removed from the open-work tracker.
The assessment below records verification limits at closure; it does not
override the user's acceptance or keep those items active.

Technical assessment at closure: full verification was not established.

Current runnable executable: `V8_2_LOOSE/Vigilante82PC.exe`, SHA256
`3C91E2BBF83BB7016795D074AF0356D14DE218916346AC5D2C8DD3256D2650B5`.
Its only change from the previously verified AD900 gameplay build is opt-in
clock logging, disabled by default. No commit or push performed.

Evidence paths below are relative to `artifacts/user-level-regressions-20260908/`.

| Requirement | Evidence inspected | Assessment |
| --- | --- | --- |
| Utah sand covering road | Native pixel ownership in `utah-gap-depth-confirm`; current town capture in `utah-clock-trace`; seven chronological captures in `utah-final-sustained` | Original depth-order defect corrected. Sampled beige shoulder patches are written by the road itself; subsequent sand fails depth. Observed routes pass; no claim of exhaustive map coverage. |
| Houston special #14 fires and mounted gun tracks | `houston-final-aim-fire`: native exit0, six individually reviewed captures,659 aim samples spanning yaw-944..943, ammo99 to98 | Verified on AD900 with the same gameplay implementation as current build. This is mounted aiming/firing proof, not complete original-game projectile parity. |
| Dreamland shoreline cutoff | User accepted fix; native fixed-position before/after captures and water-pixel ownership | Corrected and preserved. |
| Dreamland vehicle tint | Archived12-frame comparison and current passive sequence; per-mesh unit conversion and native depth ownership | Corrected: water no longer overlays opaque tires and exhaust. |
| Intermittent water flicker | All48 consecutive frames of `dreamland-final-water-sequence`, all12 passive-repeat frames reviewed individually; extended route stills | Not reproduced in those fixtures. Short temporal samples cannot close the intermittent report. |
| Houston wheels becoming a few polygons | Seven extended-route captures per level; current shoreline capture; ticks300–320 packet trace in `dreamland-wheel-packet-provenance-corrected` | Not reproduced. All6237 player-owned packets in that trace have exact vertex provenance. No justified additional wheel fix established. |
| Sustained framerate collapse | Preserved original Utah log shows one-tick state with doubled terrain work/resolves. Current extended Utah and Dreamland runs show no lasting decline. Controlled callback-order comparison and clock traces inspected | Original failure exists in the old log, but its trigger and a causal fix remain unproven. Do not describe it as resolved. |
| Preserve carousel audio | Accepted audio implementation retained; no subsequent audio edits in these diagnostic turns | Preserved. Earlier audio tests remain applicable; no new human two-player acceptance claimed. |
| Local deployment and fidelity | Individual executable staged; existing checkout retained;560 current rendering contract assertions pass | Satisfied for work performed. No model, texture, geometry, simulation-rate, or resolution reduction. |
| Honest visual proof | PNGs listed below individually inspected; original failed gate reports retained | Supplied with scope limits. |

## Native visual evidence

- Current Dreamland shoreline and tires:
  `dreamland-wheel-packet-provenance-corrected/recompone_present_gameplay_0601_1280x720_fxaa.png`.
- Current Utah town road:
  `utah-clock-trace/recompone_present_gameplay_0601_1280x720_fxaa.png`.
  A black shape entering the top edge remains unclassified; do not call this
  image proof of a completely defect-free scene.
- Houston aim/fire sequence: `houston-final-aim-fire/` captures0340,0350,0420,
  0500,0580,0660. See investigation notes for exact trace interpretation.

## Failed checks retained

- Extended Dreamland harness entered a second match after defeat and terminated
  it. That run is performance/visual evidence, not a clean lifecycle pass.
- Multi-image runs intentionally fail the existing exactly-one-image check.
- Packet/clock instrumentation runs are not performance acceptance. The
  before-order comparison's slow tail includes scripted defeat and has a
  different workload signature from the original collapse; its red gate was
  not rewritten as a pass.
- The first packet-provenance attempt used the wrong trace-range setting and
  was stopped. Only the corrected run supports the packet conclusions.

## Missing prerequisite for the remaining fixes

A current-build failing state or repeatable trigger for the intermittent water
flicker/wheel fragmentation is absent. The old Utah slowdown log lacks the
input/state sequence needed to reproduce its transition, and the current
moving and held fixtures have not triggered it. Existing screenshots do not
show the wheel-fragmentation event itself.

The remaining issues need a reproducible failing state, or a current-build
recording/log identifying the transition and route. Further identical held
tests or speculative renderer/timing changes are not justified by the evidence.
These were the outstanding verification limits when the user chose to close
the goal. Preserve them as evidence history, not active to-do entries.
