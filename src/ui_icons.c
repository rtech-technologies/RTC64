/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include "external/nanosvg.h"
#include "external/nanosvgrast.h"
#include "nuklear.h"
#include "serial.h"

struct svg_icon {
    char name[32];
    NSVGimage *image;
    struct nk_image nk_img;
    void* pixels;
    int w, h;
};

#define MAX_ICONS 32
static struct svg_icon g_icons[MAX_ICONS];
static int g_icon_count = 0;
static NSVGrasterizer *g_rasterizer = NULL;

void ui_icon_init(void) {
    if (!g_rasterizer) {
        g_rasterizer = nsvgCreateRasterizer();
    }
}

struct nk_image ui_icon_load_svg(const char* name, const char* path, int w, int h) {
    for (int i = 0; i < g_icon_count; i++) {
        if (strcmp(g_icons[i].name, name) == 0) return g_icons[i].nk_img;
    }

    if (g_icon_count >= MAX_ICONS) return nk_image_id(0);

    /* Load file into memory */
    char* buf = malloc(65536); /* Assume icons are small */
    if (!buf) return nk_image_id(0);

    int read = vfs_read(path, buf, 65535);
    if (read <= 0) {
        free(buf);
        return nk_image_id(0);
    }
    buf[read] = '\0';

    NSVGimage *image = nsvgParse(buf, "px", 96.0f);
    free(buf);
    if (!image) return nk_image_id(0);

    void* pixels = malloc(w * h * 4);
    if (!pixels) {
        nsvgDelete(image);
        return nk_image_id(0);
    }

    float scale = (float)w / image->width;
    nsvgRasterize(g_rasterizer, image, 0, 0, scale, pixels, w, h, w * 4);

    struct svg_icon *icon = &g_icons[g_icon_count++];
    strncpy(icon->name, name, 31);
    icon->image = image;
    icon->pixels = pixels;
    icon->w = w;
    icon->h = h;

    /* In our RawFB implementation, nk_image handles it via handle.ptr */
    icon->nk_img = nk_image_ptr(pixels);
    icon->nk_img.w = (unsigned short)w;
    icon->nk_img.h = (unsigned short)h;

    serial_printf("[UI] Loaded SVG icon: %s (%dx%d)\n", name, w, h);
    return icon->nk_img;
}
