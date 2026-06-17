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
    if (nk_begin(ctx, "Taskbar", nk_rect(0, (float)wh - 50, (float)ww, 50), NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_static(ctx, 30, 40, 6);
        if (nk_button_label(ctx, "M")) app->show_launcher = !app->show_launcher;

        if (app->show_terminal) nk_label(ctx, "[T]", NK_TEXT_CENTERED);
        if (app->show_explorer) nk_label(ctx, "[F]", NK_TEXT_CENTERED);
        if (app->show_settings) nk_label(ctx, "[S]", NK_TEXT_CENTERED);

        nk_layout_row_dynamic(ctx, 30, 1);
        nk_spacer(ctx);

        char clock_buf[64];
        int h, m, s;
        rtc_get_time(&h, &m, &s);
        snprintf(clock_buf, 64, "%02d:%02d:%02d | USB: %d | 📶", h, m, s, hal_storage_get_device_count());
        nk_label(ctx, clock_buf, NK_TEXT_RIGHT);
    }
    nk_end(ctx);
}

void ui_render(struct nk_context *ctx, struct app_state *app, int window_width, int window_height)
{
    float ww = (float)window_width;
    float wh = (float)window_height;

    if (app->current_state == STATE_LOGIN) {
        /* Modified by Sovereign: Boot directly into a simplified WinPE-style Recovery Environment */
        app->current_state = STATE_DESKTOP;
        app->show_terminal = 1;
    } else if (app->current_state == STATE_INSTALLER) {
        if (nk_begin(ctx, "Installer", nk_rect(ww/2 - 250, wh/2 - 200, 500, 400),
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

        if (nk_begin(ctx, "AppGrid", nk_rect(20, 20, 300, 400), NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row_static(ctx, 60, 60, 4);
            if (nk_button_label(ctx, "Term")) app->show_terminal = 1;
            if (nk_button_label(ctx, "Files")) app->show_explorer = 1;
            if (nk_button_label(ctx, "Lab")) app->show_explorer = 1;
            if (nk_button_label(ctx, "Setup")) app->show_settings = 1;
        }
        nk_end(ctx);

        if (app->show_terminal) {
            chell_update(ctx, app);
        }

        if (app->show_explorer) {
            lab_update(ctx, app);

            if (nk_begin(ctx, "Explorer", nk_rect(150, 150, 500, 350),
                NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE))
            {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "Mounted Volumes:", NK_TEXT_LEFT);
                int count = hal_storage_get_device_count();
                for (int i = 0; i < count; i++) {
                    storage_device_t *dev = hal_storage_get_device(i);
                    if (dev) {
                        nk_layout_row_dynamic(ctx, 30, 1);
                        nk_label(ctx, dev->name, NK_TEXT_LEFT);
                    }
                }
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

        if (nk_begin(ctx, "SysMon", nk_rect(ww - 320, 20, 300, 180),
            NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_MOVABLE))
        {
            size_t used = hal_malloc_get_used();
            size_t total = hal_malloc_get_total();
            char mem_buf[64];
            snprintf(mem_buf, 64, "Memory: %d KB / %d KB", (int)(used/1024), (int)(total/1024));
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, mem_buf, NK_TEXT_LEFT);

            char cpu_buf[64];
            int load = scheduler_get_cpu_load();
            snprintf(cpu_buf, 64, "CPU Load: %d%%", load);
            nk_label(ctx, cpu_buf, NK_TEXT_LEFT);
            nk_progress(ctx, (nk_size*)&load, 100, nk_false);
        }
        nk_end(ctx);
    }
}
