/* main.c -- entry point for the v8 recomp binary.
 *
 * Phase 0: parses CLI flags, runs --selftest, exits. Does not yet
 * boot the main loop (Phase 1 wires that).
 */
#include <stdio.h>
#include <string.h>

int Platform_Init(int w, int h, const char *title);
void Platform_Shutdown(void);
int  Smoke_RunSelfTest(void);

static void print_help(void) {
    puts("v8 -- Vigilante 8 recomp\n"
         "Usage: v8 [flags]\n"
         "  --selftest           run deterministic unit checks, exit 0/1\n"
         "  --frames N           run N main-loop ticks then exit (phase 2+)\n"
         "  --headless           no window (phase 2+)\n"
         "  --replay <path>      feed recorded pad-bytes (phase 4+)\n"
         "  --screenshot <path>  dump backbuffer to PNG on exit (phase 3+)\n"
         "  --audio-capture <p>  dump mixer to WAV (phase 8+)\n"
         "  --report-heap        alloc/free balance on exit\n"
         "  -h, --help           this message\n");
}

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--selftest"))   return Smoke_RunSelfTest();
        if (!strcmp(argv[i], "-h") ||
            !strcmp(argv[i], "--help"))       { print_help(); return 0; }
    }

    printf("v8: phase 0 -- selftest is the only wired command.\n");
    printf("    Run with --selftest to invoke; --help for usage.\n");
    printf("    Phase 1+ will wire the main loop.\n");
    return 0;
}
