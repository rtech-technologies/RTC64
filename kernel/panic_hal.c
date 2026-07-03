/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"

struct panic_framebuffer {
    uint64_t address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
};

/* External from kernel.c */
extern volatile struct limine_framebuffer_request framebuffer_request;

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
