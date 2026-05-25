/* screenshot.c -- write the GL backbuffer to a PNG file.
 *
 * Uses stb_image_write (single-header, MIT) for PNG encoding.
 * Falls back to PPM if the platform window is not available.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* stb_image_write single-file implementation. */
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../third_party/stb_image_write.h"

extern int Platform_ReadBackbuffer(uint8_t *rgba, int *w, int *h);
extern int Platform_Width(void);
extern int Platform_Height(void);

int Screenshot_Save(const char *path)
{
    int w = Platform_Width();
    int h = Platform_Height();
    if (w <= 0 || h <= 0) {
        fprintf(stderr, "v8: screenshot -- no platform window (headless?)\n");
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

    /* GL gives bottom-up rows; PNG wants top-down. Flip. */
    uint8_t *flipped = (uint8_t *)malloc(nrgba);
    if (!flipped) { free(rgba); return -1; }
    for (int y = 0; y < h; y++) {
        memcpy(flipped + (size_t)(h - 1 - y) * w * 4,
               rgba   + (size_t)y            * w * 4,
               (size_t)w * 4);
    }
    free(rgba);

    int ok = stbi_write_png(path, w, h, 4, flipped, w * 4);
    free(flipped);

    if (!ok) {
        fprintf(stderr, "v8: screenshot -- stbi_write_png failed for %s\n", path);
        return -1;
    }

    fprintf(stderr, "v8: screenshot saved -> %s (%dx%d PNG)\n", path, w, h);
    return 0;
}
