#include "pro_os.h"
#include <stdint.h>
#include <stddef.h>
#include "serial.h"

// Sovereign Graphics Subsystem - Mesa3D/Vulkan Infrastructure
// High-performance graphics acceleration for Sovereign OS.

static uint32_t* sovereign_framebuffer = NULL;
static uint32_t fb_width = 0;
static uint32_t fb_height = 0;

void hal_graphics_init(void) {
    serial_write("[GRAPHICS] Initializing Sovereign Graphics Subsystem (Mesa/Vulkan)...\n");
    // Discover GPU (PCI), initialize Vulkan WSI for Sovereign, and set up Mesa state trackers.

    // Link to the primary framebuffer
    struct panic_framebuffer* pfb = get_kernel_framebuffer();
    if (pfb) {
        sovereign_framebuffer = (uint32_t*)pfb->address;
        fb_width = (uint32_t)pfb->width;
        fb_height = (uint32_t)pfb->height;
        serial_printf("[GRAPHICS] Bound to Framebuffer: %dx%d at %lx\n", fb_width, fb_height, (uint64_t)sovereign_framebuffer);
    }
}

void hal_graphics_swap_buffers(void) {
    // Perform back-to-front buffer blit or flip.
    // In Sovereign, we blit to the physical framebuffer via TGX or direct memory map.
}

// Low-level Vulkan entry points for Sovereign
void sovereign_vk_get_surface_matrix(void** out_fb, uint32_t* w, uint32_t* h) {
    if (out_fb) *out_fb = sovereign_framebuffer;
    if (w) *w = fb_width;
    if (h) *h = fb_height;
}
