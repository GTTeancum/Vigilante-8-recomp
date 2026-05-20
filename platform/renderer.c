/* renderer.c -- OpenGL 3.3 terrain mesh + minimal vehicle box renderer.
 *
 * Phase 3d: synthesize a procedural heightmap so the GL pipeline gets
 * exercised end-to-end. Phase 3c+ will swap this for the real .EXP
 * parsed heightmap.
 *
 * Pipeline:
 *   - One VAO/VBO/IBO for terrain (triangle list, vertex-colored).
 *   - One VAO/VBO for the player vehicle box (cube).
 *   - One shader: per-vertex color passthrough, MVP from uniforms.
 *   - Camera: fixed orbit looking down at the terrain.
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
static GLuint g_terr_vao   = 0, g_terr_vbo = 0, g_terr_ibo = 0;
static int    g_terr_idxCount = 0;
static GLuint g_box_vao    = 0, g_box_vbo = 0, g_box_ibo = 0;
static int    g_box_idxCount = 0;
static int    g_initialized = 0;

#define TERR_GRID 32  /* sample grid */
#define TERR_EXTENT 20.0f
#define TERR_HEIGHT 3.0f

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
    "out vec4 oCol;\n"
    "void main(){ oCol = vec4(vCol, 1.0); }\n";

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

/* Procedural heightmap: smooth bumps + a "vehicle pad" flat zone. */
static float heightAt(int xi, int zi) {
    float x = ((float)xi / (TERR_GRID - 1)) * 2.0f - 1.0f;   /* [-1,+1] */
    float z = ((float)zi / (TERR_GRID - 1)) * 2.0f - 1.0f;
    /* Two sin lobes. */
    float h = 0.5f * sinf(2.5f * x) * cosf(2.0f * z)
            + 0.3f * sinf(5.0f * x + z) * cosf(3.0f * z);
    return h * TERR_HEIGHT;
}

static void colorAt(float y, float *r, float *g, float *b) {
    /* low = brown, mid = green, high = grey */
    float t = (y / TERR_HEIGHT + 0.6f) * 0.5f;  /* normalize roughly to [0,1] */
    if (t < 0.0f) t = 0.0f;  if (t > 1.0f) t = 1.0f;
    if (t < 0.4f) { *r = 0.40f; *g = 0.30f; *b = 0.20f; }     /* dirt */
    else if (t < 0.75f) {
        float u = (t - 0.4f) / 0.35f;
        *r = 0.40f - 0.20f * u;
        *g = 0.30f + 0.30f * u;
        *b = 0.20f - 0.10f * u;
    }
    else { *r = 0.55f; *g = 0.55f; *b = 0.50f; }              /* rock */
}

static void build_terrain_mesh(void) {
    int N = TERR_GRID;
    int nverts = N * N;
    int ntris  = (N - 1) * (N - 1) * 2;
    int nidx   = ntris * 3;

    float *verts = (float *)malloc(sizeof(float) * 6 * nverts); /* xyz + rgb */
    uint32_t *idx = (uint32_t *)malloc(sizeof(uint32_t) * nidx);

    for (int z = 0; z < N; z++) {
        for (int x = 0; x < N; x++) {
            float fx = ((float)x / (N - 1) - 0.5f) * 2.0f * TERR_EXTENT;
            float fz = ((float)z / (N - 1) - 0.5f) * 2.0f * TERR_EXTENT;
            float fy = heightAt(x, z);
            int   k  = (z * N + x) * 6;
            verts[k+0] = fx; verts[k+1] = fy; verts[k+2] = fz;
            colorAt(fy, &verts[k+3], &verts[k+4], &verts[k+5]);
        }
    }
    int j = 0;
    for (int z = 0; z < N - 1; z++) {
        for (int x = 0; x < N - 1; x++) {
            uint32_t a = z*N + x;
            uint32_t b = z*N + x + 1;
            uint32_t c = (z+1)*N + x;
            uint32_t d = (z+1)*N + x + 1;
            idx[j++] = a; idx[j++] = c; idx[j++] = b;
            idx[j++] = b; idx[j++] = c; idx[j++] = d;
        }
    }
    g_terr_idxCount = nidx;

    glGenVertexArrays(1, &g_terr_vao); glBindVertexArray(g_terr_vao);
    glGenBuffers(1, &g_terr_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_terr_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * nverts, verts, GL_STATIC_DRAW);
    glGenBuffers(1, &g_terr_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_terr_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * nidx, idx, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, (void *)(sizeof(float)*3));

    free(verts); free(idx);
}

static void build_vehicle_box(void) {
    /* Unit cube, red, indexed. */
    float v[] = {
        -0.5f, 0.0f, -0.5f,  1.0f, 0.2f, 0.2f,
         0.5f, 0.0f, -0.5f,  1.0f, 0.2f, 0.2f,
         0.5f, 1.0f, -0.5f,  1.0f, 0.5f, 0.2f,
        -0.5f, 1.0f, -0.5f,  1.0f, 0.5f, 0.2f,
        -0.5f, 0.0f,  0.5f,  1.0f, 0.2f, 0.2f,
         0.5f, 0.0f,  0.5f,  1.0f, 0.2f, 0.2f,
         0.5f, 1.0f,  0.5f,  1.0f, 0.5f, 0.2f,
        -0.5f, 1.0f,  0.5f,  1.0f, 0.5f, 0.2f,
    };
    uint32_t i[] = {
        0,1,2, 0,2,3,   /* back */
        4,6,5, 4,7,6,   /* front */
        0,4,5, 0,5,1,   /* bottom */
        3,2,6, 3,6,7,   /* top */
        0,3,7, 0,7,4,   /* left */
        1,5,6, 1,6,2,   /* right */
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
    g_loc_mvp = glGetUniformLocation(g_prog, "uMVP");
    glDeleteShader(vs); glDeleteShader(fs);

    build_terrain_mesh();
    build_vehicle_box();

    g_initialized = 1;
    fprintf(stderr, "v8: renderer ready (terrain=%d tris, vehicle box)\n",
            g_terr_idxCount/3);
}

/* Vehicle pose from pad_shim.c (read by the chase camera + box draw). */
extern float g_veh_x, g_veh_y, g_veh_z, g_veh_yaw, g_veh_speed;

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
    float zn = 0.1f, zf = 500.0f;
    float f = 1.0f / tanf(fov * 0.5f);
    memset(P, 0, sizeof(float) * 16);
    P[0]  = f/aspect;
    P[5]  = f;
    P[10] = (zf+zn)/(zn-zf);
    P[11] = -1.0f;
    P[14] = (2*zf*zn)/(zn-zf);
}

static void make_lookat(float V[16], const float eye[3], const float ctr[3], const float up[3]) {
    float fwd[3] = { ctr[0]-eye[0], ctr[1]-eye[1], ctr[2]-eye[2] };
    norm3(fwd);
    float side[3]; cross3(fwd, up, side); norm3(side);
    float u2[3];   cross3(side, fwd, u2);

    V[0] = side[0]; V[4] = side[1]; V[ 8] = side[2]; V[12] = -dot3(side, eye);
    V[1] = u2[0];   V[5] = u2[1];   V[ 9] = u2[2];   V[13] = -dot3(u2, eye);
    V[2] = -fwd[0]; V[6] = -fwd[1]; V[10] = -fwd[2]; V[14] =  dot3(fwd, eye);
    V[3] = 0;       V[7] = 0;       V[11] = 0;       V[15] = 1;
}

/* Camera VP only (no model). out is column-major. */
static void make_chase_camera(float out[16], int w, int h) {
    float chase_back = 10.0f, chase_up = 5.0f;
    float eye[3] = {
        g_veh_x - sinf(g_veh_yaw) * chase_back,
        g_veh_y + chase_up,
        g_veh_z - cosf(g_veh_yaw) * chase_back
    };
    float ctr[3] = { g_veh_x, g_veh_y + 1.0f, g_veh_z };
    float up[3]  = { 0, 1, 0 };

    float P[16], V[16];
    make_perspective(P, w, h);
    make_lookat(V, eye, ctr, up);
    mat4_mul(P, V, out);
}

/* Right-multiply with a model matrix (Y-rotation + translation). */
static void make_model_yt(float M[16], float yaw, float tx, float ty, float tz) {
    float c = cosf(yaw), s = sinf(yaw);
    M[0] = c;  M[4] = 0;  M[ 8] = s;  M[12] = tx;
    M[1] = 0;  M[5] = 1;  M[ 9] = 0;  M[13] = ty;
    M[2] = -s; M[6] = 0;  M[10] = c;  M[14] = tz;
    M[3] = 0;  M[7] = 0;  M[11] = 0;  M[15] = 1;
}

void Renderer_DrawFrame(int w, int h, int frame_idx)
{
    (void)frame_idx;
    init_once();

    glViewport(0, 0, w, h);
    glClearColor(0.45f, 0.62f, 0.85f, 1.0f);   /* sky */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(g_prog);

    float VP[16];
    make_chase_camera(VP, w, h);

    /* Terrain at world origin -- identity model. MVP = VP * I = VP. */
    glUniformMatrix4fv(g_loc_mvp, 1, GL_FALSE, VP);
    glBindVertexArray(g_terr_vao);
    glDrawElements(GL_TRIANGLES, g_terr_idxCount, GL_UNSIGNED_INT, 0);

    /* Vehicle box at (g_veh_x, y_ground, g_veh_z), rotated by g_veh_yaw. */
    float M[16], MVP[16];
    make_model_yt(M, g_veh_yaw, g_veh_x, g_veh_y, g_veh_z);
    mat4_mul(VP, M, MVP);
    glUniformMatrix4fv(g_loc_mvp, 1, GL_FALSE, MVP);
    glBindVertexArray(g_box_vao);
    glDrawElements(GL_TRIANGLES, g_box_idxCount, GL_UNSIGNED_INT, 0);
}

#else  /* no SDL/GL */

void Renderer_DrawFrame(int w, int h, int f) { (void)w; (void)h; (void)f; }

#endif
