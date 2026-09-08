# Unstretched attachment diagnosis, 2026-09-08

Current follow-up: the user approved whole-assembly chassis anchoring. See `notes/v82_rigid_water_attachments.md`; the old vertex-stretching helper remains inactive. Historical findings below are retained chronologically.

Status: original PS1 spacing independently confirmed for Sheila. The support-stretching experiment is disconnected from object rendering. No replacement mesh deformation or guessed mount offset has been introduced. The visual gap has not been closed: doing so would intentionally change the original assembly.

The staged executable is `V8_2_LOOSE/Vigilante82PC.exe`, SHA256 `83FEC2293A6A9EA4B7664B1DE174E98DF6262974F9B6F06925FF0F9F6B23A9DA`.

## Evidence

- Source and staged `SHARED/COMMON.EXP` are identical, SHA256 `E4E2F49C633F37276E6E271A863A6EF77CD4242EF3F4C7A7DC8947D2E9C0BA1E`.
- Native mode-2 transform selection uses common slots `(63,70,59,77,63,70)` for Sheila. The rear support roots retain the source mount X/Z coordinates, approximately X +/-17694 and Z -27590. Their native parent walker resolves to the vehicle, not an unrelated sibling.
- Settled runtime source vertices remain unchanged. Scale shift 11 makes each rear support's inward reach 54 * 32 = 1728 fixed units. The leftmost whole-body bound is -14080 and rightmost is 14208; inward support ends are -15966 and 15965. Even the whole-body bounding box therefore leaves gaps of at least 1886 and 1757 fixed units. Actual surface distance can be larger.
- Reproducible read-only audit: `python artifacts/audit-unstretched-attachments.py`. Output: `artifacts/visual-issues-attachments-unstretched/geometric-audit.json`.
- Native free-physics repeated water/dry transitions complete through tick 1050 with no fixture failures or fit records. This proves lifecycle operation, not visual attachment continuity.
- `artifacts/attachments-original-renderer/recompone_present_gameplay_0601_1280x720_off.png` was individually inspected: the same rear separation appears through the original polygon renderer. This is still the recompiled port, **not an independent original-console capture**.

## Native paths inspected

`8003E32C` wraps mounts during transition; `8003E774` handles the 32-tick animation and reparents them; `8003E4A8` selects authored overrides or common fallback models and preserves mount X/Z. Water controller `80040598` animates the rear child/rotor and handles water contact forces. No support-extension adjustment was found in these paths.

Recursive render `8002D778`, object render `8002D9E0`, vertex projection `80021F70`, matrix update `8002CF74`, alternate-LOD selection `8002C7D0` and compatibility LOD handling were inspected for missing connectors or a further support transform. No such adjustment was identified. This is a source-inspection finding, not a proof that every possible native path has been excluded.

The user's earlier original-game reference shows another vehicle and cannot establish Sheila-specific mount offsets. The independent same-vehicle comparison below supersedes the previously outstanding reference requirement.

## Independent PS1 execution, follow-up

Executed the supplied original `Vigilante 8 - 2nd Offense (USA).cue` using the PCSX ReARMed libretro core in a headless Python process. This executes the original MIPS binary with an independent CPU/GTE/GPU implementation, not the recompiled port. HLE BIOS is used; no console BIOS was present. All controller input is a libretro callback contained within that process; images come directly from its video framebuffer callback. No desktop/window control or capture occurred.

Reproducible harness, states, RAM snapshots, and fixture scripts are in `artifacts/ps1-attachment-reference`. Booted through the original menus into Florida with stock Sheila. The fixture temporarily redirects the native physics entry to the original `8003E32C` transformation routine with mode 2, restores the original instructions after four emulated frames, and lets the original transition finish. The later inspection fixture freezes physics and changes only vehicle world pose and camera distance/angles. It does not alter attachment/body geometry, mounting coordinates, or child matrices.

Individually inspected native `close.png` and `rear.png`: the rear support ends visibly stop outside the body in the original PS1 renderer. `rear.png` provides the clearest unobscured view. Earlier boot/menu images are navigation evidence only.

`python artifacts/ps1-attachment-reference/compare.py` compares the independent original game's RAM against the port's unstretched frame-150 snapshot. All 40 exact checks pass across ten rendered nodes (body slots 5/17, supports 63/70/59/77, rear children 60/61/78/79): vertices, scale shifts, local translations, and local rotation matrices are identical. Full evidence is `comparison.json`.

This resolves the earlier uncertainty about Sheila's authored spacing. It does not establish other vehicles' placement or prove every intermediate animation frame equivalent. Closing this gap requires an intentional departure from the original assembly, not restoring a missing native transformation. Preserve the user's rejection of noncanonical fitting; do not misreport this reference verification as a gap-closing code change.
