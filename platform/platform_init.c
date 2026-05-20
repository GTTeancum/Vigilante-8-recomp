/* platform_init.c -- SDL2 window + OpenGL 3.3 core context.
 *
 * Phase 3a: open a window, init GL, expose a clear color so we can
 * tell the renderer is alive. Phase 3c+ adds terrain mesh.
 *
 * When V8_HAVE_SDL is not defined (CMake didn't find SDL2), all
 * functions degrade to no-ops so headless smoke tests still work.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#if defined(V8_HAVE_SDL)
  #include <SDL.h>
  #if defined(V8_HAVE_GL)
    #include <GL/gl3w.h>
  #endif
#endif

static int      g_initialized = 0;
static int      g_width = 640, g_height = 480;
#if defined(V8_HAVE_SDL)
static SDL_Window   *g_window   = NULL;
static SDL_GLContext g_glctx    = NULL;
#endif

int Platform_Init(int width, int height, const char *title)
{
    g_width  = width;
    g_height = height;

#if defined(V8_HAVE_SDL)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "v8: SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    g_window = SDL_CreateWindow(title,
                                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                width, height,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!g_window) {
        fprintf(stderr, "v8: SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    g_glctx = SDL_GL_CreateContext(g_window);
    if (!g_glctx) {
        fprintf(stderr, "v8: SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return -1;
    }
    SDL_GL_MakeCurrent(g_window, g_glctx);
    SDL_GL_SetSwapInterval(0);   /* no vsync; we time off frame count */

  #if defined(V8_HAVE_GL)
    if (gl3wInit() != 0) {
        fprintf(stderr, "v8: gl3wInit failed\n");
        return -1;
    }
    fprintf(stderr, "v8: GL %s | %s\n",
            (const char *)glGetString(GL_VERSION),
            (const char *)glGetString(GL_RENDERER));
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
  #endif

    g_initialized = 1;
    fprintf(stderr, "v8: Platform_Init OK (%dx%d, '%s')\n", width, height, title);
    return 0;
#else
    fprintf(stderr, "v8: Platform_Init -- SDL not compiled in (headless build)\n");
    (void)title;
    return 0;
#endif
}

void Platform_Shutdown(void)
{
#if defined(V8_HAVE_SDL)
    if (g_glctx)  SDL_GL_DeleteContext(g_glctx);
    if (g_window) SDL_DestroyWindow(g_window);
    SDL_Quit();
    g_glctx = NULL; g_window = NULL;
    g_initialized = 0;
    fprintf(stderr, "v8: Platform_Shutdown\n");
#endif
}

/* Called once per logical frame from sched_shim.c / pad_shim.c.
 * Pumps SDL events, clears + swaps the GL backbuffer. */
void Platform_FrameTick(void)
{
#if defined(V8_HAVE_SDL)
    if (!g_initialized) return;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            fprintf(stderr, "v8: SDL_QUIT received\n");
            exit(0);
        }
    }
  #if defined(V8_HAVE_GL)
    extern void Renderer_DrawFrame(int w, int h, int frame_idx);
    extern int  g_v8_frame_count;
    Renderer_DrawFrame(g_width, g_height, g_v8_frame_count);
  #endif
    SDL_GL_SwapWindow(g_window);
#endif
}

/* Read backbuffer pixels into the supplied RGBA8 buffer. Returns 0 on success. */
int Platform_ReadBackbuffer(uint8_t *rgba, int *width_out, int *height_out)
{
#if defined(V8_HAVE_SDL) && defined(V8_HAVE_GL)
    if (!g_initialized) return -1;
    /* The last SwapWindow promoted the back to the front; back is now
     * undefined. Re-render one frame into back and read from there
     * BEFORE the next swap, so we always get the current visible
     * frame. */
    extern void Renderer_DrawFrame(int w, int h, int frame_idx);
    extern int  g_v8_frame_count;
    Renderer_DrawFrame(g_width, g_height, g_v8_frame_count);
    glFlush();
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, g_width, g_height, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    if (width_out)  *width_out  = g_width;
    if (height_out) *height_out = g_height;
    return 0;
#else
    (void)rgba; (void)width_out; (void)height_out;
    return -1;
#endif
}

int Platform_Width(void)  { return g_width; }
int Platform_Height(void) { return g_height; }
