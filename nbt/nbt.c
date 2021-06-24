#include "apilib.h"
#include <stdio.h>
#include <string.h>

void callback();
void magic_button();

int win, timer;
struct MOUSE_WINDOWS_Info msgs;

void HariMain(void)
{
    char* buf;

    api_initmalloc();
    buf = api_malloc(100 * 100);
    win = api_openwin(buf, 100, 100, 255, "New Bttn");
    api_wincallback(win, &msgs);

    for (;;) {
        magic_button();
    }
    api_end();
}
int i = 0;

int dx, dy, pre_dx = 0, pre_dy = 0;
void magic_button()
{
    int r, g, b, c;
    char enter = 0;
    if (20 <= msgs.dx && msgs.dx <= 80 && 25 <= msgs.dy && msgs.dy <= 85) {

        enter = 1;

        if (msgs.status == 1) {
            r = 0xD4;
            g = 0xE7;
            b = 0xA9;
        } else {
            r = 0xFF;
            g = 0x70;
            b = 0x43;
        }

        dx = (msgs.dx - 50) * 5 / 30;
        dy = (msgs.dy - 55) * 5 / 30;
    } else {
        r = 0xCC;
        g = 0x68;
        b = 0xAA;
        enter = 0;
        dx = 0;
        dy = 0;
    }

    c = 16 + (r / 43) + (g / 43) * 6 + (b / 43) * 36;
    if ((dx ^ pre_dx) || (dy ^ pre_dy)) {
        api_boxfilwin(win, 15, 23, 85, 90, 7);
        if (enter) {
            api_rboxfilwin(win, 23 + dx, 28 + dy, 77 + dx, 82 + dy, (0 << 16) + 10);
        }
        api_cursorvis(!enter);
        api_rboxfilwin(win, 25 + dx, 30 + dy, 75 + dx, 80 + dy, (c << 16) + 10);

        api_refreshwin(win, 20, 25, 80, 85);
        pre_dx = dx;
        pre_dy = dy;
    }
}