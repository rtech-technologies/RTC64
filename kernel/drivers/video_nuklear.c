#include "video_nuklear.h"

static uint32_t nk_color_to_u32(struct nk_color c) {
    return (c.r << 16) | (c.g << 8) | c.b;
}

extern void draw_rect(uint32_t* fb, uint32_t pitch, int x, int y, int w, int h, uint32_t color);
extern void draw_rect_outline(uint32_t* fb, uint32_t pitch, int x, int y, int w, int h, uint32_t color);
extern void draw_text(uint32_t* fb, uint32_t pitch, int x, int y, const char* text, int len, uint32_t color);

void nk_software_render(struct nk_context* ctx, uint32_t* fb, uint32_t width, uint32_t height, uint32_t pitch) {
    (void)width; (void)height;
    const struct nk_command *cmd;
    nk_foreach(cmd, ctx) {
        switch (cmd->type) {
            case NK_COMMAND_RECT: {
                const struct nk_command_rect *r = (const struct nk_command_rect *)cmd;
                draw_rect_outline(fb, pitch, r->x, r->y, r->w, r->h, nk_color_to_u32(r->color));
            } break;
            case NK_COMMAND_RECT_FILLED: {
                const struct nk_command_rect_filled *r = (const struct nk_command_rect_filled *)cmd;
                draw_rect(fb, pitch, r->x, r->y, r->w, r->h, nk_color_to_u32(r->color));
            } break;
            case NK_COMMAND_TEXT: {
                const struct nk_command_text *t = (const struct nk_command_text *)cmd;
                draw_text(fb, pitch, t->x, t->y, (const char*)t->string, t->length, nk_color_to_u32(t->foreground));
            } break;
            default: break;
        }
    }
    nk_clear(ctx);
}
