/* terrain_mesh.c -- load and upload the visual level mesh from the EXP.
 *
 * The terrain EXP (e.g. OILFIELD.EXP) contains FORM XOBF banks.  The runtime
 * HEAD records select a bank and slot; FUN_8001ac44 walks the slot tree and
 * FUN_8001be5c draws the render group cached by FUN_8001b49c.
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
 * Renderer note: all triangles are retained in the CPU store for source-path
 * verification.  TerrainMesh_HeightAt() still filters the CPU store to upward
 * ground-like faces, so walls and props do not become driveable merely because
 * they are visible in the renderer.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include <math.h>
#include <ctype.h>
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
    uint8_t coll_shape[0x20];
    const uint8_t *probe_stream;
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
static uint8_t      **g_spawn_placeholders = NULL;
static int            g_spawn_nplaceholders = 0;
static TmObjectBspNode *g_object_bsp = NULL;
static int              g_object_bsp_nnodes = 0;
static int              g_object_bsp_root = -1;

extern void  Heap_Free(void *p);
extern void *FUN_8001178c(uint32_t size, uint32_t mode);
extern uint32_t *Object_BuildFromBin(int *templateBody, void *animPtr);
extern intptr_t FUN_80021b80(intptr_t (*cb)(intptr_t, int, int),
                             intptr_t bank, uint16_t kind, uint32_t flags);
extern uint32_t FUN_800223dc(uint32_t *self, int mode, intptr_t arg);
extern void     FUN_8001d708(uint32_t *obj);
extern int      FUN_8001dc1c(intptr_t obj);
extern int      FUN_8001b0c4(uint32_t *obj);
extern uint32_t FUN_8001ec48(uint32_t *obj);
extern uint32_t FUN_8002036c(uint32_t *obj);
extern intptr_t FUN_80021c6c(uint32_t *placeholder);
extern void    *FUN_8001d470(uint32_t size);
extern void     FUN_8001fe50(uintptr_t listSentinel, uintptr_t payload);
extern void     FUN_80101574(intptr_t obj, void *parentMatrix);
extern uint8_t  DAT_80065a50[];
extern int8_t   cRam00000016;
extern char     g_v8_level_exp_path[128];
extern uintptr_t iRam000006fc;
extern uintptr_t uRam000006fc;
extern void RotMatrixYXZ_gte(const SVECTOR *r, MATRIX *m);
extern long CompMatrixLV(const MATRIX *m0, const MATRIX *m1, MATRIX *m2);
extern int32_t Terrain_HeightAt(uint32_t x, uint32_t z);
extern uintptr_t DAT_800911a0[32 * 32];
extern int rsin(int a);
extern int rcos(int a);
extern int Object_FindObstacleAt(int *parent_obj, int terrain_y,
                                 int *posXyz, int16_t *normalOut);
extern void Object_SetCallbackPsxSlot(void *obj, uintptr_t callback);

uintptr_t g_v8_static_collision_objs[512];
int g_v8_static_collision_obj_count;

typedef struct {
    uintptr_t obj;
    char name[48];
    int head_index;
    int slot;
} TmStaticDebugInfo;

static TmStaticDebugInfo g_static_debug[512];
static int g_static_debug_count;

extern uint32_t AG_TrackerDish(uint32_t *self, int mode, int *impulse);
extern uint32_t AG_CruiseMissile(uint32_t *self, int mode, int *impulse);
extern uint32_t AG_B17HitTick(uint32_t *self, int mode, uint32_t *impulse);
extern uint32_t CL_Launcher(uint32_t *self, int mode, int *impulse);
extern uint32_t FUN_80100cbc(uint32_t *self, uint32_t mode, uint32_t *impulse);
extern uint32_t CC_BlimpTick(uint32_t *self, int mode, int arg);
extern uint32_t FUN_801004e8(uint32_t *parent, uint32_t mode, int *impulse);
extern uint32_t CC_BlimpMain(uint32_t *self, int mode, uint32_t *impulse);
extern uint32_t FUN_80101ca8(uint32_t obj, int mode);
extern uint32_t FUN_80101fe0(int obj, int mode);
extern uint32_t FUN_8010036c(void *obj, int mode);
extern uint32_t HD_TransformerAim(uint32_t *self, int mode, int arg);
extern uint32_t HD_SirenStrobe(uint32_t *self, int mode, uint32_t *impulse);
extern uint32_t FUN_80101118(int obj, uint32_t mode, int *impulse);
extern uint32_t HD_WaterBob(uint32_t *self, int mode, int *impulse);
extern uint32_t FUN_8010135c(int obj, uint32_t mode, void *impactCtx);
extern uint32_t FUN_801013e0(int obj, uint32_t mode, void *impactCtx);
extern uint32_t FUN_80101580(uint32_t *self, int mode);
extern uint32_t HD_SpillwayGrab(uint32_t *self, uint32_t mode, uint32_t *impulse);
extern uint32_t FUN_80101a98(int obj, uint32_t mode, void *impactCtx);
extern void     FUN_801001cc(uint32_t *self, int mode, uint32_t arg);
extern uint32_t FUN_80100870(uint32_t *parent, uint32_t mode, int32_t *impulse);
extern uint32_t FUN_80100540(int obj, uint32_t mode, void *impact);
extern void     FUN_80100e78(int self, uint32_t mode, void *impactCtx);
extern uint32_t OF_RigDestroy(int self, int mode, uint32_t arg);
extern uint32_t SF_ElevatorFullTick(int obj, int mode, void *arg);
extern uint32_t FUN_80100b34(int obj, int mode);
extern uint32_t SF_ConveyorGrab(int obj, int mode, int *impulse);
extern void     FUN_801010c4(int obj, int mode, int impact);
extern uint32_t SB_RadarSweep(int obj, int mode, int *impulse);
extern uint32_t SCRT_RadarTick(uint32_t *self, int mode, int *impulse);
extern uint32_t FUN_801006f4(int obj, uint32_t mode, void *impact);
extern uint32_t SB_TurretTrack(int obj, int mode, int *impulse);
extern uint32_t SB_SecurityDoor(int obj, uint32_t mode, int *impulse);
extern uint32_t FUN_80101acc(int self, uint32_t mode, int *impulse);
extern uint32_t SB_BunkerDoor(uint32_t *self, int mode, int *impulse);
extern uint32_t FUN_801025ec(int obj, uint32_t mode, int *impulse);
extern uint32_t SB_SiloDoor(uint32_t *self, int mode, int *impulse);
extern uint32_t SK_PowderBoulder(uint32_t *self, int mode, int arg);
extern uint32_t SR_LiftStation(uint32_t *self, int mode, uint32_t *impulse);
extern uint32_t FUN_80101424(int obj, int mode);
extern uint32_t SK_LiftChairGrab(uint32_t *self, int mode, int *impulse);
extern uint32_t SK_SnowPulse(uint32_t *self, int mode, int arg);
extern uint32_t SR_BallDestroy(int obj, int mode, uint32_t arg);
extern uint32_t SR_SnowEmit(uint32_t *self, int mode, int arg);
extern uint32_t FUN_80102094(int obj, uint32_t mode, void *impact);
extern uint32_t VF_GenericCleanup(uint32_t obj, int mode);
extern uint32_t VF_WindmillAlarm(uint32_t *self, int mode, uint32_t *impulse);
extern uint32_t VF_SiloRotate(int obj, int mode, int arg);
extern uint32_t FUN_80100c1c(uint32_t *self, int mode, int *impulse);
extern uint32_t VF_SiloSlide(uint32_t *self, int mode, int *impulse);
extern uint32_t VF_PumpTick(int obj, uint32_t mode, uint32_t arg);
extern uint32_t FUN_8010035c(int obj, uint32_t mode, int *impulse);
extern uint32_t FUN_801003ec(int obj, uint32_t mode, void *impactCtx);
extern uint32_t WW_TrainInit(uint32_t *self, int mode, uint32_t *impulse);
extern uint32_t WW_ShackTick(int obj, int mode, uint32_t arg);
extern uint32_t WW_DynamiteKeg(uint32_t *self, int mode, uint32_t *impulse);
extern uint32_t WW_GallowTick(uint32_t *self, int mode, uint32_t *impulse);
extern uint32_t WW_BridgeCollapse(uint32_t *self, int mode, int *impulse);
extern uint32_t WW_StageCoach(uint32_t *self, int mode, int *impulse);
extern uint32_t WW_SaloonDestruct(uint32_t *self, int mode, int *impulse);
extern intptr_t LAB_8003c61c(intptr_t obj, int event, intptr_t param3);

static int32_t tm_q12_mul_i32(int32_t a, int32_t b)
{
    return (int32_t)(((int64_t)a * (int64_t)b) >> 12);
}

static int32_t tm_q12_dot3_i32(int32_t a0, int32_t a1, int32_t a2,
                               int32_t b0, int32_t b1, int32_t b2)
{
    return (int32_t)(((int64_t)a0 * b0 + (int64_t)a1 * b1 + (int64_t)a2 * b2) >> 12);
}

static uintptr_t tm_resolve_head_callback(const char *levelPath,
                                          const char *headName,
                                          int *resolved);

static uintptr_t tm_resolve_global_head_callback(const char *headName)
{
    static const char *const pickup_names[] = {
        "PU_WeaponUpgrade", "PU_RadarJammer", "PU_Shield", "PU_Health",
        "I_RocktL", "I_MisslL", "I_Cannon", "I_Mortar", "I_MineDr",
        "I_Special", "I_Surprise", NULL
    };

    if (headName == NULL)
        return 0;
    for (int i = 0; pickup_names[i] != NULL; i++) {
        if (strcmp(headName, pickup_names[i]) == 0)
            return (uintptr_t)LAB_8003c61c;
    }
    return 0;
}

static void tm_clear_obstacles(void)
{
    Heap_Free(g_obstacle_objs);
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
    g_static_debug_count = 0;
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

int TerrainMesh_ObstacleProbeAt(int32_t pos_x, int32_t pos_y, int32_t pos_z,
                                int32_t terrain_y, int16_t *out_normal,
                                int32_t *out_y)
{
    int pos[3] = { pos_x, pos_y, pos_z };
    int16_t normal[4] = { 0, 0, 0, 0 };
    int16_t best_normal[4] = { 0, -4096, 0, 0 };
    int32_t best_y = 0;
    int found = 0;

    if (uRam000006fc != 0) {
        int32_t *node = (int32_t *)(uintptr_t)uRam000006fc;
        int guard = 0;

        while (node != NULL && guard++ < 4096) {
            if (node[0] == 0)
                break;
            if (node[0] == 1) {
                node = (int32_t *)(uintptr_t)(uint32_t)
                    node[node[1] < pos_x ? 3 : 2];
            } else if (node[0] == 2) {
                node = (int32_t *)(uintptr_t)(uint32_t)
                    node[node[1] < pos_z ? 3 : 2];
            } else {
                node = NULL;
            }
        }

        if (node != NULL && node[0] == 0 && node[1] != 0 && node[2] == 0) {
            int32_t *list = (int32_t *)(uintptr_t)(uint32_t)node[1];
            while (list != NULL && list[0] != 0) {
                int32_t *obj = (int32_t *)(uintptr_t)(uint32_t)list[2];
                if (obj != NULL && ((*(uint32_t *)obj & 0x20u) == 0)) {
                    int32_t hit = Object_FindObstacleAt((int *)obj,
                                                        terrain_y, pos, normal);
                    if (hit != 0 && (!found || hit < best_y)) {
                        best_y = hit;
                        best_normal[0] = normal[0];
                        best_normal[1] = normal[1];
                        best_normal[2] = normal[2];
                        best_normal[3] = normal[3];
                        found = 1;
                    }
                }
                list = (int32_t *)(uintptr_t)(uint32_t)list[0];
            }
        }
    }

    if (found)
        goto done;

    if (g_obstacle_objs == NULL || g_obstacle_nobjs <= 0) return 0;

    int leaf = tm_object_bsp_find_leaf(pos_x, pos_z);
    int i = (leaf >= 0) ? g_object_bsp[leaf].first_obj : 0;

    while (i >= 0 && i < g_obstacle_nobjs) {
        uint32_t saved_shape = *(uint32_t *)(g_obstacle_objs[i].obj + 0x5c);
        *(uint32_t *)(g_obstacle_objs[i].obj + 0x5c) =
            (uint32_t)(uintptr_t)g_obstacle_objs[i].probe_stream;
        int32_t hit = Object_FindObstacleAt((int *)g_obstacle_objs[i].obj,
                                            terrain_y, pos, normal);
        *(uint32_t *)(g_obstacle_objs[i].obj + 0x5c) = saved_shape;
        if (hit != 0 && (!found || hit < best_y)) {
            best_y = hit;
            best_normal[0] = normal[0];
            best_normal[1] = normal[1];
            best_normal[2] = normal[2];
            best_normal[3] = normal[3];
            found = 1;
        }
        i = (leaf >= 0) ? g_obstacle_objs[i].next_in_leaf : i + 1;
    }
done:
    if (found && out_normal != NULL) {
        out_normal[0] = best_normal[0];
        out_normal[1] = best_normal[1];
        out_normal[2] = best_normal[2];
        out_normal[3] = best_normal[3];
    }
    if (found && out_y != NULL) *out_y = best_y;
    return found;
}

int TerrainMesh_ObstacleHeightAt(int32_t pos_x, int32_t pos_y, int32_t pos_z,
                                 int32_t terrain_y, int32_t *out_y)
{
    return TerrainMesh_ObstacleProbeAt(pos_x, pos_y, pos_z,
                                       terrain_y, NULL, out_y);
}

static int32_t tm_abs_i32(int32_t v)
{
    return (v < 0) ? (int32_t)(0u - (uint32_t)v) : v;
}

static const TmStaticDebugInfo *tm_static_debug_for_obj(uintptr_t obj)
{
    for (int i = 0; i < g_static_debug_count; i++) {
        if (g_static_debug[i].obj == obj)
            return &g_static_debug[i];
    }
    return NULL;
}

static void tm_debug_transform_point_mat(const MATRIX *mat,
                                         int32_t lx, int32_t ly, int32_t lz,
                                         float *x, float *y, float *z)
{
    int64_t wx = (int64_t)mat->t[0]
               + ((int64_t)mat->m[0][0] * lx +
                  (int64_t)mat->m[0][1] * ly +
                  (int64_t)mat->m[0][2] * lz) / 4096;
    int64_t wy = (int64_t)mat->t[1]
               + ((int64_t)mat->m[1][0] * lx +
                  (int64_t)mat->m[1][1] * ly +
                  (int64_t)mat->m[1][2] * lz) / 4096;
    int64_t wz = (int64_t)mat->t[2]
               + ((int64_t)mat->m[2][0] * lx +
                  (int64_t)mat->m[2][1] * ly +
                  (int64_t)mat->m[2][2] * lz) / 4096;

    *x = (float)wx / 65536.0f;
    *y = -(float)wy / 65536.0f;
    *z = (float)wz / 65536.0f;
}

static void tm_debug_transform_point(const uint8_t *obj,
                                     int32_t lx, int32_t ly, int32_t lz,
                                     float *x, float *y, float *z)
{
    tm_debug_transform_point_mat((const MATRIX *)(obj + 0x10), lx, ly, lz,
                                 x, y, z);
}

static void tm_debug_local_point_mat(const MATRIX *mat,
                                     int32_t wx, int32_t wy, int32_t wz,
                                     int32_t *lx, int32_t *ly, int32_t *lz)
{
    int32_t dx = wx - mat->t[0];
    int32_t dy = wy - mat->t[1];
    int32_t dz = wz - mat->t[2];

    *lx = (int32_t)(((int64_t)mat->m[0][0] * dx +
                    (int64_t)mat->m[1][0] * dy +
                    (int64_t)mat->m[2][0] * dz) / 4096);
    *ly = (int32_t)(((int64_t)mat->m[0][1] * dx +
                    (int64_t)mat->m[1][1] * dy +
                    (int64_t)mat->m[2][1] * dz) / 4096);
    *lz = (int32_t)(((int64_t)mat->m[0][2] * dx +
                    (int64_t)mat->m[1][2] * dy +
                    (int64_t)mat->m[2][2] * dz) / 4096);
}

static void tm_debug_local_point(const uint8_t *obj,
                                 int32_t wx, int32_t wy, int32_t wz,
                                 int32_t *lx, int32_t *ly, int32_t *lz)
{
    tm_debug_local_point_mat((const MATRIX *)(obj + 0x10), wx, wy, wz,
                             lx, ly, lz);
}

static int tm_debug_obj_has_inside_box_mat(const uint8_t *obj,
                                           const MATRIX *mat,
                                           int32_t px, int32_t py, int32_t pz,
                                           int32_t *out_lx, int32_t *out_ly, int32_t *out_lz,
                                           int32_t *out_minx, int32_t *out_miny, int32_t *out_minz,
                                           int32_t *out_maxx, int32_t *out_maxy, int32_t *out_maxz)
{
    const uint8_t *shape = (const uint8_t *)(uintptr_t)*(const uint32_t *)(obj + 0x5c);
    int32_t lx, ly, lz;
    int guard = 0;

    if (shape == NULL)
        return 0;
    tm_debug_local_point_mat(mat, px, py, pz, &lx, &ly, &lz);
    while (guard++ < 128) {
        int16_t kind = *(const int16_t *)shape;
        if (kind == 0)
            break;
        if (kind == 1) {
            int32_t minx = *(const int32_t *)(shape + 4);
            int32_t miny = *(const int32_t *)(shape + 8);
            int32_t minz = *(const int32_t *)(shape + 12);
            int32_t maxx = *(const int32_t *)(shape + 16);
            int32_t maxy = *(const int32_t *)(shape + 20);
            int32_t maxz = *(const int32_t *)(shape + 24);
            if (lx > minx && lx < maxx &&
                ly > miny && ly < maxy &&
                lz > minz && lz < maxz) {
                if (out_lx) *out_lx = lx;
                if (out_ly) *out_ly = ly;
                if (out_lz) *out_lz = lz;
                if (out_minx) *out_minx = minx;
                if (out_miny) *out_miny = miny;
                if (out_minz) *out_minz = minz;
                if (out_maxx) *out_maxx = maxx;
                if (out_maxy) *out_maxy = maxy;
                if (out_maxz) *out_maxz = maxz;
                return 1;
            }
            shape += 0x1c;
        } else if (kind == 2) {
            shape += 4 + (size_t)*(const uint16_t *)(shape + 2) * 12;
        } else {
            break;
        }
    }
    return 0;
}

static int tm_debug_obj_has_inside_box(const uint8_t *obj,
                                       int32_t px, int32_t py, int32_t pz,
                                       int32_t *out_lx, int32_t *out_ly, int32_t *out_lz,
                                       int32_t *out_minx, int32_t *out_miny, int32_t *out_minz,
                                       int32_t *out_maxx, int32_t *out_maxy, int32_t *out_maxz)
{
    return tm_debug_obj_has_inside_box_mat(obj, (const MATRIX *)(obj + 0x10),
                                           px, py, pz,
                                           out_lx, out_ly, out_lz,
                                           out_minx, out_miny, out_minz,
                                           out_maxx, out_maxy, out_maxz);
}

static int tm_debug_obj_first_box(const uint8_t *obj,
                                  int32_t *out_minx, int32_t *out_miny, int32_t *out_minz,
                                  int32_t *out_maxx, int32_t *out_maxy, int32_t *out_maxz)
{
    const uint8_t *shape = (const uint8_t *)(uintptr_t)*(const uint32_t *)(obj + 0x5c);
    int guard = 0;

    while (shape != NULL && guard++ < 128) {
        int16_t kind = *(const int16_t *)shape;
        if (kind == 0)
            return 0;
        if (kind == 1) {
            if (out_minx) *out_minx = *(const int32_t *)(shape + 4);
            if (out_miny) *out_miny = *(const int32_t *)(shape + 8);
            if (out_minz) *out_minz = *(const int32_t *)(shape + 12);
            if (out_maxx) *out_maxx = *(const int32_t *)(shape + 16);
            if (out_maxy) *out_maxy = *(const int32_t *)(shape + 20);
            if (out_maxz) *out_maxz = *(const int32_t *)(shape + 24);
            return 1;
        }
        if (kind == 2) {
            shape += 4 + (size_t)*(const uint16_t *)(shape + 2) * 12;
        } else {
            return 0;
        }
    }
    return 0;
}

static int tm_debug_append_line(float *out, int max_vertices, int n,
                                float ax, float ay, float az,
                                float bx, float by, float bz,
                                float r, float g, float b)
{
    if (n + 2 > max_vertices)
        return n;
    out[n * 9 + 0] = ax; out[n * 9 + 1] = ay; out[n * 9 + 2] = az;
    out[n * 9 + 3] = r;  out[n * 9 + 4] = g;  out[n * 9 + 5] = b;
    out[n * 9 + 6] = -1.0f; out[n * 9 + 7] = -1.0f; out[n * 9 + 8] = 0.0f;
    n++;
    out[n * 9 + 0] = bx; out[n * 9 + 1] = by; out[n * 9 + 2] = bz;
    out[n * 9 + 3] = r;  out[n * 9 + 4] = g;  out[n * 9 + 5] = b;
    out[n * 9 + 6] = -1.0f; out[n * 9 + 7] = -1.0f; out[n * 9 + 8] = 0.0f;
    return n + 1;
}

static int tm_debug_append_box_mat(const MATRIX *mat, const uint8_t *shape,
                               float *out, int max_vertices, int n,
                               float r, float g, float b)
{
    static const uint8_t edges[12][2] = {
        {0,1},{1,3},{3,2},{2,0}, {4,5},{5,7},{7,6},{6,4},
        {0,4},{1,5},{2,6},{3,7}
    };
    int32_t minx = *(const int32_t *)(shape + 4);
    int32_t miny = *(const int32_t *)(shape + 8);
    int32_t minz = *(const int32_t *)(shape + 12);
    int32_t maxx = *(const int32_t *)(shape + 16);
    int32_t maxy = *(const int32_t *)(shape + 20);
    int32_t maxz = *(const int32_t *)(shape + 24);
    int32_t p[8][3] = {
        {minx,miny,minz}, {maxx,miny,minz}, {minx,maxy,minz}, {maxx,maxy,minz},
        {minx,miny,maxz}, {maxx,miny,maxz}, {minx,maxy,maxz}, {maxx,maxy,maxz}
    };
    float w[8][3];

    for (int i = 0; i < 8; i++)
        tm_debug_transform_point_mat(mat, p[i][0], p[i][1], p[i][2],
                                     &w[i][0], &w[i][1], &w[i][2]);
    for (int e = 0; e < 12; e++) {
        const float *a = w[edges[e][0]];
        const float *c = w[edges[e][1]];
        n = tm_debug_append_line(out, max_vertices, n,
                                 a[0], a[1], a[2], c[0], c[1], c[2], r, g, b);
    }
    return n;
}

static int tm_debug_append_obj_shapes(const uint8_t *obj,
                                      const MATRIX *parent_mat,
                                      int32_t px, int32_t py, int32_t pz,
                                      float *out, int max_vertices, int n,
                                      int depth)
{
    const uint8_t *shape;
    MATRIX composed;
    int32_t lx, ly, lz;
    int inside_any = 0;
    int guard = 0;

    if (obj == NULL || depth > 16)
        return n;

    if (parent_mat != NULL) {
        CompMatrixLV(parent_mat, (const MATRIX *)(obj + 0x10), &composed);
    } else {
        memcpy(&composed, obj + 0x10, sizeof(composed));
    }

    inside_any = tm_debug_obj_has_inside_box_mat(obj, &composed, px, py, pz,
                                                 &lx, &ly, &lz,
                                                 NULL, NULL, NULL, NULL, NULL, NULL);
    shape = (const uint8_t *)(uintptr_t)*(const uint32_t *)(obj + 0x5c);
    while (shape != NULL && guard++ < 128) {
        int16_t kind = *(const int16_t *)shape;
        if (kind == 0)
            break;
        if (kind == 1) {
            n = tm_debug_append_box_mat(&composed, shape, out, max_vertices, n,
                                        inside_any ? 0.1f : 1.0f,
                                        inside_any ? 1.0f : 0.15f,
                                        0.1f);
            shape += 0x1c;
        } else if (kind == 2) {
            float x, y, z;
            tm_debug_transform_point_mat(&composed, 0, 0, 0, &x, &y, &z);
            n = tm_debug_append_line(out, max_vertices, n,
                                     x - 0.35f, y, z, x + 0.35f, y, z,
                                     1.0f, 0.0f, 1.0f);
            n = tm_debug_append_line(out, max_vertices, n,
                                     x, y - 0.35f, z, x, y + 0.35f, z,
                                     1.0f, 0.0f, 1.0f);
            n = tm_debug_append_line(out, max_vertices, n,
                                     x, y, z - 0.35f, x, y, z + 0.35f,
                                     1.0f, 0.0f, 1.0f);
            shape += 4 + (size_t)*(const uint16_t *)(shape + 2) * 12;
        } else {
            break;
        }
    }

    for (uint8_t *child = (uint8_t *)(uintptr_t)*(const uint32_t *)(obj + 0x38);
         child != NULL;
         child = (uint8_t *)(uintptr_t)*(const uint32_t *)(child + 0x34)) {
        n = tm_debug_append_obj_shapes(child, &composed, px, py, pz,
                                       out, max_vertices, n, depth + 1);
    }
    return n;
}

int TerrainMesh_DebugCollisionLines(int32_t player_x, int32_t player_y, int32_t player_z,
                                    float *out_vertices, int max_vertices)
{
    int n = 0;

    if (out_vertices == NULL || max_vertices <= 0)
        return 0;
    for (int i = 0; i < g_v8_static_collision_obj_count; i++) {
        uint8_t *obj = (uint8_t *)(uintptr_t)g_v8_static_collision_objs[i];
        int32_t dx, dz;
        int64_t dist2;
        if (obj == NULL)
            continue;
        dx = player_x - *(int32_t *)(obj + 0x24);
        dz = player_z - *(int32_t *)(obj + 0x2c);
        dist2 = (int64_t)dx * dx + (int64_t)dz * dz;
        if (dist2 > (int64_t)(12 * 65536) * (12 * 65536))
            continue;
        n = tm_debug_append_obj_shapes(obj, NULL, player_x, player_y, player_z,
                                       out_vertices, max_vertices, n, 0);
        if (n >= max_vertices - 64)
            break;
    }
    return n;
}

void TerrainMesh_DebugCollisionLog(int32_t player_x, int32_t player_y, int32_t player_z,
                                   int frame_idx)
{
    typedef struct {
        const uint8_t *obj;
        int64_t dist2;
        int32_t dx, dz;
    } Candidate;
    Candidate best[6] = {0};
    static int last_frame = -9999;

    if (frame_idx - last_frame < 30)
        return;
    last_frame = frame_idx;

    for (int i = 0; i < g_v8_static_collision_obj_count; i++) {
        const uint8_t *obj = (const uint8_t *)(uintptr_t)g_v8_static_collision_objs[i];
        int32_t dx, dz;
        int64_t dist2;
        if (obj == NULL)
            continue;
        dx = player_x - *(const int32_t *)(obj + 0x24);
        dz = player_z - *(const int32_t *)(obj + 0x2c);
        dist2 = (int64_t)dx * dx + (int64_t)dz * dz;
        if (dist2 > (int64_t)(16 * 65536) * (16 * 65536))
            continue;
        for (int k = 0; k < 6; k++) {
            if (best[k].obj == NULL || dist2 < best[k].dist2) {
                memmove(&best[k + 1], &best[k], (size_t)(5 - k) * sizeof(best[0]));
                best[k].obj = obj;
                best[k].dist2 = dist2;
                best[k].dx = dx;
                best[k].dz = dz;
                break;
            }
        }
    }

    for (int k = 0; k < 6 && best[k].obj != NULL; k++) {
        const uint8_t *obj = best[k].obj;
        const TmStaticDebugInfo *info = tm_static_debug_for_obj((uintptr_t)obj);
        int32_t lx = 0, ly = 0, lz = 0;
        int32_t minx = 0, miny = 0, minz = 0, maxx = 0, maxy = 0, maxz = 0;
        int inside;
        tm_debug_local_point(obj, player_x, player_y, player_z, &lx, &ly, &lz);
        (void)tm_debug_obj_first_box(obj, &minx, &miny, &minz,
                                     &maxx, &maxy, &maxz);
        inside = tm_debug_obj_has_inside_box(obj, player_x, player_y, player_z,
                                             NULL, NULL, NULL,
                                             NULL, NULL, NULL,
                                             NULL, NULL, NULL);
        const uint8_t *shape = (const uint8_t *)(uintptr_t)*(const uint32_t *)(obj + 0x5c);
        int kind = shape ? *(const int16_t *)shape : -1;
        fprintf(stderr,
                "v8: coll_dbg frame=%d rank=%d name=%s obj=%p flags=0x%x layer=%d kind=%d child=%p dist_m=%.2f local=(%.2f,%.2f,%.2f) box=(%.2f,%.2f,%.2f..%.2f,%.2f,%.2f) inside=%d\n",
                frame_idx, k,
                info ? info->name : "?",
                (const void *)obj,
                (unsigned)*(const uint32_t *)obj,
                (int)*(const int16_t *)(obj + 0x06),
                kind,
                (void *)(uintptr_t)*(const uint32_t *)(obj + 0x38),
                sqrt((double)best[k].dist2) / 65536.0,
                (double)lx / 65536.0, (double)ly / 65536.0, (double)lz / 65536.0,
                (double)minx / 65536.0, (double)miny / 65536.0, (double)minz / 65536.0,
                (double)maxx / 65536.0, (double)maxy / 65536.0, (double)maxz / 65536.0,
                inside);
    }
}

static int32_t tm_radius_from_obstacle_stream(const uint8_t *stream)
{
    int32_t best = 0x10000;
    const uint8_t *p = stream;
    int guard = 0;

    if (p == NULL) return best;
    while (guard++ < 256) {
        int16_t kind = *(const int16_t *)p;
        if (kind == 0) break;
        if (kind == 1) {
            const int32_t *box = (const int32_t *)(p + 4);
            for (int i = 0; i < 6; i++) {
                int32_t a = tm_abs_i32(box[i]);
                if (a > best) best = a;
            }
            p += 0x1c;
        } else if (kind == 2) {
            uint16_t count = *(const uint16_t *)(p + 2);
            const int16_t *s = (const int16_t *)(p + 4);
            for (uint16_t i = 0; i < count; i++) {
                int32_t x = (int32_t)s[i * 6 + 0] << 12;
                int32_t y = (int32_t)s[i * 6 + 1] << 12;
                int32_t z = (int32_t)s[i * 6 + 2] << 12;
                int32_t r = ((int32_t)(uint16_t)s[i * 6 + 3]) << 12;
                int32_t candidate = tm_abs_i32(x) + tm_abs_i32(y) + tm_abs_i32(z) + r;
                if (candidate > best) best = candidate;
            }
            p += 4 + (size_t)count * 12;
        } else {
            break;
        }
    }
    return best;
}

static void tm_collision_shape_from_obstacle_stream(const uint8_t *stream,
                                                    uint8_t *shape,
                                                    int32_t radius)
{
    const uint8_t *p = stream;
    int guard = 0;

    memset(shape, 0, 0x20);
    while (p != NULL && guard++ < 256) {
        int16_t kind = *(const int16_t *)p;
        if (kind == 0) break;
        if (kind == 1) {
            memcpy(shape, p, 0x1c);
            return;
        }
        if (kind == 2) {
            uint16_t count = *(const uint16_t *)(p + 2);
            p += 4 + (size_t)count * 12;
        } else {
            break;
        }
    }

    *(int16_t *)(shape + 0x00) = 1;
    *(int32_t *)(shape + 0x04) = -radius;
    *(int32_t *)(shape + 0x08) = -radius;
    *(int32_t *)(shape + 0x0c) = -radius;
    *(int32_t *)(shape + 0x10) = radius;
    *(int32_t *)(shape + 0x14) = radius;
    *(int32_t *)(shape + 0x18) = radius;
}

/* ------------------------------------------------------------------ */
/* Everything below requires SDL + OpenGL.                             */
/* ------------------------------------------------------------------ */
#if defined(V8_HAVE_SDL) && defined(V8_HAVE_GL)
#include <GL/gl3w.h>

/* ---- Public GL globals ---- */
GLuint g_terrainmesh_vao = 0;
GLuint g_terrainmesh_tex = 0;
GLuint g_terrainmesh_tex_bank1 = 0;
GLuint g_terrainmesh_xbmp_tex = 0;
GLuint g_terrainmesh_sky_tex = 0;
GLuint g_terrainmesh_route_tex0 = 0;
GLuint g_terrainmesh_route_tex1 = 0;
static int g_tm_allow_gl_upload = 1;
int    g_terrainmesh_vtx = 0;
int    g_terrainmesh_tex_w = 0;
int    g_terrainmesh_tex_h = 0;
int    g_terrainmesh_has_dynamic_water = 0;
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
static int g_tm_pkt_kind[16] = {0};
static int g_tm_pkt_uv_kind[16] = {0};
static int g_tm_pkt_no_uv_kind[16] = {0};

/* ---- Config ---- */
#define TERR_SCALE      (1.0f / 16.0f)   /* fallback unplaced BIN scale */
/* OpenGL Y of BIN Y=0.
 * Derived by cross-validating BIN vertex data against ZONE heightmap:
 *   - Bone[1] (main 71-vertex terrain patch): centroid raw_y ~= 0.
 *   - Converted ZONE runtime terrain is around gl_y=-22.5m.
 *   - Solve: -(0/16) + TERR_Y_ORIGIN = -22.5 -> TERR_Y_ORIGIN = -22.5. */
#define TERR_Y_ORIGIN   (-22.5f)         /* OpenGL Y of BIN Y=0 */
#define TM_ROUTE_VISUAL_Y_BIAS (0.025f)

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
static const int TM_SOURCE_PKT_SIZE[16] = {
    12,28,20,28, 12,20,12,20, 16,24,12,24, 20,20,0,20
};
/* Source draw handlers at SLUS 8001c100..8001cbd8 emit one indexed
 * polygon from raw terrain packet kinds 4/5/7.  The extra word in those
 * packets is renderer metadata, not a fourth vertex for host-side triangle
 * fabrication; emitting it as (0,2,3) draws a reversed coplanar duplicate. */
static const int TM_IS_QUAD[16] = {
    0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};

#define TM_DISPLAY_XZ_SCALE (1.0f / 65536.0f)
#define TM_DISPLAY_Y_SCALE  (1.0f / 65536.0f)

/* ---- Per-vertex GL data ---- */
typedef struct { float x,y,z,r,g,b,u,v,tex; } TmVert;

typedef struct {
    const uint8_t *data;
    uint32_t size;
    uintptr_t source_bank;
    uint32_t group_count;
    uint32_t group_table;
    uint32_t obstacle_count;
    uint32_t obstacle_table;
    uint32_t obstacle_end;
    uint32_t slot_count;
    int texture_kind;
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
    const uint8_t *tex_payload;
    uint32_t tex_size;
    int tex_slot;
    int tex_w, tex_h;
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
    int32_t x, y, z;
    int16_t dx, dz;
} TmTrainPathSample;

#define TM_WW_TRAIN_MAX_SAMPLES 1024
static TmTrainPathSample g_ww_train_path[TM_WW_TRAIN_MAX_SAMPLES];
static int g_ww_train_path_count = 0;

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

static uintptr_t tm_build_source_bank(const uint8_t *bin, uint32_t size)
{
    uint8_t *copy;

    if (bin == NULL || size < 0x1c)
        return 0;
    copy = (uint8_t *)FUN_8001178c(size, 1);
    if (copy == NULL)
        return 0;
    memcpy(copy, bin, size);
    return (uintptr_t)Object_BuildFromBin((int *)copy, NULL);
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

static GLuint tm_upload_tim_indexed_texture(const uint8_t *p, uint32_t size,
                                            int semi_trans,
                                            int *out_w, int *out_h)
{
    if (out_w) *out_w = 0;
    if (out_h) *out_h = 0;
    if (p == NULL || size < 0x20)
        return 0;
    if (tm_rd32le(p, 0) != 0x10u)
        return 0;

    uint32_t flags = tm_rd32le(p, 4);
    int depth = (int)(flags & 3u);
    if ((flags & 8u) == 0 || (depth != 0 && depth != 1))
        return 0;

    uint32_t clut_len = tm_rd32le(p, 8);
    if (clut_len < 12 || 8u + clut_len + 12u > size)
        return 0;

    int clut_w = tm_rds16le(p, 16);
    int clut_h = tm_rds16le(p, 18);
    int clut_count = clut_w * clut_h;
    if (clut_count <= 0 || 20u + (uint32_t)clut_count * 2u > 8u + clut_len)
        return 0;

    uint32_t image = 8u + clut_len;
    uint32_t image_len = tm_rd32le(p, image);
    if (image_len < 12 || image + image_len > size)
        return 0;

    int image_words = tm_rds16le(p, image + 8);
    int image_h = tm_rds16le(p, image + 10);
    int image_w = tm_pixel_width_from_words(image_words, depth);
    uint32_t pix_off = image + 12u;
    uint32_t packed_row_size = (uint32_t)image_words * 2u;
    uint32_t pix_size = packed_row_size * (uint32_t)image_h;
    if (image_w <= 0 || image_h <= 0 || pix_off + pix_size > image + image_len)
        return 0;

    uint8_t palette[256][4];
    memset(palette, 0, sizeof(palette));
    for (int i = 0; i < clut_count && i < 256; i++) {
        uint16_t c = tm_rd16le(p, 20u + (uint32_t)i * 2u);
        tm_psx555_rgba(c, palette[i]);
        if ((c & 0x7fffu) == 0 && (c & 0x8000u) == 0)
            palette[i][3] = 0;
        else if (semi_trans)
            palette[i][3] = 128;
        else
            palette[i][3] = 255;
    }

    uint8_t *rgba = (uint8_t *)malloc((size_t)image_w * (size_t)image_h * 4u);
    if (rgba == NULL)
        return 0;
    const uint8_t *pix = p + pix_off;
    for (int y = 0; y < image_h; y++) {
        const uint8_t *row = pix + (uint32_t)y * packed_row_size;
        for (int x = 0; x < image_w; x++) {
            uint8_t idx;
            if (depth == 0) {
                uint8_t packed = row[(uint32_t)x >> 1];
                idx = (x & 1) ? (packed >> 4) : (packed & 0x0f);
            } else {
                idx = row[x];
            }
            int dst = (y * image_w + x) * 4;
            rgba[dst + 0] = palette[idx][0];
            rgba[dst + 1] = palette[idx][1];
            rgba[dst + 2] = palette[idx][2];
            rgba[dst + 3] = palette[idx][3];
        }
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

    if (out_w) *out_w = image_w;
    if (out_h) *out_h = image_h;
    return tex;
}

static void tm_upload_route_textures(TmRouteType *rtypes, int nrtypes)
{
    int slot = 0;
    for (int i = 0; i < nrtypes && slot < 2; i++) {
        TmRouteType *rt = &rtypes[i];
        if (rt->tex_payload == NULL || rt->tex_size == 0)
            continue;
        int w = 0, h = 0;
        GLuint tex = tm_upload_tim_indexed_texture(
            rt->tex_payload, rt->tex_size,
            (rt->flags & 0x100) != 0, &w, &h);
        if (tex == 0)
            continue;
        rt->tex_slot = slot;
        rt->tex_w = w;
        rt->tex_h = h;
        if (slot == 0)
            g_terrainmesh_route_tex0 = tex;
        else
            g_terrainmesh_route_tex1 = tex;
        fprintf(stderr,
                "v8: TerrainMesh -- uploaded XRTP route texture type=%d slot=%d %dx%d payload=%u\n",
                i, slot, w, h, (unsigned)rt->tex_size);
        slot++;
    }
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

static int tm_trace_visual_collision_enabled(void)
{
    static int cached = -1;
    if (cached < 0) {
        const char *env = getenv("V8_TRACE_VISUAL_COLLISION");
        cached = (env != NULL && env[0] != 0 && env[0] != '0');
    }
    return cached;
}

static void tm_trace_head_visual_extent(const TmHead *h,
                                        const TmTri *tris,
                                        int first, int last)
{
    float minx, miny, minz, maxx, maxy, maxz;

    if (!tm_trace_visual_collision_enabled() || h == NULL || first >= last)
        return;

    minx = miny = minz =  1.0e30f;
    maxx = maxy = maxz = -1.0e30f;
    for (int ti = first; ti < last; ti++) {
        const TmTri *t = &tris[ti];
        const float xs[3] = { t->ax, t->bx, t->cx };
        const float ys[3] = { t->ay, t->by, t->cy };
        const float zs[3] = { t->az, t->bz, t->cz };
        for (int vi = 0; vi < 3; vi++) {
            if (xs[vi] < minx) minx = xs[vi];
            if (ys[vi] < miny) miny = ys[vi];
            if (zs[vi] < minz) minz = zs[vi];
            if (xs[vi] > maxx) maxx = xs[vi];
            if (ys[vi] > maxy) maxy = ys[vi];
            if (zs[vi] > maxz) maxz = zs[vi];
        }
    }
    fprintf(stderr,
            "v8: visual_head name=%s type=%d bank=%d slot=%d tris=%d raw=(0x%x,0x%x,0x%x) ext_m=(%.3f,%.3f,%.3f..%.3f,%.3f,%.3f)\n",
            h->name, h->type, h->bank, h->slot, last - first,
            (unsigned)h->raw_x, (unsigned)h->raw_y, (unsigned)h->raw_z,
            minx, miny, minz, maxx, maxy, maxz);
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

static int16_t tm_type5_scaled_strength(const TmHead *h)
{
    int32_t strength = h->initial_strength;
    if (h->id >= 0) {
        int32_t scaled = strength * ((int32_t)cRam00000016 + 2);
        if (scaled < 0)
            scaled += 3;
        strength = scaled >> 2;
    }
    return (int16_t)strength;
}

static void tm_type5_insert_source_list(uint8_t *obj)
{
    int16_t id = *(int16_t *)(obj + 0x06);
    uint8_t *prev_same = NULL;

    for (int i = 0; i < g_spawn_nplaceholders; i++) {
        uint8_t *cur = g_spawn_placeholders[i];
        if (cur == NULL || *(int16_t *)(cur + 0x06) != id)
            continue;
        int guard = 0;
        while (*(uint32_t *)(cur + 0x34) != 0 && guard++ < g_spawn_nplaceholders) {
            uint8_t *next = (uint8_t *)(uintptr_t)(uint32_t)*(uint32_t *)(cur + 0x34);
            if (next == obj)
                break;
            cur = next;
        }
        prev_same = cur;
        break;
    }

    if (prev_same != NULL) {
        *(uint32_t *)(prev_same + 0x34) = (uint32_t)(uintptr_t)obj;
        *(uint32_t *)(obj + 0x3c) = (uint32_t)(uintptr_t)prev_same;
        return;
    }

    FUN_8001fe50((uintptr_t)DAT_80065a50, (uintptr_t)obj);
}

static void tm_build_spawn_placeholders(const TmBank *banks, int nbanks,
                                        const TmHead *heads, int nheads)
{
    int count = 0;
    int resolved = 0;
    int table_fallback = 0;
    int materialized = 0;
    for (int i = 0; i < nheads; i++) {
        if (heads[i].type == 5)
            count++;
    }

    free(g_spawn_placeholders);
    g_spawn_placeholders = NULL;
    g_spawn_nplaceholders = 0;
    if (count == 0)
        return;

    g_spawn_placeholders = (uint8_t **)calloc((size_t)count, sizeof(g_spawn_placeholders[0]));
    if (g_spawn_placeholders == NULL)
        return;

    for (int i = 0; i < nheads; i++) {
        const TmHead *h = &heads[i];
        if (h->type != 5)
            continue;
        uint8_t *obj = (uint8_t *)FUN_8001d470(0x80);
        int16_t strength = tm_type5_scaled_strength(h);
        int did_resolve = 0;
        uintptr_t callback = tm_resolve_global_head_callback(h->name);
        if (callback != 0)
            did_resolve = 1;
        else
            callback = tm_resolve_head_callback(g_v8_level_exp_path,
                                                h->name, &did_resolve);
        if (did_resolve) resolved++;
        else table_fallback++;
        if (obj == NULL)
            continue;
        *(uint32_t *)(obj + 0x00) = h->flags & 0xfff867feu;
        *(uint8_t  *)(obj + 0x04) = (uint8_t)h->type;
        *(int16_t  *)(obj + 0x06) = h->id;
        *(uint8_t  *)(obj + 0x08) = h->script;
        *(int16_t  *)(obj + 0x0a) = h->slot;
        *(int16_t  *)(obj + 0x0c) = strength;
        *(int16_t  *)(obj + 0x0e) = strength;
        *(int16_t  *)(obj + 0x40) = h->ry;
        *(int16_t  *)(obj + 0x42) = h->rx;
        *(int16_t  *)(obj + 0x44) = h->rz;
        *(int32_t  *)(obj + 0x48) = h->raw_x;
        *(int32_t  *)(obj + 0x4c) = h->raw_y;
        *(int32_t  *)(obj + 0x50) = h->raw_z;
        if (h->bank >= 0 && h->bank < nbanks)
            *(uint32_t *)(obj + 0x58) = (uint32_t)banks[h->bank].source_bank;
        Object_SetCallbackPsxSlot(obj, callback);
        FUN_8001d708((uint32_t *)obj);
        tm_type5_insert_source_list(obj);
        g_spawn_placeholders[g_spawn_nplaceholders] = obj;
        g_spawn_nplaceholders++;

        if (callback == (uintptr_t)LAB_8003c61c &&
            h->bank >= 0 && h->bank < nbanks && banks[h->bank].source_bank != 0) {
            uint8_t *live = (uint8_t *)(uintptr_t)FUN_80021c6c((uint32_t *)obj);
            if (live != NULL && FUN_8002036c((uint32_t *)live) != 0)
                materialized++;
        }
    }

    fprintf(stderr,
            "v8: TerrainMesh -- source spawn placeholders=%d live_pickups=%d callbacks=%d fallback=%d\n",
            g_spawn_nplaceholders, materialized, resolved, table_fallback);
}

intptr_t Host_TerrainFindPlaceholderById(int spawn_id)
{
    for (int i = 0; i < g_spawn_nplaceholders; i++) {
        uint8_t *obj = g_spawn_placeholders[i];
        if (obj == NULL)
            continue;
        if (*(int16_t *)(obj + 0x06) == (int16_t)spawn_id)
            return (intptr_t)obj;
    }
    return 0;
}

int Host_TerrainCountPlaceholdersWithFlag(uint32_t flag)
{
    int count = 0;
    for (int i = 0; i < g_spawn_nplaceholders; i++) {
        uint8_t *obj = g_spawn_placeholders[i];
        if (obj == NULL || *(uint32_t *)(obj + 0x3c) != 0)
            continue;
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
        uint8_t *obj = g_spawn_placeholders[i];
        if (obj == NULL || *(uint32_t *)(obj + 0x3c) != 0)
            continue;
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
                int bank_index = *nbanks;
                TmBank *b = &banks[(*nbanks)++];
                b->data        = data + body;
                b->size        = csz;
                b->texture_kind = (bank_index == 1) ? 2 : 0;
                b->source_bank = tm_build_source_bank(b->data, b->size);
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
                if (g_tm_allow_gl_upload)
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
                dst->tex_payload = (csz > 12) ? h + 12 : NULL;
                dst->tex_size = (csz > 12) ? csz - 12u : 0u;
                dst->tex_slot = -1;
                dst->tex_w = 0;
                dst->tex_h = 0;
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

static int32_t tm_route_deriv_component_q8(int32_t p0, int32_t p1,
                                           int32_t p2, int32_t p3,
                                           int32_t t)
{
    int32_t a = tm_trunc_shift32((p1 * 3 - p0) - p2 * 3 + p3, 4) * 3;
    int32_t b = tm_trunc_shift32(p0 * 3 - p1 * 6 + p2 * 3, 4) * 2;
    int32_t c = tm_trunc_shift32(p1 * 3 - p0 * 3, 4);
    int32_t t2 = tm_trunc_shift32((int32_t)((int64_t)t * t), 12);
    int64_t v = (int64_t)a * t2 + (int64_t)b * t;
    if (v < 0)
        v += 0xfff;
    v = (v >> 12) + c;
    if (v < 0)
        v += 0xff;
    return (int32_t)(v >> 8);
}

static const TmRseg *tm_find_rseg_between(const TmRseg *rsegs, int nrsegs,
                                          int a, int b)
{
    for (int i = 0; i < nrsegs; i++) {
        const TmRseg *seg = &rsegs[i];
        if ((seg->node_a == a && seg->node_b == b) ||
            (seg->node_a == b && seg->node_b == a))
            return seg;
    }
    return NULL;
}

static int32_t tm_bezier_eval_runtime(int32_t p0, int32_t p1,
                                      int32_t p2, int32_t p3,
                                      int32_t t)
{
    int64_t u = 0x1000 - t;
    int64_t u2 = (u * u) >> 12;
    int64_t t2 = ((int64_t)t * t) >> 12;
    int64_t u3 = (u2 * u) >> 12;
    int64_t t3 = (t2 * t) >> 12;
    int64_t v = u3 * p0 + 3 * ((u2 * t) >> 12) * p1
              + 3 * ((u * t2) >> 12) * p2 + t3 * p3;
    if (v < 0) v -= 0x800;
    else v += 0x800;
    return (int32_t)(v >> 12);
}

static void tm_ww_train_add_sample(int32_t x, int32_t z, int16_t dx, int16_t dz)
{
    if (g_ww_train_path_count >= TM_WW_TRAIN_MAX_SAMPLES)
        return;
    TmTrainPathSample *s = &g_ww_train_path[g_ww_train_path_count++];
    s->x = x;
    s->z = z;
    s->y = Terrain_HeightAt((uint32_t)x, (uint32_t)z);
    s->dx = dx;
    s->dz = dz;
}

static void tm_build_wildwest_train_path(const TmJuncNode *juncs, int njuncs,
                                         const TmRseg *rsegs, int nrsegs)
{
    static const int loop[] = {
        2, 11, 3, 4, 5, 6, 35, 36, 7, 8, 9, 10,
        38, 37, 34, 0, 1, 2
    };
    g_ww_train_path_count = 0;

    for (int li = 0; li + 1 < (int)(sizeof(loop) / sizeof(loop[0])); li++) {
        int aidx = loop[li];
        int bidx = loop[li + 1];
        if (aidx < 0 || aidx >= njuncs || bidx < 0 || bidx >= njuncs)
            continue;
        const TmRseg *seg = tm_find_rseg_between(rsegs, nrsegs, aidx, bidx);
        if (seg == NULL)
            continue;

        const TmJuncNode *a = &juncs[aidx];
        const TmJuncNode *b = &juncs[bidx];
        int forward = (seg->node_a == aidx && seg->node_b == bidx);
        int32_t p0x = a->raw_x, p0z = a->raw_z;
        int32_t p3x = b->raw_x, p3z = b->raw_z;
        int32_t p1x, p1z, p2x, p2z;
        if (forward) {
            p1x = p0x + seg->ctrl_ax;
            p1z = p0z + seg->ctrl_az;
            p2x = p3x + seg->ctrl_bx;
            p2z = p3z + seg->ctrl_bz;
        } else {
            p1x = p0x - seg->ctrl_bx;
            p1z = p0z - seg->ctrl_bz;
            p2x = p3x - seg->ctrl_ax;
            p2z = p3z - seg->ctrl_az;
        }

        for (int step = 0; step < 24; step++) {
            int32_t t0 = (step * 0x1000) / 24;
            int32_t t1 = ((step + 1) * 0x1000) / 24;
            int32_t x0 = tm_bezier_eval_runtime(p0x, p1x, p2x, p3x, t0);
            int32_t z0 = tm_bezier_eval_runtime(p0z, p1z, p2z, p3z, t0);
            int32_t x1 = tm_bezier_eval_runtime(p0x, p1x, p2x, p3x, t1);
            int32_t z1 = tm_bezier_eval_runtime(p0z, p1z, p2z, p3z, t1);
            int32_t dx = x1 - x0;
            int32_t dz = z1 - z0;
            double len = sqrt((double)dx * (double)dx + (double)dz * (double)dz);
            int16_t ndx = 0, ndz = 0x1000;
            if (len > 1.0) {
                ndx = (int16_t)((double)dx * 4096.0 / len);
                ndz = (int16_t)((double)dz * 4096.0 / len);
            }
            tm_ww_train_add_sample(x0, z0, ndx, ndz);
        }
    }

    fprintf(stderr, "v8: WILDWEST train path samples=%d\n",
            g_ww_train_path_count);
}

static uint16_t tm_terrain_word_at_fixed(int32_t fx, int32_t fz)
{
    int x_cell = fx >> 16;
    int z_cell = fz >> 16;
    int chunk_x = (x_cell >> 6) & 0x1f;
    int chunk_z = (z_cell >> 6) & 0x1f;
    uintptr_t base = DAT_800911a0[chunk_x * 32 + chunk_z];

    if (base == 0)
        return 0;
    return *(uint16_t *)(base + (((uint32_t)(x_cell & 0x3f) << 7) |
                                ((uint32_t)(z_cell & 0x3f) << 1)));
}

static float tm_route_source_light(int32_t fx, int32_t fz)
{
    uint16_t word = tm_terrain_word_at_fixed(fx, fz);
    int shade = (int)(word >> 11) << 2;
    return (float)shade * (1.0f / 128.0f);
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
        vbuf[nvtx++] = (TmVert){vx[ib],vy[ib],vz[ib],lr,lg,lb,-1.0f,-1.0f,0.0f};
        vbuf[nvtx++] = (TmVert){vx[ic],vy[ic],vz[ic],lr,lg,lb,-1.0f,-1.0f,0.0f};
    }

    *nvtx_io = nvtx;
    *ntris_io = ntris;
}

static void tm_emit_route_face(float vx[4], float vy[4], float vz[4],
                               float uv[4][2], float tex_kind,
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
        vbuf[nvtx++] = (TmVert){vx[ia],vy[ia],vz[ia],lr,lg,lb,uv[ia][0],uv[ia][1],tex_kind};
        vbuf[nvtx++] = (TmVert){vx[ib],vy[ib],vz[ib],lr,lg,lb,uv[ib][0],uv[ib][1],tex_kind};
        vbuf[nvtx++] = (TmVert){vx[ic],vy[ic],vz[ic],lr,lg,lb,uv[ic][0],uv[ic][1],tex_kind};
    }

    *nvtx_io = nvtx;
    *ntris_io = ntris;
}

static void tm_emit_route_tri_shaded(float vx[9], float vy[9], float vz[9],
                                     float uv[9][2], float shade[9],
                                     float tex_kind, int ia, int ib, int ic,
                                     TmVert *vbuf, int vcap, int *nvtx_io,
                                     TmTri *tribuf, int tcap, int *ntris_io)
{
    int nvtx = *nvtx_io;
    int ntris = *ntris_io;

    float ex = vx[ib] - vx[ia], ey = vy[ib] - vy[ia], ez = vz[ib] - vz[ia];
    float fx = vx[ic] - vx[ia], fy = vy[ic] - vy[ia], fz = vz[ic] - vz[ia];
    float tnx = ey * fz - ez * fy;
    float tny = ez * fx - ex * fz;
    float tnz = ex * fy - ey * fx;
    float nl = sqrtf(tnx * tnx + tny * tny + tnz * tnz);
    if (nl > 1e-6f) { tnx /= nl; tny /= nl; tnz /= nl; }
    tnx = -tnx; tny = -tny; tnz = -tnz;

    if (ntris < tcap) {
        TmTri *tt = &tribuf[ntris++];
        tt->ax = vx[ia]; tt->ay = vy[ia]; tt->az = vz[ia];
        tt->bx = vx[ib]; tt->by = vy[ib]; tt->bz = vz[ib];
        tt->cx = vx[ic]; tt->cy = vy[ic]; tt->cz = vz[ic];
        tt->nx = tnx; tt->ny = tny; tt->nz = tnz;
        tt->pd = tnx * vx[ia] + tny * vy[ia] + tnz * vz[ia];
    }
    if (nvtx + 3 <= vcap) {
        float sa = shade[ia], sb = shade[ib], sc = shade[ic];
        vbuf[nvtx++] = (TmVert){vx[ia], vy[ia], vz[ia], sa, sa, sa, uv[ia][0], uv[ia][1], tex_kind};
        vbuf[nvtx++] = (TmVert){vx[ib], vy[ib], vz[ib], sb, sb, sb, uv[ib][0], uv[ib][1], tex_kind};
        vbuf[nvtx++] = (TmVert){vx[ic], vy[ic], vz[ic], sc, sc, sc, uv[ic][0], uv[ic][1], tex_kind};
    }

    *nvtx_io = nvtx;
    *ntris_io = ntris;
}

static void tm_emit_route_subquad(float vx[9], float vy[9], float vz[9],
                                  float uv[9][2], float shade[9],
                                  float tex_kind,
                                  int a, int b, int c, int d,
                                  TmVert *vbuf, int vcap, int *nvtx,
                                  TmTri *tribuf, int tcap, int *ntris)
{
    tm_emit_route_tri_shaded(vx, vy, vz, uv, shade, tex_kind,
                             a, c, b, vbuf, vcap, nvtx, tribuf, tcap, ntris);
    tm_emit_route_tri_shaded(vx, vy, vz, uv, shade, tex_kind,
                             b, c, d, vbuf, vcap, nvtx, tribuf, tcap, ntris);
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
        /*
         * XOBF stores the source packet kind in the low nibble.  The retail
         * loader's Object_BuildFromBin pass later moves that nibble into bits
         * 2..5 for Bone_AllocLevel and the cached renderer packets.  This
         * routine reads the loose file before either pass, so decoding
         * (typ >> 2) here changes a native kind-5 20-byte textured triangle
         * into a kind-1 28-byte record and loses packet alignment.
         */
        int source_kind = typ & 0xf;
        int sz  = TM_PKT_SIZE[source_kind];
        g_tm_pkt_kind[source_kind]++;
        if (sz == 0 || po + (uint32_t)sz > bsz) {
            (*bad_io)++;
            po += 4;
            continue;
        }
        if (source_kind == 10) {
            /* FUN_8001b49c expands kind 0xa as a run of textured tile/sprite
             * primitives from texture-slot records, not as indexed triangle
             * geometry.  The old raw reader interpreted those fields as
             * vertex indices and could emit garbage faces through props. */
            g_tm_pkt_no_uv_kind[source_kind]++;
            po += sz;
            continue;
        }

        uint16_t vi[4];
        vi[0] = tm_rd16le(B, po+4);
        vi[1] = tm_rd16le(B, po+6);
        vi[2] = tm_rd16le(B, po+8);
        vi[3] = TM_IS_QUAD[source_kind] ? tm_rd16le(B, po+10) : vi[2];

        int nv = TM_IS_QUAD[source_kind] ? 4 : 3;
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

            float cr = B[po + 0] / 255.0f;
            float cg = B[po + 1] / 255.0f;
            float cb = B[po + 2] / 255.0f;
            float ndotl = tnx*LX + tny*LY + tnz*LZ;
            float lit   = 0.42f + 0.45f * (ndotl > 0 ? ndotl : 0);
            float lr = cr * lit;
            float lg = cg * lit;
            float lb = cb * lit;
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
            if (source_kind == 13 &&
                tm_rd16le(B, po + 0x12) == 0xffffu) {
                /*
                 * Native raw kind-13/0xffff selects the engine-owned dynamic
                 * image descriptor at DAT_80065a28.  Ordinary texture ids
                 * may carry 0x4000/0x8000 mode bits; FUN_8001b49c masks those
                 * before indexing the XOBF texture table, so they must not be
                 * mistaken for dynamic imagery.
                 */
                for (int k = 0; k < 3; k++) {
                    int uvo = 0x0c + k * 2;
                    uv[k][0] = (float)B[po + uvo] * (1.0f / 39.0f);
                    uv[k][1] = (float)B[po + uvo + 1] * (1.0f / 22.0f);
                }
                has_uv = 1;
                tex_kind = 5.0f;
                g_terrainmesh_has_dynamic_water = 1;
            } else {
                has_uv = V8_XobfTex_DecodePacketUv(
                    &bank->atlas, B + po, source_kind, (int)tex_base, uv
                );
                tex_kind = has_uv ? (float)bank->texture_kind : 0.0f;
            }
            if (has_uv) {
                /*
                 * PSX polygon RGB is texture modulation with 0x80 as neutral,
                 * not an ordinary 0..255 color.  The old /255 path halved
                 * every texture before lighting and made props nearly black.
                 */
                cr = (float)B[po + 0] * (1.0f / 128.0f);
                cg = (float)B[po + 1] * (1.0f / 128.0f);
                cb = (float)B[po + 2] * (1.0f / 128.0f);
                lr = cr * lit;
                lg = cg * lit;
                lb = cb * lit;
            }
            if (ground0) {
                if (has_uv) g_tm_uv_xobf++;
                else g_tm_uv_ground_none++;
            } else {
                if (has_uv) g_tm_uv_xobf++;
                else g_tm_uv_none++;
            }
            if (has_uv) g_tm_pkt_uv_kind[source_kind]++;
            else g_tm_pkt_no_uv_kind[source_kind]++;
#else
            has_uv = tm_decode_packet_uv(B, po, nib, uv);
#endif

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
                /* PSX Y-down to GL Y-up changes handedness. */
                vbuf[nvtx++] = (TmVert){vx[0],vy[0],vz[0],lr,lg,lb, has_uv ? uv[0][0] : -1.0f, has_uv ? uv[0][1] : -1.0f, tex_kind};
                vbuf[nvtx++] = (TmVert){vx[2],vy[2],vz[2],lr,lg,lb, has_uv ? uv[2][0] : -1.0f, has_uv ? uv[2][1] : -1.0f, tex_kind};
                vbuf[nvtx++] = (TmVert){vx[1],vy[1],vz[1],lr,lg,lb, has_uv ? uv[1][0] : -1.0f, has_uv ? uv[1][1] : -1.0f, tex_kind};
            }

            if (TM_IS_QUAD[source_kind]) {
                int ground1 = tm_is_render_ground_tri(vx[0], vy[0], vz[0],
                                                      vx[2], vy[2], vz[2],
                                                      vx[3], vy[3], vz[3], tny);
                float lr1 = cr * lit, lg1 = cg * lit, lb1 = cb * lit;
                float uv1[4][2];
                memcpy(uv1, uv, sizeof(uv1));
                int has_uv1 = has_uv;
                float tex_kind1 = tex_kind;
                if (ground1) {
                    if (has_uv1) g_tm_uv_xobf++;
                    else g_tm_uv_ground_none++;
                } else {
                    if (has_uv1) g_tm_uv_xobf++;
                    else g_tm_uv_none++;
                }
                if (has_uv1) g_tm_pkt_uv_kind[source_kind]++;
                else g_tm_pkt_no_uv_kind[source_kind]++;
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
    int16_t tex_base = tm_rds16le(B, bd + 0x12);
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
        int source_kind = typ & 0xf;
        int sz = TM_PKT_SIZE[source_kind];
        if (sz == 0 || po + (uint32_t)sz > bsz) {
            (*bad_io)++;
            po += 4;
            continue;
        }
        if (source_kind == 10) {
            po += sz;
            continue;
        }

        uint16_t vi[4];
        vi[0] = tm_rd16le(B, po + 4);
        vi[1] = tm_rd16le(B, po + 6);
        vi[2] = tm_rd16le(B, po + 8);
        vi[3] = TM_IS_QUAD[source_kind] ? tm_rd16le(B, po + 10) : vi[2];
        int nv = TM_IS_QUAD[source_kind] ? 4 : 3;
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
            float cr = B[po + 0] / 255.0f;
            float cg = B[po + 1] / 255.0f;
            float cb = B[po + 2] / 255.0f;
            float uv[4][2] = {
                {-1.0f, -1.0f}, {-1.0f, -1.0f},
                {-1.0f, -1.0f}, {-1.0f, -1.0f}
            };
            int has_uv = 0;
            float tex_kind = 0.0f;
#if defined(V8_HAVE_SDL) && defined(V8_HAVE_GL)
            has_uv = V8_XobfTex_DecodePacketUv(&bank->atlas, B + po,
                                                source_kind, (int)tex_base,
                                                uv);
            tex_kind = has_uv ? (float)bank->texture_kind : 0.0f;
#endif
            if (has_uv) {
                tm_emit_route_face(vx, vy, vz, uv, tex_kind, 0, 1, 2,
                                   cr, cg, cb, vbuf, vcap, nvtx_io,
                                   tribuf, tcap, ntris_io);
            } else {
                tm_emit_flat_face(vx, vy, vz, 0, 1, 2, cr, cg, cb,
                                  vbuf, vcap, nvtx_io, tribuf, tcap,
                                  ntris_io);
            }
            if (TM_IS_QUAD[source_kind]) {
                if (has_uv) {
                    tm_emit_route_face(vx, vy, vz, uv, tex_kind, 0, 2, 3,
                                       cr, cg, cb, vbuf, vcap, nvtx_io,
                                       tribuf, tcap, ntris_io);
                } else {
                    tm_emit_flat_face(vx, vy, vz, 0, 2, 3, cr, cg, cb,
                                      vbuf, vcap, nvtx_io, tribuf, tcap,
                                      ntris_io);
                }
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
    int type_tris[16] = {0};
    int type_vtx[16] = {0};
    float type_minx[16], type_miny[16], type_minz[16];
    float type_maxx[16], type_maxy[16], type_maxz[16];
    int trace = getenv("V8_TRACE_ROUTE_VISUAL") != NULL;

    if (trace) {
        for (int i = 0; i < 16; i++) {
            type_minx[i] = type_miny[i] = type_minz[i] =  1.0e30f;
            type_maxx[i] = type_maxy[i] = type_maxz[i] = -1.0e30f;
        }
    }

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
        if (rt->width <= 0 || rt->step <= 0 || rt->tex_slot < 0) continue;

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

        float prev_lx = 0, prev_ly = 0, prev_lz = 0;
        float prev_rx = 0, prev_ry = 0, prev_rz = 0;
        float prev_lshade = 0.0f, prev_rshade = 0.0f;
        int have_prev = 0;

        int32_t t = 0;
        int sample = 0;
        while (t <= 0x1000 && sample < 512) {
            int32_t draw_t = t;
            if (draw_t > 0x1000)
                draw_t = 0x1000;
            int32_t x = tm_bezier_coord_q12(px[0], px[1], px[2], px[3], t);
            int32_t z = tm_bezier_coord_q12(pz[0], pz[1], pz[2], pz[3], t);
            int32_t dx_q8 = tm_route_deriv_component_q8(px[0], px[1], px[2], px[3], draw_t);
            int32_t dz_q8 = tm_route_deriv_component_q8(pz[0], pz[1], pz[2], pz[3], draw_t);
            int32_t speed = (int32_t)sqrt((double)dx_q8 * (double)dx_q8 +
                                          (double)dz_q8 * (double)dz_q8);
            if (speed <= 0)
                break;

            int32_t ox = (int32_t)(((int64_t)dz_q8 * (int64_t)rt->width / 2) / speed);
            int32_t oz = (int32_t)(((int64_t)dx_q8 * (int64_t)rt->width / 2) / speed);
            int32_t lx = x - ox;
            int32_t lz = z + oz;
            int32_t rx = x + ox;
            int32_t rz = z - oz;
            int32_t ly = Terrain_HeightAt((uint32_t)lx, (uint32_t)lz);
            int32_t ry = Terrain_HeightAt((uint32_t)rx, (uint32_t)rz);
            float lshade = tm_route_source_light(lx, lz);
            float rshade = tm_route_source_light(rx, rz);

            float cur_lx = (float)lx * (1.0f / 65536.0f);
            float cur_ly = (float)-ly * TM_DISPLAY_Y_SCALE + TM_ROUTE_VISUAL_Y_BIAS;
            float cur_lz = (float)lz * (1.0f / 65536.0f);
            float cur_rx = (float)rx * (1.0f / 65536.0f);
            float cur_ry = (float)-ry * TM_DISPLAY_Y_SCALE + TM_ROUTE_VISUAL_Y_BIAS;
            float cur_rz = (float)rz * (1.0f / 65536.0f);

            if (have_prev) {
                int prev_ntris = *ntris;
                int prev_nvtx = *nvtx;

                if (rt->tex_slot >= 0) {
                    float vx[9] = {
                        prev_lx, prev_rx, cur_lx, cur_rx,
                        (prev_lx + prev_rx) * 0.5f,
                        (prev_lx + cur_lx) * 0.5f,
                        (prev_lx + prev_rx + cur_lx + cur_rx) * 0.25f,
                        (prev_rx + cur_rx) * 0.5f,
                        (cur_lx + cur_rx) * 0.5f
                    };
                    float vy[9] = {
                        prev_ly, prev_ry, cur_ly, cur_ry,
                        (prev_ly + prev_ry) * 0.5f,
                        (prev_ly + cur_ly) * 0.5f,
                        (prev_ly + prev_ry + cur_ly + cur_ry) * 0.25f,
                        (prev_ry + cur_ry) * 0.5f,
                        (cur_ly + cur_ry) * 0.5f
                    };
                    float vz[9] = {
                        prev_lz, prev_rz, cur_lz, cur_rz,
                        (prev_lz + prev_rz) * 0.5f,
                        (prev_lz + cur_lz) * 0.5f,
                        (prev_lz + prev_rz + cur_lz + cur_rz) * 0.25f,
                        (prev_rz + cur_rz) * 0.5f,
                        (cur_lz + cur_rz) * 0.5f
                    };
                    float shade[9] = {
                        prev_lshade, prev_rshade, lshade, rshade,
                        (prev_lshade + prev_rshade) * 0.5f,
                        (prev_lshade + lshade) * 0.5f,
                        (prev_lshade + prev_rshade + lshade + rshade) * 0.25f,
                        (prev_rshade + rshade) * 0.5f,
                        (lshade + rshade) * 0.5f
                    };
                    for (int si = 0; si < 9; si++) {
                        if (shade[si] < 0.08f)
                            shade[si] = 0.08f;
                    }
                    float u0 = (rt->tex_w > 0) ? (0.5f / (float)rt->tex_w) : 0.0f;
                    float u1 = (rt->tex_w > 0) ? (((float)rt->tex_w - 0.5f) / (float)rt->tex_w) : 1.0f;
                    float v0 = (rt->tex_h > 0) ? (0.5f / (float)rt->tex_h) : 0.0f;
                    float v1 = (rt->tex_h > 0) ? (((float)rt->tex_h - 0.5f) / (float)rt->tex_h) : 1.0f;
                    float um = (u0 + u1) * 0.5f;
                    float vm = (v0 + v1) * 0.5f;
                    float uv[9][2] = {
                        { u0, v0 }, { u1, v0 },
                        { u0, v1 }, { u1, v1 },
                        { um, v0 }, { u0, vm },
                        { um, vm }, { u1, vm },
                        { um, v1 }
                    };
                    float tex_kind = 3.0f + (float)rt->tex_slot;
                    tm_emit_route_subquad(vx, vy, vz, uv, shade, tex_kind,
                                          0, 4, 5, 6,
                                          vbuf, vcap, nvtx, tribuf, tcap, ntris);
                    tm_emit_route_subquad(vx, vy, vz, uv, shade, tex_kind,
                                          4, 1, 6, 7,
                                          vbuf, vcap, nvtx, tribuf, tcap, ntris);
                    tm_emit_route_subquad(vx, vy, vz, uv, shade, tex_kind,
                                          5, 6, 2, 8,
                                          vbuf, vcap, nvtx, tribuf, tcap, ntris);
                    tm_emit_route_subquad(vx, vy, vz, uv, shade, tex_kind,
                                          6, 7, 8, 3,
                                          vbuf, vcap, nvtx, tribuf, tcap, ntris);
                }
                if (trace && seg->type >= 0 && seg->type < 16) {
                    type_tris[seg->type] += *ntris - prev_ntris;
                    type_vtx[seg->type] += *nvtx - prev_nvtx;
                    float bx[4] = { prev_lx, prev_rx, cur_lx, cur_rx };
                    float by[4] = { prev_ly, prev_ry, cur_ly, cur_ry };
                    float bz[4] = { prev_lz, prev_rz, cur_lz, cur_rz };
                    for (int k = 0; k < 4; k++) {
                        if (bx[k] < type_minx[seg->type]) type_minx[seg->type] = bx[k];
                        if (by[k] < type_miny[seg->type]) type_miny[seg->type] = by[k];
                        if (bz[k] < type_minz[seg->type]) type_minz[seg->type] = bz[k];
                        if (bx[k] > type_maxx[seg->type]) type_maxx[seg->type] = bx[k];
                        if (by[k] > type_maxy[seg->type]) type_maxy[seg->type] = by[k];
                        if (bz[k] > type_maxz[seg->type]) type_maxz[seg->type] = bz[k];
                    }
                }
            }

            prev_lx = cur_lx; prev_ly = cur_ly; prev_lz = cur_lz;
            prev_rx = cur_rx; prev_ry = cur_ry; prev_rz = cur_rz;
            prev_lshade = lshade;
            prev_rshade = rshade;
            have_prev = 1;
            sample++;
            if (draw_t >= 0x1000)
                break;
            t += rt->step / speed;
            if (t <= draw_t)
                t = draw_t + 1;
        }
    }

    if (trace) {
        for (int i = 0; i < nrtypes && i < 16; i++) {
            if (type_tris[i] == 0)
                continue;
            fprintf(stderr,
                    "v8: route_visual type=%d tris=%d gpu_vtx=%d tex_slot=%d tex=%dx%d flags=0x%x bounds=(%.3f..%.3f, %.3f..%.3f, %.3f..%.3f)\n",
                    i, type_tris[i], type_vtx[i], rtypes[i].tex_slot,
                    rtypes[i].tex_w, rtypes[i].tex_h,
                    (unsigned)(uint16_t)rtypes[i].flags,
                    type_minx[i], type_maxx[i],
                    type_miny[i], type_maxy[i],
                    type_minz[i], type_maxz[i]);
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
         * at obj+0x68. FUN_8001de08 renders obj+0x30 normally and selects
         * obj+0x68 only when the camera-space Z distance exceeds obj+0x6c.
         * Static upload therefore walks siblings like FUN_8001ac44 but does
         * not render these distance-selected alternates unconditionally. */
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
    int32_t radius = tm_radius_from_obstacle_stream(stream);
    memset(o->obj, 0, sizeof(o->obj));
    *(uint32_t *)(o->obj + 0x00) = 0x40u;
    *(int16_t  *)(o->obj + 0x06) = (int16_t)(0x6000 + (*count & 0x1fff));
    memcpy(o->obj + 0x10, mat, sizeof(*mat));
    *(int32_t  *)(o->obj + 0x24) = mat->t[0];
    *(int32_t  *)(o->obj + 0x28) = mat->t[1];
    *(int32_t  *)(o->obj + 0x2c) = mat->t[2];
    *(int32_t  *)(o->obj + 0x48) = mat->t[0];
    *(int32_t  *)(o->obj + 0x4c) = mat->t[1];
    *(int32_t  *)(o->obj + 0x50) = mat->t[2];
    *(int32_t  *)(o->obj + 0x54) = radius;
    tm_collision_shape_from_obstacle_stream(stream, o->coll_shape, radius);
    *(uint32_t *)(o->obj + 0x5c) = (uint32_t)(uintptr_t)o->coll_shape;
    o->probe_stream = stream;
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

static int tm_str_ieq(const char *a, const char *b)
{
    while (*a != 0 && *b != 0) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;
        a++;
        b++;
    }
    return *a == 0 && *b == 0;
}

static uint32_t tm_rd32le_mem(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8)
        | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static intptr_t tm_ww_warehouse_init(intptr_t self, int mode, int arg)
{
    uint32_t *obj = (uint32_t *)(uintptr_t)self;
    (void)arg;
    if (obj != NULL && mode == 1) {
        uint8_t *geom = (uint8_t *)(uintptr_t)obj[0x0c];
        if (geom != NULL)
            *(uint16_t *)(geom + 0x28) = 0x14;
        Object_SetCallbackPsxSlot(obj, (uintptr_t)FUN_800223dc);
    }
    return 0;
}

static int tm_ww_train_closest_sample(const uint32_t *obj)
{
    if (g_ww_train_path_count <= 0)
        return 0;
    int32_t x = (int32_t)obj[0x12];
    int32_t z = (int32_t)obj[0x14];
    int best = 0;
    uint64_t best_d = UINT64_MAX;
    for (int i = 0; i < g_ww_train_path_count; i++) {
        int64_t dx = (int64_t)x - g_ww_train_path[i].x;
        int64_t dz = (int64_t)z - g_ww_train_path[i].z;
        uint64_t d = (uint64_t)(dx * dx + dz * dz);
        if (d < best_d) {
            best_d = d;
            best = i;
        }
    }
    return best;
}

static void tm_ww_train_apply_pose(uint32_t *obj, int sample_idx)
{
    if (g_ww_train_path_count <= 0)
        return;
    sample_idx %= g_ww_train_path_count;
    if (sample_idx < 0)
        sample_idx += g_ww_train_path_count;
    const TmTrainPathSample *s = &g_ww_train_path[sample_idx];

    obj[0x09] = (uint32_t)s->x;
    obj[0x0a] = (uint32_t)s->y;
    obj[0x0b] = (uint32_t)s->z;
    obj[0x12] = (uint32_t)s->x;
    obj[0x13] = (uint32_t)s->y;
    obj[0x14] = (uint32_t)s->z;

    *(int16_t *)((uint8_t *)obj + 0x10) = s->dz;
    *(int16_t *)((uint8_t *)obj + 0x14) = s->dx;
    *(int16_t *)((uint8_t *)obj + 0x18) = 0;
    *(int16_t *)((uint8_t *)obj + 0x1c) = (int16_t)-s->dx;
    *(int16_t *)((uint8_t *)obj + 0x20) = s->dz;
    *(int16_t *)((uint8_t *)obj + 0x22) = 0;
}

static intptr_t tm_ww_train_runtime(intptr_t self, int mode, int arg)
{
    uint32_t *obj = (uint32_t *)(uintptr_t)self;
    if (obj == NULL)
        return 0;

    if (mode == 1) {
        int sample = tm_ww_train_closest_sample(obj);
        obj[0x29] = 1;
        obj[0x2a] = 0x5000;
        obj[0x2c] = (uint32_t)(sample << 16);
        *obj |= 0x180u;
        tm_ww_train_apply_pose(obj, sample);
        return 0;
    }

    if (mode == 0 && g_ww_train_path_count > 0) {
        uint32_t progress = obj[0x2c] + (obj[0x2a] ? obj[0x2a] : 0x5000);
        uint32_t wrap = (uint32_t)g_ww_train_path_count << 16;
        if (wrap != 0) {
            while (progress >= wrap)
                progress -= wrap;
        }
        obj[0x2c] = progress;
        tm_ww_train_apply_pose(obj, (int)(progress >> 16));
        return 0;
    }

    return (intptr_t)FUN_800223dc(obj, mode, (intptr_t)arg);
}

intptr_t TM_WW_TrainEngineCallback(intptr_t self, int mode, int arg)
{
    return tm_ww_train_runtime(self, mode, arg);
}

intptr_t TM_WW_TrainCoalCallback(intptr_t self, int mode, int arg)
{
    return tm_ww_train_runtime(self, mode, arg);
}

intptr_t TM_WW_TrainFlatbedCallback(intptr_t self, int mode, int arg)
{
    return tm_ww_train_runtime(self, mode, arg);
}

static uintptr_t tm_host_level_callback_by_offset(const char *stem, uint32_t off)
{
    if (tm_str_ieq(stem, "WILDWEST")) {
        switch (off) {
        case 0x04a0: return (uintptr_t)tm_ww_warehouse_init;
        case 0x09a8: return (uintptr_t)TM_WW_TrainEngineCallback;
        case 0x0fa4: return (uintptr_t)TM_WW_TrainCoalCallback;
        case 0x10a8: return (uintptr_t)TM_WW_TrainFlatbedCallback;
        default: break;
        }
    }
    return 0;
}

static uintptr_t tm_resolve_head_callback(const char *levelPath,
                                          const char *headName,
                                          int *resolved)
{
    char stem[32];
    char path[128];
    const char *base;
    const char *dot;
    FILE *f;
    uint8_t hdr[8];
    uint8_t *dll;
    long len;
    uintptr_t cb = 0;

    if (resolved != NULL)
        *resolved = 0;
    if (headName == NULL || headName[0] == 0)
        return (uintptr_t)FUN_800223dc;

    base = strrchr(levelPath, '\\');
    if (base == NULL) base = strrchr(levelPath, '/');
    base = (base != NULL) ? base + 1 : levelPath;
    dot = strrchr(base, '.');
    size_t n = (dot != NULL) ? (size_t)(dot - base) : strlen(base);
    if (n >= sizeof(stem)) n = sizeof(stem) - 1;
    memcpy(stem, base, n);
    stem[n] = 0;

    snprintf(path, sizeof(path), "TERRAIN\\%s.DLL", stem);
    f = fopen(path, "rb");
    if (f == NULL) {
        snprintf(path, sizeof(path), "Terrain\\%s.DLL", stem);
        f = fopen(path, "rb");
    }
    if (f == NULL)
        return (uintptr_t)FUN_800223dc;

    if (fread(hdr, 1, sizeof(hdr), f) != sizeof(hdr)) {
        fclose(f);
        return (uintptr_t)FUN_800223dc;
    }
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (len <= 0 || len > 256 * 1024) {
        fclose(f);
        return (uintptr_t)FUN_800223dc;
    }
    dll = (uint8_t *)malloc((size_t)len);
    if (dll == NULL) {
        fclose(f);
        return (uintptr_t)FUN_800223dc;
    }
    if (fread(dll, 1, (size_t)len, f) != (size_t)len) {
        free(dll);
        fclose(f);
        return (uintptr_t)FUN_800223dc;
    }
    fclose(f);

    for (uint32_t off = 8; off + 8 <= (uint32_t)len; off += 8) {
        uint32_t strOff = tm_rd32le_mem(dll + off);
        uint32_t fnOff = tm_rd32le_mem(dll + off + 4);
        if (strOff == 0 && fnOff == 0)
            break;
        if (strOff >= (uint32_t)len || fnOff == 0 || fnOff >= (uint32_t)len)
            break;
        const char *s = (const char *)dll + strOff;
        if (memchr(s, 0, (size_t)len - strOff) == NULL)
            break;
        if (strcmp(s, headName) == 0) {
            cb = tm_host_level_callback_by_offset(stem, fnOff);
            if (cb != 0 && resolved != NULL)
                *resolved = 1;
            break;
        }
    }
    free(dll);
    return cb != 0 ? cb : (uintptr_t)FUN_800223dc;
}

static uint8_t *tm_source_build_placed_object(const TmBank *bank,
                                              const TmHead *h,
                                              uintptr_t callback)
{
    uint8_t *obj;
    uint32_t flags;

    flags = (h->flags & 4u) << 1;
    obj = (uint8_t *)FUN_80021b80((intptr_t (*)(intptr_t, int, int))callback,
                                  (intptr_t)bank->source_bank,
                                  (uint16_t)h->slot, flags);
    if (obj == NULL)
        return NULL;

    *(uint32_t *)(obj + 0x00) = h->flags & 0xfff867feu;
    *(uint8_t  *)(obj + 0x04) = (uint8_t)h->type;
    *(int16_t  *)(obj + 0x06) = h->id;
    *(uint8_t  *)(obj + 0x08) = h->script;
    *(int16_t  *)(obj + 0x0c) = (int16_t)h->initial_strength;
    *(int16_t  *)(obj + 0x0e) = (int16_t)h->initial_strength;
    /* LOAD 801006f0 copies HEAD rotation halfwords 0x14/0x16 as one
     * unaligned word into obj+0x40..0x43, then HEAD 0x18 into +0x44.
     * In host terms that leaves rot0 at +0x40 and rot1 at +0x42. */
    *(int16_t  *)(obj + 0x40) = h->ry;
    *(int16_t  *)(obj + 0x42) = h->rx;
    *(int16_t  *)(obj + 0x44) = h->rz;
    *(int32_t  *)(obj + 0x48) = h->raw_x;
    *(int32_t  *)(obj + 0x4c) = h->raw_y;
    *(int32_t  *)(obj + 0x50) = h->raw_z;
    Object_SetCallbackPsxSlot(obj, callback);
    return obj;
}

static void tm_source_propagate_strength_to_children(uint8_t *obj,
                                                     int16_t strength)
{
    uint8_t *child = (uint8_t *)(uintptr_t)(uint32_t)*(uint32_t *)(obj + 0x38);
    while (child != NULL) {
        *(int16_t *)(child + 0x0c) = strength;
        *(int16_t *)(child + 0x0e) = strength;
        child = (uint8_t *)(uintptr_t)(uint32_t)*(uint32_t *)(child + 0x34);
    }
}

static void tm_build_source_dynamic_objects(const TmBank *banks, int nbanks,
                                            const TmHead *heads, int nheads)
{
    int built = 0;
    int resolved = 0;
    int table_fallback = 0;

    for (int hi = 0; hi < nheads; hi++) {
        const TmHead *h = &heads[hi];
        uintptr_t callback;
        int did_resolve = 0;
        uint8_t *obj;

        /* LOAD.DLL's switch table routes HEAD types 2, 3, and 4 to the
         * default live-object path at 80100a08.  Type 0 is inserted into the
         * static BSP, type 5 into the placeholder/path list, and type 6 into
         * the light/object cleanup list. */
        if (h->type != 2 && h->type != 3 && h->type != 4)
            continue;
        if (h->bank < 0 || h->bank >= nbanks || h->slot < 0)
            continue;
        if (banks[h->bank].source_bank == 0)
            continue;

        callback = tm_resolve_head_callback(g_v8_level_exp_path, h->name,
                                            &did_resolve);
        if (did_resolve) resolved++;
        else table_fallback++;

        obj = tm_source_build_placed_object(&banks[h->bank], h, callback);
        if (obj == NULL)
            continue;

        tm_source_propagate_strength_to_children(obj,
                                                 (int16_t)h->initial_strength);
        if (FUN_8002036c((uint32_t *)obj) != 0) {
            FUN_8001b0c4((uint32_t *)obj);
        }
        built++;
    }

    fprintf(stderr,
            "v8: TerrainMesh -- source dynamic collision objects=%d callbacks=%d fallback=%d\n",
            built, resolved, table_fallback);
}

static int32_t *tm_source_list_node(uint32_t *obj)
{
    int32_t *node = (int32_t *)FUN_8001178c(0x0c, 1);
    if (node == NULL)
        return NULL;
    node[0] = 0;
    node[1] = 0;
    node[2] = (int32_t)(uintptr_t)obj;
    return node;
}

static int32_t *tm_source_bsp_parse_node(const uint8_t *payload,
                                         uint32_t size, uint32_t *pos)
{
    int32_t *node;
    int32_t kind;

    if (*pos + 2 > size)
        return NULL;

    kind = tm_rd16be_s(payload, *pos);
    *pos += 2;

    if (kind == 0) {
        node = (int32_t *)FUN_8001178c(0x10, 1);
        if (node == NULL)
            return NULL;
        node[0] = 0;
        node[1] = (int32_t)(uintptr_t)(node + 2);
        node[2] = 0;
        node[3] = (int32_t)(uintptr_t)(node + 1);
        return node;
    }

    if (kind != 1 && kind != 2)
        return NULL;
    if (*pos + 4 > size)
        return NULL;

    node = (int32_t *)FUN_8001178c(0x10, 1);
    if (node == NULL)
        return NULL;
    node[0] = kind;
    node[1] = tm_rd32be_s(payload, *pos);
    *pos += 4;
    node[2] = (int32_t)(uintptr_t)tm_source_bsp_parse_node(payload, size, pos);
    node[3] = (int32_t)(uintptr_t)tm_source_bsp_parse_node(payload, size, pos);
    if (node[2] == 0 || node[3] == 0)
        return NULL;
    return node;
}

static void tm_source_bsp_insert_object(int32_t *node, int32_t *child)
{
    int32_t kind;
    uint8_t *obj;
    int32_t *tail;

    if (node == NULL || child == NULL)
        return;
    kind = node[0];
    if (kind == 1) {
        obj = (uint8_t *)(uintptr_t)(uint32_t)child[2];
        tm_source_bsp_insert_object((int32_t *)(uintptr_t)(uint32_t)
                                    node[node[1] < *(int32_t *)(obj + 0x48) ? 3 : 2],
                                    child);
        return;
    }
    if (kind == 2) {
        obj = (uint8_t *)(uintptr_t)(uint32_t)child[2];
        tm_source_bsp_insert_object((int32_t *)(uintptr_t)(uint32_t)
                                    node[node[1] < *(int32_t *)(obj + 0x50) ? 3 : 2],
                                    child);
        return;
    }
    if (kind != 0)
        return;

    tail = (int32_t *)(uintptr_t)(uint32_t)node[3];
    node[3] = (int32_t)(uintptr_t)child;
    tail[0] = (int32_t)(uintptr_t)child;
    child[1] = (int32_t)(uintptr_t)tail;
    child[0] = (int32_t)(uintptr_t)(node + 2);
}

static void tm_build_source_static_tree(const uint8_t *bsp_payload,
                                        uint32_t bsp_size,
                                        const TmBank *banks, int nbanks,
                                        const TmHead *heads, int nheads)
{
    int32_t *root;
    int inserted = 0;
    int resolved = 0;
    int table_fallback = 0;
    int trace_static = 0;
    int trace_printed = 0;

    iRam000006fc = 0;
    uRam000006fc = 0;
    g_v8_static_collision_obj_count = 0;
    if (bsp_payload == NULL)
        return;

    uint32_t pos = 0;
    root = tm_source_bsp_parse_node(bsp_payload, bsp_size, &pos);
    if (root == NULL)
        return;

    iRam000006fc = (uintptr_t)root;
    uRam000006fc = (uintptr_t)root;
    {
        const char *env = getenv("V8_TRACE_STATIC_OBJECTS");
        trace_static = (env != NULL && env[0] != 0 && env[0] != '0');
    }

    for (int hi = 0; hi < nheads; hi++) {
        const TmHead *h = &heads[hi];
        uint8_t *obj;
        int32_t *node;
        MATRIX identity;
        uintptr_t callback;
        int did_resolve = 0;

        /* LOAD.DLL inserts only DAT_80107da0 entries into the terrain BSP.
         * FUN_801006f0 appends that list from its case-0 HEAD path; the
         * default/type-5/type-6 paths feed other runtime lists instead. */
        if (h->type != 0)
            continue;
        if (h->bank < 0 || h->bank >= nbanks || h->slot < 0)
            continue;
        if (banks[h->bank].source_bank == 0)
            continue;

        callback = tm_resolve_head_callback(g_v8_level_exp_path, h->name,
                                            &did_resolve);
        if (did_resolve) resolved++;
        else table_fallback++;

        obj = tm_source_build_placed_object(&banks[h->bank], h, callback);
        if (obj == NULL)
            continue;

        FUN_8001d708((uint32_t *)obj);
        FUN_8001dc1c((intptr_t)obj);
        if (callback != 0 &&
            ((intptr_t (*)(intptr_t, int, int))callback)((intptr_t)obj, 1, 0) < 0)
            continue;
        FUN_8001b0c4((uint32_t *)obj);
        (void)FUN_8001ec48((uint32_t *)obj);

        memset(&identity, 0, sizeof(identity));
        identity.m[0][0] = 0x1000;
        identity.m[1][1] = 0x1000;
        identity.m[2][2] = 0x1000;
        FUN_80101574((intptr_t)obj, &identity);

        node = tm_source_list_node((uint32_t *)obj);
        if (node == NULL)
            continue;
        tm_source_bsp_insert_object(root, node);
        if (g_v8_static_collision_obj_count <
            (int)(sizeof g_v8_static_collision_objs / sizeof g_v8_static_collision_objs[0])) {
            g_v8_static_collision_objs[g_v8_static_collision_obj_count++] = (uintptr_t)obj;
        }
        if (g_static_debug_count <
            (int)(sizeof g_static_debug / sizeof g_static_debug[0])) {
            TmStaticDebugInfo *dbg = &g_static_debug[g_static_debug_count++];
            dbg->obj = (uintptr_t)obj;
            dbg->head_index = hi;
            dbg->slot = h->slot;
            snprintf(dbg->name, sizeof(dbg->name), "%s", h->name);
        }
        if (trace_static && trace_printed < 160) {
            uint8_t *child = (uint8_t *)(uintptr_t)(uint32_t)*(uint32_t *)(obj + 0x38);
            uint32_t child_flags = child ? *(uint32_t *)(child + 0x00) : 0;
            uint32_t child_shape = child ? *(uint32_t *)(child + 0x5c) : 0;
            int32_t child_radius = child ? *(int32_t *)(child + 0x54) : 0;
            uint8_t *shape = (uint8_t *)(uintptr_t)*(uint32_t *)(obj + 0x5c);
            int shape_type = shape ? *(uint16_t *)shape : 0;
            int32_t sx0 = 0, sy0 = 0, sz0 = 0, sx1 = 0, sy1 = 0, sz1 = 0;
            if (shape != NULL && shape_type == 1) {
                sx0 = *(int32_t *)(shape + 4);
                sy0 = *(int32_t *)(shape + 8);
                sz0 = *(int32_t *)(shape + 12);
                sx1 = *(int32_t *)(shape + 16);
                sy1 = *(int32_t *)(shape + 20);
                sz1 = *(int32_t *)(shape + 24);
            }
            fprintf(stderr,
                    "v8: static_obj %03d name=%s slot=%d flags=0x%x layer=%d pos24=(0x%x,0x%x,0x%x) pos48=(0x%x,0x%x,0x%x) r=0x%x shape=0x%x stype=%d saabb=(0x%x,0x%x,0x%x..0x%x,0x%x,0x%x) child=%p child_flags=0x%x child_r=0x%x child_shape=0x%x\n",
                    inserted, h->name, h->slot,
                    (unsigned)*(uint32_t *)(obj + 0x00),
                    (int)*(int16_t *)(obj + 0x06),
                    (unsigned)*(uint32_t *)(obj + 0x24),
                    (unsigned)*(uint32_t *)(obj + 0x28),
                    (unsigned)*(uint32_t *)(obj + 0x2c),
                    (unsigned)*(uint32_t *)(obj + 0x48),
                    (unsigned)*(uint32_t *)(obj + 0x4c),
                    (unsigned)*(uint32_t *)(obj + 0x50),
                    (unsigned)*(uint32_t *)(obj + 0x54),
                    (unsigned)*(uint32_t *)(obj + 0x5c),
                    shape_type,
                    (unsigned)sx0, (unsigned)sy0, (unsigned)sz0,
                    (unsigned)sx1, (unsigned)sy1, (unsigned)sz1,
                    (void *)child,
                    (unsigned)child_flags,
                    (unsigned)child_radius,
                    (unsigned)child_shape);
            trace_printed++;
        }
        inserted++;
    }

    fprintf(stderr,
            "v8: TerrainMesh -- source static collision objects=%d callbacks=%d fallback=%d\n",
            inserted, resolved, table_fallback);
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
    if (nbanks > 1 && banks[1].atlas.tex != 0) {
        g_terrainmesh_tex_bank1 = banks[1].atlas.tex;
        fprintf(stderr, "v8: TerrainMesh -- XOBF texture atlas bank1 %dx%d slots=%d\n",
                banks[1].atlas.w, banks[1].atlas.h, banks[1].atlas.slots);
    }
    tm_upload_route_textures(rtypes, nrtypes);
#endif
    tm_log_head_name_summary(heads, nheads);
    if (strstr(g_v8_level_exp_path, "WILDWEST") != NULL ||
        strstr(g_v8_level_exp_path, "WildWest") != NULL)
        tm_build_wildwest_train_path(juncs, njuncs, rsegs, nrsegs);
    tm_build_spawn_placeholders(banks, nbanks, heads, nheads);
    tm_build_source_static_tree(bsp_payload, bsp_size, banks, nbanks, heads, nheads);
    tm_build_source_dynamic_objects(banks, nbanks, heads, nheads);

    {
        int obstacle_cap = 8192;
        TmObstacleObj *objs = (TmObstacleObj *)FUN_8001178c(
            (uint32_t)((size_t)obstacle_cap * sizeof(TmObstacleObj)), 1);
        int nobjs = 0;
        if (objs != NULL) {
            memset(objs, 0, (size_t)obstacle_cap * sizeof(TmObstacleObj));
            nobjs = tm_build_obstacle_instances(banks, nbanks, heads, nheads,
                                                objs, obstacle_cap);
            tm_build_object_bsp(bsp_payload, bsp_size, objs, nobjs);
            Heap_Free(g_obstacle_objs);
            if (nobjs > 0) {
                g_obstacle_objs = objs;
                g_obstacle_nobjs = nobjs;
                g_obstacle_raw = (uint8_t *)raw;
            } else {
                Heap_Free(objs);
                g_obstacle_objs = NULL;
                g_obstacle_nobjs = 0;
                g_obstacle_raw = NULL;
            }
        }
    }

    for (int hi = 0; hi < nheads; hi++) {
        const TmHead *h = &heads[hi];
        if (h->type == 1 || h->type == 6 || h->type > 6) continue;
        if (strcmp(h->name, "M1train_engine_1") == 0 ||
            strcmp(h->name, "M1train_coalcar_1") == 0 ||
            strcmp(h->name, "M1train_flatbed_1") == 0)
            continue;
        if (h->type == 5 &&
            tm_resolve_global_head_callback(h->name) != (uintptr_t)LAB_8003c61c)
            continue;
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
        int head_first_tri = ntris;
        tm_build_slot_like_original(&banks[h->bank], h->slot,
                                    &root_xf, 1, (h->flags & 4u) << 1,
                                    0, seen, vbuf, vcap, &nvtx,
                                    tribuf, tcap, &ntris, &bad);
        tm_trace_head_visual_extent(h, tribuf, head_first_tri, ntris);
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

    /* LOAD 80104550 dispatches RSEG edges through 801036bc/80102bd4 after
     * selecting the XRTP descriptor.  Wild West's rail bed is the textured
     * type-0 route loop; type 2 is only a narrow connector and carries no TIM
     * payload of its own. */
    route_tris = tm_emit_route_strips(juncs, njuncs, rsegs, nrsegs,
                                      rtypes, nrtypes,
                                      vbuf, vcap, &nvtx,
                                      tribuf, tcap, &ntris);

    fprintf(stderr, "v8: TerrainMesh -- instanced XOBF banks=%d heads=%d roots=%d junc_nodes=%d rsegs=%d rtypes=%d junc_tris=%d route_tris=%d cpu_tris=%d gpu_vtx=%d bad_pkts=%d\n",
            nbanks, nheads, roots, njuncs, nrsegs, nrtypes,
            junc_patch_tris, route_tris, ntris, nvtx, bad);
    fprintf(stderr, "v8: TerrainMesh -- texture uv ground_xbmp=%d ground_none=%d xobf=%d untextured=%d\n",
            g_tm_uv_ground_xbmp, g_tm_uv_ground_none,
            g_tm_uv_xobf, g_tm_uv_none);
    fprintf(stderr, "v8: TerrainMesh -- packet kinds");
    for (int i = 0; i < 16; i++) {
        if (g_tm_pkt_kind[i] != 0)
            fprintf(stderr, " %x=%d/%d/%d",
                    i, g_tm_pkt_kind[i], g_tm_pkt_uv_kind[i],
                    g_tm_pkt_no_uv_kind[i]);
    }
    fprintf(stderr, "\n");
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
    g_terrainmesh_has_dynamic_water = 0;
    g_terrainmesh_tex = 0;
    g_terrainmesh_tex_bank1 = 0;
    g_terrainmesh_route_tex0 = 0;
    g_terrainmesh_route_tex1 = 0;
    g_terrainmesh_xbmp_w = 0;
    g_terrainmesh_xbmp_h = 0;
    g_terrainmesh_xbmp_x = 0;
    g_terrainmesh_xbmp_y = 0;
    g_tm_uv_ground_xbmp = 0;
    g_tm_uv_ground_xbmp_oob = 0;
    g_tm_uv_ground_none = 0;
    g_tm_uv_xobf = 0;
    g_tm_uv_none = 0;
    memset(g_tm_pkt_kind, 0, sizeof(g_tm_pkt_kind));
    memset(g_tm_pkt_uv_kind, 0, sizeof(g_tm_pkt_uv_kind));
    memset(g_tm_pkt_no_uv_kind, 0, sizeof(g_tm_pkt_no_uv_kind));
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
    g_terrainmesh_tex = 0;
    g_terrainmesh_tex_bank1 = 0;
    g_terrainmesh_route_tex0 = 0;
    g_terrainmesh_route_tex1 = 0;
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
    int old_allow_gl = g_tm_allow_gl_upload;
    g_tm_allow_gl_upload = 0;
    int nvtx = tm_parse_level_instances(raw, fsz,
                                        vbuf, cap,
                                        tribuf, tricap, &ntris);
    g_tm_allow_gl_upload = old_allow_gl;
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
