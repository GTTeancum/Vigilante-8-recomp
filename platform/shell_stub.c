/* shell_stub.c -- minimal stand-in for SHELL.DLL (out of scope per
 * CLAUDE.md).
 *
 * V8_MainLoop boots by loading SHELL.DLL and calling its entry, which
 * returns the chosen level path string. The SHELL module is the entire
 * pre-match UI; for the recomp we bypass it and hardcode the level.
 *
 * The command-line/runtime terrain path in main.c is the authoritative
 * selection.  The shell stub mirrors it into the Track\<name>.TER path shape
 * expected by the cleaned main-loop path, so terrain/render selection and
 * gameplay level selection cannot drift apart.
 *
 * The handle layout V8_MainLoop expects:
 *   *(int *)(h + 4) is a function pointer returning char*.
 *
 * We allocate a static struct that mimics that layout.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern char g_v8_level_exp_path[128];

static char g_level_path[64] = "Track\\WILDWEST.TER";

static char *shell_get_path(void)
{
    const char *name = g_v8_level_exp_path;
    const char *slash = strrchr(name, '\\');
    const char *slash2 = strrchr(name, '/');
    if (slash2 != NULL && (slash == NULL || slash2 > slash)) slash = slash2;
    if (slash != NULL) name = slash + 1;

    char bare[32];
    snprintf(bare, sizeof(bare), "%s", name);
    char *dot = strrchr(bare, '.');
    if (dot != NULL) *dot = 0;
    snprintf(g_level_path, sizeof(g_level_path), "Track\\%s.TER", bare);
    return g_level_path;
}

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
