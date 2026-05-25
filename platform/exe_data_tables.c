/* exe_data_tables.c -- load original SLUS data tables used by source code.
 *
 * These globals are not asset files, but literal initialized data embedded in
 * the PS-X EXE.  Keeping them byte-sourced from SLUS avoids drifting physics
 * constants while the surrounding systems are still being decompiled.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern const int16_t g_v8_sincostbl[8192];

extern int16_t  DAT_800607b4[8192];
extern int16_t  DAT_800647b4[1025];
extern uint16_t DAT_8005ec68[8];
extern uint8_t  DAT_8005ec74[256];
extern int16_t  DAT_8005ec84[12];
extern int32_t  DAT_80065748[3];
extern int32_t  DAT_80065754[3];
extern int32_t  DAT_8006576c[4];
extern int32_t  DAT_80065778[4];
extern int32_t  DAT_80065788[4];
extern int32_t  DAT_800657a4[4];
extern int32_t  DAT_80065864[3];
extern uint8_t  DAT_80065674[16];
extern uint32_t DAT_80065930[4];
extern uint8_t  DAT_80056774[256];
extern uint8_t  DAT_80056778[256];
extern uint8_t  DAT_800567d4[64];
extern int32_t  iRam00000030;
extern int16_t  sRam000005e4;
extern uint8_t  bRam000006cf;

#define EXE_LOADADDR 0x80010000u

static int read_at(FILE *f, uint32_t psx_addr, void *dst, size_t nbytes)
{
    uint32_t off = 0x800u + (psx_addr - EXE_LOADADDR);
    if (fseek(f, (long)off, SEEK_SET) != 0)
        return -1;
    return fread(dst, 1, nbytes, f) == nbytes ? 0 : -1;
}

static FILE *open_slus(void)
{
    static const char *paths[] = {
        "SLUS_005.10",
        "input\\SLUS_005.10",
        "..\\input\\SLUS_005.10",
        "disc_extract\\SLUS_005.10",
        "..\\disc_extract\\SLUS_005.10",
        NULL
    };

    for (int i = 0; paths[i]; i++) {
        FILE *f = fopen(paths[i], "rb");
        if (f) {
            char hdr[8];
            if (fread(hdr, 1, sizeof(hdr), f) == sizeof(hdr) &&
                memcmp(hdr, "PS-X EXE", 8) == 0) {
                rewind(f);
                return f;
            }
            fclose(f);
        }
    }
    return NULL;
}

int Host_LoadExeDataTables(void)
{
    static int done = 0;
    if (done)
        return 0;
    done = 1;

    memcpy(DAT_800607b4, g_v8_sincostbl, sizeof(DAT_800607b4));

    FILE *f = open_slus();
    if (!f)
        return 0;

    int ok = 1;
    ok &= read_at(f, 0x800607b4u, DAT_800607b4, sizeof(DAT_800607b4)) == 0;
    ok &= read_at(f, 0x800647b4u, DAT_800647b4, sizeof(DAT_800647b4)) == 0;
    ok &= read_at(f, 0x8005ec68u, DAT_8005ec68, sizeof(DAT_8005ec68)) == 0;
    ok &= read_at(f, 0x8005ec74u, DAT_8005ec74, sizeof(DAT_8005ec74)) == 0;
    ok &= read_at(f, 0x8005ec84u, DAT_8005ec84, sizeof(DAT_8005ec84)) == 0;
    ok &= read_at(f, 0x80065748u, DAT_80065748, sizeof(DAT_80065748)) == 0;
    ok &= read_at(f, 0x80065754u, DAT_80065754, sizeof(DAT_80065754)) == 0;
    ok &= read_at(f, 0x8006576cu, DAT_8006576c, sizeof(DAT_8006576c)) == 0;
    ok &= read_at(f, 0x80065778u, DAT_80065778, sizeof(DAT_80065778)) == 0;
    ok &= read_at(f, 0x80065788u, DAT_80065788, sizeof(DAT_80065788)) == 0;
    ok &= read_at(f, 0x800657a4u, DAT_800657a4, sizeof(DAT_800657a4)) == 0;
    ok &= read_at(f, 0x80065864u, DAT_80065864, sizeof(DAT_80065864)) == 0;
    ok &= read_at(f, 0x80065674u, DAT_80065674, sizeof(DAT_80065674)) == 0;
    ok &= read_at(f, 0x80065930u, DAT_80065930, sizeof(DAT_80065930)) == 0;
    ok &= read_at(f, 0x80056774u, DAT_80056774, sizeof(DAT_80056774)) == 0;
    ok &= read_at(f, 0x80056778u, DAT_80056778, sizeof(DAT_80056778)) == 0;
    ok &= read_at(f, 0x800567d4u, DAT_800567d4, sizeof(DAT_800567d4)) == 0;
    ok &= read_at(f, 0x80065334u, &iRam00000030, sizeof(iRam00000030)) == 0;
    ok &= read_at(f, 0x800658e8u, &sRam000005e4, sizeof(sRam000005e4)) == 0;
    ok &= read_at(f, 0x800659d3u, &bRam000006cf, sizeof(bRam000006cf)) == 0;
    fclose(f);

    if (!ok)
        fprintf(stderr, "v8: EXE data table load had one or more short reads\n");
    return ok ? 0 : -1;
}
