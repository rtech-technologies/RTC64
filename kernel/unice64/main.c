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
    if (x >= fb->width || y >= fb->height) return;
    ((uint32_t*)fb->address)[y * (fb->pitch/4) + x] = color;
}

void draw_nk_commands(struct nk_context *ctx) {
    if (!fb || !fb->address) return;
    const struct nk_command *cmd;
    nk_foreach(cmd, ctx) {
        if (cmd->type == NK_COMMAND_RECT_FILLED) {
             const struct nk_command_rect_filled *r = (const struct nk_command_rect_filled*)cmd;
             uint32_t col = (r->color.a << 24) | (r->color.r << 16) | (r->color.g << 8) | r->color.b;
             for (uint32_t y = 0; y < r->h; y++) {
                for (uint32_t x = 0; x < r->w; x++) draw_pixel(r->x + x, r->y + y, col);
             }
        }
        // Additional commands...
    }
    nk_clear(ctx);
}

void gui_main_loop() {
    for (;;) {
        // Handle input events...

        if (nk_begin(&ctx, "RTECH OSx2 Dashboard", nk_rect(50, 50, 400, 400),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE)) {
            nk_layout_row_dynamic(&ctx, 30, 1);
            nk_label(&ctx, "Sovereign Core Active", NK_TEXT_LEFT);
            if (nk_button_label(&ctx, "Execute Lockdown")) {
                extern void quartermaster_panic(const char* msg);
                quartermaster_panic("User initiated lockdown.");
            }
        }
        nk_end(&ctx);

        draw_nk_commands(&ctx);
    }
}

void _start(void) {
    extern void gdt_init(void);
    gdt_init();
    extern void syscall_init(void);
    syscall_init();

    register_service(vga_serial_service);
    dispatch_event(EVENT_INIT);

    // lwIP / wolfSSL init would go here

    extern struct limine_module_response *get_modules(void);
    struct limine_module_response *mod_res = get_modules();
    if (mod_res && mod_res->module_count > 0) {
        extern void typography_init(void* ttf_buffer);
        typography_init(mod_res->modules[0]->address);
    }

    void *nk_memory = malloc(1 * 1024 * 1024);
    if (nk_memory) nk_init_fixed(&ctx, nk_memory, 1 * 1024 * 1024, &font);

    dispatch_event(EVENT_MAIN);

    // Drop to Ring 3 and hand off to shell (or GUI if user preferred)
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
