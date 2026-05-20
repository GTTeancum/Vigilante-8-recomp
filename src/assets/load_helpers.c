/* load_helpers.c -- small leaf helpers from LOAD.DLL.
 *
 * Source: LOAD.DLL
 *   FUN_801001ec  -- IffNode_Link: link a freshly built FORM node into
 *                    a parent's child chain. Stops if the kind field
 *                    is 2 (sentinel) and otherwise normalises kind to 1.
 *   FUN_8010063c  -- Map_ReadRegion: reads a 14-byte map-region record
 *                    (x,y, x2-x1+1, y2-y1+1, attr1, attr2) from the
 *                    map stream and links into the global region list.
 *   FUN_801061c4  -- DrawPrim_SetColor: rewrite the 24-bit colour in a
 *                    GP0 primitive header (keep the upper command byte).
 *   FUN_801064ac  -- DMA_RegisterCallback0
 *   FUN_801064d0  -- DMA_RegisterCallback1
 *   FUN_80106414  -- empty MDEC stub
 *   FUN_801060ac  -- empty MDEC stub
 *   FUN_801060d0  -- empty MDEC stub
 *   FUN_80106434  -- MDEC_StatusBit_29 (bit 29 of $v0 -- output FIFO ready)
 *   FUN_80106470  -- MDEC_StatusBit_24 (bit 24 of $v0 -- output queue empty)
 *
 * HIGH on each (every one is < 100 bytes).
 */
#include <stdint.h>

extern void *Heap_AllocOrRetry(uint32_t n);
extern int16_t XobfStream_ReadI16(void *st);    /* func_0x800224b4 */
extern void DMACallback(int ch, void (*cb)(int));

extern void **_DAT_80065aa8;
extern uint8_t DAT_80065aa4[];

void IffNode_Link(int *parent, int *child)
{
    int kind = *parent;
    if (kind == 1) return;
    if (kind == 2) return;
    if (kind != 0) parent = (int *)1;    /* sentinel-handling artifact */
    void *tail = (void *)parent[3];
    parent[3] = (int)child;
    *(int *)tail = (int)child;
    child[1] = (int)tail;
    *child   = (int)(parent + 2);
}

/* HIGH: 14-byte map region record:
 *   +0x0c i16 x         +0x0e i16 y          (origin)
 *   +0x10 i16 w         +0x12 i16 h          (rect size, inclusive +1)
 *   +0x08 i16 attrA     +0x0a i16 attrB      (palette / icon ids)
 * Then linked into the global region list at DAT_80065aa4.
 */
void *Map_ReadRegion(void *stream)
{
    uint16_t *r = (uint16_t *)Heap_AllocOrRetry(0x14);
    int16_t x1, y1, x2, y2;
    r[3 * 2 + 0] = (uint16_t)(x1 = XobfStream_ReadI16(stream));   /* x  @ +0x0c */
    r[3 * 2 + 1] = (uint16_t)(y1 = XobfStream_ReadI16(stream));   /* y  @ +0x0e */
    x2 = XobfStream_ReadI16(stream);  r[4 * 2 + 0] = (uint16_t)((x2 - x1) + 1);  /* w @+0x10 */
    y2 = XobfStream_ReadI16(stream);  r[4 * 2 + 1] = (uint16_t)((y2 - y1) + 1);  /* h @+0x12 */
    (void)XobfStream_ReadI16(stream); /* discarded i16 */
    r[2 * 2 + 0] = (uint16_t)XobfStream_ReadI16(stream);          /* attrA @ +0x08 */
    r[2 * 2 + 1] = (uint16_t)XobfStream_ReadI16(stream);          /* attrB @ +0x0a */
    /* Tail-link insertion. */
    uint32_t **head = (uint32_t **)_DAT_80065aa8;
    *_DAT_80065aa8  = (void *)r;
    _DAT_80065aa8   = (void **)r;
    r[2 * 2 + 2]    = (uint16_t)(uintptr_t)head;
    *(uint32_t *)r  = (uint32_t)(uintptr_t)DAT_80065aa4;
    return r;
}

void DrawPrim_SetColor(uint32_t *prim, uint32_t rgb24)
{
    *prim = (*prim & 0xff000000u) | (rgb24 & 0x00ffffffu);
}

void DMA_RegisterCallback0(void (*cb)(int)) { DMACallback(0, cb); }
void DMA_RegisterCallback1(void (*cb)(int)) { DMACallback(1, cb); }

void MDEC_Empty0(void) { /* @ 0x801060ac */ }
void MDEC_Empty1(void) { /* @ 0x801060d0 */ }
void MDEC_Empty2(void) { /* @ 0x80106414 */ }

uint32_t MDEC_StatusBit_29(uint32_t v0_latched) { return (v0_latched >> 29) & 1u; }
uint32_t MDEC_StatusBit_24(uint32_t v0_latched) { return (v0_latched >> 24) & 1u; }
