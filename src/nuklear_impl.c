/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

/* STB Truetype configuration to avoid undefined STBTT_acos */
#define STBTT_acos acos
#define STBTT_cos cos
#define STBTT_sin sin
#define STBTT_pow pow
#define STBTT_sqrt sqrt
#define STBTT_fmod fmod
#define STBTT_fabs fabs

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#include "nuklear.h"
#define NK_RAWFB_IMPLEMENTATION
#include "nuklear_rawfb.h"

struct nk_context* nk_rawfb_get_ctx(struct rawfb_context* rawfb) {
    return &rawfb->ctx;
}
