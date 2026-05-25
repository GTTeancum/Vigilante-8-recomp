/* host_vehicle_exp_probe.c -- headless Vehicles.exp object-template audit.
 *
 * This is a host-only verification hook for the source-driven XOBF loader
 * path.  It avoids the renderer and CD streamer: read SHELL/VEHICLES.EXP
 * from the native runtime directory, copy each XOBF BIN/ANM into the engine
 * heap, then call Object_BuildFromBin exactly as XOBF_Parse does.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void *Heap_AllocOrRetry(uint32_t n);
extern uint32_t *Object_BuildFromBin(int *templateBody, void *animPtr);
extern void Audio_BankSelect(uint32_t mask);
extern uintptr_t DAT_800737a0[32];
extern uintptr_t DAT_800737d4;
extern uintptr_t DAT_800737d8;
extern uintptr_t DAT_800737dc;
extern int g_v8_vehicle_construct_probe_kind;
extern uint8_t DAT_80065674[];
extern intptr_t Host_TerrainFindPlaceholderById(int spawn_id);
extern intptr_t FUN_80021f30(uint8_t *record);

int g_v8_vehicle_spawn_probe_trace = 0;

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

static int probe_xobf(const uint8_t *xobf, uint32_t size, int index)
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
        p = body + csz + (csz & 1);
    }

    if (model == NULL) {
        fprintf(stderr, "v8: vehicle-template-probe[%d] missing BIN\n", index);
        return 0;
    }

    uint32_t *obj = Object_BuildFromBin((int *)model, anim);
    if (obj == NULL) {
        fprintf(stderr, "v8: vehicle-template-probe[%d] build returned NULL\n", index);
        return 0;
    }

    int *m = (int *)model;
    int ok = obj[0] == (uint32_t)(uintptr_t)model
          && obj[1] == (uint32_t)(uintptr_t)anim
          && m[0] >= 0 && m[0] < 512
          && m[4] >= 0 && m[4] < 512;
    fprintf(stderr,
            "v8: vehicle-template-probe[%02d] obj=%p model=%p groups=%d slots=%d ok=%d\n",
            index, (void *)obj, model, m[0], m[4], ok);
    return ok;
}

static int walk_forms(const uint8_t *data, uint32_t off, uint32_t end,
                      const char parent[4], int *index, int *ok_count)
{
    while (off + 8 <= end) {
        uint32_t csz = be32(data + off + 4);
        uint32_t body = off + 8;
        if (body + csz > end) break;

        if (tag_eq(data + off, "FORM") && csz >= 4) {
            const char *ftype = (const char *)(data + body);
            if (memcmp(ftype, "XOBF", 4) == 0) {
                int ok = probe_xobf(data + body + 4, csz - 4, *index);
                *ok_count += ok != 0;
                (*index)++;
            } else {
                char next_parent[4];
                memcpy(next_parent, ftype, 4);
                walk_forms(data, body + 4, body + csz, next_parent,
                           index, ok_count);
            }
        }
        (void)parent;
        off = body + csz + (csz & 1);
    }
    return 1;
}

int Host_VehicleTemplateProbe(void)
{
    const char *path = "SHELL/VEHICLES.EXP";
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        fprintf(stderr, "v8: vehicle-template-probe cannot open %s\n", path);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long fsz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (fsz <= 0) {
        fclose(f);
        return 1;
    }

    uint8_t *raw = (uint8_t *)malloc((size_t)fsz);
    if (raw == NULL) {
        fclose(f);
        return 1;
    }
    if (fread(raw, 1, (size_t)fsz, f) != (size_t)fsz) {
        free(raw);
        fclose(f);
        return 1;
    }
    fclose(f);

    int count = 0;
    int ok_count = 0;
    char root[4] = {0, 0, 0, 0};
    walk_forms(raw, 0, (uint32_t)fsz, root, &count, &ok_count);
    free(raw);

    fprintf(stderr, "v8: vehicle-template-probe summary count=%d ok=%d\n",
            count, ok_count);
    return (count == 14 && ok_count == 14) ? 0 : 1;
}

int Host_VehicleConstructProbe(void)
{
    Audio_BankSelect(0x0000e001u);
    if (DAT_800737a0[0] == 0 || DAT_800737d4 == 0 ||
        DAT_800737d8 == 0 || DAT_800737dc == 0) {
        fprintf(stderr,
                "v8: vehicle-construct-probe missing common banks "
                "v0=%p d4=%p d8=%p dc=%p\n",
                (void *)DAT_800737a0[0], (void *)DAT_800737d4,
                (void *)DAT_800737d8, (void *)DAT_800737dc);
        return 1;
    }

    fprintf(stderr,
            "v8: vehicle-construct-probe common banks loaded; "
            "constructor spawn root still requires original caller/kind audit "
            "(requested kind=%d)\n",
            g_v8_vehicle_construct_probe_kind);
    return 0;
}

int Host_VehicleSpawnProbe(void)
{
    uint8_t record[6] = { 0xff, 0x00, 0xff, 0xff, 0x00, 0x00 };

    int slot = g_v8_vehicle_construct_probe_kind;
    if (slot < 0 || slot >= 12)
        slot = 0;
    DAT_80065674[0] = (uint8_t)slot;
    Audio_BankSelect(0x0000e000u | (1u << (uint32_t)slot));
    if (DAT_800737a0[slot] == 0) {
        fprintf(stderr, "v8: vehicle-spawn-probe missing Common.exp vehicle bank %d\n",
                slot);
        return 1;
    }
    if (Host_TerrainFindPlaceholderById(-1) == 0) {
        fprintf(stderr, "v8: vehicle-spawn-probe cannot resolve placeholder id -1\n");
        return 1;
    }

    fprintf(stderr, "v8: vehicle-spawn-probe trying slot=%d\n", slot);
    g_v8_vehicle_spawn_probe_trace = 1;
    intptr_t obj = FUN_80021f30(record);
    g_v8_vehicle_spawn_probe_trace = 0;
    if (obj == 0) {
        fprintf(stderr, "v8: vehicle-spawn-probe spawn returned NULL\n");
        return 1;
    }

    fprintf(stderr,
            "v8: vehicle-spawn-probe spawned obj=%p pos=(0x%x,0x%x,0x%x) "
            "flags=0x%x cb=%p\n",
            (void *)obj,
            *(uint32_t *)((uintptr_t)obj + 0x48),
            *(uint32_t *)((uintptr_t)obj + 0x4c),
            *(uint32_t *)((uintptr_t)obj + 0x50),
            *(uint32_t *)(uintptr_t)obj,
            *(void **)((uintptr_t)obj + 0x64));
    return 0;
}
