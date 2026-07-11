/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license.
 * Modified by Sovereign for GNOME Adwaita Dark styling. */
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

    /* Official GNOME Adwaita Dark color palette */
    table[NK_COLOR_TEXT] = nk_rgba(255, 255, 255, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(36, 36, 36, 255); /* Adwaita Dark window background #242424 */
    table[NK_COLOR_HEADER] = nk_rgba(48, 48, 48, 255); /* Adwaita Dark titlebar #303030 */
    table[NK_COLOR_BORDER] = nk_rgba(48, 48, 48, 0); /* Borderless headers */

    /* Adwaita Accent Blue: `#3584e4` (53, 132, 228) */
    table[NK_COLOR_BUTTON] = nk_rgba(53, 132, 228, 255);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgba(74, 144, 235, 255); /* Lighter blue */
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(38, 105, 196, 255); /* Darker blue */

    table[NK_COLOR_TOGGLE] = nk_rgba(48, 48, 48, 255);
    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(53, 132, 228, 255);

    table[NK_COLOR_SELECT] = nk_rgba(53, 132, 228, 255);
    table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(38, 105, 196, 255);

    table[NK_COLOR_SLIDER] = nk_rgba(30, 30, 30, 255);
    table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(53, 132, 228, 255);
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(74, 144, 235, 255);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(38, 105, 196, 255);

    table[NK_COLOR_PROPERTY] = nk_rgba(48, 48, 48, 255);
    table[NK_COLOR_EDIT] = nk_rgba(30, 30, 30, 255);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(53, 132, 228, 255);
    table[NK_COLOR_COMBO] = nk_rgba(48, 48, 48, 255);

    table[NK_COLOR_CHART] = nk_rgba(36, 36, 36, 255);
    table[NK_COLOR_CHART_COLOR] = nk_rgba(53, 132, 228, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(224, 27, 36, 255); /* Adwaita Red */

    table[NK_COLOR_SCROLLBAR] = nk_rgba(36, 36, 36, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(53, 132, 228, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(74, 144, 235, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(38, 105, 196, 255);
    table[NK_COLOR_TAB_HEADER] = nk_rgba(48, 48, 48, 255);

    nk_style_from_table(ctx, table);

    /* Enforce modern Adwaita 12px rounding on windows & dialogs */
    ctx->style.window.rounding = 12.0f;
    ctx->style.window.border = 1.0f;
    ctx->style.window.header.normal = nk_style_item_color(nk_rgba(48, 48, 48, 255));
    ctx->style.window.header.hover = nk_style_item_color(nk_rgba(58, 58, 58, 255));
    ctx->style.window.header.active = nk_style_item_color(nk_rgba(58, 58, 58, 255));
    ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(36, 36, 36, 255));
    ctx->style.window.background = nk_rgba(36, 36, 36, 255);
    ctx->style.window.border_color = nk_rgba(48, 48, 48, 255);
    ctx->style.window.group_border = 1.0f;
    ctx->style.window.group_border_color = nk_rgba(48, 48, 48, 255);

    /* Adwaita spacing guidelines */
    ctx->style.window.padding = nk_vec2(16, 16);
    ctx->style.window.group_padding = nk_vec2(12, 12);
    ctx->style.window.spacing = nk_vec2(10, 10);

    /* Clean, rounded Adwaita button styling (8px) */
    ctx->style.button.rounding = 8.0f;
    ctx->style.button.border = 0.0f;
    ctx->style.button.padding = nk_vec2(8, 8);
}

void ui_init_style(struct nk_context *ctx) {
    ui_init_style_internal(ctx);
}

static bool verify_user_credentials(const char* username, const char* password) {
    char buf[2048];
    if (vfs_cat("/etc/passwd", buf, sizeof(buf)) != 0) return false;

    char *line = buf;
    while (line && *line) {
        char *next_line = strchr(line, '\n');
        if (next_line) *next_line = '\0';

        char *colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            char *p_user = line;
            char *p_pass = colon + 1;

            if (strcmp(p_user, username) == 0 && strcmp(p_pass, password) == 0) {
                return true;
            }
        }

        if (next_line) {
            *next_line = '\n';
            line = next_line + 1;
        } else {
            line = NULL;
        }
    }
    return false;
}

static void add_user_account(const char* username, const char* password) {
    char buf[2048];
    buf[0] = '\0';
    vfs_cat("/etc/passwd", buf, sizeof(buf));

    char new_line[128];
    snprintf(new_line, sizeof(new_line), "%s:%s\n", username, password);

    if (strlen(buf) + strlen(new_line) < sizeof(buf) - 1) {
        strcat(buf, new_line);
        vfs_write("/etc/passwd", buf);
    }
}

static void ensure_user_desktop(struct app_state *app) {
    char desktop_path[320];
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
    /* Float GNOME-style top or bottom floating panel */
    float tb_w = (float)ww * 0.95f;
    if (tb_w > 1200.0f) tb_w = 1200.0f;
    float tb_x = ((float)ww - tb_w) / 2.0f;
    float tb_y = (float)wh - 84.0f; /* 20px off bottom, 64px height */

    if (nk_begin(ctx, "taskbar", nk_rect(tb_x, tb_y, tb_w, 64.0f), NK_WINDOW_NO_SCROLLBAR)) {
        struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);

        /* GNOME Adwaita Dark: solid dark gray header #303030 with clean border and elegant 12px rounding */
        nk_fill_rect(canvas, nk_rect(tb_x, tb_y, tb_w, 64.0f), 12.0f, nk_rgba(48, 48, 48, 240));
        nk_stroke_rect(canvas, nk_rect(tb_x, tb_y, tb_w, 64.0f), 12.0f, 1.0f, nk_rgba(60, 60, 60, 255));

        nk_layout_row_begin(ctx, NK_STATIC, 48, 12);

        /* Padding */
        nk_layout_row_push(ctx, 12);
        nk_label(ctx, "", NK_TEXT_LEFT);

        /* Start hamburger start menu (White, modern) */
        nk_layout_row_push(ctx, 36);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 0, 0, 0));
        ctx->style.button.hover = nk_style_item_color(nk_rgba(255, 255, 255, 20));
        ctx->style.button.active = nk_style_item_color(nk_rgba(255, 255, 255, 40));
        ctx->style.button.rounding = 6.0f;
        if (nk_button_label(ctx, "")) {
            app->show_launcher = !app->show_launcher;
        }
        float gx = tb_x + 24.0f;
        float gy = tb_y + 20.0f;
        nk_stroke_line(canvas, gx, gy,       gx + 24.0f, gy,       2.0f, nk_rgba(255, 255, 255, 255));
        nk_stroke_line(canvas, gx, gy + 6.0f,  gx + 24.0f, gy + 6.0f,  2.0f, nk_rgba(255, 255, 255, 255));
        nk_stroke_line(canvas, gx, gy + 12.0f, gx + 24.0f, gy + 12.0f, 2.0f, nk_rgba(255, 255, 255, 255));

        /* App 1: Welcome (Red, customized to fit Adwaita grid) */
        nk_layout_row_push(ctx, 38);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(224, 27, 36, 255)); /* Adwaita Red */
        ctx->style.button.hover = nk_style_item_color(nk_rgba(235, 74, 81, 255));
        ctx->style.button.rounding = 8.0f;
        if (nk_button_label(ctx, "")) {
            app->show_launcher = !app->show_launcher;
        }

        /* App 2: Terminal (Accent Blue) */
        nk_layout_row_push(ctx, 38);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(53, 132, 228, 255)); /* Adwaita Blue */
        ctx->style.button.hover = nk_style_item_color(nk_rgba(74, 144, 235, 255));
        if (nk_button_label(ctx, "")) {
            extern int app_spawn_binary(const char* path);
            app_spawn_binary("/shell.bin");
        }

        /* App 3: Files (Orange) */
        nk_layout_row_push(ctx, 38);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(255, 120, 0, 255)); /* Adwaita Orange */
        ctx->style.button.hover = nk_style_item_color(nk_rgba(255, 150, 50, 255));
        if (nk_button_label(ctx, "")) {
            app->show_explorer = !app->show_explorer;
        }

        /* App 4: Diagnostics (Light Blue) */
        nk_layout_row_push(ctx, 38);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 190, 240, 255));
        ctx->style.button.hover = nk_style_item_color(nk_rgba(100, 220, 255, 255));
        if (nk_button_label(ctx, "")) {
            extern int app_spawn_binary(const char* path);
            app_spawn_binary("/lab.bin");
        }

        /* Restore standard button style */
        ui_init_style_internal(ctx);

        /* Cyan/White Status Tray on far right with clean silhouettes */
        float tray_w = 120.0f;
        float tray_x = tb_x + tb_w - tray_w - 20.0f;
        float tray_y = tb_y + 16.0f;

        /* Checkmark icon✓ */
        nk_stroke_line(canvas, tray_x + 10, tray_y + 16, tray_x + 15, tray_y + 22, 2.0f, nk_rgba(255, 255, 255, 200));
        nk_stroke_line(canvas, tray_x + 15, tray_y + 22, tray_x + 25, tray_y + 10, 2.0f, nk_rgba(255, 255, 255, 200));

        /* Bluetooth icon */
        float bt_x = tray_x + 45;
        nk_stroke_line(canvas, bt_x + 6, tray_y + 8,  bt_x + 6,  tray_y + 24, 2.0f, nk_rgba(255, 255, 255, 200));
        nk_stroke_line(canvas, bt_x + 6, tray_y + 8,  bt_x + 12, tray_y + 12, 2.0f, nk_rgba(255, 255, 255, 200));
        nk_stroke_line(canvas, bt_x + 12, tray_y + 12, bt_x + 6,  tray_y + 16, 2.0f, nk_rgba(255, 255, 255, 200));
        nk_stroke_line(canvas, bt_x + 6, tray_y + 16, bt_x + 12, tray_y + 20, 2.0f, nk_rgba(255, 255, 255, 200));
        nk_stroke_line(canvas, bt_x + 12, tray_y + 20, bt_x + 6,  tray_y + 24, 2.0f, nk_rgba(255, 255, 255, 200));
        nk_stroke_line(canvas, bt_x + 6, tray_y + 8,  bt_x,      tray_y + 12, 2.0f, nk_rgba(255, 255, 255, 200));
        nk_stroke_line(canvas, bt_x + 6, tray_y + 24, bt_x,      tray_y + 20, 2.0f, nk_rgba(255, 255, 255, 200));

        /* Wi-Fi icon */
        float wf_x = tray_x + 80;
        nk_stroke_arc(canvas, wf_x + 10, tray_y + 22, 12.0f, -0.7f, -2.44f, 2.0f, nk_rgba(255, 255, 255, 200));
        nk_stroke_arc(canvas, wf_x + 10, tray_y + 22, 7.0f, -0.7f, -2.44f, 2.0f, nk_rgba(255, 255, 255, 200));
        nk_fill_circle(canvas, nk_rect(wf_x + 8, tray_y + 20, 4, 4), nk_rgba(255, 255, 255, 200));
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

    /* Clean Adwaita 12px rounding on squares */
    /* Draw standard translucent background */
    nk_fill_rect(canvas, bounds, 12, nk_rgba(40, 40, 40, 220));
    nk_stroke_rect(canvas, bounds, 12, 1.0f, nk_rgba(60, 60, 60, 255));

    /* Load and draw the SVG icon! */
    const char* icon_path = get_app_icon_path(name);
    struct nk_image img = ui_icon_load_svg(name, icon_path, 36, 36);
    if (img.handle.ptr) {
        nk_draw_image(canvas, nk_rect(x + 14.5f, y + 14.5f, 36.0f, 36.0f), &img, nk_rgba(255, 255, 255, 255));
    } else {
        nk_fill_rect(canvas, nk_rect(x + 14.5f, y + 14.5f, 36.0f, 36.0f), 6, color);
    }

    /* Hover and Click actions */
    if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds)) {
        nk_fill_rect(canvas, bounds, 12, nk_rgba(255, 255, 255, 30));
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

    /* Draw signature GNOME Adwaita background geometric/striped wallpaper! */
    /* Deep blue to dark indigo-purple transition layout */
    int steps = 120;
    float bar_h = (float)wh / (float)steps;
    for (int i = 0; i < steps; i++) {
        float ratio = (float)i / (float)steps;
        int r = (int)(26.0f + (15.0f - 26.0f) * ratio);
        int g = (int)(51.0f + (23.0f - 51.0f) * ratio);
        int b = (int)(116.0f + (42.0f - 116.0f) * ratio);
        nk_fill_rect(canvas, nk_rect(0, (float)i * bar_h, (float)ww, bar_h + 1.0f), 0, nk_rgba(r, g, b, 255));
    }

    /* Draw subtle geometric dark polygonal shapes to look like high-fidelity default GNOME wallpaper */
    float poly1[] = {
        (float)ww * 0.1f, 0.0f,
        (float)ww * 0.4f, 0.0f,
        (float)ww * 0.2f, (float)wh * 0.6f
    };
    nk_fill_polygon(canvas, poly1, 3, nk_rgba(40, 75, 170, 45));

    float poly2[] = {
        (float)ww * 0.6f, (float)wh,
        (float)ww * 0.9f, (float)wh,
        (float)ww * 0.75f, (float)wh * 0.3f
    };
    nk_fill_polygon(canvas, poly2, 3, nk_rgba(45, 80, 190, 45));

    /* Elegant, crisp digital clock centered or right-aligned */
    int h = 0, m = 0, s = 0;
    rtc_get_time(&h, &m, &s);
    char time_str[32];
    snprintf(time_str, sizeof(time_str), "%02d:%02d", h, m);

    float cl_x = (float)ww * 0.9f - 180.0f;
    float cl_y = 60.0f;
    nk_draw_text(canvas, nk_rect(cl_x - 300.0f, cl_y, 480.0f, 130.0f), time_str, (int)strlen(time_str), ctx->style.font, nk_rgba(255, 255, 255, 225), nk_rgba(0,0,0,0));
    nk_draw_text(canvas, nk_rect(cl_x - 300.0f, cl_y + 135.0f, 480.0f, 30.0f), "eastern standard (+3:00)", 24, ctx->style.font, nk_rgba(154, 195, 245, 230), nk_rgba(0,0,0,0));

    /* Monolithic 5x5 Grid matrix layout of Apps */
    float grid_x = 40.0f;
    float grid_y = 50.0f;
    float box_sz = 65.0f;
    float gap = 12.0f;

    /* Top Arrow label */
    nk_draw_text(canvas, nk_rect(grid_x + 5 * box_sz + 4 * gap - 20, grid_y - 25, 20, 20), "v", 1, ctx->style.font, nk_rgba(255, 255, 255, 102), nk_rgba(0,0,0,0));

    struct nk_color row_colors[5] = {
        nk_rgba(224, 27, 36, 255),  /* Adwaita Red */
        nk_rgba(53, 132, 228, 255), /* Adwaita Blue */
        nk_rgba(255, 120, 0, 255),  /* Adwaita Orange */
        nk_rgba(0, 190, 240, 255),  /* Light Blue */
        nk_rgba(46, 194, 126, 255)  /* Adwaita Green */
    };

    /* Populate grid */
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
    if (nk_begin(ctx, "welcome - user", nk_rect(420, 220, 400, 320), NK_WINDOW_MOVABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, "your recent apps:", NK_TEXT_LEFT);

        nk_layout_row_begin(ctx, NK_STATIC, 35, 6);
        nk_layout_row_push(ctx, 20);
        nk_label(ctx, "<", NK_TEXT_CENTERED);

        nk_layout_row_push(ctx, 35);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(224, 27, 36, 255));
        nk_button_label(ctx, "");

        nk_layout_row_push(ctx, 35);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(53, 132, 228, 255));
        nk_button_label(ctx, "");

        nk_layout_row_push(ctx, 35);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(255, 120, 0, 255));
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

        nk_layout_row_push(ctx, 35);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 190, 240, 255));
        nk_button_label(ctx, "");

        nk_layout_row_push(ctx, 35);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(255, 120, 0, 255));
        nk_button_label(ctx, "");

        nk_layout_row_push(ctx, 35);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(53, 132, 228, 255));
        nk_button_label(ctx, "");

        nk_layout_row_push(ctx, 35);
        ctx->style.button.normal = nk_style_item_color(nk_rgba(224, 27, 36, 255));
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

    /* Initial Boot Check */
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
        if (nk_begin(ctx, "login", nk_rect(ww/2 - 210, wh/2 - 220, 420, 440), NK_WINDOW_NO_SCROLLBAR)) {
            static bool signup_mode = false;
            static char signup_user[32] = "";
            static char signup_pass[32] = "";

            if (!signup_mode) {
                nk_layout_row_dynamic(ctx, 40, 1);
                nk_label(ctx, "welcome - user. sign in here:", NK_TEXT_CENTERED);
                nk_layout_row_dynamic(ctx, 24, 1);
                nk_label(ctx, "login to start creating things.", NK_TEXT_CENTERED);

                nk_layout_row_dynamic(ctx, 28, 1);
                nk_label(ctx, "username:", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 32, 1);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->username, sizeof(app->username) - 1, nk_filter_default);

                nk_layout_row_dynamic(ctx, 28, 1);
                nk_label(ctx, "password:", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 32, 1);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->password, sizeof(app->password) - 1, nk_filter_default);

                nk_layout_row_dynamic(ctx, 36, 2);
                if (nk_button_label(ctx, "let me in!")) {
                    if (verify_user_credentials(app->username, app->password)) {
                        app->current_state = STATE_DESKTOP;
                        app->show_launcher = 1;
                    }
                }
                if (nk_button_label(ctx, "create account")) {
                    signup_mode = true;
                }
            } else {
                nk_layout_row_dynamic(ctx, 40, 1);
                nk_label(ctx, "create new account", NK_TEXT_CENTERED);
                nk_layout_row_dynamic(ctx, 24, 1);
                nk_label(ctx, "join the sovereign matrix.", NK_TEXT_CENTERED);

                nk_layout_row_dynamic(ctx, 28, 1);
                nk_label(ctx, "new username:", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 32, 1);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, signup_user, sizeof(signup_user) - 1, nk_filter_default);

                nk_layout_row_dynamic(ctx, 28, 1);
                nk_label(ctx, "new password:", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 32, 1);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, signup_pass, sizeof(signup_pass) - 1, nk_filter_default);

                nk_layout_row_dynamic(ctx, 36, 2);
                if (nk_button_label(ctx, "register & login")) {
                    if (strlen(signup_user) > 0 && strlen(signup_pass) > 0) {
                        add_user_account(signup_user, signup_pass);
                        strncpy(app->username, signup_user, sizeof(app->username) - 1);
                        strncpy(app->password, signup_pass, sizeof(app->password) - 1);

                        ensure_user_desktop(app);

                        app->current_state = STATE_DESKTOP;
                        app->show_launcher = 1;
                    }
                }
                if (nk_button_label(ctx, "go back")) {
                    signup_mode = false;
                }
            }

            nk_layout_row_dynamic(ctx, 40, 1);
            if (nk_button_label(ctx, "setup system via installer")) app->current_state = STATE_INSTALLER;

            nk_layout_row_dynamic(ctx, 22, 1);
            nk_label(ctx, "sovereign rtc64: power & simplicity redefined.", NK_TEXT_CENTERED);
        }
        nk_end(ctx);
    } else if (app->current_state == STATE_INSTALLER) {
        if (nk_begin(ctx, "installer", nk_rect(ww/2 - 260, wh/2 - 220, 520, 440), NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_TITLE)) {
            static int install_step = 0;
            static int target_drive = 0;
            static char install_user[32] = "admin";
            static char install_pass[32] = "password";

            if (install_step == 0) {
                nk_layout_row_dynamic(ctx, 28, 1);
                nk_label(ctx, "install sovereign rtc64", NK_TEXT_CENTERED);
                nk_layout_row_dynamic(ctx, 60, 1);
                nk_label_wrap(ctx, "hey, let's install the system! choose your personal administrator credentials below:");

                nk_layout_row_dynamic(ctx, 24, 1);
                nk_label(ctx, "admin username:", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 28, 1);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, install_user, sizeof(install_user) - 1, nk_filter_default);

                nk_layout_row_dynamic(ctx, 24, 1);
                nk_label(ctx, "admin password:", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 28, 1);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, install_pass, sizeof(install_pass) - 1, nk_filter_default);

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
                    vfs_mkdir("/etc");

                    char user_entry[128];
                    snprintf(user_entry, sizeof(user_entry), "%s:%s\n", install_user, install_pass);
                    vfs_write("/etc/passwd", user_entry);

                    vfs_mkdir("/bin");
                    vfs_mkdir("/home");

                    char user_dir[128];
                    snprintf(user_dir, sizeof(user_dir), "/home/%s", install_user);
                    vfs_mkdir(user_dir);

                    app->installed = 1;
                    strncpy(app->username, install_user, sizeof(app->username) - 1);
                    strncpy(app->password, install_pass, sizeof(app->password) - 1);
                    install_step = 3;
                }
            } else if (install_step == 3) {
                nk_layout_row_dynamic(ctx, 28, 1);
                nk_label(ctx, "done! system loaded successfully.", NK_TEXT_CENTERED);
                nk_layout_row_dynamic(ctx, 26, 1);
                nk_label(ctx, "all files written. click finish to boot.", NK_TEXT_CENTERED);
                nk_layout_row_dynamic(ctx, 34, 1);
                if (nk_button_label(ctx, "boot desktop")) {
                    ensure_user_desktop(app);
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
