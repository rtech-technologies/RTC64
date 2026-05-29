#include "nk_software_renderer.h"
#include "external/tgx.h"
#include <string.h>
#include <stdbool.h>
#include "drivers/font_8x8.h"

void draw_text_tgx(tgx_canvas_t *canvas, int x, int y, const char *text, int len, struct nk_color col) {
    uint32_t c = (col.r << 16) | (col.g << 8) | col.b;
    for (int i = 0; i < len; i++) {
        uint8_t idx = (uint8_t)text[i];
        if (idx > 127) idx = '?';
        for (int row = 0; row < 8; row++) {
            uint8_t bits = font_8x8_data[idx][row];
            for (int bit = 0; row < 8 && bit < 8; bit++) {
                if (bits & (1 << bit)) {
                    tgx_blit_rect(canvas, x + i * 8 + bit, y + row, 1, 1, c);
                }
            }
        }
    }
}

void nk_sw_render(struct nk_sw_fb *fb, struct nk_context *ctx) {
    tgx_canvas_t canvas = { (uint32_t*)fb->pixels, fb->width, fb->height, fb->pitch };
    const struct nk_command *cmd;

    nk_foreach(cmd, ctx) {
        uint32_t col;
        switch (cmd->type) {
            case NK_COMMAND_RECT_FILLED: {
                const struct nk_command_rect_filled *r = (const struct nk_command_rect_filled*)cmd;
                col = (r->color.r << 16) | (r->color.g << 8) | r->color.b;
                tgx_blit_rect(&canvas, r->x, r->y, r->w, r->h, col);
            } break;
            case NK_COMMAND_RECT: {
                const struct nk_command_rect *r = (const struct nk_command_rect*)cmd;
                col = (r->color.r << 16) | (r->color.g << 8) | r->color.b;
                tgx_blit_rect(&canvas, r->x, r->y, r->w, 1, col);
                tgx_blit_rect(&canvas, r->x, r->y + r->h - 1, r->w, 1, col);
                tgx_blit_rect(&canvas, r->x, r->y, 1, r->h, col);
                tgx_blit_rect(&canvas, r->x + r->w - 1, r->y, 1, r->h, col);
            } break;
            case NK_COMMAND_TEXT: {
                const struct nk_command_text *t = (const struct nk_command_text*)cmd;
                draw_text_tgx(&canvas, t->x, t->y, (const char*)t->string, t->length, t->foreground);
            } break;
            case NK_COMMAND_LINE: {
                const struct nk_command_line *l = (const struct nk_command_line*)cmd;
                col = (l->color.r << 16) | (l->color.g << 8) | l->color.b;
                // Simplified line using rects for now to ensure TGX connection
                int dx = l->end.x - l->begin.x;
                int dy = l->end.y - l->begin.y;
                if (dx == 0) tgx_blit_rect(&canvas, l->begin.x, l->begin.y, 1, dy > 0 ? dy : -dy, col);
                else if (dy == 0) tgx_blit_rect(&canvas, l->begin.x, l->begin.y, dx > 0 ? dx : -dx, 1, col);
            } break;
            default: break;
        }
    }
    nk_clear(ctx);
}
