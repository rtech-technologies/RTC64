/* Modified by Sovereign: Professional Software Renderer with Image support */
#ifndef NK_SOFTWARE_RENDERER_H
#define NK_SOFTWARE_RENDERER_H

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"

struct nk_sw_fb {
    void *pixels;
    unsigned int width;
    unsigned int height;
    unsigned int pitch;
};

void nk_sw_render(struct nk_sw_fb *fb, struct nk_context *ctx);

#endif
