/* terrain_visibility.c -- predicate "is most of the terrain visible?".
 *
 * Source: SLUS_005.10  FUN_800220d4.
 *
 * Walks the terrain kd-tree (iRam000006fc) with the callback at
 * LAB_80022044 (a small leaf visitor that increments a counter for
 * each visited leaf and a second counter for each whose chunk pair
 * is currently fully resident in RAM). Returns true if the resident
 * ratio is at least 50% (visited <= resident*2).
 *
 * Used by the AI / quest dispatcher to decide whether to spawn new
 * objects this tick, or wait for more terrain to stream in.
 *
 * HIGH on shape; MED on the counter meaning until LAB_80022044 is
 * promoted.
 */
#include <stdint.h>

extern uintptr_t iRam000006fc;
extern void Tree_Apply(void *root, void (*visit)(void *node, int32_t *ctx), int32_t *ctx);  /* FUN_8002123c */
extern void  TerrainVis_LeafVisitor(void *node, int32_t *ctx);   /* LAB_80022044 */
extern void *V8_MemSet(void *dst, int v, int n);

int Terrain_IsMostlyResident(void)
{
    int32_t ctx[2] = { 0, 0 };
    V8_MemSet(ctx, 0, 8);
    Tree_Apply((void *)iRam000006fc, TerrainVis_LeafVisitor, ctx);
    return ctx[0] <= (ctx[1] << 1);
}
