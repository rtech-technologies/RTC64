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

    vfs_mkdir("/Windows");
    vfs_mkdir("/Windows/System32");
    vfs_mkdir("/Windows/System32/config");
    vfs_mkdir("/Users");
    snprintf(user_home, sizeof(user_home), "/Users/%s", app->username);
    vfs_mkdir(user_home);
    snprintf(desktop_path, sizeof(desktop_path), "%s/Desktop", user_home);
    vfs_mkdir(desktop_path);

    /* Copy typical apps to user's Desktop if they are not already there */
    char check_path[320];

    snprintf(check_path, sizeof(check_path), "%s/shell.bin", desktop_path);
    vfs_copy_file("/shell.bin", check_path);
    vfs_copy_file("/bin/shell.bin", check_path);
    vfs_copy_file("/Windows/System32/shell.bin", check_path);

    snprintf(check_path, sizeof(check_path), "%s/lab.bin", desktop_path);
    vfs_copy_file("/lab.bin", check_path);
    vfs_copy_file("/bin/lab.bin", check_path);
    vfs_copy_file("/Windows/System32/lab.bin", check_path);

    snprintf(check_path, sizeof(check_path), "%s/notepad.bin", desktop_path);
    vfs_copy_file("/notepad.bin", check_path);
    vfs_copy_file("/bin/notepad.bin", check_path);
    vfs_copy_file("/Windows/System32/notepad.bin", check_path);

    snprintf(check_path, sizeof(check_path), "%s/studio.bin", desktop_path);
    vfs_copy_file("/studio.bin", check_path);
    vfs_copy_file("/bin/studio.bin", check_path);
    vfs_copy_file("/Windows/System32/studio.bin", check_path);

    snprintf(check_path, sizeof(check_path), "%s/tests.bin", desktop_path);
    vfs_copy_file("/tests.bin", check_path);
    vfs_copy_file("/bin/tests.bin", check_path);
    vfs_copy_file("/Windows/System32/tests.bin", check_path);
}

static void scan_desktop_apps(struct app_state *app) {
    ensure_user_desktop(app);

    apps_on_desktop_count = 0;
    char desktop_path[256];
    snprintf(desktop_path, sizeof(desktop_path), "/Users/%s/Desktop", app->username);

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
            } else if (strcmp(name, "advanced settings") == 0) {
                if (strcmp(app->username, "Administrator") == 0 || strcmp(app->username, "admin") == 0 || app->uac_authenticated) {
                    app->show_advanced_settings = !app->show_advanced_settings;
                } else {
                    app->show_uac = 1;
                }
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

    int r_start = 30, g_start = 41, b_start = 59;
    int r_end = 15, g_end = 23, b_end = 42;

    static bool wallpaper_loaded = false;
    if (!wallpaper_loaded) {
        const char* saved = registry_get("desktop.wallpaper");
        if (saved) {
            strncpy(app->wallpaper_color, saved, sizeof(app->wallpaper_color) - 1);
        } else {
            strcpy(app->wallpaper_color, "Charcoal");
        }
        wallpaper_loaded = true;
    }

    if (strcmp(app->wallpaper_color, "Sovereign Blue") == 0) {
        r_start = 15; g_start = 32; b_start = 67;
        r_end = 5; g_end = 10; b_end = 26;
    } else if (strcmp(app->wallpaper_color, "Industrial Plum") == 0) {
        r_start = 48; g_start = 15; b_start = 59;
        r_end = 24; g_end = 5; b_end = 30;
    } else if (strcmp(app->wallpaper_color, "Pitch Black") == 0) {
        r_start = 10; g_start = 10; b_start = 10;
        r_end = 0; g_end = 0; b_end = 0;
    }

    for (int i = 0; i < steps; i++) {
        float ratio = (float)i / (float)steps;
        int r = (int)((float)r_start + (float)(r_end - r_start) * ratio);
        int g = (int)((float)g_start + (float)(g_end - g_start) * ratio);
        int b = (int)((float)b_start + (float)(b_end - b_start) * ratio);
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
    struct nk_rect clock_rect = nk_rect(cl_x - 300.0f, cl_y, 480.0f, 175.0f);

    /* Draw background highlight for clock when hovered */
    if (nk_input_is_mouse_hovering_rect(&ctx->input, clock_rect)) {
        nk_fill_rect(canvas, clock_rect, 12.0f, nk_rgba(255, 255, 255, 15));
        if (nk_input_is_mouse_pressed(&ctx->input, NK_BUTTON_LEFT)) {
            app->show_calendar = !app->show_calendar;
        }
    }
    nk_draw_text(canvas, nk_rect(cl_x - 300.0f, cl_y, 480.0f, 130.0f), time_str, (int)strlen(time_str), ctx->style.font, nk_rgba(255, 255, 255, 217), nk_rgba(0,0,0,0));
    nk_draw_text(canvas, nk_rect(cl_x - 300.0f, cl_y + 135.0f, 480.0f, 30.0f), "eastern standard (+3:00)", 24, ctx->style.font, nk_rgba(0, 190, 240, 204), nk_rgba(0,0,0,0));

    /* 5x5 Grid matrix layout of Apps exactly matching HTML spec */
    float grid_x = 40.0f;
    float grid_y = 50.0f;
    float box_sz = 65.0f;
    float gap = 12.0f;

    /* Interactive Top Arrow button to toggle sliding App Drawer (Android style) */
    struct nk_rect arrow_bounds = nk_rect(grid_x + 5 * box_sz + 4 * gap - 30, grid_y - 30, 30, 30);
    nk_fill_rect(canvas, arrow_bounds, 6, nk_rgba(30, 30, 30, 150));
    nk_stroke_rect(canvas, arrow_bounds, 6, 1.0f, nk_rgba(255, 255, 255, 50));
    nk_draw_text(canvas, nk_rect(arrow_bounds.x + 10, arrow_bounds.y + 4, 20, 20), "v", 1, ctx->style.font, nk_rgba(255, 255, 255, 200), nk_rgba(0,0,0,0));
    if (nk_input_is_mouse_hovering_rect(&ctx->input, arrow_bounds)) {
        nk_fill_rect(canvas, arrow_bounds, 6, nk_rgba(255, 255, 255, 40));
        if (nk_input_is_mouse_pressed(&ctx->input, NK_BUTTON_LEFT)) {
            app->show_drawer = !app->show_drawer;
        }
    }

    /* Row colors mapping based on row index */
    struct nk_color row_colors[5] = {
        nk_rgba(255, 59, 48, 255),  /* Red */
        nk_rgba(59, 48, 224, 255),  /* Blue */
        nk_rgba(255, 142, 40, 255), /* Orange */
        nk_rgba(0, 190, 240, 255),  /* Light Blue */
        nk_rgba(28, 198, 94, 255)   /* Green */
    };

    /* Populate the grid with select pinned desktop apps */
    int pinned_count = 5;
    const char* pinned_names[] = {"files", "notepad", "welcome", "settings", "advanced settings"};
    const char* pinned_paths[] = {"", "", "", "", ""};
    const char* pinned_display[] = {"files", "notepad", "welcome", "settings", "admin settings"};

    for (int i = 0; i < pinned_count; i++) {
        int row = i / 5;
        int col = i % 5;
        float x = grid_x + col * (box_sz + gap);
        float y = grid_y + row * (box_sz + gap);

        struct nk_color color = row_colors[row % 5];
        ui_render_desktop_square(ctx, app, pinned_names[i], pinned_paths[i], x, y, color);

        /* Draw short label text under desktop square */
        nk_draw_text(canvas, nk_rect(x, y + box_sz + 2, box_sz, 18), pinned_display[i], (int)strlen(pinned_display[i]), ctx->style.font, nk_rgba(255, 255, 255, 200), nk_rgba(0,0,0,0));
    }
}

static void ui_render_launcher(struct nk_context *ctx, struct app_state *app, int ww, int wh)
{
    if (!app->show_launcher) return;

    /* Place it nicely on the bottom-left just above the taskbar start button */
    float sm_w = 420.0f;
    float sm_h = 450.0f;
    float sm_x = ((float)ww - (float)ww * 0.95f) / 2.0f; /* align with taskbar left */
    if (sm_x < 20.0f) sm_x = 20.0f;
    float sm_y = (float)wh - 84.0f - sm_h - 10.0f; /* 10px spacing above taskbar */

    if (nk_begin(ctx, "Sovereign Start Menu", nk_rect(sm_x, sm_y, sm_w, sm_h), NK_WINDOW_NO_SCROLLBAR)) {
        /* Left column (Navigation) and Right column (Apps Grid) */
        nk_layout_row_begin(ctx, NK_STATIC, 400, 2);

        /* Column 1: Navigation / User info (Width: 150) */
        nk_layout_row_push(ctx, 150);
        if (nk_group_begin(ctx, "start_left", NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row_dynamic(ctx, 24, 1);
            char user_lbl[64];
            snprintf(user_lbl, sizeof(user_lbl), "hi, %s!", app->username);
            nk_label(ctx, user_lbl, NK_TEXT_LEFT);
            nk_label(ctx, "system links:", NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 32, 1);
            if (nk_button_label(ctx, "explorer")) {
                app->show_explorer = 1;
                app->show_launcher = 0;
            }
            if (nk_button_label(ctx, "text pad")) {
                app->show_notepad = 1;
                app->show_launcher = 0;
            }
            if (nk_button_label(ctx, "preferences")) {
                app->show_settings = 1;
                app->show_launcher = 0;
            }
            if (nk_button_label(ctx, "advanced settings")) {
                app->show_advanced_settings = 1;
                app->show_launcher = 0;
            }

            nk_layout_row_dynamic(ctx, 40, 1);
            nk_label(ctx, "", NK_TEXT_LEFT); /* Spacer */

            nk_layout_row_dynamic(ctx, 32, 1);
            if (nk_button_label(ctx, "log out")) {
                app->current_state = STATE_LOGIN;
                app->show_launcher = 0;
            }
            if (nk_button_label(ctx, "shutdown")) {
                scheduler_stop_all();
            }
            nk_group_end(ctx);
        }

        /* Column 2: Application matrix / Recent (Width: 230) */
        nk_layout_row_push(ctx, 230);
        if (nk_group_begin(ctx, "start_right", 0)) {
            nk_layout_row_dynamic(ctx, 24, 1);
            nk_label(ctx, "pinned applications:", NK_TEXT_LEFT);

            /* Vertical list of all compiled OS applications in startup menu */
            const char* menu_paths[] = {"/bin/shell.bin", "/bin/lab.bin", "/bin/notepad.bin", "/bin/studio.bin", "/bin/tests.bin"};
            const char* menu_labels[] = {"Terminal (Shell)", "System Laboratory", "Notepad (User)", "App Studio", "System Diagnostics"};

            for (int i = 0; i < 5; i++) {
                nk_layout_row_dynamic(ctx, 40, 1);
                if (nk_button_label(ctx, menu_labels[i])) {
                    extern int app_spawn_binary(const char* path);
                    app_spawn_binary(menu_paths[i]);
                    app->show_launcher = 0;
                }
            }
            nk_group_end(ctx);
        }
        nk_layout_row_end(ctx);
    }
    if (nk_window_is_closed(ctx, "Sovereign Start Menu")) app->show_launcher = 0;
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

static bool check_path_permission(struct app_state* app, const char* path) {
    if (strcmp(app->username, "Administrator") == 0 || strcmp(app->username, "admin") == 0) {
        return true;
    }
    /* Restrict regular user to their own home directory or non-system paths */
    if (strncmp(path, "/Users/", 7) == 0) {
        const char* sub = path + 7;
        char current_user_home[64];
        snprintf(current_user_home, sizeof(current_user_home), "%s", app->username);
        size_t ulen = strlen(current_user_home);
        if (strncmp(sub, current_user_home, ulen) != 0 || (sub[ulen] != '/' && sub[ulen] != '\0')) {
            return false; /* Access denied to other users' directories */
        }
    }
    if (strncmp(path, "/etc", 4) == 0 || strncmp(path, "/Windows/System32/config", 24) == 0 || strncmp(path, "/mnt/disk0/system", 17) == 0) {
        return false; /* Access denied to administrative folders */
    }
    return true;
}

static void ui_render_files(struct nk_context *ctx, struct app_state *app)
{
    if (nk_begin(ctx, "file explorer", nk_rect(550, 150, 480, 400), NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 26, 1);
        nk_label(ctx, "index: root/vfs/Users", NK_TEXT_LEFT);

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

        /* Create Item Panel */
        nk_layout_row_dynamic(ctx, 30, 3);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->explorer_new_item_name, sizeof(app->explorer_new_item_name) - 1, nk_filter_default);
        if (nk_button_label(ctx, "new folder")) {
            if (app->explorer_new_item_name[0]) {
                char full_path[320];
                snprintf(full_path, sizeof(full_path), "%s%s%s",
                         app->explorer_path,
                         (app->explorer_path[strlen(app->explorer_path)-1] == '/') ? "" : "/",
                         app->explorer_new_item_name);
                if (check_path_permission(app, full_path)) {
                    vfs_mkdir(full_path);
                }
                app->explorer_new_item_name[0] = '\0';
            }
        }
        if (nk_button_label(ctx, "new file")) {
            if (app->explorer_new_item_name[0]) {
                char full_path[320];
                snprintf(full_path, sizeof(full_path), "%s%s%s",
                         app->explorer_path,
                         (app->explorer_path[strlen(app->explorer_path)-1] == '/') ? "" : "/",
                         app->explorer_new_item_name);
                if (check_path_permission(app, full_path)) {
                    vfs_write(full_path, "new file text");
                }
                app->explorer_new_item_name[0] = '\0';
            }
        }

        /* Rename Row if renaming is active */
        if (app->explorer_rename_active) {
            nk_layout_row_dynamic(ctx, 30, 4);
            nk_label(ctx, "rename to:", NK_TEXT_LEFT);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->explorer_rename_name, sizeof(app->explorer_rename_name) - 1, nk_filter_default);
            if (nk_button_label(ctx, "apply")) {
                if (app->explorer_rename_name[0]) {
                    char parent_dir[256];
                    strncpy(parent_dir, app->explorer_rename_target, sizeof(parent_dir)-1);
                    parent_dir[sizeof(parent_dir)-1] = '\0';
                    char *last_slash = strrchr(parent_dir, '/');
                    if (last_slash) {
                        *last_slash = '\0';
                    }
                    char new_full_path[320];
                    snprintf(new_full_path, sizeof(new_full_path), "%s/%s", parent_dir, app->explorer_rename_name);
                    if (check_path_permission(app, app->explorer_rename_target) && check_path_permission(app, new_full_path)) {
                        vfs_rename(app->explorer_rename_target, new_full_path);
                    }
                    app->explorer_rename_active = 0;
                    app->explorer_rename_name[0] = '\0';
                }
            }
            if (nk_button_label(ctx, "cancel")) {
                app->explorer_rename_active = 0;
            }
        }

        nk_layout_row_dynamic(ctx, 200, 1);
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
                        nk_layout_row_template_push_static(ctx, 60);
                        nk_layout_row_template_push_static(ctx, 60);
                        nk_layout_row_template_end(ctx);

                        char clean_line[128];
                        strncpy(clean_line, line, sizeof(clean_line)-1);
                        clean_line[sizeof(clean_line)-1] = '\0';
                        for (int i = 0; clean_line[i]; i++) {
                            if (clean_line[i] >= 'A' && clean_line[i] <= 'Z') {
                                clean_line[i] = clean_line[i] - 'A' + 'a';
                            }
                        }

                        char item_full_path[320];
                        snprintf(item_full_path, sizeof(item_full_path), "%s%s%s",
                                 app->explorer_path,
                                 (app->explorer_path[strlen(app->explorer_path)-1] == '/') ? "" : "/",
                                 name);

                        if (nk_button_label(ctx, clean_line)) {
                            if (is_dir) {
                                if (check_path_permission(app, item_full_path)) {
                                    if (app->explorer_path[strlen(app->explorer_path)-1] != '/') {
                                        strncat(app->explorer_path, "/", sizeof(app->explorer_path) - strlen(app->explorer_path) - 1);
                                    }
                                    strncat(app->explorer_path, name, sizeof(app->explorer_path) - strlen(app->explorer_path) - 1);
                                } else {
                                    /* Permission denied notice can be printed or serial logged */
                                    serial_printf("[EXPLORER] Access Denied: User %s has no clearance for %s\n", app->username, item_full_path);
                                }
                            } else {
                                /* Open in Notepad */
                                if (check_path_permission(app, item_full_path)) {
                                    if (vfs_cat(item_full_path, app->notepad_buffer, sizeof(app->notepad_buffer)) == 0) {
                                        strncpy(app->notepad_file, item_full_path, sizeof(app->notepad_file)-1);
                                        app->show_notepad = 1;
                                    }
                                } else {
                                    serial_printf("[EXPLORER] Access Denied: User %s has no clearance for %s\n", app->username, item_full_path);
                                }
                            }
                        }
                        if (nk_button_label(ctx, "rename")) {
                            if (check_path_permission(app, item_full_path)) {
                                app->explorer_rename_active = 1;
                                strncpy(app->explorer_rename_target, item_full_path, sizeof(app->explorer_rename_target)-1);
                                strncpy(app->explorer_rename_name, name, sizeof(app->explorer_rename_name)-1);
                            }
                        }
                        if (nk_button_label(ctx, "destroy")) {
                            if (check_path_permission(app, item_full_path)) {
                                vfs_rm(item_full_path);
                            }
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
    if (nk_begin(ctx, "system preferences", nk_rect(650, 240, 420, 320), NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "desktop wallpaper customizer:", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 28, 2);
        if (nk_option_label(ctx, "charcoal (matte)", strcmp(app->wallpaper_color, "Charcoal") == 0)) {
            strcpy(app->wallpaper_color, "Charcoal");
        }
        if (nk_option_label(ctx, "sovereign blue", strcmp(app->wallpaper_color, "Sovereign Blue") == 0)) {
            strcpy(app->wallpaper_color, "Sovereign Blue");
        }
        if (nk_option_label(ctx, "industrial plum", strcmp(app->wallpaper_color, "Industrial Plum") == 0)) {
            strcpy(app->wallpaper_color, "Industrial Plum");
        }
        if (nk_option_label(ctx, "pitch black", strcmp(app->wallpaper_color, "Pitch Black") == 0)) {
            strcpy(app->wallpaper_color, "Pitch Black");
        }

        nk_layout_row_dynamic(ctx, 32, 1);
        if (nk_button_label(ctx, "save & apply wallpaper")) {
            registry_set("desktop.wallpaper", app->wallpaper_color);
            registry_flush();
            serial_printf("[SETTINGS] Wallpaper saved: %s\n", app->wallpaper_color);
        }

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "workspace notes / memory pad:", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 80, 1);
        int len = (int)strlen(app->notepad_buffer);
        nk_edit_string(ctx, NK_EDIT_MULTILINE, app->notepad_buffer, &len, sizeof(app->notepad_buffer)-1, nk_filter_default);
        app->notepad_buffer[len] = '\0';
    }
    if (nk_window_is_closed(ctx, "system preferences")) app->show_settings = 0;
    nk_end(ctx);
}

static void ui_render_task_manager(struct nk_context *ctx, struct app_state *app)
{
    if (!app->show_task_manager) return;
    if (nk_begin(ctx, "task manager", nk_rect(400, 200, 520, 400), NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_template_begin(ctx, 30);
        nk_layout_row_template_push_static(ctx, 40);
        nk_layout_row_template_push_dynamic(ctx);
        nk_layout_row_template_push_static(ctx, 50);
        nk_layout_row_template_push_static(ctx, 70);
        nk_layout_row_template_push_static(ctx, 80);
        nk_layout_row_template_end(ctx);

        nk_label(ctx, "id", NK_TEXT_LEFT);
        nk_label(ctx, "name", NK_TEXT_LEFT);
        nk_label(ctx, "upid", NK_TEXT_LEFT);
        nk_label(ctx, "state", NK_TEXT_LEFT);
        nk_label(ctx, "action", NK_TEXT_LEFT);

        int count = scheduler_get_task_count();
        for (int i = 0; i < count; i++) {
            task_t* t = scheduler_get_task(i);
            if (!t || t->state == TASK_DEAD) continue;

            nk_layout_row_template_begin(ctx, 24);
            nk_layout_row_template_push_static(ctx, 40);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_push_static(ctx, 50);
            nk_layout_row_template_push_static(ctx, 70);
            nk_layout_row_template_push_static(ctx, 80);
            nk_layout_row_template_end(ctx);

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

            /* Action button to End Task */
            if (t->id == 0 || strcmp(t->name, "Environment Manager") == 0 || strcmp(t->name, "Compliance") == 0) {
                nk_label(ctx, "locked", NK_TEXT_CENTERED);
            } else {
                if (nk_button_label(ctx, "end task")) {
                    scheduler_remove_task(t->id);
                }
            }
        }
    }
    if (nk_window_is_closed(ctx, "task manager")) app->show_task_manager = 0;
    nk_end(ctx);
}

static void notepad_find_replace(struct app_state* app) {
    if (!app->notepad_search_term[0]) return;
    char temp[4096];
    char *src = app->notepad_buffer;
    char *dst = temp;
    size_t search_len = strlen(app->notepad_search_term);
    size_t replace_len = strlen(app->notepad_replace_term);

    while (*src) {
        if (strncmp(src, app->notepad_search_term, search_len) == 0) {
            if ((size_t)(dst - temp) + replace_len < sizeof(temp) - 1) {
                strcpy(dst, app->notepad_replace_term);
                dst += replace_len;
                src += search_len;
            } else {
                break;
            }
        } else {
            if ((size_t)(dst - temp) < sizeof(temp) - 1) {
                *dst++ = *src++;
            } else {
                break;
            }
        }
    }
    *dst = '\0';
    strncpy(app->notepad_buffer, temp, sizeof(app->notepad_buffer)-1);
    app->notepad_buffer[sizeof(app->notepad_buffer)-1] = '\0';
}

static void ui_render_notepad(struct nk_context *ctx, struct app_state *app)
{
    if (!app->show_notepad) return;
    if (nk_begin(ctx, "text pad", nk_rect(300, 100, 600, 500), NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {
        /* Notepad Action Bar */
        nk_layout_row_dynamic(ctx, 28, 4);
        if (nk_button_label(ctx, "new")) {
            app->notepad_buffer[0] = '\0';
            app->notepad_file[0] = '\0';
        }
        if (nk_button_label(ctx, "open path")) {
            if (app->notepad_file[0]) {
                vfs_cat(app->notepad_file, app->notepad_buffer, sizeof(app->notepad_buffer));
            }
        }
        if (nk_button_label(ctx, "save")) {
            if (app->notepad_file[0]) {
                vfs_write(app->notepad_file, app->notepad_buffer);
            }
        }
        if (nk_button_label(ctx, "search & replace")) {
            app->notepad_show_search_replace = !app->notepad_show_search_replace;
        }

        /* Filepath Input Bar */
        nk_layout_row_dynamic(ctx, 28, 2);
        nk_label(ctx, "file path:", NK_TEXT_LEFT);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->notepad_file, sizeof(app->notepad_file) - 1, nk_filter_default);

        /* Search and Replace Sub-Panel */
        if (app->notepad_show_search_replace) {
            nk_layout_row_dynamic(ctx, 28, 5);
            nk_label(ctx, "find:", NK_TEXT_LEFT);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->notepad_search_term, sizeof(app->notepad_search_term) - 1, nk_filter_default);
            nk_label(ctx, "replace:", NK_TEXT_LEFT);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->notepad_replace_term, sizeof(app->notepad_replace_term) - 1, nk_filter_default);
            if (nk_button_label(ctx, "replace all")) {
                notepad_find_replace(app);
            }
        }

        nk_layout_row_dynamic(ctx, 320, 1);
        int len = (int)strlen(app->notepad_buffer);
        nk_edit_string(ctx, NK_EDIT_MULTILINE, app->notepad_buffer, &len, sizeof(app->notepad_buffer)-1, nk_filter_default);
        app->notepad_buffer[len] = '\0';
    }
    if (nk_window_is_closed(ctx, "text pad")) app->show_notepad = 0;
    nk_end(ctx);
}

static void ui_render_drawer(struct nk_context *ctx, struct app_state *app, int ww, int wh) {
    if (!app->show_drawer) return;

    struct nk_rect drawer_rect = nk_rect(0, 0, (float)ww, (float)wh * 0.7f);
    if (nk_begin(ctx, "app_drawer", drawer_rect, NK_WINDOW_NO_SCROLLBAR)) {
        struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
        nk_fill_rect(canvas, drawer_rect, 0.0f, nk_rgba(15, 23, 42, 240)); /* Very dark slate overlay */
        nk_stroke_rect(canvas, drawer_rect, 0.0f, 1.0f, nk_rgba(0, 190, 240, 100));

        nk_layout_row_dynamic(ctx, 30, 1);
        char title_buf[128];
        snprintf(title_buf, sizeof(title_buf), "app drawer - signed in as %s", app->username);
        nk_label(ctx, title_buf, NK_TEXT_CENTERED);
        nk_label(ctx, "all accessible applications on this system:", NK_TEXT_CENTERED);

        /* Render dynamic grid of applications */
        char list_buf[4096];
        int count = 0;

        /* List /bin binaries */
        if (vfs_ls("/bin", list_buf, sizeof(list_buf)) == 0) {
            char *line = list_buf;
            while (line && *line && count < 24) {
                char *next_line = strchr(line, '\n');
                if (next_line) *next_line = '\0';

                if (strlen(line) > 6) {
                    bool is_dir = (strncmp(line, "<DIR>", 5) == 0);
                    const char *name = line + 6;
                    if (!is_dir && strstr(name, ".bin")) {
                        /* Render item */
                        if (count % 6 == 0) {
                            nk_layout_row_dynamic(ctx, 80, 6);
                        }
                        char full_path[320];
                        snprintf(full_path, sizeof(full_path), "/bin/%s", name);

                        /* Strip .bin suffix for display */
                        char display_name[64];
                        strncpy(display_name, name, sizeof(display_name)-1);
                        char *suffix = strstr(display_name, ".bin");
                        if (suffix) *suffix = '\0';

                        if (nk_button_label(ctx, display_name)) {
                            extern int app_spawn_binary(const char* path);
                            app_spawn_binary(full_path);
                            app->show_drawer = 0; /* Auto close drawer */
                        }
                        count++;
                    }
                }
                if (next_line) line = next_line + 1;
                else line = NULL;
            }
        }

        nk_layout_row_dynamic(ctx, 40, 1);
        if (nk_button_label(ctx, "close drawer (^)")) {
            app->show_drawer = 0;
        }
    }
    nk_end(ctx);
}

static bool verify_admin_password(const char* password) {
    if (verify_user_credentials("Administrator", password)) return true;
    if (verify_user_credentials("admin", password)) return true;
    return false;
}

static void ui_render_uac(struct nk_context *ctx, struct app_state *app) {
    if (nk_begin(ctx, "User Account Control", nk_rect(350, 180, 420, 260), NK_WINDOW_MOVABLE|NK_WINDOW_TITLE|NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, "Do you want to allow this app to make changes?", NK_TEXT_CENTERED);
        nk_label(ctx, "App: Advanced Settings Manager", NK_TEXT_CENTERED);
        nk_label(ctx, "Enter Administrator Password:", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 32, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->uac_password_buffer, sizeof(app->uac_password_buffer) - 1, nk_filter_default);

        nk_layout_row_dynamic(ctx, 36, 2);
        if (nk_button_label(ctx, "Yes / Elevate")) {
            if (verify_admin_password(app->uac_password_buffer)) {
                app->uac_authenticated = 1;
                app->show_advanced_settings = 1;
                app->show_uac = 0;
                app->uac_password_buffer[0] = '\0';
            } else {
                serial_printf("[UAC] Elevation failed: Invalid Admin Password entered.\n");
            }
        }
        if (nk_button_label(ctx, "No / Cancel")) {
            app->show_uac = 0;
            app->uac_password_buffer[0] = '\0';
        }
    }
    nk_end(ctx);
}

static void ui_render_advanced_settings(struct nk_context *ctx, struct app_state *app) {
    if (nk_begin(ctx, "Advanced Settings Manager", nk_rect(300, 120, 500, 380), NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, "Sovereign Advanced Policy Configurator", NK_TEXT_LEFT);
        nk_label(ctx, "Direct Registry and System Capability Overrides", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 30, 2);
        app->perm_net = nk_check_label(ctx, "Enable System Networking (SYS_NET_FETCH)", app->perm_net);
        app->perm_storage = nk_check_label(ctx, "Enable Storage Direct Block Writing", app->perm_storage);

        nk_layout_row_dynamic(ctx, 36, 1);
        if (nk_button_label(ctx, "Apply Security Policy Changes")) {
            uac_set_permit(0, app->perm_net, app->perm_storage);
            serial_printf("[POLICY] Admin updated security capabilities: Net=%d, Storage=%d\n", app->perm_net, app->perm_storage);
        }

        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, "Quick-Format Storage Partition:", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 36, 2);
        if (nk_button_label(ctx, "Format secondary drive (FAT32)")) {
            BYTE work[FF_MAX_SS];
            MKFS_PARM opt = {FM_FAT32, 0, 0, 0, 0};
            FRESULT res = f_mkfs("1:", &opt, work, sizeof(work));
            if (res == FR_OK) {
                serial_printf("[SYS] Formatted drive 1 to FAT32 successfully.\n");
            } else {
                serial_printf("[SYS] Failed to format drive 1 (FRESULT=%d).\n", (int)res);
            }
        }
        if (nk_button_label(ctx, "Flush System Registry")) {
            registry_flush();
        }
    }
    if (nk_window_is_closed(ctx, "Advanced Settings Manager")) {
        app->show_advanced_settings = 0;
        app->uac_authenticated = 0;
    }
    nk_end(ctx);
}

static void ui_render_calendar(struct nk_context *ctx, struct app_state *app, int ww, int wh) {
    if (!app->show_calendar) return;
    (void)wh;

    float cl_x = (float)ww * 0.9f - 180.0f;
    float cl_y = 60.0f;

    /* Render dropdown window directly below the desktop clock */
    if (nk_begin(ctx, "Calendar & Time", nk_rect(cl_x - 100.0f, cl_y + 175.0f, 280.0f, 320.0f), NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_TITLE)) {
        int h = 0, m = 0, s = 0;
        rtc_get_time(&h, &m, &s);
        char full_time_str[64];
        snprintf(full_time_str, sizeof(full_time_str), "%02d:%02d:%02d UTC", h, m, s);

        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, full_time_str, NK_TEXT_CENTERED);
        nk_label(ctx, "March 2025", NK_TEXT_CENTERED);

        /* Weekday headers */
        nk_layout_row_dynamic(ctx, 20, 7);
        nk_label(ctx, "Mo", NK_TEXT_CENTERED);
        nk_label(ctx, "Tu", NK_TEXT_CENTERED);
        nk_label(ctx, "We", NK_TEXT_CENTERED);
        nk_label(ctx, "Th", NK_TEXT_CENTERED);
        nk_label(ctx, "Fr", NK_TEXT_CENTERED);
        nk_label(ctx, "Sa", NK_TEXT_CENTERED);
        nk_label(ctx, "Su", NK_TEXT_CENTERED);

        /* March 2025 starts on a Saturday.
         * Monday-Friday are empty slots in first week.
         * Saturday is 1st, Sunday is 2nd. */
        nk_layout_row_dynamic(ctx, 20, 7);
        for (int i = 0; i < 5; i++) {
            nk_label(ctx, " ", NK_TEXT_CENTERED);
        }

        /* Draw day 1 and 2 */
        nk_label(ctx, "1", NK_TEXT_CENTERED);
        nk_label(ctx, "2", NK_TEXT_CENTERED);

        /* Draw the rest of March (3 to 31) */
        int col = 0;
        for (int day = 3; day <= 31; day++) {
            if (col == 0) {
                nk_layout_row_dynamic(ctx, 20, 7);
            }
            char day_str[8];
            snprintf(day_str, sizeof(day_str), "%d", day);

            /* Highlight today/current day (e.g. 5th of March) as cyan */
            if (day == 5) {
                ctx->style.button.normal = nk_style_item_color(nk_rgba(0, 190, 240, 150));
                if (nk_button_label(ctx, day_str)) {}
                ui_init_style_internal(ctx);
            } else {
                nk_label(ctx, day_str, NK_TEXT_CENTERED);
            }

            col = (col + 1) % 7;
        }
    }
    if (nk_window_is_closed(ctx, "Calendar & Time")) app->show_calendar = 0;
    nk_end(ctx);
}

static void ui_render_desktop(struct nk_context *ctx, struct app_state *app, int ww, int wh)
{
    ui_render_background(ctx, app, ww, wh);
    ui_render_taskbar(ctx, app, ww, wh);
    ui_render_launcher(ctx, app, ww, wh);
    ui_render_drawer(ctx, app, ww, wh);
    ui_render_system_panel(ctx, app);
    ui_render_crash_reports(ctx, app);
    ui_render_task_manager(ctx, app);

    if (app->show_explorer) ui_render_files(ctx, app);
    if (app->show_settings) ui_render_settings(ctx, app);
    if (app->show_notepad) ui_render_notepad(ctx, app);
    if (app->show_uac) ui_render_uac(ctx, app);
    if (app->show_advanced_settings) ui_render_advanced_settings(ctx, app);
    ui_render_calendar(ctx, app, ww, wh);
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

                        /* Pre-initialize Desktop workspace for the newly created account! */
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
                    /* Create essential system files */
                    vfs_mkdir("/etc");

                    char user_entry[128];
                    snprintf(user_entry, sizeof(user_entry), "%s:%s\n", install_user, install_pass);
                    vfs_write("/etc/passwd", user_entry);

                    vfs_mkdir("/bin");
                    vfs_mkdir("/home");
                    vfs_mkdir("/Windows");
                    vfs_mkdir("/Windows/System32");
                    vfs_mkdir("/Windows/System32/config");
                    vfs_mkdir("/Users");

                    char user_dir[128];
                    snprintf(user_dir, sizeof(user_dir), "/Users/%s", install_user);
                    vfs_mkdir(user_dir);
                    char fallback_dir[128];
                    snprintf(fallback_dir, sizeof(fallback_dir), "/home/%s", install_user);
                    vfs_mkdir(fallback_dir);

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
                    /* Initialise newly installed administrator workspace */
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
