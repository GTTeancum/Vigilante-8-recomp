/* renderer.c -- OpenGL 3.3 view of the engine's world state.
 *
 * RESET: the procedural sin/cos terrain and host-side gameplay actors
 * are gone. The renderer reads from the engine's Vehicle struct
 * (puRam000007d0) and the engine's terrain object tree. Until those
 * are populated by a real level-load, the frame is mostly empty.
 *
 * Vehicle pose is in 17.15 fixed-point world units (4 units of fixed
 * per nominal "metre"); yaw is 4.12 fixed-point with 4096 == 2*pi.
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
static GLuint g_box_vao    = 0, g_box_vbo = 0, g_box_ibo = 0;
static int    g_box_idxCount = 0;
static GLuint g_terr_vao   = 0, g_terr_vbo = 0, g_terr_ibo = 0;
static int    g_terr_idxCount = 0;
static int    g_initialized = 0;

/* Engine terrain table + populated-tile bounds, supplied by host_terrain.c. */
extern uintptr_t DAT_800911a0[32 * 32];
extern int       g_terrain_loaded;
extern uint8_t   g_terrain_tile_x_min, g_terrain_tile_x_max;
extern uint8_t   g_terrain_tile_z_min, g_terrain_tile_z_max;

static const char *VS_SRC =
    "#version 330 core\n"
    "layout(location=0) in vec3 aPos;\n"
    "layout(location=1) in vec3 aCol;\n"
    "uniform mat4 uMVP;\n"
    "out vec3 vCol;\n"
    "void main(){ vCol = aCol; gl_Position = uMVP * vec4(aPos, 1.0); }\n";

static const char *FS_SRC =
    "#version 330 core\n"
    "in vec3 vCol;\n"
    "uniform vec3 uTint;\n"
    "out vec4 oCol;\n"
    "void main(){ oCol = vec4(vCol * uTint, 1.0); }\n";

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

/* Vehicle box, 4 units long x 2 wide x 1.5 tall. Roughly car-sized
 * relative to a chase camera at 10 units back. */
static void build_vehicle_box(void) {
    const float L = 2.0f;   /* half-length (Z) */
    const float W = 1.0f;   /* half-width  (X) */
    const float H = 1.5f;   /* full height (Y) */
    float v[] = {
        -W, 0,    -L,  1.0f, 0.2f, 0.2f,
         W, 0,    -L,  1.0f, 0.2f, 0.2f,
         W, H,    -L,  1.0f, 0.6f, 0.2f,
        -W, H,    -L,  1.0f, 0.6f, 0.2f,
        -W, 0,     L,  1.0f, 0.2f, 0.2f,
         W, 0,     L,  1.0f, 0.2f, 0.2f,
         W, H,     L,  1.0f, 0.6f, 0.2f,
        -W, H,     L,  1.0f, 0.6f, 0.2f,
    };
    uint32_t i[] = {
        0,1,2, 0,2,3,   4,6,5, 4,7,6,
        0,4,5, 0,5,1,   3,2,6, 3,6,7,
        0,3,7, 0,7,4,   1,5,6, 1,6,2,
    };
    g_box_idxCount = sizeof(i)/sizeof(i[0]);

    glGenVertexArrays(1, &g_box_vao); glBindVertexArray(g_box_vao);
    glGenBuffers(1, &g_box_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_box_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof v, v, GL_STATIC_DRAW);
    glGenBuffers(1, &g_box_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_box_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof i, i, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, (void *)(sizeof(float)*3));
}

/* Convert engine's 17.15 fixed-point world unit -> float metres.
 * V8 uses 4 (17.15) units per nominal metre based on the heightmap
 * spacing seen in src/physics/terrain_height.c. */
static float fixed1715_to_m_s(int32_t v) { return (float)v / (32768.0f * 4.0f); }

/* Build a static triangle mesh of the loaded terrain. The engine's
 * cleaned terrain_sample says:
 *   chunk_base = DAT_800911a0[chunk_x*32 + chunk_z]
 *   h(cell_x,cell_z) = *(u16*)(chunk_base + (cell_x<<7)|(cell_z<<1)) & 0x7ff
 *   world_x_fixed = (chunk_x*64 + cell_x) << 16
 *   world_z_fixed = (chunk_z*64 + cell_z) << 16
 *   world_y_fixed = h * 0x800  (matches Terrain_HeightAt >> 5 scaling) */
static uint32_t terr_sample(int cx_global, int cz_global) {
    int chunk_x = (cx_global >> 6) & 0x1f;
    int chunk_z = (cz_global >> 6) & 0x1f;
    uintptr_t base = DAT_800911a0[chunk_x * 32 + chunk_z];
    if (!base) return 0;
    uint32_t off = ((cx_global & 0x3f) << 7) | ((cz_global & 0x3f) << 1);
    return (uint32_t)(*(uint16_t *)(base + off)) & 0x7ffu;
}

static void build_terrain_mesh(void) {
    if (!g_terrain_loaded) return;

    int tx0 = g_terrain_tile_x_min, tx1 = g_terrain_tile_x_max;
    int tz0 = g_terrain_tile_z_min, tz1 = g_terrain_tile_z_max;
    int verts_x = (tx1 - tx0 + 1) * 64 + 1;
    int verts_z = (tz1 - tz0 + 1) * 64 + 1;
    int n_verts = verts_x * verts_z;
    int n_quads = (verts_x - 1) * (verts_z - 1);

    float *vbuf = (float *)malloc(sizeof(float) * 6 * n_verts);
    uint32_t *ibuf = (uint32_t *)malloc(sizeof(uint32_t) * 6 * n_quads);
    if (!vbuf || !ibuf) { free(vbuf); free(ibuf); return; }

    for (int gz = 0; gz < verts_z; gz++) {
        for (int gx = 0; gx < verts_x; gx++) {
            int cx_global = tx0 * 64 + gx;
            int cz_global = tz0 * 64 + gz;
            uint32_t h = terr_sample(cx_global, cz_global);

            int32_t fx = cx_global << 16;
            int32_t fz = cz_global << 16;
            int32_t fy = (int32_t)(h << 11);   /* h * 0x800 */

            float *p = vbuf + (gz * verts_x + gx) * 6;
            p[0] = fixed1715_to_m_s(fx);
            p[1] = fixed1715_to_m_s(fy);
            p[2] = fixed1715_to_m_s(fz);
            /* Colour: stripe by height + grid lines per cell to make
             * it obvious whether geometry actually loaded. */
            float t = (float)h / 2047.0f;
            float grid = ((cx_global & 1) ^ (cz_global & 1)) ? 1.0f : 0.85f;
            p[3] = (0.25f + 0.45f * t) * grid;
            p[4] = (0.45f + 0.40f * t) * grid;
            p[5] = (0.25f + 0.20f * (1.0f - t)) * grid;
        }
    }

    uint32_t *ip = ibuf;
    for (int gz = 0; gz < verts_z - 1; gz++) {
        for (int gx = 0; gx < verts_x - 1; gx++) {
            uint32_t a = gz * verts_x + gx;
            uint32_t b = a + 1;
            uint32_t c = a + verts_x;
            uint32_t d = c + 1;
            *ip++ = a; *ip++ = c; *ip++ = b;
            *ip++ = b; *ip++ = c; *ip++ = d;
        }
    }
    g_terr_idxCount = 6 * n_quads;

    glGenVertexArrays(1, &g_terr_vao); glBindVertexArray(g_terr_vao);
    glGenBuffers(1, &g_terr_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_terr_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * n_verts, vbuf, GL_STATIC_DRAW);
    glGenBuffers(1, &g_terr_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_terr_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * g_terr_idxCount, ibuf, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, (void *)(sizeof(float)*3));

    fprintf(stderr, "v8: terrain mesh built -- tiles [%d..%d]x[%d..%d], "
            "%d verts, %d tris\n",
            tx0, tx1, tz0, tz1, n_verts, g_terr_idxCount / 3);

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
    glDeleteShader(vs); glDeleteShader(fs);
    build_vehicle_box();
    build_terrain_mesh();
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
    float fov = 60.0f * 3.1415926f / 180.0f;
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
static void make_model_yt(float M[16], float yaw, float tx, float ty, float tz) {
    float c = cosf(yaw), s = sinf(yaw);
    M[0]=c;  M[4]=0; M[ 8]=s;  M[12]=tx;
    M[1]=0;  M[5]=1; M[ 9]=0;  M[13]=ty;
    M[2]=-s; M[6]=0; M[10]=c;  M[14]=tz;
    M[3]=0;  M[7]=0; M[11]=0;  M[15]=1;
}

/* Engine's player Vehicle (NULL until the engine's loader populates it). */
extern void *puRam000007d0;

static float fixed1715_to_m(int32_t v) { return fixed1715_to_m_s(v); }

void Renderer_DrawFrame(int w, int h, int frame_idx)
{
    (void)frame_idx;
    init_once();

    glViewport(0, 0, w, h);
    glClearColor(0.08f, 0.10f, 0.16f, 1.0f);   /* dark slate -- no engine state yet */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

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

    float vx = fixed1715_to_m(fx);
    float vy = fixed1715_to_m(fy);
    float vz = fixed1715_to_m(fz);
    float yaw_rad = atan2f((float)R13, (float)R33);

    static int log_first = 1;
    if (log_first) {
        fprintf(stderr, "v8: renderer first draw -- pos=(0x%x,0x%x,0x%x) m=(%.3f,%.3f,%.3f) yaw=%.2f rad\n",
                fx, fy, fz, vx, vy, vz, yaw_rad);
        log_first = 0;
    }

    glUseProgram(g_prog);
    glUniform3f(g_loc_tint, 1.0f, 1.0f, 1.0f);

    /* Chase camera. */
    float chase_back = 10.0f, chase_up = 5.0f;
    float eye[3] = {
        vx - sinf(yaw_rad) * chase_back,
        vy + chase_up,
        vz - cosf(yaw_rad) * chase_back
    };
    float ctr[3] = { vx, vy + 1.0f, vz };
    float up[3]  = { 0, 1, 0 };

    float P[16], V[16], VP[16], M[16], MVP[16];
    make_perspective(P, w, h);
    make_lookat(V, eye, ctr, up);
    mat4_mul(P, V, VP);

    /* Draw the terrain (identity model). */
    if (g_terr_idxCount > 0) {
        float I[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
        mat4_mul(VP, I, MVP);
        glUniformMatrix4fv(g_loc_mvp, 1, GL_FALSE, MVP);
        glBindVertexArray(g_terr_vao);
        glDrawElements(GL_TRIANGLES, g_terr_idxCount, GL_UNSIGNED_INT, 0);
    }

    /* Draw the vehicle as a cube at engine-supplied pose. */
    make_model_yt(M, yaw_rad, vx, vy, vz);
    mat4_mul(VP, M, MVP);
    glUniformMatrix4fv(g_loc_mvp, 1, GL_FALSE, MVP);
    glBindVertexArray(g_box_vao);
    glDrawElements(GL_TRIANGLES, g_box_idxCount, GL_UNSIGNED_INT, 0);
}

#else  /* no SDL/GL */

void Renderer_DrawFrame(int w, int h, int f) { (void)w; (void)h; (void)f; }

#endif
