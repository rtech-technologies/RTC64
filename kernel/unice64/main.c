#include <pro_os.h>

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_PRIVATE
#include <nuklear.h>

// Explicitly export these for other modules
void nk_input_begin_wrap(struct nk_context *ctx) { nk_input_begin(ctx); }
void nk_input_char_wrap(struct nk_context *ctx, char c) { nk_input_char(ctx, c); }
void nk_input_end_wrap(struct nk_context *ctx) { nk_input_end(ctx); }

// Global context for the kernel GUI
struct nk_context ctx;
struct nk_user_font font;

// Graphics plumbing: Software cursor and draw command dispatcher
void draw_nk_commands(struct nk_context *ctx) {
    const struct nk_command *cmd;
    nk_foreach(cmd, ctx) {
        // Here we map NK_COMMAND_RECT, NK_COMMAND_TEXT etc to the GOP Framebuffer
    }
    nk_clear(ctx);
}

void _start(void) {
    // 1. Initial services registration
    register_service(vga_serial_service);

    // 2. Dispatch INIT
    dispatch_event(EVENT_INIT);

    // 3. Initialize Nuklear
    extern void* bump_alloc(size_t size);
    void *nk_memory = bump_alloc(1 * 1024 * 1024);
    if (nk_memory) {
        nk_init_fixed(&ctx, nk_memory, 1 * 1024 * 1024, &font);
    }

    // 4. Dispatch MAIN
    dispatch_event(EVENT_MAIN);

    // 5. Hand off to shell
    shell_main();

    // Lockdown
    for (;;) {
        __asm__("hlt");
    }
}
