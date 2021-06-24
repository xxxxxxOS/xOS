#include "apilib.h"
#include <stdio.h>
#include <string.h>

void callback();

int win;
struct MOUSE_WINDOWS_Info msgs;

void HariMain(void)
{
    char* buf;

    api_initmalloc();
    buf = api_malloc(180 * 100);
    win = api_openwin(buf, 180, 100, 255, "star1");
    api_wincallback(win, &msgs);
    // api_boxfilwin(win, 6, 26, 143, 93, 7);
    // api_point(win, 75, 59, 3);
    for (;;) {
        callback();
    }
    api_end();
}
int i = 0;
void callback()
{
    char str[30];
    sprintf(str, "%d %d", msgs.dx, msgs.dy);
    // api_boxfilwin(win, 10, 30, 80, 60, 9); 
    api_putstrwin(win, 10, 30, 6, strlen(str), str);
}