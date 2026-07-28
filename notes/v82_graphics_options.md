# Vigilante 8: 2nd Offense graphics options

The wrapper exposes three graphics profiles:

- **Original** restores the authored 4:3 presentation, stock distance LOD,
  native texture sampling, PS1 dithering, and stock effects.
- **Enhanced** is the recommended Dreamcast/PS2-style profile. It enables the
  complete enhancement set below.
- **Custom** is selected automatically when any individual option changes.

Every enhancement remains independently selectable:

1. **Anti-aliasing and texture filtering**
   - 2x/4x/8x multisampling occurs on the high-resolution HLE framebuffer.
   - FXAA remains an independent final presentation pass.
   - Mipmap and anisotropic filtering reconstruct a screen-space footprint
     after palette lookup. This avoids blending 4-bit/8-bit palette indices.
2. **Bounded texture reconstruction**
   - 3D PS1 pages are reconstructed in shader within the 512x512 content
     class. Menu/UI reconstruction remains within the 1024x1024 class.
3. **Widescreen and HUD**
   - The world framebuffer expands to 16:9 rather than stretching 4:3.
   - During gameplay only, left/right HUD groups anchor into the added margins.
     Menus and FMVs retain their authored layout.
4. **Projected shadows**
   - Dark, semitransparent world projections receive multisampled,
     derivative-softened triangle edges.
5. **Particles**
   - Semitransparent depth-bearing sprites render at the enhanced internal
     resolution and reconstruct partially covered transparent edge texels.
6. **Fonts, logos, and icons**
   - Small UI glyph and icon rectangles use high-resolution,
     transparency-preserving reconstruction. The host wrapper UI continues to
     use resolution-independent vector-source fonts.
7. **Distance detail and haze**
   - Extended distance doubles the recovered native far-object acceptance
     threshold. The separate maximum-LOD option controls which mesh is used.
   - Enhanced haze adds a subtle far-depth blend in the host renderer without
     modifying simulation or collision state.

`RECOMPONE_GRAPHICS_PRESET=Original` and
`RECOMPONE_GRAPHICS_PRESET=Enhanced` are non-persistent automation overrides.
All ordinary menu changes persist in `settings.json`.
