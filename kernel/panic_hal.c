/* Modified by Sovereign: License Compliance Update */
#include "pro_os.h"

/* Hardware bridge for graphical panic reporting */

struct panic_framebuffer* get_kernel_framebuffer(void) {
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        return NULL;
    }
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    static struct panic_framebuffer pfb;
    pfb.address = (uint64_t)fb->address;
    pfb.width = fb->width;
    pfb.height = fb->height;
    pfb.pitch = fb->pitch;
    return &pfb;
}
