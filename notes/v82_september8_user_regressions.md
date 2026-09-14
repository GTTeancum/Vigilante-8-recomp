# September 8 user gameplay regressions — investigation

User accepted the carousel audio fix, but reported Houston's imported special
still does not track (#14), Dreamland water flicker and intermittent wheel
geometry loss, Utah road occlusion, and a sustained frame-rate collapse.
These issues are not closed by the earlier visual smoke tests.

Evidence root: `artifacts/user-level-regressions-20260908/`.
`user-runtime.log` and `utah-user.png` preserve the original report, from
executable DFC578A358D20165178A87CE40332EAEAB13425BE3BC2649E61BA27C402344D9.
The log is a single Canyonlands session; it does not reproduce Dreamland.

## Confirmed implementation defects

- Native `8003319C` loads special DLLs only for the 18-bit retail mask.
  Imported identities do not request their package's retail behavior dependency.
  Houston's native test logged `native-type=3 descriptor=0x00000000`, falling
  back to generic behavior. `SpecialResourceMask` adds declared dependencies
  from active selections and the six native participants before this loader.
  Five focused resource-mask checks pass. The `utah-road-special` native run
  now loads SHARED_TOWTRUCK and arms relocated callback 80775874. This is
  resource/callback evidence, not yet proof of projectile homing.
- GPU quads arrive as a triangle strip. HLE submitted 0,1,2 then 1,2,3,
  reversing the facing of the second half. World backface culling could discard
  half the quad. HLE now submits 0,1,2 then 2,1,3. The software GPU is unchanged.
  Eight real GP0 decoder tests cover all four quad packet forms and mirrored
  winding; the complete mesh contract reports 534 assertions passing.
- Panorama packets wrote depth. With the existing TRUE_DEPTH diagnostic,
  that depth blocked distant terrain. Sky packets now never write depth.
  This does not enable TRUE_DEPTH by default.

## Native visual evidence and rejected hypotheses

- `utah-before`: DFC578, held Houston at spawn, missing road coverage.
- `utah-true-depth`: DFC578, TRUE_DEPTH diagnostic. Rejected: distant terrain
  disappears because of sky depth writes.
- `utah-sky-depth`: B6256FF2, sky-depth exclusion plus special dependency fix,
  TRUE_DEPTH diagnostic. Distant terrain restored; road not cleared.
- `utah-road-special`: B6256FF2, held at an earlier position from the user log,
  special kind 7 armed. View faces buildings, not the user's screenshot angle.
- `dreamland-before`: B6256FF2, moving Houston, 12 consecutive presentations
  at gameplay poll 901 inspected individually, in order. Bank/water visible;
  no flicker or wheel collapse reproduced in this short sequence. A late
  sampling-profiler capture includes transition/wait time and is not evidence
  for the sustained slowdown. Do not tune from its aggregate percentages.
- `utah-quad`: 01631874, quad split fixed, default depth, held spawn. Road
  coverage remains unresolved.
- `utah-last-position`: 01631874, default depth, held at the last logged user
  position (61428137,2933611,81913599). Garage visible, road fragment at right.
- `utah-road-depth-trace`: same position with TRUE_DEPTH; no road improvement.
  Trace tick syntax was invalid (`300:300`), so this run has no owner trace.
- `utah-road-cull-trace`: diagnostic in progress, world cull disabled, correct
  trace range `300-301`, points `330,135;345,145`.

Visual runs use native process-local input/capture. Some are unthrottled to
shorten diagnosis. Their aggregate timing/traversal/image-count gate failures
are retained; they are not release acceptance or FPS improvement claims.

## Outstanding

Reproduce and resolve Utah road ownership/occlusion, prove Houston homing in
an actual firing test, reproduce Dreamland water/wheel failures, and isolate
the sustained slowdown in gameplay. Preserve audio and attachment fixes.
Final executable validation and cleanup are still pending. No commit/push
has been made for this work or the preceding audio fix.

## Confirmed Utah road cause and correction

The original software raster pixel probe at (20,210), native tick 300, writes
cached route packet 0x006C7C8C (TPage 08E, CLUT 4894). Enhanced did not submit
that packet. Disabling the native terrain-detail override did not fix it.
Disabling NO_RETAIL_CULL did restore the road. The workaround forced every
non-terrain NCLIP result positive, including cached route/effect emitters
which have their own sign conventions and no registered replacement mesh cull.
It now applies only while V82Compat.ObjectRenderScopeDepth > 0.

The new real-GTE test fails before the fix on an unscoped negative face,
passes after the fix, preserves the owned mesh bypass, and verifies restoration
on scope exit. MeshClipContract now passes 537 assertions.
The temporary NATIVE_TERRAIN_DETAIL diagnostic was removed.

Staged candidate: 670E7AB239FB017F39B0FBFE3C25A4E266B2926C5FFFF252898BBBC17D64D35B.
Native held-position capture `utah-road-fixed.png` shows the continuous road.
All seven `utah-scoped-cull-special` captures were reviewed individually in
chronological order. The road remains present. This is not an FPS acceptance.

Special DLL is now loaded (descriptor 80774AC0, callback 807753F4), but the
held firing fixture has no valid target and ammunition remains 99. Native
Towtruck event 12 rejects a missing player+F0 target or target without flag
4000. Do not claim homing is verified. A targeted native acquisition test is
still required.

## September 9 resumption: remaining holes isolated

The earlier `utah-road-fixed.png` claim was too broad: NCLIP scope repaired missing submissions, but sand still crossed submitted road sections. At held user position, yaw 1024, native pixel (140,145), enhanced road camera Z is 1376.1056 while its OT-derived raster Z was 1575.9758. Sand Z is 1391.9288. Thus physically farther sand passed depth and overwrote the road.

`utah-gap-depth-confirm` proves using exact source depth retains the road: road writes depth 0.999287426 and later sand leaves the same depth and RGBA intact. Source now defaults exact coherent camera depth on, retaining OT fallback for reconstructed/uncorrelated vertices and native painter order. 543 mesh assertions pass, including six source/fallback/depth-order cases. New candidate still needs staging and visual regression checks.

Houston model conversion now packs independent roots first and inserts identity rendered children expected by the special callback. All 15 conversion tests and full package round-trip pass. The child-wrapper package is staged (CUSTOM.EXP 212C341357040E982D308A34717B5448D67E7A5002A8578B100C9AC117467C41); firing/tracking remains unverified. Prior long Dreamland run crashed in special projectile creation around tick2342 before the requested later captures; do not treat it as a completed long test or solved slowdown.

## Native projectile and long traversal progress

The focused fixture initially pressed R2 (machine gun), not L2 (selected attachment). Corrected artifact helper `run_focused_special.py` to L2. `houston-child-l2-fire` on staged 91D572BE1F437DB01FBF99648DCE80627D19B53DEE791F89CAE4001512936DD7 logs native special ammo99->98. Capture340 is pre-fire and was inspected; it is not evidence of homing. The nearby-enemy/held fixture still overflows recursively in native render callback80014BE4 when forced defeat starts. This is separate from the repaired projectile child pointer failure. A proof-only hold guard now stops after defeat is injected; not yet built or verified.

`dreamland-depth-long` passes tick5751 with native special firing twice and no projectile crash. First12 captures were individually inspected sequentially; visible wheels retain shape, but view does not sufficiently cover water to close flicker. Recent host timing windows are steady around60FPS (1280x720 presentation); this is not a reproduction or closure of the user slowdown. Five special resource contract assertions pass against the staged package. No commit/push.

## User correction: mounted gun auto-tracking is the actual #14 symptom

User explicitly clarified that the gun MODEL must auto-track the selected enemy. Do not substitute projectile firing or homing evidence for that requirement. Found original PS1 SLUS_005.10 special callback table at8005ECB0, entry7=80035CF8. Tick block80035D40..80035E24 obtains composed weapon world matrix, inverse-rotates target physics-position delta, computes signed12-bit yaw and negative pitch with native ratan2, clamps pitch[-128,256], and updates the mounted first child's YXZ matrix. The imported sequel callback lacks this aiming step.

Added registry capability `special_turret_aim` (flag1<<22) enabled by conversion metadata, with no runtime vehicle/name/weapon ID branch. Runtime uses source-equivalent V8:2 native matrix, inverse-GTE-vector, ratan2 and matrix rebuild routines before native selected-weapon update. Ownership is established by selected weapon bank matching the vehicle bank. Boolean metadata round-trips through project/registry/package; 15 conversion tests pass. `turret-right` native no-fire pose proof is running. Staged turret candidate and registry146251303D4A1B83F4E059653347C07E41434F7299E650145B95EA2CA762C75F. Gun rotation is not verified yet.

The previous projectile target-link diagnostic was removed from current source because it did not verify the user's symptom. Important source discrepancy for follow-up: original V8 callback initializes only integrated gun ammo/flags, whereas native Towtruck callback also creates a visible ready object from independent root2. That mapping was pre-existing metadata, but the newly root-packed package exposes an extra white/black sphere in the proof. Do not call the full special behavior canonically equivalent without auditing that ready-object distinction.

## Resumed gun visual verification and correction

The extreme side-target native capture `turret-side-proof/proof.png` visibly shows the mounted gun rotated sideways (native yaw approximately 941/4096 turns). This resolves the ambiguity in the earlier rear view: the white/black sphere is part of the actual gun silhouette. The prior suggestion that this silhouette proved an extra idle object was incorrect. Removed the speculative ready-child release code; retained only original fixed-point yaw/pitch aiming. Aim/resource contracts pass 17 checks. Published and staged aim-only D36C5E5C1AB746720D27A1ABB6C96ED7E0713E190551BB869D68B4257B0C0BE0.

The attempted opposite-side fixture still selected the right-side enemy because the fixture places two enemies symmetrically; changing offset sign swaps their identities. Do not claim left/right pair acceptance. `turret-opposite-fire` shows special ammo 99->98 and a side-aimed gun; forced-defeat render-callback stack overflow remains. Its whole-run gate also recorded two process IDs and is not acceptance evidence. Latest aim-only build needs its own native visual/firing verification.

All twelve `dreamland-water-depth` captures have now been inspected individually in order. They reproduce the user's circled hard rectangular shoreline band; earlier limited visual review did not close this issue. A targeted depth/triangle trace of the same route is running under `dreamland-water-edge-trace`. Water flicker, ordinary wheel integrity and user sustained slowdown remain open.

## Accepted water coverage and vehicle depth units

The user visually accepted the shoreline repair on September 9. The old textured wave patch ended at camera Z12.288 world units; the circled bank pixel intersects the water plane around17 units away. Plane/frustum intersection now covers the visible water plane through the existing65.536-unit far limit. Original wave spacing, amplitude, timing and UV formulas remain unchanged. Conservative whole-cell frustum rejection reduces that held view from47940 to22704 water vertices without removing visible cells. No Dreamcast assets were introduced.

The subsequent blue-green vehicle tint was water overdraw, not a palette problem. In `dreamland-wheel-tint-trace`, native pixel126,203 is black after the tire draws, then becomesRGBA41,66,90 after WaterSurface. The tire's raw camera Z2327.5286 was in a different coordinate scale from water. Both native mesh entry paths80021F70 and80021FA8 shift16.16 camera translation by16-mesh[1]. Enhanced terrain/water use256 units/world, so enhanced vertex XYZ and perspective W must be multiplied by2^(8-mesh[1]). Native SZ, SXY and OT remain untouched. Both entry wrappers restore the previous metadata scale in finally blocks.

`dreamland-mesh-depth-fixed/proof.png` and its native pixel trace verify the correction: both sampled tires remainRGBA0,0,0 after water, and the sampled exhaust remainsRGBA165,165,165. Tire depth is now0.998299599, ahead of the water's0.999109745. The accepted bank water remains visible. Staged executableF5C2FB0F38116AB5C628D2580E27F886FC7F8A2E33B33B6C8C174E56697C03D6;556 mesh assertions pass. Native process exits cleanly; held-position route gates intentionally do not qualify as traversal acceptance. All12 earlier `water-wheels-visible-proof` frames were individually inspected in order; that short burst has stable wheel silhouettes but persistent pre-fix tint.

The crate's provenance remains unverified. Dreamland's tree root377 contains multiple destruction states and effect/sound records; this alone does not establish that the circled crate is an intended reward. No crate behavior was changed. Utah regression capture is pending; the first360-frame attempt reached the known forced-defeat callback recursion before capture601 and is not visual evidence.

Final Utah visual check: utah-mesh-depth-visual/proof.png was captured and inspected on F5C2FB0F at preserved user position. The foreground road and town bend remain continuous, with no sampled sand holes; wheels and attachment remain intact. The later forced-defeat sequence still stack-overflows in80014BE4, so the run does not pass lifecycle acceptance. Settings restored to AutoWaterski(2), no game process remains, stale root v8_latest.log archived individually and removed. Crate provenance, intermittent wheel geometry/flicker and long-session slowdown remain open. No commit/push.

## Display callback lifecycle repair
Native callback trace at forced-defeat tick360 showed80014CCC installing80014BE4 while80014BE4 was still pending from the prior draw. It saved itself atgp+CD8, then recursed when VBlank arrived. The synchronous host DrawSync wait was delivering a second completion before the pending display flip. ServiceDrawSyncWait now drains that pending flip through normal PresentFrame/VBlank before delivering the new completion. It reads the original PsyQ callback table80065460 index4, confirmed by8005551C/80055574/800555E0; no recursion suppression or gameplay identity branch. Tests cover unrelated/no callback, predecessor restoration, and bounded failure.560 mesh/lifecycle assertions pass.
Staged AD900ECAF3E05A9B680E89B4099FD72384198460726104281AFB8999A20ACD76. Matching utah-callback-order-fixed native run exits0 and reports zero self-predecessor installations, unlike utah-callback-order-trace. Held route/performance gates remain failures, so this is lifecycle evidence only. Final Houston side-aim/firing proof started on this build.

Final-build Houston verification: houston-final-aim-fire on AD900ECAF completed with native exit0. All six captures0340,0350,0420,0500,0580,0660 were individually inspected in sequence. The mounted gun is visibly rotated sideways; native aim trace has659 samples with yaw-944..943, and the native selected-special path logs kind7 ammo99->98. Shot340 precedes the ammo decrement seen350. This verifies mounted aiming and firing on the current build; it does not claim full original-special projectile behavior equivalence or human signoff. Held-route and exactly-one-image gates intentionally fail for this multi-capture proof; sustained-performance check passes in this short run. Dreamland final sustained traversal session48197 is running7200 native ticks with captures601/2401/4801/7201/9601/12001/14001 to investigate outstanding intermittent water/wheel/slowdown reports.

## Current-build extended Dreamland review (2026-09-09)

`dreamland-final-sustained/acceptance.json` records staged AD900ECAF unchanged throughout the run. All seven native captures (polls 601, 2401, 4801, 7201, 9601, 12001, 14001) were converted losslessly from PPM to PNG and individually inspected in chronological order. Captures cover shore collisions, deployed water attachments, submerged vehicle, return to land and combat. No obvious wheel fragmentation in the visible land captures; the underwater captures cannot establish wheel integrity. Spaced stills do not prove absence of water flicker. Continuous water reaches the visible banks; the accepted cutoff correction remains visible.

Performance: 259 accepted gameplay windows, host frames 3661..19200, median59.97 FPS, minimum36.00, head median59.17, tail median60.02. No lasting collapse reproduced during about4.3 minutes of measured gameplay. This is evidence of stability for this route, not closure of the user's intermittent report. Zero edge-pool drops, packet arena high-water61.37%. In preserved user-runtime.log, slow periods correlate with roughly doubled terrain generation, allocations, presentation-source switches and MSAA resolves. Current active windows generally resolve30 times per60 host frames, versus120 during the old slow periods. The pending-display callback correction may affect this cadence; causation is not yet established by a controlled reproduction.

The full gate is RED: after native lethal damage at tick7200, defeated-menu scripted inputs selected another match (second gameplay stage at absolute poll19838), and the harness terminated the process. This was not a native crash: no_fatal passes, stderr contains no fatal exception. Clean-exit, gameplay-count and deterministic-completion checks fail, and exactly-one-image intentionally fails for seven captures. Do not describe this as a clean completed lifecycle test. Earlier focused Utah/Houston lifecycle tests remain the valid clean-exit evidence. Session48197 is terminal; no replacement launched.

Remaining: reproduce intermittent wheel/flicker symptoms with temporal evidence; correlate old workload doubling with callback/render cadence; classify residual Utah spawn shoulder patches before universal road closure. Goal remains active.

## Utah shoulder pixel ownership and slowdown attribution (2026-09-09)

Two current AD900ECAF held-spawn checks: `utah-shoulder-probe` exits0; initial trace tick range used unsupported colon syntax, so it supplies only the visual. Retried `utah-shoulder-depth` with documented range170-170, exits0; held route gates fail, not a crash. Three native points245,185 /249,190 /242,182 map to presentation pixels897,555 /909,570 /888,546 within the suspicious beige shoulder rectangles.

Native road packet0x006DFBEC (texpage0x288E,clut0x4454,UV149..181/16..32) produces the final beige colors. At point249,190 road cameraZ765.492 writes depth0.998707712; later terrain cell952,1260 texture225 cameraZ768.83716 fails depth and retains the same RGBA156,132,99. Other two points show the same ordering. These sampled patches are therefore road-texture output, not sand overwriting the road. Do not remove source road shoulder artwork based on color alone. Exact decoded route DDS images saved beside trace: all three show road shoulder bands with irregular outside edges. Their existence does not establish exact texture-coordinate fidelity; do not call all possible road problems closed based on three points.

The preserved `user-runtime.log` slowdown is from Utah (Dispatcher loaded LEVELS_V8_CANYNLND at line559), not Dreamland. During its slower windows the native scheduler changes from ticks2 to ticks1 while per-host-frame terrain workload roughly doubles. Current Dreamland stayed ticks2 throughout sampled heartbeats, but cross-level comparison cannot prove a fix. Next sustained reproduction targets Utah on the current build.

## Render cadence comparison in progress

Read-only `analyze_render_cadence.py` produces `utah-user-cadence.json` and `utah-current-cadence.json`. Old Utah windows grouped by the most recent sparse native heartbeat: ticks1 has20 windows median29.00 FPS,270313 terrain triangles,120 MSAA resolves; ticks2 has57 windows median59.83 FPS,134292 triangles,60 resolves. Transition windows may straddle a heartbeat; this is correlation, not causal proof. Native loop80013B2C increments gp28 once per physics tick; scheduler receives the nonzero count only on the last tick of the scene. One tick per slower host frame therefore also implies slower simulation, not merely an FPS label change. Do not improve an FPS statistic by silently reducing simulation or image detail.

Live Utah session76421/PID26584 at tick2420: first71 active windows all most-recent ticks2, median52.74 FPS. Native screenshots601,2401,4801 inspected individually in order: tires visibly retain their circular sidewall geometry at tilted positions; available road sections remain visible. Combat obscures parts of the road in the latter two images. Remaining four captures not yet produced/reviewed. No closure claim while run is active.

Checked projection metadata lifecycle: Runtime.PresentFrame rotates GTE correlation after presentation, but Gte.BeginFrame explicitly retains eight bounded generations for delayed ordering tables. This alone does not prove stale/missing wheel projection and no speculative cache edit was made.

## Utah extended test completed

`utah-final-sustained` finished native exit0 with deterministic defeat and shell return. All seven captures601/2401/4801/7201/9601/12001/14001 individually reviewed in chronological order. No obvious wheel breakup in the exposed tilted/car-side views; roads remain present where visible, with the previously traced beige authored shoulder output. Combat and slopes obscure some road coverage.

255 gameplay timing windows: median55.01 FPS, minimum31.30, head56.62, tail59.16, no persistent decline. All native lifecycle, route, packet headroom, fidelity and sustained-performance checks pass. The report's overall false status is solely exactly_one_gameplay_image, because this deliberate diagnostic collected seven. No original one-tick/doubled-workload collapse reproduced. This does not prove an intermittent problem impossible.

## Consecutive current-build water evidence

`dreamland-final-water-sequence`: native exit0,48 full-size PNGs losslessly converted from PPM, every frame000..047 individually inspected in order. Water remains continuous without alternating missing patches; rear tires in000..003 and front-view tire silhouettes004..047 remain intact and opaque. One camera cut at004 switches the complete view; it is not isolated water flicker. `dreamland-passive-water-sequence` repeats with an empty-mask checkpoint (logged p1=0,p2=0 atpoll601), native exit0,12 frames all individually inspected. Same cut at004, so P2 capture input is ruled out. Both reports fail only the exactly-one-image check. Short held sequences are temporal evidence for this position, not universal intermittent-failure closure.

## Archived mesh-depth temporal comparison completed

`dreamland-before-mesh-depth-sequence` finished; acceptance fails only exactly_one_gameplay_image for the intentional 12-frame burst. All twelve PNGs000..011 individually reviewed sequentially, in groups of three. Rear view000..003 shows blue/green water overlay on tires, exhaust and mounted weapon. Front view004..011 shows the same overlay on tires, weapon and grille. Tire silhouettes stay present throughout; this reproduces tint, not the reported loss of most wheel polygons. Current passive-sequence000 and004 were re-opened for matching-view comparison: black opaque tires and untinted rear exhaust, with accepted continuous water. Ambient combat/HUD differs between runs, so they are matching-position comparisons, not byte-identical simulations. Staged AD900 SHA reverified unchanged.

Next causal investigation: generated arena loop80013AA0 subtracts physics clock gp28 from gp-5370 to choose ticks; gpD0C nonzero forces two via VSync wait, while gp40 bit2000 can force one in the other branch. Scheduler compatibility additionally calls Runtime.PresentFrame at the final simulation pass. Follow the native clock writers and presentation/callback sequence before changing cadence; no timing or fidelity patch made on correlation alone.

## Native clock source traced

Generated startup sets GP=0x8006A7F0, so gp-0x5370 is exactly0x80065480, the PsyQ VBlank counter. Native80055574 increments that counter before iterating eight callbacks at80065460. Arena80013AA0 computes due physics ticks as this counter minus gp28. Runtime.PresentFrame calls one DispatchIrq(0) after presenting/throttling; it does not account for additional elapsed VBlanks when the host frame is slow. LibEtc._vcount is a separate counter and is not the arena's clock source.

Consequently the observed old one-tick/29FPS state is compatible with the host delivering only29 native ticks per second; changing LibEtc._vcount alone would not repair it. The pending-display drain adds actual native IRQ opportunities and may prevent a transition into that cadence, but its causal role remains unproven. A future diagnostic should record native counter65480, physics gp28, replay gate gpD0C, flag gp40 bit2000, and presentation callers around the transition. Do not blindly force two physics ticks or add unbounded catch-up: those would change original timing/behavior without establishing the correct seam.

## Opt-in clock diagnostic staged and verified

Added RECOMPONE_V82_TRACE_CLOCK=1 read-only enter/leave records around Runtime.PresentFrame. Records native VBlank65480, physicsgp28, replaygpD0C, flagsgp40, display callback and completion flag, caller and monotonic timestamp. Disabled by default; no timing, graphics, physics or accepted audio behavior changed. Build clock-trace-candidate compiled and560 contract assertions passed. Individual exe staged in place: SHA2563C91E2BBF83BB7016795D074AF0356D14DE218916346AC5D2C8DD3256D2650B5.

Focused utah-clock-trace uses preserved town position61428137,2933611,81913599/yaw1024, Houston71, held600-tick fixture, native1280x720 capture601. Native exit0; map gatePASS. Trace overhead means reported FPS is not performance acceptance. All2201 enter/leave pairs balanced; each increments VBlank by1 and changes physics by0 within presentation. Callers: scheduler80013B64=1101, displaywait800146B4=1099, final800147AC=1. Replay0 and flags00100040 throughout; no forced-one-tick flag. Flip restores80014BE4->80015104 on1101 presents. This verifies a steady two-presentation cadence for the active scene and native teardown. No one-tick collapse reproduced. Parser and full observations saved in clock-summary.json beside trace, via analyze_native_clock.py.

Native capture individually reviewed: continuous town road and opaque intact rear tires. Large black shape entering top edge is present in this combat still; its provenance is unclassified, and this screenshot does not establish absence of all visual issues. No game process remains after completion. Goal remains active; no commit/push.

## Controlled callback-order comparison

Inspected native alternate draw paths: gp2C selects viewport layout; both paths call the same80014D94 completion seam, followed by the synchronous LibGpu.DrawSync stub. No evidence that viewport branching caused the old one-tick slowdown.

Built diagnostic84D431EC095065DAF72D375FE1B44B6AAB3EF3D29BEB6013F7BA31EC417F4265 by omitting only DrainPendingDisplayFlip invocation. Source restored byte-for-byte in finally immediately after compilation. Staged comparison for same held Utah600 fixture, tracing/capture601. Native exited0. Overall gate fails sustained_performance/all_map_contracts; trace logging invalidates performance acceptance, so this is not a reproduced uninstrumented collapse.

All2201 clock pairs match the fixed-build aggregate exactly: scheduler1101, displaywait1099, finalwait1, all nativeVBlank+1, physicsunchanged withinpresent, replay0/flags00100040. Thus current evidence does NOT support attributing stable gameplay cadence to the callback-order fix; retain that fix for separately reproduced recursive defeat failure, not as proven slowdown repair. No one-tick state triggered in either held run. Combat/HUD differ between runs, so only controlled code difference and identical fixture are asserted, not identical full simulation.

Comparison capture individually reviewed: continuous road, intact rear tires, black shape at top edge also present with different orientation/size. Its provenance remains unclassified. Verified3C91E2BB executable restored automatically after native process exit; no game process remains. Goal active. Next investigation must target another causal mechanism or a reproduction matching the original moving-session transition; do not repeat this held callback-order comparison.

## Comparison performance failure classified

Read the actual gate rather than attributing its red result to logging alone. performance_summary excludes only the first/last terrain-bearing windows; its tail includes roughly500 scene frames after scripted lethal damage at600. Supplemental read-only compare_clock_workloads.py splits on the actual defeat marker and excludes its straddling timing window. Original acceptance reports are unchanged.

Before defeat: current20windows median59.995FPS, pre-order20windows58.735FPS; both terrain133740 triangles and60 MSAA resolves/window. After defeat: current16windows median50.7FPS, pre-order41.77FPS; both terrain132705 and60resolves/window. Flushes increase from4940/5262.5 before to11380/10838.5 after. Clock counts remain identical and physics ticks2. Thus the tail failure occurs during a changed defeat workload, without the original one-tick/120-resolve/doubled-terrain signature. Instrumentation and differing combat state prevent a causal speed comparison between these two binaries. Do not hide the red gate, declare it logging-only, or use this as closure of the original user slowdown.

Native GTE flagErrors counter increments when software reads FLAG with bit31 set; it counts native GTE saturation/overflow status, not exceptions or independently proven memory corruption. The pre-order trace's570 flagErrors cannot by itself prove corrupt wheel data. No renderer/timing patch or new runtime launch made from these ambiguous counters.

## Wheel packet provenance investigation

Reviewed memory/CPU provenance: byte/halfword/word stores invalidate exact metadata, CPU arithmetic setters clear register metadata, supported loads/stores propagate it, and packet consumption checks exact address/value. Found a possible mixed-primitive concern: PopulateEnhancedViewSpace reconstructs missing endpoints despite the nearby exact-only comment; DepthOf uses exact ViewZ for direct endpoints and OT fallback for reconstructed ones. This remains a hypothesis, not an established wheel failure.

First diagnostic dreamland-wheel-packet-provenance mistakenly used terrain-cell tick-range setting instead of RECOMPONE_TRACE_GAMEPLAY_TICKS. Stopped explicitly targeted native PID25344 once error confirmed; its red/incomplete report is intentional and provides no packet proof. Corrected run dreamland-wheel-packet-provenance-corrected uses packet geometry trace ticks300-320, same preserved bank, player71,420ticks,capture601,1280x720,unthrottled. Native exits0, no fatal; overall gate red on instrumented sustained-performance. No throughput claim from107352 logged packets.

Read-only analyze_packet_provenance.py categorizes every packet in the21-tick window:106428 all-exact;924 all-inexact (882water-base and42sky); zero mixed. All6237 packets owned by the live player0x806EC4D0 are all-exact. Thus this fixture does not exercise mixed-depth reconstruction for wheels and supplies no basis to patch it as the user symptom. Exact PNG converted and individually reviewed: intact opaque tires, neutral rear attachments, continuous shoreline. Staged3C91E2BB SHA unchanged. Goal still active; do not repeat this same held provenance fixture without new evidence.
