# V8:2 current fidelity visual proofs

## 2026-09-04 current five-map review — NOT USER APPROVED

**Paused by user:** user reported terrain and Sheila transparency look much
better, then explicitly redirected work to side clipping. The Winter Games
capture was interrupted immediately. This is not a completed five-map set;
do not resume it automatically or mark the original goal complete.

Executable: `V8_2_LOOSE/Vigilante82PC.exe`

SHA-256: `71618E6B89D311CC47522B0AAB1A8EAB873E5E054B6E3A5D426ECBBE4B0FA8DD`

All current proofs use the same frozen Enhanced executable and native
1920x1080 FXAA presentation capture. Runs are hidden, muted, and sequential,
with no host OS input. The common process-local proof fixture holds vehicle
transforms at gameplay frame 30 and captures at gameplay poll 601 using the
default camera (zero camera cycles). These are visual comparison fixtures,
not traversal tests. The ordinary unheld Hoover traversal separately passes.

Shared repairs remove the synthetic vehicle-glass path and restore native
ordering of ordinary reflection packets. The terrain path now implements
binary-derived recursive 4/2/1 leaves and intermediate height/color morph,
rather than the withdrawn uniform four-unit geometry. No map, vehicle,
texture, CLUT, packet-address or authored-cell exception was introduced.

### Current evidence register

Per-map coordinates, FPS, framing notes and image links follow when the
remaining captures complete. FPS is the median/minimum of logged 60-frame
effective-FPS windows in that proof run, not a claimed instantaneous value
at the captured pixel frame. Each run retains its own `acceptance.json`.

**Known technical failure:** held Hoover measures 56.11 median / 29.21 minimum
FPS. The minimum occurs in frames 3841–3900, before the screenshot operation.
This fails the 30 FPS hard floor. The separate ordinary traversal's
59.96 / 55.70 FPS does not erase or replace this failure.

**Minnesota failure:** 21.38 median / 7.41 minimum FPS. The individual image
also retains the previously reported broken building polygons and bright
distant objects. Neither the performance nor those geometry defects passes.
This build is not release-ready regardless of the roof's appearance.

Utah means native V8:2 Winter Games / `LEVELS_OLYMPIC` (slot 1). An initial
queue selection captured original V8 Ski Resort (slot 17); that artifact is
not the requested Utah proof and must not be presented as Winter Games.

All five map grades and Sheila's material fix remain pending user visual
approval. Technical checks cannot close them.

## Historical evidence — 2026-08-31 (not the current candidate)

Captured: 2026-08-31 EDT
Executable: `V8_2_LOOSE/Vigilante82PC.exe`
SHA-256: `F881E249C416CC81617F5A83B126D36690C360878BDBA22E16461F4D085AB418`
Renderer: Enhanced only
Capture mode: hidden, muted, one process at a time
Presentation format: individual lossless PNGs; no contact sheet or resize

## Loading glyph proof

[Open full-resolution loading proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/olympic-loading-quest-f881-current-20260831/olympic-loading-quest-f881-current.png)

Resolution: 1920x1080

PNG SHA-256: `6DBC3B625CB8576C632BDE8106B4CEED58D4B5FEE0823A6F58E77A319CBF1236`

The test-only deterministic tip selector forced table entry 35 so the previously
unresolved uppercase `Q` and `O` appear repeatedly in `Quest` and `Objectives`.
The capture shows those glyphs at the same edge definition as adjacent letters.
The exact HD Winter Games loading card is also present; the acceptance pixel
correlation against its source asset is `0.999891`. The entire PNG is byte-for-
byte pixel-identical to the accepted 6C3 loading proof, establishing that the
runtime-only pacing/overlay changes did not alter this output.

## Terrain, stock wheel, and close-structure proof

[Open full-resolution Olympic gameplay proof](C:/Programming/GitHub/Vigilante-8-recomp/artifacts/v82-map-fidelity-current/olympic-f881-visual-20260831/olympic-f881-current.png)

Resolution: 1280x720

PNG SHA-256: `842CAF126FC221B4E8036E02DFAC46FAE80716BA4DCB8FF05B3D6226CA1B8956`

This single unscaled gameplay frame exposes the three remaining shared visual
contracts:

- Snow texture is visible continuously from the foreground through the middle
  distance and onto the visible ridge. There is no flat bright-white outer
  terrain ring or hard per-cell switch to an untextured representation.
- The stock V8:2 vehicle is pitched at an oblique angle with its wheels exposed.
  The visible wheel surfaces remain round and do not show the rejected square
  interior/back-face polygon.
- The nearby lodge intersects the upper-left view edge while its wall remains
  present. It does not disappear or open a clipping hole at the screen edge.

These proofs document the currently staged candidate only. They do not mark the
user-owned fidelity goal complete.
