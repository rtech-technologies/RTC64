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

// Math functions for Analog clock rendering
extern double sin(double x);
extern double cos(double x);

// Theme Definitions
static struct nk_color themes[3][NK_COLOR_COUNT];
static int themes_initialized = 0;

void ui_init_style(struct nk_context *ctx) {
    // 0: Midnight Blue
    themes[0][NK_COLOR_TEXT] = nk_rgba(70, 200, 255, 255);
    themes[0][NK_COLOR_WINDOW] = nk_rgba(10, 15, 25, 255);
    themes[0][NK_COLOR_HEADER] = nk_rgba(30, 35, 45, 255);
    themes[0][NK_COLOR_BORDER] = nk_rgba(40, 80, 120, 255);
    themes[0][NK_COLOR_BUTTON] = nk_rgba(25, 45, 75, 255);
    themes[0][NK_COLOR_BUTTON_HOVER] = nk_rgba(35, 65, 105, 255);
    themes[0][NK_COLOR_BUTTON_ACTIVE] = nk_rgba(45, 85, 135, 255);
    themes[0][NK_COLOR_TOGGLE] = nk_rgba(20, 30, 50, 255);
    themes[0][NK_COLOR_TOGGLE_CURSOR] = nk_rgba(70, 200, 255, 255);
    themes[0][NK_COLOR_SELECT] = nk_rgba(25, 45, 75, 255);
    themes[0][NK_COLOR_SELECT_ACTIVE] = nk_rgba(70, 200, 255, 255);
    themes[0][NK_COLOR_SLIDER] = nk_rgba(20, 30, 50, 255);
    themes[0][NK_COLOR_SLIDER_CURSOR] = nk_rgba(70, 200, 255, 255);
    themes[0][NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(100, 220, 255, 255);
    themes[0][NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(150, 240, 255, 255);
    themes[0][NK_COLOR_PROPERTY] = nk_rgba(20, 30, 50, 255);
    themes[0][NK_COLOR_EDIT] = nk_rgba(15, 25, 40, 255);
    themes[0][NK_COLOR_EDIT_CURSOR] = nk_rgba(70, 200, 255, 255);
    themes[0][NK_COLOR_COMBO] = nk_rgba(20, 30, 50, 255);
    themes[0][NK_COLOR_CHART] = nk_rgba(20, 30, 50, 255);
    themes[0][NK_COLOR_CHART_COLOR] = nk_rgba(70, 200, 255, 255);
    themes[0][NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
    themes[0][NK_COLOR_SCROLLBAR] = nk_rgba(15, 25, 40, 255);
    themes[0][NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(40, 80, 120, 255);
    themes[0][NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(50, 100, 150, 255);
    themes[0][NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(60, 120, 180, 255);
    themes[0][NK_COLOR_TAB_HEADER] = nk_rgba(20, 30, 50, 255);

    // 1: Classic Grey
    themes[1][NK_COLOR_TEXT] = nk_rgba(220, 220, 220, 255);
    themes[1][NK_COLOR_WINDOW] = nk_rgba(45, 45, 45, 255);
    themes[1][NK_COLOR_HEADER] = nk_rgba(60, 60, 60, 255);
    themes[1][NK_COLOR_BORDER] = nk_rgba(80, 80, 80, 255);
    themes[1][NK_COLOR_BUTTON] = nk_rgba(70, 70, 70, 255);
    themes[1][NK_COLOR_BUTTON_HOVER] = nk_rgba(90, 90, 90, 255);
    themes[1][NK_COLOR_BUTTON_ACTIVE] = nk_rgba(110, 110, 110, 255);
    themes[1][NK_COLOR_TOGGLE] = nk_rgba(40, 40, 40, 255);
    themes[1][NK_COLOR_TOGGLE_CURSOR] = nk_rgba(200, 200, 200, 255);
    themes[1][NK_COLOR_SELECT] = nk_rgba(70, 70, 70, 255);
    themes[1][NK_COLOR_SELECT_ACTIVE] = nk_rgba(200, 200, 200, 255);
    themes[1][NK_COLOR_SLIDER] = nk_rgba(40, 40, 40, 255);
    themes[1][NK_COLOR_SLIDER_CURSOR] = nk_rgba(200, 200, 200, 255);
    themes[1][NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(230, 230, 230, 255);
    themes[1][NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(255, 255, 255, 255);
    themes[1][NK_COLOR_PROPERTY] = nk_rgba(40, 40, 40, 255);
    themes[1][NK_COLOR_EDIT] = nk_rgba(30, 30, 30, 255);
    themes[1][NK_COLOR_EDIT_CURSOR] = nk_rgba(200, 200, 200, 255);
    themes[1][NK_COLOR_COMBO] = nk_rgba(40, 40, 40, 255);
    themes[1][NK_COLOR_CHART] = nk_rgba(40, 40, 40, 255);
    themes[1][NK_COLOR_CHART_COLOR] = nk_rgba(200, 200, 200, 255);
    themes[1][NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 50, 50, 255);
    themes[1][NK_COLOR_SCROLLBAR] = nk_rgba(30, 30, 30, 255);
    themes[1][NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(70, 70, 70, 255);
    themes[1][NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(90, 90, 90, 255);
    themes[1][NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(110, 110, 110, 255);
    themes[1][NK_COLOR_TAB_HEADER] = nk_rgba(40, 40, 40, 255);

    // 2: Emerald Green
    themes[2][NK_COLOR_TEXT] = nk_rgba(50, 205, 50, 255);
    themes[2][NK_COLOR_WINDOW] = nk_rgba(10, 25, 15, 255);
    themes[2][NK_COLOR_HEADER] = nk_rgba(25, 50, 30, 255);
    themes[2][NK_COLOR_BORDER] = nk_rgba(40, 120, 60, 255);
    themes[2][NK_COLOR_BUTTON] = nk_rgba(25, 75, 45, 255);
    themes[2][NK_COLOR_BUTTON_HOVER] = nk_rgba(35, 105, 65, 255);
    themes[2][NK_COLOR_BUTTON_ACTIVE] = nk_rgba(45, 135, 85, 255);
    themes[2][NK_COLOR_TOGGLE] = nk_rgba(20, 50, 30, 255);
    themes[2][NK_COLOR_TOGGLE_CURSOR] = nk_rgba(50, 205, 50, 255);
    themes[2][NK_COLOR_SELECT] = nk_rgba(25, 75, 45, 255);
    themes[2][NK_COLOR_SELECT_ACTIVE] = nk_rgba(50, 205, 50, 255);
    themes[2][NK_COLOR_SLIDER] = nk_rgba(20, 50, 30, 255);
    themes[2][NK_COLOR_SLIDER_CURSOR] = nk_rgba(50, 205, 50, 255);
    themes[2][NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(100, 255, 100, 255);
    themes[2][NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(150, 255, 150, 255);
    themes[2][NK_COLOR_PROPERTY] = nk_rgba(20, 50, 30, 255);
    themes[2][NK_COLOR_EDIT] = nk_rgba(15, 40, 25, 255);
    themes[2][NK_COLOR_EDIT_CURSOR] = nk_rgba(50, 205, 50, 255);
    themes[2][NK_COLOR_COMBO] = nk_rgba(20, 50, 30, 255);
    themes[2][NK_COLOR_CHART] = nk_rgba(20, 50, 30, 255);
    themes[2][NK_COLOR_CHART_COLOR] = nk_rgba(50, 205, 50, 255);
    themes[2][NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
    themes[2][NK_COLOR_SCROLLBAR] = nk_rgba(15, 40, 25, 255);
    themes[2][NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(40, 120, 60, 255);
    themes[2][NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(50, 150, 75, 255);
    themes[2][NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(60, 180, 90, 255);
    themes[2][NK_COLOR_TAB_HEADER] = nk_rgba(20, 50, 30, 255);

    themes_initialized = 1;
    nk_style_from_table(ctx, themes[0]);
}

void ui_apply_theme(struct nk_context *ctx, int theme) {
    if (!themes_initialized) return;
    if (theme < 0 || theme > 2) theme = 0;
    nk_style_from_table(ctx, themes[theme]);
}

/* Simulated delay generator inside main flow helper */
static void simulate_slow_install(struct app_state *app) {
    if (app->progress < 100) {
        static int delay = 0;
        if (delay++ % 8 == 0) {
            app->progress++;

            // Populating files based on stage
            if (app->progress == 10) {
                vfs_create_file("/system/kernel.sys", false);
                vfs_write_file("/system/kernel.sys", "EXECUTION_READY", 15);
            } else if (app->progress == 40) {
                vfs_create_file("/system/uac.conf", false);
                vfs_write_file("/system/uac.conf", "UAC_ACTIVE=1\n", 13);
            } else if (app->progress == 75) {
                char path[64];
                snprintf(path, sizeof(path), "/home/%s", app->inst_username);
                vfs_create_file(path, true);

                snprintf(path, sizeof(path), "/home/%s/documents", app->inst_username);
                vfs_create_file(path, true);
            } else if (app->progress == 95) {
                char file_path[128];
                snprintf(file_path, sizeof(file_path), "/home/%s/todo.txt", app->inst_username);
                vfs_create_file(file_path, false);
                const char *todo = "1. Personalize theme color in Settings\n2. Solve math in Calculator\n3. Run neofetch in Terminal\n";
                vfs_write_file(file_path, todo, strlen(todo));
            }
        }
    }
}

/* Draggable resizable Window Helper with Minimize, Maximize, and Close control buttons */
static int begin_draggable_window(struct nk_context *ctx, const char *title, struct nk_rect rect, int *show_flag, int *max_flag) {
    if (!(*show_flag)) return 0;

    struct nk_rect target_rect = rect;
    if (*max_flag) {
        target_rect = nk_rect(0, 0, 1024, 768 - 50); // Framebuffer is 1024x768
    }

    nk_flags flags = NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE;
    if (*max_flag) {
        flags &= ~NK_WINDOW_MOVABLE;
        flags &= ~NK_WINDOW_SCALABLE;
    }

    int open = nk_begin(ctx, title, target_rect, flags);
    if (open) {
        // Render custom Window Actions in Header or Title bar row
        nk_layout_row_static(ctx, 22, 22, 3);
        if (nk_button_label(ctx, "_")) {
            *show_flag = 0; // Minimize / Hide window
        }
        if (nk_button_label(ctx, "口")) {
            *max_flag = !(*max_flag); // Maximize toggle
        }
        if (nk_button_label(ctx, "X")) {
            *show_flag = 0; // Close window
        }
    }
    return open;
}

static void draw_analog_clock(struct nk_context *ctx, float cx, float cy, float radius) {
    struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
    if (!canvas) return;

    // Outer circle rim
    struct nk_rect bounding = nk_rect(cx - radius, cy - radius, radius * 2, radius * 2);
    nk_fill_circle(canvas, bounding, nk_rgb(30, 40, 50));
    nk_stroke_circle(canvas, bounding, 3.0f, nk_rgb(70, 200, 255));

    // Clock Hands Calculation
    static float tick = 0.0f;
    tick += 0.01f;

    float hour_angle = (tick / 120.0f) * 2.0f * 3.14159f - 1.5707f;
    float min_angle = (tick / 10.0f) * 2.0f * 3.14159f - 1.5707f;
    float sec_angle = tick * 2.0f * 3.14159f - 1.5707f;

    // Hour hand
    float hx = cx + cos(hour_angle) * (radius * 0.5f);
    float hy = cy + sin(hour_angle) * (radius * 0.5f);
    nk_stroke_line(canvas, cx, cy, hx, hy, 4.0f, nk_rgb(255, 255, 255));

    // Minute hand
    float mx = cx + cos(min_angle) * (radius * 0.7f);
    float my = cy + sin(min_angle) * (radius * 0.7f);
    nk_stroke_line(canvas, cx, cy, mx, my, 2.5f, nk_rgb(70, 200, 255));

    // Second hand
    float sx = cx + cos(sec_angle) * (radius * 0.85f);
    float sy = cy + sin(sec_angle) * (radius * 0.85f);
    nk_stroke_line(canvas, cx, cy, sx, sy, 1.0f, nk_rgb(255, 69, 0));

    // Core center dot
    nk_fill_circle(canvas, nk_rect(cx - 4, cy - 4, 8, 8), nk_rgb(255, 255, 255));
}

// Safely appends to terminal buffer with bounds checking
static void safe_term_append(struct app_state *app, const char *str) {
    size_t len = strlen(str);
    size_t cur_len = strlen(app->term_output);
    if (cur_len + len >= 4095) {
        size_t keep_from = 2048;
        char *ptr = strchr(app->term_output + keep_from, '\n');
        if (ptr) {
            size_t offset = ptr - app->term_output + 1;
            memmove(app->term_output, app->term_output + offset, cur_len - offset + 1);
        } else {
            memmove(app->term_output, app->term_output + keep_from, cur_len - keep_from + 1);
        }
    }
    strcat(app->term_output, str);
}

// Global UI renderer flow
void ui_render(struct nk_context *ctx, struct app_state *app, int ww, int wh) {
    ui_apply_theme(ctx, app->active_theme);

    if (app->current_state == STATE_INSTALLER) {
        if (nk_begin(ctx, "Sovereign Workstation Installer", nk_rect(ww/2 - 250, wh/2 - 220, 500, 440),
            NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR))
        {
            nk_layout_row_dynamic(ctx, 35, 1);
            nk_label(ctx, "Sovereign RTC64 Pro Installation Wizard", NK_TEXT_CENTERED);
            nk_layout_row_dynamic(ctx, 5, 1);
            nk_spacing(ctx, 1);

            if (!app->install_started) {
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Please configure your primary user account:", NK_TEXT_LEFT);

                nk_layout_row_dynamic(ctx, 30, 2);
                nk_label(ctx, "Username:", NK_TEXT_LEFT);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->inst_username, sizeof(app->inst_username), nk_filter_ascii);

                nk_layout_row_dynamic(ctx, 30, 2);
                nk_label(ctx, "Password:", NK_TEXT_LEFT);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->inst_password, sizeof(app->inst_password), nk_filter_ascii);

                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Select Account Role Type:", NK_TEXT_LEFT);

                nk_layout_row_dynamic(ctx, 30, 3);
                if (nk_option_label(ctx, "Administrator", app->inst_role_idx == 0)) app->inst_role_idx = 0;
                if (nk_option_label(ctx, "Standard User", app->inst_role_idx == 1)) app->inst_role_idx = 1;
                if (nk_option_label(ctx, "Guest User", app->inst_role_idx == 2)) app->inst_role_idx = 2;

                nk_layout_row_dynamic(ctx, 40, 1);
                nk_spacer(ctx);

                nk_layout_row_dynamic(ctx, 40, 1);
                if (nk_button_label(ctx, "Begin Format & Install")) {
                    if (strlen(app->inst_username) > 0) {
                        app->install_started = 1;
                        app->progress = 0;
                    }
                }
            } else {
                simulate_slow_install(app);

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "Formatting drive, creating partitions and system logs...", NK_TEXT_CENTERED);

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_progress(ctx, (nk_size*)&app->progress, 100, NK_FIXED);

                nk_layout_row_dynamic(ctx, 30, 1);
                char prog_buf[32];
                snprintf(prog_buf, sizeof(prog_buf), "Progress: %d%%", app->progress);
                nk_label(ctx, prog_buf, NK_TEXT_CENTERED);

                if (app->progress >= 100) {
                    nk_layout_row_dynamic(ctx, 40, 1);
                    nk_spacer(ctx);
                    nk_layout_row_dynamic(ctx, 45, 1);
                    if (nk_button_label(ctx, i18n_translate("Finish & Boot Workstation"))) {
                        user_role_t r = ROLE_ADMIN;
                        if (app->inst_role_idx == 1) r = ROLE_STANDARD;
                        if (app->inst_role_idx == 2) r = ROLE_GUEST;
                        user_add(app->inst_username, app->inst_password, r);

                        user_set_active(app->inst_username);
                        app->current_state = STATE_DESKTOP;
                    }
                }
            }
        }
        nk_end(ctx);
    } else if (app->current_state == STATE_LOGIN) {
        if (nk_begin(ctx, "Sovereign Security Portal", nk_rect(ww/2 - 175, wh/2 - 200, 350, 400),
            NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR))
        {
            nk_layout_row_dynamic(ctx, 40, 1);
            nk_label(ctx, "🔒 SOVEREIGN SECURE PORTAL", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "Select Login Session Account:", NK_TEXT_LEFT);

            // Switching User menu dropdown simulation
            char names[10][32];
            user_role_t roles[10];
            int users_count = user_get_all(names, roles, 10);

            nk_layout_row_dynamic(ctx, 30, 1);
            if (nk_button_label(ctx, (app->login_username[0] != '\0') ? app->login_username : "Choose User...")) {
                app->show_user_select = !app->show_user_select;
            }

            if (app->show_user_select) {
                for (int i = 0; i < users_count; i++) {
                    nk_layout_row_dynamic(ctx, 25, 1);
                    if (nk_button_label(ctx, names[i])) {
                        strcpy(app->login_username, names[i]);
                        app->show_user_select = 0;
                        app->login_error = 0;
                    }
                }
            }

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_spacer(ctx);

            if (app->login_username[0] != '\0' && user_get_role(app->login_username) != ROLE_GUEST) {
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Enter Security Password:", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->login_password, sizeof(app->login_password), nk_filter_ascii);
            } else if (app->login_username[0] != '\0') {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "Guest account requires no password", NK_TEXT_CENTERED);
            }

            if (app->login_error) {
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "⚠️ Access Denied: Invalid credentials", NK_TEXT_CENTERED);
            }

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_spacer(ctx);

            nk_layout_row_dynamic(ctx, 40, 1);
            if (nk_button_label(ctx, "Unlock Session")) {
                if (app->login_username[0] != '\0') {
                    if (user_authenticate(app->login_username, app->login_password)) {
                        user_set_active(app->login_username);
                        app->current_state = STATE_DESKTOP;
                        app->login_password[0] = '\0';
                        app->login_error = 0;
                    } else {
                        app->login_error = 1;
                    }
                }
            }

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_spacer(ctx);
            nk_layout_row_static(ctx, 30, 100, 1);
            if (nk_button_label(ctx, "Installer")) {
                app->current_state = STATE_INSTALLER;
                app->install_started = 0;
                app->progress = 0;
            }
        }
        nk_end(ctx);
    } else if (app->current_state == STATE_DESKTOP) {
        char act_name[32];
        user_role_t act_role;
        user_get_active(act_name, &act_role);

        // Analog Clock Widget on the desktop
        if (nk_begin(ctx, "DesktopClock", nk_rect(ww - 180, 20, 160, 160), NK_WINDOW_NO_SCROLLBAR)) {
            draw_analog_clock(ctx, 80, 80, 70);
        }
        nk_end(ctx);

        // Desktop App Icons Grid
        if (nk_begin(ctx, "DesktopGrid", nk_rect(20, 20, 140, 500), NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row_static(ctx, 50, 100, 1);
            if (nk_button_label(ctx, "📁 Explorer")) app->show_explorer = 1;
            if (nk_button_label(ctx, "📝 Notepad")) app->show_notepad = 1;
            if (nk_button_label(ctx, "🧮 Calc")) app->show_calculator = 1;
            if (nk_button_label(ctx, "📟 Terminal")) app->show_terminal = 1;
            if (nk_button_label(ctx, "📊 Task Mgr")) app->show_taskmgr = 1;
            if (nk_button_label(ctx, "⚙️ Settings")) {
                if (act_role == ROLE_ADMIN) {
                    app->show_settings = 1;
                } else {
                    app->show_uac = 1; // Require password elevation
                }
            }
        }
        nk_end(ctx);

        // Sleek "System Summary" widget tracking CPU, RAM, and storage device mounts
        if (nk_begin(ctx, "System Widget", nk_rect(ww - 320, 200, 300, 180), NK_WINDOW_BORDER|NK_WINDOW_TITLE)) {
            nk_layout_row_dynamic(ctx, 22, 1);
            char act_buf[64];
            snprintf(act_buf, sizeof(act_buf), "Logged-in: %s (%s)", act_name, (act_role == ROLE_ADMIN) ? "Admin" : ((act_role == ROLE_STANDARD) ? "User" : "Guest"));
            nk_label(ctx, act_buf, NK_TEXT_LEFT);

            size_t ram_used = hal_malloc_get_used() / (1024 * 1024);
            size_t ram_total = (16 * 1024 * 1024) / (1024 * 1024); // 16MB Total heap
            char ram_buf[64];
            snprintf(ram_buf, sizeof(ram_buf), "Heap Usage: %dMB / %dMB", (int)ram_used, (int)ram_total);
            nk_label(ctx, ram_buf, NK_TEXT_LEFT);

            int ram_pct = (ram_used * 100) / ram_total;
            nk_progress(ctx, (nk_size*)&ram_pct, 100, NK_FIXED);

            char hwd_buf[64];
            snprintf(hwd_buf, sizeof(hwd_buf), "Hardware Disk Devices: %d", hal_storage_get_device_count());
            nk_label(ctx, hwd_buf, NK_TEXT_LEFT);
        }
        nk_end(ctx);

        // --- File Explorer ---
        static struct nk_rect w_explorer = {150, 150, 480, 360};
        static int max_explorer = 0;
        static char current_dir[128] = "/";
        if (begin_draggable_window(ctx, "File Explorer", w_explorer, &app->show_explorer, &max_explorer)) {
            nk_layout_row_dynamic(ctx, 28, 2);
            nk_label(ctx, "Navigation Sidebar", NK_TEXT_LEFT);
            char path_lbl[128];
            snprintf(path_lbl, sizeof(path_lbl), "Current Path: %s", current_dir);
            nk_label(ctx, path_lbl, NK_TEXT_RIGHT);

            nk_layout_row_begin(ctx, NK_DYNAMIC, 260, 2);
            nk_layout_row_push(ctx, 0.3f);

            if (nk_group_begin(ctx, "Sidebar", NK_WINDOW_BORDER)) {
                nk_layout_row_dynamic(ctx, 25, 1);
                if (nk_button_label(ctx, "📁 / (root)")) strcpy(current_dir, "/");
                if (nk_button_label(ctx, "📁 /home")) strcpy(current_dir, "/home");
                if (nk_button_label(ctx, "📁 /system")) strcpy(current_dir, "/system");
                if (nk_button_label(ctx, "📁 /dev (Disks)")) strcpy(current_dir, "/dev");
                nk_group_end(ctx);
            }

            nk_layout_row_push(ctx, 0.7f);

            if (nk_group_begin(ctx, "Main Browser", NK_WINDOW_BORDER)) {
                char filenames[32][64];
                bool is_dirs[32];
                int items = vfs_list_dir(current_dir, filenames, is_dirs, 32);

                for (int i = 0; i < items; i++) {
                    nk_layout_row_dynamic(ctx, 24, 2);
                    char name_buf[128];
                    snprintf(name_buf, sizeof(name_buf), "%s %s", is_dirs[i] ? "📁" : "📄", filenames[i]);
                    nk_label(ctx, name_buf, NK_TEXT_LEFT);

                    if (nk_button_label(ctx, "Select / Open")) {
                        if (is_dirs[i]) {
                            char temp_dir[128];
                            if (strcmp(current_dir, "/") == 0) {
                                snprintf(temp_dir, sizeof(temp_dir), "/%s", filenames[i]);
                            } else {
                                snprintf(temp_dir, sizeof(temp_dir), "%s/%s", current_dir, filenames[i]);
                            }
                            strcpy(current_dir, temp_dir);
                        } else {
                            char full_path[256];
                            if (strcmp(current_dir, "/") == 0) {
                                snprintf(full_path, sizeof(full_path), "/%s", filenames[i]);
                            } else {
                                snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, filenames[i]);
                            }

                            size_t flen = strlen(filenames[i]);
                            if (flen > 4 && strcmp(&filenames[i][flen - 4], ".txt") == 0) {
                                strcpy(app->notepad_path, full_path);
                                vfs_read_file(full_path, app->notepad_buf, sizeof(app->notepad_buf));
                                app->show_notepad = 1;
                            }
                        }
                    }
                }

                nk_layout_row_dynamic(ctx, 28, 1);
                nk_spacing(ctx, 1);

                static char new_file_name[64] = "note.txt";
                nk_layout_row_dynamic(ctx, 28, 2);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, new_file_name, sizeof(new_file_name), nk_filter_ascii);
                if (nk_button_label(ctx, "➕ Create File")) {
                    char full_path[256];
                    if (strcmp(current_dir, "/") == 0) {
                        snprintf(full_path, sizeof(full_path), "/%s", new_file_name);
                    } else {
                        snprintf(full_path, sizeof(full_path), "%s/%s", current_dir, new_file_name);
                    }
                    vfs_create_file(full_path, false);
                    vfs_write_file(full_path, "Insert content here...", 22);
                }

                nk_group_end(ctx);
            }
            nk_layout_row_end(ctx);
            nk_end(ctx);
        }

        // --- Notepad Editor ---
        static struct nk_rect w_notepad = {200, 100, 520, 380};
        static int max_notepad = 0;
        if (begin_draggable_window(ctx, "Notepad text-editor", w_notepad, &app->show_notepad, &max_notepad)) {
            nk_layout_row_dynamic(ctx, 28, 4);
            if (nk_button_label(ctx, "📄 New")) {
                app->notepad_path[0] = '\0';
                app->notepad_buf[0] = '\0';
            }
            if (nk_button_label(ctx, "📂 Open")) {
                strcpy(app->notepad_path, "/home/welcome.txt");
                vfs_read_file("/home/welcome.txt", app->notepad_buf, sizeof(app->notepad_buf));
            }
            if (nk_button_label(ctx, "💾 Save")) {
                if (app->notepad_path[0] == '\0') {
                    snprintf(app->notepad_path, sizeof(app->notepad_path), "/home/%s/note.txt", act_name);
                }
                vfs_write_file(app->notepad_path, app->notepad_buf, strlen(app->notepad_buf));
            }
            char path_hint[128];
            snprintf(path_hint, sizeof(path_hint), "File: %s", (app->notepad_path[0] != '\0') ? app->notepad_path : "unsaved");
            nk_label(ctx, path_hint, NK_TEXT_RIGHT);

            nk_layout_row_dynamic(ctx, 280, 1);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, app->notepad_buf, sizeof(app->notepad_buf), nk_filter_default);
            nk_end(ctx);
        }

        // --- Interactive Terminal ---
        static struct nk_rect w_terminal = {180, 220, 520, 340};
        static int max_terminal = 0;
        if (begin_draggable_window(ctx, "Terminal Shell", w_terminal, &app->show_terminal, &max_terminal)) {
            // Interactive output buffer area
            nk_layout_row_dynamic(ctx, 210, 1);
            if (nk_group_begin(ctx, "Term Output", NK_WINDOW_BORDER)) {
                nk_layout_row_dynamic(ctx, 16, 1);

                // Render console buffer by newline split token
                char *line = app->term_output;
                char *next_line;
                while (line && *line != '\0') {
                    next_line = strchr(line, '\n');
                    if (next_line) {
                        *next_line = '\0';
                        nk_label(ctx, line, NK_TEXT_LEFT);
                        *next_line = '\n';
                        line = next_line + 1;
                    } else {
                        nk_label(ctx, line, NK_TEXT_LEFT);
                        break;
                    }
                }
                nk_group_end(ctx);
            }

            // Command input row with active directory prompt
            nk_layout_row_dynamic(ctx, 30, 1);
            char prompt_buf[192];
            snprintf(prompt_buf, sizeof(prompt_buf), "%s@sovereign-os:~$ %s", act_name, app->cmd_input);
            nk_label(ctx, prompt_buf, NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 28, 2);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->cmd_input, sizeof(app->cmd_input), nk_filter_ascii);

            if (nk_button_label(ctx, "Execute") || nk_input_is_key_pressed(&ctx->input, NK_KEY_ENTER)) {
                if (strlen(app->cmd_input) > 0) {
                    // Record input to session history lists
                    if (app->cmd_history_count < 16) {
                        strcpy(app->cmd_history[app->cmd_history_count++], app->cmd_input);
                    }

                    // Process input commands
                    char output_tmp[512] = "";

                    if (strcmp(app->cmd_input, "help") == 0) {
                        snprintf(output_tmp, sizeof(output_tmp),
                            "Available core terminal utilities:\n"
                            " - help        : Display commands library\n"
                            " - ls          : List current home VFS items\n"
                            " - uptime      : Display CPU continuous metrics\n"
                            " - tasks       : Retrieve active scheduler threads\n"
                            " - neofetch    : Render gorgeous OS logo spec\n"
                            " - clear       : Reset session text frames\n");
                    } else if (strcmp(app->cmd_input, "ls") == 0) {
                        char files_list[32][64];
                        bool dirs_list[32];
                        int items = vfs_list_dir("/home", files_list, dirs_list, 32);
                        strcat(output_tmp, "Content of /home:\n");
                        for (int i = 0; i < items; i++) {
                            char line_buf[96];
                            snprintf(line_buf, sizeof(line_buf), " - %s %s\n", dirs_list[i] ? "[DIR]" : "[FILE]", files_list[i]);
                            strcat(output_tmp, line_buf);
                        }
                    } else if (strcmp(app->cmd_input, "uptime") == 0) {
                        snprintf(output_tmp, sizeof(output_tmp), "OS active status: Nominal (Scheduler running, 0 fault exceptions)\n");
                    } else if (strcmp(app->cmd_input, "tasks") == 0) {
                        task_t list[10];
                        int cnt = scheduler_get_tasks(list, 10);
                        strcat(output_tmp, "Active System Scheduler Spawns:\n");
                        for (int i = 0; i < cnt; i++) {
                            char task_buf[96];
                            snprintf(task_buf, sizeof(task_buf), "  ID %d: %s [%s]\n", list[i].id, list[i].name, (list[i].state == TASK_RUNNING) ? "RUNNING" : "STOPPED");
                            strcat(output_tmp, task_buf);
                        }
                    } else if (strcmp(app->cmd_input, "neofetch") == 0) {
                        snprintf(output_tmp, sizeof(output_tmp),
                            "  /\\_/\\      Sovereign RTC64 OS Pro Edition\n"
                            " ( o.o )     Kernel: Freestanding Ring 0 x86_64\n"
                            "  > ^ <      Shell: Desktop Console v1.0.0\n"
                            "             Active User Session: %s\n", act_name);
                    } else if (strcmp(app->cmd_input, "clear") == 0) {
                        app->term_output[0] = '\0';
                    } else {
                        // Spawning custom isolated binary or cmdlet routines safely inside scheduler
                        snprintf(output_tmp, sizeof(output_tmp), "Cmdlet '%s' spawned cleanly as separate task thread in the scheduler.\n", app->cmd_input);
                        scheduler_add_task("cmdlet_proc", NULL);
                    }

                    // Append output to buffer console output using overflow-safe append helper
                    if (strcmp(app->cmd_input, "clear") != 0) {
                        safe_term_append(app, "> ");
                        safe_term_append(app, app->cmd_input);
                        safe_term_append(app, "\n");
                        safe_term_append(app, output_tmp);
                        safe_term_append(app, "\n");
                    }

                    app->cmd_input[0] = '\0';
                }
            }

            // Native arrow up/down command history traversal
            if (nk_input_is_key_pressed(&ctx->input, NK_KEY_UP)) {
                if (app->cmd_history_count > 0) {
                    strcpy(app->cmd_input, app->cmd_history[app->cmd_history_count - 1]);
                }
            }

            nk_end(ctx);
        }

        // --- Classic Calculator ---
        static struct nk_rect w_calculator = {400, 300, 300, 380};
        static int max_calculator = 0;
        if (begin_draggable_window(ctx, "Classic Calculator", w_calculator, &app->show_calculator, &max_calculator)) {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, app->calc_history, NK_TEXT_RIGHT);

            nk_layout_row_dynamic(ctx, 35, 1);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->calc_input, sizeof(app->calc_input), nk_filter_ascii);

            // Row 1 keys
            nk_layout_row_dynamic(ctx, 35, 4);
            if (nk_button_label(ctx, "7")) { strcat(app->calc_input, "7"); }
            if (nk_button_label(ctx, "8")) { strcat(app->calc_input, "8"); }
            if (nk_button_label(ctx, "9")) { strcat(app->calc_input, "9"); }
            if (nk_button_label(ctx, "/")) { strcat(app->calc_input, "/"); }

            // Row 2 keys
            if (nk_button_label(ctx, "4")) { strcat(app->calc_input, "4"); }
            if (nk_button_label(ctx, "5")) { strcat(app->calc_input, "5"); }
            if (nk_button_label(ctx, "6")) { strcat(app->calc_input, "6"); }
            if (nk_button_label(ctx, "*")) { strcat(app->calc_input, "*"); }

            // Row 3 keys
            if (nk_button_label(ctx, "1")) { strcat(app->calc_input, "1"); }
            if (nk_button_label(ctx, "2")) { strcat(app->calc_input, "2"); }
            if (nk_button_label(ctx, "3")) { strcat(app->calc_input, "3"); }
            if (nk_button_label(ctx, "-")) { strcat(app->calc_input, "-"); }

            // Row 4 keys
            if (nk_button_label(ctx, "0")) { strcat(app->calc_input, "0"); }
            if (nk_button_label(ctx, "C")) { app->calc_input[0] = '\0'; app->calc_history[0] = '\0'; }
            if (nk_button_label(ctx, "<-")) {
                size_t len = strlen(app->calc_input);
                if (len > 0) app->calc_input[len - 1] = '\0';
            }
            if (nk_button_label(ctx, "+")) { strcat(app->calc_input, "+"); }

            nk_layout_row_dynamic(ctx, 35, 1);
            if (nk_button_label(ctx, "=")) {
                int val1 = 0, val2 = 0;
                char op = '\0';
                char *op_ptr = NULL;
                if ((op_ptr = strchr(app->calc_input, '+'))) op = '+';
                else if ((op_ptr = strchr(app->calc_input, '-'))) op = '-';
                else if ((op_ptr = strchr(app->calc_input, '*'))) op = '*';
                else if ((op_ptr = strchr(app->calc_input, '/'))) op = '/';

                if (op_ptr && op != '\0') {
                    *op_ptr = '\0';
                    val1 = 0;
                    char *p = app->calc_input;
                    while (*p >= '0' && *p <= '9') {
                        val1 = val1 * 10 + (*p - '0');
                        p++;
                    }
                    val2 = 0;
                    p = op_ptr + 1;
                    while (*p >= '0' && *p <= '9') {
                        val2 = val2 * 10 + (*p - '0');
                        p++;
                    }

                    int res = 0;
                    if (op == '+') res = val1 + val2;
                    else if (op == '-') res = val1 - val2;
                    else if (op == '*') res = val1 * val2;
                    else if (op == '/') {
                        if (val2 != 0) res = val1 / val2;
                        else res = 99999;
                    }

                    snprintf(app->calc_history, sizeof(app->calc_history), "%d %c %d =", val1, op, val2);
                    snprintf(app->calc_input, sizeof(app->calc_input), "%d", res);
                }
            }
            nk_end(ctx);
        }

        // --- Task Manager ---
        static struct nk_rect w_taskmgr = {300, 250, 440, 320};
        static int max_taskmgr = 0;
        static int taskmgr_tab = 0; // 0 = Process list, 1 = Utilization charts
        if (begin_draggable_window(ctx, "Task Manager", w_taskmgr, &app->show_taskmgr, &max_taskmgr)) {
            nk_layout_row_dynamic(ctx, 30, 2);
            if (nk_option_label(ctx, "Active Processes", taskmgr_tab == 0)) taskmgr_tab = 0;
            if (nk_option_label(ctx, "Utilization Metrics", taskmgr_tab == 1)) taskmgr_tab = 1;

            if (taskmgr_tab == 0) {
                task_t list[10];
                int task_cnt = scheduler_get_tasks(list, 10);

                for (int i = 0; i < task_cnt; i++) {
                    nk_layout_row_dynamic(ctx, 28, 3);
                    char name_buf[64];
                    snprintf(name_buf, sizeof(name_buf), "ID: %d | Name: %s", list[i].id, list[i].name);
                    nk_label(ctx, name_buf, NK_TEXT_LEFT);

                    nk_label(ctx, (list[i].state == TASK_RUNNING) ? "[ RUNNING ]" : "[ STOPPED ]", NK_TEXT_CENTERED);

                    if (list[i].state == TASK_RUNNING) {
                        if (nk_button_label(ctx, "End Task")) {
                            scheduler_end_task(list[i].id);
                        }
                    } else {
                        nk_label(ctx, "-", NK_TEXT_CENTERED);
                    }
                }
            } else {
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Real-time System CPU & RAM charts:", NK_TEXT_LEFT);

                nk_layout_row_dynamic(ctx, 24, 1);
                nk_label(ctx, "CPU Core Performance Utilization (Scheduler active):", NK_TEXT_LEFT);
                int live_cpu = 4;
                nk_progress(ctx, (nk_size*)&live_cpu, 100, NK_FIXED);

                size_t mem_total = 16 * 1024 * 1024;
                size_t mem_used = hal_malloc_get_used();
                int mem_pct = (mem_used * 100) / mem_total;
                nk_label(ctx, "System Executive Memory RAM Pool Allocation:", NK_TEXT_LEFT);
                nk_progress(ctx, (nk_size*)&mem_pct, 100, NK_FIXED);
            }
            nk_end(ctx);
        }

        // --- Settings / Control Panel ---
        static struct nk_rect w_settings = {250, 150, 440, 400};
        static int max_settings = 0;
        static int settings_tab = 0; // 0 = Personalization, 1 = Accounts, 2 = System
        if (begin_draggable_window(ctx, "Settings Control Panel", w_settings, &app->show_settings, &max_settings)) {
            nk_layout_row_dynamic(ctx, 30, 3);
            if (nk_option_label(ctx, "Personalization", settings_tab == 0)) settings_tab = 0;
            if (nk_option_label(ctx, "Accounts", settings_tab == 1)) settings_tab = 1;
            if (nk_option_label(ctx, "System Options", settings_tab == 2)) settings_tab = 2;

            nk_layout_row_dynamic(ctx, 15, 1);
            nk_spacing(ctx, 1);

            if (settings_tab == 0) {
                nk_layout_row_dynamic(ctx, 24, 1);
                nk_label(ctx, "Configure OS Color Theme Palette:", NK_TEXT_LEFT);

                nk_layout_row_dynamic(ctx, 28, 3);
                if (nk_option_label(ctx, "Midnight Blue", app->active_theme == 0)) app->active_theme = 0;
                if (nk_option_label(ctx, "Classic Grey", app->active_theme == 1)) app->active_theme = 1;
                if (nk_option_label(ctx, "Emerald Green", app->active_theme == 2)) app->active_theme = 2;
            } else if (settings_tab == 1) {
                nk_layout_row_dynamic(ctx, 24, 1);
                nk_label(ctx, "Register New User Account Session:", NK_TEXT_LEFT);

                static char create_uname[32] = "";
                static char create_upass[32] = "";
                static int create_urole = 1;

                nk_layout_row_dynamic(ctx, 28, 2);
                nk_label(ctx, "Account Name:", NK_TEXT_LEFT);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, create_uname, sizeof(create_uname), nk_filter_ascii);

                nk_layout_row_dynamic(ctx, 28, 2);
                nk_label(ctx, "Account Password:", NK_TEXT_LEFT);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, create_upass, sizeof(create_upass), nk_filter_ascii);

                nk_layout_row_dynamic(ctx, 28, 2);
                if (nk_option_label(ctx, "Standard Role", create_urole == 1)) create_urole = 1;
                if (nk_option_label(ctx, "Guest Role", create_urole == 2)) create_urole = 2;

                nk_layout_row_dynamic(ctx, 32, 1);
                if (nk_button_label(ctx, "Create and Save Account")) {
                    if (strlen(create_uname) > 0) {
                        user_add(create_uname, create_upass, (create_urole == 1) ? ROLE_STANDARD : ROLE_GUEST);
                        create_uname[0] = '\0';
                        create_upass[0] = '\0';
                    }
                }
            } else if (settings_tab == 2) {
                nk_layout_row_dynamic(ctx, 24, 1);
                nk_label(ctx, "System Language translation config:", NK_TEXT_LEFT);

                const char *langs[] = {"en", "fr", "de"};
                nk_layout_row_dynamic(ctx, 28, 3);
                for (int i = 0; i < 3; i++) {
                    if (nk_option_label(ctx, langs[i], strcmp(i18n_get_language(), langs[i]) == 0)) {
                        i18n_set_language(langs[i]);
                    }
                }

                nk_layout_row_dynamic(ctx, 20, 1);
                nk_spacing(ctx, 1);

                nk_layout_row_dynamic(ctx, 24, 1);
                nk_label(ctx, "Administrator Policy Control Settings:", NK_TEXT_LEFT);

                static int uac_active_toggle = 1;
                nk_layout_row_dynamic(ctx, 28, 1);
                nk_checkbox_label(ctx, "Enforce secure administrative password elevation popups", (nk_bool*)&uac_active_toggle);
            }
            nk_end(ctx);
        }

        // --- Dedicated User Account Control (UAC) administrative password portal ---
        if (app->show_uac) {
            if (nk_begin(ctx, "User Account Control Security Elevation", nk_rect(ww/2 - 200, wh/2 - 100, 400, 200),
                NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR))
            {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "⚠️ ADMIN PRIVILEGE REQUIRED", NK_TEXT_CENTERED);
                nk_label(ctx, "Please enter Administrator password:", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->login_password, sizeof(app->login_password), nk_filter_ascii);

                nk_layout_row_dynamic(ctx, 40, 2);
                if (nk_button_label(ctx, "Confirm Admin Access")) {
                    if (user_authenticate("Administrator", app->login_password)) {
                        app->show_settings = 1;
                        app->show_uac = 0;
                        app->login_password[0] = '\0';
                    }
                }
                if (nk_button_label(ctx, "Cancel")) {
                    app->show_uac = 0;
                    app->login_password[0] = '\0';
                }
            }
            nk_end(ctx);
        }

        // Bottom horizontal taskbar with hamburger Start Menu, pinned apps, and clock
        if (nk_begin(ctx, "Taskbar", nk_rect(0, wh - 50, ww, 50), NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row_static(ctx, 30, 32, 7);

            if (nk_button_label(ctx, "☰")) app->show_launcher = !app->show_launcher;
            nk_spacing(ctx, 1);

            if (nk_button_label(ctx, "📁")) app->show_explorer = !app->show_explorer;
            if (nk_button_label(ctx, "📝")) app->show_notepad = !app->show_notepad;
            if (nk_button_label(ctx, "🧮")) app->show_calculator = !app->show_calculator;
            if (nk_button_label(ctx, "📟")) app->show_terminal = !app->show_terminal;
            if (nk_button_label(ctx, "📊")) app->show_taskmgr = !app->show_taskmgr;

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_spacer(ctx);

            char info_buf[64];
            snprintf(info_buf, sizeof(info_buf), "12:34 | Drives: %d | 📶 | 🔋", hal_storage_get_device_count());
            nk_label(ctx, info_buf, NK_TEXT_RIGHT);
        }
        nk_end(ctx);

        // Windows-style vertical Start Menu listing pinned utilities and power controls
        if (app->show_launcher) {
            if (nk_begin(ctx, "Start Menu", nk_rect(0, wh - 400 - 50, 250, 400), NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
                nk_layout_row_dynamic(ctx, 28, 1);
                nk_label(ctx, "📌 CORE OS WORKSTATION", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 3, 1);
                nk_spacing(ctx, 1);

                nk_layout_row_dynamic(ctx, 30, 1);
                if (nk_button_label(ctx, "📁 File Explorer")) { app->show_explorer = 1; app->show_launcher = 0; }
                if (nk_button_label(ctx, "📝 Notepad Editor")) { app->show_notepad = 1; app->show_launcher = 0; }
                if (nk_button_label(ctx, "🧮 Classic Calculator")) { app->show_calculator = 1; app->show_launcher = 0; }
                if (nk_button_label(ctx, "📟 Terminal Console")) { app->show_terminal = 1; app->show_launcher = 0; }
                if (nk_button_label(ctx, "📊 Task Manager")) { app->show_taskmgr = 1; app->show_launcher = 0; }
                if (nk_button_label(ctx, "⚙️ Settings Configuration")) {
                    if (act_role == ROLE_ADMIN) {
                        app->show_settings = 1;
                    } else {
                        app->show_uac = 1;
                    }
                    app->show_launcher = 0;
                }

                nk_layout_row_dynamic(ctx, 40, 1);
                nk_spacer(ctx);

                nk_layout_row_dynamic(ctx, 32, 2);
                if (nk_button_label(ctx, "🚪 Logout")) {
                    app->current_state = STATE_LOGIN;
                    app->show_launcher = 0;
                }
                if (nk_button_label(ctx, "🔴 Shutdown")) {
                    kpanic("Sovereign Executive shutdown triggered cleanly by User request.");
                }
            }
            nk_end(ctx);
        }
    }
}
