/* load_renderer.c -- LOAD.DLL renderer-adjacent helpers (OUT OF SCOPE).
 *
 * Per CLAUDE.md, the renderer is to be rewritten, not decompiled.
 * LOAD.DLL is the on-disc level/asset loader, but it also bundles a
 * small set of per-frame renderer helpers that build GPU primitive
 * lists and walk vertex tables. These are stubbed here for reference;
 * the rewritten renderer will provide equivalent functionality.
 *
 * Functions covered (all FUN_xxxx at the LOAD.DLL VA):
 *
 *   0x801006f0  Vertex_BuildPrimList
 *               Walks a 0x40-byte-per-entry primitive table emitting
 *               GPU commands (TRIPOLY, QUADPOLY, TEXTRIPOLY) into the
 *               output OT. Used by the splash/legal/loading screens.
 *
 *   0x8010131c  Prim_EmitWithCallback
 *               Same primitive walker but takes a callback to invoke
 *               per vertex; used for warp animations on splash text.
 *
 *   0x8010246c  Prim_EmitFlat
 *               Flat-shaded variant -- writes color into OT cells
 *               using shared color word at +0x17.
 *
 *   0x80102bd4  Prim_EmitGouraud
 *               Gouraud-shaded variant with per-vertex color words.
 *
 *   0x801036bc  Prim_FreeListCell
 *               Returns a primitive cell to the small-pool free list
 *               at _DAT_80065bd0 (8-byte head with size limit 0x100000).
 *
 *   0x801039bc  Sprite_LayoutText
 *               Walks a string + glyph-table, emitting one sprite per
 *               glyph at successive X positions; used by the LOAD
 *               progress message ("LOADING..." with the spinner).
 *
 *   0x80104550  Mdec_DecodeBlock          [MDEC video, out of scope]
 *   0x80104d1c  Mdec_QueueStream           [MDEC video, out of scope]
 *   0x80105060  Mdec_FrameAdvance          [MDEC video, out of scope]
 *
 *   0x801051c8  Scene_WalkLevelOfDetail
 *               Two-loop visitor walking _DAT_80065bd8[_DAT_80065bc4]
 *               (LOD cells) and _DAT_80065bc0 (per-object draw count).
 *               Originally body is intentionally empty after pass 1
 *               iteration of LOD pyramid -- a stat counter / cache
 *               primer with no observable effect on physics or game
 *               state. Safe to no-op in the rewritten renderer.
 *
 *   0x801057f0  Scene_PushDrawState
 *               Sets up the active draw-state cell for the next
 *               primitive emission (texture page, semi-transparency,
 *               dither mode).
 *
 * These functions do not affect physics, gameplay state, or RNG.
 * The rewritten renderer can provide drop-in replacements for the
 * primitive-emitter ones; the MDEC slots can be left as no-ops if
 * the rewrite uses a different video codec or simply skips intro
 * videos.
 *
 * The seam contract:
 *   - All renderer state lives in DAT_80065bc0..DAT_80065be0 (a small
 *     fixed pool referenced as the "GPU command builder" working set).
 *   - The OT chain is shared via DAT_8006f680/6c0 in the main EXE.
 *   - V8_MainLoop calls Render_BuildLists (FUN_80021600) and
 *     Render_PointCameraAt (FUN_8001db24) -- these are also
 *     renderer-adjacent and stubbed in src/gameplay/main_loop.c.
 */
