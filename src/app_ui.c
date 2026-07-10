/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license.
 * Modified by Sovereign for Boutique Cyber-Minimalist Dark Mode styling. */
#include "nuklear.h"
#include "pro_os.h"
#include "app_ui.h"
#include "app_loader.h"
#include "math.h"
#include <string.h>
#include <stdio.h>

extern size_t hal_malloc_get_used(void);
extern size_t hal_malloc_get_total(void);
extern void rtc_get_time(int *h, int *m, int *s);
extern const char* get_last_crash_path(void);

static void ui_init_style_internal(struct nk_context *ctx)
{
    struct nk_color table[NK_COLOR_COUNT];

    /* Matte charcoal gray background, white text */
    table[NK_COLOR_TEXT] = nk_rgba(255, 255, 255, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(51, 51, 51, 255);
    table[NK_COLOR_HEADER] = nk_rgba(51, 51, 51, 255);
    table[NK_COLOR_BORDER] = nk_rgba(51, 51, 51, 0); /* Flat, borderless */

    /* Interactive element base: pure solid blue as default */
    table[NK_COLOR_BUTTON] = nk_rgba(0, 0, 255, 255);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgba(0, 255, 255, 255); /* Pure Cyan */
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(255, 102, 0, 255); /* Pure Orange */

    table[NK_COLOR_TOGGLE] = nk_rgba(51, 51, 51, 255);
    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(0, 255, 0, 255); /* Pure Green */

    table[NK_COLOR_SELECT] = nk_rgba(0, 0, 255, 255);
    table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(255, 102, 0, 255);

    table[NK_COLOR_SLIDER] = nk_rgba(0, 0, 0, 255);
    table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(0, 255, 255, 255);
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(255, 0, 0, 255); /* Pure Red */
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(0, 255, 0, 255);

    table[NK_COLOR_PROPERTY] = nk_rgba(51, 51, 51, 255);
    table[NK_COLOR_EDIT] = nk_rgba(0, 0, 0, 255);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(0, 255, 255, 255);
    table[NK_COLOR_COMBO] = nk_rgba(51, 51, 51, 255);

    table[NK_COLOR_CHART] = nk_rgba(51, 51, 51, 255);
    table[NK_COLOR_CHART_COLOR] = nk_rgba(0, 255, 255, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);

    table[NK_COLOR_SCROLLBAR] = nk_rgba(51, 51, 51, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(0, 0, 255, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(0, 255, 255, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(255, 102, 0, 255);
    table[NK_COLOR_TAB_HEADER] = nk_rgba(51, 51, 51, 255);

    nk_style_from_table(ctx, table);

    /* Enforce "Block & Capsule" layout with aggressive corner rounding & no borders */
    ctx->style.window.rounding = 24.0f;
    ctx->style.window.border = 0.0f;
    ctx->style.window.header.normal = nk_style_item_color(nk_rgba(51, 51, 51, 255));
    ctx->style.window.header.hover = nk_style_item_color(nk_rgba(51, 51, 51, 255));
    ctx->style.window.header.active = nk_style_item_color(nk_rgba(51, 51, 51, 255));
    ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(51, 51, 51, 255));
    ctx->style.window.background = nk_rgba(51, 51, 51, 255);
    ctx->style.window.border_color = nk_rgba(51, 51, 51, 0);
    ctx->style.window.group_border = 0.0f;
    ctx->style.window.group_border_color = nk_rgba(51, 51, 51, 0);

    /* Close, block-filling internal spacing and tight padding */
    ctx->style.window.padding = nk_vec2(4, 4);
    ctx->style.window.group_padding = nk_vec2(2, 2);
    ctx->style.window.spacing = nk_vec2(4, 4);

    /* Clean, block-based button styling */
    ctx->style.button.rounding = 8.0f;
    ctx->style.button.border = 0.0f;
    ctx->style.button.padding = nk_vec2(4, 4);
}

void ui_init_style(struct nk_context *ctx) {
    ui_init_style_internal(ctx);
}

static void ui_render_taskbar(struct nk_context *ctx, struct app_state *app, int ww, int wh)
{
    /* Flat matte black bar pinned to bottom */
    if (nk_begin(ctx, "taskbar", nk_rect(0, (float)wh - 48, (float)ww, 48), NK_WINDOW_NO_SCROLLBAR)) {
        struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);

        /* Fill matte black background explicitly */
        nk_fill_rect(canvas, nk_rect(0, (float)wh - 48, (float)ww, 48), 0, nk_rgba(0, 0, 0, 255));

        /* Left-aligned content start */
        nk_layout_row_begin(ctx, NK_STATIC, 36, 12);

        /* Start hamburger icon: white 4-line graphic on far left */
        nk_layout_row_push(ctx, 40);

        /* Interactive overlay button styled as transparent */
        ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
        ctx->style.button.hover = nk_style_item_color(nk_rgba(255, 255, 255, 40));
        ctx->style.button.active = nk_style_item_color(nk_rgba(255, 255, 255, 80));
        ctx->style.button.rounding = 4.0f;
        if (nk_button_label(ctx, "")) {
            app->show_launcher = !app->show_launcher;
        }

        /* Draw the actual white hamburger lines */
        float gx = 15.0f;
        float gy = (float)wh - 34.0f;
        nk_stroke_line(canvas, gx, gy,       gx + 24.0f, gy,       2.0f, nk_rgba(255, 255, 255, 255));
        nk_stroke_line(canvas, gx, gy + 6.0f,  gx + 24.0f, gy + 6.0f,  2.0f, nk_rgba(255, 255, 255, 255));
        nk_stroke_line(canvas, gx, gy + 12.0f, gx + 24.0f, gy + 12.0f, 2.0f, nk_rgba(255, 255, 255, 255));
        nk_stroke_line(canvas, gx, gy + 18.0f, gx + 24.0f, gy + 18.0f, 2.0f, nk_rgba(255, 255, 255, 255));

        /* Apps represented by solid, uniform colored squares */

        /* Square 1: Terminal (Solid Red) */
        nk_layout_row_push(ctx, 28);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(255, 0, 0, 255));
        ctx->style.button.hover = nk_style_item_color(nk_rgba(255, 80, 80, 255));
        ctx->style.button.active = nk_style_item_color(nk_rgba(150, 0, 0, 255));
        ctx->style.button.rounding = 4.0f;
        if (nk_button_label(ctx, "")) {
            extern int app_spawn_binary(const char* path);
            app_spawn_binary("/shell.bin");
        }

        /* Square 2: Files (Solid Blue) */
        nk_layout_row_push(ctx, 28);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 0, 255, 255));
        ctx->style.button.hover = nk_style_item_color(nk_rgba(80, 80, 255, 255));
        ctx->style.button.active = nk_style_item_color(nk_rgba(0, 0, 150, 255));
        if (nk_button_label(ctx, "")) {
            app->show_explorer = !app->show_explorer;
        }

        /* Square 3: Diagnostics (Solid Orange) */
        nk_layout_row_push(ctx, 28);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(255, 102, 0, 255));
        ctx->style.button.hover = nk_style_item_color(nk_rgba(255, 150, 50, 255));
        ctx->style.button.active = nk_style_item_color(nk_rgba(180, 70, 0, 255));
        if (nk_button_label(ctx, "")) {
            extern int app_spawn_binary(const char* path);
            app_spawn_binary("/lab.bin");
        }

        /* Square 4: Notepad (Solid Green) */
        nk_layout_row_push(ctx, 28);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 255, 0, 255));
        ctx->style.button.hover = nk_style_item_color(nk_rgba(100, 255, 100, 255));
        ctx->style.button.active = nk_style_item_color(nk_rgba(0, 150, 0, 255));
        if (nk_button_label(ctx, "")) {
            app->show_notepad = !app->show_notepad;
        }

        /* Square 5: Settings (Solid Pink/Magenta) */
        nk_layout_row_push(ctx, 28);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(255, 0, 255, 255));
        ctx->style.button.hover = nk_style_item_color(nk_rgba(255, 100, 255, 255));
        ctx->style.button.active = nk_style_item_color(nk_rgba(150, 0, 150, 255));
        if (nk_button_label(ctx, "")) {
            app->show_settings = !app->show_settings;
        }

        /* Restore standard button style */
        ui_init_style_internal(ctx);

        /* Status tray on far right with flat, bright cyan silhouettes */
        float tray_w = 120.0f;
        float tray_x = (float)ww - tray_w - 15.0f;
        float tray_y = (float)wh - 32.0f;

        /* Checkmark silhouette ✓ */
        nk_stroke_line(canvas, tray_x + 10, tray_y + 12, tray_x + 15, tray_y + 18, 2.0f, nk_rgba(0, 255, 255, 255));
        nk_stroke_line(canvas, tray_x + 15, tray_y + 18, tray_x + 25, tray_y + 6,  2.0f, nk_rgba(0, 255, 255, 255));

        /* Bluetooth silhouette */
        float bt_x = tray_x + 40;
        nk_stroke_line(canvas, bt_x + 6, tray_y + 4,  bt_x + 6,  tray_y + 20, 2.0f, nk_rgba(0, 255, 255, 255));
        nk_stroke_line(canvas, bt_x + 6, tray_y + 4,  bt_x + 12, tray_y + 8,  2.0f, nk_rgba(0, 255, 255, 255));
        nk_stroke_line(canvas, bt_x + 12, tray_y + 8,  bt_x + 6,  tray_y + 12, 2.0f, nk_rgba(0, 255, 255, 255));
        nk_stroke_line(canvas, bt_x + 6, tray_y + 12, bt_x + 12, tray_y + 16, 2.0f, nk_rgba(0, 255, 255, 255));
        nk_stroke_line(canvas, bt_x + 12, tray_y + 16, bt_x + 6,  tray_y + 20, 2.0f, nk_rgba(0, 255, 255, 255));
        nk_stroke_line(canvas, bt_x + 6, tray_y + 4,  bt_x,      tray_y + 8,  2.0f, nk_rgba(0, 255, 255, 255));
        nk_stroke_line(canvas, bt_x + 6, tray_y + 20, bt_x,      tray_y + 16, 2.0f, nk_rgba(0, 255, 255, 255));

        /* Wi-Fi silhouette */
        float wf_x = tray_x + 75;
        nk_stroke_line(canvas, wf_x + 10, tray_y + 18, wf_x + 10, tray_y + 20, 3.0f, nk_rgba(0, 255, 255, 255));
        nk_stroke_line(canvas, wf_x + 4,  tray_y + 14, wf_x + 16, tray_y + 14, 2.0f, nk_rgba(0, 255, 255, 255));
        nk_stroke_line(canvas, wf_x,      tray_y + 8,  wf_x + 20, tray_y + 8,  2.0f, nk_rgba(0, 255, 255, 255));
    }
    nk_end(ctx);
}

static void ui_render_desktop_square(struct nk_context *ctx, struct app_state *app, const char* name, const char* path, float x, float y, struct nk_color color)
{
    struct nk_rect bounds = nk_rect(x, y, 56, 56);
    struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);

    /* Draw uniform solid colored square with slightly rounded corners */
    nk_fill_rect(canvas, bounds, 8, color);

    /* Hover and click handling */
    if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds)) {
        nk_fill_rect(canvas, bounds, 8, nk_rgba(255, 255, 255, 40));
        if (nk_input_is_mouse_pressed(&ctx->input, NK_BUTTON_LEFT)) {
            if (strcmp(name, "files") == 0) {
                app->show_explorer = !app->show_explorer;
            } else if (strcmp(name, "notepad") == 0) {
                app->show_notepad = !app->show_notepad;
            } else if (strcmp(name, "settings") == 0) {
                app->show_settings = !app->show_settings;
            } else if (strcmp(name, "installer") == 0) {
                app->current_state = STATE_INSTALLER;
            } else {
                extern int app_spawn_binary(const char* path);
                app_spawn_binary(path);
            }
        }
    }
}

static void ui_render_background(struct nk_context *ctx, struct app_state *app, int ww, int wh)
{
    struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);

    /* Flat, matte charcoal gray background */
    nk_fill_rect(canvas, nk_rect(0, 0, (float)ww, (float)wh), 0, nk_rgba(51, 51, 51, 255));

    /* Draw the prominent, gorgeous analog vector clock on the far right */
    int h = 0, m = 0, s = 0;
    rtc_get_time(&h, &m, &s);
    float cx = (float)ww - 180.0f;
    float cy = 140.0f;
    float radius = 60.0f;

    nk_stroke_circle(canvas, nk_rect(cx - radius, cy - radius, radius*2, radius*2), 1.0f, nk_rgba(255, 255, 255, 20));
    nk_stroke_line(canvas, cx, cy - radius, cx, cy - radius + 8, 2.0f, nk_rgba(255, 255, 255, 60));
    nk_stroke_line(canvas, cx, cy + radius, cx, cy + radius - 8, 2.0f, nk_rgba(255, 255, 255, 60));
    nk_stroke_line(canvas, cx - radius, cy, cx - radius + 8, cy, 2.0f, nk_rgba(255, 255, 255, 60));
    nk_stroke_line(canvas, cx + radius, cy, cx + radius - 8, cy, 2.0f, nk_rgba(255, 255, 255, 60));

    float hr_angle = ((float)(h % 12) + (float)m / 60.0f) * (2.0f * 3.14159f / 12.0f) - 3.14159f / 2.0f;
    float hr_x = cx + cosf(hr_angle) * (radius * 0.5f);
    float hr_y = cy + sinf(hr_angle) * (radius * 0.5f);
    nk_stroke_line(canvas, cx, cy, hr_x, hr_y, 3.0f, nk_rgba(0, 255, 255, 255));

    float min_angle = (float)m * (2.0f * 3.14159f / 60.0f) - 3.14159f / 2.0f;
    float min_x = cx + cosf(min_angle) * (radius * 0.8f);
    float min_y = cy + sinf(min_angle) * (radius * 0.8f);
    nk_stroke_line(canvas, cx, cy, min_x, min_y, 2.0f, nk_rgba(255, 255, 255, 180));

    float sec_angle = (float)s * (2.0f * 3.14159f / 60.0f) - 3.14159f / 2.0f;
    float sec_x = cx + cosf(sec_angle) * (radius * 0.9f);
    float sec_y = cy + sinf(sec_angle) * (radius * 0.9f);
    nk_stroke_line(canvas, cx, cy, sec_x, sec_y, 1.0f, nk_rgba(255, 0, 0, 255));

    /* Branding - lower case indie style */
    nk_draw_text(canvas, nk_rect(ww - 260, wh - 100, 240, 30), "sovereign rtc64 pro", 19, ctx->style.font, nk_rgba(255, 255, 255, 40), nk_rgba(0,0,0,0));

    /* Grid matrix of perfectly uniform, solid-colored squares with slightly rounded corners on the left side of the screen */
    float start_x = 40.0f;
    float start_y = 40.0f;
    float step = 62.0f;

    /* Row 1 */
    ui_render_desktop_square(ctx, app, "terminal", "/shell.bin", start_x, start_y, nk_rgba(255, 0, 0, 255));
    ui_render_desktop_square(ctx, app, "files", "/files.bin", start_x + step, start_y, nk_rgba(0, 0, 255, 255));
    /* Row 2 */
    ui_render_desktop_square(ctx, app, "diagnostics", "/lab.bin", start_x, start_y + step, nk_rgba(255, 102, 0, 255));
    ui_render_desktop_square(ctx, app, "notepad", "/notepad.bin", start_x + step, start_y + step, nk_rgba(0, 255, 0, 255));
    /* Row 3 */
    ui_render_desktop_square(ctx, app, "settings", "/settings", start_x, start_y + 2 * step, nk_rgba(255, 0, 255, 255));
    ui_render_desktop_square(ctx, app, "installer", "/installer", start_x + step, start_y + 2 * step, nk_rgba(0, 255, 255, 255));
}

static void ui_render_launcher(struct nk_context *ctx, struct app_state *app)
{
    if (!app->show_launcher) return;
    if (nk_begin(ctx, "apps", nk_rect(20, 60, 420, 420), NK_WINDOW_MOVABLE|NK_WINDOW_TITLE)) {
        extern int app_spawn_binary(const char* path);
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, "available applications:", NK_TEXT_LEFT);

        /* Dynamic App Discovery */
        char list_buf[1024];
        if (vfs_ls("/bin", list_buf, sizeof(list_buf)) == 0) {
            nk_layout_row_dynamic(ctx, 36, 4);
            char *line = list_buf;
            while (line && *line) {
                char *next_line = strchr(line, '\n');
                if (next_line) *next_line = '\0';

                if (strlen(line) > 6 && strstr(line, ".bin")) {
                    const char *name = line + 6;
                    char clean_name[32];
                    strncpy(clean_name, name, sizeof(clean_name)-1);
                    char *ext = strstr(clean_name, ".bin");
                    if (ext) *ext = '\0';

                    /* Make text lowercase */
                    for (int i = 0; clean_name[i]; i++) {
                        if (clean_name[i] >= 'A' && clean_name[i] <= 'Z') {
                            clean_name[i] = clean_name[i] - 'A' + 'a';
                        }
                    }

                    if (nk_button_label(ctx, clean_name)) {
                        char full_path[64];
                        snprintf(full_path, sizeof(full_path), "/bin/%s", name);
                        app_spawn_binary(full_path);
                    }
                }
                if (next_line) { *next_line = '\n'; line = next_line + 1; } else line = NULL;
            }
        }

        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, "system utilities:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 36, 4);
        if (nk_button_label(ctx, "files")) app->show_explorer = 1;
        if (nk_button_label(ctx, "tasks")) app->show_task_manager = 1;
        if (nk_button_label(ctx, "preferences")) app->show_settings = 1;
        if (nk_button_label(ctx, "os installer")) app->current_state = STATE_INSTALLER;

        nk_layout_row_dynamic(ctx, 36, 4);
        if (nk_button_label(ctx, "lock screen")) app->current_state = STATE_LOGIN;
        if (nk_button_label(ctx, "close menu")) app->show_launcher = 0;
    }
    if (nk_window_is_closed(ctx, "apps")) app->show_launcher = 0;
    nk_end(ctx);
}

static void ui_render_system_panel(struct nk_context *ctx, struct app_state *app)
{
    if (nk_begin(ctx, "telemetry", nk_rect(1180, 60, 320, 180), NK_WINDOW_TITLE|NK_WINDOW_MOVABLE)) {
        size_t used = hal_malloc_get_used();
        size_t total = hal_malloc_get_total();
        char mem_buf[64];
        snprintf(mem_buf, sizeof(mem_buf), "ram used: %d kb / %d kb", (int)(used / 1024), (int)(total / 1024));
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, mem_buf, NK_TEXT_LEFT);
        nk_label(ctx, "cpu load: minimal", NK_TEXT_LEFT);
        nk_label(ctx, "active user: root", NK_TEXT_LEFT);
        nk_label(ctx, app->show_script_app ? "script app is running" : "script app is idle", NK_TEXT_LEFT);
        if (nk_button_label(ctx, "view crash logs")) {
            app->show_crash_reports = 1;
            const char *p = get_last_crash_path();
            if (p) strncpy(app->last_crash_path, p, sizeof(app->last_crash_path)-1);
        }
    }
    nk_end(ctx);
}

static void ui_render_crash_reports(struct nk_context *ctx, struct app_state *app)
{
    if (!app->show_crash_reports) return;
    if (nk_begin(ctx, "crash logs", nk_rect(360, 140, 640, 420), NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, "system errors history:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 28, 2);
        if (nk_button_label(ctx, "check again")) {
            const char *p = get_last_crash_path();
            if (p) strncpy(app->last_crash_path, p, sizeof(app->last_crash_path)-1);
        }
        if (nk_button_label(ctx, "view report") && app->last_crash_path[0]) {
            char buf[8192];
            if (vfs_cat(app->last_crash_path, buf, sizeof(buf)) == 0) {
                nk_layout_row_dynamic(ctx, 300, 1);
                nk_label_wrap(ctx, buf);
            } else {
                nk_label(ctx, "failed to read report.", NK_TEXT_LEFT);
            }
        }
    }
    if (nk_window_is_closed(ctx, "crash logs")) app->show_crash_reports = 0;
    nk_end(ctx);
}

static void ui_render_files(struct nk_context *ctx, struct app_state *app)
{
    if (nk_begin(ctx, "file explorer", nk_rect(240, 140, 540, 420), NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 26, 1);
        nk_label(ctx, "files list:", NK_TEXT_LEFT);

        nk_layout_row_template_begin(ctx, 30);
        nk_layout_row_template_push_static(ctx, 60);
        nk_layout_row_template_push_dynamic(ctx);
        nk_layout_row_template_end(ctx);

        if (nk_button_label(ctx, "go up")) {
            char *last_slash = strrchr(app->explorer_path, '/');
            if (last_slash && last_slash != app->explorer_path) {
                *last_slash = '\0';
            } else if (last_slash == app->explorer_path) {
                app->explorer_path[1] = '\0';
            }
        }
        nk_label(ctx, app->explorer_path, NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 280, 1);
        if (nk_group_begin(ctx, "FileView", 0)) {
            char list_buf[2048];
            if (vfs_ls(app->explorer_path, list_buf, sizeof(list_buf)) == 0) {
                char *line = list_buf;
                char *next_line;
                while (line && *line) {
                    next_line = strchr(line, '\n');
                    if (next_line) *next_line = '\0';

                    if (strlen(line) > 6) {
                        bool is_dir = (strncmp(line, "<DIR>", 5) == 0);
                        const char *name = line + 6;

                        nk_layout_row_template_begin(ctx, 24);
                        nk_layout_row_template_push_dynamic(ctx);
                        nk_layout_row_template_push_static(ctx, 80);
                        nk_layout_row_template_end(ctx);

                        /* Print files in conversational lowercase */
                        char clean_line[128];
                        strncpy(clean_line, line, sizeof(clean_line)-1);
                        clean_line[sizeof(clean_line)-1] = '\0';
                        for (int i = 0; clean_line[i]; i++) {
                            if (clean_line[i] >= 'A' && clean_line[i] <= 'Z') {
                                clean_line[i] = clean_line[i] - 'A' + 'a';
                            }
                        }

                        if (nk_button_label(ctx, clean_line)) {
                            if (is_dir) {
                                if (app->explorer_path[strlen(app->explorer_path)-1] != '/') {
                                    strncat(app->explorer_path, "/", sizeof(app->explorer_path) - strlen(app->explorer_path) - 1);
                                }
                                strncat(app->explorer_path, name, sizeof(app->explorer_path) - strlen(app->explorer_path) - 1);
                            } else {
                                /* Open in Notepad */
                                char full_path[256];
                                snprintf(full_path, sizeof(full_path), "%s%s%s",
                                         app->explorer_path,
                                         (app->explorer_path[strlen(app->explorer_path)-1] == '/') ? "" : "/",
                                         name);
                                if (vfs_cat(full_path, app->notepad_buffer, sizeof(app->notepad_buffer)) == 0) {
                                    strncpy(app->notepad_file, full_path, sizeof(app->notepad_file)-1);
                                    app->show_notepad = 1;
                                }
                            }
                        }
                        if (nk_button_label(ctx, "destroy")) {
                            char full_path[256];
                            snprintf(full_path, sizeof(full_path), "%s%s%s",
                                     app->explorer_path,
                                     (app->explorer_path[strlen(app->explorer_path)-1] == '/') ? "" : "/",
                                     name);
                            vfs_rm(full_path);
                        }
                    }

                    if (next_line) {
                        *next_line = '\n';
                        line = next_line + 1;
                    } else {
                        line = NULL;
                    }
                }
            } else {
                nk_layout_row_dynamic(ctx, 24, 1);
                nk_label(ctx, "failed to scan directory.", NK_TEXT_LEFT);
            }
            nk_group_end(ctx);
        }
    }
    if (nk_window_is_closed(ctx, "file explorer")) app->show_explorer = 0;
    nk_end(ctx);
}

static void ui_render_settings(struct nk_context *ctx, struct app_state *app)
{
    if (nk_begin(ctx, "preferences", nk_rect(280, 180, 420, 340), NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 28, 1);
        nk_label(ctx, "global preferences", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 28, 1);
        if (nk_checkbox_label(ctx, "allow terminal to browse internet", (nk_bool*)&app->perm_net)) {
            uac_set_permit(0, app->perm_net, app->perm_storage);
        }
        if (nk_checkbox_label(ctx, "allow terminal to edit files", (nk_bool*)&app->perm_storage)) {
            uac_set_permit(0, app->perm_net, app->perm_storage);
        }
        nk_layout_row_dynamic(ctx, 28, 1);
        nk_label(ctx, "signed in as:", NK_TEXT_LEFT);
        nk_label(ctx, app->username, NK_TEXT_LEFT);
    }
    if (nk_window_is_closed(ctx, "preferences")) app->show_settings = 0;
    nk_end(ctx);
}

static void ui_render_task_manager(struct nk_context *ctx, struct app_state *app)
{
    if (!app->show_task_manager) return;
    if (nk_begin(ctx, "task manager", nk_rect(400, 200, 500, 400), NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 30, 4);
        nk_label(ctx, "id", NK_TEXT_LEFT);
        nk_label(ctx, "name", NK_TEXT_LEFT);
        nk_label(ctx, "upid", NK_TEXT_LEFT);
        nk_label(ctx, "state", NK_TEXT_LEFT);

        int count = scheduler_get_task_count();
        for (int i = 0; i < count; i++) {
            task_t* t = scheduler_get_task(i);
            if (!t || t->state == TASK_DEAD) continue;
            nk_layout_row_dynamic(ctx, 24, 4);
            char id_buf[16]; snprintf(id_buf, sizeof(id_buf), "%d", t->id);
            nk_label(ctx, id_buf, NK_TEXT_LEFT);

            /* Print task names in lowercase conversational style */
            char t_name[32];
            strncpy(t_name, t->name, sizeof(t_name)-1);
            t_name[sizeof(t_name)-1] = '\0';
            for (int k = 0; t_name[k]; k++) {
                if (t_name[k] >= 'A' && t_name[k] <= 'Z') t_name[k] = t_name[k] - 'A' + 'a';
            }
            nk_label(ctx, t_name, NK_TEXT_LEFT);

            char upid_buf[16]; snprintf(upid_buf, sizeof(upid_buf), "%u", t->upid);
            nk_label(ctx, upid_buf, NK_TEXT_LEFT);
            nk_label(ctx, t->state == TASK_RUNNING ? "running" : "idle", NK_TEXT_LEFT);
        }
    }
    if (nk_window_is_closed(ctx, "task manager")) app->show_task_manager = 0;
    nk_end(ctx);
}

static void ui_render_notepad(struct nk_context *ctx, struct app_state *app)
{
    if (!app->show_notepad) return;
    if (nk_begin(ctx, "text pad", nk_rect(300, 100, 600, 500), NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 24, 2);
        nk_label(ctx, app->notepad_file[0] ? app->notepad_file : "untitled.txt", NK_TEXT_LEFT);
        if (nk_button_label(ctx, "save file") && app->notepad_file[0]) {
            vfs_write(app->notepad_file, app->notepad_buffer);
        }

        nk_layout_row_dynamic(ctx, 400, 1);
        int len = (int)strlen(app->notepad_buffer);
        nk_edit_string(ctx, NK_EDIT_MULTILINE, app->notepad_buffer, &len, sizeof(app->notepad_buffer)-1, nk_filter_default);
        app->notepad_buffer[len] = '\0';
    }
    if (nk_window_is_closed(ctx, "text pad")) app->show_notepad = 0;
    nk_end(ctx);
}

static void ui_render_desktop(struct nk_context *ctx, struct app_state *app, int ww, int wh)
{
    ui_render_background(ctx, app, ww, wh);
    ui_render_taskbar(ctx, app, ww, wh);
    ui_render_launcher(ctx, app);
    ui_render_system_panel(ctx, app);
    ui_render_crash_reports(ctx, app);
    ui_render_task_manager(ctx, app);

    if (app->show_explorer) ui_render_files(ctx, app);
    if (app->show_settings) ui_render_settings(ctx, app);
    if (app->show_notepad) ui_render_notepad(ctx, app);
}

void ui_render(struct nk_context *ctx, struct app_state *app, int window_width, int window_height)
{
    float ww = (float)window_width;
    float wh = (float)window_height;

    /* Initial Boot Check: Run installer if no user or booting from removable media */
    static bool boot_check_done = false;
    if (!boot_check_done) {
        bool removable_boot = false;
        int dev_count = hal_storage_get_device_count();
        for (int i = 0; i < dev_count; i++) {
            storage_device_t *dev = hal_storage_get_device(i);
            if (dev && (dev->type == STORAGE_TYPE_USB || dev->type == STORAGE_TYPE_SATAPI)) {
                removable_boot = true;
                break;
            }
        }

        char user_check[128];
        bool has_users = (vfs_cat("/etc/passwd", user_check, sizeof(user_check)) == 0);

        if (removable_boot || !has_users) {
            app->current_state = STATE_INSTALLER;
        } else {
            app->current_state = STATE_LOGIN;
        }
        boot_check_done = true;
    }

    if (app->current_state == STATE_LOGIN) {
        if (nk_begin(ctx, "login", nk_rect(ww/2 - 210, wh/2 - 200, 420, 360), NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row_dynamic(ctx, 60, 1);
            nk_label(ctx, "welcome - user. sign in here:", NK_TEXT_CENTERED);
            nk_layout_row_dynamic(ctx, 28, 1);
            nk_label(ctx, "login to start creating things.", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, "username: admin", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(ctx, 34, 1);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->password, sizeof(app->password), nk_filter_default);

            nk_layout_row_dynamic(ctx, 40, 2);
            if (nk_button_label(ctx, "let me in!")) {
                app->current_state = STATE_DESKTOP;
                app->show_launcher = 1;
            }
            if (nk_button_label(ctx, "setup system")) app->current_state = STATE_INSTALLER;
            nk_layout_row_dynamic(ctx, 22, 1);
            nk_label(ctx, "sovereign rtc64: power & simplicity redefined.", NK_TEXT_CENTERED);
        }
        nk_end(ctx);
    } else if (app->current_state == STATE_INSTALLER) {
        if (nk_begin(ctx, "installer", nk_rect(ww/2 - 260, wh/2 - 220, 520, 420), NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_TITLE)) {
            static int install_step = 0;
            static int target_drive = 0;

            if (install_step == 0) {
                nk_layout_row_dynamic(ctx, 28, 1);
                nk_label(ctx, "install sovereign rtc64", NK_TEXT_CENTERED);
                nk_layout_row_dynamic(ctx, 100, 1);
                nk_label_wrap(ctx, "hey, let's install the system! warning: we will format your disk, so back up anything important first.");
                nk_layout_row_dynamic(ctx, 34, 1);
                if (nk_button_label(ctx, "let's go")) install_step = 1;
            } else if (install_step == 1) {
                nk_layout_row_dynamic(ctx, 28, 1);
                nk_label(ctx, "where should we install it?", NK_TEXT_LEFT);
                int dev_count = hal_storage_get_device_count();
                for (int i = 0; i < dev_count; i++) {
                    storage_device_t *dev = hal_storage_get_device(i);
                    if (!dev) continue;
                    char drive_label[64];
                    snprintf(drive_label, sizeof(drive_label), "drive %d: %s (%llu mb)", i, dev->name, (dev->total_blocks * dev->block_size) / (1024*1024));
                    if (nk_option_label(ctx, drive_label, target_drive == i)) target_drive = i;
                }
                nk_layout_row_dynamic(ctx, 34, 2);
                if (nk_button_label(ctx, "go back")) install_step = 0;
                if (nk_button_label(ctx, "write files")) install_step = 2;
            } else if (install_step == 2) {
                nk_layout_row_dynamic(ctx, 28, 1);
                nk_label(ctx, "writing sovereign files...", NK_TEXT_CENTERED);

                static int progress = 0;
                progress++;
                nk_progress(ctx, (nk_size*)&progress, 1000, NK_MODIFIABLE);

                if (progress >= 1000) {
                    /* Create essential system files */
                    vfs_mkdir("/etc");
                    vfs_write("/etc/passwd", "admin:password\n");
                    vfs_mkdir("/bin");
                    vfs_mkdir("/home");
                    vfs_mkdir("/home/Administrator");

                    app->installed = 1;
                    install_step = 3;
                }
            } else if (install_step == 3) {
                nk_layout_row_dynamic(ctx, 28, 1);
                nk_label(ctx, "done! system loaded successfully.", NK_TEXT_CENTERED);
                nk_layout_row_dynamic(ctx, 26, 1);
                nk_label(ctx, "all files written. click finish to boot.", NK_TEXT_CENTERED);
                nk_layout_row_dynamic(ctx, 34, 1);
                if (nk_button_label(ctx, "boot desktop")) {
                    app->current_state = STATE_DESKTOP;
                    app->show_launcher = 1;
                }
            }
        }
        nk_end(ctx);
    } else if (app->current_state == STATE_DESKTOP) {
        ui_render_desktop(ctx, app, window_width, window_height);
    }
}
