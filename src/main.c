#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl_gl3.h"

/* CherryUSB headers (Skeleton integration) */
#include "usbd_core.h"
#include "usbh_core.h"

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

/* Application States */
enum {
    STATE_LOGIN,
    STATE_INSTALLER,
    STATE_MAIN
};

struct app_state {
    int current_state;
    char username[64];
    char password[64];
    int progress;
    int install_started;
};

static void
ui_login(struct nk_context *ctx, struct app_state *app)
{
    struct nk_panel layout;
    if (nk_begin(ctx, "Login", nk_rect(WINDOW_WIDTH/2 - 150, WINDOW_HEIGHT/2 - 100, 300, 200),
        NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "R-TECH SYSTEM LOGIN", NK_TEXT_CENTERED);

        nk_layout_row_dynamic(ctx, 30, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->username, sizeof(app->username), nk_filter_default);

        nk_layout_row_dynamic(ctx, 30, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->password, sizeof(app->password), nk_filter_default);

        nk_layout_row_dynamic(ctx, 30, 1);
        if (nk_button_label(ctx, "Login")) {
            app->current_state = STATE_INSTALLER;
        }
    }
    nk_end(ctx);
}

static void
ui_installer(struct nk_context *ctx, struct app_state *app)
{
    if (nk_begin(ctx, "Installer", nk_rect(WINDOW_WIDTH/2 - 200, WINDOW_HEIGHT/2 - 150, 400, 300),
        NK_WINDOW_BORDER|NK_WINDOW_TITLE))
    {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "Welcome to R-TECH Installer", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 150, 1);
        nk_label_wrap(ctx, "This will install the non-functional features to your system. Please press 'Start' to begin the simulation.");

        if (app->install_started) {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_size prog = (nk_size)app->progress;
            nk_progress(ctx, &prog, 100, NK_MODIFIABLE);
            if (app->progress < 100) app->progress++;
            else {
                nk_label(ctx, "Installation Complete!", NK_TEXT_CENTERED);
                if (nk_button_label(ctx, "Finish")) {
                    app->current_state = STATE_MAIN;
                }
            }
        } else {
            nk_layout_row_dynamic(ctx, 30, 1);
            if (nk_button_label(ctx, "Start Installation")) {
                app->install_started = 1;
            }
        }
    }
    nk_end(ctx);
}

static void
ui_main(struct nk_context *ctx, struct app_state *app)
{
    if (nk_begin(ctx, "R-TECH Console", nk_rect(10, 10, WINDOW_WIDTH-20, WINDOW_HEIGHT-20),
        NK_WINDOW_BORDER|NK_WINDOW_TITLE))
    {
        nk_layout_row_dynamic(ctx, 30, 2);
        nk_label(ctx, "USB Status: Simulated Connected", NK_TEXT_LEFT);
        nk_label(ctx, "CherryUSB Stack: Initialized (Dummy)", NK_TEXT_RIGHT);

        nk_layout_row_static(ctx, 100, 100, 4);
        for (int i = 0; i < 8; ++i) {
            char buf[32];
            sprintf(buf, "Module %d", i);
            if (nk_button_label(ctx, buf)) {
                /* No features */
            }
        }
    }
    nk_end(ctx);
}

int main(int argc, char* argv[])
{
    /* Platform */
    SDL_Window *win;
    SDL_GLContext glContext;
    int width = 0, height = 0;

    /* SDL setup */
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    win = SDL_CreateWindow("R-TECH Nuklear CherryUSB",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);

    if (!win) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    glContext = SDL_GL_CreateContext(win);
    SDL_GetWindowSize(win, &width, &height);

    /* GUI */
    struct nk_context *ctx;
    ctx = nk_sdl_init(win);
    {
        struct nk_font_atlas *atlas;
        nk_sdl_font_stash_begin(&atlas);
        nk_sdl_font_stash_end();
    }

    /* Application state */
    struct app_state app;
    memset(&app, 0, sizeof(app));
    app.current_state = STATE_LOGIN;
    strcpy(app.username, "admin");
    strcpy(app.password, "password");

    /* CherryUSB Dummy Init */
    // usbd_init(); // Would need a real controller for actual init
    // usbh_initialize();

    int running = 1;
    while (running)
    {
        /* Input */
        SDL_Event evt;
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) running = 0;
            nk_sdl_handle_event(&evt);
        }
        nk_input_end(ctx);

        /* GUI */
        switch (app.current_state) {
            case STATE_LOGIN: ui_login(ctx, &app); break;
            case STATE_INSTALLER: ui_installer(ctx, &app); break;
            case STATE_MAIN: ui_main(ctx, &app); break;
        }

        /* Draw */
        SDL_GetWindowSize(win, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
        SDL_GL_SwapWindow(win);
    }

    nk_sdl_shutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
