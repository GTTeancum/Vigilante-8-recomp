/* object_bone.c -- linked-list and bone-matrix helpers for the
 * engine's parent/child object hierarchy.
 *
 * Source: SLUS_005.10
 *   FUN_8001d4f0  -- Object_AppendChild(parent, child)      (22 instr)
 *   FUN_8001d544  -- Object_PrependChild(parent, child)     (10 instr)
 *   FUN_8001d708  -- Object_InitBoneMatrix(obj)             (18 instr)
 *   FUN_8001b2fc  -- Object_SetupBone(parent, tmpl, child)  (30 instr)
 *
 * Object linked-list layout (byte offsets):
 *   +0x34  i32  nextSibling   -- singly-linked forward chain
 *   +0x38  i32  firstChild    -- head of this object's child list
 *   +0x3c  i32  backLink      -- parent or previous sibling
 *
 * Bone-matrix fields used by SetupBone + InitBoneMatrix:
 *   +0x10  MATRIX  localMatrix   -- 3x3 rotation (built by RotMatrixYXZ_gte)
 *   +0x24  i32[3]  jointOffset   -- mirror of +0x48 (updated by InitBoneMatrix)
 *   +0x40  SVECTOR eulerAngles   -- YXZ euler in Q12 degrees
 *   +0x44  i16     eulerPad
 *   +0x48  i32[3]  boneLength    -- world-space bone vector
 *
 * HIGH: all four verified line-for-line against MIPS.
 */
#include <stdint.h>
#include <string.h>
#include "structs.h"

extern void RotMatrixYXZ_gte(const SVECTOR *r, MATRIX *m);

/* ---- Object_AppendChild -------------------------------------------- */
/*
 * HIGH: insert child at the TAIL of parent's sibling list.
 *
 * Matches MIPS FUN_8001d4f0: if parent[0x38]==0 set directly; otherwise
 * walk the [0x34] chain to find the last node and append.
 */
void FUN_8001d4f0(uint32_t *parent, uint32_t *child)
{
    uint8_t *p = (uint8_t *)parent;
    uint8_t *c = (uint8_t *)child;

    uint32_t head = *(uint32_t *)(p + 0x38);
    if (head == 0) {
        *(uint32_t *)(p + 0x38) = (uint32_t)(uintptr_t)child;
        *(uint32_t *)(c + 0x3c) = (uint32_t)(uintptr_t)parent;
        return;
    }
    /* Walk to the tail. */
    uint8_t *node = (uint8_t *)(uintptr_t)head;
    uint32_t next = *(uint32_t *)(node + 0x34);
    while (next != 0) {
        node = (uint8_t *)(uintptr_t)*(uint32_t *)(node + 0x34);
        next = *(uint32_t *)(node + 0x34);
    }
    *(uint32_t *)(node + 0x34) = (uint32_t)(uintptr_t)child;
    *(uint32_t *)(c    + 0x3c) = (uint32_t)(uintptr_t)node;
}

/* ---- Object_PrependChild ------------------------------------------- */
/*
 * HIGH: insert child at the HEAD of parent's sibling list.
 *
 * Matches MIPS FUN_8001d544.
 */
void FUN_8001d544(uint32_t *parent, uint32_t *child)
{
    uint8_t *p = (uint8_t *)parent;
    uint8_t *c = (uint8_t *)child;

    uint32_t old_head = *(uint32_t *)(p + 0x38);
    *(uint32_t *)(p + 0x38) = (uint32_t)(uintptr_t)child;
    *(uint32_t *)(c + 0x3c) = (uint32_t)(uintptr_t)parent;
    *(uint32_t *)(c + 0x34) = old_head;
    if (old_head != 0) {
        uint8_t *old = (uint8_t *)(uintptr_t)old_head;
        *(uint32_t *)(old + 0x3c) = (uint32_t)(uintptr_t)child;
    }
}

/* ---- Object_InitBoneMatrix ----------------------------------------- */
/*
 * HIGH: mirror bone-length to joint-offset and build local rotation
 * matrix via RotMatrixYXZ_gte.
 *
 * Matches MIPS FUN_8001d708.
 */
void FUN_8001d708(uint32_t *obj)
{
    uint8_t *b = (uint8_t *)obj;

    *(uint32_t *)(b + 0x24) = *(uint32_t *)(b + 0x48);
    *(uint32_t *)(b + 0x28) = *(uint32_t *)(b + 0x4c);
    *(uint32_t *)(b + 0x2c) = *(uint32_t *)(b + 0x50);

    RotMatrixYXZ_gte((const SVECTOR *)(b + 0x40), (MATRIX *)(b + 0x10));
}

/* ---- Object_SetupBone ---------------------------------------------- */
/*
 * HIGH: copy bone template into child and wire into parent's hierarchy.
 *
 * Matches MIPS FUN_8001b2fc.
 * Template layout (bytes):
 *   [+0x04..+0x0c] i32[3]  bone-length   → child[+0x48..+0x50]
 *   [+0x10..+0x13] u8[4]   euler YXZ     → child[+0x40..+0x43]  (unaligned)
 *   [+0x14]        i16     euler pad      → child[+0x44]
 */
void FUN_8001b2fc(uint32_t *parent, const void *bone_template, uint32_t *child)
{
    const uint8_t *tmpl = (const uint8_t *)bone_template;
    uint8_t       *c    = (uint8_t *)child;

    /* Unaligned 4-byte copy of euler angles (MIPS LWL/LWR). */
    uint32_t euler_word;
    memcpy(&euler_word, tmpl + 0x10, sizeof(uint32_t));
    memcpy(c + 0x40,   &euler_word, sizeof(uint32_t));

    *(int16_t *)(c + 0x44) = *(const int16_t *)(tmpl + 0x14);

    *(uint32_t *)(c + 0x48) = *(const uint32_t *)(tmpl + 0x04);
    *(uint32_t *)(c + 0x4c) = *(const uint32_t *)(tmpl + 0x08);
    *(uint32_t *)(c + 0x50) = *(const uint32_t *)(tmpl + 0x0c);

    FUN_8001d708(child);
    FUN_8001d4f0(parent, child);
}

/* ============================================================
 * // GHIDRA REF (audit ground truth — DO NOT EDIT MANUALLY)
 * ============================================================ */
#if 0

/* FUN_8001d4f0: void(int,int) -- tail-insert child into parent[0x38] list. */
/* FUN_8001d544: void(int,int) -- head-insert child into parent[0x38] list. */
/* FUN_8001d708: void(int)     -- copy +0x48->+0x24; RotMatrixYXZ_gte(+0x40,+0x10). */
/* FUN_8001b2fc: void(p1,p2,p3) -- memcpy template fields; InitBoneMatrix; AppendChild. */

#endif  /* GHIDRA REF */
