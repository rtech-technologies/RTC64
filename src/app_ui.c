/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "nuklear.h"
#include "pro_os.h"
#include "app_ui.h"
#include "app_loader.h"
#include <string.h>
#include <stdio.h>

extern size_t hal_malloc_get_used(void);
extern size_t hal_malloc_get_total(void);

static void ui_init_style_internal(struct nk_context *ctx)
{
    struct nk_color table[NK_COLOR_COUNT];
    table[NK_COLOR_TEXT] = nk_rgba(220, 225, 235, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(15, 22, 34, 255);
    table[NK_COLOR_HEADER] = nk_rgba(22, 35, 55, 255);
    table[NK_COLOR_BORDER] = nk_rgba(42, 70, 110, 255);
    table[NK_COLOR_BUTTON] = nk_rgba(35, 55, 85, 255);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgba(60, 100, 150, 255);
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(95, 155, 220, 255);
    table[NK_COLOR_TOGGLE] = nk_rgba(30, 40, 54, 255);
    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(130, 190, 255, 255);
    table[NK_COLOR_SELECT] = nk_rgba(35, 55, 85, 255);
    table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(130, 190, 255, 255);
    table[NK_COLOR_SLIDER] = nk_rgba(25, 35, 48, 255);
    table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(130, 190, 255, 255);
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(170, 220, 255, 255);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(205, 245, 255, 255);
    table[NK_COLOR_PROPERTY] = nk_rgba(28, 38, 55, 255);
    table[NK_COLOR_EDIT] = nk_rgba(20, 28, 40, 255);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(130, 190, 255, 255);
    table[NK_COLOR_COMBO] = nk_rgba(28, 38, 55, 255);
    table[NK_COLOR_CHART] = nk_rgba(25, 36, 50, 255);
    table[NK_COLOR_CHART_COLOR] = nk_rgba(130, 190, 255, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 145, 105, 255);
    table[NK_COLOR_SCROLLBAR] = nk_rgba(18, 26, 36, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(50, 75, 105, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(90, 135, 180, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(130, 180, 220, 255);
    table[NK_COLOR_TAB_HEADER] = nk_rgba(22, 35, 55, 255);
    nk_style_from_table(ctx, table);
}

static void ui_render_taskbar(struct nk_context *ctx, struct app_state *app, int ww, int wh)
{
    if (nk_begin(ctx, "Taskbar", nk_rect(0, (float)wh - 48, (float)ww, 48), NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_begin(ctx, NK_STATIC, 36, 4);
        nk_layout_row_push(ctx, 90);
        if (nk_button_label(ctx, "Start")) app->show_launcher = !app->show_launcher;
        nk_layout_row_push(ctx, 0);
        nk_spacer(ctx);
        nk_layout_row_push(ctx, 250);
        char clock_buf[80];
        int h, m, s;
        rtc_get_time(&h, &m, &s);
        snprintf(clock_buf, sizeof(clock_buf), "%02d:%02d:%02d   |   USB %d   |   Ready", h, m, s, hal_storage_get_device_count());
        nk_label(ctx, clock_buf, NK_TEXT_RIGHT);
    }
    nk_end(ctx);
}

static void ui_render_launcher(struct nk_context *ctx, struct app_state *app)
{
    if (!app->show_launcher) return;
    if (nk_begin(ctx, "App Launcher", nk_rect(20, 60, 420, 360), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, "Applications", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 72, 4);
        if (nk_button_label(ctx, "Terminal")) app->show_terminal = 1;
        if (nk_button_label(ctx, "Files")) app->show_explorer = 1;
        if (nk_button_label(ctx, "Studio")) app->show_app_studio = 1;
        if (nk_button_label(ctx, "Diagnostics")) app->show_lab = 1;
        nk_layout_row_dynamic(ctx, 72, 4);
        if (nk_button_label(ctx, "Settings")) app->show_settings = 1;
        if (nk_button_label(ctx, "Installer")) app->current_state = STATE_INSTALLER;
        if (nk_button_label(ctx, "Lock")) app->current_state = STATE_LOGIN;
        if (nk_button_label(ctx, "Close")) app->show_launcher = 0;
    }
    if (nk_window_is_closed(ctx, "App Launcher")) app->show_launcher = 0;
    nk_end(ctx);
}

static void ui_render_background(struct nk_context *ctx, int ww, int wh)
{
    struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
    nk_fill_rect(canvas, nk_rect(0, 0, (float)ww, (float)wh), 0, nk_rgba(10, 16, 28, 255));
    nk_fill_rect(canvas, nk_rect(26, 26, (float)ww - 52, (float)wh - 98), 0, nk_rgba(20, 50, 91, 210));
    nk_fill_rect(canvas, nk_rect(36, 36, 300, 150), 6, nk_rgba(0, 120, 220, 180));
    nk_fill_rect(canvas, nk_rect((float)ww - 336, 36, 300, 150), 6, nk_rgba(110, 160, 245, 160));
    nk_draw_text(canvas, nk_rect(60, 56, 260, 40), "Sovereign RTC64", 13, ctx->style.font, nk_rgba(255,255,255,255), nk_rgba(0,0,0,0));
    nk_draw_text(canvas, nk_rect(60, 102, 260, 24), "Modern scripts and desktop apps on legacy hardware.", 41, ctx->style.font, nk_rgba(220,220,220,255), nk_rgba(0,0,0,0));
}

static void ui_render_system_panel(struct nk_context *ctx, struct app_state *app)
{
    if (nk_begin(ctx, "System Panel", nk_rect(1180, 60, 320, 180), NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_MOVABLE)) {
        size_t used = hal_malloc_get_used();
        size_t total = hal_malloc_get_total();
        char mem_buf[64];
        snprintf(mem_buf, sizeof(mem_buf), "Memory: %d KB / %d KB", (int)(used / 1024), (int)(total / 1024));
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, mem_buf, NK_TEXT_LEFT);
        nk_label(ctx, "CPU: 2% (Scheduler active)", NK_TEXT_LEFT);
        nk_label(ctx, "User: Administrator", NK_TEXT_LEFT);
        nk_label(ctx, app->show_script_app ? "Script App: Running" : "Script App: Idle", NK_TEXT_LEFT);
        if (nk_button_label(ctx, "Crash Reports")) {
            app->show_crash_reports = 1;
            /* copy last crash path from kernel-visible storage if available */
            const char *p = NULL;
            extern const char* get_last_crash_path(void);
            p = get_last_crash_path();
            if (p) strncpy(app->last_crash_path, p, sizeof(app->last_crash_path)-1);
        }
    }
    nk_end(ctx);
}

static void ui_render_crash_reports(struct nk_context *ctx, struct app_state *app)
{
    if (!app->show_crash_reports) return;
    if (nk_begin(ctx, "Crash Reports", nk_rect(360, 140, 640, 420), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, "Crash report viewer", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 28, 2);
        if (nk_button_label(ctx, "Refresh")) { }
        if (nk_button_label(ctx, "Open Latest") && app->last_crash_path[0]) {
            char buf[8192];
            if (vfs_cat(app->last_crash_path, buf, sizeof(buf)) == 0) {
                nk_layout_row_dynamic(ctx, 300, 1);
                nk_label_wrap(ctx, buf);
            } else {
                nk_label(ctx, "Failed to read report.", NK_TEXT_LEFT);
            }
        }
    }
    if (nk_window_is_closed(ctx, "Crash Reports")) app->show_crash_reports = 0;
    nk_end(ctx);
}

static void ui_render_files(struct nk_context *ctx, struct app_state *app)
{
    if (nk_begin(ctx, "Files", nk_rect(240, 140, 540, 420), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 26, 1);
        nk_label(ctx, "File Manager", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label_wrap(ctx, "Browse mounted volumes, inspect storage health, and manage system files from a native desktop view.");
        nk_layout_row_dynamic(ctx, 34, 2);
        if (nk_button_label(ctx, "Refresh")) { }
        if (nk_button_label(ctx, "Open Root")) { }
        nk_layout_row_dynamic(ctx, 220, 1);
        nk_label_wrap(ctx, "Mounted storage will appear here when file drivers are active. This app is the foundation for a real file browser.");
    }
    if (nk_window_is_closed(ctx, "Files")) app->show_explorer = 0;
    nk_end(ctx);
}

static void ui_render_settings(struct nk_context *ctx, struct app_state *app)
{
    if (nk_begin(ctx, "Settings", nk_rect(280, 180, 420, 340), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 28, 1);
        nk_label(ctx, "System Settings", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 28, 1);
        nk_checkbox_label(ctx, "Terminal network access", (nk_bool*)&app->perm_net);
        nk_checkbox_label(ctx, "Terminal storage access", (nk_bool*)&app->perm_storage);
        nk_layout_row_dynamic(ctx, 28, 1);
        nk_label(ctx, "Current user:", NK_TEXT_LEFT);
        nk_label(ctx, app->username, NK_TEXT_LEFT);
    }
    if (nk_window_is_closed(ctx, "Settings")) app->show_settings = 0;
    nk_end(ctx);
}

static void ui_render_desktop(struct nk_context *ctx, struct app_state *app, int ww, int wh)
{
    ui_render_background(ctx, ww, wh);
    ui_render_taskbar(ctx, app, ww, wh);
    ui_render_launcher(ctx, app);
    ui_render_system_panel(ctx, app);
    ui_render_crash_reports(ctx, app);

    if (app->show_terminal) chell_update(ctx, app);
    if (app->show_explorer) ui_render_files(ctx, app);
    if (app->show_settings) ui_render_settings(ctx, app);
    if (app->show_app_studio) studio_update(ctx, app);
    if (app->show_script_app) app_loader_update(ctx, app);
    if (app->show_lab) lab_update(ctx, app);
}

void ui_init_style(struct nk_context *ctx) {
    ui_init_style_internal(ctx);
}

void ui_render(struct nk_context *ctx, struct app_state *app, int window_width, int window_height)
{
    float ww = (float)window_width;
    float wh = (float)window_height;

    if (app->current_state == STATE_LOGIN) {
        if (nk_begin(ctx, "Login", nk_rect(ww/2 - 210, wh/2 - 200, 420, 360), NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row_dynamic(ctx, 60, 1);
            nk_label(ctx, "Welcome to Sovereign RTC64", NK_TEXT_CENTERED);
            nk_layout_row_dynamic(ctx, 28, 1);
            nk_label(ctx, "Sign in to access your desktop and applications.", NK_TEXT_CENTERED);
            nk_layout_row_dynamic(ctx, 34, 1);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->password, sizeof(app->password), nk_filter_default);
            nk_layout_row_dynamic(ctx, 40, 2);
            if (nk_button_label(ctx, "Sign In")) {
                app->current_state = STATE_DESKTOP;
                app->show_launcher = 1;
            }
            if (nk_button_label(ctx, "Installer")) app->current_state = STATE_INSTALLER;
            nk_layout_row_dynamic(ctx, 22, 1);
            nk_label(ctx, "RTC64 is designed for modern desktop workflows on legacy PC architecture.", NK_TEXT_CENTERED);
        }
        nk_end(ctx);
    } else if (app->current_state == STATE_INSTALLER) {
        if (nk_begin(ctx, "Installer", nk_rect(ww/2 - 260, wh/2 - 220, 520, 420), NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_TITLE)) {
            nk_layout_row_dynamic(ctx, 28, 1);
            nk_label(ctx, "Sovereign Installer", NK_TEXT_CENTERED);
            nk_layout_row_dynamic(ctx, 26, 1);
            nk_label_wrap(ctx, "Prepare storage and install the operating system with a guided setup flow.");
            nk_layout_row_dynamic(ctx, 30, 1);
            static int target_drive = 0;
            if (nk_option_label(ctx, "Disk 0 (Primary)", target_drive == 0)) target_drive = 0;
            if (nk_option_label(ctx, "Disk 1 (Secondary)", target_drive == 1)) target_drive = 1;
            nk_layout_row_dynamic(ctx, 34, 2);
            if (nk_button_label(ctx, "Install")) {
                app->installed = 1;
                app->current_state = STATE_DESKTOP;
                app->show_launcher = 1;
            }
            if (nk_button_label(ctx, "Back")) app->current_state = STATE_LOGIN;
        }
        nk_end(ctx);
    } else if (app->current_state == STATE_DESKTOP) {
        ui_render_desktop(ctx, app, window_width, window_height);
    }
}
