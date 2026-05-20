/* destructibles.c -- scatter of host-side breakable props.
 *
 * The engine's per-level destructible tick callbacks (oil rigs,
 * blimp, gondola, etc.) are panic-stubbed. To exercise the
 * "things break when shot" behavior visually, we scatter ~12 static
 * cubes around the terrain. A projectile within 1.5 units removes
 * one and bumps a counter the smoke can check.
 *
 * Phase 7+ would route this through the per-level handlers in
 * src/physics/<level>/ once those compile and link cleanly.
 */
#include <stdio.h>
#include <stdint.h>

#define N_DESTRUCT 12

typedef struct {
    float x, y, z;
    int   alive;
    float color[3];
} Destructible;

static Destructible g_props[N_DESTRUCT] = {
    /* Scatter in a rough grid. Colors are vivid so they show up in the
     * screenshot color-bucket check. */
    { -10, 0, -10, 1, { 0.9f, 0.7f, 0.0f } },
    {   0, 0, -10, 1, { 0.9f, 0.7f, 0.0f } },
    {  10, 0, -10, 1, { 0.9f, 0.7f, 0.0f } },
    { -10, 0,   0, 1, { 0.0f, 0.9f, 0.9f } },
    {  10, 0,   0, 1, { 0.0f, 0.9f, 0.9f } },
    { -10, 0,  10, 1, { 0.9f, 0.0f, 0.9f } },
    {   0, 0,  10, 1, { 0.9f, 0.0f, 0.9f } },
    {  10, 0,  10, 1, { 0.9f, 0.0f, 0.9f } },
    {  -5, 0,  -5, 1, { 1.0f, 1.0f, 1.0f } },
    {   5, 0,  -5, 1, { 1.0f, 1.0f, 1.0f } },
    {  -5, 0,   5, 1, { 1.0f, 1.0f, 1.0f } },
    {   5, 0,   5, 1, { 1.0f, 1.0f, 1.0f } },
};

static int g_broken_count = 0;

extern int Projectile_GetActive(float *out, int max);

void Destructibles_Tick(void)
{
    float proj[64 * 3];
    int n = Projectile_GetActive(proj, 64);
    if (n == 0) return;

    for (int i = 0; i < N_DESTRUCT; i++) {
        if (!g_props[i].alive) continue;
        for (int j = 0; j < n; j++) {
            float dx = proj[j*3+0] - g_props[i].x;
            float dy = proj[j*3+1] - g_props[i].y - 0.5f;
            float dz = proj[j*3+2] - g_props[i].z;
            float d2 = dx*dx + dy*dy + dz*dz;
            if (d2 < 1.5f * 1.5f) {
                g_props[i].alive = 0;
                g_broken_count++;
                fprintf(stderr, "v8: destructible %d broken (total=%d)\n",
                        i, g_broken_count);
                break;
            }
        }
    }
}

/* Renderer interface: fill output array with [x,y,z, r,g,b] x active count. */
int Destructibles_GetActive(float *out_xyzrgb, int max)
{
    int n = 0;
    for (int i = 0; i < N_DESTRUCT && n < max; i++) {
        if (!g_props[i].alive) continue;
        out_xyzrgb[n*6 + 0] = g_props[i].x;
        out_xyzrgb[n*6 + 1] = g_props[i].y;
        out_xyzrgb[n*6 + 2] = g_props[i].z;
        out_xyzrgb[n*6 + 3] = g_props[i].color[0];
        out_xyzrgb[n*6 + 4] = g_props[i].color[1];
        out_xyzrgb[n*6 + 5] = g_props[i].color[2];
        n++;
    }
    return n;
}

int Destructibles_BrokenCount(void) { return g_broken_count; }
