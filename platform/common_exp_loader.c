/* common_exp_loader.c -- host backing for SLUS FUN_800227a4.
 *
 * The original function clears DAT_800737a0, then walks Common.exp's 16
 * XOBF entries.  The bitmask argument selects which entries are parsed.
 * Entries 13, 14, and 15 alias the named globals at 0x800737d4/d8/dc.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void *Heap_AllocOrRetry(uint32_t n);
extern uint32_t *Object_BuildFromBin(int *templateBody, void *animPtr);

extern uintptr_t DAT_800737a0[32];
extern uintptr_t DAT_800737d4;
extern uintptr_t DAT_800737d8;
extern uintptr_t _DAT_800737d8;
extern uintptr_t DAT_800737dc;
extern void     *puRam000007d0;
extern void     *puRam000007d4;
extern int32_t **piRam0000076c;
extern int32_t  *piRam00000774;
extern int32_t  *piRam0000075c;
extern int32_t  *piRam0000077c;
extern int32_t  *piRam000007bc;
extern int32_t  *piRam00000714;
extern void     *puRam000007c4;
extern uint8_t   DAT_80065a18[];
extern uint8_t   DAT_80065a50[];
extern uint8_t   DAT_80065a60[];
extern uint8_t   DAT_80065a80[];
extern uint8_t   DAT_80065aa0[];
extern uint8_t   DAT_80065ac0[];
extern void      ObjList_HostResetPool(void);
extern void      ObjectEventQueue_HostReset(void);

typedef struct HostObjListNode {
    struct HostObjListNode *next;
    struct HostObjListNode *prev;
    uintptr_t payload;
    uint32_t deadline;
} HostObjListNode;

static void host_list_clear(uint8_t *head)
{
    HostObjListNode *sentinel = (HostObjListNode *)head;
    sentinel->next = NULL;
    sentinel->prev = sentinel;
    sentinel->payload = 0;
    sentinel->deadline = 0;
}

static uint32_t be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16)
         | ((uint32_t)p[2] <<  8) |  (uint32_t)p[3];
}

static int tag_eq(const uint8_t *p, const char *tag)
{
    return p[0] == tag[0] && p[1] == tag[1]
        && p[2] == tag[2] && p[3] == tag[3];
}

static void *heap_copy(const uint8_t *src, uint32_t n)
{
    void *dst = Heap_AllocOrRetry(n);
    if (dst != NULL) memcpy(dst, src, n);
    return dst;
}

static uintptr_t parse_xobf(const uint8_t *xobf, uint32_t size)
{
    const uint8_t *p = xobf;
    const uint8_t *end = xobf + size;
    void *model = NULL;
    void *anim = NULL;

    while (p + 8 <= end) {
        uint32_t csz = be32(p + 4);
        const uint8_t *body = p + 8;
        if (body + csz > end) break;

        if (tag_eq(p, "BIN ")) {
            model = heap_copy(body, csz);
        } else if (tag_eq(p, "ANM ")) {
            anim = heap_copy(body, csz);
        }
        p = body + csz + (csz & 1u);
    }

    if (model == NULL) return 0;
    return (uintptr_t)Object_BuildFromBin((int *)model, anim);
}

static void sync_common_aliases(void)
{
    DAT_800737d4 = DAT_800737a0[13];
    DAT_800737d8 = DAT_800737a0[14];
    _DAT_800737d8 = DAT_800737a0[14];
    DAT_800737dc = DAT_800737a0[15];
}

static void maybe_store_xobf(int index, uint32_t mask,
                             const uint8_t *body, uint32_t size)
{
    if (index < 0 || index >= 32) return;
    if (((mask >> (index & 31)) & 1u) == 0) return;
    DAT_800737a0[index] = parse_xobf(body, size);
}

static void walk_forms(const uint8_t *data, uint32_t off, uint32_t end,
                       uint32_t mask, int *xobf_index)
{
    while (off + 8 <= end) {
        uint32_t csz = be32(data + off + 4);
        uint32_t body = off + 8;
        if (body + csz > end) break;

        if (tag_eq(data + off, "FORM") && csz >= 4) {
            if (tag_eq(data + body, "XOBF")) {
                maybe_store_xobf((*xobf_index)++, mask,
                                 data + body + 4, csz - 4);
            } else {
                walk_forms(data, body + 4, body + csz, mask, xobf_index);
            }
        } else if (tag_eq(data + off, "XOBF")) {
            maybe_store_xobf((*xobf_index)++, mask, data + body, csz);
        }

        off = body + csz + (csz & 1u);
    }
}

void Audio_BankSelect(uint32_t mask)
{
    memset(DAT_800737a0, 0, sizeof(uintptr_t) * 32);
    sync_common_aliases();

    FILE *f = fopen("COMMON.EXP", "rb");
    if (f == NULL) {
        fprintf(stderr, "v8: Common.exp loader cannot open COMMON.EXP\n");
        return;
    }
    fseek(f, 0, SEEK_END);
    long fsz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (fsz <= 0) {
        fclose(f);
        return;
    }

    uint8_t *raw = (uint8_t *)malloc((size_t)fsz);
    if (raw == NULL) {
        fclose(f);
        return;
    }
    if (fread(raw, 1, (size_t)fsz, f) != (size_t)fsz) {
        free(raw);
        fclose(f);
        return;
    }
    fclose(f);

    int xobf_index = 0;
    walk_forms(raw, 0, (uint32_t)fsz, mask, &xobf_index);
    sync_common_aliases();

    fprintf(stderr,
            "v8: Common.exp mask=0x%08x xobfs=%d d4=%p d8=%p dc=%p\n",
            mask, xobf_index,
            (void *)DAT_800737d4, (void *)DAT_800737d8, (void *)DAT_800737dc);
    free(raw);
}

void FUN_800227a4(uint32_t mask)
{
    ObjList_HostResetPool();
    ObjectEventQueue_HostReset();
    piRam0000076c = NULL;

    host_list_clear(DAT_80065a18);
    host_list_clear(DAT_80065a50);
    host_list_clear(DAT_80065a60);
    host_list_clear(DAT_80065a80);
    host_list_clear(DAT_80065aa0);
    host_list_clear(DAT_80065ac0);

    piRam00000714 = (int32_t *)DAT_80065a18;
    piRam0000075c = (int32_t *)DAT_80065a60;
    piRam0000077c = (int32_t *)DAT_80065a80;
    piRam000007bc = (int32_t *)DAT_80065ac0;
    puRam000007c4 = DAT_80065ac0;
    piRam00000774 = NULL;

    Audio_BankSelect(mask);

    puRam000007d4 = NULL;
    puRam000007d0 = NULL;
}
