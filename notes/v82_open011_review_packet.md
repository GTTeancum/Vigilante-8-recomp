# V82-OPEN-011 Review Packet

Status: **READY FOR USER REVIEW**. Do not close this item without explicit
user signoff.

## What This Fix Covers

`V82-OPEN-011` is the imported-NPC target HUD regression found after the
accepted `OPEN-020` NPC selector work. The accepted NPC proof was from
`candidate8`; the HUD backing repair is the later `candidate10` V8:2 build.

When an imported NPC is targeted, its target icon uses a guest texture bank.
That changes UV/page/CLUT from the retail target backing, so the old
texture-state classifier missed the target-status backing. The fixed classifier
uses the stable authored HUD rectangle and material instead:

- source rectangle: viewport-local `x=80`, `y=20`, `84x34`
- material: raw, semitransparent, textured UI/screen-effect packet
- output backing: `x=74`, `width=90`
- target icon: `x=76`, `width=40`

## Review Build

The only supported current V8:2 test and deployment path is:

- `V8_2_LOOSE/Vigilante82PC.exe`

`PS1 game` is reserved for original Vigilante 8. Never stage or launch
`Vigilante82PC.exe` or V8:2 mods there.

The historical OPEN-011 acceptance build hashed to:

`EBE9693D4E24BF50342047C3E5D9E4B25951614650169CF0A8DE159FF4064F08`

Do not use `Vigilante8PC.exe` for this review. That is the separate original-V8
Dreamland executable.

## Evidence

- Acceptance report:
  `artifacts/v82-open011-hud/candidate10-acceptance/acceptance.json`
- Fresh current deployed-run acceptance:
  `artifacts/v82-open011-hud/current-review-20260815/acceptance.json`
- Handoff verifier:
  `artifacts/v82-open011-hud/open011_handoff_verification.json`
- Visual proof:
  `artifacts/v82-open011-hud/open011_guest_target_hud_before_after.png`

The handoff verifier currently passes with report SHA-256:

`A9A6F826FF05E1A7E0EAA24A4A725C846E925B23B1FB5B2B20D290FBA89C6E3E`

It checks candidate10 acceptance, the then-current deployed-run acceptance and
historical executable hashes, the source classifier invariants, the visual
proof, this review packet, the open-issues tracker, stale runtime-log cleanup,
and the explicit objective-requirement matrix. The fresh run selected
`guest.v8.dave` as the imported NPC target, injected type `68`, and cleanly
completed in `181.235` seconds.

## Recheck Command

```powershell
python tools\recompone-v8-2\verify_open011_hud_handoff.py
```

## User Signoff Target

Playtest a match where an imported vehicle is selected as an NPC target. The
green enemy silhouette, target icon, armor bar, and complete SVG backing should
line up without the left side missing. Close `OPEN-011` only after explicit
acceptance of that visual result.
