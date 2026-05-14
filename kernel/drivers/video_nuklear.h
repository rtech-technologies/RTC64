#ifndef VIDEO_NUKLEAR_H
#define VIDEO_NUKLEAR_H

#include "nuklear.h"
#include <stdint.h>

void nk_software_render(struct nk_context* ctx, uint32_t* fb, uint32_t width, uint32_t height, uint32_t pitch);

#endif
