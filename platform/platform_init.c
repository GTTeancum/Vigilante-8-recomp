/* platform_init.c -- Phase 0 placeholder. SDL window / GL context / audio
 * device init happens here once we have third_party deps. */
#include <stdio.h>

int Platform_Init(int width, int height, const char *title)
{
    (void)width; (void)height; (void)title;
    printf("Platform_Init: phase 0 stub\n");
    return 0;
}

void Platform_Shutdown(void)
{
    printf("Platform_Shutdown: phase 0 stub\n");
}
