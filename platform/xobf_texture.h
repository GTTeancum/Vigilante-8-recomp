#pragma once

#include <stdint.h>

#if defined(V8_HAVE_SDL) && defined(V8_HAVE_GL)
#include <GL/gl3w.h>

#define V8_XOBF_TEX_MAX_SLOTS 256

typedef struct {
    int valid;
    int x, y, w, h;
} V8XobfTexSlot;

typedef struct {
    GLuint tex;
    int w, h;
    int slots;
    V8XobfTexSlot slot[V8_XOBF_TEX_MAX_SLOTS];
} V8XobfTexAtlas;

void V8_XobfTexAtlas_Reset(V8XobfTexAtlas *atlas);
int  V8_XobfTexAtlas_BuildFromBin(V8XobfTexAtlas *atlas,
                                  const uint8_t *bin, uint32_t binSize);
int  V8_XobfTex_DecodePacketUv(const V8XobfTexAtlas *atlas,
                               const uint8_t *pkt, int nib, int texBase,
                               float uv[4][2]);

#endif
