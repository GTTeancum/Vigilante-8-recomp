# src/skipped

Out-of-scope subsystems (per CLAUDE.md): renderer, controls/input,
audio playback engine. These will be **rewritten** in the port; their
1:1 decomp is explicitly excluded.

This directory documents the **seam contract**: the exact entry
points and data structures the in-scope code expects, so the new
implementations can drop in.

## Contents

- `renderer.md` -- prose description of the renderer interface
  (otag chain, DRAWENV/DISPENV split-screen layout, primitive
  emission).
- `audio_init.c`, `audio_voice.c` -- audio engine seam (the asset
  loaders for SND/VAG remain in src/assets/).
- `pad_input.c` -- controller input seam, documenting the uRam-
  0000062c (P1) / 00000630 (P2) button word bit layout and how
  V8_MainLoop / Menu_Pause consume it.
- `load_renderer.c` -- LOAD.DLL renderer-adjacent helpers (primitive
  emitters + MDEC video player), 11 functions documented with seam
  contracts.
- `shell_ui.c` -- SHELL.DLL UI/menu state machine (splash, vehicle
  select, options, demo player), 72 functions documented with the
  shell->main_loop input/output contract.

## How the seam works

In-scope code references the out-of-scope code only via:
1. Specific globals that hold the chosen-vehicle / chosen-mode /
   pad-button-word state. These are listed in include/globals.h.
2. Named function calls into the seam (e.g. Audio_PlaySfx,
   Pad_Tick, Render_BuildLists). The rewritten subsystems can
   provide these as drop-in replacements.

No physics, gameplay, or asset-loading code reads the renderer's
output buffers or owns audio voice state. The split is clean.
