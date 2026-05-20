/* host_asset.c -- file I/O backing for the v8 asset loaders.
 *
 * Asset_LoadFile (FUN_80015948) is the PSY-Q-era "read this file from
 * the CD into a freshly-malloc'd buffer" wrapper. On the host we back
 * it with fopen/fread against the extracted CD files in input/.
 *
 * Path translation:
 *   - The cleaned code uses PSX backslash paths like "Track\\Foo.TER"
 *     and "Sounds\\Main.SND" -- translate '\' -> '/'.
 *   - Prepend "input/" since that's where the extracted assets live.
 *
 * Returned buffers come from the v8 heap (Heap_AllocOrRetry) so the
 * existing free path (Heap_Free) works identically.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

extern void *Heap_AllocOrRetry(uint32_t n);
extern void  Heap_Free(void *p);

#define ROOT_PREFIX "input/"

/* Translate "Track\\Foo.TER" -> "input/TRACK/FOO.TER" (upper-cased)
 * since the extracted files are upper-case on disk. */
static void translate_path(const char *in, char *out, size_t outsz)
{
    size_t pre = strlen(ROOT_PREFIX);
    if (pre + strlen(in) + 1 > outsz) { out[0] = 0; return; }
    memcpy(out, ROOT_PREFIX, pre);
    for (size_t i = 0; in[i]; i++) {
        char c = in[i];
        if (c == '\\') c = '/';
        out[pre + i] = (char)toupper((unsigned char)c);
    }
    out[pre + strlen(in)] = 0;
}

void *Asset_LoadFile(const char *path)
{
    char real[256];
    translate_path(path, real, sizeof real);

    FILE *f = fopen(real, "rb");
    if (!f) {
        fprintf(stderr, "v8: Asset_LoadFile(%s) -> %s: NOT FOUND\n", path, real);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    void *buf = Heap_AllocOrRetry((uint32_t)size);
    if (!buf) {
        fprintf(stderr, "v8: Asset_LoadFile(%s) -> OOM (%ld bytes)\n", path, size);
        fclose(f);
        return NULL;
    }
    if (fread(buf, 1, size, f) != (size_t)size) {
        fprintf(stderr, "v8: Asset_LoadFile(%s) -> short read\n", path);
        Heap_Free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);
    fprintf(stderr, "v8: Asset_LoadFile(%s) -> %s [%ld bytes] @%p\n",
            path, real, size, buf);
    return buf;
}
