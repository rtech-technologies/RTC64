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

void ui_init_style(struct nk_context *ctx)
{
    struct nk_color table[NK_COLOR_COUNT];
    table[NK_COLOR_TEXT] = nk_rgba(70, 200, 255, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(10, 15, 25, 255);
    table[NK_COLOR_HEADER] = nk_rgba(20, 30, 50, 255);
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
        if (nk_begin(ctx, "Login", nk_rect(window_width/2 - 150, window_height/2 - 100, 300, 200),
            NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, "R-TECH SYSTEM LOGIN", NK_TEXT_CENTERED);
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->username, sizeof(app->username), nk_filter_default);
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->password, sizeof(app->password), nk_filter_default);
            nk_layout_row_dynamic(ctx, 30, 1);
            if (nk_button_label(ctx, "Login")) app->current_state = STATE_INSTALLER;
        }
        nk_end(ctx);
    } else if (app->current_state == STATE_INSTALLER) {
        if (nk_begin(ctx, "Installer", nk_rect(window_width/2 - 200, window_height/2 - 150, 400, 300),
            NK_WINDOW_BORDER|NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, "Welcome to R-TECH Installer", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 150, 1);
            nk_label_wrap(ctx, "Installing system features...");
            if (app->install_started) {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_size prog = (nk_size)app->progress;
                nk_progress(ctx, &prog, 100, NK_MODIFIABLE);
                if (app->progress < 100) app->progress++;
                else if (nk_button_label(ctx, "Finish")) app->current_state = STATE_DESKTOP;
            } else if (nk_button_label(ctx, "Start")) app->install_started = 1;
        }
        nk_end(ctx);
    } else if (app->current_state == STATE_DESKTOP) {
        /* Desktop Background / Branding */
        // Removed for now due to font handle mismatch in hosted vs kernel environment

        /* Taskbar */
        if (nk_begin(ctx, "Taskbar", nk_rect(0, window_height - 40, window_width, 40),
            NK_WINDOW_NO_SCROLLBAR))
        {
            nk_layout_row_static(ctx, 30, 100, 10);
            if (nk_button_label(ctx, "R-TECH")) app->show_launcher = !app->show_launcher;

            if (app->show_terminal) if (nk_button_label(ctx, "Terminal")) app->show_terminal = 1;
            if (app->show_explorer) if (nk_button_label(ctx, "Explorer")) app->show_explorer = 1;
            if (app->show_settings) if (nk_button_label(ctx, "Settings")) app->show_settings = 1;

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_spacer(ctx);
            if (g_services.clock) {
                nk_label(ctx, "12:00 PM", NK_TEXT_RIGHT);
            } else {
                nk_label(ctx, "--:--", NK_TEXT_RIGHT);
            }
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
                nk_label(ctx, "Devices:", NK_TEXT_LEFT);
                nk_layout_row_static(ctx, 80, 80, 4);
                if (g_services.storage) {
                    if (nk_button_label(ctx, "DISK 0")) { /* Browse storage */ }
                }
                if (g_services.usb) {
                    nk_button_label(ctx, "USB 0");
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
