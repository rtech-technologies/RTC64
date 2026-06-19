/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"
#include "nk_software_renderer.h"

#include "app_ui.h"
#include "services.h"

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

static float font_get_width(nk_handle handle, float height, const char *text, int len) {
    (void)handle; (void)height; (void)text;
    return (float)len * 8.0f;
}

service_table_t g_services = { NULL, NULL, NULL, NULL };

/* Sovereign services to show how it works */
static int sovereign_init(void) { return 0; }
static void sovereign_storage_work(void) { printf("Storage work performed!\n"); }
service_t sovereign_storage = { "Sovereign Storage", sovereign_init, NULL, sovereign_storage_work };

int main(int argc, char* argv[])
{
    SDL_Window *win;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    int width, height;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) return -1;

    win = SDL_CreateWindow("R-TECH™ (Software Rendered)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!win) return -1;

    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);

    struct nk_context ctx;
    struct nk_user_font font;
    font.userdata = nk_handle_ptr(0);
    font.height = 8.0f;
    font.width = font_get_width;

    nk_init_default(&ctx, &font);
    ui_init_style(&ctx);

    struct app_state app;
    memset(&app, 0, sizeof(app));
    app.current_state = STATE_LOGIN;

    /* "Plug in" the sovereign storage service */
    g_services.storage = &sovereign_storage;

    int running = 1;
    while (running)
    {
        SDL_Event evt;
        nk_input_begin(&ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) running = 0;
            if (evt.type == SDL_MOUSEMOTION) {
                nk_input_motion(&ctx, evt.motion.x, evt.motion.y);
            } else if (evt.type == SDL_MOUSEBUTTONDOWN || evt.type == SDL_MOUSEBUTTONUP) {
                int down = (evt.type == SDL_MOUSEBUTTONDOWN);
                if (evt.button.button == SDL_BUTTON_LEFT) nk_input_button(&ctx, NK_BUTTON_LEFT, evt.button.x, evt.button.y, down);
            }
        }
        nk_input_end(&ctx);

        SDL_GetWindowSize(win, &width, &height);
        ui_render(&ctx, &app, width, height);

        void *pixels;
        int pitch;
        SDL_LockTexture(texture, NULL, &pixels, &pitch);
        memset(pixels, 0x05, pitch * height); // Clear screen

        struct nk_sw_fb fb = { pixels, width, height, pitch };
        nk_sw_render(&fb, &ctx);

        SDL_UnlockTexture(texture);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
