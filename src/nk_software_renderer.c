#include "nk_software_renderer.h"
#include <string.h>

/* Very simple 8x8 font integrated into the renderer */
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
                /* Outline - very simple */
                for(int i=r->x; i<r->x+r->w; i++) { draw_pixel(fb, i, r->y, r->color); draw_pixel(fb, i, r->y+r->h-1, r->color); }
                for(int i=r->y; i<r->y+r->h; i++) { draw_pixel(fb, r->x, i, r->color); draw_pixel(fb, r->x+r->w-1, i, r->color); }
            } break;
            default: break;
        }
    }
    nk_clear(ctx);
}
