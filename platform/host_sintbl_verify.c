/* host_sintbl_verify.c -- one-shot integrity check that the host's
 * generated sin/cos table is bit-identical to the user's EXE table
 * at 0x800607b4. Diagnoses ROM-revision drift early so the user
 * isn't chasing physics ghosts.
 *
 * Silent on match; logs a warning with the first mismatch on miss.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern const int16_t g_v8_sincostbl[8192];

#define EXE_PATH     "input/SLUS_005.10"
#define EXE_LOADADDR 0x80010000u
#define LUT_ADDR     0x800607b4u
#define LUT_BYTES    (4096 * 2 * sizeof(int16_t))

int Host_VerifySinCosTable(void)
{
    FILE *f = fopen(EXE_PATH, "rb");
    if (!f) {
        /* No EXE available -- skip silently. The recomp is allowed to
         * run on extracted assets without the EXE present. */
        return 0;
    }

    unsigned long file_off = 0x800u + (LUT_ADDR - EXE_LOADADDR);
    if (fseek(f, (long)file_off, SEEK_SET) != 0) {
        fprintf(stderr, "v8: sintbl verify -- seek to 0x%lx failed\n", file_off);
        fclose(f);
        return -1;
    }

    int16_t exe_tbl[8192];
    size_t got = fread(exe_tbl, 1, LUT_BYTES, f);
    fclose(f);
    if (got != LUT_BYTES) {
        fprintf(stderr, "v8: sintbl verify -- short read (%zu of %zu)\n",
                got, (size_t)LUT_BYTES);
        return -1;
    }

    if (memcmp(exe_tbl, g_v8_sincostbl, LUT_BYTES) == 0) {
        /* match -- quiet success */
        return 0;
    }

    /* Find first mismatch for the warning. */
    int first = -1;
    for (int i = 0; i < 8192; i++) {
        if (exe_tbl[i] != g_v8_sincostbl[i]) { first = i; break; }
    }
    fprintf(stderr,
        "v8: sintbl verify -- MISMATCH at entry %d: exe=%d gen=%d\n"
        "v8:   This means your %s is a different revision than what the\n"
        "v8:   recomp was calibrated against. Physics will diverge from\n"
        "v8:   the original. (The generator formula is\n"
        "v8:   round(sin(2*pi*i/4096)*4096).)\n",
        first, exe_tbl[first], g_v8_sincostbl[first], EXE_PATH);
    return -1;
}
