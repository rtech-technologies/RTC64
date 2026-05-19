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
#include "pro_os.h"
#include "ff.h"

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

static void ui_render_taskbar(struct nk_context *ctx, struct app_state *app, int ww, int wh) {
    if (nk_begin(ctx, "Taskbar", nk_rect(0, wh - 50, ww, 50), NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_static(ctx, 30, 40, 7);
        if (nk_button_label(ctx, "M")) app->show_launcher = !app->show_launcher;

        if (app->show_terminal) nk_label(ctx, "[T]", NK_TEXT_CENTERED);
        if (app->show_explorer) nk_label(ctx, "[F]", NK_TEXT_CENTERED);
        if (app->show_settings) nk_label(ctx, "[S]", NK_TEXT_CENTERED);
        if (app->show_chell) nk_label(ctx, "[C]", NK_TEXT_CENTERED);

        nk_layout_row_dynamic(ctx, 30, 1);
        nk_spacer(ctx);

        char clock_buf[32];
        snprintf(clock_buf, 32, "12:34 | USB: %d | 📶", hal_storage_get_device_count());
        nk_label(ctx, clock_buf, NK_TEXT_RIGHT);
    }
    nk_end(ctx);
}

void ui_render(struct nk_context *ctx, struct app_state *app, int window_width, int window_height)
{
    if (app->current_state == STATE_LOGIN) {
        if (nk_begin(ctx, "Login", nk_rect(window_width/2 - 175, window_height/2 - 180, 350, 360),
            NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR))
        {
            nk_layout_row_dynamic(ctx, 80, 1);
            nk_label(ctx, "[ AVATAR ]", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, "Sovereign User", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_spacer(ctx);

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->password, sizeof(app->password), nk_filter_default);

            nk_layout_row_dynamic(ctx, 40, 1);
            if (nk_button_label(ctx, i18n_translate("login"))) {
                app->current_state = STATE_INSTALLER;
                app->show_installer = 1;
                app->show_welcome = 1;
            }

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_spacer(ctx);

            nk_layout_row_static(ctx, 30, 80, 1);
            if (nk_button_label(ctx, "Power")) { /* Shutdown sequence placeholder */ }
        }
        nk_end(ctx);
    } else if (app->current_state == STATE_INSTALLER) {
        if (nk_begin(ctx, "Installer", nk_rect(window_width/2 - 250, window_height/2 - 200, 500, 400),
            NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, "R-TECH™ Installation Wizard", NK_TEXT_CENTERED);
            nk_label(ctx, "How familiar are you with this environment?", NK_TEXT_LEFT);

            static int familiarity = 0;
            nk_layout_row_dynamic(ctx, 30, 1);
            if (nk_option_label(ctx, "I know my way around (Advanced)", familiarity == 0)) familiarity = 0;
            if (nk_option_label(ctx, "I'm new here (Standard)", familiarity == 1)) familiarity = 1;

            nk_layout_row_dynamic(ctx, 100, 1);
            nk_spacer(ctx);

            nk_layout_row_dynamic(ctx, 40, 2);
            nk_spacer(ctx);
            if (nk_button_label(ctx, "Continue to Desktop")) app->current_state = STATE_DESKTOP;
        }
        nk_end(ctx);
    } else if (app->current_state == STATE_DESKTOP) {
        ui_render_taskbar(ctx, app, window_width, window_height);

        if (app->show_welcome) {
            if (nk_begin(ctx, "Welcome", nk_rect(window_width/2 - 200, window_height/2 - 150, 400, 300),
                NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_CLOSABLE|NK_WINDOW_MOVABLE))
            {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "Welcome to R-TECH™ Sovereign OS", NK_TEXT_CENTERED);
                nk_layout_row_dynamic(ctx, 100, 1);
                nk_label_wrap(ctx, "You have successfully bootstrapped the Sovereign kernel environment. FatFs and CherryUSB are active.");
                nk_layout_row_dynamic(ctx, 40, 1);
                if (nk_button_label(ctx, "Explore Now")) app->show_welcome = 0;
            }
            if (nk_window_is_closed(ctx, "Welcome")) app->show_welcome = 0;
            nk_end(ctx);
        }

        if (nk_begin(ctx, "AppGrid", nk_rect(20, 20, 300, 400), NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row_static(ctx, 60, 60, 4);
            if (nk_button_label(ctx, "Term")) app->show_terminal = 1;
            if (nk_button_label(ctx, "Files")) app->show_explorer = 1;
            if (nk_button_label(ctx, "Setup")) app->show_settings = 1;
            if (nk_button_label(ctx, "Chell")) app->show_chell = 1;
            if (nk_button_label(ctx, "Lab")) app->show_lab = 1;
        }
        nk_end(ctx);

        if (app->show_terminal) {
            if (nk_begin(ctx, "Terminal", nk_rect(100, 100, 600, 400),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE))
            {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, "root@pro-os:~#", NK_TEXT_LEFT);
                if (nk_button_label(ctx, "Request Network Access")) {
                    uac_request_permit(0, "network");
                    app->show_uac = 1;
                }
            }
            if (nk_window_is_closed(ctx, "Terminal")) app->show_terminal = 0;
            nk_end(ctx);
        }

        if (app->show_chell) {
            app->chell.active = 1;
            chell_ui_render(ctx, &app->chell);
            if (!app->chell.active) app->show_chell = 0;
        }

        if (app->show_lab) {
            app->lab.active = 1;
            lab_ui_render(ctx, &app->lab);
            if (!app->lab.active) app->show_lab = 0;
        }

        if (app->show_installer) {
            installer_ui_render(ctx, &app->installer);
            if (!app->installer.active) app->show_installer = 0;
        }

        if (app->show_explorer) {
            if (nk_begin(ctx, "Explorer", nk_rect(150, 150, 500, 350),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE))
            {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "Physical Devices:", NK_TEXT_LEFT);
                int count = hal_storage_get_device_count();
                for (int i = 0; i < count; i++) {
                    storage_device_t *dev = hal_storage_get_device(i);
                    char buf[64];
                    snprintf(buf, 64, "  [DISK %d] %s (%lu blocks)", i, dev->name, (unsigned long)dev->total_blocks);
                    nk_label(ctx, buf, NK_TEXT_LEFT);

                    nk_layout_row_dynamic(ctx, 30, 2);
                    if (nk_button_label(ctx, "Format FAT32")) {
                        char drv[4];
                        snprintf(drv, 4, "%d:", i);
                        void* work = malloc(FF_MAX_SS);
                        if (work) {
                            f_mkfs(drv, NULL, work, FF_MAX_SS);
                            free(work);
                        }
                    }
                }

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "Filesystems:", NK_TEXT_LEFT);

                static char ls_buf[512];
                static int ls_done = 0;
                if (!ls_done) {
                    if (vfs_ls("0:", ls_buf, 512) == 0) ls_done = 1;
                    else snprintf(ls_buf, 512, "No files found or device not ready.");
                }

                nk_layout_row_dynamic(ctx, 150, 1);
                nk_label_wrap(ctx, ls_buf);
            }
            if (nk_window_is_closed(ctx, "Explorer")) app->show_explorer = 0;
            nk_end(ctx);
        }

        if (app->show_settings) {
            if (nk_begin(ctx, "Settings", nk_rect(200, 200, 400, 400),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE))
            {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "App Permissions", NK_TEXT_LEFT);
                nk_checkbox_label(ctx, "Terminal: Network", (nk_bool*)&app->perm_net);
                nk_checkbox_label(ctx, "Terminal: Storage", (nk_bool*)&app->perm_storage);
            }
            if (nk_window_is_closed(ctx, "Settings")) app->show_settings = 0;
            nk_end(ctx);
        }

        if (app->show_uac) {
            if (nk_begin(ctx, "UAC Security", nk_rect(window_width/2 - 200, window_height/2 - 100, 400, 200),
                NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR))
            {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "PERMISSION REQUEST", NK_TEXT_CENTERED);
                nk_label(ctx, "App 0 wants to access Network.", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 40, 2);
                if (nk_button_label(ctx, "Allow")) { app->perm_net = 1; app->show_uac = 0; }
                if (nk_button_label(ctx, "Deny")) { app->show_uac = 0; }
            }
            nk_end(ctx);
        }

        if (nk_begin(ctx, "SysMon", nk_rect(window_width - 320, 20, 300, 150),
            NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_MOVABLE))
        {
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "Memory: 12MB / 64MB", NK_TEXT_LEFT);
            nk_label(ctx, "CPU: 5% (Scheduler OK)", NK_TEXT_LEFT);
            nk_progress(ctx, (nk_size*)&app->cpu_usage, 100, NK_FIXED);
        }
        nk_end(ctx);
    }
}
