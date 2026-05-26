/* host_terrain.c -- parse a V8 .EXP file in memory and populate the
 * engine's DAT_800911a0 chunk-pointer table.
 *
 * Approach: use Asset_LoadFile (host fopen-backed) to pull the .EXP
 * into the engine heap, walk the IFF FORM tree in-memory (no need for
 * Stream_Read), find ZMAP + ZONE chunks, fix up the chunk-pointer
 * table so the engine's Terrain_HeightAt works.
 *
 * On-disc layout observed in OILFIELD.EXP:
 *   FORM 'TERR'
 *     'XLSC' x4   (vertex data per terrain sector?)
 *     'HEAD'      (file header)
 *     FORM 'XOBF' (mesh / animation data)
 *     ...
 *     'ZMAP' 0x800 bytes  (32 x 32 x u16: chunk index per world tile)
 *     'ZONE' 0x4000 bytes (6 of them: heightmap chunks)
 *     'AIMP', 'XBGM', etc (other systems)
 *
 * The engine's cleaned Terrain_HeightAt expects:
 *   chunkBase = DAT_800911a0[chunk_x * 32 + chunk_z]
 *   h = *(u16*)(chunkBase + (cell_x << 7) | (cell_z << 1)) & 0x7ff
 *
 * So we need to point each DAT_800911a0[i] entry at the right ZONE
 * chunk's body, indexed by the ZMAP lookup.
 *
 * On disc, a ZONE is 0x4000 bytes: 64x64 cells, 4 bytes per source
 * cell. LOAD.DLL expands each one into the engine's 0x3000-byte runtime
 * chunk:
 *   [0x0000..0x1fff] 64x64 u16 heights
 *   [0x2000..0x2fff] 64x64 u8 material ids
 * The conversion is preserved below from LOAD.DLL @ 0x801057f0.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern void *Asset_LoadFile(const char *path);
extern void *Heap_Alloc(uint32_t n);
extern void *Heap_AllocOrRetry(uint32_t n);
extern uintptr_t iRam000006ec;
extern uintptr_t uRam000006ec;
extern void *_DAT_800659f0;
extern uint32_t _DAT_800659e8;
extern uint8_t DAT_8008f020[0x2000];

/* Engine globals we'll populate. The cleaned terrain_height.c declares
 * this extern; we provide the actual storage. 1024 entries, each a
 * pointer-sized address; 0 means empty/sky tile. */
uintptr_t DAT_800911a0[32 * 32];

/* Host-side renderer needs to walk populated tiles. Expose them. */
int      g_terrain_loaded = 0;
uint8_t  g_terrain_tile_x_min = 0, g_terrain_tile_x_max = 0;
uint8_t  g_terrain_tile_z_min = 0, g_terrain_tile_z_max = 0;
int      g_terrain_xbmp_w = 0, g_terrain_xbmp_h = 0;

typedef struct {
    uint8_t valid;
    uint16_t u[4], v[4];
} HostTerrainMaterialRender;

HostTerrainMaterialRender g_terrain_material_render[256];

/* Original LOAD.DLL calls Terrain_InitFlatWorld before it loads any ZONE
 * chunks. That routine fills every DAT_800911a0 slot with a valid 0x3000-byte
 * flat chunk, then the level ZMAP replaces only detailed chunks. Keep the
 * same behavior on host: non-ZMAP terrain is still solid, renderable ground. */
static uint8_t g_flat_terrain_chunk[0x3000];
static uint8_t *g_zone_runtime_chunks[512];

/* IFF helpers -- big-endian u32 size. */
static uint32_t be32(const uint8_t *p) {
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16
         | (uint32_t)p[2] <<  8 | (uint32_t)p[3];
}
static uint16_t be16(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0] << 8) | (uint16_t)p[1];
}
static int is_form_tag(const uint8_t *p) {
    return p[0]=='F' && p[1]=='O' && p[2]=='R' && p[3]=='M';
}
static int tag_eq(const uint8_t *p, const char *s) {
    return p[0]==s[0] && p[1]==s[1] && p[2]==s[2] && p[3]==s[3];
}

/* Walk to first chunk matching `tag` (4cc). Returns pointer to its
 * payload, *out_size set to the chunk size. NULL if not found. */
static const uint8_t *find_chunk(const uint8_t *data, uint32_t size,
                                 const char *tag, uint32_t *out_size)
{
    const uint8_t *p   = data;
    const uint8_t *end = data + size;
    while (p + 8 <= end) {
        uint32_t csize = be32(p + 4);
        const uint8_t *body = p + 8;
        if (body + csize > end) break;
        if (is_form_tag(p)) {
            /* Descend into FORM (skip its 4-byte type field). */
            const uint8_t *r = find_chunk(body + 4, csize - 4, tag, out_size);
            if (r) return r;
        } else if (tag_eq(p, tag)) {
            *out_size = csize;
            return body;
        }
        p = body + ((csize + 1) & ~1u);
    }
    return NULL;
}

/* Collect up to `max` chunks matching `tag`. Returns count found. */
static int collect_chunks(const uint8_t *data, uint32_t size,
                          const char *tag,
                          const uint8_t **out_bodies, uint32_t *out_sizes, int max)
{
    int count = 0;
    const uint8_t *p   = data;
    const uint8_t *end = data + size;
    while (p + 8 <= end && count < max) {
        uint32_t csize = be32(p + 4);
        const uint8_t *body = p + 8;
        if (body + csize > end) break;
        if (is_form_tag(p)) {
            count += collect_chunks(body + 4, csize - 4, tag,
                                    out_bodies + count, out_sizes + count, max - count);
        } else if (tag_eq(p, tag)) {
            out_bodies[count] = body;
            out_sizes[count]  = csize;
            count++;
        }
        p = body + ((csize + 1) & ~1u);
    }
    return count;
}

/* HIGH: LOAD.DLL FUN_801057f0 ZONE expansion.
 * Source cell layout is 4 bytes. The runtime chunk layout is the same
 * DAT_800911a0 format consumed by Terrain_HeightAt/Terrain_MaterialAt. */
static uint8_t *convert_zone_to_runtime_chunk(const uint8_t *zone, uint32_t zone_size)
{
    if (zone_size < 0x4000) return NULL;
    uint8_t *dst = (uint8_t *)malloc(0x3000);
    if (!dst) return NULL;
    for (int row = 0; row < 64; row++) {
        for (int col = 0; col < 64; col++) {
            int src_off = (row * 64 + col) * 4;
            uint16_t src = (uint16_t)zone[src_off + 0]
                         | (uint16_t)((uint16_t)zone[src_off + 1] << 8);
            uint16_t h = (uint16_t)(((src >> 8) | (src << 8)) - 0x0200);
            h = (uint16_t)(h | (uint16_t)((zone[src_off + 2] >> 3) << 11));
            *(uint16_t *)(dst + row * 0x80 + col * 2) = h;
            dst[0x2000 + row * 0x40 + col] = zone[src_off + 3];
        }
    }
    return dst;
}

/* HIGH-MED: LOAD.DLL FUN_80105550 TINF material table, physics subset.
 * Runtime DAT_8008f020 is 256 records * 0x20.  The first half of each record
 * is renderer texture/color state; driving physics consumes the seven swapped
 * halfwords copied to record +0x10..+0x1c. */
static void load_tinf_materials(const uint8_t *tinf, uint32_t tinf_size)
{
    memset(DAT_8008f020, 0, 0x2000);
    memset(g_terrain_material_render, 0, sizeof(g_terrain_material_render));
    uint32_t n = tinf_size / 0x28u;
    if (n > 256u)
        n = 256u;
    for (uint32_t i = 0; i < n; i++) {
        const uint8_t *src = tinf + i * 0x28u;
        uint8_t *dst = DAT_8008f020 + i * 0x20u;
        uint16_t base_u = be16(src + 2);
        uint16_t base_v = be16(src + 4);
        for (uint32_t j = 0; j < 7; j++) {
            uint16_t v = be16(src + (4u + j) * 2u);
            *(uint16_t *)(dst + 0x10u + j * 2u) = v;
        }
        *(uint16_t *)(dst + 0x1eu) = (uint16_t)((be16(src + 6) >> 11) & 1u);

        g_terrain_material_render[i].valid = 1;
        g_terrain_material_render[i].u[0] = base_u;
        g_terrain_material_render[i].v[0] = base_v;
        g_terrain_material_render[i].u[1] = (uint16_t)(base_u + 48u);
        g_terrain_material_render[i].v[1] = base_v;
        g_terrain_material_render[i].u[2] = base_u;
        g_terrain_material_render[i].v[2] = (uint16_t)(base_v + 48u);
        g_terrain_material_render[i].u[3] = (uint16_t)(base_u + 48u);
        g_terrain_material_render[i].v[3] = (uint16_t)(base_v + 48u);
    }
    fprintf(stderr, "v8: TINF materials loaded -- %u records\n", (unsigned)n);
}

static int psx_pixel_width_from_words(int words, int depth)
{
    if (depth == 0) return words * 4;
    if (depth == 1) return words * 2;
    return words;
}

static void load_xbmp_meta(const uint8_t *xbmp, uint32_t xbmp_size)
{
    g_terrain_xbmp_w = 0;
    g_terrain_xbmp_h = 0;
    if (xbmp == NULL || xbmp_size < 0x220)
        return;
    uint32_t flags = (uint32_t)xbmp[4] | ((uint32_t)xbmp[5] << 8)
                   | ((uint32_t)xbmp[6] << 16) | ((uint32_t)xbmp[7] << 24);
    uint32_t image_off = (uint32_t)xbmp[8] | ((uint32_t)xbmp[9] << 8)
                       | ((uint32_t)xbmp[10] << 16) | ((uint32_t)xbmp[11] << 24);
    int depth = (int)(flags & 3u);
    if (image_off + 0x14 > xbmp_size)
        return;
    int words = (int)(int16_t)((uint16_t)xbmp[image_off + 0x10]
                             | ((uint16_t)xbmp[image_off + 0x11] << 8));
    int h = (int)(int16_t)((uint16_t)xbmp[image_off + 0x12]
                         | ((uint16_t)xbmp[image_off + 0x13] << 8));
    g_terrain_xbmp_w = psx_pixel_width_from_words(words, depth);
    g_terrain_xbmp_h = h;
    fprintf(stderr, "v8: XBMP terrain texture meta -- %dx%d depth=%d\n",
            g_terrain_xbmp_w, g_terrain_xbmp_h, depth);
}

/* Public: load + parse + wire-up. Path is a V8-style "TRACK\\Foo.TER";
 * we map it to the real input/TERRAIN/Foo.EXP via Asset_LoadFile's
 * translation table once we add one, but for now feed the EXP path
 * directly. Returns 0 on success, -1 on error. */
int Host_TerrainLoad(const char *exp_path)
{
    for (int i = 0; i < 512; i++) {
        free(g_zone_runtime_chunks[i]);
        g_zone_runtime_chunks[i] = NULL;
    }

    for (int i = 0; i < 0x2000; i += 2) {
        g_flat_terrain_chunk[i + 0] = 0xff;
        g_flat_terrain_chunk[i + 1] = 0x45;
    }
    memset(g_flat_terrain_chunk + 0x2000, 0, 0x1000);
    for (int i = 0; i < 32 * 32; i++)
        DAT_800911a0[i] = (uintptr_t)g_flat_terrain_chunk;

    void *blob = Asset_LoadFile(exp_path);
    if (!blob) {
        fprintf(stderr, "v8: Host_TerrainLoad(%s) -- load failed\n", exp_path);
        return -1;
    }
    /* Asset_LoadFile returns the raw file contents. The first 12 bytes
     * are FORM, BE-size, formType (e.g. "TERR"). We skip the outer
     * FORM header and walk its body. */
    const uint8_t *data = (const uint8_t *)blob;
    if (!is_form_tag(data)) {
        fprintf(stderr, "v8: Host_TerrainLoad -- not a FORM file\n");
        return -1;
    }
    uint32_t form_size = be32(data + 4);
    const uint8_t *form_body = data + 8 + 4;       /* skip "FORM",size,4cc */
    uint32_t form_inner_size = form_size - 4;

    /* Find ZMAP (32x32 u16 chunk-index table). */
    uint32_t zmap_size = 0;
    const uint8_t *zmap = find_chunk(form_body, form_inner_size, "ZMAP", &zmap_size);
    if (!zmap || zmap_size != 0x800) {
        fprintf(stderr, "v8: ZMAP not found or wrong size (%u)\n", zmap_size);
        return -1;
    }

    uint32_t tinf_size = 0;
    const uint8_t *tinf = find_chunk(form_body, form_inner_size, "TINF", &tinf_size);
    if (tinf && tinf_size >= 0x28) {
        load_tinf_materials(tinf, tinf_size);
    } else {
        memset(DAT_8008f020, 0, 0x2000);
        memset(g_terrain_material_render, 0, sizeof(g_terrain_material_render));
        fprintf(stderr, "v8: TINF materials not found\n");
    }

    {
        uint32_t xbmp_size = 0;
        const uint8_t *xbmp = find_chunk(form_body, form_inner_size, "XBMP", &xbmp_size);
        load_xbmp_meta(xbmp, xbmp_size);
    }

    /* LOAD.DLL FUN_801005e8 raw-copies AIMP and exposes it via the
     * GP+0x6ec quadtree-root alias used by FUN_800244c4/FUN_80024d54. */
    uint32_t aimp_size = 0;
    const uint8_t *aimp = find_chunk(form_body, form_inner_size, "AIMP", &aimp_size);
    if (aimp && aimp_size >= 10) {
        _DAT_800659f0 = Heap_AllocOrRetry(aimp_size);
        memcpy(_DAT_800659f0, aimp, aimp_size);
        _DAT_800659e8 = aimp_size;
        iRam000006ec = (uintptr_t)_DAT_800659f0;
        uRam000006ec = iRam000006ec;
        fprintf(stderr, "v8: AIMP navigation loaded -- %u bytes at %p\n",
                (unsigned)_DAT_800659e8, _DAT_800659f0);
    } else {
        iRam000006ec = 0;
        uRam000006ec = 0;
        _DAT_800659f0 = NULL;
        _DAT_800659e8 = 0;
        fprintf(stderr, "v8: AIMP navigation not found\n");
    }

    /* Collect ZONE chunks (heightmaps). */
    const uint8_t *zones[512];
    uint32_t       zone_sizes[512];
    int n_zones = collect_chunks(form_body, form_inner_size, "ZONE",
                                 zones, zone_sizes, 512);
    if (n_zones == 0) {
        fprintf(stderr, "v8: no ZONE chunks found\n");
        return -1;
    }
    for (int i = 0; i < n_zones; i++) {
        g_zone_runtime_chunks[i] = convert_zone_to_runtime_chunk(zones[i], zone_sizes[i]);
        if (!g_zone_runtime_chunks[i]) {
            fprintf(stderr, "v8: ZONE[%d] conversion failed\n", i);
            return -1;
        }
    }

    /* Populate DAT_800911a0 from the ZMAP. ZMAP cells are BIG-ENDIAN
     * u16 indices: 0 = empty/sky, N >= 1 = ZONE[N-1].
     *
     * File rows map to engine chunk-Z and file columns map to engine chunk-X.
     * LOAD.DLL copies HEAD +0x08 into object posX and +0x10 into posZ, and
     * JUNC calls Terrain_HeightAt(first_i32, second_i32). Keeping ZMAP in the
     * same X/Z convention prevents the level from being transposed. */
    int populated = 0;
    uint8_t cx_min = 0xff, cx_max = 0, cz_min = 0xff, cz_max = 0;
    for (int file_row = 0; file_row < 32; file_row++) {
        for (int file_col = 0; file_col < 32; file_col++) {
            uint32_t idx = ((uint32_t)zmap[(file_row * 32 + file_col) * 2 + 0] << 8)
                          | (uint32_t)zmap[(file_row * 32 + file_col) * 2 + 1];
            if (idx == 0 || (int)idx > n_zones) {
                continue;
            }
            int cx = file_col;
            int cz = file_row;
            DAT_800911a0[cx * 32 + cz] = (uintptr_t)g_zone_runtime_chunks[idx - 1];
            populated++;
            if (cx < cx_min) cx_min = (uint8_t)cx;
            if (cx > cx_max) cx_max = (uint8_t)cx;
            if (cz < cz_min) cz_min = (uint8_t)cz;
            if (cz > cz_max) cz_max = (uint8_t)cz;
        }
    }
    g_terrain_loaded = (populated > 0);
    g_terrain_tile_x_min = cx_min;
    g_terrain_tile_x_max = cx_max;
    g_terrain_tile_z_min = cz_min;
    g_terrain_tile_z_max = cz_max;
    fprintf(stderr, "v8: terrain populated -- %d of 1024 tiles, "
            "cx [%d..%d], cz [%d..%d]\n",
            populated, cx_min, cx_max, cz_min, cz_max);
    return 0;
}
