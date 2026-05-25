/* radar_minimap.c -- Radar / minimap dot renderer.
 *
 * Source: SLUS_005.10  FUN_8002ad30.
 *
 * OUT OF SCOPE per CLAUDE.md (renderer, will be rewritten).
 *
 * FUN_8002ad30 iterates the active-object list (piRam00000714) and,
 * for each live vehicle, projects its world position into player-relative
 * camera space to compute screen (x, y) offsets.  The result is a set of
 * GPU line-primitive dots on the minimap, emitted via AddPrim() into the
 * ordering table (param_2).
 *
 * CONTRACT (seam document):
 *   param_1: player Vehicle * (self)
 *   param_2: ordering-table pointer (OT) for the current frame
 *   param_3: radar centre X (screen pixels)
 *   param_4: radar centre Y (screen pixels)
 *
 * Scratch buffer: 64 entries × 16 bytes at UNK_800a2bb8.
 * Counter: uRam000003bc (rolling 0..63).
 *
 * Primitives emitted per vehicle:
 *   - colour at UNK_800a2bb8 + entry * 16 + 4:
 *       0x6000ff00 = yellow (current target)
 *       0x600000ff = blue (enemy)
 *       0x60808080 = grey (dead/disabled)
 *   - screen X = radar_centre_x + dot_x (camera-space projection)
 *   - screen Y = radar_centre_y - dot_y
 *
 * The new renderer will reproduce this by drawing 2D dots via its
 * own HUD render layer.
 */

/* Nothing compiled here -- host radar is re-implemented in the renderer. */
