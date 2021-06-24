#include "apilib.h"
#include <stdio.h>
#include <string.h>

void callback();

int win;
char* msgs;

void HariMain(void)
{
    char* buf;
    

    api_initmalloc();
    buf = api_malloc(150 * 100);
    msgs = api_malloc(4);
    win = api_openwin(buf, 150, 100, -1, "star1");
    api_wincallback(win, msgs);
    api_boxfilwin(win, 6, 26, 143, 93, 8);
    api_point(win, 75, 59, 3);
    for (;;) {
        callback();
        // if (api_getkey(1) == 0x0a) {
        //     break;
        // }
    }
    api_end();
}
int i = 0;
void callback()
{
    char str[20];
    sprintf(str, "%d %d", ((int*)msgs)[0], msgs[1]);
    api_putstrwin(win, 10, 30, 6, strlen(str), str);
}