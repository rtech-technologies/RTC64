#include <pro_os.h>
#include <limine.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <external/stb_truetype.h>

extern struct limine_framebuffer *fb;
static stbtt_fontinfo font_info;
static uint8_t* font_data = NULL;

void typography_init(void* ttf_buffer) {
    font_data = (uint8_t*)ttf_buffer;
    stbtt_InitFont(&font_info, font_data, 0);
}

void typography_draw_text(const char* text, int x, int y, uint32_t color) {
    if (!fb || !fb->address || !font_data) return;

    float scale = stbtt_ScaleForPixelHeight(&font_info, 16.0);
    int x_off = x;

    while (*text) {
        int width, height, xoff, yoff;
        uint8_t* bitmap = stbtt_GetCodepointBitmap(&font_info, 0, scale, *text, &width, &height, &xoff, &yoff);

        if (bitmap) {
            uint32_t* screen = (uint32_t*)fb->address;
            for (int py = 0; py < height; py++) {
                for (int px = 0; px < width; px++) {
                    int sx = x_off + xoff + px;
                    int sy = y + yoff + py;
                    if (sx >= 0 && sx < (int)fb->width && sy >= 0 && sy < (int)fb->height) {
                        uint8_t alpha = bitmap[py * width + px];
                        if (alpha > 128) {
                             screen[sy * (fb->pitch/4) + sx] = color;
                        }
                    }
                }
            }
            int advance, lsb;
            stbtt_GetCodepointHMetrics(&font_info, *text, &advance, &lsb);
            x_off += (int)(advance * scale);
            stbtt_FreeBitmap(bitmap, NULL);
        }
        text++;
    }
}
