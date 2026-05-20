/* bone_lookup.c -- per-template "bone" index lookup.
 *
 * Source: SLUS_005.10  FUN_8001afa0.
 *
 * Templates store an array of 0x1c-byte "bone" records starting at
 * template+0x1c. Each record has a u16 kind/typeId at +0x00 and a u16
 * next-link at +0x0c*2 (24). FUN_8001afa0 chases the next-link list
 * (rooted at param_2) until it finds a record whose kind matches
 * `targetKind`, or returns NULL.
 *
 * The 0xffff terminator marks end-of-chain.
 *
 * HIGH confidence.
 */
#include <stdint.h>
#include <stddef.h>

int16_t *Bone_FindByKind(int template, uint16_t headIdx, int16_t targetKind)
{
    while (headIdx != 0xffff) {
        int16_t *rec = (int16_t *)(intptr_t)(template + headIdx * 0x1c + 0x1c);
        if (rec[0] == targetKind) return rec;
        headIdx = (uint16_t)rec[12];
    }
    return NULL;
}
