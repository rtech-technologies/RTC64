/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "nk_software_renderer.h"
#include <string.h>
#include <stdbool.h>

/* Genuine Pro Software Rasterizer
 * Implements precise triangle rasterization with barycentric coordinate testing.
 */

#include "../kernel/drivers/font_8x8.h"

static void draw_pixel(struct nk_sw_fb *fb, int x, int y, struct nk_color col) {
    if (x < 0 || y < 0 || x >= (int)fb->width || y >= (int)fb->height) return;
    uint32_t *p = (uint32_t*)((uint8_t*)fb->pixels + y * fb->pitch + x * 4);
    *p = (col.r << 16) | (col.g << 8) | col.b;
}

static void draw_rect(struct nk_sw_fb *fb, int x, int y, int w, int h, struct nk_color col) {
    for (int j = y; j < y + h; j++) {
        for (int i = x; i < x + w; i++) {
            draw_pixel(fb, i, j, col);
        }
    }
}

static void draw_line(struct nk_sw_fb *fb, int x0, int y0, int x1, int y1, struct nk_color col) {
    int dx = (x1 - x0 > 0) ? (x1 - x0) : (x0 - x1);
    int dy = (y1 - y0 > 0) ? (y1 - y0) : (y0 - y1);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        draw_pixel(fb, x0, y0, col);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

static float cross_product(int ax, int ay, int bx, int by, int cx, int cy) {
    return (float)(bx - ax) * (float)(cy - ay) - (float)(by - ay) * (float)(cx - ax);
}

static void draw_triangle_filled(struct nk_sw_fb *fb, int x0, int y0, int x1, int y1, int x2, int y2, struct nk_color col) {
    // 1. Calculate Bounding Box
    int min_x = x0; if(x1 < min_x) min_x = x1; if(x2 < min_x) min_x = x2;
    int max_x = x0; if(x1 > max_x) max_x = x1; if(x2 > max_x) max_x = x2;
    int min_y = y0; if(y1 < min_y) min_y = y1; if(y2 < min_y) min_y = y2;
    int max_y = y0; if(y1 > max_y) max_y = y1; if(y2 > max_y) max_y = y2;

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x >= (int)fb->width) max_x = fb->width - 1;
    if (max_y >= (int)fb->height) max_y = fb->height - 1;

    // 2. Scanline Bounding Box
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            // Barycentric edge functions
            float d1 = cross_product(x0, y0, x1, y1, x, y);
            float d2 = cross_product(x1, y1, x2, y2, x, y);
            float d3 = cross_product(x2, y2, x0, y0, x, y);

            // Winding order check (handles both CW and CCW)
            bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
            bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

            if (!(has_neg && has_pos)) {
                draw_pixel(fb, x, y, col);
            }
        }
    }
}

static void draw_text(struct nk_sw_fb *fb, int x, int y, const char *text, int len, struct nk_color col) {
    for (int i = 0; i < len; i++) {
        uint8_t c = (uint8_t)text[i];
        if (c < 32 || c > 126) continue;
        int idx = c - 32;
        for (int row = 0; row < 8; row++) {
            for (int col_idx = 0; col_idx < 8; col_idx++) {
                if (font_8x8_data[idx][row] & (1 << col_idx)) {
                    draw_pixel(fb, x + i * 8 + col_idx, y + row, col);
                }
            }
        }
    }
}

void nk_sw_render(struct nk_sw_fb *fb, struct nk_context *ctx) {
    const struct nk_command *cmd;
    nk_foreach(cmd, ctx) {
        switch (cmd->type) {
            case NK_COMMAND_RECT_FILLED: {
                const struct nk_command_rect_filled *r = (const struct nk_command_rect_filled*)cmd;
                draw_rect(fb, r->x, r->y, r->w, r->h, r->color);
            } break;
            case NK_COMMAND_TEXT: {
                const struct nk_command_text *t = (const struct nk_command_text*)cmd;
                draw_text(fb, t->x, t->y, (const char*)t->string, t->length, t->foreground);
            } break;
            case NK_COMMAND_RECT: {
                const struct nk_command_rect *r = (const struct nk_command_rect*)cmd;
                for(int i=r->x; i<r->x+r->w; i++) { draw_pixel(fb, i, r->y, r->color); draw_pixel(fb, i, r->y+r->h-1, r->color); }
                for(int i=r->y; i<r->y+r->h; i++) { draw_pixel(fb, r->x, i, r->color); draw_pixel(fb, r->x+r->w-1, i, r->color); }
            } break;
            case NK_COMMAND_LINE: {
                const struct nk_command_line *l = (const struct nk_command_line*)cmd;
                draw_line(fb, l->begin.x, l->begin.y, l->end.x, l->end.y, l->color);
            } break;
            case NK_COMMAND_TRIANGLE_FILLED: {
                const struct nk_command_triangle_filled *t = (const struct nk_command_triangle_filled*)cmd;
                draw_triangle_filled(fb, t->a.x, t->a.y, t->b.x, t->b.y, t->c.x, t->c.y, t->color);
            } break;
            default: break;
        }
    }
    nk_clear(ctx);
}
