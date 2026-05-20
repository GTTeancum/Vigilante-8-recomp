/* screenshot.c -- write the GL backbuffer to a portable image file.
 *
 * Phase 3b: writes PPM (binary P6) so we don't need a PNG dependency.
 * The smoke-test predicate (tools/smoke/check_screenshot.py) reads
 * the raw RGB pixels and asserts non-uniform color.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern int Platform_ReadBackbuffer(uint8_t *rgba, int *w, int *h);
extern int Platform_Width(void);
extern int Platform_Height(void);

int Screenshot_Save(const char *path)
{
    int w = Platform_Width();
    int h = Platform_Height();
    if (w <= 0 || h <= 0) {
        fprintf(stderr, "v8: screenshot -- no platform window\n");
        return -1;
    }

    size_t   nrgba = (size_t)w * h * 4;
    uint8_t *rgba  = (uint8_t *)malloc(nrgba);
    if (!rgba) return -1;

    if (Platform_ReadBackbuffer(rgba, &w, &h) != 0) {
        fprintf(stderr, "v8: screenshot -- ReadBackbuffer failed\n");
        free(rgba);
        return -1;
    }

    /* PPM is bottom-up vs top-down: GL gives us bottom-up, ppm wants
     * top-down. Flip vertically. */
    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "v8: screenshot -- cannot open %s\n", path);
        free(rgba);
        return -1;
    }
    fprintf(f, "P6\n%d %d\n255\n", w, h);
    uint8_t *row = (uint8_t *)malloc((size_t)w * 3);
    for (int y = h - 1; y >= 0; y--) {
        const uint8_t *src = rgba + (size_t)y * w * 4;
        for (int x = 0; x < w; x++) {
            row[x*3+0] = src[x*4+0];
            row[x*3+1] = src[x*4+1];
            row[x*3+2] = src[x*4+2];
        }
        fwrite(row, 1, (size_t)w * 3, f);
    }
    free(row);
    fclose(f);
    free(rgba);

    fprintf(stderr, "v8: screenshot saved -> %s (%dx%d PPM)\n", path, w, h);
    return 0;
}
