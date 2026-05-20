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
 * ZONE size mystery: 0x4000 vs the engine's 0x2000 expectation. Two
 * possibilities -- (a) each ZONE holds 2 sub-chunks of 0x2000 (the
 * engine reads only the first 0x2000), or (b) the per-cell layout is
 * actually 4 bytes wide.  The cleaned terrain_height.c uses cell
 * stride 2 bytes and row stride 0x80, so 64*64*2 = 0x2000 -- if the
 * ZONE is 0x4000 then half of it is "below" data (attributes? a
 * second layer?). We treat the first 0x2000 of each ZONE as a chunk
 * for now and refine when we see how the data looks.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern void *Asset_LoadFile(const char *path);
extern void *Heap_Alloc(uint32_t n);

/* Engine globals we'll populate. The cleaned terrain_height.c declares
 * this extern; we provide the actual storage. 1024 entries, each a
 * pointer-sized address; 0 means empty/sky tile. */
uintptr_t DAT_800911a0[32 * 32];

/* Host-side renderer needs to walk populated tiles. Expose them. */
int      g_terrain_loaded = 0;
uint8_t  g_terrain_tile_x_min = 0, g_terrain_tile_x_max = 0;
uint8_t  g_terrain_tile_z_min = 0, g_terrain_tile_z_max = 0;

/* IFF helpers -- big-endian u32 size. */
static uint32_t be32(const uint8_t *p) {
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16
         | (uint32_t)p[2] <<  8 | (uint32_t)p[3];
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

/* Public: load + parse + wire-up. Path is a V8-style "TRACK\\Foo.TER";
 * we map it to the real input/TERRAIN/Foo.EXP via Asset_LoadFile's
 * translation table once we add one, but for now feed the EXP path
 * directly. Returns 0 on success, -1 on error. */
int Host_TerrainLoad(const char *exp_path)
{
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

    /* Collect ZONE chunks (heightmaps). */
    const uint8_t *zones[64];
    uint32_t       zone_sizes[64];
    int n_zones = collect_chunks(form_body, form_inner_size, "ZONE",
                                 zones, zone_sizes, 64);
    if (n_zones == 0) {
        fprintf(stderr, "v8: no ZONE chunks found\n");
        return -1;
    }
    fprintf(stderr, "v8: terrain '%s' -- ZMAP @%p, %d ZONE chunk%s\n",
            exp_path, (const void *)zmap, n_zones, n_zones == 1 ? "" : "s");
    for (int i = 0; i < n_zones; i++) {
        fprintf(stderr, "v8:   ZONE[%d] @%p size 0x%x\n",
                i, (const void *)zones[i], zone_sizes[i]);
    }

    /* Populate DAT_800911a0 from the ZMAP. ZMAP cells are BIG-ENDIAN
     * u16 indices: 0 = empty/sky, N >= 1 = ZONE[N-1]. */
    int populated = 0;
    uint8_t cx_min = 0xff, cx_max = 0, cz_min = 0xff, cz_max = 0;
    for (int cx = 0; cx < 32; cx++) {
        for (int cz = 0; cz < 32; cz++) {
            uint32_t idx = ((uint32_t)zmap[(cx * 32 + cz) * 2 + 0] << 8)
                          | (uint32_t)zmap[(cx * 32 + cz) * 2 + 1];
            if (idx == 0 || (int)idx > n_zones) {
                DAT_800911a0[cx * 32 + cz] = 0;
                continue;
            }
            DAT_800911a0[cx * 32 + cz] = (uintptr_t)zones[idx - 1];
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
