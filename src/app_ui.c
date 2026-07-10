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
extern uint64_t hal_get_uptime_ms(void);

struct desktop_app {
    char name[64];
    char path[320];
};

static struct desktop_app apps_on_desktop[25];
static int apps_on_desktop_count = 0;

static void ui_init_style_internal(struct nk_context *ctx)
{
    struct nk_color table[NK_COLOR_COUNT];

    /* Matte dark/charcoal styling with white text */
    table[NK_COLOR_TEXT] = nk_rgba(255, 255, 255, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(30, 30, 30, 217); /* rgba(30,30,30,0.85) => 217/255 */
    table[NK_COLOR_HEADER] = nk_rgba(255, 255, 255, 8); /* rgba(255,255,255,0.03) */
    table[NK_COLOR_BORDER] = nk_rgba(255, 255, 255, 25); /* rgba(255,255,255,0.1) */

    /* Interactive elements: `#3b30e0` base */
    table[NK_COLOR_BUTTON] = nk_rgba(59, 48, 224, 255);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgba(0, 190, 240, 255); /* #00bef0 */
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(0, 190, 240, 255);

    table[NK_COLOR_TOGGLE] = nk_rgba(30, 30, 30, 255);
    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(0, 190, 240, 255);

    table[NK_COLOR_SELECT] = nk_rgba(59, 48, 224, 255);
    table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(0, 190, 240, 255);

    table[NK_COLOR_SLIDER] = nk_rgba(10, 10, 10, 128);
    table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(0, 190, 240, 255);
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(255, 59, 48, 255);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(28, 198, 62, 255);

    table[NK_COLOR_PROPERTY] = nk_rgba(30, 30, 30, 255);
    table[NK_COLOR_EDIT] = nk_rgba(0, 0, 0, 76); /* rgba(0,0,0,0.3) => 76 */
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(0, 190, 240, 255);
    table[NK_COLOR_COMBO] = nk_rgba(30, 30, 30, 255);

    table[NK_COLOR_CHART] = nk_rgba(30, 30, 30, 255);
    table[NK_COLOR_CHART_COLOR] = nk_rgba(0, 190, 240, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 59, 48, 255);

    table[NK_COLOR_SCROLLBAR] = nk_rgba(30, 30, 30, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(59, 48, 224, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(0, 190, 240, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(0, 190, 240, 255);
    table[NK_COLOR_TAB_HEADER] = nk_rgba(30, 30, 30, 255);

    nk_style_from_table(ctx, table);

    /* Enforce 24px rounded squircle borders */
    ctx->style.window.rounding = 24.0f;
    ctx->style.window.border = 1.0f;
    ctx->style.window.header.normal = nk_style_item_color(nk_rgba(255, 255, 255, 8));
    ctx->style.window.header.hover = nk_style_item_color(nk_rgba(255, 255, 255, 15));
    ctx->style.window.header.active = nk_style_item_color(nk_rgba(255, 255, 255, 15));
    ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(30, 30, 30, 217));
    ctx->style.window.background = nk_rgba(30, 30, 30, 217);
    ctx->style.window.border_color = nk_rgba(255, 255, 255, 25);
    ctx->style.window.group_border = 0.0f;
    ctx->style.window.group_border_color = nk_rgba(0, 0, 0, 0);

    /* Tight, compact padding and spacing */
    ctx->style.window.padding = nk_vec2(12, 12);
    ctx->style.window.group_padding = nk_vec2(4, 4);
    ctx->style.window.spacing = nk_vec2(8, 8);

    /* Action buttons: rounded 8px */
    ctx->style.button.rounding = 8.0f;
    ctx->style.button.border = 0.0f;
    ctx->style.button.padding = nk_vec2(6, 6);
}

void ui_init_style(struct nk_context *ctx) {
    ui_init_style_internal(ctx);
}

static void ensure_user_desktop(struct app_state *app) {
    char desktop_path[256];
    char user_home[128];

    vfs_mkdir("/etc");
    vfs_mkdir("/etc/icons");

    /* Write SVGs */
    vfs_write("/etc/icons/terminal.svg", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#ffffff\" stroke-width=\"2\"><polyline points=\"4 17 10 12 4 7\"/><line x1=\"12\" y1=\"19\" x2=\"20\" y2=\"19\"/></svg>");
    vfs_write("/etc/icons/files.svg", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#ffffff\" stroke-width=\"2\"><path d=\"M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z\"/></svg>");
    vfs_write("/etc/icons/telemetry.svg", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#ffffff\" stroke-width=\"2\"><line x1=\"18\" y1=\"20\" x2=\"18\" y2=\"10\"/><line x1=\"12\" y1=\"20\" x2=\"12\" y2=\"4\"/><line x1=\"6\" y1=\"20\" x2=\"6\" y2=\"14\"/></svg>");
    vfs_write("/etc/icons/notepad.svg", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#ffffff\" stroke-width=\"2\"><path d=\"M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z\"/><polyline points=\"14 2 14 8 20 8\"/></svg>");
    vfs_write("/etc/icons/settings.svg", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#ffffff\" stroke-width=\"2\"><circle cx=\"12\" cy=\"12\" r=\"3\"/><path d=\"M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z\"/></svg>");
    vfs_write("/etc/icons/installer.svg", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#ffffff\" stroke-width=\"2\"><circle cx=\"12\" cy=\"12\" r=\"10\"/><circle cx=\"12\" cy=\"12\" r=\"3\"/></svg>");
    vfs_write("/etc/icons/default.svg", "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#ffffff\" stroke-width=\"2\"><rect x=\"3\" y=\"3\" width=\"18\" height=\"18\" rx=\"2\" ry=\"2\"/><rect x=\"7\" y=\"7\" width=\"3\" height=\"3\"/><rect x=\"14\" y=\"7\" width=\"3\" height=\"3\"/><rect x=\"7\" y=\"14\" width=\"3\" height=\"3\"/><rect x=\"14\" y=\"14\" width=\"3\" height=\"3\"/></svg>");

    vfs_mkdir("/home");
    snprintf(user_home, sizeof(user_home), "/home/%s", app->username);
    vfs_mkdir(user_home);
    snprintf(desktop_path, sizeof(desktop_path), "%s/Desktop", user_home);
    vfs_mkdir(desktop_path);

    /* Copy typical apps to user's Desktop if they are not already there */
    char check_path[320];

    snprintf(check_path, sizeof(check_path), "%s/shell.bin", desktop_path);
    vfs_copy_file("/shell.bin", check_path);
    vfs_copy_file("/bin/shell.bin", check_path);

    snprintf(check_path, sizeof(check_path), "%s/lab.bin", desktop_path);
    vfs_copy_file("/lab.bin", check_path);
    vfs_copy_file("/bin/lab.bin", check_path);

    snprintf(check_path, sizeof(check_path), "%s/notepad.bin", desktop_path);
    vfs_copy_file("/notepad.bin", check_path);
    vfs_copy_file("/bin/notepad.bin", check_path);

    snprintf(check_path, sizeof(check_path), "%s/studio.bin", desktop_path);
    vfs_copy_file("/studio.bin", check_path);
    vfs_copy_file("/bin/studio.bin", check_path);

    snprintf(check_path, sizeof(check_path), "%s/tests.bin", desktop_path);
    vfs_copy_file("/tests.bin", check_path);
    vfs_copy_file("/bin/tests.bin", check_path);
}

static void scan_desktop_apps(struct app_state *app) {
    ensure_user_desktop(app);

    apps_on_desktop_count = 0;
    char desktop_path[256];
    snprintf(desktop_path, sizeof(desktop_path), "/home/%s/Desktop", app->username);

    char list_buf[2048];
    if (vfs_ls(desktop_path, list_buf, sizeof(list_buf)) == 0) {
        char *line = list_buf;
        char *next_line;
        while (line && *line && apps_on_desktop_count < 25) {
            next_line = strchr(line, '\n');
            if (next_line) *next_line = '\0';

            if (strlen(line) > 6) {
                bool is_dir = (strncmp(line, "<DIR>", 5) == 0);
                const char *name = line + 6;

                if (!is_dir && strstr(name, ".bin")) {
                    struct desktop_app *da = &apps_on_desktop[apps_on_desktop_count++];
                    strncpy(da->name, name, sizeof(da->name) - 1);
                    snprintf(da->path, sizeof(da->path), "%s/%s", desktop_path, name);
                }
            }

            if (next_line) {
                *next_line = '\n';
                line = next_line + 1;
            } else {
                line = NULL;
            }
        }
    }
}

static void ui_render_taskbar(struct nk_context *ctx, struct app_state *app, int ww, int wh)
{
    /* Center bottom taskbar layout mimicking the floating style */
    float tb_w = (float)ww * 0.95f;
    if (tb_w > 1200.0f) tb_w = 1200.0f;
    float tb_x = ((float)ww - tb_w) / 2.0f;
    float tb_y = (float)wh - 84.0f; /* 20px bottom offset, 64px height */

    /* Ensure we are rendering completely flat transparent window but drawing inside custom background */
    if (nk_begin(ctx, "taskbar", nk_rect(tb_x, tb_y, tb_w, 64.0f), NK_WINDOW_NO_SCROLLBAR)) {
        struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);

        /* Draw the beautiful dark-translucent floating container */
        nk_fill_rect(canvas, nk_rect(tb_x, tb_y, tb_w, 64.0f), 20.0f, nk_rgba(0, 0, 0, 166)); /* rgba(0,0,0,0.65) => 166 */
        nk_stroke_rect(canvas, nk_rect(tb_x, tb_y, tb_w, 64.0f), 20.0f, 1.0f, nk_rgba(255, 255, 255, 20)); /* rgba(255,255,255,0.08) */

        /* Start row for start hamburger menu button and pinned apps */
        nk_layout_row_begin(ctx, NK_STATIC, 48, 12);

        /* Push offset so content is inside padding */
        nk_layout_row_push(ctx, 8);
        nk_label(ctx, "", NK_TEXT_LEFT);

        /* Start Hamburger: 3 white lines */
        nk_layout_row_push(ctx, 36);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
        ctx->style.button.hover = nk_style_item_color(nk_rgba(255, 255, 255, 30));
        ctx->style.button.active = nk_style_item_color(nk_rgba(255, 255, 255, 60));
        ctx->style.button.rounding = 6.0f;
        if (nk_button_label(ctx, "")) {
            app->show_launcher = !app->show_launcher;
        }
        float gx = tb_x + 24.0f;
        float gy = tb_y + 18.0f;
        nk_stroke_line(canvas, gx, gy,       gx + 28.0f, gy,       2.5f, nk_rgba(255, 255, 255, 255));
        nk_stroke_line(canvas, gx, gy + 7.5f,  gx + 28.0f, gy + 7.5f,  2.5f, nk_rgba(255, 255, 255, 255));
        nk_stroke_line(canvas, gx, gy + 15.0f, gx + 28.0f, gy + 15.0f, 2.5f, nk_rgba(255, 255, 255, 255));

        /* App 1: Welcome/Launcher (Red) */
        nk_layout_row_push(ctx, 38);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(255, 59, 48, 255)); /* #ff3b30 */
        ctx->style.button.hover = nk_style_item_color(nk_rgba(255, 100, 100, 255));
        ctx->style.button.rounding = 10.0f;
        if (nk_button_label(ctx, "")) {
            app->show_launcher = !app->show_launcher;
        }

        /* App 2: Terminal (Blue) */
        nk_layout_row_push(ctx, 38);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(59, 48, 224, 255)); /* #3b30e0 */
        ctx->style.button.hover = nk_style_item_color(nk_rgba(100, 100, 255, 255));
        if (nk_button_label(ctx, "")) {
            extern int app_spawn_binary(const char* path);
            app_spawn_binary("/shell.bin");
        }

        /* App 3: Files (Orange) */
        nk_layout_row_push(ctx, 38);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(255, 142, 40, 255)); /* #ff8e28 */
        ctx->style.button.hover = nk_style_item_color(nk_rgba(255, 180, 100, 255));
        if (nk_button_label(ctx, "")) {
            app->show_explorer = !app->show_explorer;
        }

        /* App 4: Telemetry (Light Blue) */
        nk_layout_row_push(ctx, 38);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 190, 240, 255)); /* #00bef0 */
        ctx->style.button.hover = nk_style_item_color(nk_rgba(100, 220, 255, 255));
        if (nk_button_label(ctx, "")) {
            extern int app_spawn_binary(const char* path);
            app_spawn_binary("/lab.bin");
        }

        /* Restore standard button style */
        ui_init_style_internal(ctx);

        /* Cyan Status Tray with beautiful vector icons on the far right */
        float tray_w = 120.0f;
        float tray_x = tb_x + tb_w - tray_w - 20.0f;
        float tray_y = tb_y + 16.0f;

        /* Checkmark icon✓ */
        nk_stroke_line(canvas, tray_x + 10, tray_y + 16, tray_x + 15, tray_y + 22, 2.5f, nk_rgba(0, 190, 240, 217)); /* #00bef0 with high opacity */
        nk_stroke_line(canvas, tray_x + 15, tray_y + 22, tray_x + 25, tray_y + 10, 2.5f, nk_rgba(0, 190, 240, 217));

        /* Bluetooth icon */
        float bt_x = tray_x + 45;
        nk_stroke_line(canvas, bt_x + 6, tray_y + 8,  bt_x + 6,  tray_y + 24, 2.0f, nk_rgba(0, 190, 240, 217));
        nk_stroke_line(canvas, bt_x + 6, tray_y + 8,  bt_x + 12, tray_y + 12, 2.0f, nk_rgba(0, 190, 240, 217));
        nk_stroke_line(canvas, bt_x + 12, tray_y + 12, bt_x + 6,  tray_y + 16, 2.0f, nk_rgba(0, 190, 240, 217));
        nk_stroke_line(canvas, bt_x + 6, tray_y + 16, bt_x + 12, tray_y + 20, 2.0f, nk_rgba(0, 190, 240, 217));
        nk_stroke_line(canvas, bt_x + 12, tray_y + 20, bt_x + 6,  tray_y + 24, 2.0f, nk_rgba(0, 190, 240, 217));
        nk_stroke_line(canvas, bt_x + 6, tray_y + 8,  bt_x,      tray_y + 12, 2.0f, nk_rgba(0, 190, 240, 217));
        nk_stroke_line(canvas, bt_x + 6, tray_y + 24, bt_x,      tray_y + 20, 2.0f, nk_rgba(0, 190, 240, 217));

        /* Wi-Fi icon */
        float wf_x = tray_x + 80;
        nk_stroke_arc(canvas, wf_x + 10, tray_y + 22, 12.0f, -0.7f, -2.44f, 2.0f, nk_rgba(0, 190, 240, 217));
        nk_stroke_arc(canvas, wf_x + 10, tray_y + 22, 7.0f, -0.7f, -2.44f, 2.0f, nk_rgba(0, 190, 240, 217));
        nk_fill_circle(canvas, nk_rect(wf_x + 8, tray_y + 20, 4, 4), nk_rgba(0, 190, 240, 217));
    }
    nk_end(ctx);
}

static const char* get_app_icon_path(const char* name) {
    if (strstr(name, "shell") || strstr(name, "terminal")) {
        return "/etc/icons/terminal.svg";
    } else if (strstr(name, "file") || strstr(name, "explorer")) {
        return "/etc/icons/files.svg";
    } else if (strstr(name, "lab") || strstr(name, "telemetry")) {
        return "/etc/icons/telemetry.svg";
    } else if (strstr(name, "notepad") || strstr(name, "text")) {
        return "/etc/icons/notepad.svg";
    } else if (strstr(name, "studio") || strstr(name, "settings") || strstr(name, "preferences")) {
        return "/etc/icons/settings.svg";
    } else if (strstr(name, "welcome") || strstr(name, "installer")) {
        return "/etc/icons/installer.svg";
    }
    return "/etc/icons/default.svg";
}

static void ui_render_desktop_square(struct nk_context *ctx, struct app_state *app, const char* name, const char* path, float x, float y, struct nk_color color)
{
    struct nk_rect bounds = nk_rect(x, y, 65, 65);
    struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);

    /* Draw container with aggressive rounding 14px like CSS `.desktop-app-square { border-radius: 14px; }` */
    /* Beautiful dark-translucent block container */
    nk_fill_rect(canvas, bounds, 14, nk_rgba(30, 30, 30, 200));
    nk_stroke_rect(canvas, bounds, 14, 1.0f, nk_rgba(255, 255, 255, 30));

    /* Load and draw the SVG icon! */
    const char* icon_path = get_app_icon_path(name);
    struct nk_image img = ui_icon_load_svg(name, icon_path, 36, 36);
    if (img.handle.ptr) {
        /* Centered inside 65x65 bounds (offset of 14.5px from top-left) */
        nk_draw_image(canvas, nk_rect(x + 14.5f, y + 14.5f, 36.0f, 36.0f), &img, nk_rgba(255, 255, 255, 255));
    } else {
        /* Fallback: draw a small centered circle/rect using the row-color */
        nk_fill_rect(canvas, nk_rect(x + 14.5f, y + 14.5f, 36.0f, 36.0f), 6, color);
    }

    /* Hover and Click actions */
    if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds)) {
        nk_fill_rect(canvas, bounds, 14, nk_rgba(255, 255, 255, 40));
        if (nk_input_is_mouse_pressed(&ctx->input, NK_BUTTON_LEFT)) {
            if (strcmp(name, "files") == 0) {
                app->show_explorer = !app->show_explorer;
            } else if (strcmp(name, "notepad") == 0) {
                app->show_notepad = !app->show_notepad;
            } else if (strcmp(name, "settings") == 0) {
                app->show_settings = !app->show_settings;
            } else if (strcmp(name, "welcome") == 0) {
                app->show_launcher = !app->show_launcher;
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

    /* Dynamic desktop scanning */
    static uint32_t last_scan_time = 0;
    uint32_t now_ms = (uint32_t)hal_get_uptime_ms();
    if (apps_on_desktop_count == 0 || (now_ms - last_scan_time) > 2000) {
        scan_desktop_apps(app);
        last_scan_time = now_ms;
    }

    /* Draw smooth vertical color gradient representing linear-gradient(135deg, #1e293b 0%, #0f172a 100%) */
    int steps = 64;
    float bar_h = (float)wh / (float)steps;
    for (int i = 0; i < steps; i++) {
        float ratio = (float)i / (float)steps;
        int r = (int)(30.0f + (15.0f - 30.0f) * ratio);
        int g = (int)(41.0f + (23.0f - 41.0f) * ratio);
        int b = (int)(59.0f + (42.0f - 59.0f) * ratio);
        nk_fill_rect(canvas, nk_rect(0, (float)i * bar_h, (float)ww, bar_h + 1.0f), 0, nk_rgba(r, g, b, 255));
    }

    /* Large background digital clock with Eastern Standard Zone info */
    int h = 0, m = 0, s = 0;
    rtc_get_time(&h, &m, &s);
    char time_str[32];
    snprintf(time_str, sizeof(time_str), "%02d:%02d", h, m);

    /* Render clock text on top right */
    float cl_x = (float)ww * 0.9f - 180.0f;
    float cl_y = 60.0f;
    nk_draw_text(canvas, nk_rect(cl_x - 300.0f, cl_y, 480.0f, 130.0f), time_str, (int)strlen(time_str), ctx->style.font, nk_rgba(255, 255, 255, 217), nk_rgba(0,0,0,0));
    nk_draw_text(canvas, nk_rect(cl_x - 300.0f, cl_y + 135.0f, 480.0f, 30.0f), "eastern standard (+3:00)", 24, ctx->style.font, nk_rgba(0, 190, 240, 204), nk_rgba(0,0,0,0));

    /* 5x5 Grid matrix layout of Apps exactly matching HTML spec */
    float grid_x = 40.0f;
    float grid_y = 50.0f;
    float box_sz = 65.0f;
    float gap = 12.0f;

    /* Top Arrow label */
    nk_draw_text(canvas, nk_rect(grid_x + 5 * box_sz + 4 * gap - 20, grid_y - 25, 20, 20), "v", 1, ctx->style.font, nk_rgba(255, 255, 255, 102), nk_rgba(0,0,0,0));

    /* Row colors mapping based on row index */
    struct nk_color row_colors[5] = {
        nk_rgba(255, 59, 48, 255),  /* Red */
        nk_rgba(59, 48, 224, 255),  /* Blue */
        nk_rgba(255, 142, 40, 255), /* Orange */
        nk_rgba(0, 190, 240, 255),  /* Light Blue */
        nk_rgba(28, 198, 94, 255)   /* Green */
    };

    /* Populate the grid dynamically using apps discovered in Desktop folder */
    for (int i = 0; i < apps_on_desktop_count; i++) {
        int row = i / 5;
        int col = i % 5;
        float x = grid_x + col * (box_sz + gap);
        float y = grid_y + row * (box_sz + gap);

        struct nk_color color = row_colors[row % 5];

        ui_render_desktop_square(ctx, app, apps_on_desktop[i].name, apps_on_desktop[i].path, x, y, color);
    }
}

static void ui_render_launcher(struct nk_context *ctx, struct app_state *app)
{
    if (!app->show_launcher) return;
    /* Anchored under the clock / on right, beautifully overlaying welcoming updates */
    if (nk_begin(ctx, "welcome - user", nk_rect(420, 220, 400, 320), NK_WINDOW_MOVABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, "your recent apps:", NK_TEXT_LEFT);

        nk_layout_row_begin(ctx, NK_STATIC, 35, 6);
        nk_layout_row_push(ctx, 20);
        nk_label(ctx, "<", NK_TEXT_CENTERED);

        /* Colored mini squares */
        nk_layout_row_push(ctx, 35);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(255, 59, 48, 255));
        nk_button_label(ctx, "");

        nk_layout_row_push(ctx, 35);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(59, 48, 224, 255));
        nk_button_label(ctx, "");

        nk_layout_row_push(ctx, 35);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(255, 142, 40, 255));
        nk_button_label(ctx, "");

        nk_layout_row_push(ctx, 35);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 190, 240, 255));
        nk_button_label(ctx, "");

        nk_layout_row_push(ctx, 20);
        nk_label(ctx, ">", NK_TEXT_CENTERED);
        nk_layout_row_end(ctx);

        ui_init_style_internal(ctx);

        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, "updates:", NK_TEXT_LEFT);

        nk_layout_row_begin(ctx, NK_STATIC, 35, 6);
        nk_layout_row_push(ctx, 20);
        nk_label(ctx, "<", NK_TEXT_CENTERED);

        /* Colored mini squares in reverse/different layout */
        nk_layout_row_push(ctx, 35);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 190, 240, 255));
        nk_button_label(ctx, "");

        nk_layout_row_push(ctx, 35);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(255, 142, 40, 255));
        nk_button_label(ctx, "");

        nk_layout_row_push(ctx, 35);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(59, 48, 224, 255));
        nk_button_label(ctx, "");

        nk_layout_row_push(ctx, 35);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(255, 59, 48, 255));
        nk_button_label(ctx, "");

        nk_layout_row_push(ctx, 20);
        nk_label(ctx, ">", NK_TEXT_CENTERED);
        nk_layout_row_end(ctx);

        ui_init_style_internal(ctx);
    }
    if (nk_window_is_closed(ctx, "welcome - user")) app->show_launcher = 0;
    nk_end(ctx);
}

static void ui_render_system_panel(struct nk_context *ctx, struct app_state *app)
{
    /* Keep active telemetry but in boutique styled window with action-btn */
    if (nk_begin(ctx, "system telemetry", nk_rect(600, 240, 400, 280), NK_WINDOW_TITLE|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)) {
        size_t used = hal_malloc_get_used();
        size_t total = hal_malloc_get_total();
        char mem_buf[64];
        snprintf(mem_buf, sizeof(mem_buf), "ram availability: %d mb / %d mb", (int)(used / (1024*1024)), (int)(total / (1024*1024)));

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "live parameters:", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 70, 1);
        if (nk_group_begin(ctx, "telemetry_block", 0)) {
            nk_layout_row_dynamic(ctx, 18, 1);
            nk_label(ctx, "cpu allocation: 2.4% execution frame", NK_TEXT_LEFT);
            nk_label(ctx, mem_buf, NK_TEXT_LEFT);
            nk_label(ctx, "active user: root", NK_TEXT_LEFT);
            nk_group_end(ctx);
        }

        nk_layout_row_dynamic(ctx, 32, 1);
        if (nk_button_label(ctx, "extract stack telemetry")) {
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
    if (nk_begin(ctx, "file explorer", nk_rect(550, 200, 400, 300), NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 26, 1);
        nk_label(ctx, "index: root/vfs/bin", NK_TEXT_LEFT);

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

        nk_layout_row_dynamic(ctx, 140, 1);
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
    if (nk_begin(ctx, "system preferences", nk_rect(650, 280, 400, 260), NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "workspace adjustments:", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 100, 1);
        int len = (int)strlen(app->notepad_buffer);
        nk_edit_string(ctx, NK_EDIT_MULTILINE, app->notepad_buffer, &len, sizeof(app->notepad_buffer)-1, nk_filter_default);
        app->notepad_buffer[len] = '\0';

        nk_layout_row_dynamic(ctx, 32, 1);
        if (nk_button_label(ctx, "commit edits")) {
            uac_set_permit(0, app->perm_net, app->perm_storage);
        }
    }
    if (nk_window_is_closed(ctx, "system preferences")) app->show_settings = 0;
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
