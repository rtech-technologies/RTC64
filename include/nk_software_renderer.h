#ifndef NK_SOFTWARE_RENDERER_H
#define NK_SOFTWARE_RENDERER_H

#include <stdint.h>
#include "nuklear.h"

struct nk_sw_fb {
    void *pixels;
    int width;
    int height;
    int pitch; // in bytes
};

void nk_sw_render(struct nk_sw_fb *fb, struct nk_context *ctx);

#endif
