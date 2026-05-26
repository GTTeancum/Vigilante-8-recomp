#include "xobf_texture.h"

#if defined(V8_HAVE_SDL) && defined(V8_HAVE_GL)

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    int valid;
    int w, h;
    uint8_t *rgba;
} DecodedSlot;

static uint32_t rd32le(const uint8_t *b, uint32_t o)
{
    return (uint32_t)b[o] | ((uint32_t)b[o + 1] << 8) |
           ((uint32_t)b[o + 2] << 16) | ((uint32_t)b[o + 3] << 24);
}

static uint16_t rd16le(const uint8_t *b, uint32_t o)
{
    return (uint16_t)b[o] | ((uint16_t)b[o + 1] << 8);
}

static int16_t rds16le(const uint8_t *b, uint32_t o)
{
    return (int16_t)rd16le(b, o);
}

static void psx555_rgba(uint16_t c, int blackCutout, uint8_t out[4])
{
    uint8_t r = (uint8_t)(c & 0x1f);
    uint8_t g = (uint8_t)((c >> 5) & 0x1f);
    uint8_t b = (uint8_t)((c >> 10) & 0x1f);
    out[0] = (uint8_t)((r << 3) | (r >> 2));
    out[1] = (uint8_t)((g << 3) | (g >> 2));
    out[2] = (uint8_t)((b << 3) | (b >> 2));
    out[3] = (c == 0 || (blackCutout && (c & 0x7fffu) == 0)) ? 0 : 255;
}

static int pixel_width_from_words(int words, int depth)
{
    if (depth == 0) return words * 4;
    if (depth == 1) return words * 2;
    return words;
}

static int decode_slot(const uint8_t *bin, uint32_t binSize,
                       uint32_t off, int blackCutout, DecodedSlot *out)
{
    if (off + 0x20 > binSize)
        return 0;

    uint32_t flags = rd32le(bin, off + 4);
    uint32_t imageOff = rd32le(bin, off + 8);
    int depth = (int)(flags & 3u);
    if (depth != 0 && depth != 1)
        return 0;
    if (off + imageOff + 0x14 > binSize)
        return 0;

    int words = rds16le(bin, off + imageOff + 0x10);
    int h = rds16le(bin, off + imageOff + 0x12);
    int w = pixel_width_from_words(words, depth);
    if (w <= 0 || h <= 0 || w > 512 || h > 512)
        return 0;

    uint32_t pixOff = off + imageOff + 0x14;
    uint32_t pixBytes = (depth == 0) ? (uint32_t)((w * h + 1) / 2)
                                     : (uint32_t)(w * h);
    if (pixOff + pixBytes > binSize)
        return 0;

    uint8_t palette[256][4];
    int palCount = (depth == 0) ? 16 : 256;
    if (off + 0x14 + (uint32_t)palCount * 2u > binSize)
        return 0;
    for (int i = 0; i < palCount; i++)
        psx555_rgba(rd16le(bin, off + 0x14 + (uint32_t)i * 2u),
                    blackCutout, palette[i]);

    uint8_t *rgba = (uint8_t *)calloc((size_t)w * (size_t)h, 4u);
    if (rgba == NULL)
        return 0;

    const uint8_t *pix = bin + pixOff;
    if (depth == 0) {
        for (int i = 0; i < w * h; i++) {
            uint8_t packed = pix[i >> 1];
            uint8_t idx = (uint8_t)((i & 1) ? (packed >> 4) : (packed & 0x0f));
            memcpy(rgba + i * 4, palette[idx], 4);
        }
    } else {
        for (int i = 0; i < w * h; i++)
            memcpy(rgba + i * 4, palette[pix[i]], 4);
    }

    out->valid = 1;
    out->w = w;
    out->h = h;
    out->rgba = rgba;
    return 1;
}

void V8_XobfTexAtlas_Reset(V8XobfTexAtlas *atlas)
{
    if (atlas == NULL)
        return;
    if (atlas->tex != 0)
        glDeleteTextures(1, &atlas->tex);
    memset(atlas, 0, sizeof(*atlas));
}

int V8_XobfTexAtlas_BuildFromBin(V8XobfTexAtlas *atlas,
                                 const uint8_t *bin, uint32_t binSize)
{
    return V8_XobfTexAtlas_BuildFromBinEx(atlas, bin, binSize, 0);
}

int V8_XobfTexAtlas_BuildFromBinEx(V8XobfTexAtlas *atlas,
                                   const uint8_t *bin, uint32_t binSize,
                                   int blackCutout)
{
    if (atlas == NULL || bin == NULL || binSize < 0x18)
        return 0;
    V8_XobfTexAtlas_Reset(atlas);

    uint32_t slotCount = rd32le(bin, 0x10);
    uint32_t slotTable = rd32le(bin, 0x14);
    if (slotCount == 0 || slotCount > V8_XOBF_TEX_MAX_SLOTS ||
        slotTable + slotCount * 4u > binSize)
        return 0;

    DecodedSlot decoded[V8_XOBF_TEX_MAX_SLOTS];
    memset(decoded, 0, sizeof(decoded));

    int valid = 0;
    int rowW = 0, rowH = 0, atlasW = 0, atlasH = 0;
    const int maxRowW = 2048;
    for (uint32_t i = 0; i < slotCount; i++) {
        uint32_t rel = rd32le(bin, slotTable + i * 4u);
        uint32_t off = slotTable + rel;
        if (!decode_slot(bin, binSize, off, blackCutout, &decoded[i]))
            continue;
        if (rowW != 0 && rowW + decoded[i].w > maxRowW) {
            if (rowW > atlasW) atlasW = rowW;
            atlasH += rowH;
            rowW = 0;
            rowH = 0;
        }
        atlas->slot[i].valid = 1;
        atlas->slot[i].x = rowW;
        atlas->slot[i].y = atlasH;
        atlas->slot[i].w = decoded[i].w;
        atlas->slot[i].h = decoded[i].h;
        rowW += decoded[i].w;
        if (decoded[i].h > rowH) rowH = decoded[i].h;
        valid++;
    }
    if (valid == 0)
        return 0;
    if (rowW > atlasW) atlasW = rowW;
    atlasH += rowH;
    if (atlasW <= 0 || atlasH <= 0)
        return 0;

    uint8_t *rgba = (uint8_t *)calloc((size_t)atlasW * (size_t)atlasH, 4u);
    if (rgba == NULL)
        return 0;

    for (uint32_t i = 0; i < slotCount; i++) {
        if (!decoded[i].valid)
            continue;
        V8XobfTexSlot *s = &atlas->slot[i];
        for (int y = 0; y < s->h; y++) {
            uint8_t *dst = rgba + ((size_t)(s->y + y) * (size_t)atlasW + (size_t)s->x) * 4u;
            uint8_t *src = decoded[i].rgba + (size_t)y * (size_t)s->w * 4u;
            memcpy(dst, src, (size_t)s->w * 4u);
        }
    }

    glGenTextures(1, &atlas->tex);
    glBindTexture(GL_TEXTURE_2D, atlas->tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, atlasW, atlasH, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    glBindTexture(GL_TEXTURE_2D, 0);
    free(rgba);

    for (uint32_t i = 0; i < slotCount; i++)
        free(decoded[i].rgba);

    atlas->w = atlasW;
    atlas->h = atlasH;
    atlas->slots = (int)slotCount;
    return valid;
}

static int packet_slot_offset(int nib)
{
    switch (nib) {
    case 1:
    case 5:
    case 13:
    case 15:
        return 0x12;
    case 9:
    case 11:
        return 0x16;
    case 12:
        return 0x10;
    default:
        return -1;
    }
}

static int packet_uv_offset(int nib, int idx)
{
    if (nib == 1 || nib == 3) {
        static const int off[4] = {0x0c, 0x10, 0x14, 0x14};
        return off[idx & 3];
    }
    if (nib == 9 || nib == 11) {
        static const int off[4] = {0x10, 0x12, 0x14, 0x14};
        return off[idx & 3];
    }
    static const int off[4] = {0x0c, 0x0e, 0x10, 0x10};
    return off[idx & 3];
}

int V8_XobfTex_DecodePacketUv(const V8XobfTexAtlas *atlas,
                              const uint8_t *pkt, int nib, int texBase,
                              float uv[4][2])
{
    if (atlas == NULL || atlas->tex == 0 || pkt == NULL)
        return 0;
    int slotOff = packet_slot_offset(nib);
    if (slotOff < 0)
        return 0;
    uint16_t rawSlot = rd16le(pkt, (uint32_t)slotOff);
    if (rawSlot == 0xffffu)
        return 0;
    int slot = texBase + (int)(rawSlot & 0x3fffu);
    if (slot < 0 || slot >= atlas->slots || !atlas->slot[slot].valid)
        return 0;

    const V8XobfTexSlot *s = &atlas->slot[slot];
    for (int i = 0; i < 4; i++) {
        int uvo = packet_uv_offset(nib, i);
        float u = (float)pkt[uvo + 0];
        float v = (float)pkt[uvo + 1];
        if (u > (float)(s->w - 1)) u = (float)(s->w - 1);
        if (v > (float)(s->h - 1)) v = (float)(s->h - 1);
        uv[i][0] = ((float)s->x + u + 0.5f) / (float)atlas->w;
        uv[i][1] = ((float)s->y + v + 0.5f) / (float)atlas->h;
    }
    return 1;
}

#endif
