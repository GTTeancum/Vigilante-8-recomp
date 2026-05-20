/* shell_stub.c -- minimal stand-in for SHELL.DLL (out of scope per
 * CLAUDE.md).
 *
 * V8_MainLoop boots by loading SHELL.DLL and calling its entry, which
 * returns the chosen level path string. The SHELL module is the entire
 * pre-match UI; for the recomp we bypass it and hardcode the level.
 *
 * Hardcoded for now: "Track\\OilField.TER". Phase 5+ will replace this
 * with command-line --level <name> or a config file lookup.
 *
 * The handle layout V8_MainLoop expects:
 *   *(int *)(h + 4) is a function pointer returning char*.
 *
 * We allocate a static struct that mimics that layout.
 */
#include <stdint.h>
#include <stdio.h>

static char g_level_path[64] = "Track\\OilField.TER";

static char *shell_get_path(void) { return g_level_path; }

/* PSY-Q overlay handle layout (from main_loop.c usage):
 *   offset 0: opaque (image base on PSX)
 *   offset 4: function pointer to "get path" / entry
 *
 * On x64 default alignment, a function pointer field after uint32_t
 * lands at offset 8, not 4 -- the cleaned code reads at h+4 directly
 * (PSX 32-bit layout). Force pack(4) to keep the layout 1:1.
 */
#pragma pack(push, 4)
typedef struct {
    uint32_t opaque;
    char *(*get_path)(void);   /* 8 bytes on x64; pack(4) keeps offset 4 */
} ShellHandle;
#pragma pack(pop)

static ShellHandle g_shell_handle = { 0, shell_get_path };

uintptr_t Overlay_Open(const char *path)
{
    fprintf(stderr, "v8: Overlay_Open(%s) -> shell stub @%p\n", path, (void *)&g_shell_handle);
    return (uintptr_t)&g_shell_handle;
}

void Overlay_Close(uintptr_t handle)
{
    (void)handle;
}
