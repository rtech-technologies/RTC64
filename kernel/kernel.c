#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "pro_os.h"
#include "limine.h"
#include "app_ui.h"
#include "nk_software_renderer.h"
#include "serial.h"
__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);
__attribute__((used, section(".limine_requests"))) volatile struct limine_framebuffer_request framebuffer_request = { .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0 };
__attribute__((used, section(".limine_requests"))) static volatile struct limine_hhdm_request hhdm_request = { .id = LIMINE_HHDM_REQUEST, .revision = 0 };
__attribute__((used, section(".limine_requests"))) static volatile struct limine_memmap_request memmap_request = { .id = LIMINE_MEMMAP_REQUEST, .revision = 0 };
__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;
uint64_t hhdm_offset = 0;
static uint8_t kernel_stack[65536] __attribute__((aligned(16)));
struct limine_framebuffer *primary_fb;
struct nk_context nk_ctx;
struct app_state os_app;
static float font_get_width(nk_handle handle, float height, const char *text, int len) { (void)handle; (void)height; (void)text; return (float)len * 8.0f; }
static const uint8_t cursor_bitmap[12] = { 0b10000000, 0b11000000, 0b11100000, 0b11110000, 0b11111000, 0b11111100, 0b11111110, 0b11110000, 0b11011000, 0b10001100, 0b00001100, 0b00000000 };
void draw_cursor(tgx_canvas_t *canvas, int x, int y) { for (int i = 0; i < 12; i++) { for (int j = 0; j < 8; j++) { if (cursor_bitmap[i] & (0x80 >> j)) { tgx_blit_rect(canvas, x + j, y + i, 1, 1, 0x00FFFF); } } } }
void init_sse(void) { uint64_t cr0, cr4; __asm__ volatile("mov %%cr0, %0" : "=r"(cr0)); cr0 &= ~(1 << 2); cr0 |= (1 << 1); __asm__ volatile("mov %0, %%cr0" : : "r"(cr0)); __asm__ volatile("mov %%cr4, %0" : "=r"(cr4)); cr4 |= (3 << 9); __asm__ volatile("mov %0, %%cr4" : : "r"(cr4)); }
void environment_manager_entry(void* arg) {
    (void)arg; struct nk_user_font font; font.userdata = nk_handle_ptr(0); font.height = 8.0f; font.width = font_get_width; nk_init_default(&nk_ctx, &font); ui_init_style(&nk_ctx);
    void* virt_fb_addr = (void*)((uint64_t)primary_fb->address + hhdm_offset);
    tgx_canvas_t canvas = { (uint32_t*)virt_fb_addr, (int)primary_fb->width, (int)primary_fb->height, (int)primary_fb->pitch };
    int cursor_x = (int)primary_fb->width / 2, cursor_y = (int)primary_fb->height / 2;
    while (1) {
        tgx_clear(&canvas, 0x001010); hal_usb_poll(); input_event_t ev; nk_input_begin(&nk_ctx);
        while (hal_input_pop_event(&ev)) {
            if (ev.type == INPUT_TYPE_MOUSE) { cursor_x = ev.mouse.x; cursor_y = ev.mouse.y; nk_input_motion(&nk_ctx, cursor_x, cursor_y); nk_input_button(&nk_ctx, NK_BUTTON_LEFT, cursor_x, cursor_y, (ev.mouse.buttons & 1)); }
            else if (ev.type == INPUT_TYPE_KEYBOARD) { if (ev.kbd.down) { if (ev.kbd.key >= 32 && ev.kbd.key <= 126) nk_input_unicode(&nk_ctx, (nk_rune)ev.kbd.key); if (ev.kbd.key == 0x0A || ev.kbd.key == 0x0D) nk_input_key(&nk_ctx, NK_KEY_ENTER, 1); if (ev.kbd.key == 0x08) nk_input_key(&nk_ctx, NK_KEY_BACKSPACE, 1); } else { if (ev.kbd.key == 0x0A || ev.kbd.key == 0x0D) nk_input_key(&nk_ctx, NK_KEY_ENTER, 0); if (ev.kbd.key == 0x08) nk_input_key(&nk_ctx, NK_KEY_BACKSPACE, 0); } }
        }
        nk_input_end(&nk_ctx); ui_render(&nk_ctx, &os_app, (int)primary_fb->width, (int)primary_fb->height);
        struct nk_sw_fb sw_fb = { virt_fb_addr, (unsigned int)primary_fb->width, (unsigned int)primary_fb->height, (unsigned int)primary_fb->pitch };
        nk_sw_render(&sw_fb, &nk_ctx); draw_cursor(&canvas, cursor_x, cursor_y); scheduler_yield();
    }
}
void session_manager_task(void* arg) { (void)arg; memset(&os_app, 0, sizeof(os_app)); os_app.current_state = STATE_DESKTOP; os_app.show_terminal = 1; scheduler_spawn("Environment Manager", environment_manager_entry, NULL); while(1) { scheduler_yield(); } }
void kernel_main(void) {
    __asm__ volatile("cli\n\tmovq %0, %%rsp\n\tmovq %%rsp, %%rbp": : "r"(&kernel_stack[65536]) : "memory");
    serial_init();
    if (!framebuffer_request.response || !hhdm_request.response || !memmap_request.response) while(1) __asm__("hlt");
    hhdm_offset = hhdm_request.response->offset; primary_fb = framebuffer_request.response->framebuffers[0]; init_sse(); pmm_init(memmap_request.response);
    void* heap_phys = pmm_alloc_blocks(4096); hal_malloc_init((void*)((uint64_t)heap_phys + hhdm_offset), 4096 * 4096);
    gdt_init(); idt_init(); apic_init(); irq_install_handler(32, (void*)timer_handler); hal_input_init(); cm_orchestrate_drivers(); scheduler_init();
    __asm__ volatile("sti"); vfs_init(); vfs_refresh_mounts(); hal_storage_init(); hal_storage_finish_init(); system_shell_init();
    scheduler_add_task("System Shell", (void*)system_shell_task, NULL, 1, 1); scheduler_add_task("COMPREC", (void*)comprec_task, NULL, 1, 1); apic_timer_unmask();
    scheduler_add_task("SMSS", session_manager_task, NULL, 1, 1);
    while (1) { scheduler_run(); __asm__ volatile("hlt"); }
}
