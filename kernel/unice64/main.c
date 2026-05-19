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

static void draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!fb || x >= fb->width || y >= fb->height) return;
    ((uint32_t*)fb->address)[y * (fb->pitch/4) + x] = color;
}

static void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = NK_ABS(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = NK_ABS(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;
    for (;;) {
        draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

void draw_nk_commands(struct nk_context *ctx) {
    if (!fb || !fb->address) return;
    const struct nk_command *cmd;
    nk_foreach(cmd, ctx) {
        switch (cmd->type) {
            case NK_COMMAND_RECT_FILLED: {
                const struct nk_command_rect_filled *r = (const struct nk_command_rect_filled*)cmd;
                uint32_t col = (r->color.a << 24) | (r->color.r << 16) | (r->color.g << 8) | r->color.b;
                for (uint32_t y = 0; y < r->h; y++) {
                    for (uint32_t x = 0; x < r->w; x++) draw_pixel(r->x + x, r->y + y, col);
                }
            } break;
            case NK_COMMAND_LINE: {
                const struct nk_command_line *l = (const struct nk_command_line*)cmd;
                uint32_t col = (l->color.a << 24) | (l->color.r << 16) | (l->color.g << 8) | l->color.b;
                draw_line(l->begin.x, l->begin.y, l->end.x, l->end.y, col);
            } break;
            case NK_COMMAND_TRIANGLE_FILLED: {
                const struct nk_command_triangle_filled *t = (const struct nk_command_triangle_filled*)cmd;
                uint32_t col = (t->color.a << 24) | (t->color.r << 16) | (t->color.g << 8) | t->color.b;
                int min_x = NK_MIN(t->a.x, NK_MIN(t->b.x, t->c.x));
                int max_x = NK_MAX(t->a.x, NK_MAX(t->b.x, t->c.x));
                int min_y = NK_MIN(t->a.y, NK_MIN(t->b.y, t->c.y));
                int max_y = NK_MAX(t->a.y, NK_MAX(t->b.y, t->c.y));
                for (int y = min_y; y <= max_y; y++) {
                    for (int x = min_x; x <= max_x; x++) draw_pixel(x, y, col);
                }
            } break;
            case NK_COMMAND_TEXT: {
                const struct nk_command_text *t = (const struct nk_command_text*)cmd;
                extern void typography_draw_text(const char* text, int x, int y, uint32_t color);
                uint32_t col = (t->foreground.a << 24) | (t->foreground.r << 16) | (t->foreground.g << 8) | t->foreground.b;
                typography_draw_text((const char*)t->string, t->x, t->y, col);
            } break;
            default: break;
        }
    }
    nk_clear(ctx);
}

void _start(void) {
    extern void gdt_init(void);
    gdt_init();
    extern void syscall_init(void);
    syscall_init();
    extern void vga_serial_service(kernel_event_t event);
    register_service(vga_serial_service);
    dispatch_event(EVENT_INIT);

    extern void xhci_init(void);
    extern void ahci_init(void);
    xhci_init();
    ahci_init();

    extern void vfs_init(void);
    vfs_init();

    extern struct limine_module_response *get_modules(void);
    struct limine_module_response *mod_res = get_modules();
    if (mod_res && mod_res->module_count > 0) {
        extern void typography_init(void* ttf_buffer);
        typography_init(mod_res->modules[0]->address);
    }
    void *nk_memory = malloc(1 * 1024 * 1024);
    if (nk_memory) nk_init_fixed(&ctx, nk_memory, 1 * 1024 * 1024, &font);
    dispatch_event(EVENT_MAIN);

    extern void* paging_create_user_space(void);
    void* user_pml4 = paging_create_user_space();
    extern void paging_switch(void* pml4);
    paging_switch(user_pml4);

    __asm__ volatile (
        "mov $0x23, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%rsp, %%rax\n"
        "push $0x23\n"
        "push %%rax\n"
        "pushf\n"
        "push $0x1B\n"
        "push $shell_main\n"
        "iretq\n"
        : : : "rax", "memory"
    );

    for (;;) __asm__("hlt");
}
