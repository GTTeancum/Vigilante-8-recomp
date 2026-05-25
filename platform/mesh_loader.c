/* mesh_loader.c -- load vehicle models from input/SHELL/VEHICLES.EXP.
 *
 * Parses the IFF/XOBF/BIN structure, extracts polygon geometry for each
 * of the 14 vehicle characters, and uploads interleaved position+colour
 * VBOs to OpenGL.  Called from renderer.c init_once().
 *
 * BIN chunk format (little-endian):
 *   Header (24 B): num_bones(i32), bone_table_off(i32), ..., num_segs(i32), seg_off(i32)
 *   Bone table: array of i32 offsets relative to bone_table_base
 *   Bone descriptor:
 *     +0x00 i32 vert_count
 *     +0x04 i32 vert_offset    (relative to descriptor address)
 *     +0x08 i32 norm_count
 *     +0x0c i32 norm_offset    (relative to descriptor address)
 *     +0x10 u16 poly_count
 *     +0x14 i32 poly_offset    (relative to descriptor address)
 *   Vertex: i16 x, y, z, pad  (8 bytes each)
 *   Polygon packet: [0]R [1]G [2]B [3]type_byte, then u16 vertex indices
 *
 * Packet sizes by (type_byte & 0xf) nibble:
 *   0->12  1->28  2->20  3->28
 *   4->12  5->20  6->12  7->20
 *   8->16  9->24  10->12 11->24
 *   12->20 13->20 14->0  15->20
 *
 * Quad nibbles (4 vertex indices at +4,+6,+8,+10): 4, 5, 7
 * All other handled nibbles: 3 vertex indices at +4,+6,+8
 *
 * Scale: source group descriptor byte at +0x18, matching the XOBF render
 * path used by SLUS 8001be5c and platform/terrain_mesh.c.
 * Y inverted:   PSX Y-down -> OpenGL Y-up
 *
 * Confirmed across all 14 vehicles: 0 invalid packets with this table.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>

#if defined(V8_HAVE_SDL) && defined(V8_HAVE_GL)
#include <GL/gl3w.h>

/* ------------------------------------------------------------------ */
/* Public state consumed by renderer.c                                  */
/* ------------------------------------------------------------------ */
GLuint g_mesh_vao[14];
int    g_mesh_vtx[14];   /* vertex (not triangle) count */
GLuint g_wheel_mesh_vao[10];
int    g_wheel_mesh_vtx[10];

/* ------------------------------------------------------------------ */
/* Format constants                                                     */
/* ------------------------------------------------------------------ */
#define MAX_VEHICLES 14
#define MAX_TRIS_PER_VEH 4096   /* headroom above observed ~402 tris */
#define MAX_WHEEL_KINDS 10
#define MAX_TRIS_PER_WHEEL 128

static const int PKT_SIZE[16] = {
    12, 28, 20, 28,   /* nibble 0-3  */
    12, 20, 12, 20,   /* nibble 4-7  */
    16, 24, 12, 24,   /* nibble 8-11 */
    20, 20,  0, 20,   /* nibble 12-15 */
};
/* 1 = nibble has 4th vertex index at packet+10 (quad) */
static const int IS_QUAD[16] = {
    0,0,0,0, 1,1,0,1, 0,0,0,0, 0,0,0,0
};

/* ------------------------------------------------------------------ */
/* Binary helpers                                                       */
/* ------------------------------------------------------------------ */
static uint32_t rd32be(const uint8_t *b, uint32_t o)
{
    return ((uint32_t)b[o]<<24)|((uint32_t)b[o+1]<<16)|
           ((uint32_t)b[o+2]<<8)|(uint32_t)b[o+3];
}
static uint32_t rd32le(const uint8_t *b, uint32_t o)
{
    return (uint32_t)b[o]|((uint32_t)b[o+1]<<8)|
           ((uint32_t)b[o+2]<<16)|((uint32_t)b[o+3]<<24);
}
static uint16_t rd16le(const uint8_t *b, uint32_t o)
{
    return (uint16_t)b[o]|((uint16_t)b[o+1]<<8);
}
static int16_t rds16le(const uint8_t *b, uint32_t o)
{
    return (int16_t)rd16le(b, o);
}

/* ------------------------------------------------------------------ */
/* Per-vertex data sent to GL: 6 floats (x,y,z,r,g,b)                  */
/* ------------------------------------------------------------------ */
typedef struct { float x,y,z,r,g,b; } GlVert;

/* ------------------------------------------------------------------ */
/* IFF walker: collects (payload_offset, size) for every BIN chunk     */
/* ------------------------------------------------------------------ */
#define MAX_BINS 32
static struct { uint32_t off; uint32_t sz; } s_bins[MAX_BINS];
static int s_nbin;

static void iff_walk(const uint8_t *d, uint32_t off, uint32_t end)
{
    while (off + 8 <= end && s_nbin < MAX_BINS) {
        uint32_t sz = rd32be(d, off + 4);
        uint32_t pe = off + 8 + sz;
        if (pe > end) break;
        if (memcmp(d+off, "FORM", 4) == 0) {
            iff_walk(d, off + 12, pe);   /* skip 4-byte form type */
        } else if (memcmp(d+off, "BIN ", 4) == 0) {
            s_bins[s_nbin].off = off + 8;
            s_bins[s_nbin].sz  = sz;
            s_nbin++;
        }
        off = pe + (sz & 1);   /* IFF even-byte pad */
    }
}

static int iff_find_bin_in_form(const uint8_t *d, uint32_t off, uint32_t end,
                                uint32_t *binOff, uint32_t *binSize)
{
    while (off + 8 <= end) {
        uint32_t sz = rd32be(d, off + 4);
        uint32_t pe = off + 8 + sz;
        if (pe > end) break;

        if (memcmp(d + off, "BIN ", 4) == 0) {
            *binOff = off + 8;
            *binSize = sz;
            return 1;
        }
        if (memcmp(d + off, "FORM", 4) == 0) {
            if (iff_find_bin_in_form(d, off + 12, pe, binOff, binSize))
                return 1;
        }
        off = pe + (sz & 1);
    }
    return 0;
}

static int iff_find_xobf_bin(const uint8_t *d, uint32_t off, uint32_t end,
                             int wanted, int *xobfIndex,
                             uint32_t *binOff, uint32_t *binSize)
{
    while (off + 12 <= end) {
        uint32_t sz = rd32be(d, off + 4);
        uint32_t pe = off + 8 + sz;
        if (pe > end) break;

        if (memcmp(d + off, "FORM", 4) == 0) {
            if (memcmp(d + off + 8, "XOBF", 4) == 0) {
                int thisIndex = *xobfIndex;
                (*xobfIndex)++;
                if (thisIndex == wanted)
                    return iff_find_bin_in_form(d, off + 12, pe, binOff, binSize);
            } else if (iff_find_xobf_bin(d, off + 12, pe, wanted, xobfIndex,
                                         binOff, binSize)) {
                return 1;
            }
        }
        off = pe + (sz & 1);
    }
    return 0;
}

static void parse_group_desc(const uint8_t *B, uint32_t bsz, uint32_t bd,
                             GlVert *vbuf, int *nvtx, int cap)
{
    if (bd + 0x19 > bsz) return;

    uint32_t vc  = rd32le(B, bd + 0x00);
    uint32_t vr  = rd32le(B, bd + 0x04);
    uint16_t pc  = rd16le(B, bd + 0x10);
    uint32_t pr  = rd32le(B, bd + 0x14);
    uint8_t scale_shift = B[bd + 0x18];
    float model_scale = scale_shift <= 15 ? (1.0f / (float)(1u << scale_shift))
                                          : (1.0f / 160.0f);

    uint32_t vo = bd + vr;   /* vertex table base */
    uint32_t po = bd + pr;   /* polygon data base */

    for (uint16_t pi = 0; pi < pc; pi++) {
        if (po + 4 > bsz) break;
        uint8_t typ = B[po + 3];
        int     nib = typ & 0xf;
        int     sz  = PKT_SIZE[nib];
        if (sz == 0 || po + (uint32_t)sz > bsz) { po += 4; continue; }

        /* Flat polygon colour (8-bit, stored directly) */
        float cr = B[po+0] / 255.0f;
        float cg = B[po+1] / 255.0f;
        float cb = B[po+2] / 255.0f;

        /* Vertex indices (raw uint16 at +4,+6,+8[,+10]) */
        uint16_t vi[4];
        vi[0] = rd16le(B, po+4);
        vi[1] = rd16le(B, po+6);
        vi[2] = rd16le(B, po+8);
        vi[3] = IS_QUAD[nib] ? rd16le(B, po+10) : vi[2];

        int nverts = IS_QUAD[nib] ? 4 : 3;
        int ok = 1;
        for (int k = 0; k < nverts; k++)
            if (vi[k] >= vc) { ok = 0; break; }

        if (ok) {
            /* Read positions; negate Y for PSX-down -> GL-up */
            float vx[4], vy[4], vz[4];
            for (int k = 0; k < nverts; k++) {
                uint32_t oe = vo + vi[k]*8;
                if (oe + 6 > bsz) { ok=0; break; }
                vx[k] =  rds16le(B, oe+0) * model_scale;
                vy[k] = -rds16le(B, oe+2) * model_scale;
                vz[k] =  rds16le(B, oe+4) * model_scale;
            }
            if (ok && *nvtx + 3 <= cap) {
                /* Pre-baked face lighting: compute face normal from
                 * the first triangle's edges, dot against a fixed
                 * sun direction, then modulate the polygon colour.
                 * This gives the model visible depth without a
                 * lighting pass in the shader. */
                float ex = vx[1]-vx[0], ey = vy[1]-vy[0], ez = vz[1]-vz[0];
                float fx = vx[2]-vx[0], fy = vy[2]-vy[0], fz = vz[2]-vz[0];
                float nx = ey*fz - ez*fy;
                float ny = ez*fx - ex*fz;
                float nz = ex*fy - ey*fx;
                float nl = sqrtf(nx*nx + ny*ny + nz*nz);
                if (nl > 1e-6f) { nx/=nl; ny/=nl; nz/=nl; }
                /* Sun direction (normalised): upper-right-forward */
                static const float LX= 0.408f, LY= 0.816f, LZ=-0.408f;
                float ndotl = nx*LX + ny*LY + nz*LZ;
                float lit = 0.35f + 0.65f * (ndotl > 0.0f ? ndotl : 0.0f);
                float lr = cr * lit, lg = cg * lit, lb = cb * lit;

                /* Triangle 0: v0,v1,v2 */
                vbuf[*nvtx] = (GlVert){vx[0],vy[0],vz[0],lr,lg,lb}; (*nvtx)++;
                vbuf[*nvtx] = (GlVert){vx[1],vy[1],vz[1],lr,lg,lb}; (*nvtx)++;
                vbuf[*nvtx] = (GlVert){vx[2],vy[2],vz[2],lr,lg,lb}; (*nvtx)++;
                /* Quad second triangle: v0,v2,v3 */
                if (IS_QUAD[nib] && *nvtx + 3 <= cap) {
                    vbuf[*nvtx] = (GlVert){vx[0],vy[0],vz[0],lr,lg,lb}; (*nvtx)++;
                    vbuf[*nvtx] = (GlVert){vx[2],vy[2],vz[2],lr,lg,lb}; (*nvtx)++;
                    vbuf[*nvtx] = (GlVert){vx[3],vy[3],vz[3],lr,lg,lb}; (*nvtx)++;
                }
            }
        }
        po += sz;
    }
}

/* ------------------------------------------------------------------ */
/* Parse one BIN chunk -> append triangles to *vbuf, increment *nvtx   */
/* ------------------------------------------------------------------ */
static void parse_bin(const uint8_t *B, uint32_t bsz,
                      GlVert *vbuf, int *nvtx, int cap)
{
    if (bsz < 8) return;
    uint32_t nb       = rd32le(B, 0);
    uint32_t btbase   = rd32le(B, 4);

    for (uint32_t bi = 0; bi < nb; bi++) {
        if (btbase + bi*4 + 4 > bsz) break;
        uint32_t rel = rd32le(B, btbase + bi*4);
        uint32_t bd  = btbase + rel;
        parse_group_desc(B, bsz, bd, vbuf, nvtx, cap);
    }
}

static GLuint upload_mesh(const GlVert *vbuf, int nvtx)
{
    GLuint vao, vbo;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(nvtx * sizeof(GlVert)),
                 vbuf, GL_STATIC_DRAW);
    /* attrib 0: position (xyz) */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GlVert),
                          (void *)offsetof(GlVert, x));
    /* attrib 1: colour (rgb) */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GlVert),
                          (void *)offsetof(GlVert, r));
    glBindVertexArray(0);

    return vao;
}

static uint8_t *read_whole_file(const char *path, long *outSize)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long fsz = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *raw = (uint8_t *)malloc((size_t)fsz);
    if (!raw) {
        fclose(f);
        return NULL;
    }
    if (fread(raw, 1, (size_t)fsz, f) != (size_t)fsz) {
        free(raw);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *outSize = fsz;
    return raw;
}

/* ------------------------------------------------------------------ */
/* Public init: called from renderer.c init_once()                      */
/* ------------------------------------------------------------------ */
void MeshLoader_Init(void)
{
    /* Read VEHICLES.EXP into memory.  Path is relative to cwd
     * (the game data directory containing SHELL/, TERRAIN/, etc.). */
    const char *path = "SHELL/VEHICLES.EXP";
    long fsz = 0;
    uint8_t *raw = read_whole_file(path, &fsz);
    if (!raw) {
        fprintf(stderr, "v8: MeshLoader -- cannot open '%s'\n", path);
        return;
    }

    /* Walk IFF to collect BIN chunk locations */
    s_nbin = 0;
    iff_walk(raw, 0, (uint32_t)fsz);
    fprintf(stderr, "v8: MeshLoader -- found %d vehicle BINs in %s\n",
            s_nbin, path);

    int n_load = s_nbin < MAX_VEHICLES ? s_nbin : MAX_VEHICLES;
    GlVert *vbuf = (GlVert *)malloc(sizeof(GlVert) * MAX_TRIS_PER_VEH * 3);
    if (!vbuf) { free(raw); fprintf(stderr, "v8: MeshLoader vbuf OOM\n"); return; }

    for (int vid = 0; vid < n_load; vid++) {
        int nvtx = 0;
        parse_bin(raw + s_bins[vid].off, s_bins[vid].sz,
                  vbuf, &nvtx, MAX_TRIS_PER_VEH * 3);

        g_mesh_vtx[vid] = nvtx;

        if (nvtx == 0) {
            g_mesh_vao[vid] = 0;
            fprintf(stderr, "v8:   veh[%d]: 0 vertices -- skip\n", vid);
            continue;
        }

        g_mesh_vao[vid] = upload_mesh(vbuf, nvtx);
        fprintf(stderr, "v8:   veh[%2d]: %d vertices (%d tris)\n",
                vid, nvtx, nvtx/3);
    }

    free(vbuf);
    free(raw);
}

void WheelMeshLoader_Init(void)
{
    const char *path = "COMMON.EXP";
    long fsz = 0;
    uint8_t *raw = read_whole_file(path, &fsz);
    if (!raw) {
        fprintf(stderr, "v8: WheelMeshLoader -- cannot open '%s'\n", path);
        return;
    }

    uint32_t binOff = 0, binSize = 0;
    int xidx = 0;
    if (!iff_find_xobf_bin(raw, 0, (uint32_t)fsz, 13, &xidx, &binOff, &binSize)) {
        fprintf(stderr, "v8: WheelMeshLoader -- COMMON.EXP XOBF[13] BIN not found\n");
        free(raw);
        return;
    }

    const uint8_t *B = raw + binOff;
    uint32_t bsz = binSize;
    if (bsz < 0x1c) {
        fprintf(stderr, "v8: WheelMeshLoader -- COMMON.EXP XOBF[13] BIN too small\n");
        free(raw);
        return;
    }

    uint32_t numGroups = rd32le(B, 0);
    uint32_t groupTable = rd32le(B, 4);
    GlVert *vbuf = (GlVert *)malloc(sizeof(GlVert) * MAX_TRIS_PER_WHEEL * 3);
    if (!vbuf) {
        fprintf(stderr, "v8: WheelMeshLoader vbuf OOM\n");
        free(raw);
        return;
    }

    for (int kind = 0; kind < MAX_WHEEL_KINDS; kind++) {
        uint32_t entry = 0x1c + (uint32_t)kind * 0x1c;
        if (entry + 2 > bsz) break;

        int16_t head = rds16le(B, entry);
        if (head < 0) continue;

        uint32_t group = (uint32_t)head & 0x7ffu;
        if (group >= numGroups || groupTable + group * 4 + 4 > bsz)
            continue;

        uint32_t rel = rd32le(B, groupTable + group * 4);
        uint32_t bd = groupTable + rel;
        int nvtx = 0;
        parse_group_desc(B, bsz, bd, vbuf, &nvtx, MAX_TRIS_PER_WHEEL * 3);
        g_wheel_mesh_vtx[kind] = nvtx;
        if (nvtx <= 0) {
            g_wheel_mesh_vao[kind] = 0;
            fprintf(stderr, "v8:   wheel[%d]: 0 vertices -- skip\n", kind);
            continue;
        }

        g_wheel_mesh_vao[kind] = upload_mesh(vbuf, nvtx);
        fprintf(stderr, "v8:   wheel[%d]: %d vertices (%d tris)\n",
                kind, nvtx, nvtx / 3);
    }

    free(vbuf);
    free(raw);
}

#else  /* no SDL/GL */
void MeshLoader_Init(void) {}
void WheelMeshLoader_Init(void) {}
#endif
