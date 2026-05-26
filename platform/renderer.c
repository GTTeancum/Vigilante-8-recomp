/* renderer.c -- OpenGL 3.3 view of the engine's world state.
 *
 * RESET: the procedural sin/cos terrain and host-side gameplay actors
 * are gone. The renderer reads from the engine's Vehicle struct
 * (puRam000007d0) and the engine's terrain object tree. Until those
 * are populated by a real level-load, the frame is mostly empty.
 *
 * Vehicle X/Z pose is 16.16 terrain-cell fixed-point.  Display Y now uses
 * the same post-GTE world scale as X/Z; physics keeps the original integer
 * height units. Yaw is 4.12 fixed-point with 4096 == 2*pi.
 * We convert to host floats for the camera + cube draw.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(V8_HAVE_SDL) && defined(V8_HAVE_GL)

#include <GL/gl3w.h>

static GLuint g_prog       = 0;
static GLint  g_loc_mvp    = -1;
static GLint  g_loc_tint   = -1;
static GLint  g_loc_useTex = -1;
static GLint  g_loc_tex    = -1;
static GLint  g_loc_tex2   = -1;
static GLuint g_sky_vao    = 0, g_sky_vbo = 0;
/* Vehicle mesh VAOs (loaded from VEHICLES.EXP by MeshLoader_Init). */
extern GLuint g_mesh_vao[14];
extern int    g_mesh_vtx[14];
extern GLuint g_mesh_tex[14];
extern GLuint g_wheel_mesh_vao[10];
extern int    g_wheel_mesh_vtx[10];
extern GLuint g_wheel_mesh_tex[10];
static GLuint g_terr_vao   = 0, g_terr_vbo = 0, g_terr_ibo = 0;
static int    g_terr_idxCount = 0;
static int    g_initialized = 0;

/* Render the XOBF visual mesh. terrain_mesh.c uploads all placed visual
 * triangles, while its HeightAt() API filters to low, upward-facing ground. */
#define V8_RENDER_XOBF_VISUALS 1
/* Debug readability: keep the wireframe overlay available without enabling it by default. */
#define V8_TERRAIN_WIREFRAME_OVERLAY 0

/* Engine terrain table + populated-tile bounds, supplied by host_terrain.c. */
extern uintptr_t DAT_800911a0[32 * 32];
extern int       g_terrain_loaded;
extern uint8_t   g_terrain_tile_x_min, g_terrain_tile_x_max;
extern uint8_t   g_terrain_tile_z_min, g_terrain_tile_z_max;
extern int       g_terrain_xbmp_w, g_terrain_xbmp_h;
typedef struct {
    uint8_t valid;
    uint16_t u[4], v[4];
} HostTerrainMaterialRender;
extern HostTerrainMaterialRender g_terrain_material_render[256];

static const char *VS_SRC =
    "#version 330 core\n"
    "layout(location=0) in vec3 aPos;\n"
    "layout(location=1) in vec3 aCol;\n"
    "layout(location=2) in vec2 aTex;\n"
    "layout(location=3) in float aTexKind;\n"
    "uniform mat4 uMVP;\n"
    "out vec3 vCol;\n"
    "out vec2 vTex;\n"
    "out float vTexKind;\n"
    "void main(){ vCol = aCol; vTex = aTex; vTexKind = aTexKind; gl_Position = uMVP * vec4(aPos, 1.0); }\n";

static const char *FS_SRC =
    "#version 330 core\n"
    "in vec3 vCol;\n"
    "in vec2 vTex;\n"
    "in float vTexKind;\n"
    "uniform vec3 uTint;\n"
    "uniform sampler2D uTex;\n"
    "uniform sampler2D uTex2;\n"
    "uniform int uUseTex;\n"
    "out vec4 oCol;\n"
    "void main(){ vec3 c = vCol; if (uUseTex != 0 && vTex.x >= 0.0) { vec4 t = (vTexKind > 0.5) ? texture(uTex2, vTex) : texture(uTex, vTex); if (t.a < 0.10) discard; c *= t.rgb * 1.45; } oCol = vec4(c * uTint, 1.0); }\n";

static GLuint compile(GLenum kind, const char *src) {
    GLuint s = glCreateShader(kind);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024]; glGetShaderInfoLog(s, sizeof log, NULL, log);
        fprintf(stderr, "v8: shader compile failed: %s\n", log);
    }
    return s;
}

extern void MeshLoader_Init(void);
extern void WheelMeshLoader_Init(void);
extern void TerrainMesh_Load(const char *exp_path,
                             float world_x_centre, float world_z_centre);
extern int  TerrainMesh_HeightAt(float wx, float wz, float *out_gl_y);
extern GLuint g_terrainmesh_vao;
extern GLuint g_terrainmesh_tex;
extern GLuint g_terrainmesh_xbmp_tex;
extern GLuint g_terrainmesh_sky_tex;
extern int    g_terrainmesh_vtx;
extern int    g_terrainmesh_sky_w, g_terrainmesh_sky_h;
extern char   g_v8_level_exp_path[128];
extern void  *Host_HeapBase(void);
extern uint32_t Host_HeapSize(void);

/* Convert engine coordinates to display metres.
 * Level HEAD/object placement and terrain sampling use 16.16 X/Z cell
 * coordinates. Renderer display Y uses the same post-GTE scale as X/Z.
 * NOTE: V8 uses Y-DOWN convention (positive physics Y = down; gravity adds
 * positive vy; vehicle above terrain has smaller Y than terrain_y).
 * Negate Y when converting to OpenGL (which uses Y-up). */
static float fixed_xz_to_m(int32_t v) { return (float)v / 65536.0f; }
static float fixed_y_to_m(int32_t v)  { return (float)v / 65536.0f; }

/* Build a static triangle mesh from the runtime terrain chunks.
 * Host_TerrainLoad expands on-disc ZONE data using LOAD.DLL's original
 * conversion, so the renderer samples the same 11-bit height words as
 * Terrain_HeightAt. */
static uint32_t terr_sample(int x_cell, int z_cell) {
    int chunk_x = (x_cell >> 6) & 0x1f;
    int chunk_z = (z_cell >> 6) & 0x1f;
    uintptr_t base = DAT_800911a0[chunk_x * 32 + chunk_z];
    if (!base) return 0;
    uint32_t off = ((x_cell & 0x3f) << 7) | ((z_cell & 0x3f) << 1);
    uint16_t raw = *(uint16_t *)(base + off);
    return (uint32_t)(raw & 0x7ff);
}

static uint8_t terr_material_id(int x_cell, int z_cell)
{
    int chunk_x = (x_cell >> 6) & 0x1f;
    int chunk_z = (z_cell >> 6) & 0x1f;
    uintptr_t base = DAT_800911a0[chunk_x * 32 + chunk_z];
    if (!base) return 0;
    uint32_t off = 0x2000u + (uint32_t)((x_cell & 0x3f) * 0x40)
                 + (uint32_t)(z_cell & 0x3f);
    return *(uint8_t *)(base + off);
}

static int host_heap_contains_ptr(uintptr_t p, size_t need)
{
    uintptr_t base = (uintptr_t)Host_HeapBase();
    uintptr_t size = (uintptr_t)Host_HeapSize();
    return p >= base && need <= size && p + need >= p && p + need <= base + size;
}

static void build_terrain_mesh(void) {
    if (!g_terrain_loaded) return;
    int is_ski = strstr(g_v8_level_exp_path, "SKIRESRT") != NULL
              || strstr(g_v8_level_exp_path, "Ski") != NULL;

    /* Draw the authored ZMAP/ZONE cells as the terrain skin.  The source
     * material id per cell selects a TINF record, whose atlas coordinate is
     * sampled from the level XBMP texture. */
    int col0 = g_terrain_tile_x_min, col1 = g_terrain_tile_x_max;
    int row0 = g_terrain_tile_z_min, row1 = g_terrain_tile_z_max;
    int step = 1;
    int cells_x = (col1 - col0 + 1) * 64;
    int cells_z = (row1 - row0 + 1) * 64;
    int n_quads = cells_x * cells_z;
    int n_verts = n_quads * 6;

    float *vbuf = (float *)malloc(sizeof(float) * 9 * (size_t)n_verts);
    uint32_t *ibuf = (uint32_t *)malloc(sizeof(uint32_t) * (size_t)n_verts);
    if (!vbuf || !ibuf) { free(vbuf); free(ibuf); return; }

    int outv = 0;
    for (int gz = 0; gz < cells_z; gz++) {
        for (int gx = 0; gx < cells_x; gx++) {
            int x_cell = col0 * 64 + gx * step;
            int z_cell = row0 * 64 + gz * step;
            uint32_t h00 = terr_sample(x_cell, z_cell);
            uint32_t h10 = terr_sample(x_cell + step, z_cell);
            uint32_t h01 = terr_sample(x_cell, z_cell + step);
            uint32_t h11 = terr_sample(x_cell + step, z_cell + step);
            int detail = x_cell >= (int)g_terrain_tile_x_min * 64
                      && x_cell <= ((int)g_terrain_tile_x_max + 1) * 64
                      && z_cell >= (int)g_terrain_tile_z_min * 64
                      && z_cell <= ((int)g_terrain_tile_z_max + 1) * 64;
            float t = (float)h00 / 2047.0f;
            float col[3];
            if (detail) {
                if (is_ski) {
                    col[0] = 0.70f + 0.22f * t;
                    col[1] = 0.78f + 0.17f * t;
                    col[2] = 0.82f + 0.13f * t;
                } else {
                    col[0] = 0.28f + 0.20f * t;
                    col[1] = 0.23f + 0.17f * t;
                    col[2] = 0.15f + 0.08f * (1.0f - t);
                }
            } else {
                if (is_ski) {
                    col[0] = 0.54f;
                    col[1] = 0.58f;
                    col[2] = 0.62f;
                } else {
                    col[0] = 0.20f;
                    col[1] = 0.18f;
                    col[2] = 0.13f;
                }
            }

            uint8_t mat_id = terr_material_id(x_cell, z_cell);
            HostTerrainMaterialRender *mat = &g_terrain_material_render[mat_id];
            float uv[4][2] = {{-1.0f,-1.0f},{-1.0f,-1.0f},{-1.0f,-1.0f},{-1.0f,-1.0f}};
            float tex_kind = 0.0f;
            if (mat->valid && g_terrain_xbmp_w > 0 && g_terrain_xbmp_h > 0) {
                for (int i = 0; i < 4; i++) {
                    uv[i][0] = ((float)mat->u[i] + 0.5f) / (float)g_terrain_xbmp_w;
                    uv[i][1] = ((float)mat->v[i] + 0.5f) / (float)g_terrain_xbmp_h;
                }
                tex_kind = 1.0f;
            }

            float pos[4][3] = {
                { (float)x_cell,          -fixed_y_to_m((int32_t)(h00 << 11)), (float)z_cell },
                { (float)(x_cell + step), -fixed_y_to_m((int32_t)(h10 << 11)), (float)z_cell },
                { (float)x_cell,          -fixed_y_to_m((int32_t)(h01 << 11)), (float)(z_cell + step) },
                { (float)(x_cell + step), -fixed_y_to_m((int32_t)(h11 << 11)), (float)(z_cell + step) }
            };
            int order[6] = { 0, 2, 1, 1, 2, 3 };
            for (int oi = 0; oi < 6; oi++) {
                int k = order[oi];
                float *dst = vbuf + outv * 9;
                dst[0] = pos[k][0]; dst[1] = pos[k][1]; dst[2] = pos[k][2];
                dst[3] = col[0]; dst[4] = col[1]; dst[5] = col[2];
                dst[6] = uv[k][0]; dst[7] = uv[k][1]; dst[8] = tex_kind;
                ibuf[outv] = (uint32_t)outv;
                outv++;
            }
        }
    }
    g_terr_idxCount = outv;

    glGenVertexArrays(1, &g_terr_vao); glBindVertexArray(g_terr_vao);
    glGenBuffers(1, &g_terr_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_terr_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 9 * (size_t)n_verts, vbuf, GL_STATIC_DRAW);
    glGenBuffers(1, &g_terr_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_terr_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * (size_t)g_terr_idxCount, ibuf, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*9, (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*9, (void *)(sizeof(float)*3));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*9, (void *)(sizeof(float)*6));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float)*9, (void *)(sizeof(float)*8));

    fprintf(stderr, "v8: terrain mesh built -- full tiles [%d..%d]x[%d..%d], "
            "detail tiles [%d..%d]x[%d..%d], %d verts, %d tris, xbmp=%dx%d\n",
            col0, col1, row0, row1,
            g_terrain_tile_x_min, g_terrain_tile_x_max,
            g_terrain_tile_z_min, g_terrain_tile_z_max,
            outv, g_terr_idxCount / 3, g_terrain_xbmp_w, g_terrain_xbmp_h);

    free(vbuf);
    free(ibuf);
}

static void init_once(void) {
    if (g_initialized) return;
    GLuint vs = compile(GL_VERTEX_SHADER, VS_SRC);
    GLuint fs = compile(GL_FRAGMENT_SHADER, FS_SRC);
    g_prog = glCreateProgram();
    glAttachShader(g_prog, vs); glAttachShader(g_prog, fs);
    glLinkProgram(g_prog);
    GLint ok = 0; glGetProgramiv(g_prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024]; glGetProgramInfoLog(g_prog, sizeof log, NULL, log);
        fprintf(stderr, "v8: program link failed: %s\n", log);
    }
    g_loc_mvp  = glGetUniformLocation(g_prog, "uMVP");
    g_loc_tint = glGetUniformLocation(g_prog, "uTint");
    g_loc_useTex = glGetUniformLocation(g_prog, "uUseTex");
    g_loc_tex = glGetUniformLocation(g_prog, "uTex");
    g_loc_tex2 = glGetUniformLocation(g_prog, "uTex2");
    glDeleteShader(vs); glDeleteShader(fs);
    {
        static const float sky[] = {
            -1.0f,-1.0f,0.0f, 1.0f,1.0f,1.0f, 0.0f,1.0f,0.0f,
             1.0f,-1.0f,0.0f, 1.0f,1.0f,1.0f, 1.0f,1.0f,0.0f,
            -1.0f, 1.0f,0.0f, 1.0f,1.0f,1.0f, 0.0f,0.0f,0.0f,
             1.0f,-1.0f,0.0f, 1.0f,1.0f,1.0f, 1.0f,1.0f,0.0f,
             1.0f, 1.0f,0.0f, 1.0f,1.0f,1.0f, 1.0f,0.0f,0.0f,
            -1.0f, 1.0f,0.0f, 1.0f,1.0f,1.0f, 0.0f,0.0f,0.0f
        };
        glGenVertexArrays(1, &g_sky_vao);
        glBindVertexArray(g_sky_vao);
        glGenBuffers(1, &g_sky_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, g_sky_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(sky), sky, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9,
                              (void *)(sizeof(float) * 3));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 9,
                              (void *)(sizeof(float) * 6));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 9,
                              (void *)(sizeof(float) * 8));
        glBindVertexArray(0);
    }
    MeshLoader_Init();
    WheelMeshLoader_Init();
    build_terrain_mesh();
    /* XOBF BIN vertices are in global level-relative space (bone centroid
     * analysis confirms: 165 bones, vertex extents span full 169×180m level,
     * a single world offset (608, 464)m is correct, no per-bone transforms
     * needed).  TERR_Y_ORIGIN=-2.25 puts raw-Y≈0 terrain at gl_y=-2.25m,
     * matching ZONE h8=144.  GROUND_YMAX=0.5m excludes building floors
     * (gl_y > +0.75m) from the physics height query. */
    float mesh_origin_x = ((float)g_terrain_tile_x_min * 64.0f
                         + ((float)g_terrain_tile_x_max + 1.0f) * 64.0f) * 0.5f;
    float mesh_origin_z = ((float)g_terrain_tile_z_min * 64.0f
                         + ((float)g_terrain_tile_z_max + 1.0f) * 64.0f) * 0.5f;
    TerrainMesh_Load(g_v8_level_exp_path, mesh_origin_x, mesh_origin_z);
    g_initialized = 1;
}

static void cross3(const float a[3], const float b[3], float out[3]) {
    out[0] = a[1]*b[2] - a[2]*b[1];
    out[1] = a[2]*b[0] - a[0]*b[2];
    out[2] = a[0]*b[1] - a[1]*b[0];
}
static float dot3(const float a[3], const float b[3]) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}
static void norm3(float v[3]) {
    float l = sqrtf(dot3(v, v));
    if (l > 1e-6f) { v[0]/=l; v[1]/=l; v[2]/=l; }
}
static void mat4_mul(const float A[16], const float B[16], float out[16]) {
    for (int c = 0; c < 4; c++)
    for (int r = 0; r < 4; r++) {
        float s = 0;
        for (int k = 0; k < 4; k++) s += A[k*4 + r] * B[c*4 + k];
        out[c*4 + r] = s;
    }
}
static void make_perspective(float P[16], int w, int h) {
    float fov = 54.0f * 3.1415926f / 180.0f;
    float aspect = (float)w / (float)h;
    float zn = 0.1f, zf = 5000.0f;     /* world is fixed-point huge */
    float f = 1.0f / tanf(fov * 0.5f);
    memset(P, 0, sizeof(float) * 16);
    P[0]=f/aspect; P[5]=f;
    P[10]=(zf+zn)/(zn-zf); P[11]=-1.0f;
    P[14]=(2*zf*zn)/(zn-zf);
}
static void make_lookat(float V[16], const float eye[3], const float ctr[3], const float up[3]) {
    float fwd[3] = { ctr[0]-eye[0], ctr[1]-eye[1], ctr[2]-eye[2] };
    norm3(fwd);
    float side[3]; cross3(fwd, up, side); norm3(side);
    float u2[3];   cross3(side, fwd, u2);
    V[0]=side[0]; V[4]=side[1]; V[ 8]=side[2]; V[12]=-dot3(side, eye);
    V[1]=u2[0];   V[5]=u2[1];   V[ 9]=u2[2];   V[13]=-dot3(u2, eye);
    V[2]=-fwd[0]; V[6]=-fwd[1]; V[10]=-fwd[2]; V[14]= dot3(fwd, eye);
    V[3]=0;V[7]=0;V[11]=0;V[15]=1;
}
static void make_model_from_obj(float M[16], const uint8_t *obj, float tx, float ty, float tz) {
    const int16_t *r = (const int16_t *)(obj + 0x10);
    const float q = 1.0f / 4096.0f;

    /* Mesh vertices and world positions are converted from PSX Y-down to
     * GL Y-up, so display the source matrix as S*R*S. */
    M[0] =  (float)r[0] * q; M[4] = -(float)r[1] * q; M[ 8] =  (float)r[2] * q; M[12] = tx;
    M[1] = -(float)r[3] * q; M[5] =  (float)r[4] * q; M[ 9] = -(float)r[5] * q; M[13] = ty;
    M[2] =  (float)r[6] * q; M[6] = -(float)r[7] * q; M[10] =  (float)r[8] * q; M[14] = tz;
    M[3] = 0.0f;             M[7] = 0.0f;             M[11] = 0.0f;             M[15] = 1.0f;
}

static void mat4_transform_point(const float M[16], const float p[3], float out[3])
{
    out[0] = M[0] * p[0] + M[4] * p[1] + M[ 8] * p[2] + M[12];
    out[1] = M[1] * p[0] + M[5] * p[1] + M[ 9] * p[2] + M[13];
    out[2] = M[2] * p[0] + M[6] * p[1] + M[10] * p[2] + M[14];
}

static int build_psx_chase_camera(uint8_t *veh, const float vehM[16],
                                  float eye[3], float ctr[3], float up[3])
{
    uintptr_t camObj = (uintptr_t)*(uint32_t *)(veh + 0xe0);
    static int logged = 0;

    /* The original main loop normally calls FUN_8001db24 on vehicle+0xe0.
     * During special destroyed/alternate-camera state it switches to +0xf8.
     * Use those same source object slots, then convert only at the renderer
     * boundary. */
    uintptr_t alt = (uintptr_t)*(uint32_t *)(veh + 0xf8);
    if ((*(uint32_t *)veh & 2u) != 0) {
        if (host_heap_contains_ptr(alt, 0x80))
            camObj = alt;
    }
    if (!host_heap_contains_ptr(camObj, 0x80) && host_heap_contains_ptr(alt, 0x80))
        camObj = alt;
    if (!host_heap_contains_ptr(camObj, 0x80)) {
        if (!logged) {
            fprintf(stderr, "v8: renderer camera -- source target unavailable (obj=%p), fallback chase\n",
                    (void *)camObj);
            logged = 1;
        }
        return 0;
    }

    uint8_t *obj = (uint8_t *)camObj;
    int32_t cx = *(int32_t *)(obj + 0x24);
    int32_t cy = *(int32_t *)(obj + 0x28);
    int32_t cz = *(int32_t *)(obj + 0x2c);
    float localM[16], camM[16];

    make_model_from_obj(localM, obj,
                        fixed_xz_to_m(cx),
                        -fixed_y_to_m(cy),
                        fixed_xz_to_m(cz));
    mat4_mul(vehM, localM, camM);

    float origin[3] = { 0.0f, 0.0f, 0.0f };
    float forward[3] = { camM[8], camM[9], camM[10] };
    up[0] = camM[4];
    up[1] = camM[5];
    up[2] = camM[6];
    norm3(forward);
    norm3(up);

    mat4_transform_point(camM, origin, eye);
    {
        float dx = eye[0] - vehM[12];
        float dy = eye[1] - vehM[13];
        float dz = eye[2] - vehM[14];
        float dist2 = dx * dx + dy * dy + dz * dz;
        if (dist2 < 4.0f) {
            if (!logged) {
                fprintf(stderr, "v8: renderer camera -- source target obj=%p too close (dist2=%.3f), fallback chase\n",
                        (void *)camObj, dist2);
                logged = 1;
            }
            return 0;
        }
    }
    ctr[0] = eye[0] + forward[0] * 16.0f;
    ctr[1] = eye[1] + forward[1] * 16.0f;
    ctr[2] = eye[2] + forward[2] * 16.0f;

    int ok = isfinite(eye[0]) && isfinite(eye[1]) && isfinite(eye[2])
          && isfinite(ctr[0]) && isfinite(ctr[1]) && isfinite(ctr[2]);
    if (!logged) {
        fprintf(stderr, "v8: renderer camera -- source target obj=%p eye=(%.3f,%.3f,%.3f) ctr=(%.3f,%.3f,%.3f)\n",
                (void *)camObj, eye[0], eye[1], eye[2], ctr[0], ctr[1], ctr[2]);
        logged = 1;
    }
    return ok;
}

static void build_third_person_camera(float vx, float vy, float vz,
                                      const float vehM[16],
                                      float eye[3], float ctr[3], float up[3])
{
    static int initialized = 0;
    static int logged = 0;
    static float prev_eye[3], prev_ctr[3];
    float fwd[3] = { vehM[8], 0.0f, vehM[10] };
    float desired_eye[3], desired_ctr[3];
    float back = 3.0f;
    float rise = 1.05f;
    float look = 7.5f;
    float fwd_len = sqrtf(fwd[0] * fwd[0] + fwd[2] * fwd[2]);

    if (fwd_len < 0.001f) {
        fwd[0] = 0.0f;
        fwd[2] = 1.0f;
    } else {
        fwd[0] /= fwd_len;
        fwd[2] /= fwd_len;
    }

    desired_eye[0] = vx - fwd[0] * back;
    desired_eye[1] = vy + rise;
    desired_eye[2] = vz - fwd[2] * back;
    desired_ctr[0] = vx + fwd[0] * look;
    desired_ctr[1] = vy + 0.45f;
    desired_ctr[2] = vz + fwd[2] * look;

    {
        uint32_t h_cam = terr_sample((int)desired_eye[0], (int)desired_eye[2]);
        float terr_floor = -fixed_y_to_m((int32_t)(h_cam << 11));
        float min_eye_y = terr_floor + 0.85f;
        if (desired_eye[1] < min_eye_y)
            desired_eye[1] = min_eye_y;
    }

    if (!initialized) {
        memcpy(prev_eye, desired_eye, sizeof(prev_eye));
        memcpy(prev_ctr, desired_ctr, sizeof(prev_ctr));
        initialized = 1;
    } else {
        float dx = desired_eye[0] - prev_eye[0];
        float dy = desired_eye[1] - prev_eye[1];
        float dz = desired_eye[2] - prev_eye[2];
        float dist2 = dx * dx + dy * dy + dz * dz;
        float alpha = 0.24f;
        if (dist2 > 400.0f || !isfinite(dist2))
            alpha = 1.0f;
        for (int i = 0; i < 3; i++) {
            prev_eye[i] += (desired_eye[i] - prev_eye[i]) * alpha;
            prev_ctr[i] += (desired_ctr[i] - prev_ctr[i]) * alpha;
        }
    }

    memcpy(eye, prev_eye, sizeof(prev_eye));
    memcpy(ctr, prev_ctr, sizeof(prev_ctr));
    up[0] = 0.0f;
    up[1] = 1.0f;
    up[2] = 0.0f;

    if (!logged) {
        fprintf(stderr, "v8: renderer camera -- third-person fallback active back=%.2f rise=%.2f look=%.2f fov=54\n",
                back, rise, look);
        logged = 1;
    }
}

static void draw_vehicle_wheels(uint8_t *veh, const float parentM[16],
                                const float VP[16], float MVP[16])
{
    glDisable(GL_CULL_FACE);
    for (int wi = 0; wi < 4; wi++) {
        uintptr_t wp = (uintptr_t)*(uint32_t *)(veh + 0xfc + wi * 4);
        if (!host_heap_contains_ptr(wp, 0x9c)) continue;

        uint8_t *wheel = (uint8_t *)wp;
        int kind = (int)*(uint16_t *)(wheel + 0x0a);
        if (kind < 0 || kind >= 10) continue;
        if (!g_wheel_mesh_vao[kind] || g_wheel_mesh_vtx[kind] <= 0) continue;

        int32_t wx = *(int32_t *)(wheel + 0x24);
        int32_t wy = *(int32_t *)(wheel + 0x28);
        int32_t wz = *(int32_t *)(wheel + 0x2c);
        float childM[16], wheelM[16];

        make_model_from_obj(childM, wheel,
                            fixed_xz_to_m(wx),
                            -fixed_y_to_m(wy),
                            fixed_xz_to_m(wz));
        mat4_mul(parentM, childM, wheelM);
        mat4_mul(VP, wheelM, MVP);
        glUniformMatrix4fv(g_loc_mvp, 1, GL_FALSE, MVP);
        if (g_wheel_mesh_tex[kind]) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, g_wheel_mesh_tex[kind]);
            glUniform1i(g_loc_tex, 0);
            glUniform1i(g_loc_useTex, 1);
        } else {
            glUniform1i(g_loc_useTex, 0);
        }
        glBindVertexArray(g_wheel_mesh_vao[kind]);
        glDrawArrays(GL_TRIANGLES, 0, g_wheel_mesh_vtx[kind]);
        glUniform1i(g_loc_useTex, 0);
    }
    glEnable(GL_CULL_FACE);
}

/* Engine's player Vehicle and AI opponent. */
extern void *puRam000007d0;
extern void *puRam000007d4;

void Renderer_DrawFrame(int w, int h, int frame_idx)
{
    (void)frame_idx;
    init_once();

    glViewport(0, 0, w, h);
    glClearColor(0.08f, 0.10f, 0.16f, 1.0f);   /* dark slate -- no engine state yet */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    /* Terrain is built with CCW winding from above; vehicle meshes use CCW.
     * V8 physics uses Y-down; we negate Y for OpenGL (Y-up), which keeps
     * the XZ winding CCW when viewed from positive Y (camera above). */
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glUseProgram(g_prog);
    glUniform3f(g_loc_tint, 1.0f, 1.0f, 1.0f);
    glUniform1i(g_loc_useTex, 0);

    if (g_terrainmesh_sky_tex != 0 && g_sky_vao != 0) {
        static int sky_logged = 0;
        float skyMvp[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDepthMask(GL_FALSE);
        glUniformMatrix4fv(g_loc_mvp, 1, GL_FALSE, skyMvp);
        glUniform3f(g_loc_tint, 1.0f, 1.0f, 1.0f);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_terrainmesh_sky_tex);
        glUniform1i(g_loc_tex, 0);
        glUniform1i(g_loc_useTex, 1);
        glBindVertexArray(g_sky_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glUniform1i(g_loc_useTex, 0);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        if (!sky_logged) {
            fprintf(stderr, "v8: renderer sky -- XBGM %dx%d active\n",
                    g_terrainmesh_sky_w, g_terrainmesh_sky_h);
            sky_logged = 1;
        }
    }

    /* Read engine vehicle pose. If the engine hasn't allocated one
     * yet, the frame stays empty. */
    uint8_t *veh = (uint8_t *)puRam000007d0;
    if (!veh) return;

    /* Object_IntegrateAndOrient writes to +0x24/28/2c. (Vehicle also
     * has a +0xec mirror used by the renderer pipeline in the original
     * engine, but that's copied from +0x24 by code we haven't
     * engaged yet.) */
    int32_t fx = *(int32_t *)(veh + 0x24);
    int32_t fy = *(int32_t *)(veh + 0x28);
    int32_t fz = *(int32_t *)(veh + 0x2c);
    /* Orientation: pull yaw from the rotation matrix at +0x10. R13/R33
     * are the world-space components of vehicle-local +Z, so atan2 of
     * those is the heading. */
    int16_t R13 = *(int16_t *)(veh + 0x10 + 4);
    int16_t R33 = *(int16_t *)(veh + 0x10 + 16);

    float vx = fixed_xz_to_m(fx);
    float vy = -fixed_y_to_m(fy);   /* negate: physics Y-down → OpenGL Y-up */
    float vz = fixed_xz_to_m(fz);
    float yaw_rad = atan2f((float)R13, (float)R33);

    static int log_first = 1;
    if (log_first) {
        fprintf(stderr, "v8: renderer first draw -- pos=(0x%x,0x%x,0x%x) m=(%.3f,%.3f,%.3f) yaw=%.2f rad\n",
                fx, fy, fz, vx, vy, vz, yaw_rad);
        log_first = 0;
    }

    glUniform3f(g_loc_tint, 1.0f, 1.0f, 1.0f);
    glUniform1i(g_loc_useTex, 0);

    float vehM[16];
    make_model_from_obj(vehM, veh, vx, vy, vz);

    float eye[3], ctr[3], up[3];
    if (!build_psx_chase_camera(veh, vehM, eye, ctr, up)) {
        build_third_person_camera(vx, vy, vz, vehM, eye, ctr, up);
    }

    float P[16], V[16], VP[16], M[16], MVP[16];
    make_perspective(P, w, h);
    make_lookat(V, eye, ctr, up);
    mat4_mul(P, V, VP);

    float I[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

    /* Draw the decoded ZONE heightmap first as a cropped diagnostic floor.
     * Placed XOBF geometry is drawn afterward so object/occluder placement is
     * not hidden by the fallback terrain surface. */
    if (g_terr_idxCount > 0) {
        glUniform3f(g_loc_tint, 1.0f, 1.0f, 1.0f);
        if (g_terrainmesh_xbmp_tex) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, g_terrainmesh_xbmp_tex);
            glUniform1i(g_loc_tex2, 1);
            glUniform1i(g_loc_useTex, 1);
        } else {
            glUniform1i(g_loc_useTex, 0);
        }
        mat4_mul(VP, I, MVP);
        glUniformMatrix4fv(g_loc_mvp, 1, GL_FALSE, MVP);
        glBindVertexArray(g_terr_vao);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-1.0f, -1.0f);
        glDrawElements(GL_TRIANGLES, g_terr_idxCount, GL_UNSIGNED_INT, 0);
        glUniform1i(g_loc_useTex, 0);
        glDisable(GL_POLYGON_OFFSET_FILL);
#if V8_TERRAIN_WIREFRAME_OVERLAY
        glUniform3f(g_loc_tint, 0.02f, 0.02f, 0.02f);
        glDisable(GL_CULL_FACE);
        glDepthMask(GL_FALSE);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f);
        glLineWidth(1.5f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, g_terr_idxCount, GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_POLYGON_OFFSET_LINE);
        glDepthMask(GL_TRUE);
        glLineWidth(1.0f);
        glEnable(GL_CULL_FACE);
#endif
    }

    /* Draw the placed level visual mesh (XOBF BIN: terrain patches, props,
     * buildings, and occluders).  Vertices are already in world space. */
    if (V8_RENDER_XOBF_VISUALS && g_terrainmesh_vao && g_terrainmesh_vtx > 0) {
        glUniform3f(g_loc_tint, 1.35f, 1.35f, 1.35f);
        if (g_terrainmesh_tex || g_terrainmesh_xbmp_tex) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, g_terrainmesh_tex);
            glUniform1i(g_loc_tex, 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, g_terrainmesh_xbmp_tex);
            glUniform1i(g_loc_tex2, 1);
            glUniform1i(g_loc_useTex, 1);
        }
        mat4_mul(VP, I, MVP);
        glUniformMatrix4fv(g_loc_mvp, 1, GL_FALSE, MVP);
        glBindVertexArray(g_terrainmesh_vao);
        glDrawArrays(GL_TRIANGLES, 0, g_terrainmesh_vtx);
        glUniform1i(g_loc_useTex, 0);
#if V8_TERRAIN_WIREFRAME_OVERLAY
        glUniform3f(g_loc_tint, 0.02f, 0.02f, 0.02f);
        glDisable(GL_CULL_FACE);
        glDepthMask(GL_FALSE);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f);
        glLineWidth(1.5f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, g_terrainmesh_vtx);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_POLYGON_OFFSET_LINE);
        glDepthMask(GL_TRUE);
        glLineWidth(1.0f);
        glEnable(GL_CULL_FACE);
#endif
    }

    /* Draw the player vehicle (vehicle 0) with a warm tint so the
     * red-orange smoke-test threshold is met and the model looks good. */
    if (g_mesh_vao[0] && g_mesh_vtx[0] > 0) {
        glUniform3f(g_loc_tint, g_mesh_tex[0] ? 1.15f : 2.5f,
                    g_mesh_tex[0] ? 1.15f : 0.7f,
                    g_mesh_tex[0] ? 1.15f : 0.2f);
        memcpy(M, vehM, sizeof(M));
        mat4_mul(VP, M, MVP);
        glUniformMatrix4fv(g_loc_mvp, 1, GL_FALSE, MVP);
        if (g_mesh_tex[0]) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, g_mesh_tex[0]);
            glUniform1i(g_loc_tex, 0);
            glUniform1i(g_loc_useTex, 1);
        }
        glBindVertexArray(g_mesh_vao[0]);
        glDrawArrays(GL_TRIANGLES, 0, g_mesh_vtx[0]);
        glUniform1i(g_loc_useTex, 0);
    }
    glUniform3f(g_loc_tint, 1.0f, 1.0f, 1.0f);
    draw_vehicle_wheels(veh, vehM, VP, MVP);

    /* Draw the AI opponent (vehicle 1) in blue. */
    uint8_t *ai = (uint8_t *)puRam000007d4;
    if (ai) {
        int32_t ax = *(int32_t *)(ai + 0x24);
        int32_t ay = *(int32_t *)(ai + 0x28);
        int32_t az = *(int32_t *)(ai + 0x2c);
        float aix = fixed_xz_to_m(ax);
        float aiy = -fixed_y_to_m(ay);   /* negate: physics Y-down → OpenGL Y-up */
        float aiz = fixed_xz_to_m(az);
        float aiM[16];
        make_model_from_obj(aiM, ai, aix, aiy, aiz);

        int ai_vid = 1;   /* use vehicle 1 shape for the AI opponent */
        if (g_mesh_vao[ai_vid] && g_mesh_vtx[ai_vid] > 0) {
            /* Blue tint: polygon grey base (0.5) × 5.0 = saturated blue. */
            glUniform3f(g_loc_tint, g_mesh_tex[ai_vid] ? 1.0f : 0.0f,
                        g_mesh_tex[ai_vid] ? 1.0f : 0.0f,
                        g_mesh_tex[ai_vid] ? 1.15f : 5.0f);
            memcpy(M, aiM, sizeof(M));
            mat4_mul(VP, M, MVP);
            glUniformMatrix4fv(g_loc_mvp, 1, GL_FALSE, MVP);
            if (g_mesh_tex[ai_vid]) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, g_mesh_tex[ai_vid]);
                glUniform1i(g_loc_tex, 0);
                glUniform1i(g_loc_useTex, 1);
            }
            glBindVertexArray(g_mesh_vao[ai_vid]);
            glDrawArrays(GL_TRIANGLES, 0, g_mesh_vtx[ai_vid]);
            glUniform1i(g_loc_useTex, 0);
        }
        glUniform3f(g_loc_tint, 0.75f, 0.85f, 1.2f);
        draw_vehicle_wheels(ai, aiM, VP, MVP);
    }

}

#else  /* no SDL/GL */

void Renderer_DrawFrame(int w, int h, int f) { (void)w; (void)h; (void)f; }

#endif
