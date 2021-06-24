/* 显示 */

#include "bootpack.h"


void init_palette(void)
{
    static unsigned char table_rgb[16 * 3] = {
        0x00, 0x00, 0x00,
        0xff, 0x00, 0x00,
        0x00, 0xff, 0x00,
        0xff, 0xff, 0x00,
        0x00, 0x00, 0xff,
        0xff, 0x00, 0xff,
        0x00, 0xff, 0xff,
        0xff, 0xff, 0xff,
        0xc6, 0xc6, 0xc6,
        0x84, 0x00, 0x00,
        0x00, 0x84, 0x00,
        0x84, 0x84, 0x00,
        0x00, 0x00, 0x84,
        0x84, 0x00, 0x84,
        0x00, 0x84, 0x84,
        0x84, 0x84, 0x84
    };
    unsigned char table2[216 * 3];
    int r, g, b;
    set_palette(0, 15, table_rgb);
    for (b = 0; b < 6; b++) {
        for (g = 0; g < 6; g++) {
            for (r = 0; r < 6; r++) {
                table2[(r + g * 6 + b * 36) * 3 + 0] = r * 51;
                table2[(r + g * 6 + b * 36) * 3 + 1] = g * 51;
                table2[(r + g * 6 + b * 36) * 3 + 2] = b * 51;
            }
        }
    }
    set_palette(16, 231, table2);
    return;

    /* static char 相当于汇编的DB */
}

void set_palette(int start, int end, unsigned char* rgb)
{
    int i, eflags;
    eflags = io_load_eflags(); 
    io_cli(); 
    io_out8(0x03c8, start);
    for (i = start; i <= end; i++) {
        io_out8(0x03c9, rgb[0] / 4);
        io_out8(0x03c9, rgb[1] / 4);
        io_out8(0x03c9, rgb[2] / 4);
        rgb += 3;
    }
    io_store_eflags(eflags);
    return;
}

void boxfill8(unsigned char* vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
    int x, y;
    for (y = y0; y <= y1; y++) {
        for (x = x0; x <= x1; x++)
            vram[y * xsize + x] = c;
    }
    return;
}

void circle_fill(unsigned char* vram, int xsize, unsigned char c, int x0, int y0, int radius)
{
    int x, y;
    int index;
    for (y = -radius; y <= radius; y += 1) {
        for (x = -radius; x <= radius; x += 1)
            if (x * x + y * y <= radius * radius) {
                index = (int)(y + y0) * xsize + (int)x + x0;
                if (index >= 0)
                {
                    vram[index] = c;
                }
            }
    }
    return;
}

void radius_box_fill(unsigned char* vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1, int radius)
{
    circle_fill(vram, xsize, c, x0 + radius, y0 + radius, radius);
    circle_fill(vram, xsize, c, x1 - radius, y0 + radius, radius);
    circle_fill(vram, xsize, c, x0 + radius, y1 - radius, radius);
    circle_fill(vram, xsize, c, x1 - radius, y1 - radius, radius);

    boxfill8(vram, xsize, c, x0 + radius, y0 + radius, x1 - radius, y1 - radius); // 主体
    boxfill8(vram, xsize, c, x0 + radius, y0, x1 - radius, y0 + radius); // 上
    boxfill8(vram, xsize, c, x1 - radius, y0 + radius, x1, y1 - radius); // 右
    boxfill8(vram, xsize, c, x0 + radius, y1 - radius, x1 - radius, y1); // 下
    boxfill8(vram, xsize, c, x0, y0 + radius, x0 + radius, y1 - radius); // 左
}

void init_screen8(char* vram, int x, int y)
{
    int xsize = x, ysize = y;
    int bar_width;
    int bar_x, bar_bottom = 5;

    bar_width = 0.8 * xsize;
    bar_x = 0.1 * xsize;

    boxfill8(vram, xsize, GET_COLOR(0xC3, 0x52, 0x30), 0, 0, xsize, ysize);
    
    // boxfill8(vram, xsize, COL8_FFFF00, 30, 50, 80, 100);
    // radius_box_fill(vram, xsize, COL8_C6C6C6, 10, 10, 30, 20, 5);
    // circle_fill(vram, xsize, COL8_C6C6C6, 100, 50, 10);

    boxfill8(vram, xsize, 7, 0, 0, xsize, 24);
    putfonts8_asc(vram, xsize, 20, 5, 0, "( xOS )");
    putfonts8_asc(vram, xsize, 450, 5, 0, "<< 9:20 AM >>");

    radius_box_fill(vram, xsize, 7, bar_x - 2, ysize - 46 - bar_bottom, xsize + 1 - bar_x, ysize - 2 - bar_bottom, 10); //开始菜单长条
    radius_box_fill(vram, xsize, COL8_C6C6C6, bar_x, ysize - 50 - bar_bottom, xsize - 1 - bar_x, ysize - 4 - bar_bottom, 10);

    radius_box_fill(vram, xsize, 7, bar_x + 68, ysize - 47 - bar_bottom, xsize - 12 - bar_x, ysize - 11 - bar_bottom, 6); //开始菜单旁边的装饰物
    radius_box_fill(vram, xsize, 0, bar_x + 72, ysize - 43 - bar_bottom, xsize - 8 - bar_x, ysize - 7 - bar_bottom, 6);
    radius_box_fill(vram, xsize, 15, bar_x + 70, ysize - 45 - bar_bottom, xsize - 10 - bar_x, ysize - 9 - bar_bottom, 6);

    radius_box_fill(vram, xsize, 0, bar_x + 12, ysize - 37 - bar_bottom, bar_x + 62, ysize - 12 - bar_bottom, 4);
    radius_box_fill(vram, xsize, COL8_FFFFFF, bar_x + 10, ysize - 40 - bar_bottom, bar_x + 60, ysize - 14 - bar_bottom, 4);
    putfonts16_chn(vram, xsize, bar_x + 18 + 1, ysize - 34 - bar_bottom, 15, "\1\2");
    putfonts16_chn(vram, xsize, bar_x + 18, ysize - 35 - bar_bottom, 0, "\1\2");

    char good_morning[] = { 4, 5, 6, 7, 8, 9, 10, 0 };
   
    putfonts16_chn(vram, xsize, bar_x + 400, ysize - 35 - bar_bottom, 7, good_morning);
    return;
}

void putfont8(char* vram, int xsize, int x, int y, char c, char* font)
{
    int i;
    char *p, d /* data */;
    for (i = 0; i < 16; i++) {
        p = vram + (y + i) * xsize + x;
        d = font[i];
        for (int j = 0; j < 8; j++)
            if ((d & (0x01 << j)) != 0)
                p[7 - j] = c;
    }
    return;
}

void putfont16(char* vram, int xsize, int x, int y, char c, short* font)
{
    int i;
    char* p;
    short d /* data */;
    for (i = 0; i < 16; i++) {
        p = vram + (y + i) * xsize + x;
        d = font[i];
        for (int j = 0; j < 8; j++)
            if ((d & (0x01 << j)) != 0)
                p[7 - j] = c;
        for (int j = 8; j < 16; j++)
            if ((d & (0x01 << j)) != 0)
                p[23 - j] = c;
    }
    return;
}

void putfonts16_chn(char* vram, int xsize, int x, int y, char c, unsigned char* s)
{
    extern char Chinese[];
    for (; *s != 0x00; s++) {
        putfont16(vram, xsize, x, y, c, Chinese + (*s - 1) * 32);
        x += 18;
    }
    return;
}

void putfonts8_asc(char* vram, int xsize, int x, int y, char c, unsigned char* s)
{
    extern char hankaku[4096];
    for (; *s != 0x00; s++) {
        putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
        x += 8;
    }
    return;
}

void init_mouse_cursor8(char* mouse, char bc)
{
    static char cursor[16][16] = {
        "......****......",
        "....**OOOO**....",
        "...*OOOOOOOO*...",
        "..*OOOOOOOOOO*..",
        ".*OOOOOOOOOOOO*.",
        ".*OOOOO**OOOOO*.",
        "*OOOOO*..*OOOOO*",
        "*OOOO*....*OOOO*",
        "*OOOO*....*OOOO*",
        "*OOOOO*..*OOOOO*",
        ".*OOOOO**OOOOO*.",
        ".*OOOOOOOOOOOO*.",
        "..*OOOOOOOOOO*..",
        "...*OOOOOOOO*...",
        "....**OOOO**....",
        "......****......",
    };
    int x, y;
    int index;
    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++) {
            index = y * 16 + x;
            if (Mouse_Vis)
            {
                if (cursor[y][x] == '*') {
                    mouse[index] = COL8_000000;
                }
                if (cursor[y][x] == 'O') {
                    mouse[index] = COL8_FFFFFF;
                }
                if (cursor[y][x] == '.') {
                    mouse[index] = bc;
                }
            }else{
                mouse[index] = bc;
            }
        }
    }
    return;
}

void putblock8_8(char* vram, int vxsize, int pxsize,
    int pysize, int px0, int py0, char* buf, int bxsize)
{
    int x, y;
    for (y = 0; y < pysize; y++) {
        for (x = 0; x < pxsize; x++) {
            vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
        }
    }
    return;
}
