# Pause-menu cleanup — Rockwell Extra Bold

User-selected family: Rockwell Extra Bold (`ROCKEB.TTF`). Cyan is unchanged.

## Font path

`build_rockwell_extra_bold_font.py` builds a complete indexed GAME.FNT plus an
HD atlas matched by `SHARED/GAME.FNT` provenance. It does not call the older
compiler's glyph-hash manifest writer. It checks that all originally present
glyphs still exist. Native metrics drive text placement and truncation.

The inconsistent pause lettering was not caused by missing font files:
the baseline log resolves GAME.FNT at 4x. `DrawRect` classified anything above
42% screen height as compact HUD. PAUSED (y=74) and Track (y=98) therefore lost
font coverage/smoothing flags while Quit and Resume (y=114) retained them.
The fix excludes active native modals from that HUD rule and marks every
successful filename-resolved FNT as a font, independent of dimensions/position.
All four rectangle vertices retain the same shader flags. The DDS keeps TTF
coverage alpha; the shader uses that alpha rather than the native glyph edge.
The approved family is compiled at 11 logical pixels (reduced from 12 after
the user requested a smaller size) to suit its heavier/wider
metrics, retaining the original line height. Both native choice-value calls
temporarily inset the text box by 12 pixels on each side while measuring and
drawing, then restore it for the arrow calls. Long values still use the native
fitting behavior; track selection and audio logic are unchanged.

## Panel path

`V82ModalStyle.DrawPanel` replaces the presentation-only common native
`80011BB8` panel builder, preserving its input rectangle, OT link and packet
allocator. A four-word native tile is tagged after its bytes are written.
The renderer draws a rounded dark panel with a gold border using an analytic
distance function antialiased at output resolution. No texture/VRAM guesses,
map or vehicle branches, or rewritten menu/input state machine are involved.

The tag is invalidated by packet reuse and cleared on scene reset. Native
rendering is retained outside Enhanced gameplay. The same native seam covers
pause, confirmation and quest/objective panels.

## Verification

- `tests/ModalStyleContract`: 42 assertions cover three panel sizes,
  allocator/link preservation, fallback, coordinate retention, stack/return
  preservation, normalized addresses, provenance invalidation and choice-box
  restoration even when drawing unwinds through an exception.
- `run_pause_style_proof.py`: one hidden, silent process from V8_2_LOOSE;
  native START at gameplay poll 120, capture at poll 601, CROSS/Resume at 660.
- First candidate capture caught an invisible panel caused by uninitialized
  UV varyings for untextured primitives. Corrected by assigning UVs for both
  textured and untextured vertices; retained the rejected capture as evidence.
- First candidate resumed, reached native defeat and returned to the shell
  with clean exit. Its inherited map-fidelity gate failed route/performance
  checks (median 51.94, minimum 25.33 FPS); this is not a map/performance pass.

The next capture verified the panel but exposed track/arrow crowding with the
larger font. That candidate is also retained, not offered as a finished proof.
The final candidate includes the size/gutter correction above.

The 12-pixel candidate was captured and inspected with correctly separated
track text/arrows, smooth border and readable selection state. Native Resume,
defeat, shell return and clean exit passed. Run FPS median/minimum was
52.98/29.60: the performance floor remains failed, not certified by this UI
test. Executable SHA256 is
`63BB8CC3D2C61CCA1F310AB2254007D78FE488AE2C73D278731D34AC2E2DCB19`.

The user requested a smaller font after seeing that proof. The 11-pixel
revision changes only the FNT/DDS/manifest entry, not the executable, panel or
menu logic. Its native capture is under `artifacts/pause-style-20260904/smaller-font`.
The full-size frame was inspected: the smaller face, gold border, disabled gray
entries, selected white Resume and separated arrows are visible. Resume,
native defeat, shell return and clean exit passed again with one process.
That run's median/minimum was 52.51/10.65 FPS, so it fails the performance
gate and is NOT a renderer/map release approval. No attempt was made to hide
or fix that separate performance failure within this UI task.

The user visually approved the smaller-font pause menu on 2026-09-04 and
requested commit/push. This accepts that font size, panel and pause layout,
not the separately failing performance gate or other renderer work.
The exact staged runtime was exported independently of older uncommitted
renderer/audio/vehicle changes; all 42 modal and 159 mesh packet assertions pass.
Full menus and 4:3,
confirmation/objective pages remain review work; unit tests are not visual
acceptance of those pages.
