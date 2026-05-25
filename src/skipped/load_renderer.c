/* load_renderer.c -- LOAD.DLL renderer-adjacent helpers (OUT OF SCOPE).
 *
 * Per CLAUDE.md, the renderer is to be rewritten, not decompiled.
 * LOAD.DLL is the on-disc level/asset loader, but it also bundles a
 * small set of per-frame renderer helpers that build GPU primitive
 * lists and walk vertex tables. These are stubbed here for reference;
 * the rewritten renderer will provide equivalent functionality.
 *
 * Earlier pass notes misclassified several terrain-loader functions here as
 * primitive/MDEC helpers. Source re-audit corrected those roles:
 *
 *   0x80100148  Terrain_LoadBspNode
 *               Recursively decodes the BSP static-object kd-tree stream.
 *
 *   0x801001ec  Terrain_BspInsertObject
 *               Inserts a placed object-list node into the BSP leaf chosen by
 *               object +0x48/+0x50.
 *
 *   0x801002ac  Terrain_LoadTerrHead
 *               Reads top-level TERR/HEAD setup words and initializes the flat
 *               terrain table before ZMAP/ZONE installation.
 *
 *   0x801005c0  Terrain_LoadBsp
 *               Loads the BSP tree root into the runtime global at
 *               _DAT_80065a00 / uRam000006fc.
 *
 *   0x801005e8  Terrain_LoadAimp
 *               Raw-copies the AIMP navigation quadtree blob.
 *
 *   0x801006f0  Terrain_LoadObjHead
 *               Builds placed OBJ/HEAD runtime objects from source transforms,
 *               object templates, and XOBF slots.
 *
 *   0x80102bd4  Terrain_BuildRouteStrip
 *               Tessellates a Bezier RSEG edge into a terrain-conformed
 *               road/route strip using XRTP width/step and Terrain_HeightAt.
 *
 *   0x801039bc  Terrain_SplitRouteBezier
 *               Splits an RSEG cubic until an endpoint distance constraint is
 *               satisfied.
 *
 *   0x80104550  Terrain_PrepareRouteStrip
 *               Builds RSEG control points from JUNC endpoints, adjusts
 *               endpoint anchors against JUNC patch meshes, then dispatches
 *               route-strip generation.
 *
 *   0x80104d1c  Terrain_LoadJunc
 *               Loads JUNC graph nodes and optional terrain-conformed patch
 *               meshes.
 *
 *   0x80105060  Terrain_LoadRseg
 *               Loads RSEG graph edges and links them into both endpoint JUNC
 *               edge arrays.
 *
 * Remaining renderer-adjacent functions covered (all FUN_xxxx at the LOAD.DLL VA):
 *
 *   0x8010131c  Prim_EmitWithCallback
 *               Same primitive walker but takes a callback to invoke
 *               per vertex; used for warp animations on splash text.
 *
 *   0x8010246c  Prim_EmitFlat
 *               Flat-shaded variant -- writes color into OT cells
 *               using shared color word at +0x17.
 *
 *   0x801036bc  Prim_FreeListCell
 *               Returns a primitive cell to the small-pool free list
 *               at _DAT_80065bd0 (8-byte head with size limit 0x100000).
 *
 *   0x801051c8  Scene_WalkLevelOfDetail
 *               Two-loop visitor walking _DAT_80065bd8[_DAT_80065bc4]
 *               (LOD cells) and _DAT_80065bc0 (per-object draw count).
 *               Originally body is intentionally empty after pass 1
 *               iteration of LOD pyramid -- a stat counter / cache
 *               primer with no observable effect on physics or game
 *               state. Safe to no-op in the rewritten renderer.
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
