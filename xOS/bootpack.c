#include "bootpack.h"
#include "../apilib.h"
#include <stdio.h>
#include <string.h>

#define KEYCMD_LED 0xed

void keywin_off(struct SHEET* key_win);
void keywin_on(struct SHEET* key_win);
void close_console(struct SHEET* sht);
void close_constask(struct TASK* task);

char prev_Mouse_Vis = 1;
char Mouse_Vis = 1;

void HariMain(void)
{
    struct BOOTINFO* binfo = (struct BOOTINFO*)ADR_BOOTINFO;
    struct SHTCTL* shtctl;
    char s[40];
    struct FIFO32 fifo, keycmd;
    int fifobuf[128], keycmd_buf[32];
    int mx, my, i, new_mx = -1, new_my = 0, new_wx = 0x7fffffff, new_wy = 0;
    unsigned int memtotal;
    struct MOUSE_DEC mdec;
    struct MEMMAN* memman = (struct MEMMAN*)MEMMAN_ADDR;
    unsigned char *buf_back, buf_mouse[256];
    struct SHEET *sht_back, *sht_mouse;
    struct TASK *task_a, *task;
    int last_key_win_loc[2] = { 32, 16 };

    struct MOUSE_WINDOWS_Info mw_info;

    static char keytable0[0x80] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0x08, 0,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0x0a, 0, 'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0, 0, ']', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0x5c, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x5c, 0, 0
    };
    static char keytable1[0x80] = {
        0, 0, '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0x08, 0,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0x0a, 0, 'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0, 0, '}', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, '_', 0, 0, 0, 0, 0, 0, 0, 0, 0, '|', 0, 0
    };
    int key_shift = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;
    int j, x, y, mmx = -1, mmy = -1, mmx2 = 0;
    struct SHEET *sht = 0, *key_win, *sht2;

    init_gdtidt();
    init_pic();
    io_sti(); /* IDT/PIC */
    fifo32_init(&fifo, 128, fifobuf, 0);
    *((int*)0x0fec) = (int)&fifo;
    init_pit();
    init_keyboard(&fifo, 256);
    enable_mouse(&fifo, 512, &mdec);
    io_out8(PIC0_IMR, 0xf8); /* PIC1键盘(11111001)*/
    io_out8(PIC1_IMR, 0xef); /*鼠标(11101111)*/
    fifo32_init(&keycmd, 32, keycmd_buf, 0);

    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    init_palette();
    shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
    task_a = task_init(memman);
    fifo.task = task_a;
    task_run(task_a, 1, 2);
    *((int*)0x0fe4) = (int)shtctl;

    /* sht_back */
    sht_back = sheet_alloc(shtctl);
    buf_back = (unsigned char*)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
    init_screen8(buf_back, binfo->scrnx, binfo->scrny);

    /* sht_cons */
    // key_win = open_console(shtctl, memtotal);
    key_win = 0;

    /* sht_mouse */
    sht_mouse = sheet_alloc(shtctl);
    sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
    init_mouse_cursor8(buf_mouse, 99);
    mx = (binfo->scrnx - 16) / 2;
    my = (binfo->scrny - 28 - 16) / 2;

    sheet_slide(sht_back, 0, 0);
    // sheet_slide(key_win, 32, 4);
    sheet_slide(sht_mouse, mx, my);
    sheet_updown(sht_back, 0);
    // sheet_updown(key_win, 1);
    sheet_updown(sht_mouse, 2);
    // keywin_on(key_win);

    fifo32_put(&keycmd, KEYCMD_LED);
    fifo32_put(&keycmd, key_leds);

    for (;;) {
        if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
            keycmd_wait = fifo32_get(&keycmd);
            wait_KBC_sendready();
            io_out8(PORT_KEYDAT, keycmd_wait);
        }
        io_cli();
        if (fifo32_status(&fifo) == 0) {
            if (new_mx >= 0) {
                io_sti();
                sheet_slide(sht_mouse, new_mx - 8, new_my - 8);
                new_mx = -1;
            } else if (new_wx != 0x7fffffff) {
                io_sti();
                sheet_slide(sht, new_wx, new_wy);
                new_wx = 0x7fffffff;
            } else {
                task_sleep(task_a);
                io_sti();
            }
        } else {
            i = fifo32_get(&fifo);
            io_sti();
            if (key_win != 0 && key_win->flags == 0) { 
                if (shtctl->top == 1) { 
                    key_win = 0;
                } else {
                    key_win = shtctl->sheets[shtctl->top - 1];
                    keywin_on(key_win);
                }
            }
            if (256 <= i && i <= 511) { 
                if (i < 0x80 + 256) { 
                    if (key_shift == 0) {
                        s[0] = keytable0[i - 256];
                    } else {
                        s[0] = keytable1[i - 256];
                    }
                } else {
                    s[0] = 0;
                }
                if ('A' <= s[0] && s[0] <= 'Z') { 
                    if (((key_leds & 4) == 0 && key_shift == 0) || ((key_leds & 4) != 0 && key_shift != 0)) {
                        s[0] += 0x20; 
                    }
                }
                if (s[0] != 0 && key_win != 0) { 
                    fifo32_put(&key_win->task->fifo, s[0] + 256);
                }
                if (i == 256 + 0x0f && key_win != 0) { 
                    keywin_off(key_win);
                    j = key_win->height - 1;
                    if (j == 0) {
                        j = shtctl->top - 1;
                    }
                    key_win = shtctl->sheets[j];
                    keywin_on(key_win);
                }
                if (i == 256 + 0x2a) { /* 左移 ON */
                    key_shift |= 1;
                }
                if (i == 256 + 0x36) { /* 右移 ON */
                    key_shift |= 2;
                }
                if (i == 256 + 0xaa) { /* 左移 OFF */
                    key_shift &= ~1;
                }
                if (i == 256 + 0xb6) { /* 右移 OFF */
                    key_shift &= ~2;
                }
                if (i == 256 + 0x3a) { /* CapsLock */
                    key_leds ^= 4;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }
                if (i == 256 + 0x45) { /* NumLock */
                    key_leds ^= 2;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }
                if (i == 256 + 0x46) { /* ScrollLock */
                    key_leds ^= 1;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }
                if (i == 256 + 0x3b && key_shift != 0 && key_win != 0) { /* Shift+F1 */
                    task = key_win->task;
                    if (task != 0 && task->tss.ss0 != 0) {
                        cons_putstr0(task->cons, "\nBreak(key) :\n");
                        io_cli(); 
                        task->tss.eax = (int)&(task->tss.esp0);
                        task->tss.eip = (int)asm_end_app;
                        io_sti();
                        task_run(task, -1, 0);
                    }
                }
                if (i == 256 + 0x3c && key_shift != 0) { /* Shift+F2 */
                    
                    if (key_win != 0) {
                        keywin_off(key_win);
                    }
                    key_win = open_console(shtctl, memtotal);
                    sheet_slide(key_win, last_key_win_loc[0], last_key_win_loc[1]);
                    sheet_updown(key_win, shtctl->top);
                    keywin_on(key_win);

                    last_key_win_loc[0] = (last_key_win_loc[0] + 16) % (binfo->scrnx / 5 * 4);
                    last_key_win_loc[1] = (last_key_win_loc[1] + 16) % (binfo->scrny / 5 * 4);
                }
                if (i == 256 + 0x57) { /* F11 */
                    sheet_updown(shtctl->sheets[1], shtctl->top - 1);
                }
                if (i == 256 + 0xfa) { 
                    keycmd_wait = -1;
                }
                if (i == 256 + 0xfe) { 
                    wait_KBC_sendready();
                    io_out8(PORT_KEYDAT, keycmd_wait);
                }
            } else if (512 <= i && i <= 767) {
                int decode_res = mouse_decode(&mdec, i - 512);
                if (decode_res != 0) {
                    
                    mx += mdec.x;
                    my += mdec.y;
                    if (mx < 0) {
                        mx = 0;
                    }
                    if (my < 0) {
                        my = 0;
                    }
                    if (mx > binfo->scrnx - 1) {
                        mx = binfo->scrnx - 1;
                    }
                    if (my > binfo->scrny - 1) {
                        my = binfo->scrny - 1;
                    }
                    new_mx = mx;
                    new_my = my;
                    if ((mdec.btn & 0x01) != 0) {
                        if (mmx < 0) {
                            for (j = shtctl->top - 1; j > 0; j--) {
                                sht = shtctl->sheets[j];
                                x = mx - sht->vx0;
                                y = my - sht->vy0;
                                if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize) {
                                    if (sht->buf[y * sht->bxsize + x] != sht->col_inv) {
                                        sheet_updown(sht, shtctl->top - 1);
                                        if (sht != key_win) {
                                            keywin_off(key_win);
                                            key_win = sht;
                                            keywin_on(key_win);
                                        }
                                        if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21) {
                                            mmx = mx; 
                                            mmy = my;
                                            mmx2 = sht->vx0;
                                            new_wy = sht->vy0;
                                        }
                                        if (4 <= x && x < 20 && 5 <= y && y < 19) {
                                            if ((sht->flags & 0x10) != 0) { 
                                                task = sht->task;
                                                cons_putstr0(task->cons, "\nBreak(mouse) :\n");
                                                io_cli(); 
                                                task->tss.eax = (int)&(task->tss.esp0);
                                                task->tss.eip = (int)asm_end_app;
                                                io_sti();
                                                task_run(task, -1, 0);
                                            } else { 
                                                task = sht->task;
                                                sheet_updown(sht, -1); 
                                                keywin_off(key_win);
                                                key_win = shtctl->sheets[shtctl->top - 1];
                                                keywin_on(key_win);
                                                io_cli();
                                                fifo32_put(&task->fifo, 4);
                                                io_sti();
                                            }
                                        }else{
                                            // TODO 窗口的鼠标事件
                                            if (sht->msgs!=0)
                                            {

                                                mw_info.mx = mx;
                                                mw_info.my = my;
                                                mw_info.wx = sht->vx0;
                                                mw_info.wy = sht->vy0;
                                                mw_info.dx = x;
                                                mw_info.dy = y;
                                                mw_info.status = 1;

                                                for (size_t i = 0; i < sizeof(mw_info); i++) {
                                                    ((char*)sht->msgs)[i] = ((char *)&mw_info)[i];
                                                }
                                            }
                                            // sprintf(s, "(%d, %d, %d)", sht->msgs, mx, ((struct MOUSE_WINDOWS_Info *)sht->msgs)->dx);
                                            // putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_000000, s, strlen(s));
                                        }
                                        
                                        break;
                                    }
                                }
                            }
                        } else {
                            
                            x = mx - mmx; 
                            y = my - mmy;
                            new_wx = (mmx2 + x + 2) & ~3;
                            new_wy = new_wy + y;
                            mmy = my; 
                        }
                    } else {
                        
                        mmx = -1; 
                        if (new_wx != 0x7fffffff) {
                            sheet_slide(sht, new_wx, new_wy); 
                            new_wx = 0x7fffffff;
                        }
                        mw_info.status = 0;
                    }
                }
                if (key_win) {
                    int dx = mx - key_win->vx0;
                    int dy = my - key_win->vy0;
                    if (0 <= dx && dx <= key_win->bxsize && 0 <= dy && dy <= key_win->bysize && key_win->msgs) {
                        mw_info.mx = mx;
                        mw_info.my = my;
                        mw_info.wx = key_win->vx0;
                        mw_info.wy = key_win->vy0;
                        mw_info.dx = dx;
                        mw_info.dy = dy;
                        // mw_info.status = 0;

                        for (size_t i = 0; i < sizeof(mw_info); i++) {
                            ((char*)key_win->msgs)[i] = ((char*)&mw_info)[i];
                        }
                    }
                }
                if (prev_Mouse_Vis ^ Mouse_Vis)
                {
                    init_mouse_cursor8(buf_mouse, 99);
                    prev_Mouse_Vis = Mouse_Vis;
                }

                int bar_width;
                int bar_x, bar_bottom = 5;

                bar_width = binfo->scrnx * 4 / 5;
                bar_x = binfo->scrnx / 10;
                if (bar_x + 10 <= mx && mx <= bar_x + 60 && binfo->scrny - 40 - bar_bottom <= my && my <= binfo->scrny - 14 - bar_bottom) {
                    radius_box_fill(sht_back->buf, sht_back->bxsize, 7, bar_x, binfo->scrny - 200 - bar_bottom, bar_x + 300, binfo->scrny - 60 - bar_bottom, 20);
                    // putfonts8_asc_sht(sht_back, bar_x + 30, binfo->scrny - 80 - bar_bottom, 0, 7, s, strlen(s));
                    char AUTHOR[] = {11, 12, 13, 0};
                    char XBN[] = {14, 15, 16, 0};
                    char ZHL[] = {17, 18, 19, 0};
                    char CY[] = {20, 3, 21, 0};

                    putfonts16_chn(sht_back->buf, sht_back->bxsize, bar_x + 30, binfo->scrny - 180 - bar_bottom, 0, AUTHOR);
                    putfonts16_chn(sht_back->buf, sht_back->bxsize, bar_x + 50, binfo->scrny - 150 - bar_bottom, 0, XBN);
                    putfonts16_chn(sht_back->buf, sht_back->bxsize, bar_x + 50, binfo->scrny - 130 - bar_bottom, 0, ZHL);
                    putfonts16_chn(sht_back->buf, sht_back->bxsize, bar_x + 50, binfo->scrny - 110 - bar_bottom, 0, CY);

                    putfonts8_asc(sht_back->buf, sht_back->bxsize, bar_x + 140, binfo->scrny - 130 - bar_bottom, 0, "ZZULI CS 1801");
                    sheet_refresh(sht_back, bar_x, binfo->scrny - 200 - bar_bottom, bar_x + 300, binfo->scrny - 50 - bar_bottom);
                }else{
                    radius_box_fill(sht_back->buf, sht_back->bxsize, GET_COLOR(0xC3, 0x52, 0x30), bar_x, binfo->scrny - 200 - bar_bottom, bar_x + 300, binfo->scrny - 60 - bar_bottom, 20);
                    sheet_refresh(sht_back, bar_x, binfo->scrny - 200 - bar_bottom, bar_x + 300, binfo->scrny - 50 - bar_bottom);
                }


            } else if (768 <= i && i <= 1023) { 
                close_console(shtctl->sheets0 + (i - 768));
            } else if (1024 <= i && i <= 2023) {
                close_constask(taskctl->tasks0 + (i - 1024));
            } else if (2024 <= i && i <= 2279) {
                sht2 = shtctl->sheets0 + (i - 2024);
                memman_free_4k(memman, (int)sht2->buf, 256 * 165);
                sheet_free(sht2); 
            }
        }
    }
}

void keywin_off(struct SHEET* key_win)
{
    change_wtitle8(key_win, 0);
    if ((key_win->flags & 0x20) != 0) {
        fifo32_put(&key_win->task->fifo, 3); 
    }
    return;
}

void keywin_on(struct SHEET* key_win)
{
    change_wtitle8(key_win, 1);
    if ((key_win->flags & 0x20) != 0) {
        fifo32_put(&key_win->task->fifo, 2); 
    }
    return;
}

struct TASK* open_constask(struct SHEET* sht, unsigned int memtotal)
{
    struct MEMMAN* memman = (struct MEMMAN*)MEMMAN_ADDR;
    struct TASK* task = task_alloc();
    int* cons_fifo = (int*)memman_alloc_4k(memman, 128 * 4);
    task->cons_stack = memman_alloc_4k(memman, 64 * 1024);
    task->tss.esp = task->cons_stack + 64 * 1024 - 12;
    task->tss.eip = (int)&console_task;
    task->tss.es = 1 * 8;
    task->tss.cs = 2 * 8;
    task->tss.ss = 1 * 8;
    task->tss.ds = 1 * 8;
    task->tss.fs = 1 * 8;
    task->tss.gs = 1 * 8;
    *((int*)(task->tss.esp + 4)) = (int)sht;
    *((int*)(task->tss.esp + 8)) = memtotal;
    task_run(task, 2, 2); 
    fifo32_init(&task->fifo, 128, cons_fifo, task);
    return task;
}

#define CONSOLE_W 512
#define CONSOLE_H 256

struct SHEET* open_console(struct SHTCTL* shtctl, unsigned int memtotal)
{
    struct MEMMAN* memman = (struct MEMMAN*)MEMMAN_ADDR;
    struct SHEET* sht = sheet_alloc(shtctl);
    unsigned char* buf = (unsigned char*)memman_alloc_4k(memman, CONSOLE_W * CONSOLE_H);
    sheet_setbuf(sht, buf, CONSOLE_W, CONSOLE_H, 255);
    make_window8(buf, CONSOLE_W, CONSOLE_H, "console", 0);
    make_textbox8(sht, 8, 28, CONSOLE_W - 16, CONSOLE_H - 56, COL8_000000);
    sht->task = open_constask(sht, memtotal);
    sht->flags |= 0x20; 
    return sht;
}

void close_constask(struct TASK* task)
{
    struct MEMMAN* memman = (struct MEMMAN*)MEMMAN_ADDR;
    task_sleep(task);
    memman_free_4k(memman, task->cons_stack, 64 * 1024);
    memman_free_4k(memman, (int)task->fifo.buf, 128 * 4);
    task->flags = 0; /* task_free(task); */
    return;
}

void close_console(struct SHEET* sht)
{
    struct MEMMAN* memman = (struct MEMMAN*)MEMMAN_ADDR;
    struct TASK* task = sht->task;
    memman_free_4k(memman, (int)sht->buf, CONSOLE_W * CONSOLE_H);
    sheet_free(sht);
    close_constask(task);
    return;
}
