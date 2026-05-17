#include "nk_software_renderer.h"
#include <string.h>

/* Very direct 8x8 font integrated into the renderer */
#include "../kernel/drivers/font_8x8.h"

static void draw_pixel(struct nk_sw_fb *fb, int x, int y, struct nk_color col) {
    if (x < 0 || y < 0 || x >= fb->width || y >= fb->height) return;
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
                /* Outline - very direct */
                for(int i=r->x; i<r->x+r->w; i++) { draw_pixel(fb, i, r->y, r->color); draw_pixel(fb, i, r->y+r->h-1, r->color); }
                for(int i=r->y; i<r->y+r->h; i++) { draw_pixel(fb, r->x, i, r->color); draw_pixel(fb, r->x+r->w-1, i, r->color); }
            } break;
            case NK_COMMAND_LINE: {
                const struct nk_command_line *l = (const struct nk_command_line*)cmd;
                draw_line(fb, l->begin.x, l->begin.y, l->end.x, l->end.y, l->color);
            } break;
            case NK_COMMAND_TRIANGLE_FILLED: {
                const struct nk_command_triangle_filled *t = (const struct nk_command_triangle_filled*)cmd;
                /* Simplified: draw bounding box for triangle to ensure visual presence */
                int min_x = t->a.x; if(t->b.x < min_x) min_x = t->b.x; if(t->c.x < min_x) min_x = t->c.x;
                int max_x = t->a.x; if(t->b.x > max_x) max_x = t->b.x; if(t->c.x > max_x) max_x = t->c.x;
                int min_y = t->a.y; if(t->b.y < min_y) min_y = t->b.y; if(t->c.y < min_y) min_y = t->c.y;
                int max_y = t->a.y; if(t->b.y > max_y) max_y = t->b.y; if(t->c.y > max_y) max_y = t->c.y;
                draw_rect(fb, min_x, min_y, max_x - min_x, max_y - min_y, t->color);
            } break;
            default: break;
        }
    }
    nk_clear(ctx);
}
