#include <pro_os.h>

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_PRIVATE
#include <nuklear.h>

void nk_input_begin_wrap(struct nk_context *ctx) { nk_input_begin(ctx); }
void nk_input_char_wrap(struct nk_context *ctx, char c) { nk_input_char(ctx, c); }
void nk_input_end_wrap(struct nk_context *ctx) { nk_input_end(ctx); }

struct nk_context ctx;
struct nk_user_font font;
extern struct limine_framebuffer *fb;

// Direct Framebuffer Graphics Plumbing
void draw_nk_commands(struct nk_context *ctx) {
    if (!fb || !fb->address) return;
    uint32_t *pixels = (uint32_t*)fb->address;

    const struct nk_command *cmd;
    nk_foreach(cmd, ctx) {
        if (cmd->type == NK_COMMAND_RECT_FILLED) {
             const struct nk_command_rect_filled *r = (const struct nk_command_rect_filled*)cmd;
             for (uint32_t y = 0; y < r->h; y++) {
                for (uint32_t x = 0; x < r->w; x++) {
                    uint32_t px = r->x + x;
                    uint32_t py = r->y + y;
                    if (px < fb->width && py < fb->height) {
                        pixels[py * (fb->pitch/4) + px] = (r->color.a << 24) | (r->color.r << 16) | (r->color.g << 8) | r->color.b;
                    }
                }
            }
        }
    }
    nk_clear(ctx);
}

void _start(void) {
    register_service(vga_serial_service);
    dispatch_event(EVENT_INIT);

    extern void* bump_alloc(size_t size);
    void *nk_memory = bump_alloc(1 * 1024 * 1024);
    if (nk_memory) {
        nk_init_fixed(&ctx, nk_memory, 1 * 1024 * 1024, &font);
    }

    dispatch_event(EVENT_MAIN);
    shell_main();

    for (;;) {
        __asm__("hlt");
    }
}
