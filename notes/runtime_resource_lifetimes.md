> September 14, 2026: User closed the resource-cleanup/soak TO-DO item and waived the five-hour run based on accumulated testing. No five-hour soak was performed; earlier pending-run statements below are historical.

# Runtime resource lifetime policy

User direction, September 11: treat level transitions like a console game.
Release data once its last consumer stops, before the next load. Continue
looking for such boundaries when adding or changing runtime features.

| Resource | Owner and release boundary |
| --- | --- |
| Imported gameplay BIN/ANM/model runtime and SND/SPU handles | Match; retail bank destructor after match objects retire. |
| Imported statistics and upgrade tables | Match; free alongside imported banks. |
| Shared identical imported image/palette rectangles | Native texture allocation tree; individual banks cannot invalidate shared backing. Native whole-tree destruction retires backing ownership. |
| Synthetic texture descriptors | Host bank/match owner; retained independently of native backing until explicit release. |
| Packet ownership, object tracking, water/atmosphere inference | Scene; clear at match exit before shell/next arena. |
| Texture replacement lookup entries and terrain anchors | Scene; clear at match exit, trim lookup capacity. |
| Display configuration, immutable catalogs/atlas assets, reusable rendering infrastructure | Shared runtime; retained across scenes while still needed. |

The native LOAD routine itself discards its packing tree at 0x80104930 after
uploads. This releases allocator bookkeeping, not the uploaded VRAM pixels.
Do not confuse that boundary with freeing CPU descriptors or wiping VRAM.
Never let old coordinate ownership suppress a later scene's native frees.

Do not use an allocation failure as a cleanup trigger, reset an allocator with
live consumers, or increase memory budgets to hide missing ownership. New caches
should declare their owner and invalidation boundary. Fixed reusable buffers do
not need repeated allocation merely to make process memory fall at every exit.

Validation: the continuous Quest harness requires a bank, scene, and texture
lookup release after every mission and rejects SPU/VRAM exhaustion, stale backing
warnings, CPU-heap exhaustion, and the legacy blanket audio reset. Forced wins
test transitions and resource lifetime. The user accepted the observed shared
objective event as sufficient trigger coverage on September 11; this harness
does not independently complete every mission's objective through natural play.

## September 13 cleanup audit (pre-soak checkpoint)

The four-mission diagnostic attributes live PC allocations to allocation callers
and transition generations when `RECOMPONE_TRACE_RESOURCE_LIFETIMES=1` is set.
This is opt-in instrumentation only; neither allocation age nor caller address
is used to decide what to free.

The baseline retained 11 abandoned upgrade-status allocations (264 bytes) after
cycling imported selector vehicles. `ObjectUpgradeStatus.Remove` discarded the
owner mapping without releasing its allocation. The fix releases this private
allocation when its vehicle is actually freed, preserving result-screen callbacks
and CPU registers. Match teardown also releases any remaining owned entries
before clearing the dictionary.

The baseline's total PC allocation bytes across four teardown snapshots were
207312, 275264, 199520, and 165064. Most arena-dependent allocations were replaced
between snapshots, not cumulatively retained. Animation/model allocations also
survived the campaign; their longer-term trend is not established by this short
run. No blanket heap sweep or forced per-frame garbage collection was added.

Evidence lives in `artifacts/resource-audit-20260913/runtime.log` and the fixed
run's `artifacts/resource-audit-fixed-20260913/proof.json`. These fixture-driven
runs exercise menu selection, gameplay, results, level transitions and ending;
they do not prove natural objective completion or visual/audio fidelity. Captures
are disabled to avoid accumulating disposable screenshots.

Per user direction, the five-hour soak must not start at this checkpoint. It
remains pending, including broader allocation trends, sustained frame times,
resource exhaustion, and visual/audio review. Do not claim soak completion from
these short diagnostics.

Fixed-build short verification passed: four missions in 163.406 seconds, clean
exit (0), settings unchanged, no invalid frees, and no remaining selector upgrade
leak records. Each boundary has exactly 11 fewer allocations and 264 fewer bytes
than baseline. Staged executable SHA256:
`ad878a5f3cc0ea4950fd7e30d4d3937aad40f71c963f3692ff127604ad7b45fb`.
The process has exited; no soak or background monitor is running.
