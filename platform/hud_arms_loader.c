/* hud_arms_loader.c -- host loader for HUD/ARMS.EXP selected weapon bank. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern uint8_t DAT_80065674[];
extern uintptr_t DAT_800737e0;
extern void *Heap_AllocOrRetry(uint32_t n);
extern uint32_t Sound_LoadSNDFromMemory(const void *data, uint32_t len,
                                        const char *label);

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

static uint32_t parse_selected_xobf_snd(const uint8_t *xobf, uint32_t size,
                                        uint32_t selected)
{
    const uint8_t *p = xobf;
    const uint8_t *end = xobf + size;

    while (p + 8 <= end) {
        uint32_t csz = be32(p + 4);
        const uint8_t *body = p + 8;
        if (body + csz > end)
            break;

        if (tag_eq(p, "SND ")) {
            char label[64];
            snprintf(label, sizeof(label), "HUD.ARMS.EXP.XOBF[%u].SND",
                     (unsigned)selected);
            return Sound_LoadSNDFromMemory(body, csz, label);
        }

        p = body + csz + (csz & 1u);
    }

    return 0;
}

static uint32_t walk_forms_for_selected(const uint8_t *data, uint32_t off,
                                        uint32_t end, uint32_t selected,
                                        uint32_t *xobf_index)
{
    while (off + 8 <= end) {
        uint32_t csz = be32(data + off + 4);
        uint32_t body = off + 8;
        uint32_t snd;
        if (body + csz > end)
            break;

        if (tag_eq(data + off, "FORM") && csz >= 4) {
            if (tag_eq(data + body, "XOBF")) {
                uint32_t idx = (*xobf_index)++;
                if (idx == selected) {
                    snd = parse_selected_xobf_snd(data + body + 4,
                                                  csz - 4, selected);
                    if (snd != 0)
                        return snd;
                }
            } else {
                snd = walk_forms_for_selected(data, body + 4, body + csz,
                                              selected, xobf_index);
                if (snd != 0)
                    return snd;
            }
        } else if (tag_eq(data + off, "XOBF")) {
            uint32_t idx = (*xobf_index)++;
            if (idx == selected) {
                snd = parse_selected_xobf_snd(data + body, csz, selected);
                if (snd != 0)
                    return snd;
            }
        }

        off = body + csz + (csz & 1u);
    }

    return 0;
}

void HudArms_LoadSelectedBanks(void)
{
    uint32_t selected = DAT_80065674[0];
    if (selected >= 13)
        selected = 0;

    FILE *f = fopen("HUD\\ARMS.EXP", "rb");
    if (f == NULL)
        f = fopen("HUD/ARMS.EXP", "rb");
    if (f == NULL) {
        fprintf(stderr, "v8: HudArms loader cannot open HUD/ARMS.EXP\n");
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

    uint32_t xobf_index = 0;
    uint32_t snd = walk_forms_for_selected(raw, 0, (uint32_t)fsz,
                                           selected, &xobf_index);
    free(raw);

    if (snd == 0)
        return;

    uint8_t *owner = (uint8_t *)Heap_AllocOrRetry(12);
    if (owner == NULL)
        return;
    memset(owner, 0, 12);
    *(uint32_t *)(owner + 8) = snd;
    DAT_800737e0 = (uintptr_t)owner;
}
