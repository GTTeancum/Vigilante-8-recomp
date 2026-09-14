# Canyonlands road/terrain ordering — September 13, 2026

## Recovered behavior

The original V8 route builder (`analysis/dll/LOAD/decomp/80102bd4.c`)
subdivides the authored curve, samples terrain height at the two strip edges,
and stores those heights relative to the strip origin. It does not lift the
entire road above terrain. A strip surface is therefore not necessarily the
same triangulation as the terrain underneath it.

The original V8 road emitter (`analysis/SLUS_005.10/decomp/80040e5c.c`)
inserts XRTP packets relative to ordering-table base +0x40. Its scratchpad
wrapper is 80040e38. Original V8 terrain setup in
`analysis/SLUS_005.10/decomp/800290d8.c:84` sets scratchpad OT base to
`iRam0000060c + 0x100`, independently confirming the same relative priority
in the original game. The matching V8:2 road functions are 8004F828 and 8004F804:
descriptor +8, vertices +0x20, descriptor +0x2c flags, and the same OT +0x40.
The V8:2 terrain setup in 8001C158 installs OT +0x100 at scratchpad +0x80
(instructions around 8001C820). Native depth bucketing spans eight SZ units
per four-byte OT slot. The relative road priority is therefore 48 slots,
equivalent to 384 SZ units. These constants come from code, not visual tuning.

## Withdrawn depth-priority experiment (historical)

Exact camera depth repaired the older coarse-depth error but removed this
relative painter priority. Tag writes inside the native XRTP emitter and carry
that provenance to rasterization. For coherent exact road vertices, preserve
the recovered road-versus-terrain priority in raster depth. Projection,
perspective UVs, geometry, collision and converted assets are unchanged.
OT fallback already carries native ordering and receives no additional shift.
Untagged primitives keep their current behavior. Packet reuse and scene reset
clear provenance.

This is an adaptation of native ordering to a depth buffer, not a claim that
per-vertex depth is bit-identical to the PS1's polygon-level ordering table.
No arena or vehicle identifier selects the correction.

## Historical validation of the withdrawn experiment

Artifacts: `artifacts/canyonlands-source-re-20260913/`.
Build succeeds; MeshClipContract passes 567 assertions, including native
priority, unrelated geometry, fallback handling and packet reuse.
Initial Quest gameplay captures exist for the previous Enhanced build,
candidate and software renderer. Combat differs between those runs, so they
are not pixel-aligned causal proof. Fixed-camera comparison is in progress.
The software oracle is the V8:2 recompiled native packet renderer using the
converted original map, not execution of the untouched original V8 binary.

## Shared tiling regression (user correction)

User circled terrain grid artifacts and road shoulder blocks, and subsequently
reported missing bridge polygons while driving. All remain tracked in #5.
Do not classify the shoulder blocks as correct merely because a road packet
owns their pixels.

Controlled same-spawn diagnostics: `survey-no-pack` removes the obvious
terrain grid, `survey-nearest` (native textures/no smoothing) does likewise,
and `survey-mip-off` keeps HD replacements but removes terrain mipmapping and
also removes the grid. The issue is in replacement terrain filtering.

The packed mip sampler used a base-level half-texel inset at every LOD, allowing
coarser bilinear samples to touch another cell. It also ignored the existing
anisotropic-filtering setting, unlike native-texture filtering, so grazing
terrain blurred into individual tile averages. The shader now clamps each
sample at its actual mip level and integrates the long footprint using the
configured anisotropy; mipmaps and HD textures remain enabled.

`survey-aniso-fixed` and `survey-native-aniso` exit 0 on staged SHA256
AE0094CC7E802D1D1C7B74FCF9B92227C5BE3FCDE42567E15202EDE2E02F1E0A.
Their spawn captures show the terrain grid removed/reduced on Canyonlands and
Route 66. Shoulder blocks remain. This is NOT closure of the full road issue.

The initial `survey-bridge` synthetic height placed the camera below terrain;
those captures are invalid for bridge signoff. The baseline `before-drive`
actually drives under the bridge and up a slope, rather than across its deck.
Do not present either as a successful crossing test.

## Road-priority experiment withdrawn

The recovered OT offsets are valid source evidence, but shifting road raster
depth did not resolve the reported shoulder blocks. The experimental depth
shift, packet provenance hooks and its tests were removed. Keep the RE finding
for investigation; do not ship or describe that candidate as the road fix.

A bridge fixture at native position (61931520,2800000,79757312), yaw1024,
shows the deck and both trusses while moving along the span. The four held/
translated captures do not show missing faces, but are not a physics-driven
crossing. A process-local release-frame option now allows the initial pose to
be set, then ordinary vehicle physics and synthetic input to take over.

## Correct crossing identified after user correction

The user means the other bridge at the opposite end of Canyonlands, not the
steel arch. Native EXP identifies `overpass_1` (object id48, bank0 slot450) at
(62654056,4192256,85003256); the arch is `bridge_1` slot53 at
(62455808,4192256,79757312). Overpass root render group112 has bounds
X[-903,893], Y[-452,0], Z[-687,694], scale shift7 and child slot451.
The first overpass fixture at Y3400000 is below ground; Y2100000 is well
above it. Neither is crossing evidence. A physics-released Y2800000 fixture
is underway. Current staged SHA256 is
7B35DA4B8A658FABDBEEB187E65069C40A532CE3003D6D100137F95424103725.

The early physics run captures the correct overpass at gameplay120/180.
The deck is visible at both times; by180 the yaw1024 vehicle hits the side
rail, so this is not a full traversal signoff. Group112 deck center vertices
advance roughly (150,13,-69): yaw atan2(150,-69) in4096-angle units is1305.
An aligned fixture now starts at (60800000,2700000,85961721), yaw1305 and
captures gameplay60..280 every20 frames, releasing physics at60.
Harness: survey.py overpass-drive-aligned. Output directory: survey-overpass-drive-aligned.

Aligned crossing fixture correction: the earlier widened X60800000 start was
on a cliff, not on the bridge. Retain its captures only as invalid fixture
diagnostics. The confirmed lane pose is (62391912,2800000,85210000), yaw1305.
`survey-overpass-drive-aligned-lane` exits0 on staged7B35DA4B...; captures
120/160/200/240 show landing and travel on the deck with both railings and
no missing deck region. This is approach evidence only. The `-long` mode
extends captures320..740 and exit4550 to examine the remaining traversal.

`survey-overpass-drive-aligned-lane-long` exited0. Individually inspected
200,240,260,280 (on-deck),320 (exit to road),380,440,500 (beyond bridge).
No missing deck region reproduced during this down-slope crossing. This
does not close an intermittent report or prove the reverse direction.
Reverse uphill fixture now running from (62850000,2800000,84990000),
yaw3353, with process-local physics released at frame60.

Uphill reverse fixture exited0. Inspected200 and280 on-deck and380 beyond
the exit: no missing deck region. The intermittent report remains open;
these two crossing directions do not establish a causal bridge fix.

Next diagnostic: enhanced GlShaders rounded native UVs with ceil on negative
X/Y screen derivatives, whereas GpuRaster rasterizes nonnegative weighted
UVs using integer division (truncation). Testing consistent floor sampling
for the keyed native texel; replacement texture interpolation is unchanged.
This candidate is not yet validated or claimed as the road-edge correction.

Floor-sampling test `survey-texel-floor` exited0, but the inspected0601
spawn capture retains the large shoulder blocks. The experimental rounding
change was removed; it is not established as a cause of this report.
Restoration build is underway; retain only the terrain mip/aniso fix.

Source comparison caution: original V8 disc CANYNLND.EXP was extracted
directly through PsxIso and matches PS1 game/TERRAIN byte-for-byte. Its
RSEG/JUNC and XRTP differ from staged Second Offense Canyonlands, but the
staged file matches V8_2_WORK/disc/LEVELS/V8/CANYNLND.EXP exactly:
CE5E1CA95C7DA8E6547683A62FB424C38A0BBC7558CDF64F3538C36AAD29C0AC.
Do not mistake the inter-game asset differences for a new conversion bug.
No level asset was modified. Sampler experiment withdrawn and rebuilt/staged
as64B483695058AB515B761CAA530FDECB8380589C0FFB1E69365512911F0DC2AE.

## Confirmed road blend ordering defect and candidate

`survey-road-blend-trace` tick170 point249,190 proves this sequence:
1. Sky RGBA230,197,156,0 at depth1.
2. Native road packet006D8A3C (misclassified Particle), trueZ764.50757,
   writes RGBA156,132,99,255 and depth0.998706043.
3. Terrain cell952,1260 trueZ768.7209 fails the existing road depth and
   leaves the sky-blended road result. This is not correct road blending.
Native V82 emitter8004F828 checks XRTPflags0x100 at8004F874 and constructs
GT4 opcode0x3E (rather than0x3C); Canyonlands XRTPflags are0x102.

Candidate tags packet writes in8004F804, classifies native roads as
TerrainRoute instead of Particle, buffers those batches and replays after
terrain/world but before water/screen effects, using scene depth testing.
Native blends, STP, vertices and textures are preserved; no map identifiers.
`survey-road-blend-fixed` exits0 on4992E6932CBCC37365EF7F598DDC1694F146A4162ACCDC665ACB1FCF606B6D3D.
Inspected0601: large pale shoulder blocks are gone. Some apparent road/
terrain intersections (including breaks on the yellow edge) need separate
validation; do not close full road-depth report based on blend correction.
564 MeshClipContract assertions pass, including nested road ownership,
RAM aliases and packet reuse. Route66 control run `native-road-blend` underway.
Direct read of Second Offense MODE2/2352 disc also confirms exact staged
Canyonlands EXP hashCE5E1CA9..., not just equality with the extracted copy.

## Terrain-only priority and junction validation (September 14)

Native XRTP OT+0x40 versus terrain OT+0x100 establishes a 384-SZ relative
priority. The current candidate uses stencil eligibility to apply that priority
only against terrain samples, then writes the road's actual depth. Objects and
vehicles retain ordinary scene depth tests; there is no global road depth shift.
Depth/stencil attachments and clears support this in MSAA and non-MSAA targets.

Junction emitter800507DC passes JUNC+0x18 to80021F70 with the common OT base.
The latter adds signed group+2 times four bytes to its ordering-table pointer.
Junction packet provenance therefore carries (64 - signedGroupSortOffset)*8
rather than assuming every junction shares the XRTP priority. Nested scopes
restore the outer priority; packet reuse retires both ownership and priority.

569 MeshClipContract assertions pass. The staged build is
6eda16b88f259a90cb043e8cffa435d1efdbaa400b30ae8d3c3982cd487364cf.
`survey-junction-road-fixed` exits0. Inspected0601: large pale shoulder blocks
remain absent; the yellow edge still has small breaks requiring investigation.
Texture pack is enabled and applied; diagnostic no-pack settings were confined
to their earlier isolated processes. Current bridge and cross-map validation
must finish before any broader signoff. Goal remains open.

Reverse run exited0. Closer inspection of120/160/200/240/280/320/380/440 identifies narrow triangular colour patches along deck near railing (especially240). These are suspect, not a bridge signoff. First probe tick120 shows actual deck packet002142E8 owning sampled pixels after deferred roads, but tick120 corresponds gameplay220 rather than240; corrected tick130 trace is running. Added gated deferred-road triangle tracing so remaining edge samples can distinguish rejected road geometry from texture coverage.

Corrected tick130 bridge probe also exits0: pixels786,660 /801,666 /777,675 are owned by object packet002163B4, depth approximately0.99859, and remain unchanged after deferred roads. Terrain was roughly0.99949, so these sampled patches are not terrain depth wins. Packet replacement rectangle9538,9634,52,22 is active. Earlier tick120 deck packet used replacement9530,9654,50,24. Investigate texture/quad continuity around the suspect triangle before concluding that no missing geometry exists. Deferred-road edge trace is now running with per-triangle provenance available.

Deferred scan tick170 finds native road coverage beginning around native94,157; prior85/90 probes were outside the road, not rejected road fragments. Packet006DEFA0 has priority384 and writes road depth0.999131918 with yellow paint at96/98/100. Bridge replacement crops resolve to shared imagec3e9d8a5e4fb91a2.dds (manifest source Canyonlands B0:T65:full); no mismatched per-face images for these samples. Running process-local native-UV comparison (planar/straight-edge correction disabled only in diagnostic process), texture pack still active.

Native-UV diagnostic exits0. Inspected gameplay0240 with planar and straight-edge mapping bypassed: lower deck/railing-edge triangle pattern unchanged. Pixel comparison of lower deck region confirms no differences there; differences higher up are randomized gameplay objects. Retained normal mapping settings. Route66 current-build control is running with full texture pack and normal corrections.

Route66 current-build run survey-junction-native-current exited0; inspected0601/0901/1201/1501: road continuous and no conspicuous ground tile blocks in these views. This is bounded spawn/camera control coverage, not full-map signoff. Added source-positioned junction fixture survey_nodes.py using decoded JUNC coordinates/rotation and native terrain height lookup. Node13 (JUNCTION_2LaneSplit) running to test road/terrain overlap away from original spawn.

Node13 fixture exits0, capture0601 shows obvious central sand gap behind/under car (bottom centre). This is a concrete remaining road-hole repro, unlike ambiguous tiny stripe differences at spawn. Pose recorded in survey-node-node-13/pose.json. Targeted node13-trace running to distinguish missing emitted geometry from rejected road fragments; all normal rendering and texture pack active.

Node13 trace confirms integration defect in deferred-road candidate: terrain writes depth0.998536170 at155,235; vehicle subtractive shadow packet00205FD8 writes0.998529792 without changing colour; road006D0634 then fails at approximately0.998533070. Its geometry is present. This shadow-created gap is not proof that every earlier user hole shares this cause. Candidate now stages native vehicle subtractive packets after roads, before water, depth-tested without depth writes, preserving their original geometry/blend. This prevents a shadow footprint from acting as an opaque road occluder. Build/visual verification pending.

Shadow-order candidate E92228EB5E9F26E64D5F4015C29B28B63B2F19714E2154609AC909B47438AE54 staged. Node13-shadow-fixed exits0; inspected0601 exactly matching fixture: central sand gap is gone, road is continuous beneath/behind player. Existing569 contracts pass (they cover mesh/provenance, not shadow compositing; native capture supplies that evidence). Texture pack remains enabled. Other junctions, final bridge and cross-map coverage still required.

Node28-shadow-fixed exits0. Inspected0601/0901: approach is on the hillside facing junction; do not describe this as driving through junction. Turning-lane triangular patch visible around image500,210 still needs attribution. Prepared matching node28 trace points112,70/120,72/125,76/100,72. Current-build reverse overpass traversal is running.

E92228EB reverse overpass run (survey-shadow-order-overpass-drive-aligned-lane-reverse) exits0. Inspected200/240/320/380: continuous deck and road at exit in those views. Existing deck edge colour discontinuity persists; earlier trace established object-owned pixels and shared texture source, not missing geometry at sampled points. Node28 trace running now.

Node28 trace exits0: sampled turning-lane pixels are written by junction packets00201408/002013E8 with native priority384 and replacement rectangles7410,5296,92,58 and7500,5296,76,58. These samples are not terrain depth wins. Code audit found deferred junction classification bypasses the previous AlphaTest world-object planar/straight-edge correction path. Candidate restores those helpers during native-road replay, retaining their exact-view-space tag2 guard so XRTP strips do not gain object-specific correction. Build and same-view comparison pending.

Junction-mapping build succeeds;569 contracts pass; node28-mapping-fixed exits0. Inspected0601: same authored diagonal light/tire-mark region remains; mapping differs along adjoining edges. Texture source audit identifies Canyonlands B0:T119:full -> images/663f5968d05dfc85.dds, whose pixels visibly contain that diagonal light region, black skid marks and white/yellow lane paint. Retain original asset appearance rather than flattening it into uniform asphalt. This suspected patch is not evidence of missing geometry. Dreamland current-build water/terrain control running via survey_water_control.py.

Dreamland current-build control exits0; inspected0601/0901 show path and terrain without new coverage gaps. Water is outside these views, so this run is not water validation. Low-camera reverse overpass run exits0; inspected200/240 deck and320 exit show continuous road/deck coverage. Retained captures provide further near-camera review; report remains open pending completion audit.

Opposite/downhill low-camera traversal survey-low-camera-overpass-drive-aligned-lane-long exits0. Inspected200/240/280/320: full deck and exit road coverage. No missing bridge polygons reproduced in these views on current4765D8C7 build. Bayou control underway for shared depth/water path.

Final-build junction sweep: node8-final exits0, inspected0601/0901 show continuous turnoff and dual-road coverage below cliff dwellings. This is a stationary source-positioned side view, not a traversal. Node9-final running next. Goal-file diff whitespace check passes; only existing LF-to-CRLF notices emitted.

Node9-final exits0; inspected0601/0901 show continuous road and turnoff, with arch bridge visible at distance (not a bridge test). Queued remaining source junctions12/14/18/20/22/31/13 in sweep_remaining.py; each launches the same isolated native harness sequentially, retains per-case result/hash/captures, and stops on harness failure. All use normal Enhanced settings and active texture pack.

Node12-final exits0. Inspected0601/0901: merge road generally present, but small tan triangle inside right shoulder near image720,460 in0901 is suspect. Do not pass this point without trace. Prepared node12 trace186,153/190,154/180,152; current probe tick170 uses chase view, so camera alignment must be checked (0901 low-camera view differs from0601). Remaining sequential sweep continues.

Node14-final exits0; inspected0601/0901 gas-station turnoff: no broad sand hole, but thin light seams outline parts of the junction texture boundary. Keep this distinction from missing polygon coverage. Node12 tan wedge remains queued for trace. Remaining sweep still running.

### Node12 overlap stencil defect
Final build junction sweep exits0 for12/14/18/20/22/31/13. Reviewed18/20/22/31 at0901 and13 at0601; node13 prior shadow hole remains removed. Node12 low-camera wedge at pixel720,450 is covered by packet00205188 at z1527.51 but final depth stays at distant road z8171.26. Mapping-bypass run reproduces exactly. DDS T122 corresponding UV72.72,161.90 contains road colour, so the initial transparency hypothesis was unsupported. Source inspection finds replay pass3 cleared eligibility stencil3 on the first fragment, preventing later native-ordered overlapping junctions in the same batch from drawing. Patch retains eligibility through colour replay then retires it in a colour/depth-disabled replay. Build/test pending; do not claim fixed yet.

Verified overlap patch: staged DA9A9F67588ACCA2C6EA92B5876E3B1D260B034EB36C82A8CF9E87029CD6466C;569 assertions pass. survey-node-node-12-overlap-fixed exits0. Inspected0601: tan wedge removed. Target pixels720,450 and732,450 now colour58,49,33 depth0.999358773; before they remained distant-road colour115,90,58 /90,74,49 depth0.999892414 /0.999892473. This causally verifies the batch eligibility defect. Remaining batch-order design review, junction seams and original far-bridge reproduction still open.

### Remaining seam: source junction attachment precision
`survey-node-node-14-seam-trace` exits0 on DA9A9F. At native160,205 packet00203B54 writes junction road depth0.998165131. At160,206 packet006D01D0 writes strip depth0.998142064. Their boundary edges project to Y205.35742..205.36336 and205.68451..205.71774 respectively, leaving ~0.34 native pixels (~1 presentation pixel) of uncovered geometry. This is not a depth-test rejection at the sampled junction/strip interiors.
`audit_junction_attachment_gap.py/.json` decodes the authoritative V8:2 EXP with the V8_2 packet dialect. JUNC14 uses slot771/group239/scale7. Its bottom mesh row lies at local Z=-511 with X=-199,0,200, while child connector774 is (-0.43359375,0,-512.103515625) in those mesh units. Original LOAD80104550 traverses child slots and selects the connector aligned with the RSEG control direction; LOAD80104D1C separately rotates/truncates/conforms mesh vertices. Thus the road endpoint and patch mesh take distinct quantized paths. Camera translation truncation may contribute too; it is not established as the sole cause. Do not stretch/weld arbitrary nearby geometry or change level assets without a source-backed attachment rule.
`bridge-source-placements.json` confirms exactly bridge_1(id56,slot53) and overpass_1(id48,slot450) named bridge placements; current test targets the latter. Latest overlap-fixed reverse low-camera traversal exits0; inspected200/240/320 show covered deck and exit. Original intermittent bridge failure remains unproven. Node14 thin seam persists and is not covered by the node12 stencil correction.

Camera-fraction diagnostic is rejected and removed. Build9714A488 test used RECOMPONE_DIAGNOSTIC_PRECISE_TRANSLATION=1 in its isolated process;576 assertions confirmed native SXY/SZ unchanged. `survey-node-node-14-camera-precision` exits0, inspected0601 still has seam. Trace junction edge becomes nativeY205.13264..205.13855 with z541.47 (previous539.85); route begins205.70554..205.73877 with z537.64. Gap grows to~0.60 native pixels. Exact camera translation cannot fix mismatched authored mesh/attachment endpoints. Removed GTE fraction property, scope/helpers, generator changes and diagnostic-only tests. Retained adding scale7 to existing mesh-depth scale coverage. Restoration build pending. Do not describe this experiment as a retained fix or re-run it after restoration as though its flag still works.

Restoration staged as0C09A02C5918435A8FE047663F5D58F05192228F52FCC1BFDED67C545FD6BCD9;573 existing/native mesh assertions pass (scale7 coverage adds4). Gte.cs and V82MeshClipCompat.cs have no diff from before the fraction experiment. Source stencil overlap fix remains.
`audit_junction_boundaries.py/.json` now covers all10 current V8:2 Canyonlands patch instances and30 authored child attachments. Nearest boundary distances range0.1094..1.2693 source mesh units across all5 distinct junction mesh shapes. This gives source-backed correspondence rather than an arbitrary screen-space seam threshold. The original LOAD80104550 code uses these child slots to choose each RSEG endpoint. Implementation must preserve that topology and native game state; do not apply blanket polygon dilation, depth bias inflation, or map-specific coordinates. An enhanced-only fractional vertex representation would be required to exactly join independently quantized boundaries without changing source assets. No such joining implementation is present yet.

Attachment experiment: LOAD801079A0 writes edge count at JUNC+0x11 (byte), not +0x12 (optional flag0x40 data). First diagnostic changed no vertices and is not evidence of correction. Corrected survey-node-node-14-attachment-count exits0; paired0601 inspection shows the fine horizontal seam removed. The candidate uses authored child/RSEG connection planes and only enhanced GTE metadata, preserving native vertex RAM and SXY/SZ/FLAG. It is still diagnostic-only. Replaced initial convex-hull matching with actual original BIN triangle boundary topology and connected edge membership, excluding nearby interior vertices. 583 MeshClipContract assertions pass; topology-restricted native node14 validation is running. This is enhanced seam reconstruction, not a claim that the retail PS1 moves mesh vertices this way.

Retained attachment continuity (7AF07B64): runtime face parsing must follow 80021F70, not the file packet dialect: mesh+6 is face count, mesh+0x10 is face stream, command bits26..29 select the native kind, and indices are already eight-byte vertex offsets. The first file-dialect boundary diagnostic changed nothing; it is not validation. Corrected native topology run97EED358 exits0 and removes node14 seam in0601. It corrects only connected boundary vertices within the two native coordinate quantizations; nearby interior vertices are excluded. Child selection follows LOAD8010719C, and the native source records remain unchanged. Baseline packet trace shows junction and strip endpoint packedY205, while Enhanced independently projects them to205.36/205.70. Joining their authored connection restores continuity lost by the subpixel projection; this is an Enhanced adaptation, not a claim that the PS1 itself moves the source vertices.

Control run97EED358 with the candidate enabled: nodes12/13 and Route66 exit0. Inspected node12 0901/1201, node13 0601/1201, Route66 0601/0901: previous coverage repairs persist and no new road gap is visible in those views. Shared visibility bounds now include the fractional attachment positions. 585 contract assertions pass, including native SXY/SZ/FLAG, signed quarter-turn child rotation, non-boundary exclusion, nested mesh scope and scene retirement. Retained build7AF07B64177425D24D1ECCA630E3CED433F0D8A7165D57B3D5518BF2815FF4FF is staged. survey-node-node-14-retained-default exits0 with no attachment override; inspected0601 confirms seam removed and native runtime confirms replacements On.

Additional bridge coverage: survey-steering-bridge-overpass-drive-aligned-lane-reverse on97EED358 exits0. It adds process-local LEFT195+14 and RIGHT220+14 to the actual physics crossing, with low camera. Inspected190/210/230/250/280/320 individually: deck and exit remain covered. This still does not reproduce or causally fix the user's intermittent missing-polygon report. Goal remains active, and the full mission-results location and compositing batch-invariance review are still open.

Road batch isolation: added diagnostic-only RECOMPONE_DIAGNOSTIC_ROAD_TRIANGLE_BATCHES=1, splitting the road pass after all geometry/UV preparation. This changes only draw grouping and retains native order. Normal/default path is unchanged. Staged3D5E0C204C252FDF7A1AFA59AE009B62D18F48C231984F207C6B404AA84792EA; survey-node-node-12-batch-normal and -single both exit0. Inspected0601 pairs; three road ROIs (670,490)-(1100,710), (650,410)-(800,510), (450,430)-(560,550) have zero RGB differences. Native probe720,450 retains exact0.999358773 depth and58,49,33 color. No reproduced batching defect in this scene; this bounded check does not prove every possible overlapping arrangement.


## Clipping mechanism and native victory fixture audit

`survey-native-clip-bridge-overpass-drive-aligned-lane-reverse` (3D5E0C20, exit0) explicitly disables modern mesh submission, retaining all other fixes and active replacements. Individually inspected210/250 show actual near-deck holes. Matching normal Enhanced steering captures on97EED358 and instrumented21019A4C show continuous deck. The instrumented crossing exits0 with zero MeshClipFallback and ModelBoundsMismatch messages; modern submission is confirmed active. The clipping repair exists in HEAD and predates this goal. This isolates a relevant failure mechanism but does not establish that the recent user report used the old path or that an additional bridge fix was made. See bridge-clip-path-audit.json.

Houston original7/type71 Quest fixture reaches Canyonlands and results (quest-results-road-houston-corrected,3D5E0C20, exit0); inspected result90/300 have continuous visible road. The attempted two-SELECT high-camera variant also remains a chase view, so it is not evidence for the elevated report. Native source800356AC sets camera+4 flag0x02000000 along with gp+C74=1; the preexisting result fixture wrote only C74 and thereby bypassed the native camera transition. Added opt-in RECOMPONE_QUEST_TEST_NATIVE_RESULT_TRANSITION so rendering tests let native victory logic enter that transition after native opponent destruction; ordinary fixture/default gameplay unchanged. Staged7C2A9600; proof pending.

Native victory fixture follow-through: first9000-poll run advanced to Ski Resort, overwriting same-named result images and failing expected progress3 (actual7). Exclude that run from Canyonlands visual evidence. Corrected6000-poll quest-results-native-transition-bounded exits0 on7C2A9600 with all assertions including Houston71, mission0, progress3. Inspected90/300/360/480 individually: correct orbit now runs. Road obscured by foreground cliff at90/360; visible pavement continuous at300/480.585 contracts pass on retained source. All owned game processes exited.

## September 14 bridge approach screenshot

User accepts the other fixes and supplies user-bridge-approach.png under artifacts/canyonlands-bridge-20260914. The target includes deck/abutment/road intersection seen before reaching the bridge; on-deck checks did not establish its correctness. Current7C2A9600 source-positioned approaches at(63200000,84800000) and(63500000,84662000), yaw3353, exit0. Active replacements retained. Inspected0601. Native subdivision versus modern submission at far pose produces the same entrance shape; close-camera hole mechanism is not sufficient for this image. Source RSEG10 at t0.1 gives lane position(63427824,84572570) and reverse tangent yaw3580; matching lane fixture running. Front trace(133,120) first receives terrain at0.999565542, then bridge side at0.999778271, then nearer deferred road at0.999562502. This sampled side region is covered by road; does not yet establish whether the user's deck cut is the same event.


## September 14 bridge approach: source geometry and overlap trace

The screenshot-supplied approach is reproduced on authored RSEG10 at t=.1,
position (63427824,3023954,84572570), yaw3580. The active texture pack stays
loaded in every new approach diagnostic. Original native NCLIP (retail cull
restored, Enhanced world cull disabled) retains the same entrance silhouette
as normal Enhanced. Disabling all world-face culling exposes back faces and
is not a correction.

Source triangle matching identified all 73 traced rejected bridge triangles
in bank0/group112; 25 have unique distance signatures and preserve vertex
order 0,1,2. A least-squares transform of those source vertices to the logged
camera-space vertices has maximum residual0.000198 camera units. This rules
out a changed bridge mesh or generic packet-order reversal for these faces.
The staged EXP SHA256 CE5E1CA9 equals both the disc-work copy and original
LEVELS/V8 copy.

At native display pixel(90,126), presentation(432,378), frame4075/tick170,
terrain first writes depth at camera Z1875.962. Bridge packet00215800 then
writes its side face at Z3890.9504 with depth-test=false/write=true,
replacing visibly nearer sand with a farther gray support. This is a concrete
foreground-occlusion error. No correction claimed yet: an opt-in scenery
compare/update diagnostic is being evaluated. Evidence in
artifacts/canyonlands-bridge-20260914/entrance-pixel-owners.txt,
culled-source-matches.json and bridge-view-transform.json.


The scenery-depth A/B confirms causality: with opaque-world compare/update,
the same sample remains foreground sand at depth0.999481559 when support
Z3890.9504 is submitted; previously the support replaced it at0.999758244.
The bridge entrance no longer shows buried pillars through the embankment.
Native vertex positions, NCLIP decisions, triangle order, texture coordinates,
replacement textures, road priority and collision data are unchanged.

Production correction: coherent opaque/alpha-tested WorldObject packets now
use compare/update in Enhanced, with LEQUAL for coincident faces. Vehicle
layers, translucent scenery, water/effects and uncorrelated fallback packets
retain their existing contracts. This generalizes the missing scene-depth
comparison and contains no arena/asset identity checks. The existing
source-depth batch state prevents painter ALWAYS from overriding the compare.
RECOMPONE_SCENERY_DEPTH_COMPARE=0 is an isolated diagnostic rollback switch;
normal configuration enables the fix. The bounded world-cull trace remains
opt-in and does not alter face selection.

593 MeshClipContract assertions pass, including the source-matched buried
support, eligibility exclusions and the existing source tower winding tests.
Final staged executable SHA256:
A7AE25FB8B6E7C590480F89B9456276A2BDB5659C7ACA8904D856999B2E3D86C.
Matching user-oriented entrance at yaw3353 exits0 and has been inspected;
road meets deck/embankment without the exposed buried support strips.
Crossing and shared-scene regression validation still in progress.


Final validation 2026-09-14T09:15:13.995304-04:00: all four final A7AE25FB runs exit0. Every saved image
was inspected individually. Final entrance matches the supplied approach;
bridge deck240 and exit300 remain continuous;340 is beyond the bridge.
Fixture120 starts above the deck and settles; later380/420/601 are off-bridge
terrain views, not deck-coverage proof. Hoover control preserves the tower
fronts and road. Texture replacements remain loaded/applied in every run.
Detailed manifest: artifacts/canyonlands-bridge-20260914/visual-verification.json.
Implementation/visual-validation goal complete; bridge remains pending user
review in item4. Original item2 is closed and open items were renumbered.
