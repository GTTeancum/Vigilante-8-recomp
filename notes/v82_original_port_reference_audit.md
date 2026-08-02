# V8:2 open-item reference audit against the original V8 PC port

Date: 2026-07-31

The original V8 port is the default implementation reference for sequel fixes.
Shared systems should be reused directly; sequel-only behavior should be
limited to title-specific data, addresses, and authored UI differences.

| V8:2 item | Proven original-port path | Sequel action / evidence |
| --- | --- | --- |
| OPEN-002 transition side artifacts | `HostWindow.ApplyGraphicsView` from commit `ae9cd66` keeps SHELL menus and FMVs at authored 4:3 and enables widescreen only while `GpuHle.GameplayActive`. | Trace the Quest return's `GameplayActive` transition and presentation target lifetime. Do not add a second aspect/layout system. |
| OPEN-005 imported voiceovers | The shared loose-disc path already streams XA and CDDA through `CueFs`, `LibCd`, and the SPU. | Reuse the native V8 voice assets and map their existing V8 event IDs into V8:2's voice-event table. No wrapper-owned voice player is justified. |
| OPEN-006 terrain spasms | The stable original enhanced path uses one ordering-table depth for the whole primitive and keeps projective W limited to UV interpolation (`GlBackend.DrawTri`, commit `90a5469`). | Audit sequel packets for missing/unstable OT provenance and keep the same primitive-level depth rule. Do not invent per-vertex depth multipliers. |
| OPEN-008 low-resolution textures | Original enhanced rendering uses bounded shader reconstruction (`smoothedTexture`), includes non-UI raw-texture packets, and clamps filtering to each primitive's UV bounds (commits `508ad0a` and `90a5469`). | Verify V8:2 terrain/model packets reach that same path and are not misclassified as UI. Keep the established 512-class world/model limit and 1024-class UI limit. |
| OPEN-009 defeat-text spacing | Original and sequel share the PS1 GPU text path. The first game has no host-side kerning replacement. | Inspect V8:2's submitted glyph coordinates/packet stream first. A sequel-only vector-font spacing shim would be an invented system unless the native packet data proves it necessary. |
| OPEN-010 pause/objective centering | The original port presents authored menu compositions as a fitted 4:3 target through `OutputPanel.GetPresentationSize` and `HostWindow.PresentTexture`. | Keep the entire overlay as one 4:3 composition; verify the sequel is not applying gameplay-wide expansion to individual overlay packets. |
| OPEN-011 HUD regression | The renderer already has the shared native packet path plus the accepted V8:2 SVG atlas path. | Correct packet classification/atlas measurements in those existing systems. Do not add presentation-stage patches. |
| OPEN-012 loud default music | Original CDDA is mixed through the emulated SPU CD-volume registers before the host master-volume stage (`Spu.Mix`). | Compare V8:2's authored `_cdVolL/_cdVolR` writes and default master-volume migration with the original. Do not add a separate arbitrary CDDA gain until that comparison is measured. |
| OPEN-013 absent menu music | Original loose media preserves CUE `INDEX 00` and `INDEX 01`; `LooseCdda.TryReadSector` maps the whole index-0-to-end interval. | The final V8:2 run requests LBA `147300`, but the embedded sequel manifest starts track 2 at/indexes it from `147316`, so the request is rejected as unmapped. Rebuild the sequel manifest from the actual CUE with track-2 `index0Lba=147300` and `startLba=147316`, then test the native menu CDDA state machine. |

The dither fix for OPEN-007 retains the original port's accelerated/software
`Ps1Dithering` gates. V8:2 additionally removes the residual ordered pattern
from the final enhanced gameplay presentation only. The cleanup is explicitly
title-scoped so the accepted original V8 renderer is unchanged.
