#include <string.h>
#include <stdio.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"
#include "app_ui.h"
#include "services.h"
#include "hal.h"

void ui_init_style(struct nk_context *ctx)
{
    struct nk_color table[NK_COLOR_COUNT];
    table[NK_COLOR_TEXT] = nk_rgba(70, 200, 255, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(10, 15, 25, 255);
    table[NK_COLOR_HEADER] = nk_rgba(30, 35, 45, 255);
    table[NK_COLOR_BORDER] = nk_rgba(40, 80, 120, 255);
    table[NK_COLOR_BUTTON] = nk_rgba(25, 45, 75, 255);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgba(35, 65, 105, 255);
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(45, 85, 135, 255);
    table[NK_COLOR_TOGGLE] = nk_rgba(20, 30, 50, 255);
    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(70, 200, 255, 255);
    table[NK_COLOR_SELECT] = nk_rgba(25, 45, 75, 255);
    table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(70, 200, 255, 255);
    table[NK_COLOR_SLIDER] = nk_rgba(20, 30, 50, 255);
    table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(70, 200, 255, 255);
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(100, 220, 255, 255);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(150, 240, 255, 255);
    table[NK_COLOR_PROPERTY] = nk_rgba(20, 30, 50, 255);
    table[NK_COLOR_EDIT] = nk_rgba(15, 25, 40, 255);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(70, 200, 255, 255);
    table[NK_COLOR_COMBO] = nk_rgba(20, 30, 50, 255);
    table[NK_COLOR_CHART] = nk_rgba(20, 30, 50, 255);
    table[NK_COLOR_CHART_COLOR] = nk_rgba(70, 200, 255, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
    table[NK_COLOR_SCROLLBAR] = nk_rgba(15, 25, 40, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(40, 80, 120, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(50, 100, 150, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(60, 120, 180, 255);
    table[NK_COLOR_TAB_HEADER] = nk_rgba(20, 30, 50, 255);
    nk_style_from_table(ctx, table);
}

void ui_render(struct nk_context *ctx, struct app_state *app, int window_width, int window_height)
{
    if (app->current_state == STATE_LOGIN) {
        /* Login Screen with Scenic Background reference */
        if (nk_begin(ctx, "Login", nk_rect(window_width/2 - 175, window_height/2 - 150, 350, 300),
            NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR))
        {
            nk_layout_row_dynamic(ctx, 80, 1);
            nk_label(ctx, "[ AVATAR ]", NK_TEXT_CENTERED); /* Placeholder for avatar */

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, "welcome back user!:", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "password", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER, app->password, sizeof(app->password), nk_filter_default);

            nk_layout_row_dynamic(ctx, 40, 1);
            if (nk_button_label(ctx, "<- not you?")) {
                /* Reset or different user */
            }

            nk_layout_row_dynamic(ctx, 30, 1);
            if (nk_button_label(ctx, "Login")) app->current_state = STATE_INSTALLER;
        }
        nk_end(ctx);
    } else if (app->current_state == STATE_INSTALLER) {
        if (nk_begin(ctx, "Installer", nk_rect(window_width/2 - 250, window_height/2 - 200, 500, 400),
            NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, "Have you used os*2? before?", NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 30, 1);
            if (nk_option_label(ctx, "yes but I don't want a tutorial", app->progress == 0)) app->progress = 0;
            if (nk_option_label(ctx, "yes but I'd like a tutorial", app->progress == 1)) app->progress = 1;
            if (nk_option_label(ctx, "No but I don't want a tutorial", app->progress == 2)) app->progress = 2;
            if (nk_option_label(ctx, "no but I'd like a tutorial", app->progress == 3)) app->progress = 3;

            nk_layout_row_dynamic(ctx, 100, 1);
            nk_group_begin(ctx, "Note", NK_WINDOW_BORDER);
            nk_layout_row_dynamic(ctx, 60, 1);
            nk_label_wrap(ctx, "note: this tutorial will teach you how to use this os to its fullest");
            nk_group_end(ctx);

            nk_layout_row_dynamic(ctx, 40, 1);
            if (nk_button_label(ctx, "Continue")) app->current_state = STATE_DESKTOP;
        }
        nk_end(ctx);
    } else if (app->current_state == STATE_DESKTOP) {
        /* App Grid (Top-Left) */
        if (nk_begin(ctx, "AppGrid", nk_rect(40, 40, 400, 500), NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row_static(ctx, 60, 60, 5);
            for (int i = 0; i < 30; ++i) {
                if (i < 5) nk_button_label(ctx, "R");
                else if (i < 10) nk_button_label(ctx, "B");
                else if (i < 15) nk_button_label(ctx, "O");
                else if (i < 20) nk_button_label(ctx, "L");
                else nk_button_label(ctx, "G");
            }
        }
        nk_end(ctx);

        /* System Widget (Right) */
        if (nk_begin(ctx, "Welcome", nk_rect(window_width - 340, 150, 300, 150),
            NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_MOVABLE))
        {
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "System status: Nominal", NK_TEXT_LEFT);
            nk_label(ctx, "CPU: 2%", NK_TEXT_LEFT);
            nk_label(ctx, "RAM: 1.2GB / 16GB", NK_TEXT_LEFT);
        }
        nk_end(ctx);

        /* Clock (Top-Right) */
        if (nk_begin(ctx, "Clock", nk_rect(window_width - 340, 40, 300, 100), NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row_dynamic(ctx, 50, 1);
            nk_label(ctx, "12:34", NK_TEXT_RIGHT);
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "eastern standard (+3:00)", NK_TEXT_RIGHT);
        }
        nk_end(ctx);

        /* Taskbar */
        if (nk_begin(ctx, "Taskbar", nk_rect(0, window_height - 50, window_width, 50),
            NK_WINDOW_NO_SCROLLBAR))
        {
            nk_layout_row_static(ctx, 30, 40, 6);
            if (nk_button_label(ctx, "M")) app->show_launcher = !app->show_launcher;

            if (nk_button_label(ctx, "R")) { /* Red App */ }
            if (nk_button_label(ctx, "B")) { /* Blue App */ }
            if (nk_button_label(ctx, "O")) { /* Orange App */ }
            if (nk_button_label(ctx, "C")) { /* Cyan App */ }

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_spacer(ctx);
            nk_label(ctx, "✓ 📶 🔋", NK_TEXT_RIGHT);
        }
        nk_end(ctx);

        /* Launcher */
        if (app->show_launcher) {
            if (nk_begin(ctx, "Launcher", nk_rect(0, window_height - 340, 200, 300),
                NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR))
            {
                nk_layout_row_dynamic(ctx, 30, 1);
                if (nk_button_label(ctx, "Terminal")) { app->show_terminal = 1; app->show_launcher = 0; }
                if (nk_button_label(ctx, "File Explorer")) { app->show_explorer = 1; app->show_launcher = 0; }
                if (nk_button_label(ctx, "System Settings")) { app->show_settings = 1; app->show_launcher = 0; }
                nk_rule_horizontal(ctx, nk_rgb(40, 80, 120), 1);
                if (nk_button_label(ctx, "Logout")) app->current_state = STATE_LOGIN;
            }
            nk_end(ctx);
        }

        /* Terminal Window */
        if (app->show_terminal) {
            if (nk_begin(ctx, "Terminal", nk_rect(50, 50, 600, 400),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE))
            {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, "R-TECH OS v0.1.0-alpha (Freestanding x86_64)", NK_TEXT_LEFT);
                nk_rule_horizontal(ctx, nk_rgb(40, 80, 120), 1);
                nk_label(ctx, "root@rtech:~# ls /dev", NK_TEXT_LEFT);
                nk_label(ctx, "usb0  sda0  tty0  fb0", NK_TEXT_LEFT);
                nk_label(ctx, "root@rtech:~# _", NK_TEXT_LEFT);

                nk_layout_row_dynamic(ctx, 30, 1);
                if (nk_button_label(ctx, "Run Benchmark")) {
                    /* Future feature stub */
                }
            }
            if (nk_window_is_closed(ctx, "Terminal")) app->show_terminal = 0;
            nk_end(ctx);
        }

        /* File Explorer */
        if (app->show_explorer) {
            if (nk_begin(ctx, "Explorer", nk_rect(100, 100, 500, 350),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE))
            {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "System Storage Devices:", NK_TEXT_LEFT);

                int count = hal_storage_get_device_count();
                for (int i = 0; i < count; i++) {
                    storage_device_t *dev = hal_storage_get_device(i);
                    nk_layout_row_dynamic(ctx, 30, 1);
                    if (nk_button_label(ctx, dev->name)) {
                        /* Future: browse device */
                    }
                }
            }
            if (nk_window_is_closed(ctx, "Explorer")) app->show_explorer = 0;
            nk_end(ctx);
        }

        /* Settings */
        if (app->show_settings) {
            if (nk_begin(ctx, "Settings", nk_rect(150, 150, 400, 400),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE))
            {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "System Information", NK_TEXT_LEFT);
                nk_label(ctx, "Kernel: x86_64 Freestanding", NK_TEXT_LEFT);
                nk_label(ctx, "GUI: Nuklear Immediate Mode", NK_TEXT_LEFT);
                nk_label(ctx, "USB: CherryUSB Stack", NK_TEXT_LEFT);

                nk_layout_row_dynamic(ctx, 30, 2);
                nk_label(ctx, "USB Service:", NK_TEXT_LEFT);
                if (g_services.usb) nk_label(ctx, "Online", NK_TEXT_LEFT);
                else nk_label(ctx, "Offline", NK_TEXT_LEFT);
            }
            if (nk_window_is_closed(ctx, "Settings")) app->show_settings = 0;
            nk_end(ctx);
        }
    }
}
