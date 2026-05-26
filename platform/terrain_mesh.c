/* terrain_mesh.c -- load and upload the visual level mesh from the EXP.
 *
 * The terrain EXP (e.g. OILFIELD.EXP) contains a FORM XOBF block with two
 * BIN chunks.  The first BIN is the level visual geometry -- ground, props,
 * buildings, oil derricks, etc.  The second BIN appears to be shadow/LOD data
 * and is skipped for now.
 *
 * The BIN uses the same bone/poly format as the vehicle meshes in mesh_loader.c:
 *   Header:  num_bones(u32 LE), bone_table_off(u32 LE), ...
 *   Bone table: num_bones x u32 offsets (relative to bone_table_off)
 *   Bone desc (+0x00): vert_count, vert_off, norm_count, norm_off,
 *                      poly_count(u16), ?, poly_off(u32)
 *   Vertices: i16 x, y, z, pad  (8 bytes each)
 *   Polygons: same packet format as vehicles
 *
 * Coordinate system:
 *   BIN vertices are in global level-relative space (not bone-local).
 *   Scale: 1 BIN unit = 1/16 metre  (vehicles use 1/160; terrain is 10x coarser)
 *   Y-down (PSX convention): negate for OpenGL Y-up.
 *   Fallback, uninstanced BIN display uses the selected ZMAP footprint centre
 *   as an origin.  Normal runtime display uses FORM OBJ/HEAD placement:
 *     HEAD +0x08 = X (16.16 cells)
 *     HEAD +0x0c = Y (Terrain_HeightAt units, minus 0x100000)
 *     HEAD +0x10 = Z (16.16 cells)
 *     TERR_Y_ORIGIN = -22.5 m for raw fallback BIN Y=0
 *   Ground surface (BIN Y ~= 0) maps to OpenGL Y near the converted
 *   ZONE runtime surface (OilField h ~= 1440 -> -22.5 m).
 *
 *   Full transform:
 *     Runtime render groups carry a descriptor scale byte at +0x18.  Source
 *     render code (SLUS 8001be5c) right-shifts the object MATRIX translation
 *     by 16-scale before sending it to the GTE, so BIN local vertices map to
 *     world X/Z as raw / (1 << scale).  The GTE transform path applies that
 *     same post-shift scale to X/Y/Z; physics keeps its original integer
 *     Terrain_HeightAt height units separately.
 *
 * CPU-side triangle store (TmTri) is populated at load time and retained for
 * the lifetime of the process.  TerrainMesh_HeightAt() queries it to get the
 * ground height at a world-metre XZ position -- this drives vehicle grounding
 * across the full XOBF mesh (not just the 6 ZONE chunks).
 *
 * Display Y conversion (PSX Y-down / OpenGL Y-up):
 *   psxy = (int32_t)(-gl_y * 65536.0f)
 *   gl_y = -psxy / 65536.0f
 * X/Z use 16.16 terrain-cell coordinates (1 display unit per cell).  Display
 * Y uses the same post-GTE scale as X/Z; physics still consumes the original
 * integer Terrain_HeightAt value.
 *
 * BIN coordinate cross-check:
 *   OilField placed XOBF, after HEAD/slot instancing, spans roughly
 *   X=[781..1002], Z=[1119..1283].  The independently decoded runtime
 *   navigation/route data is inside that footprint:
 *   AIMP X=[852..988], Z=[1186..1254].
 *
 * Confirmed: all 165 bones parse with 0 invalid packets using vehicle packet table.
 * Total triangles (OilField): ~4114.
 *
 * Renderer note: all triangles are retained in the CPU store for diagnostics.
 * The GL upload draws every placed triangle so object/occluder placement is
 * visible during RE.  TerrainMesh_HeightAt() still filters the CPU store to
 * upward ground-like faces, so walls and props do not become driveable merely
 * because they are visible in the diagnostic renderer.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include <math.h>
#include "gte.h"
#if defined(V8_HAVE_SDL) && defined(V8_HAVE_GL)
#include "xobf_texture.h"
#endif

/* ------------------------------------------------------------------ */
/* CPU-side triangle store -- always compiled, headless-safe.          */
/* TerrainMesh_HeightAt returns 0 when g_terrain_tris is NULL (not    */
/* yet loaded), so callers fall through to the ZONE fallback.         */
/* ------------------------------------------------------------------ */
typedef struct {
    float ax, ay, az;   /* vertex A (world metres, OpenGL Y-up) */
    float bx, by, bz;   /* vertex B */
    float cx, cy, cz;   /* vertex C */
    float nx, ny, nz;   /* face normal (normalised) */
    float pd;           /* plane const: nx*ax + ny*ay + nz*az */
} TmTri;

static TmTri *g_terrain_tris  = NULL;
static int    g_terrain_ntris = 0;

typedef struct {
    uint8_t obj[0x80];
    int32_t root_x;
    int32_t root_z;
    int next_in_leaf;
} TmObstacleObj;

typedef struct {
    int kind;
    int32_t coord;
    int left;
    int right;
    int first_obj;
    int count;
} TmObjectBspNode;

static TmObstacleObj *g_obstacle_objs = NULL;
static int            g_obstacle_nobjs = 0;
static uint8_t       *g_obstacle_raw = NULL;
static uint8_t       *g_spawn_placeholders = NULL;
static int            g_spawn_nplaceholders = 0;
static TmObjectBspNode *g_object_bsp = NULL;
static int              g_object_bsp_nnodes = 0;
static int              g_object_bsp_root = -1;

extern void  Heap_Free(void *p);
extern void RotMatrixYXZ_gte(const SVECTOR *r, MATRIX *m);
extern long CompMatrixLV(const MATRIX *m0, const MATRIX *m1, MATRIX *m2);
extern int32_t Terrain_HeightAt(uint32_t x, uint32_t z);
extern int rsin(int a);
extern int rcos(int a);
extern int Object_FindObstacleAt(int *parent_obj, int terrain_y,
                                 int *posXyz, int16_t *normalOut);

static void tm_clear_obstacles(void)
{
    free(g_obstacle_objs);
    g_obstacle_objs = NULL;
    g_obstacle_nobjs = 0;
    Heap_Free(g_obstacle_raw);
    g_obstacle_raw = NULL;
    free(g_spawn_placeholders);
    g_spawn_placeholders = NULL;
    g_spawn_nplaceholders = 0;
    free(g_object_bsp);
    g_object_bsp = NULL;
    g_object_bsp_nnodes = 0;
    g_object_bsp_root = -1;
}

static int tm_object_bsp_find_leaf(int32_t x, int32_t z)
{
    int node = g_object_bsp_root;
    int guard = 0;
    while (g_object_bsp != NULL && node >= 0 &&
           node < g_object_bsp_nnodes && guard++ < 4096) {
        TmObjectBspNode *n = &g_object_bsp[node];
        if (n->kind == 0) return node;
        if (n->kind == 1) {
            node = (n->coord < x) ? n->right : n->left;
        } else if (n->kind == 2) {
            node = (n->coord < z) ? n->right : n->left;
        } else {
            break;
        }
    }
    return -1;
}

/* Given a world-metre XZ position, return the OpenGL Y of the highest
 * upward-facing terrain/driveable patch at that point.  Returns 1 on hit
 * (out_gl_y valid), 0 if no mesh coverage (caller should use ZONE fallback).
 *
 * Source-driven RE note:
 *   SLUS FUN_8001d748 first samples Terrain_HeightAt, then asks each loaded
 *   object obstacle tree (FUN_8001f51c/FUN_8001ef74) whether a patch overrides
 *   that height.  The terrain EXP supplies those patch/object shapes via the
 *   main XOBF BIN and FORM OBJ/HEAD placement data.  The host renderer does
 *   not yet build the original obstacle trees, so this CPU triangle store is
 *   the runtime stand-in for those sloped driveable patches.
 *
 * Do not filter by a fixed Y band: OilField's real upward terrain patches span
 * roughly gl_y [-64..+73] after placement.  The previous [-28..-17] band kept
 * only 212 of 898 upward triangles and made the map look almost flat/tiny. */
static int tm_tri_is_ground(float ax, float ay, float az,
                            float bx, float by, float bz,
                            float cx, float cy, float cz,
                            float ny)
{
    float area_xz = fabsf((bx - ax) * (cz - az) - (bz - az) * (cx - ax)) * 0.5f;
    return ny >= 0.30f && area_xz >= 1.0f;
}

int TerrainMesh_HeightAt(float wx, float wz, float *out_gl_y)
{
    float best  = -1e30f;
    int   found = 0;
    for (int i = 0; i < g_terrain_ntris; i++) {
        const TmTri *t = &g_terrain_tris[i];
        if (!tm_tri_is_ground(t->ax, t->ay, t->az,
                              t->bx, t->by, t->bz,
                              t->cx, t->cy, t->cz,
                              t->ny)) continue;
        /* Point-in-triangle test in XZ projection.
         * Works for both CW and CCW winding: skip if d's have mixed signs. */
        float d0 = (wx - t->ax)*(t->bz - t->az) - (wz - t->az)*(t->bx - t->ax);
        float d1 = (wx - t->bx)*(t->cz - t->bz) - (wz - t->bz)*(t->cx - t->bx);
        float d2 = (wx - t->cx)*(t->az - t->cz) - (wz - t->cz)*(t->ax - t->cx);
        if ((d0 < 0 || d1 < 0 || d2 < 0) && (d0 > 0 || d1 > 0 || d2 > 0)) continue;
        /* Y via plane equation: nx*x + ny*y + nz*z = pd -> y = (pd - nx*x - nz*z) / ny */
        float y = (t->pd - t->nx * wx - t->nz * wz) / t->ny;
        if (!found || y > best) { best = y; found = 1; }
    }
    if (found) *out_gl_y = best;
    return found;
}

int TerrainMesh_Bounds(float *out_x0, float *out_x1,
                       float *out_z0, float *out_z1)
{
    if (g_terrain_tris == NULL || g_terrain_ntris <= 0) return 0;

    float x0 = 1e30f, x1 = -1e30f;
    float z0 = 1e30f, z1 = -1e30f;
    for (int i = 0; i < g_terrain_ntris; i++) {
        const TmTri *t = &g_terrain_tris[i];
        float xs[3] = { t->ax, t->bx, t->cx };
        float zs[3] = { t->az, t->bz, t->cz };
        for (int k = 0; k < 3; k++) {
            if (xs[k] < x0) x0 = xs[k];
            if (xs[k] > x1) x1 = xs[k];
            if (zs[k] < z0) z0 = zs[k];
            if (zs[k] > z1) z1 = zs[k];
        }
    }

    if (out_x0 != NULL) *out_x0 = x0;
    if (out_x1 != NULL) *out_x1 = x1;
    if (out_z0 != NULL) *out_z0 = z0;
    if (out_z1 != NULL) *out_z1 = z1;
    return 1;
}

int TerrainMesh_ObstacleHeightAt(int32_t pos_x, int32_t pos_y, int32_t pos_z,
                                 int32_t terrain_y, int32_t *out_y)
{
    if (g_obstacle_objs == NULL || g_obstacle_nobjs <= 0) return 0;

    int pos[3] = { pos_x, pos_y, pos_z };
    int16_t normal[4] = { 0, 0, 0, 0 };
    int32_t best_y = 0;
    int found = 0;

    int leaf = tm_object_bsp_find_leaf(pos_x, pos_z);
    int i = (leaf >= 0) ? g_object_bsp[leaf].first_obj : 0;

    while (i >= 0 && i < g_obstacle_nobjs) {
        int32_t hit = Object_FindObstacleAt((int *)g_obstacle_objs[i].obj,
                                            terrain_y, pos, normal);
        if (hit != 0 && (!found || hit < best_y)) {
            best_y = hit;
            found = 1;
        }
        i = (leaf >= 0) ? g_obstacle_objs[i].next_in_leaf : i + 1;
    }
    if (found && out_y != NULL) *out_y = best_y;
    return found;
}

/* ------------------------------------------------------------------ */
/* Everything below requires SDL + OpenGL.                             */
/* ------------------------------------------------------------------ */
#if defined(V8_HAVE_SDL) && defined(V8_HAVE_GL)
#include <GL/gl3w.h>

/* ---- Public GL globals ---- */
GLuint g_terrainmesh_vao = 0;
GLuint g_terrainmesh_tex = 0;
GLuint g_terrainmesh_xbmp_tex = 0;
GLuint g_terrainmesh_sky_tex = 0;
int    g_terrainmesh_vtx = 0;
int    g_terrainmesh_tex_w = 0;
int    g_terrainmesh_tex_h = 0;
int    g_terrainmesh_sky_w = 0;
int    g_terrainmesh_sky_h = 0;
static int g_terrainmesh_xbmp_w = 0;
static int g_terrainmesh_xbmp_h = 0;
static int g_terrainmesh_xbmp_x = 0;
static int g_terrainmesh_xbmp_y = 0;
static int g_tm_uv_ground_xbmp = 0;
static int g_tm_uv_ground_xbmp_oob = 0;
static int g_tm_uv_ground_none = 0;
static int g_tm_uv_xobf = 0;
static int g_tm_uv_none = 0;

/* ---- Config ---- */
#define TERR_SCALE      (1.0f / 16.0f)   /* fallback unplaced BIN scale */
/* OpenGL Y of BIN Y=0.
 * Derived by cross-validating BIN vertex data against ZONE heightmap:
 *   - Bone[1] (main 71-vertex terrain patch): centroid raw_y ~= 0.
 *   - Converted ZONE runtime terrain is around gl_y=-22.5m.
 *   - Solve: -(0/16) + TERR_Y_ORIGIN = -22.5 -> TERR_Y_ORIGIN = -22.5. */
#define TERR_Y_ORIGIN   (-22.5f)         /* OpenGL Y of BIN Y=0 */

/* ---- Binary helpers ---- */
static uint32_t tm_rd32be(const uint8_t *b, uint32_t o)
{
    return ((uint32_t)b[o]<<24)|((uint32_t)b[o+1]<<16)|
           ((uint32_t)b[o+2]<<8)|(uint32_t)b[o+3];
}
static uint32_t tm_rd32le(const uint8_t *b, uint32_t o)
{
    return (uint32_t)b[o]|((uint32_t)b[o+1]<<8)|
           ((uint32_t)b[o+2]<<16)|((uint32_t)b[o+3]<<24);
}
static uint16_t tm_rd16le(const uint8_t *b, uint32_t o)
{
    return (uint16_t)b[o]|((uint16_t)b[o+1]<<8);
}
static int16_t tm_rds16le(const uint8_t *b, uint32_t o)
{
    return (int16_t)tm_rd16le(b, o);
}
static int16_t tm_rd16be_s(const uint8_t *b, uint32_t o)
{
    return (int16_t)(((uint16_t)b[o]<<8)|(uint16_t)b[o+1]);
}
static int32_t tm_rd32be_s(const uint8_t *b, uint32_t o)
{
    return (int32_t)tm_rd32be(b, o);
}

/* ---- Packet format (identical to vehicle BIN) ---- */
static const int TM_PKT_SIZE[16] = {
    12,28,20,28, 12,20,12,20, 16,24,12,24, 20,20,0,20
};
static const int TM_IS_QUAD[16] = {
    0,0,0,0, 1,1,0,1, 0,0,0,0, 0,0,0,0
};

#define TM_DISPLAY_XZ_SCALE (1.0f / 65536.0f)
#define TM_DISPLAY_Y_SCALE  (1.0f / 65536.0f)

/* ---- Per-vertex GL data ---- */
typedef struct { float x,y,z,r,g,b,u,v,tex; } TmVert;

typedef struct {
    const uint8_t *data;
    uint32_t size;
    uint32_t group_count;
    uint32_t group_table;
    uint32_t obstacle_count;
    uint32_t obstacle_table;
    uint32_t obstacle_end;
    uint32_t slot_count;
#if defined(V8_HAVE_SDL) && defined(V8_HAVE_GL)
    V8XobfTexAtlas atlas;
#endif
} TmBank;

typedef struct {
    float x, y, z;
    uint8_t script;
    int32_t raw_x, raw_y, raw_z;
    int16_t ry, rx, rz;
    int16_t id;
    int bank;
    int slot;
    int type;
    uint32_t flags;
    int32_t initial_strength;
    char name[48];
} TmHead;

typedef struct {
    int head_index;
    int head_type;
    int32_t raw_x, raw_y, raw_z;
    uint8_t color_r, color_g, color_b, color_pad;
    int32_t inner_radius, outer_radius;
    int16_t cone_min, cone_max, intensity;
} TmObjLight;

typedef struct {
    int32_t raw_x, raw_z;
    int32_t raw_y;
    uint8_t flags;
    uint8_t edge_count;
    int has_patch;
    int bank;
    int slot;
    int16_t rot;
} TmJuncNode;

typedef struct {
    int32_t width;
    int32_t step;
    int16_t tex_id;
    int16_t flags;
} TmRouteType;

typedef struct {
    int type;
    int order;
    int flags;
    int node_a;
    int node_b;
    int32_t ctrl_ax;
    int32_t ctrl_az;
    int32_t ctrl_bx;
    int32_t ctrl_bz;
} TmRseg;

typedef struct {
    float m[3][3];
    float x, y, z;
    MATRIX raw;
} TmXform;

extern void *Asset_LoadFile(const char *path);

static uint8_t *tm_load_exp_blob(const char *exp_path, uint32_t *out_size,
                                 const char *who)
{
    uint8_t *raw = (uint8_t *)Asset_LoadFile(exp_path);
    if (raw == NULL) {
        fprintf(stderr, "v8: %s -- cannot load '%s'\n", who, exp_path);
        return NULL;
    }
    if (memcmp(raw, "FORM", 4) != 0) {
        fprintf(stderr, "v8: %s -- '%s' is not FORM terrain data\n",
                who, exp_path);
        Heap_Free(raw);
        return NULL;
    }
    uint32_t form_size = tm_rd32be(raw, 4);
    *out_size = form_size + 8u;
    return raw;
}

static void tm_psx555_rgba(uint16_t c, uint8_t *out)
{
    uint8_t r = (uint8_t)(c & 0x1f);
    uint8_t g = (uint8_t)((c >> 5) & 0x1f);
    uint8_t b = (uint8_t)((c >> 10) & 0x1f);
    out[0] = (uint8_t)((r << 3) | (r >> 2));
    out[1] = (uint8_t)((g << 3) | (g >> 2));
    out[2] = (uint8_t)((b << 3) | (b >> 2));
    out[3] = (c == 0) ? 0 : 255;
}

static int tm_pixel_width_from_words(int words, int depth)
{
    if (depth == 0) return words * 4;
    if (depth == 1) return words * 2;
    return words;
}

static int tm_find_chunk_in_form(const uint8_t *data, uint32_t off, uint32_t end,
                                 const char tag[4],
                                 uint32_t *out_off, uint32_t *out_size)
{
    while (off + 8 <= end) {
        uint32_t csz = tm_rd32be(data, off + 4);
        uint32_t body = off + 8;
        uint32_t next = body + csz;
        if (next > end) break;
        if (memcmp(data + off, tag, 4) == 0) {
            *out_off = body;
            *out_size = csz;
            return 1;
        }
        if (memcmp(data + off, "FORM", 4) == 0 && body + 4 <= next) {
            if (tm_find_chunk_in_form(data, body + 4, next, tag,
                                      out_off, out_size)) {
                return 1;
            }
        }
        off = next + (csz & 1);
    }
    return 0;
}

static GLuint tm_upload_xbmp_texture(const uint8_t *raw, uint32_t raw_size)
{
    uint32_t off = 0, size = 0;
    if (!tm_find_chunk_in_form(raw, 0, raw_size, "XBMP", &off, &size))
        return 0;
    if (size < 0x220)
        return 0;

    const uint8_t *p = raw + off;
    uint32_t flags = tm_rd32le(p, 4);
    uint32_t image_off = tm_rd32le(p, 8);
    int depth = (int)(flags & 3u);
    if (depth != 1 || image_off + 0x14 > size)
        return 0;

    int image_words = tm_rds16le(p, image_off + 0x10);
    int image_h = tm_rds16le(p, image_off + 0x12);
    int image_w = tm_pixel_width_from_words(image_words, depth);
    uint32_t pix_off = image_off + 0x14;
    uint32_t pix_size = (uint32_t)(image_w * image_h);
    int image_x = tm_rds16le(p, image_off + 0x0c);
    int image_y = tm_rds16le(p, image_off + 0x0e);
    if (image_w <= 0 || image_h <= 0 || pix_off + pix_size > size)
        return 0;

    uint8_t palette[256][4];
    for (int i = 0; i < 256; i++)
        tm_psx555_rgba(tm_rd16le(p, 0x14 + (uint32_t)i * 2u), palette[i]);

    uint8_t *rgba = (uint8_t *)malloc((size_t)image_w * (size_t)image_h * 4u);
    if (rgba == NULL)
        return 0;
    const uint8_t *pix = p + pix_off;
    for (int i = 0; i < image_w * image_h; i++) {
        rgba[i * 4 + 0] = palette[pix[i]][0];
        rgba[i * 4 + 1] = palette[pix[i]][1];
        rgba[i * 4 + 2] = palette[pix[i]][2];
        rgba[i * 4 + 3] = palette[pix[i]][3];
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image_w, image_h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    glBindTexture(GL_TEXTURE_2D, 0);
    free(rgba);

    g_terrainmesh_xbmp_w = image_w;
    g_terrainmesh_xbmp_h = image_h;
    g_terrainmesh_xbmp_x = image_x;
    g_terrainmesh_xbmp_y = image_y;
    fprintf(stderr, "v8: TerrainMesh -- uploaded XBMP texture %dx%d vram=(%d,%d) flags=0x%x\n",
            image_w, image_h, image_x, image_y, (unsigned)flags);
    return tex;
}

static GLuint tm_upload_xbgm_texture(const uint8_t *raw, uint32_t raw_size)
{
    uint32_t off = 0, size = 0;
    if (!tm_find_chunk_in_form(raw, 0, raw_size, "XBGM", &off, &size))
        return 0;
    if (size < 0x220)
        return 0;

    const uint8_t *p = raw + off;
    uint32_t flags = tm_rd32le(p, 4);
    uint32_t image_off = tm_rd32le(p, 8);
    int depth = (int)(flags & 3u);
    if (depth != 1 || image_off + 0x14 > size)
        return 0;

    int image_words = tm_rds16le(p, image_off + 0x10);
    int image_h = tm_rds16le(p, image_off + 0x12);
    int image_w = tm_pixel_width_from_words(image_words, depth);
    uint32_t pix_off = image_off + 0x14;
    uint32_t pix_size = (uint32_t)(image_w * image_h);
    if (image_w <= 0 || image_h <= 0 || pix_off + pix_size > size)
        return 0;

    uint8_t palette[256][4];
    for (int i = 0; i < 256; i++) {
        tm_psx555_rgba(tm_rd16le(p, 0x14 + (uint32_t)i * 2u), palette[i]);
        palette[i][3] = 255;
    }

    uint8_t *rgba = (uint8_t *)malloc((size_t)image_w * (size_t)image_h * 4u);
    if (rgba == NULL)
        return 0;
    const uint8_t *pix = p + pix_off;
    for (int i = 0; i < image_w * image_h; i++) {
        rgba[i * 4 + 0] = palette[pix[i]][0];
        rgba[i * 4 + 1] = palette[pix[i]][1];
        rgba[i * 4 + 2] = palette[pix[i]][2];
        rgba[i * 4 + 3] = 255;
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image_w, image_h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    glBindTexture(GL_TEXTURE_2D, 0);
    free(rgba);

    g_terrainmesh_sky_w = image_w;
    g_terrainmesh_sky_h = image_h;
    fprintf(stderr, "v8: TerrainMesh -- uploaded XBGM sky %dx%d flags=0x%x\n",
            image_w, image_h, (unsigned)flags);
    return tex;
}

static int tm_decode_packet_uv(const uint8_t *B, uint32_t po,
                               int nib, float uv[4][2])
{
    if (g_terrainmesh_xbmp_w <= 0 || g_terrainmesh_xbmp_h <= 0)
        return 0;

    uint32_t uvbase = 0;
    uint32_t tpage_off = 0;
    switch (nib) {
    case 5:
        uvbase = 0x0c;
        tpage_off = 0x12;
        break;
    case 9:
    case 11:
        uvbase = 0x10;
        tpage_off = 0x16;
        break;
    case 12:
    case 13:
    case 15:
        uvbase = 0x0c;
        tpage_off = 0x12;
        break;
    default:
        return 0;
    }

    uint16_t tpage = tm_rd16le(B, po + tpage_off);
    float page_x = (float)((tpage & 0x0fu) * 64u);
    float page_y = (float)(((tpage & 0x10u) != 0) ? 256u : 0u);
    for (int i = 0; i < 4; i++) {
        uint8_t u = B[po + uvbase + (uint32_t)i * 2u + 0u];
        uint8_t v = B[po + uvbase + (uint32_t)i * 2u + 1u];
        float x = page_x + (float)u - (float)g_terrainmesh_xbmp_x;
        float y = page_y + (float)v - (float)g_terrainmesh_xbmp_y;
        if (x < 0.0f || y < 0.0f ||
            x >= (float)g_terrainmesh_xbmp_w ||
            y >= (float)g_terrainmesh_xbmp_h) {
            return 0;
        }
        uv[i][0] = (x + 0.5f) / (float)g_terrainmesh_xbmp_w;
        uv[i][1] = (y + 0.5f) / (float)g_terrainmesh_xbmp_h;
    }
    return 1;
}

static int tm_is_render_ground_tri(float ax, float ay, float az,
                                   float bx, float by, float bz,
                                   float cx, float cy, float cz,
                                   float ny)
{
    float area_xz = fabsf((bx - ax) * (cz - az) - (bz - az) * (cx - ax)) * 0.5f;
    return fabsf(ny) >= 0.30f && area_xz >= 1.0f;
}

static float tm_area_xz(float ax, float az, float bx, float bz, float cx, float cz)
{
    return fabsf((bx - ax) * (cz - az) - (bz - az) * (cx - ax)) * 0.5f;
}

static int tm_upload_ground_visual(float ax, float ay, float az,
                                   float bx, float by, float bz,
                                   float cx, float cy, float cz,
                                   int ground)
{
    (void)ay; (void)by; (void)cy;
    if (!ground) return 1;
    return tm_area_xz(ax, az, bx, bz, cx, cz) < 4.0f;
}

static const uint8_t *tm_obstacle_stream(const TmBank *bank, int index)
{
    if (index < 0 || (uint32_t)index >= bank->obstacle_count) return NULL;
    if (bank->obstacle_table + (uint32_t)index * 4 + 4 > bank->size) return NULL;

    uint32_t rel = tm_rd32le(bank->data, bank->obstacle_table + (uint32_t)index * 4);
    uint32_t off = bank->obstacle_table + rel;
    if (off >= bank->size || off >= bank->obstacle_end) return NULL;
    return bank->data + off;
}

static void tm_matrix_from_raw(int32_t x, int32_t y, int32_t z,
                               int16_t r0, int16_t r1, int16_t r2,
                               MATRIX *out)
{
    SVECTOR rot;
    rot.vx = r0;
    rot.vy = r1;
    rot.vz = r2;
    rot.pad = 0;
    memset(out, 0, sizeof(*out));
    RotMatrixYXZ_gte(&rot, out);
    out->t[0] = x;
    out->t[1] = y;
    out->t[2] = z;
}

static MATRIX tm_compose_raw(const MATRIX *parent,
                             int32_t x, int32_t y, int32_t z,
                             int16_t r0, int16_t r1, int16_t r2)
{
    MATRIX local, out;
    tm_matrix_from_raw(x, y, z, r0, r1, r2, &local);
    CompMatrixLV(parent, &local, &out);
    return out;
}

static int32_t tm_sra32(int32_t v, int shift)
{
    if (shift <= 0) return v;
    if (shift >= 31) return v < 0 ? -1 : 0;
    return v >> shift;
}

static int32_t tm_sra64_to32(int64_t v, int shift)
{
    if (shift <= 0) return (int32_t)v;
    return (int32_t)(v >> shift);
}

static void tm_group_vertex_world(const MATRIX *m, uint8_t scale_shift,
                                  int16_t vx, int16_t vy, int16_t vz,
                                  float *out_x, float *out_y, float *out_z)
{
    int shift = 16 - (int)scale_shift;
    int32_t tx = tm_sra32(m->t[0], shift);
    int32_t ty = tm_sra32(m->t[1], shift);
    int32_t tz = tm_sra32(m->t[2], shift);
    int32_t rx = tm_sra64_to32((int64_t)m->m[0][0] * vx +
                               (int64_t)m->m[0][1] * vy +
                               (int64_t)m->m[0][2] * vz, 12);
    int32_t ry = tm_sra64_to32((int64_t)m->m[1][0] * vx +
                               (int64_t)m->m[1][1] * vy +
                               (int64_t)m->m[1][2] * vz, 12);
    int32_t rz = tm_sra64_to32((int64_t)m->m[2][0] * vx +
                               (int64_t)m->m[2][1] * vy +
                               (int64_t)m->m[2][2] * vz, 12);
    float unshift = (float)(1u << shift);

    *out_x = (float)(tx + rx) * unshift * TM_DISPLAY_XZ_SCALE;
    *out_y = (float)-(ty + ry) * unshift * TM_DISPLAY_Y_SCALE;
    *out_z = (float)(tz + rz) * unshift * TM_DISPLAY_XZ_SCALE;
}

static void tm_rot_yxz(int16_t ry, int16_t rx, int16_t rz, float out[3][3])
{
    float ay = (float)ry * (2.0f * 3.14159265358979323846f / 4096.0f);
    float ax = (float)rx * (2.0f * 3.14159265358979323846f / 4096.0f);
    float az = (float)rz * (2.0f * 3.14159265358979323846f / 4096.0f);
    float cy = cosf(ay), sy = sinf(ay);
    float cx = cosf(ax), sx = sinf(ax);
    float cz = cosf(az), sz = sinf(az);
    float Ry[3][3] = {{ cy,0.0f, sy},{0.0f,1.0f,0.0f},{-sy,0.0f,cy}};
    float Rx[3][3] = {{1.0f,0.0f,0.0f},{0.0f,cx,-sx},{0.0f,sx,cx}};
    float Rz[3][3] = {{cz,-sz,0.0f},{sz,cz,0.0f},{0.0f,0.0f,1.0f}};
    float T[3][3];
    for (int r=0;r<3;r++) for (int c=0;c<3;c++)
        T[r][c] = Ry[r][0]*Rx[0][c] + Ry[r][1]*Rx[1][c] + Ry[r][2]*Rx[2][c];
    for (int r=0;r<3;r++) for (int c=0;c<3;c++)
        out[r][c] = T[r][0]*Rz[0][c] + T[r][1]*Rz[1][c] + T[r][2]*Rz[2][c];
}

static void tm_xform_point(const TmXform *xf, float lx, float ly, float lz,
                           float *x, float *y, float *z)
{
    *x = xf->x + xf->m[0][0]*lx + xf->m[0][1]*ly + xf->m[0][2]*lz;
    *y = xf->y + xf->m[1][0]*lx + xf->m[1][1]*ly + xf->m[1][2]*lz;
    *z = xf->z + xf->m[2][0]*lx + xf->m[2][1]*ly + xf->m[2][2]*lz;
}

static TmXform tm_xform_mul(const TmXform *parent,
                            int32_t raw_lx, int32_t raw_ly, int32_t raw_lz,
                            int16_t ry, int16_t rx, int16_t rz)
{
    TmXform out;
    float R[3][3];
    float lx = (float)raw_lx * TM_DISPLAY_XZ_SCALE;
    float ly = (float)raw_ly * -TM_DISPLAY_Y_SCALE;
    float lz = (float)raw_lz * TM_DISPLAY_XZ_SCALE;
    tm_rot_yxz(ry, rx, rz, R);
    for (int r=0;r<3;r++) {
        for (int c=0;c<3;c++) {
            out.m[r][c] = parent->m[r][0]*R[0][c]
                        + parent->m[r][1]*R[1][c]
                        + parent->m[r][2]*R[2][c];
        }
    }
    tm_xform_point(parent, lx, ly, lz, &out.x, &out.y, &out.z);
    out.raw = tm_compose_raw(&parent->raw, raw_lx, raw_ly, raw_lz,
                             ry, rx, rz);
    return out;
}

/* ---- IFF walker: find first BIN inside an XOBF FORM in the EXP ---- */
static uint32_t tm_find_xobf_bin_r(const uint8_t *data, uint32_t off,
                                    uint32_t end,
                                    uint32_t *out_off, uint32_t *out_sz)
{
    uint32_t p = off;
    while (p + 8 <= end) {
        uint32_t csz  = tm_rd32be(data, p + 4);
        uint32_t body = p + 8;
        if (body + csz > end) break;
        if (memcmp(data + p, "FORM", 4) == 0) {
            if (body + 4 <= end && memcmp(data + body, "XOBF", 4) == 0) {
                /* Walk XOBF children for BIN. */
                uint32_t q    = body + 4;
                uint32_t qend = body + csz;
                while (q + 8 <= qend) {
                    uint32_t bsz   = tm_rd32be(data, q + 4);
                    uint32_t bbody = q + 8;
                    if (bbody + bsz > qend) break;
                    if (memcmp(data + q, "BIN ", 4) == 0) {
                        *out_off = bbody; *out_sz = bsz; return 1;
                    }
                    q = bbody + bsz + (bsz & 1);
                }
            } else {
                if (tm_find_xobf_bin_r(data, body + 4, body + csz,
                                       out_off, out_sz))
                    return 1;
            }
        }
        p = body + csz + (csz & 1);
    }
    return 0;
}

static uint32_t tm_find_xobf_bin(const uint8_t *data, uint32_t size,
                                  uint32_t *out_off, uint32_t *out_sz)
{
    return tm_find_xobf_bin_r(data, 0, size, out_off, out_sz);
}

static int tm_bsp_parse_node(const uint8_t *payload, uint32_t size,
                             uint32_t *pos, TmObjectBspNode *nodes,
                             int cap, int *count)
{
    if (*pos + 2 > size || *count >= cap) return -1;
    int index = (*count)++;
    TmObjectBspNode *node = &nodes[index];
    memset(node, 0, sizeof(*node));
    node->first_obj = -1;
    node->kind = tm_rd16be_s(payload, *pos);
    *pos += 2;

    if (node->kind == 0) {
        return index;
    }
    if (node->kind != 1 && node->kind != 2) {
        node->kind = -1;
        return index;
    }
    if (*pos + 4 > size) {
        node->kind = -1;
        return index;
    }
    node->coord = tm_rd32be_s(payload, *pos);
    *pos += 4;
    node->left = tm_bsp_parse_node(payload, size, pos, nodes, cap, count);
    node->right = tm_bsp_parse_node(payload, size, pos, nodes, cap, count);
    return index;
}

static void tm_build_object_bsp(const uint8_t *payload, uint32_t size,
                                TmObstacleObj *objs, int nobjs)
{
    free(g_object_bsp);
    g_object_bsp = NULL;
    g_object_bsp_nnodes = 0;
    g_object_bsp_root = -1;

    if (payload == NULL || size < 2) return;

    int cap = (int)(size / 2u) + 1;
    TmObjectBspNode *nodes = (TmObjectBspNode *)calloc((size_t)cap,
                                                       sizeof(TmObjectBspNode));
    if (nodes == NULL) return;

    uint32_t pos = 0;
    int count = 0;
    int root = tm_bsp_parse_node(payload, size, &pos, nodes, cap, &count);
    if (root < 0 || pos != size) {
        free(nodes);
        return;
    }

    for (int i = 0; i < nobjs; i++) {
        objs[i].next_in_leaf = -1;
        int leaf = root;
        int guard = 0;
        while (leaf >= 0 && leaf < count && guard++ < count + 1) {
            TmObjectBspNode *n = &nodes[leaf];
            if (n->kind == 0) break;
            if (n->kind == 1) {
                leaf = (n->coord < objs[i].root_x) ? n->right : n->left;
            } else if (n->kind == 2) {
                leaf = (n->coord < objs[i].root_z) ? n->right : n->left;
            } else {
                leaf = -1;
            }
        }
        if (leaf >= 0 && leaf < count && nodes[leaf].kind == 0) {
            objs[i].next_in_leaf = nodes[leaf].first_obj;
            nodes[leaf].first_obj = i;
            nodes[leaf].count++;
        }
    }

    g_object_bsp = nodes;
    g_object_bsp_nnodes = count;
    g_object_bsp_root = root;
}

static TmHead *tm_decode_obj_head(const uint8_t *h, uint32_t hsz,
                                  TmHead *heads, int *nheads, int max_heads)
{
    if (*nheads >= max_heads) return NULL;
    TmHead *dst = &heads[(*nheads)++];
    dst->script = h[0];
    dst->type = h[1];
    dst->id = tm_rd16be_s(h, 2);
    dst->flags = tm_rd32be(h, 4);
    dst->x    = (float)tm_rd32be_s(h, 8)  * (1.0f / 65536.0f);
    dst->y    = (float)(tm_rd32be_s(h, 12) - 0x100000) * TM_DISPLAY_Y_SCALE;
    dst->z    = (float)tm_rd32be_s(h, 16) * (1.0f / 65536.0f);
    dst->raw_x = tm_rd32be_s(h, 8);
    dst->raw_y = tm_rd32be_s(h, 12) - 0x100000;
    dst->raw_z = tm_rd32be_s(h, 16);
    dst->ry   = tm_rd16be_s(h, 20);
    dst->rx   = tm_rd16be_s(h, 22);
    dst->rz   = tm_rd16be_s(h, 24);
    dst->bank = tm_rd16be_s(h, 26);
    dst->slot = tm_rd16be_s(h, 28);
    dst->initial_strength = tm_rd32be_s(h, 30);
    memset(dst->name, 0, sizeof(dst->name));
    {
        int n = 0;
        const uint8_t *s = h + 34;
        while (n + 1 < (int)sizeof(dst->name)
            && 34u + (uint32_t)n < hsz
            && s[n] != 0) {
            dst->name[n] = (char)s[n];
            n++;
        }
        dst->name[n] = 0;
    }
    return dst;
}

static void tm_build_spawn_placeholders(const TmHead *heads, int nheads)
{
    int count = 0;
    for (int i = 0; i < nheads; i++) {
        if (heads[i].type == 5)
            count++;
    }

    free(g_spawn_placeholders);
    g_spawn_placeholders = NULL;
    g_spawn_nplaceholders = 0;
    if (count == 0)
        return;

    g_spawn_placeholders = (uint8_t *)calloc((size_t)count, 0x80);
    if (g_spawn_placeholders == NULL)
        return;

    for (int i = 0; i < nheads; i++) {
        const TmHead *h = &heads[i];
        if (h->type != 5)
            continue;

        uint8_t *obj = g_spawn_placeholders + (size_t)g_spawn_nplaceholders * 0x80;
        *(uint32_t *)(obj + 0x00) = h->flags & 0xfff867feu;
        *(uint8_t  *)(obj + 0x04) = (uint8_t)h->type;
        *(int16_t  *)(obj + 0x06) = h->id;
        *(uint8_t  *)(obj + 0x08) = h->script;
        *(int16_t  *)(obj + 0x0a) = h->slot;
        *(int16_t  *)(obj + 0x0c) = (int16_t)h->initial_strength;
        *(int16_t  *)(obj + 0x0e) = (int16_t)h->initial_strength;
        *(int16_t  *)(obj + 0x40) = h->ry;
        *(int16_t  *)(obj + 0x42) = h->rx;
        *(int16_t  *)(obj + 0x44) = h->rz;
        *(int32_t  *)(obj + 0x48) = h->raw_x;
        *(int32_t  *)(obj + 0x4c) = h->raw_y;
        *(int32_t  *)(obj + 0x50) = h->raw_z;
        *(uint32_t *)(obj + 0x58) = 0;
        *(uint8_t  *)(obj + 0x09) = 0;
        g_spawn_nplaceholders++;
    }
}

intptr_t Host_TerrainFindPlaceholderById(int spawn_id)
{
    for (int i = 0; i < g_spawn_nplaceholders; i++) {
        uint8_t *obj = g_spawn_placeholders + (size_t)i * 0x80;
        if (*(int16_t *)(obj + 0x06) == (int16_t)spawn_id)
            return (intptr_t)obj;
    }
    return 0;
}

int Host_TerrainCountPlaceholdersWithFlag(uint32_t flag)
{
    int count = 0;
    for (int i = 0; i < g_spawn_nplaceholders; i++) {
        uint8_t *obj = g_spawn_placeholders + (size_t)i * 0x80;
        uint32_t obj_flags = *(uint32_t *)(obj + 0x00);
        if (*(int16_t *)(obj + 0x06) > 0x1f &&
            (obj_flags & flag) != 0 &&
            (obj_flags & 0x8002u) == 0) {
            count++;
        }
    }
    return count;
}

intptr_t Host_TerrainNthPlaceholderWithFlag(uint32_t flag, int n)
{
    for (int i = 0; i < g_spawn_nplaceholders; i++) {
        uint8_t *obj = g_spawn_placeholders + (size_t)i * 0x80;
        uint32_t obj_flags = *(uint32_t *)(obj + 0x00);
        if (*(int16_t *)(obj + 0x06) > 0x1f &&
            (obj_flags & flag) != 0 &&
            (obj_flags & 0x8002u) == 0) {
            if (--n == -1)
                return (intptr_t)obj;
        }
    }
    return 0;
}

static void tm_log_head_name_summary(const TmHead *heads, int nheads)
{
    typedef struct { const char *name; int count; } NameCount;
    NameCount counts[32];
    int ncounts = 0;
    for (int i = 0; i < nheads; i++) {
        const char *name = heads[i].name[0] ? heads[i].name : "<unnamed>";
        int found = -1;
        for (int j = 0; j < ncounts; j++) {
            if (strcmp(counts[j].name, name) == 0) {
                found = j;
                break;
            }
        }
        if (found >= 0) {
            counts[found].count++;
        } else if (ncounts < (int)(sizeof(counts) / sizeof(counts[0]))) {
            counts[ncounts].name = name;
            counts[ncounts].count = 1;
            ncounts++;
        }
    }
    for (int i = 0; i < ncounts; i++) {
        for (int j = i + 1; j < ncounts; j++) {
            if (counts[j].count > counts[i].count) {
                NameCount tmp = counts[i];
                counts[i] = counts[j];
                counts[j] = tmp;
            }
        }
    }
    fprintf(stderr, "v8: TerrainMesh HEAD names:");
    for (int i = 0; i < ncounts && i < 12; i++) {
        fprintf(stderr, " %s:%d", counts[i].name, counts[i].count);
    }
    fprintf(stderr, "\n");
}

static void tm_parse_obj_form(const uint8_t *data, uint32_t off, uint32_t end,
                              TmHead *heads, int *nheads, int max_heads,
                              TmObjLight *lights, int *nlights, int max_lights)
{
    uint32_t p = off;
    TmHead *head = NULL;
    int head_index = -1;
    while (p + 8 <= end) {
        uint32_t csz = tm_rd32be(data, p + 4);
        uint32_t body = p + 8;
        if (body + csz > end) break;

        if (memcmp(data + p, "HEAD", 4) == 0 && csz >= 34) {
            head_index = *nheads;
            head = tm_decode_obj_head(data + body, csz, heads, nheads, max_heads);
        } else if (memcmp(data + p, "LGHT", 4) == 0 &&
                   csz >= 18 && head != NULL && *nlights < max_lights) {
            TmObjLight *v = &lights[(*nlights)++];
            v->head_index = head_index;
            v->head_type = head->type;
            v->raw_x = head->raw_x;
            v->raw_y = head->raw_y;
            v->raw_z = head->raw_z;
            v->color_r = data[body + 0];
            v->color_g = data[body + 1];
            v->color_b = data[body + 2];
            v->color_pad = data[body + 3];
            v->inner_radius = tm_rd32be_s(data + body, 4);
            v->outer_radius = tm_rd32be_s(data + body, 8);
            v->cone_min = tm_rd16be_s(data + body, 12);
            v->cone_max = tm_rd16be_s(data + body, 14);
            v->intensity = tm_rd16be_s(data + body, 16);
        }
        p = body + csz + (csz & 1);
    }
}

static void tm_collect_level_r(const uint8_t *data, uint32_t off, uint32_t end,
                               const char parent[4],
                               TmBank *banks, int *nbanks, int max_banks,
                               TmHead *heads, int *nheads, int max_heads,
                               TmObjLight *lights, int *nlights, int max_lights,
                               TmJuncNode *juncs, int *njuncs,
                               int max_juncs,
                               TmRseg *rsegs, int *nrsegs, int max_rsegs,
                               TmRouteType *rtypes, int *nrtypes,
                               int max_rtypes,
                               const uint8_t **bsp_payload,
                               uint32_t *bsp_size)
{
    uint32_t p = off;
    while (p + 8 <= end) {
        uint32_t csz  = tm_rd32be(data, p + 4);
        uint32_t body = p + 8;
        if (body + csz > end) break;

        if (memcmp(data + p, "FORM", 4) == 0) {
            if (csz >= 4) {
                char ftype[4];
                memcpy(ftype, data + body, 4);
                if (memcmp(ftype, "OBJ ", 4) == 0) {
                    tm_parse_obj_form(data, body + 4, body + csz,
                                      heads, nheads, max_heads,
                                      lights, nlights, max_lights);
                } else {
                    tm_collect_level_r(data, body + 4, body + csz,
                                       ftype, banks, nbanks, max_banks,
                                       heads, nheads, max_heads,
                                       lights, nlights, max_lights,
                                       juncs, njuncs, max_juncs,
                                       rsegs, nrsegs, max_rsegs,
                                       rtypes, nrtypes, max_rtypes,
                                       bsp_payload, bsp_size);
                }
            }
        } else if (memcmp(parent, "XOBF", 4) == 0 &&
                   memcmp(data + p, "BIN ", 4) == 0) {
            if (*nbanks < max_banks && csz >= 8) {
                TmBank *b = &banks[(*nbanks)++];
                b->data        = data + body;
                b->size        = csz;
                b->group_count = tm_rd32le(b->data, 0);
                b->group_table = tm_rd32le(b->data, 4);
                b->obstacle_count = tm_rd32le(b->data, 0x08) + 1u;
                b->obstacle_table = tm_rd32le(b->data, 0x0c);
                b->obstacle_end   = tm_rd32le(b->data, 0x14);
                /* XOBF BIN header +0x18 is the authoritative slot count.
                 * For all audited terrain banks it matches the derived
                 * (group_table - 0x1c) / 0x1c value, but the original loader
                 * treats the header field as the count. */
                b->slot_count  = tm_rd32le(b->data, 0x18);
                if (b->slot_count > 4096 ||
                    0x1c + b->slot_count * 0x1cu > b->group_table) {
                    b->slot_count = 0;
                }
#if defined(V8_HAVE_SDL) && defined(V8_HAVE_GL)
                V8_XobfTexAtlas_BuildFromBin(&b->atlas, b->data, b->size);
#endif
                if (b->obstacle_count > 4096 ||
                    b->obstacle_table >= b->size ||
                    b->obstacle_end > b->size ||
                    b->obstacle_table > b->obstacle_end) {
                    b->obstacle_count = 0;
                    b->obstacle_table = 0;
                    b->obstacle_end = 0;
                }
            }
        } else if (memcmp(parent, "TERR", 4) == 0 &&
                   memcmp(data + p, "JUNC", 4) == 0) {
            if (*njuncs < max_juncs && csz >= 10) {
                const uint8_t *h = data + body;
                uint8_t flags = h[8];
                uint32_t patch = 10;
                TmJuncNode *dst = &juncs[(*njuncs)++];
                dst->raw_x = tm_rd32be_s(h, 0);
                dst->raw_z = tm_rd32be_s(h, 4);
                dst->raw_y = Terrain_HeightAt((uint32_t)dst->raw_x,
                                               (uint32_t)dst->raw_z);
                dst->flags = flags;
                dst->edge_count = h[9];
                dst->has_patch = 0;
                dst->bank = -1;
                dst->slot = -1;
                dst->rot = 0;
                if ((flags & 2u) != 0) {
                    patch += 4;
                }
                if (patch + 6 <= csz && patch < csz) {
                    dst->has_patch = 1;
                    dst->bank = tm_rd16be_s(h, patch);
                    dst->slot = tm_rd16be_s(h, patch + 2);
                    dst->rot = tm_rd16be_s(h, patch + 4);
                }
            }
        } else if (memcmp(parent, "TERR", 4) == 0 &&
                   memcmp(data + p, "RSEG", 4) == 0) {
            if (*nrsegs < max_rsegs && csz >= 26) {
                const uint8_t *h = data + body;
                TmRseg *dst = &rsegs[(*nrsegs)++];
                dst->type = tm_rd16be_s(h, 0);
                dst->order = tm_rd16be_s(h, 2);
                dst->flags = tm_rd16be_s(h, 4);
                dst->node_a = tm_rd16be_s(h, 6);
                dst->node_b = tm_rd16be_s(h, 8);
                dst->ctrl_ax = tm_rd32be_s(h, 10);
                dst->ctrl_az = tm_rd32be_s(h, 14);
                dst->ctrl_bx = tm_rd32be_s(h, 18);
                dst->ctrl_bz = tm_rd32be_s(h, 22);
            }
        } else if (memcmp(parent, "TERR", 4) == 0 &&
                   memcmp(data + p, "XRTP", 4) == 0) {
            if (*nrtypes < max_rtypes && csz >= 12) {
                const uint8_t *h = data + body;
                TmRouteType *dst = &rtypes[(*nrtypes)++];
                dst->width = tm_rd32be_s(h, 0);
                dst->step = tm_rd32be_s(h, 4);
                dst->tex_id = tm_rd16be_s(h, 8);
                dst->flags = tm_rd16be_s(h, 10);
            }
        } else if (memcmp(data + p, "BSP ", 4) == 0) {
            *bsp_payload = data + body;
            *bsp_size = csz;
        }
        p = body + csz + (csz & 1);
    }
}

static int16_t tm_q12_sin(int16_t angle)
{
    return (int16_t)rsin(angle);
}

static int16_t tm_q12_cos(int16_t angle)
{
    return (int16_t)rcos(angle);
}

static int16_t tm_q12_trunc(int32_t v)
{
    if (v < 0) v += 0xfff;
    return (int16_t)(v >> 12);
}

static int32_t tm_trunc_shift32(int32_t v, int shift)
{
    if (shift <= 0) return v;
    if (v < 0) v += (1 << shift) - 1;
    return v >> shift;
}

static int32_t tm_bezier_coord_q12(int32_t p0, int32_t p1,
                                   int32_t p2, int32_t p3,
                                   int32_t t)
{
    int32_t t2 = tm_trunc_shift32((int32_t)((int64_t)t * t), 12);
    int32_t t3 = tm_trunc_shift32((int32_t)((int64_t)t2 * t), 12);
    int32_t a = tm_trunc_shift32((p1 * 3 - p0) - p2 * 3 + p3, 4);
    int32_t b = tm_trunc_shift32(p0 * 3 - p1 * 6 + p2 * 3, 4);
    int32_t c = tm_trunc_shift32(p1 * 3 - p0 * 3, 4);
    int64_t v = (int64_t)a * t3 + (int64_t)b * t2 + (int64_t)c * t;
    if (v < 0) v += 0xff;
    return p0 + (int32_t)(v >> 8);
}

static void tm_emit_flat_face(float vx[4], float vy[4], float vz[4],
                              int ia, int ib, int ic,
                              float base_r, float base_g, float base_b,
                              TmVert *vbuf, int vcap, int *nvtx_io,
                              TmTri *tribuf, int tcap, int *ntris_io)
{
    int nvtx = *nvtx_io;
    int ntris = *ntris_io;
    static const float LX = 0.408f, LY = 0.816f, LZ = -0.408f;

    float ex = vx[ib]-vx[ia], ey = vy[ib]-vy[ia], ez = vz[ib]-vz[ia];
    float fx = vx[ic]-vx[ia], fy = vy[ic]-vy[ia], fz = vz[ic]-vz[ia];
    float tnx = ey*fz-ez*fy, tny = ez*fx-ex*fz, tnz = ex*fy-ey*fx;
    float nl = sqrtf(tnx*tnx + tny*tny + tnz*tnz);
    if (nl > 1e-6f) { tnx/=nl; tny/=nl; tnz/=nl; }
    tnx=-tnx; tny=-tny; tnz=-tnz;

    float ndotl = tnx*LX + tny*LY + tnz*LZ;
    float lit = 0.45f + 0.42f * (ndotl > 0 ? ndotl : 0);
    float lr = base_r * lit;
    float lg = base_g * lit;
    float lb = base_b * lit;

    if (ntris < tcap) {
        TmTri *tt = &tribuf[ntris++];
        tt->ax=vx[ia]; tt->ay=vy[ia]; tt->az=vz[ia];
        tt->bx=vx[ib]; tt->by=vy[ib]; tt->bz=vz[ib];
        tt->cx=vx[ic]; tt->cy=vy[ic]; tt->cz=vz[ic];
        tt->nx=tnx; tt->ny=tny; tt->nz=tnz;
        tt->pd = tnx*vx[ia] + tny*vy[ia] + tnz*vz[ia];
    }
    if (nvtx + 3 <= vcap) {
        vbuf[nvtx++] = (TmVert){vx[ia],vy[ia],vz[ia],lr,lg,lb,-1.0f,-1.0f,0.0f};
        vbuf[nvtx++] = (TmVert){vx[ic],vy[ic],vz[ic],lr,lg,lb,-1.0f,-1.0f,0.0f};
        vbuf[nvtx++] = (TmVert){vx[ib],vy[ib],vz[ib],lr,lg,lb,-1.0f,-1.0f,0.0f};
    }

    *nvtx_io = nvtx;
    *ntris_io = ntris;
}

static void tm_emit_group(const TmBank *bank, uint32_t group,
                          const TmXform *xf,
                          TmVert *vbuf, int vcap, int *nvtx_io,
                          TmTri *tribuf, int tcap, int *ntris_io,
                          int *bad_io)
{
    const uint8_t *B = bank->data;
    uint32_t bsz = bank->size;
    int nvtx = *nvtx_io;
    int ntris = *ntris_io;

    if (group >= bank->group_count) return;
    if (bank->group_table + group * 4 + 4 > bsz) return;
    uint32_t rel = tm_rd32le(B, bank->group_table + group * 4);
    uint32_t bd  = bank->group_table + rel;
    if (bd + 0x1a > bsz) return;

    uint32_t vc = tm_rd32le(B, bd + 0x00);
    uint32_t vr = tm_rd32le(B, bd + 0x04);
    uint16_t pc = tm_rd16le(B, bd + 0x10);
    int16_t tex_base = tm_rds16le(B, bd + 0x12);
    uint32_t pr = tm_rd32le(B, bd + 0x14);
    uint8_t scale_shift = B[bd + 0x18];
    uint32_t vo = bd + vr;
    uint32_t po = bd + pr;

    if (scale_shift > 16 || vo + vc * 8u > bsz || po >= bsz) return;

    static const float LX = 0.408f, LY = 0.816f, LZ = -0.408f;

    for (uint16_t pi = 0; pi < pc; pi++) {
        if (po + 4 > bsz) break;
        uint8_t typ = B[po + 3];
        int nib = typ & 0xf;
        int sz  = TM_PKT_SIZE[nib];
        if (sz == 0 || po + (uint32_t)sz > bsz) {
            (*bad_io)++;
            po += 4;
            continue;
        }

        uint16_t vi[4];
        vi[0] = tm_rd16le(B, po+4);
        vi[1] = tm_rd16le(B, po+6);
        vi[2] = tm_rd16le(B, po+8);
        vi[3] = TM_IS_QUAD[nib] ? tm_rd16le(B, po+10) : vi[2];

        int nv = TM_IS_QUAD[nib] ? 4 : 3;
        int ok = 1;
        for (int k = 0; k < nv; k++)
            if (vi[k] >= vc) { ok = 0; break; }

        if (ok) {
            float vx[4], vy[4], vz[4];
            for (int k = 0; k < nv; k++) {
                uint32_t oe = vo + vi[k] * 8;
                if (oe + 6 > bsz) { ok = 0; break; }
                tm_group_vertex_world(&xf->raw, scale_shift,
                                      tm_rds16le(B, oe+0),
                                      tm_rds16le(B, oe+2),
                                      tm_rds16le(B, oe+4),
                                      &vx[k], &vy[k], &vz[k]);
            }
            if (!ok) { po += sz; continue; }

            float ex = vx[1]-vx[0], ey = vy[1]-vy[0], ez = vz[1]-vz[0];
            float fx = vx[2]-vx[0], fy = vy[2]-vy[0], fz = vz[2]-vz[0];
            float tnx = ey*fz-ez*fy, tny = ez*fx-ex*fz, tnz = ex*fy-ey*fx;
            float nl = sqrtf(tnx*tnx + tny*tny + tnz*tnz);
            if (nl > 1e-6f) { tnx/=nl; tny/=nl; tnz/=nl; }
            tnx=-tnx; tny=-tny; tnz=-tnz;

            float ndotl = tnx*LX + tny*LY + tnz*LZ;
            float lit   = 0.42f + 0.45f * (ndotl > 0 ? ndotl : 0);
            float lr = 0.34f * lit;
            float lg = 0.38f * lit;
            float lb = 0.36f * lit;
            float uv[4][2] = {
                {-1.0f, -1.0f}, {-1.0f, -1.0f},
                {-1.0f, -1.0f}, {-1.0f, -1.0f}
            };
            int ground0 = tm_is_render_ground_tri(vx[0], vy[0], vz[0],
                                                  vx[1], vy[1], vz[1],
                                                  vx[2], vy[2], vz[2], tny);
            int has_uv = 0;
            float tex_kind = 0.0f;
#if defined(V8_HAVE_SDL) && defined(V8_HAVE_GL)
            if (ground0) {
                has_uv = tm_decode_packet_uv(B, po, nib, uv);
                tex_kind = has_uv ? 1.0f : 0.0f;
            }
            if (!has_uv) {
                has_uv = V8_XobfTex_DecodePacketUv(&bank->atlas, B + po, nib,
                                                    (int)tex_base, uv);
                tex_kind = 0.0f;
            }
            if (ground0) {
                if (has_uv && tex_kind > 0.5f) g_tm_uv_ground_xbmp++;
                else if (has_uv) g_tm_uv_xobf++;
                else g_tm_uv_ground_none++;
            } else {
                if (has_uv) g_tm_uv_xobf++;
                else g_tm_uv_none++;
            }
#else
            has_uv = tm_decode_packet_uv(B, po, nib, uv);
#endif

            if (ground0) {
                float gy = (vy[0] + vy[1] + vy[2]) * (1.0f / 3.0f);
                float ht = (gy + 64.0f) / 137.0f;
                if (ht < 0.0f) ht = 0.0f;
                if (ht > 1.0f) ht = 1.0f;
                lr = 0.22f + 0.20f * ht;
                lg = 0.30f + 0.18f * ht;
                lb = 0.24f + 0.12f * (1.0f - ht);
            }

            if (ntris < tcap) {
                TmTri *tt = &tribuf[ntris++];
                tt->ax=vx[0]; tt->ay=vy[0]; tt->az=vz[0];
                tt->bx=vx[1]; tt->by=vy[1]; tt->bz=vz[1];
                tt->cx=vx[2]; tt->cy=vy[2]; tt->cz=vz[2];
                tt->nx=tnx; tt->ny=tny; tt->nz=tnz;
                tt->pd = tnx*vx[0] + tny*vy[0] + tnz*vz[0];
            }
            if (tm_upload_ground_visual(vx[0], vy[0], vz[0],
                                        vx[1], vy[1], vz[1],
                                        vx[2], vy[2], vz[2], ground0)
                && nvtx + 3 <= vcap) {
                vbuf[nvtx++] = (TmVert){vx[0],vy[0],vz[0],lr,lg,lb, has_uv ? uv[0][0] : -1.0f, has_uv ? uv[0][1] : -1.0f, tex_kind};
                vbuf[nvtx++] = (TmVert){vx[2],vy[2],vz[2],lr,lg,lb, has_uv ? uv[2][0] : -1.0f, has_uv ? uv[2][1] : -1.0f, tex_kind};
                vbuf[nvtx++] = (TmVert){vx[1],vy[1],vz[1],lr,lg,lb, has_uv ? uv[1][0] : -1.0f, has_uv ? uv[1][1] : -1.0f, tex_kind};
            }

            if (TM_IS_QUAD[nib]) {
                int ground1 = tm_is_render_ground_tri(vx[0], vy[0], vz[0],
                                                      vx[2], vy[2], vz[2],
                                                      vx[3], vy[3], vz[3], tny);
                float lr1 = 0.34f * lit, lg1 = 0.38f * lit, lb1 = 0.36f * lit;
                float uv1[4][2];
                memcpy(uv1, uv, sizeof(uv1));
                int has_uv1 = has_uv;
                float tex_kind1 = tex_kind;
                if (ground1) {
                    has_uv1 = tm_decode_packet_uv(B, po, nib, uv1);
                    tex_kind1 = has_uv1 ? 1.0f : 0.0f;
                    if (!has_uv1) {
                        has_uv1 = has_uv;
                        memcpy(uv1, uv, sizeof(uv1));
                        tex_kind1 = tex_kind;
                    }
                }
                if (ground1) {
                    if (has_uv1 && tex_kind1 > 0.5f) g_tm_uv_ground_xbmp++;
                    else if (has_uv1) g_tm_uv_xobf++;
                    else g_tm_uv_ground_none++;
                } else {
                    if (has_uv1) g_tm_uv_xobf++;
                    else g_tm_uv_none++;
                }
                if (ground1) {
                    float gy = (vy[0] + vy[2] + vy[3]) * (1.0f / 3.0f);
                    float ht = (gy + 64.0f) / 137.0f;
                    if (ht < 0.0f) ht = 0.0f;
                    if (ht > 1.0f) ht = 1.0f;
                    lr1 = 0.22f + 0.20f * ht;
                    lg1 = 0.30f + 0.18f * ht;
                    lb1 = 0.24f + 0.12f * (1.0f - ht);
                }
                if (ntris < tcap) {
                    TmTri *tt = &tribuf[ntris++];
                    tt->ax=vx[0]; tt->ay=vy[0]; tt->az=vz[0];
                    tt->bx=vx[2]; tt->by=vy[2]; tt->bz=vz[2];
                    tt->cx=vx[3]; tt->cy=vy[3]; tt->cz=vz[3];
                    tt->nx=tnx; tt->ny=tny; tt->nz=tnz;
                    tt->pd = tnx*vx[0] + tny*vy[0] + tnz*vz[0];
                }
                if (tm_upload_ground_visual(vx[0], vy[0], vz[0],
                                            vx[2], vy[2], vz[2],
                                            vx[3], vy[3], vz[3], ground1)
                    && nvtx + 3 <= vcap) {
                    vbuf[nvtx++] = (TmVert){vx[0],vy[0],vz[0],lr1,lg1,lb1, has_uv1 ? uv1[0][0] : -1.0f, has_uv1 ? uv1[0][1] : -1.0f, tex_kind1};
                    vbuf[nvtx++] = (TmVert){vx[3],vy[3],vz[3],lr1,lg1,lb1, has_uv1 ? uv1[3][0] : -1.0f, has_uv1 ? uv1[3][1] : -1.0f, tex_kind1};
                    vbuf[nvtx++] = (TmVert){vx[2],vy[2],vz[2],lr1,lg1,lb1, has_uv1 ? uv1[2][0] : -1.0f, has_uv1 ? uv1[2][1] : -1.0f, tex_kind1};
                }
            }
        }
        po += sz;
    }

    *nvtx_io = nvtx;
    *ntris_io = ntris;
}

static void tm_emit_junc_patch_group(const TmBank *bank,
                                     const TmJuncNode *patch,
                                     TmVert *vbuf, int vcap, int *nvtx_io,
                                     TmTri *tribuf, int tcap, int *ntris_io,
                                     int *bad_io)
{
    const uint8_t *B = bank->data;
    uint32_t bsz = bank->size;

    if (patch->slot < 0 || (uint32_t)patch->slot >= bank->slot_count) return;
    const uint8_t *slot = B + 0x1c + (uint32_t)patch->slot * 0x1c;
    uint32_t group = (uint32_t)(((uint16_t)tm_rds16le(slot, 0)) & 0x07ffu);
    if (group >= bank->group_count) return;
    if (bank->group_table + group * 4 + 4 > bsz) return;

    uint32_t rel = tm_rd32le(B, bank->group_table + group * 4);
    uint32_t bd = bank->group_table + rel;
    if (bd + 0x1a > bsz) return;

    uint32_t vc = tm_rd32le(B, bd + 0x00);
    uint32_t vr = tm_rd32le(B, bd + 0x04);
    uint16_t pc = tm_rd16le(B, bd + 0x10);
    uint32_t pr = tm_rd32le(B, bd + 0x14);
    uint32_t vo = bd + vr;
    uint32_t po = bd + pr;
    uint8_t scale_shift = B[bd + 0x18];
    if (vc == 0 || vc > 4096 || scale_shift > 16 ||
        vo + vc * 8 > bsz || po >= bsz) {
        return;
    }

    float *wx = (float *)malloc(sizeof(float) * (size_t)vc * 3u);
    if (wx == NULL) return;
    float *wy = wx + vc;
    float *wz = wy + vc;

    int shift = 16 - (int)scale_shift;
    int16_t cs = tm_q12_cos(patch->rot);
    int16_t sn = tm_q12_sin(patch->rot);
    for (uint32_t vi = 0; vi < vc; vi++) {
        uint32_t oe = vo + vi * 8u;
        int16_t lx = tm_rds16le(B, oe + 0);
        int16_t lz = tm_rds16le(B, oe + 4);
        int16_t rx = tm_q12_trunc((int32_t)cs * (int32_t)lx +
                                  (int32_t)sn * (int32_t)lz);
        int16_t rz = tm_q12_trunc(-(int32_t)sn * (int32_t)lx +
                                  (int32_t)cs * (int32_t)lz);
        int32_t fx = patch->raw_x + ((int32_t)rx << shift);
        int32_t fz = patch->raw_z + ((int32_t)rz << shift);
        int32_t fy = Terrain_HeightAt((uint32_t)fx, (uint32_t)fz);
        wx[vi] = (float)fx * (1.0f / 65536.0f);
        wy[vi] = (float)-fy * TM_DISPLAY_Y_SCALE;
        wz[vi] = (float)fz * (1.0f / 65536.0f);
    }

    for (uint16_t pi = 0; pi < pc; pi++) {
        if (po + 4 > bsz) break;
        uint8_t typ = B[po + 3];
        int nib = typ & 0xf;
        int sz = TM_PKT_SIZE[nib];
        if (sz == 0 || po + (uint32_t)sz > bsz) {
            (*bad_io)++;
            po += 4;
            continue;
        }

        uint16_t vi[4];
        vi[0] = tm_rd16le(B, po + 4);
        vi[1] = tm_rd16le(B, po + 6);
        vi[2] = tm_rd16le(B, po + 8);
        vi[3] = TM_IS_QUAD[nib] ? tm_rd16le(B, po + 10) : vi[2];
        int nv = TM_IS_QUAD[nib] ? 4 : 3;
        int ok = 1;
        for (int k = 0; k < nv; k++) {
            if (vi[k] >= vc) { ok = 0; break; }
        }
        if (ok) {
            float vx[4], vy[4], vz[4];
            for (int k = 0; k < nv; k++) {
                vx[k] = wx[vi[k]];
                vy[k] = wy[vi[k]];
                vz[k] = wz[vi[k]];
            }
            tm_emit_flat_face(vx, vy, vz, 0, 1, 2, 0.38f, 0.42f, 0.34f,
                              vbuf, vcap, nvtx_io, tribuf, tcap, ntris_io);
            if (TM_IS_QUAD[nib]) {
                tm_emit_flat_face(vx, vy, vz, 0, 2, 3, 0.38f, 0.42f, 0.34f,
                                  vbuf, vcap, nvtx_io, tribuf, tcap,
                                  ntris_io);
            }
        }
        po += sz;
    }

    free(wx);
}

static int tm_emit_route_strips(const TmJuncNode *juncs, int njuncs,
                                const TmRseg *rsegs, int nrsegs,
                                const TmRouteType *rtypes, int nrtypes,
                                TmVert *vbuf, int vcap, int *nvtx,
                                TmTri *tribuf, int tcap, int *ntris)
{
    int before = *ntris;

    for (int ri = 0; ri < nrsegs; ri++) {
        const TmRseg *seg = &rsegs[ri];
        if (seg->node_a < 0 || seg->node_a >= njuncs ||
            seg->node_b < 0 || seg->node_b >= njuncs ||
            seg->type < 0 || seg->type >= nrtypes) {
            continue;
        }
        const TmJuncNode *a = &juncs[seg->node_a];
        const TmJuncNode *b = &juncs[seg->node_b];
        const TmRouteType *rt = &rtypes[seg->type];
        if (rt->width <= 0 || rt->step <= 0) continue;

        int32_t px[4] = {
            a->raw_x,
            a->raw_x + seg->ctrl_ax,
            b->raw_x + seg->ctrl_bx,
            b->raw_x
        };
        int32_t pz[4] = {
            a->raw_z,
            a->raw_z + seg->ctrl_az,
            b->raw_z + seg->ctrl_bz,
            b->raw_z
        };

        double length = 0.0;
        int32_t last_x = px[0], last_z = pz[0];
        for (int i = 1; i <= 24; i++) {
            int32_t t = (int32_t)((i * 0x1000) / 24);
            int32_t x = tm_bezier_coord_q12(px[0], px[1], px[2], px[3], t);
            int32_t z = tm_bezier_coord_q12(pz[0], pz[1], pz[2], pz[3], t);
            double dx = (double)x - (double)last_x;
            double dz = (double)z - (double)last_z;
            length += sqrt(dx * dx + dz * dz);
            last_x = x;
            last_z = z;
        }

        int steps = (int)ceil(length / (double)rt->step);
        if (steps < 2) steps = 2;
        if (steps > 256) steps = 256;

        float prev_lx = 0, prev_ly = 0, prev_lz = 0;
        float prev_rx = 0, prev_ry = 0, prev_rz = 0;
        int have_prev = 0;

        for (int i = 0; i <= steps; i++) {
            int32_t t = (int32_t)((i * 0x1000) / steps);
            int32_t t_prev = (i == 0) ? 0 : (int32_t)(((i - 1) * 0x1000) / steps);
            int32_t t_next = (i == steps) ? 0x1000 : (int32_t)(((i + 1) * 0x1000) / steps);
            int32_t x = tm_bezier_coord_q12(px[0], px[1], px[2], px[3], t);
            int32_t z = tm_bezier_coord_q12(pz[0], pz[1], pz[2], pz[3], t);
            int32_t x0 = tm_bezier_coord_q12(px[0], px[1], px[2], px[3], t_prev);
            int32_t z0 = tm_bezier_coord_q12(pz[0], pz[1], pz[2], pz[3], t_prev);
            int32_t x1 = tm_bezier_coord_q12(px[0], px[1], px[2], px[3], t_next);
            int32_t z1 = tm_bezier_coord_q12(pz[0], pz[1], pz[2], pz[3], t_next);
            double dx = (double)x1 - (double)x0;
            double dz = (double)z1 - (double)z0;
            double dlen = sqrt(dx * dx + dz * dz);
            if (dlen < 1.0) continue;

            double half = (double)rt->width * 0.5;
            int32_t ox = (int32_t)lrint(dz * half / dlen);
            int32_t oz = (int32_t)lrint(dx * half / dlen);
            int32_t lx = x - ox;
            int32_t lz = z + oz;
            int32_t rx = x + ox;
            int32_t rz = z - oz;
            int32_t ly = Terrain_HeightAt((uint32_t)lx, (uint32_t)lz);
            int32_t ry = Terrain_HeightAt((uint32_t)rx, (uint32_t)rz);

            float cur_lx = (float)lx * (1.0f / 65536.0f);
            float cur_ly = (float)-ly * TM_DISPLAY_Y_SCALE;
            float cur_lz = (float)lz * (1.0f / 65536.0f);
            float cur_rx = (float)rx * (1.0f / 65536.0f);
            float cur_ry = (float)-ry * TM_DISPLAY_Y_SCALE;
            float cur_rz = (float)rz * (1.0f / 65536.0f);

            if (have_prev) {
                float vx[4] = { prev_lx, prev_rx, cur_lx, cur_rx };
                float vy[4] = { prev_ly, prev_ry, cur_ly, cur_ry };
                float vz[4] = { prev_lz, prev_rz, cur_lz, cur_rz };
                tm_emit_flat_face(vx, vy, vz, 0, 1, 2,
                                  0.30f, 0.32f, 0.28f,
                                  vbuf, vcap, nvtx, tribuf, tcap, ntris);
                tm_emit_flat_face(vx, vy, vz, 1, 3, 2,
                                  0.30f, 0.32f, 0.28f,
                                  vbuf, vcap, nvtx, tribuf, tcap, ntris);
            }

            prev_lx = cur_lx; prev_ly = cur_ly; prev_lz = cur_lz;
            prev_rx = cur_rx; prev_ry = cur_ry; prev_rz = cur_rz;
            have_prev = 1;
        }
    }

    return *ntris - before;
}

static void tm_build_slot_like_original(const TmBank *bank, int slot,
                         const TmXform *parent_xf, int is_root,
                         uint32_t flags, int depth, uint8_t *seen,
                         TmVert *vbuf, int vcap, int *nvtx,
                         TmTri *tribuf, int tcap, int *ntris,
                         int *bad)
{
    if (slot < 0 || (uint32_t)slot >= bank->slot_count || depth > 256) return;
    if (seen[slot]) return;
    seen[slot] = 1;

    const uint8_t *e = bank->data + 0x1c + (uint32_t)slot * 0x1c;
    int key = tm_rds16le(e, 0);
    int32_t lx = (int32_t)tm_rd32le(e, 4);
    int32_t ly = (int32_t)tm_rd32le(e, 8);
    int32_t lz = (int32_t)tm_rd32le(e, 12);
    int16_t ry = tm_rds16le(e, 16);
    int16_t rx = tm_rds16le(e, 18);
    int16_t rz = tm_rds16le(e, 20);
    /* FUN_8001ac44 stores slot +0x18 in object +0x34 (next sibling) and
     * slot +0x1a in object +0x38 (first child).  Object walkers use
     * +0x38 as child head and +0x34 as sibling link. */
    int next_sibling = tm_rds16le(e, 24);
    int first_child  = tm_rds16le(e, 26);

    if (key < 0 && (key != -1 || ((flags & 4) != 0))) {
        /* FUN_8001b0c4 scans an object's first-child/next-sibling slot chain for
         * key0 values in the 0xcxxx range and builds one extra render group
         * at obj+0x68. Static HEAD placement does not automatically render
         * those alternates; drawing every one creates large grey state/LOD
         * panels around Wild West buildings. Keep walking siblings exactly
         * like FUN_8001ac44, but do not emit the state-controlled group here. */
        if ((flags & 1) != 0 && next_sibling != -1) {
            tm_build_slot_like_original(bank, next_sibling, parent_xf, 0, flags,
                                        depth + 1, seen, vbuf, vcap,
                                        nvtx, tribuf, tcap, ntris, bad);
        }
        return;
    }

    TmXform self_xf = *parent_xf;
    if (!is_root) {
        self_xf = tm_xform_mul(parent_xf, lx, ly, lz, ry, rx, rz);
    }

    if (key >= 0) {
        tm_emit_group(bank, (uint32_t)(key & 0x7ff), &self_xf,
                      vbuf, vcap, nvtx, tribuf, tcap, ntris, bad);
    }

    if ((flags & 1) != 0 && next_sibling != -1) {
        tm_build_slot_like_original(bank, next_sibling, parent_xf, 0, flags,
                                    depth + 1, seen, vbuf, vcap,
                                    nvtx, tribuf, tcap, ntris, bad);
    }
    if ((flags & 2) == 0 && first_child != -1) {
        tm_build_slot_like_original(bank, first_child, &self_xf, 0, flags | 1u,
                                    depth + 1, seen, vbuf, vcap,
                                    nvtx, tribuf, tcap, ntris, bad);
    }
}

static void tm_obstacle_obj_add(const MATRIX *mat, const uint8_t *stream,
                                int32_t root_x, int32_t root_z,
                                TmObstacleObj *objs, int cap, int *count)
{
    if (stream == NULL || *count >= cap) return;
    TmObstacleObj *o = &objs[(*count)++];
    memset(o->obj, 0, sizeof(o->obj));
    memcpy(o->obj + 0x10, mat, sizeof(*mat));
    *(uint32_t *)(o->obj + 0x5c) = (uint32_t)(uintptr_t)stream;
    o->root_x = root_x;
    o->root_z = root_z;
    o->next_in_leaf = -1;
}

static void tm_build_obstacle_slot_like_original(const TmBank *bank, int slot,
                         const MATRIX *parent_mat, int is_root,
                         int32_t root_x, int32_t root_z,
                         uint32_t flags, int depth, uint8_t *seen,
                         TmObstacleObj *objs, int cap, int *count)
{
    if (slot < 0 || (uint32_t)slot >= bank->slot_count || depth > 256) return;
    if (seen[slot]) return;
    seen[slot] = 1;

    const uint8_t *e = bank->data + 0x1c + (uint32_t)slot * 0x1c;
    int key0 = tm_rds16le(e, 0);
    int key1 = tm_rds16le(e, 2);
    int32_t lx = (int32_t)tm_rd32le(e, 4);
    int32_t ly = (int32_t)tm_rd32le(e, 8);
    int32_t lz = (int32_t)tm_rd32le(e, 12);
    int16_t r0 = tm_rds16le(e, 16);
    int16_t r1 = tm_rds16le(e, 18);
    int16_t r2 = tm_rds16le(e, 20);
    int next_sibling = tm_rds16le(e, 24);
    int first_child  = tm_rds16le(e, 26);

    if (key0 < 0 && (key0 != -1 || ((flags & 4) != 0))) {
        if ((flags & 1) != 0 && next_sibling != -1) {
            tm_build_obstacle_slot_like_original(bank, next_sibling, parent_mat, 0,
                                                 root_x, root_z, flags,
                                                 depth + 1, seen,
                                                 objs, cap, count);
        }
        return;
    }

    MATRIX self_mat = *parent_mat;
    if (!is_root) {
        self_mat = tm_compose_raw(parent_mat, lx, ly, lz, r0, r1, r2);
    }

    if (key1 >= 0) {
        tm_obstacle_obj_add(&self_mat, tm_obstacle_stream(bank, key1),
                            root_x, root_z,
                            objs, cap, count);
    }

    if ((flags & 1) != 0 && next_sibling != -1) {
        tm_build_obstacle_slot_like_original(bank, next_sibling, parent_mat, 0,
                                             root_x, root_z, flags,
                                             depth + 1, seen,
                                             objs, cap, count);
    }
    if ((flags & 2) == 0 && first_child != -1) {
        tm_build_obstacle_slot_like_original(bank, first_child, &self_mat, 0,
                                             root_x, root_z, flags | 1u,
                                             depth + 1, seen,
                                             objs, cap, count);
    }
}

static int tm_build_obstacle_instances(const TmBank *banks, int nbanks,
                                       const TmHead *heads, int nheads,
                                       TmObstacleObj *objs, int cap)
{
    int count = 0;

    for (int hi = 0; hi < nheads; hi++) {
        const TmHead *h = &heads[hi];
        if (h->type == 1 || h->type == 5 || h->type == 6 || h->type > 6) continue;
        if (h->bank < 0 || h->bank >= nbanks || h->slot < 0) continue;
        if (banks[h->bank].slot_count == 0) continue;

        uint8_t *seen = (uint8_t *)calloc((size_t)banks[h->bank].slot_count, 1);
        if (!seen) continue;

        MATRIX root_mat;
        tm_matrix_from_raw(h->raw_x, h->raw_y, h->raw_z,
                           h->ry, h->rx, h->rz, &root_mat);
        tm_build_obstacle_slot_like_original(&banks[h->bank], h->slot,
                                             &root_mat, 1,
                                             h->raw_x, h->raw_z,
                                             (h->flags & 4u) << 1,
                                             0, seen, objs, cap, &count);
        free(seen);
    }

    return count;
}

static int tm_parse_level_instances(const uint8_t *raw, uint32_t raw_size,
                                    TmVert *vbuf, int vcap,
                                    TmTri *tribuf, int tcap, int *out_ntris)
{
    TmBank banks[8];
    TmHead heads[1024];
    TmObjLight lights[512];
    TmJuncNode juncs[2048];
    TmRseg rsegs[4096];
    TmRouteType rtypes[64];
    int nbanks = 0, nheads = 0, nlights = 0, njuncs = 0, nrsegs = 0, nrtypes = 0;
    int nvtx = 0, ntris = 0, bad = 0, roots = 0;
    int junc_patch_tris = 0, route_tris = 0;
    char root[4] = {0,0,0,0};
    const uint8_t *bsp_payload = NULL;
    uint32_t bsp_size = 0;

    memset(banks, 0, sizeof(banks));
    memset(heads, 0, sizeof(heads));
    memset(lights, 0, sizeof(lights));
    memset(juncs, 0, sizeof(juncs));
    memset(rsegs, 0, sizeof(rsegs));
    memset(rtypes, 0, sizeof(rtypes));
    tm_collect_level_r(raw, 0, raw_size, root,
                       banks, &nbanks, 8, heads, &nheads, 1024,
                       lights, &nlights, 512,
                       juncs, &njuncs, 2048,
                       rsegs, &nrsegs, 4096,
                       rtypes, &nrtypes, 64,
                       &bsp_payload, &bsp_size);
#if defined(V8_HAVE_SDL) && defined(V8_HAVE_GL)
    if (nbanks > 0 && banks[0].atlas.tex != 0) {
        g_terrainmesh_tex = banks[0].atlas.tex;
        g_terrainmesh_tex_w = banks[0].atlas.w;
        g_terrainmesh_tex_h = banks[0].atlas.h;
        fprintf(stderr, "v8: TerrainMesh -- XOBF texture atlas %dx%d slots=%d\n",
                banks[0].atlas.w, banks[0].atlas.h, banks[0].atlas.slots);
    }
#endif
    tm_log_head_name_summary(heads, nheads);
    tm_build_spawn_placeholders(heads, nheads);

    {
        int obstacle_cap = 8192;
        TmObstacleObj *objs = (TmObstacleObj *)calloc((size_t)obstacle_cap,
                                                      sizeof(TmObstacleObj));
        int nobjs = 0;
        if (objs != NULL) {
            nobjs = tm_build_obstacle_instances(banks, nbanks, heads, nheads,
                                                objs, obstacle_cap);
            tm_build_object_bsp(bsp_payload, bsp_size, objs, nobjs);
            free(g_obstacle_objs);
            if (nobjs > 0) {
                g_obstacle_objs = objs;
                g_obstacle_nobjs = nobjs;
                g_obstacle_raw = (uint8_t *)raw;
            } else {
                free(objs);
                g_obstacle_objs = NULL;
                g_obstacle_nobjs = 0;
                g_obstacle_raw = NULL;
            }
        }
    }

    for (int hi = 0; hi < nheads; hi++) {
        const TmHead *h = &heads[hi];
        /* Source: LOAD 801006f0 switch table maps HEAD type 5 to a real
         * 0x80 runtime object allocation, matrix init, and DAT_80065a50
         * insertion, but it does not call the slot-tree constructor
         * FUN_80021b80/FUN_8001ac44.  Until that runtime callback path is
         * decoded, do not draw it as a static slot-tree visual. */
        if (h->type == 1 || h->type == 5 || h->type == 6 || h->type > 6) continue;
        if (h->bank < 0 || h->bank >= nbanks || h->slot < 0) continue;
        if (banks[h->bank].slot_count == 0) continue;
        uint8_t *seen = (uint8_t *)calloc((size_t)banks[h->bank].slot_count, 1);
        if (!seen) continue;
        roots++;
        TmXform root_xf;
        tm_rot_yxz(h->ry, h->rx, h->rz, root_xf.m);
        root_xf.x = h->x;
        root_xf.y = -((float)h->raw_y * TM_DISPLAY_Y_SCALE);
        root_xf.z = h->z;
        tm_matrix_from_raw(h->raw_x, h->raw_y, h->raw_z,
                           h->ry, h->rx, h->rz, &root_xf.raw);
        tm_build_slot_like_original(&banks[h->bank], h->slot,
                                    &root_xf, 1, (h->flags & 4u) << 1,
                                    0, seen, vbuf, vcap, &nvtx,
                                    tribuf, tcap, &ntris, &bad);
        free(seen);
    }

    for (int ji = 0; ji < njuncs; ji++) {
        const TmJuncNode *j = &juncs[ji];
        if (!j->has_patch) continue;
        if (j->bank < 0 || j->bank >= nbanks) continue;
        if (banks[j->bank].slot_count == 0) continue;
        int before = ntris;
        tm_emit_junc_patch_group(&banks[j->bank], j,
                                 vbuf, vcap, &nvtx,
                                 tribuf, tcap, &ntris, &bad);
        junc_patch_tris += ntris - before;
    }

    /* RSEG/XRTP records describe the route/navigation surface used by source
     * loaders and audits. The authored placed XOBF/JUNC geometry already
     * provides the visible props/surfaces, so uploading synthetic route strips
     * here draws a second coplanar layer through fences, bridges, and roads. */
    route_tris = 0;

    fprintf(stderr, "v8: TerrainMesh -- instanced XOBF banks=%d heads=%d roots=%d junc_nodes=%d rsegs=%d rtypes=%d junc_tris=%d route_tris=%d cpu_tris=%d gpu_vtx=%d bad_pkts=%d\n",
            nbanks, nheads, roots, njuncs, nrsegs, nrtypes,
            junc_patch_tris, route_tris, ntris, nvtx, bad);
    fprintf(stderr, "v8: TerrainMesh -- texture uv ground_xbmp=%d ground_none=%d xobf=%d untextured=%d\n",
            g_tm_uv_ground_xbmp, g_tm_uv_ground_none,
            g_tm_uv_xobf, g_tm_uv_none);
    fprintf(stderr, "v8: TerrainMesh -- placed obstacle leaf objects=%d\n",
            g_obstacle_nobjs);
    if (g_object_bsp != NULL) {
        int leaves = 0, occupied = 0, max_leaf = 0;
        for (int i = 0; i < g_object_bsp_nnodes; i++) {
            if (g_object_bsp[i].kind == 0) {
                leaves++;
                if (g_object_bsp[i].count > 0) occupied++;
                if (g_object_bsp[i].count > max_leaf) max_leaf = g_object_bsp[i].count;
            }
        }
        fprintf(stderr, "v8: TerrainMesh -- object BSP nodes=%d leaves=%d occupied=%d max_leaf=%d\n",
                g_object_bsp_nnodes, leaves, occupied, max_leaf);
    }
    if (nlights > 0) {
        int type_counts[8] = {0};
        int32_t hx0 = INT32_MAX, hx1 = INT32_MIN;
        int32_t hz0 = INT32_MAX, hz1 = INT32_MIN;
        int32_t r0 = INT32_MAX, r1 = INT32_MIN;
        int32_t cmin0 = INT32_MAX, cmin1 = INT32_MIN;
        int32_t cmax0 = INT32_MAX, cmax1 = INT32_MIN;
        int32_t inten0 = INT32_MAX, inten1 = INT32_MIN;
        for (int i = 0; i < nlights; i++) {
            if (lights[i].head_type >= 0 && lights[i].head_type < 8) {
                type_counts[lights[i].head_type]++;
            }
            if (lights[i].raw_x < hx0) hx0 = lights[i].raw_x;
            if (lights[i].raw_x > hx1) hx1 = lights[i].raw_x;
            if (lights[i].raw_z < hz0) hz0 = lights[i].raw_z;
            if (lights[i].raw_z > hz1) hz1 = lights[i].raw_z;
            if (lights[i].outer_radius < r0) r0 = lights[i].outer_radius;
            if (lights[i].outer_radius > r1) r1 = lights[i].outer_radius;
            if (lights[i].cone_min < cmin0) cmin0 = lights[i].cone_min;
            if (lights[i].cone_min > cmin1) cmin1 = lights[i].cone_min;
            if (lights[i].cone_max < cmax0) cmax0 = lights[i].cone_max;
            if (lights[i].cone_max > cmax1) cmax1 = lights[i].cone_max;
            if (lights[i].intensity < inten0) inten0 = lights[i].intensity;
            if (lights[i].intensity > inten1) inten1 = lights[i].intensity;
        }
        fprintf(stderr, "v8: TerrainMesh -- OBJ/LGHT lights=%d types:",
                nlights);
        for (int t = 0; t < 8; t++) {
            if (type_counts[t] != 0) fprintf(stderr, " %d:%d", t, type_counts[t]);
        }
        fprintf(stderr, " head_x=[%.1f..%.1f] head_z=[%.1f..%.1f] outer_radius=[%.1f..%.1f] cone_min=[%d..%d] cone_max=[%d..%d] intensity=[%d..%d]\n",
                (float)hx0 / 65536.0f, (float)hx1 / 65536.0f,
                (float)hz0 / 65536.0f, (float)hz1 / 65536.0f,
                (float)r0 / 65536.0f, (float)r1 / 65536.0f,
                cmin0, cmin1, cmax0, cmax1, inten0, inten1);
    }
    *out_ntris = ntris;
    return nvtx;
}

static void tm_log_tri_extents(const char *label, const TmTri *tris, int ntris)
{
    if (ntris <= 0) return;
    float xmin=1e30f,xmax=-1e30f,ymin=1e30f,ymax=-1e30f,zmin=1e30f,zmax=-1e30f;
    for (int ti = 0; ti < ntris; ti++) {
        float xs[3]={tris[ti].ax,tris[ti].bx,tris[ti].cx};
        float ys[3]={tris[ti].ay,tris[ti].by,tris[ti].cy};
        float zs[3]={tris[ti].az,tris[ti].bz,tris[ti].cz};
        for (int k = 0; k < 3; k++) {
            if (xs[k] < xmin) xmin = xs[k];
            if (xs[k] > xmax) xmax = xs[k];
            if (ys[k] < ymin) ymin = ys[k];
            if (ys[k] > ymax) ymax = ys[k];
            if (zs[k] < zmin) zmin = zs[k];
            if (zs[k] > zmax) zmax = zs[k];
        }
    }
    fprintf(stderr, "v8: %s extents: X=[%.1f .. %.1f] (%.1fm wide)\n",
            label, xmin, xmax, xmax - xmin);
    fprintf(stderr, "v8: %s extents: Y=[%.1f .. %.1f] (%.1fm tall)\n",
            label, ymin, ymax, ymax - ymin);
    fprintf(stderr, "v8: %s extents: Z=[%.1f .. %.1f] (%.1fm deep)\n",
            label, zmin, zmax, zmax - zmin);
}

static void tm_log_ground_extents(const char *label, const TmTri *tris, int ntris)
{
    float xmin=1e30f,xmax=-1e30f,ymin=1e30f,ymax=-1e30f,zmin=1e30f,zmax=-1e30f;
    int count = 0;
    for (int ti = 0; ti < ntris; ti++) {
        const TmTri *t = &tris[ti];
        if (!tm_tri_is_ground(t->ax, t->ay, t->az,
                              t->bx, t->by, t->bz,
                              t->cx, t->cy, t->cz, t->ny)) continue;
        float xs[3]={t->ax,t->bx,t->cx};
        float ys[3]={t->ay,t->by,t->cy};
        float zs[3]={t->az,t->bz,t->cz};
        for (int k = 0; k < 3; k++) {
            if (xs[k] < xmin) xmin = xs[k];
            if (xs[k] > xmax) xmax = xs[k];
            if (ys[k] < ymin) ymin = ys[k];
            if (ys[k] > ymax) ymax = ys[k];
            if (zs[k] < zmin) zmin = zs[k];
            if (zs[k] > zmax) zmax = zs[k];
        }
        count++;
    }
    if (count <= 0) {
        fprintf(stderr, "v8: %s ground candidates: 0\n", label);
        return;
    }
    fprintf(stderr, "v8: %s ground candidates=%d extents X=[%.1f .. %.1f] Z=[%.1f .. %.1f] Y=[%.1f .. %.1f]\n",
            label, count, xmin, xmax, zmin, zmax, ymin, ymax);
}

static void tm_log_z_coverage(const char *label, const TmTri *tris, int ntris)
{
    if (ntris <= 0) return;
    float zmin = 1e30f, zmax = -1e30f;
    for (int ti = 0; ti < ntris; ti++) {
        float cz = (tris[ti].az + tris[ti].bz + tris[ti].cz) * (1.0f / 3.0f);
        if (cz < zmin) zmin = cz;
        if (cz > zmax) zmax = cz;
    }
    if (zmax <= zmin) return;

    enum { TM_BINS = 8 };
    int total[TM_BINS] = {0};
    int up[TM_BINS] = {0};
    int ground[TM_BINS] = {0};
    for (int ti = 0; ti < ntris; ti++) {
        const TmTri *t = &tris[ti];
        float cz = (t->az + t->bz + t->cz) * (1.0f / 3.0f);
        int b = (int)((cz - zmin) * (float)TM_BINS / (zmax - zmin));
        if (b < 0) b = 0;
        if (b >= TM_BINS) b = TM_BINS - 1;
        total[b]++;
        if (t->ny >= 0.30f) up[b]++;
        if (tm_tri_is_ground(t->ax, t->ay, t->az,
                             t->bx, t->by, t->bz,
                             t->cx, t->cy, t->cz, t->ny)) {
            ground[b]++;
        }
    }

    fprintf(stderr, "v8: %s z coverage bins:", label);
    for (int b = 0; b < TM_BINS; b++) {
        float a = zmin + (zmax - zmin) * (float)b / (float)TM_BINS;
        float c = zmin + (zmax - zmin) * (float)(b + 1) / (float)TM_BINS;
        fprintf(stderr, " [%.0f..%.0f] %d/%d/%d", a, c, total[b], up[b], ground[b]);
    }
    fprintf(stderr, "\n");
}

/* ---- Parse all bones from BIN, emit triangles into vbuf + tribuf ---- */
/* Returns vertex count (for GL); also sets *out_ntris (CPU triangle count). */
static int tm_parse_bin(const uint8_t *B, uint32_t bsz,
                        TmVert *vbuf, int vcap,
                        TmTri  *tribuf, int tcap, int *out_ntris,
                        float ox, float oy, float oz)
{
    if (bsz < 8) { *out_ntris = 0; return 0; }
    uint32_t nb     = tm_rd32le(B, 0);
    uint32_t btbase = tm_rd32le(B, 4);
    int nvtx  = 0;
    int ntris = 0;
    int bad   = 0;

    static const float LX = 0.408f, LY = 0.816f, LZ = -0.408f;

    for (uint32_t bi = 0; bi < nb; bi++) {
        if (btbase + bi*4 + 4 > bsz) break;
        uint32_t rel = tm_rd32le(B, btbase + bi*4);
        uint32_t bd  = btbase + rel;
        if (bd + 0x18 > bsz) break;

        uint32_t vc = tm_rd32le(B, bd + 0x00);
        uint32_t vr = tm_rd32le(B, bd + 0x04);
        uint16_t pc = tm_rd16le(B, bd + 0x10);
        uint32_t pr = tm_rd32le(B, bd + 0x14);

        uint32_t vo = bd + vr;
        uint32_t po = bd + pr;
        float bone_xmin=1e30f, bone_xmax=-1e30f;
        float bone_ymin=1e30f, bone_ymax=-1e30f;
        float bone_zmin=1e30f, bone_zmax=-1e30f;
        int bone_has_verts = 0;

        for (uint32_t vscan = 0; vscan < vc; vscan++) {
            uint32_t oe = vo + vscan*8;
            if (oe + 6 > bsz) break;
            float sx =  tm_rds16le(B, oe+0) * TERR_SCALE + ox;
            float sy = -tm_rds16le(B, oe+2) * TERR_SCALE + oy;
            float sz =  tm_rds16le(B, oe+4) * TERR_SCALE + oz;
            if (sx < bone_xmin) bone_xmin = sx;
            if (sx > bone_xmax) bone_xmax = sx;
            if (sy < bone_ymin) bone_ymin = sy;
            if (sy > bone_ymax) bone_ymax = sy;
            if (sz < bone_zmin) bone_zmin = sz;
            if (sz > bone_zmax) bone_zmax = sz;
            bone_has_verts = 1;
        }
        (void)bone_xmin; (void)bone_xmax;
        (void)bone_ymin; (void)bone_ymax;
        (void)bone_zmin; (void)bone_zmax;
        (void)bone_has_verts;

        for (uint16_t pi = 0; pi < pc; pi++) {
            if (po + 4 > bsz) break;
            uint8_t typ = B[po + 3];
            int nib = typ & 0xf;
            int sz  = TM_PKT_SIZE[nib];
            if (sz == 0 || po + (uint32_t)sz > bsz) {
                bad++; po += 4; continue;
            }

            uint16_t vi[4];
            vi[0] = tm_rd16le(B, po+4);
            vi[1] = tm_rd16le(B, po+6);
            vi[2] = tm_rd16le(B, po+8);
            vi[3] = TM_IS_QUAD[nib] ? tm_rd16le(B, po+10) : vi[2];

            int nv = TM_IS_QUAD[nib] ? 4 : 3;
            int ok = 1;
            for (int k = 0; k < nv; k++)
                if (vi[k] >= vc) { ok = 0; break; }

            if (ok) {
                float vx[4], vy[4], vz[4];
                for (int k = 0; k < nv; k++) {
                    uint32_t oe = vo + vi[k]*8;
                    if (oe + 6 > bsz) { ok = 0; break; }
                    vx[k] =  tm_rds16le(B, oe+0) * TERR_SCALE + ox;
                    vy[k] = -tm_rds16le(B, oe+2) * TERR_SCALE + oy;
                    vz[k] =  tm_rds16le(B, oe+4) * TERR_SCALE + oz;
                }
                if (!ok) { po += sz; continue; }

                /* Face normal from cross(B-A, C-A) in GL (Y-up) space.
                 * PSX BIN polygon winding is CCW for a viewer BELOW the surface
                 * (PSX Y-down convention).  After Y-flip to OpenGL (Y-up), that
                 * same winding is CW when viewed from ABOVE -> GL back face ->
                 * culled.  Fix: negate the computed normal so stored tny > 0
                 * for GL-upward (drivable) surfaces (consistent with the
                 * HeightAt filter ny > 0.1), and emit vertices as A,C,B instead
                 * of A,B,C to make the face CCW from above -> front face. */
                float ex = vx[1]-vx[0], ey = vy[1]-vy[0], ez = vz[1]-vz[0];
                float fx = vx[2]-vx[0], fy = vy[2]-vy[0], fz = vz[2]-vz[0];
                /* Raw cross(B-A, C-A): tny < 0 for PSX-upward terrain after Y-flip. */
                float tnx = ey*fz-ez*fy, tny = ez*fx-ex*fz, tnz = ex*fy-ey*fx;
                float nl = sqrtf(tnx*tnx + tny*tny + tnz*tnz);
                if (nl > 1e-6f) { tnx/=nl; tny/=nl; tnz/=nl; }
                /* Negate -> GL-upward normal (tny > 0 for terrain, matching filter). */
                tnx=-tnx; tny=-tny; tnz=-tnz;

                /* Flat-shaded diagnostic colour.  Keep one colour per face so
                 * terrain/props are readable before texture/material binding. */
                float ndotl = tnx*LX + tny*LY + tnz*LZ;
                float lit   = 0.42f + 0.45f * (ndotl > 0 ? ndotl : 0);
                float lr = 0.34f * lit;
                float lg = 0.38f * lit;
                float lb = 0.36f * lit;

                int ground0 = tm_is_render_ground_tri(vx[0], vy[0], vz[0],
                                                      vx[1], vy[1], vz[1],
                                                      vx[2], vy[2], vz[2], tny);
                if (ground0) {
                    float gy = (vy[0] + vy[1] + vy[2]) * (1.0f / 3.0f);
                    float ht = (gy + 64.0f) / 137.0f;
                    if (ht < 0.0f) ht = 0.0f;
                    if (ht > 1.0f) ht = 1.0f;
                    lr = 0.22f + 0.20f * ht;
                    lg = 0.30f + 0.18f * ht;
                    lb = 0.24f + 0.12f * (1.0f - ht);
                }

                /* --- First triangle: A, C, B (CCW from above in GL space) --- */
                if (ntris < tcap) {
                    TmTri *tt = &tribuf[ntris++];
                    tt->ax=vx[0]; tt->ay=vy[0]; tt->az=vz[0];
                    tt->bx=vx[1]; tt->by=vy[1]; tt->bz=vz[1];
                    tt->cx=vx[2]; tt->cy=vy[2]; tt->cz=vz[2];
                    tt->nx=tnx; tt->ny=tny; tt->nz=tnz;
                    tt->pd = tnx*vx[0] + tny*vy[0] + tnz*vz[0];
                }
                if (nvtx + 3 <= vcap) {
                    vbuf[nvtx++] = (TmVert){vx[0],vy[0],vz[0],lr,lg,lb,-1.0f,-1.0f,0.0f};  /* A */
                    vbuf[nvtx++] = (TmVert){vx[2],vy[2],vz[2],lr,lg,lb,-1.0f,-1.0f,0.0f};  /* C */
                    vbuf[nvtx++] = (TmVert){vx[1],vy[1],vz[1],lr,lg,lb,-1.0f,-1.0f,0.0f};  /* B */
                }

                /* --- Quad second triangle: A, D, C (CCW from above) --- */
                if (TM_IS_QUAD[nib]) {
                    int ground1 = tm_is_render_ground_tri(vx[0], vy[0], vz[0],
                                                          vx[2], vy[2], vz[2],
                                                          vx[3], vy[3], vz[3], tny);
                    float lr1 = 0.34f * lit, lg1 = 0.38f * lit, lb1 = 0.36f * lit;
                    if (ground1) {
                        float gy = (vy[0] + vy[2] + vy[3]) * (1.0f / 3.0f);
                        float ht = (gy + 64.0f) / 137.0f;
                        if (ht < 0.0f) ht = 0.0f;
                        if (ht > 1.0f) ht = 1.0f;
                        lr1 = 0.22f + 0.20f * ht;
                        lg1 = 0.30f + 0.18f * ht;
                        lb1 = 0.24f + 0.12f * (1.0f - ht);
                    }
                    if (ntris < tcap) {
                        TmTri *tt = &tribuf[ntris++];
                        tt->ax=vx[0]; tt->ay=vy[0]; tt->az=vz[0];
                        tt->bx=vx[2]; tt->by=vy[2]; tt->bz=vz[2];
                        tt->cx=vx[3]; tt->cy=vy[3]; tt->cz=vz[3];
                        tt->nx=tnx; tt->ny=tny; tt->nz=tnz;
                        tt->pd = tnx*vx[0] + tny*vy[0] + tnz*vz[0];
                    }
                    if (nvtx + 3 <= vcap) {
                        vbuf[nvtx++] = (TmVert){vx[0],vy[0],vz[0],lr1,lg1,lb1,-1.0f,-1.0f,0.0f};  /* A */
                        vbuf[nvtx++] = (TmVert){vx[3],vy[3],vz[3],lr1,lg1,lb1,-1.0f,-1.0f,0.0f};  /* D */
                        vbuf[nvtx++] = (TmVert){vx[2],vy[2],vz[2],lr1,lg1,lb1,-1.0f,-1.0f,0.0f};  /* C */
                    }
                }
            }
            po += sz;
        }
    }
    fprintf(stderr, "v8: TerrainMesh -- bones=%u, cpu_tris=%d, gpu_vtx=%d, bad_pkts=%d\n",
            nb, ntris, nvtx, bad);
    /* Diagnostic: world-space extents of all vertices. */
    if (ntris > 0) {
        float xmin=1e30f,xmax=-1e30f,ymin=1e30f,ymax=-1e30f,zmin=1e30f,zmax=-1e30f;
        for (int ti = 0; ti < ntris; ti++) {
            float xs[3]={tribuf[ti].ax,tribuf[ti].bx,tribuf[ti].cx};
            float ys[3]={tribuf[ti].ay,tribuf[ti].by,tribuf[ti].cy};
            float zs[3]={tribuf[ti].az,tribuf[ti].bz,tribuf[ti].cz};
            for (int k=0;k<3;k++){
                if(xs[k]<xmin)xmin=xs[k]; if(xs[k]>xmax)xmax=xs[k];
                if(ys[k]<ymin)ymin=ys[k]; if(ys[k]>ymax)ymax=ys[k];
                if(zs[k]<zmin)zmin=zs[k]; if(zs[k]>zmax)zmax=zs[k];
            }
        }
        fprintf(stderr, "v8: XOBF world extents: X=[%.1f .. %.1f] (%.1fm wide)\n",
                xmin, xmax, xmax-xmin);
        fprintf(stderr, "v8: XOBF world extents: Y=[%.1f .. %.1f] (%.1fm tall)\n",
                ymin, ymax, ymax-ymin);
        fprintf(stderr, "v8: XOBF world extents: Z=[%.1f .. %.1f] (%.1fm deep)\n",
                zmin, zmax, zmax-zmin);
    }
    *out_ntris = ntris;
    return nvtx;
}

/* ------------------------------------------------------------------ */
/* Public: load, parse, upload.                                         */
/* ------------------------------------------------------------------ */
void TerrainMesh_Load(const char *exp_path,
                      float world_x_centre, float world_z_centre)
{
    tm_clear_obstacles();
    g_terrainmesh_xbmp_w = 0;
    g_terrainmesh_xbmp_h = 0;
    g_terrainmesh_xbmp_x = 0;
    g_terrainmesh_xbmp_y = 0;
    g_tm_uv_ground_xbmp = 0;
    g_tm_uv_ground_xbmp_oob = 0;
    g_tm_uv_ground_none = 0;
    g_tm_uv_xobf = 0;
    g_tm_uv_none = 0;
    g_terrainmesh_sky_w = 0;
    g_terrainmesh_sky_h = 0;

    uint32_t fsz = 0;
    uint8_t *raw = tm_load_exp_blob(exp_path, &fsz, "TerrainMesh");
    if (!raw) {
        return;
    }
    g_terrainmesh_xbmp_tex = tm_upload_xbmp_texture(raw, fsz);
    g_terrainmesh_sky_tex = tm_upload_xbgm_texture(raw, fsz);

    /* Allocate vertex + triangle buffers (generous cap). */
    int cap    = 500000;
    int tricap = cap / 3;
    TmVert *vbuf   = (TmVert *)malloc(sizeof(TmVert) * (size_t)cap);
    TmTri  *tribuf = (TmTri  *)malloc(sizeof(TmTri)  * (size_t)tricap);
    if (!vbuf || !tribuf) {
        free(vbuf);
        free(tribuf);
        Heap_Free(raw);
        return;
    }

    int ntris = 0;
    int nvtx  = tm_parse_level_instances(raw, fsz,
                                          vbuf, cap,
                                          tribuf, tricap, &ntris);
    if (nvtx == 0) {
        uint32_t bin_off = 0, bin_sz = 0;
        if (tm_find_xobf_bin(raw, fsz, &bin_off, &bin_sz)) {
            fprintf(stderr, "v8: TerrainMesh -- instanced parse empty; fallback BIN @0x%x size 0x%x, origin=(%.1f, %.2f, %.1f)\n",
                    bin_off, bin_sz, world_x_centre, TERR_Y_ORIGIN, world_z_centre);
            nvtx = tm_parse_bin(raw + bin_off, bin_sz,
                                vbuf, cap,
                                tribuf, tricap, &ntris,
                                world_x_centre, TERR_Y_ORIGIN, world_z_centre);
        }
    }
    if (g_obstacle_raw != raw) {
        Heap_Free(raw);
    }

    /* Retain CPU-side triangle data for TerrainMesh_HeightAt(). */
    g_terrain_tris  = tribuf;   /* NOT freed -- kept for process lifetime */
    g_terrain_ntris = ntris;
    {
        int ground_tris = 0;
        for (int ti = 0; ti < ntris; ti++) {
            float cy = (tribuf[ti].ay + tribuf[ti].by + tribuf[ti].cy) * (1.0f/3.0f);
            (void)cy;
            if (tm_tri_is_ground(tribuf[ti].ax, tribuf[ti].ay, tribuf[ti].az,
                                 tribuf[ti].bx, tribuf[ti].by, tribuf[ti].bz,
                                 tribuf[ti].cx, tribuf[ti].cy, tribuf[ti].cz,
                                 tribuf[ti].ny))
                ground_tris++;
        }
        fprintf(stderr, "v8: TerrainMesh -- %d CPU tris retained (%d upward driveable candidates)\n",
                ntris, ground_tris);
        tm_log_tri_extents("TerrainMesh placed XOBF", tribuf, ntris);
        tm_log_ground_extents("TerrainMesh placed XOBF", tribuf, ntris);
        tm_log_z_coverage("TerrainMesh placed XOBF", tribuf, ntris);
    }

    if (nvtx == 0) {
        free(vbuf);
        fprintf(stderr, "v8: TerrainMesh -- 0 vertices parsed\n");
        return;
    }

    /* Upload to GL. */
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(nvtx * sizeof(TmVert)),
                 vbuf, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TmVert),
                          (void *)offsetof(TmVert, x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TmVert),
                          (void *)offsetof(TmVert, r));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TmVert),
                          (void *)offsetof(TmVert, u));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(TmVert),
                          (void *)offsetof(TmVert, tex));
    glBindVertexArray(0);

    g_terrainmesh_vao = vao;
    g_terrainmesh_vtx = nvtx;
    fprintf(stderr, "v8: TerrainMesh -- uploaded %d vertices (%d tris)\n",
            nvtx, nvtx/3);
    free(vbuf);   /* GPU has a copy; CPU vertex buffer no longer needed */
}

void TerrainMesh_LoadCpuOnly(const char *exp_path,
                             float world_x_centre, float world_z_centre)
{
    tm_clear_obstacles();
    g_terrainmesh_xbmp_w = 0;
    g_terrainmesh_xbmp_h = 0;
    g_terrainmesh_xbmp_x = 0;
    g_terrainmesh_xbmp_y = 0;
    g_tm_uv_ground_xbmp = 0;
    g_tm_uv_ground_xbmp_oob = 0;
    g_tm_uv_ground_none = 0;
    g_tm_uv_xobf = 0;
    g_tm_uv_none = 0;

    uint32_t fsz = 0;
    uint8_t *raw = tm_load_exp_blob(exp_path, &fsz, "TerrainMesh CPU");
    if (!raw) {
        return;
    }

    int cap    = 500000;
    int tricap = cap / 3;
    TmVert *vbuf   = (TmVert *)malloc(sizeof(TmVert) * (size_t)cap);
    TmTri  *tribuf = (TmTri  *)malloc(sizeof(TmTri)  * (size_t)tricap);
    if (!vbuf || !tribuf) {
        free(vbuf);
        free(tribuf);
        Heap_Free(raw);
        return;
    }

    int ntris = 0;
    int nvtx = tm_parse_level_instances(raw, fsz,
                                        vbuf, cap,
                                        tribuf, tricap, &ntris);
    if (nvtx == 0) {
        uint32_t bin_off = 0, bin_sz = 0;
        if (tm_find_xobf_bin(raw, fsz, &bin_off, &bin_sz)) {
            fprintf(stderr, "v8: TerrainMesh CPU -- instanced parse empty; fallback BIN @0x%x size 0x%x\n",
                    bin_off, bin_sz);
            (void)tm_parse_bin(raw + bin_off, bin_sz,
                               vbuf, cap,
                               tribuf, tricap, &ntris,
                               world_x_centre, TERR_Y_ORIGIN, world_z_centre);
        }
    }
    if (g_obstacle_raw != raw) {
        Heap_Free(raw);
    }
    free(vbuf);

    if (g_terrain_tris != NULL) {
        free(g_terrain_tris);
    }
    g_terrain_tris  = tribuf;
    g_terrain_ntris = ntris;

    int ground_tris = 0;
    for (int ti = 0; ti < ntris; ti++) {
        if (tm_tri_is_ground(tribuf[ti].ax, tribuf[ti].ay, tribuf[ti].az,
                             tribuf[ti].bx, tribuf[ti].by, tribuf[ti].bz,
                             tribuf[ti].cx, tribuf[ti].cy, tribuf[ti].cz,
                             tribuf[ti].ny))
            ground_tris++;
    }
    fprintf(stderr, "v8: TerrainMesh CPU -- loaded %d tris (%d upward driveable candidates), origin=(%.1f, %.2f, %.1f)\n",
            ntris, ground_tris, world_x_centre, TERR_Y_ORIGIN, world_z_centre);
    tm_log_tri_extents("TerrainMesh CPU placed XOBF", tribuf, ntris);
    tm_log_ground_extents("TerrainMesh CPU placed XOBF", tribuf, ntris);
    tm_log_z_coverage("TerrainMesh CPU placed XOBF", tribuf, ntris);
}

#else  /* no SDL/GL */

void TerrainMesh_Load(const char *p, float x, float z) {
    (void)p; (void)x; (void)z;
    /* g_terrain_tris stays NULL; TerrainMesh_HeightAt returns 0 (no coverage). */
}

void TerrainMesh_LoadCpuOnly(const char *p, float x, float z) {
    (void)p; (void)x; (void)z;
}

#endif
