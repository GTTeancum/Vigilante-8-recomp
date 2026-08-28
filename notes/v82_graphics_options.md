# Vigilante 8: 2nd Offense graphics options

The wrapper exposes two shipping graphics profiles:

- **Enhanced** is the recommended Dreamcast/PS2-style profile. It enables the
  complete enhancement set below.
- **Custom** is selected automatically when any individual option changes.

Both profiles use the Enhanced GL renderer. The preserved PS1 software
renderer is a developer-only, read-only visual oracle and is not exposed in
the shipping settings UI.

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

`RECOMPONE_GRAPHICS_PRESET=Enhanced` is a non-persistent automation override.
An isolated developer oracle run must set
`RECOMPONE_ORIGINAL_RENDERER_ORACLE=1` together with either
`RECOMPONE_GRAPHICS_PRESET=Original` or `RECOMPONE_GPU_HLE=0`.
All ordinary menu changes persist in `settings.json`.
