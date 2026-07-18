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

/* Standard database functions exported from uac_policy.c */
extern int db_user_exists(const char *username);
extern int db_register_user(const char *username, const char *password, int is_admin);
extern int db_verify_user(const char *username, const char *password, int *out_is_admin);

/* Standard Math routines used for Analog Clock */
extern double sin(double x);
extern double cos(double x);

void ui_init_style(struct nk_context *ctx)
{
    struct nk_color table[NK_COLOR_COUNT];
    table[NK_COLOR_TEXT] = nk_rgba(230, 245, 255, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(15, 20, 30, 250);
    table[NK_COLOR_HEADER] = nk_rgba(35, 45, 60, 255);
    table[NK_COLOR_BORDER] = nk_rgba(70, 110, 150, 255);
    table[NK_COLOR_BUTTON] = nk_rgba(40, 60, 90, 255);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgba(60, 90, 130, 255);
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(80, 120, 170, 255);
    table[NK_COLOR_TOGGLE] = nk_rgba(30, 40, 60, 255);
    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(70, 200, 255, 255);
    table[NK_COLOR_SELECT] = nk_rgba(40, 60, 90, 255);
    table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(70, 200, 255, 255);
    table[NK_COLOR_SLIDER] = nk_rgba(30, 40, 60, 255);
    table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(70, 200, 255, 255);
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(100, 220, 255, 255);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(150, 240, 255, 255);
    table[NK_COLOR_PROPERTY] = nk_rgba(30, 40, 60, 255);
    table[NK_COLOR_EDIT] = nk_rgba(20, 25, 35, 255);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(70, 200, 255, 255);
    table[NK_COLOR_COMBO] = nk_rgba(30, 40, 60, 255);
    table[NK_COLOR_CHART] = nk_rgba(30, 40, 60, 255);
    table[NK_COLOR_CHART_COLOR] = nk_rgba(70, 200, 255, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 60, 60, 255);
    table[NK_COLOR_SCROLLBAR] = nk_rgba(20, 25, 35, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(60, 90, 130, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(80, 110, 160, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(100, 140, 200, 255);
    table[NK_COLOR_TAB_HEADER] = nk_rgba(30, 40, 60, 255);
    nk_style_from_table(ctx, table);
}

static void ui_render_taskbar(struct nk_context *ctx, struct app_state *app, int ww, int wh) {
    if (nk_begin(ctx, "Taskbar", nk_rect(0, wh - 45, ww, 45), NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_static(ctx, 30, 80, 8);
        if (nk_button_label(ctx, "Menu 🎛️")) app->show_launcher = !app->show_launcher;

        // Active app quick selectors
        if (nk_button_label(ctx, "Shell")) app->show_terminal = !app->show_terminal;
        if (nk_button_label(ctx, "Explorer")) app->show_explorer = !app->show_explorer;
        if (nk_button_label(ctx, "Settings")) app->show_settings = !app->show_settings;
        if (nk_button_label(ctx, "Notepad")) app->show_notepad = !app->show_notepad;

        nk_layout_row_dynamic(ctx, 30, 1);
        nk_spacer(ctx);

        char tray_buf[128];
        snprintf(tray_buf, sizeof(tray_buf), "User: %s | Disks: %d | 📶 | 12:00 PM",
                 app->current_user, hal_storage_get_device_count());
        nk_label(ctx, tray_buf, NK_TEXT_RIGHT);
    }
    nk_end(ctx);
}

/* Background gradient drawer */
static void draw_desktop_gradient(struct nk_context *ctx, struct app_state *app, int ww, int wh) {
    struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);

    struct nk_color c1, c2;
    if (app->wallpaper_theme == 0) { // Charcoal
        c1 = nk_rgb(10, 10, 12);
        c2 = nk_rgb(40, 40, 48);
    } else if (app->wallpaper_theme == 1) { // Sovereign Blue
        c1 = nk_rgb(2, 5, 18);
        c2 = nk_rgb(15, 30, 60);
    } else if (app->wallpaper_theme == 2) { // Industrial Plum
        c1 = nk_rgb(12, 5, 18);
        c2 = nk_rgb(45, 22, 55);
    } else { // Pitch Black
        c1 = nk_rgb(2, 2, 2);
        c2 = nk_rgb(12, 12, 12);
    }

    int steps = 30;
    int h_step = wh / steps;
    for (int i = 0; i < steps; i++) {
        float t = (float)i / (float)steps;
        struct nk_color c = nk_rgb(
            c1.r + (int)((c2.r - c1.r) * t),
            c1.g + (int)((c2.g - c1.g) * t),
            c1.b + (int)((c2.b - c1.b) * t)
        );
        struct nk_rect r = nk_rect(0, i * h_step, ww, h_step + 1);
        nk_fill_rect(canvas, r, 0.0f, c);
    }
}

/* Vector Analog Clock Widget */
static void draw_analog_clock(struct nk_context *ctx, int cx, int cy, int r) {
    struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);

    // Outer Dial
    nk_stroke_circle(canvas, nk_rect(cx - r, cy - r, r*2, r*2), 3.0f, nk_rgb(70, 200, 255));
    // Core center pin
    nk_fill_circle(canvas, nk_rect(cx - 4, cy - 4, 8, 8), nk_rgb(240, 240, 255));

    static int ticks = 0;
    ticks++;

    float seconds = (float)((ticks / 5) % 60); // swept for active feeling
    float minutes = (float)((ticks / 300) % 60);
    float hours = (float)((ticks / 3600) % 12);

    float sec_angle = seconds * (2.0f * 3.14159265f / 60.0f) - 1.57079632f;
    float min_angle = minutes * (2.0f * 3.14159265f / 60.0f) - 1.57079632f;
    float hr_angle = hours * (2.0f * 3.14159265f / 12.0f) - 1.57079632f;

    // Seconds (Red)
    int sx = cx + (int)(cos(sec_angle) * (r - 10));
    int sy = cy + (int)(sin(sec_angle) * (r - 10));
    nk_stroke_line(canvas, cx, cy, sx, sy, 1.5f, nk_rgb(255, 60, 60));

    // Minutes (Light Grey)
    int mx = cx + (int)(cos(min_angle) * (r - 15));
    int my = cy + (int)(sin(min_angle) * (r - 15));
    nk_stroke_line(canvas, cx, cy, mx, my, 2.5f, nk_rgb(220, 220, 220));

    // Hours (Sovereign Cyan)
    int hx = cx + (int)(cos(hr_angle) * (r - 25));
    int hy = cy + (int)(sin(hr_angle) * (r - 25));
    nk_stroke_line(canvas, cx, cy, hx, hy, 4.0f, nk_rgb(70, 200, 255));
}

/* March 2025 Calendar Widget */
static void draw_calendar_widget(struct nk_context *ctx, int x, int y) {
    if (nk_begin(ctx, "March 2025 Calendar", nk_rect(x, y, 220, 220),
                 NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(ctx, 18, 1);
        nk_label(ctx, "MARCH 2025", NK_TEXT_CENTERED);

        nk_layout_row_dynamic(ctx, 15, 7);
        const char *days[] = {"S", "M", "T", "W", "T", "F", "S"};
        for (int i = 0; i < 7; i++) nk_label(ctx, days[i], NK_TEXT_CENTERED);

        // March 2025 starts on a Saturday (6 empty spacers)
        nk_layout_row_dynamic(ctx, 18, 7);
        for (int i = 0; i < 6; i++) nk_spacer(ctx);
        nk_label(ctx, "1", NK_TEXT_CENTERED);

        int day = 1;
        for (int row = 0; row < 5; row++) {
            nk_layout_row_dynamic(ctx, 18, 7);
            for (int col = 0; col < 7; col++) {
                if (day < 31) {
                    day++;
                    char buf[8];
                    snprintf(buf, sizeof(buf), "%d", day);
                    if (day == 12) { // Highlight Wednesday Mar 12
                        nk_label_colored(ctx, buf, NK_TEXT_CENTERED, nk_rgb(255, 230, 40));
                    } else {
                        nk_label(ctx, buf, NK_TEXT_CENTERED);
                    }
                } else {
                    nk_spacer(ctx);
                }
            }
        }
    }
    nk_end(ctx);
}


/* Append text to Terminal History */
static void term_print(struct app_state *app, const char *text) {
    if (strlen(app->term_history) + strlen(text) < sizeof(app->term_history) - 2) {
        strcat(app->term_history, text);
    } else {
        // Roll buffer
        app->term_history[0] = '\0';
        strcpy(app->term_history, "[Console Buffer Recycled]\n");
        strcat(app->term_history, text);
    }
}

static int parse_cmd_words(const char *src, char *w1, char *w2, char *w3, char *w4) {
    int count = 0;
    const char *p = src;
    char *outs[4] = {w1, w2, w3, w4};
    for (int i = 0; i < 4; i++) outs[i][0] = '\0';

    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        char *out = outs[count];
        int len = 0;
        while (*p && *p != ' ' && *p != '\t' && len < 127) {
            out[len++] = *p++;
        }
        out[len] = '\0';
        count++;
        if (count >= 4) break;
    }
    return count;
}

/* Terminal Command Execution Logic */
static void execute_terminal_command(struct app_state *app) {
    char cmd[128] = {0};
    char arg1[128] = {0};
    char arg2[128] = {0};
    char arg3[128] = {0};

    int scanned = parse_cmd_words(app->term_cmd, cmd, arg1, arg2, arg3);
    if (scanned <= 0) return;

    // Append command prefix to history
    char prefix[256];
    snprintf(prefix, sizeof(prefix), "%s@sovereign-workstation:~$ %s\n", app->current_user, app->term_cmd);
    term_print(app, prefix);

    if (strcmp(cmd, "help") == 0) {
        term_print(app, "Available Workstation Commands:\n"
                        "  help                           - Show command list\n"
                        "  ls                             - List directory content\n"
                        "  cat <file>                     - Display file contents\n"
                        "  touch <file>                   - Create empty file\n"
                        "  mkdir <dir>                    - Create new directory\n"
                        "  rm <file>                      - Remove a file\n"
                        "  whoami                         - Print current user session details\n"
                        "  sysinfo                        - Print hardware diagnostic specs\n"
                        "  su <user> <pass>               - Login as standard or admin user\n"
                        "  sudo <cmd>                     - Elevate command via UAC protection\n"
                        "  useradd <user> <pass> <0/1>    - Create new standard(0) or admin(1) account\n"
                        "  clear                          - Reset screen history buffer\n");
    } else if (strcmp(cmd, "clear") == 0) {
        app->term_history[0] = '\0';
    } else if (strcmp(cmd, "whoami") == 0) {
        char who[256];
        snprintf(who, sizeof(who), "User: %s (%s privilege mode)\n",
                 app->current_user, app->is_admin ? "Administrator" : "Standard User");
        term_print(app, who);
    } else if (strcmp(cmd, "sysinfo") == 0) {
        char sys[512];
        snprintf(sys, sizeof(sys), "Sovereign Workstation v2.5 R-TECH\n"
                                   "System Architecture: x86_64 Freestanding Ring 0/3\n"
                                   "Primary Block Storage: SATA_Disk_0 (2MB)\n"
                                   "Secondary Block Storage: NVMe_Disk_0 (1MB)\n"
                                   "System Memory Allocated: 16MB Heap Space\n"
                                   "Input Stack: PS/2 Keyboard/Mouse & USB HID Drivers Enabled\n");
        term_print(app, sys);
    } else if (strcmp(cmd, "ls") == 0) {
        term_print(app, "Listing Directory: /\n");
        // We'll print directory contents of root flat paths
        struct list_helper {
            struct app_state *a;
        } lh = {app};

        void print_vfs_entry(const char *name, bool is_dir, uint32_t size) {
            char fbuf[128];
            if (is_dir) {
                snprintf(fbuf, sizeof(fbuf), "  [DIR]  %s\n", name);
            } else {
                snprintf(fbuf, sizeof(fbuf), "  [FILE] %s  (%d bytes)\n", name, size);
            }
            term_print(lh.a, fbuf);
        }
        vfs_readdir("/", print_vfs_entry);
    } else if (strcmp(cmd, "touch") == 0) {
        if (scanned < 2) {
            term_print(app, "Error: Missing filename parameter\n");
        } else {
            char filepath[256];
            if (arg1[0] == '/') strcpy(filepath, arg1);
            else snprintf(filepath, sizeof(filepath), "/%s", arg1);

            vfs_write(filepath, "", 0);
            term_print(app, "File touched successfully.\n");
        }
    } else if (strcmp(cmd, "mkdir") == 0) {
        if (scanned < 2) {
            term_print(app, "Error: Missing directory name\n");
        } else {
            char dirpath[256];
            if (arg1[0] == '/') strcpy(dirpath, arg1);
            else snprintf(dirpath, sizeof(dirpath), "/%s", arg1);

            vfs_mkdir(dirpath);
            term_print(app, "Directory created.\n");
        }
    } else if (strcmp(cmd, "cat") == 0) {
        if (scanned < 2) {
            term_print(app, "Error: Missing filename\n");
        } else {
            char filepath[256];
            if (arg1[0] == '/') strcpy(filepath, arg1);
            else snprintf(filepath, sizeof(filepath), "/%s", arg1);

            char text_buf[512] = {0};
            int r = vfs_read(filepath, text_buf, sizeof(text_buf) - 1);
            if (r < 0) {
                term_print(app, "Error: File not found.\n");
            } else {
                text_buf[r] = '\0';
                term_print(app, text_buf);
                term_print(app, "\n");
            }
        }
    } else if (strcmp(cmd, "rm") == 0) {
        if (scanned < 2) {
            term_print(app, "Error: Missing filename\n");
        } else {
            char filepath[256];
            if (arg1[0] == '/') strcpy(filepath, arg1);
            else snprintf(filepath, sizeof(filepath), "/%s", arg1);

            if (vfs_rm(filepath) == 0) {
                term_print(app, "File deleted successfully.\n");
            } else {
                term_print(app, "Error: File could not be deleted.\n");
            }
        }
    } else if (strcmp(cmd, "su") == 0) {
        if (scanned < 3) {
            term_print(app, "Usage: su <username> <password>\n");
        } else {
            int admin = 0;
            if (db_verify_user(arg1, arg2, &admin) == 0) {
                strcpy(app->current_user, arg1);
                app->is_admin = admin;
                term_print(app, "Session switch successful.\n");
            } else {
                term_print(app, "Authentication failed. Incorrect username or password.\n");
            }
        }
    } else if (strcmp(cmd, "sudo") == 0) {
        term_print(app, "Elevation requested...\n");
        // We'll trigger UAC for administrative action
        app->show_uac = 1;
        strcpy(app->uac_action_desc, "Execute root command");
        app->uac_admin_pass_typed[0] = '\0';
        app->uac_authorized = 0;
    } else if (strcmp(cmd, "useradd") == 0) {
        if (scanned < 4) {
            term_print(app, "Usage: useradd <username> <password> <is_admin: 0 or 1>\n");
        } else {
            // Check UAC permission before executing
            if (!app->is_admin) {
                term_print(app, "Elevation required. Standard users cannot add users directly.\n");
            } else {
                int add_adm = (arg3[0] == '1') ? 1 : 0;
                if (db_register_user(arg1, arg2, add_adm) == 0) {
                    char add_ok[128];
                    snprintf(add_ok, sizeof(add_ok), "User %s added successfully topasswd.\n", arg1);
                    term_print(app, add_ok);
                } else {
                    term_print(app, "Error: Username duplicate or registration failure.\n");
                }
            }
        }
    } else {
        term_print(app, "Command not recognized. Type 'help' for command list.\n");
    }

    app->term_cmd[0] = '\0';
}

void ui_render(struct nk_context *ctx, struct app_state *app, int window_width, int window_height)
{
    if (app->current_state == STATE_LOGIN) {
        // Formulate gradient background
        draw_desktop_gradient(ctx, app, window_width, window_height);

        if (nk_begin(ctx, "Sovereign OS Login Panel",
                     nk_rect(window_width/2 - 180, window_height/2 - 160, 360, 320),
                     NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(ctx, 45, 1);
            nk_label(ctx, "🖥️ SOVEREIGN WORKSTATION", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(ctx, 25, 1);
            nk_label(ctx, "Username:", NK_TEXT_LEFT);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->login_username, sizeof(app->login_username), nk_filter_default);

            nk_layout_row_dynamic(ctx, 25, 1);
            nk_label(ctx, "Password:", NK_TEXT_LEFT);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->login_password, sizeof(app->login_password), nk_filter_default);

            nk_layout_row_dynamic(ctx, 35, 1);
            if (nk_button_label(ctx, "Login 🔓")) {
                int admin = 0;
                if (db_verify_user(app->login_username, app->login_password, &admin) == 0) {
                    strcpy(app->current_user, app->login_username);
                    app->is_admin = admin;
                    app->current_state = STATE_DESKTOP;
                    // Prepopulate explorer current path
                    snprintf(app->explorer_path, sizeof(app->explorer_path), "/Users/%s/Desktop", app->login_username);
                    app->login_password[0] = '\0';
                    comprec_log("Stage 14: Sovereign Desktop active in Ring 3 workstation space.");
                } else {
                    // Password failed
                    // Check if default accounts haven't been created yet (force installation first)
                    if (vfs_exists("/System/Config/passwd")) {
                        app->login_password[0] = '\0';
                    } else {
                        app->current_state = STATE_INSTALLER;
                    }
                }
            }

            // Helpful tip
            nk_layout_row_dynamic(ctx, 25, 1);
            if (!vfs_exists("/System/Config/passwd")) {
                nk_label_colored(ctx, "Unconfigured. Launching Setup installer...", NK_TEXT_CENTERED, nk_rgb(255, 180, 40));
            }
        }
        nk_end(ctx);
    } else if (app->current_state == STATE_INSTALLER) {
        draw_desktop_gradient(ctx, app, window_width, window_height);

        if (nk_begin(ctx, "Sovereign Workstation Setup Wizard",
                     nk_rect(window_width/2 - 240, window_height/2 - 200, 480, 400),
                     NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, "Install and Configure Workspace Environment", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "1. Create Standard Account Details:", NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 25, 2);
            nk_label(ctx, "  Username:", NK_TEXT_LEFT);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->new_username, sizeof(app->new_username), nk_filter_default);

            nk_layout_row_dynamic(ctx, 25, 2);
            nk_label(ctx, "  Password:", NK_TEXT_LEFT);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->new_password, sizeof(app->new_password), nk_filter_default);

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "2. Create Administrator Account Details:", NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 25, 2);
            nk_label(ctx, "  Admin Password:", NK_TEXT_LEFT);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->admin_password, sizeof(app->admin_password), nk_filter_default);

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_spacer(ctx);

            nk_layout_row_dynamic(ctx, 40, 1);
            if (nk_button_label(ctx, "Format & Install Workstation 🚀")) {
                if (app->new_username[0] && app->new_password[0] && app->admin_password[0]) {
                    // Pre-init VFS disk format
                    extern void fat_format(void);
                    fat_format();
                    vfs_mkdir("/System");
                    vfs_mkdir("/System/Config");
                    vfs_mkdir("/Users");

                    // Register credentials
                    db_register_user("Administrator", app->admin_password, 1);
                    db_register_user(app->new_username, app->new_password, 0);

                    // Proceed to login
                    app->current_state = STATE_LOGIN;
                    strcpy(app->login_username, app->new_username);
                }
            }
        }
        nk_end(ctx);
    } else if (app->current_state == STATE_DESKTOP) {
        // 1. Draw Gorgeous Desktop Workspace
        draw_desktop_gradient(ctx, app, window_width, window_height);

        // 2. Vector Analog Clock Widget
        if (app->show_analog_clock) {
            draw_analog_clock(ctx, window_width - 150, 120, 70);
        }

        // 3. March 2025 Calendar Widget
        if (app->show_calendar) {
            draw_calendar_widget(ctx, window_width - 260, 220);
        }

        // 4. Taskbar App Launcher Menu Overlay
        if (app->show_launcher) {
            if (nk_begin(ctx, "AppLauncher", nk_rect(10, window_height - 300, 220, 250),
                         NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BORDER)) {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "📁 Sovereign Menu", NK_TEXT_CENTERED);

                if (nk_button_label(ctx, "💻 Terminal (Shell)")) { app->show_terminal = 1; app->show_launcher = 0; }
                if (nk_button_label(ctx, "📁 File Explorer")) { app->show_explorer = 1; app->show_launcher = 0; }
                if (nk_button_label(ctx, "⚙️ Settings Panel")) { app->show_settings = 1; app->show_launcher = 0; }
                if (nk_button_label(ctx, "📝 Text Notepad")) { app->show_notepad = 1; app->show_launcher = 0; }

                if (nk_button_label(ctx, "🔌 Log Out")) {
                    app->current_state = STATE_LOGIN;
                    app->show_launcher = 0;
                    app->show_terminal = 0;
                    app->show_explorer = 0;
                    app->show_settings = 0;
                    app->show_notepad = 0;
                }
            }
            nk_end(ctx);
        }

        // 5. Terminal Application (Shell)
        if (app->show_terminal) {
            if (nk_begin(ctx, "Terminal Emulator", nk_rect(80, 50, 640, 440),
                         NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {

                // Read-only multi-line scrolling log
                nk_layout_row_dynamic(ctx, 320, 1);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX | NK_EDIT_READ_ONLY, app->term_history, sizeof(app->term_history), nk_filter_default);

                // Command input box
                nk_layout_row_dynamic(ctx, 30, 2);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->term_cmd, sizeof(app->term_cmd), nk_filter_default);
                if (nk_button_label(ctx, "Execute")) {
                    execute_terminal_command(app);
                }
            }
            if (nk_window_is_closed(ctx, "Terminal Emulator")) app->show_terminal = 0;
            nk_end(ctx);
        }

        // 6. File Explorer
        if (app->show_explorer) {
            if (nk_begin(ctx, "File Explorer", nk_rect(160, 100, 500, 360),
                         NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {

                // Typing bar
                nk_layout_row_dynamic(ctx, 30, 2);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->explorer_path, sizeof(app->explorer_path), nk_filter_default);
                if (nk_button_label(ctx, "Go Up (Parent)")) {
                    char *last_slash = strrchr(app->explorer_path, '/');
                    if (last_slash && last_slash != app->explorer_path) {
                        *last_slash = '\0';
                    } else if (last_slash && last_slash == app->explorer_path) {
                        strcpy(app->explorer_path, "/");
                    }
                }

                // List devices
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, "💻 DISK DRIVES & PARTITIONS:", NK_TEXT_LEFT);

                nk_layout_row_dynamic(ctx, 25, 4);
                for (int i = 0; i < hal_storage_get_device_count(); i++) {
                    storage_device_t *dev = hal_storage_get_device(i);
                    if (nk_button_label(ctx, dev->name)) {
                        // Quick navigate to disk's root
                        strcpy(app->explorer_path, "/");
                    }
                }

                // List folders/files under path
                nk_layout_row_dynamic(ctx, 20, 1);
                char info_header[128];
                snprintf(info_header, sizeof(info_header), "Files in %s:", app->explorer_path);
                nk_label(ctx, info_header, NK_TEXT_LEFT);

                struct explorer_helper {
                    struct app_state *a;
                    struct nk_context *c;
                } eh = {app, ctx};

                void list_explorer_entry(const char *name, bool is_dir, uint32_t size) {
                    char display_name[128];
                    if (is_dir) {
                        snprintf(display_name, sizeof(display_name), "📁 %s", name);
                        if (nk_button_label(eh.c, display_name)) {
                            // Navigate deeper
                            if (strcmp(eh.a->explorer_path, "/") == 0) {
                                snprintf(eh.a->explorer_path, sizeof(eh.a->explorer_path), "/%s", name);
                            } else {
                                char tmp[256];
                                snprintf(tmp, sizeof(tmp), "%s/%s", eh.a->explorer_path, name);
                                strcpy(eh.a->explorer_path, tmp);
                            }
                        }
                    } else {
                        snprintf(display_name, sizeof(display_name), "📄 %s (%d B)", name, size);
                        if (nk_button_label(eh.c, display_name)) {
                            // Open in notepad
                            if (strcmp(eh.a->explorer_path, "/") == 0) {
                                snprintf(eh.a->notepad_path, sizeof(eh.a->notepad_path), "/%s", name);
                            } else {
                                snprintf(eh.a->notepad_path, sizeof(eh.a->notepad_path), "%s/%s", eh.a->explorer_path, name);
                            }
                            int len = vfs_read(eh.a->notepad_path, eh.a->notepad_text, sizeof(eh.a->notepad_text) - 1);
                            if (len >= 0) eh.a->notepad_text[len] = '\0';
                            else eh.a->notepad_text[0] = '\0';
                            eh.a->show_notepad = 1;
                        }
                    }
                }

                vfs_readdir(app->explorer_path, list_explorer_entry);
            }
            if (nk_window_is_closed(ctx, "File Explorer")) app->show_explorer = 0;
            nk_end(ctx);
        }

        // 7. Notepad Text Editor
        if (app->show_notepad) {
            if (nk_begin(ctx, "Text Notepad", nk_rect(240, 150, 450, 380),
                         NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {

                nk_layout_row_dynamic(ctx, 25, 2);
                nk_label(ctx, "File Path:", NK_TEXT_LEFT);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->notepad_path, sizeof(app->notepad_path), nk_filter_default);

                // Edit box
                nk_layout_row_dynamic(ctx, 240, 1);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, app->notepad_text, sizeof(app->notepad_text), nk_filter_default);

                nk_layout_row_dynamic(ctx, 30, 2);
                if (nk_button_label(ctx, "Save File")) {
                    vfs_write(app->notepad_path, app->notepad_text, strlen(app->notepad_text));
                }
                if (nk_button_label(ctx, "Clear Editor")) {
                    app->notepad_text[0] = '\0';
                }
            }
            if (nk_window_is_closed(ctx, "Text Notepad")) app->show_notepad = 0;
            nk_end(ctx);
        }

        // 8. Settings Application
        if (app->show_settings) {
            if (nk_begin(ctx, "System Control Settings", nk_rect(220, 80, 480, 420),
                         NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {

                // Tab layout
                static int active_tab = 0;
                nk_layout_row_dynamic(ctx, 30, 3);
                if (nk_button_label(ctx, "🎨 Theme")) active_tab = 0;
                if (nk_button_label(ctx, "👥 Users")) active_tab = 1;
                if (nk_button_label(ctx, "⚙️ Devices")) active_tab = 2;

                if (active_tab == 0) {
                    nk_layout_row_dynamic(ctx, 25, 1);
                    nk_label(ctx, "Customize Desktop Background Scheme:", NK_TEXT_LEFT);

                    nk_layout_row_dynamic(ctx, 25, 4);
                    if (nk_option_label(ctx, "Charcoal", app->wallpaper_theme == 0)) app->wallpaper_theme = 0;
                    if (nk_option_label(ctx, "Midnight Blue", app->wallpaper_theme == 1)) app->wallpaper_theme = 1;
                    if (nk_option_label(ctx, "Industrial Plum", app->wallpaper_theme == 2)) app->wallpaper_theme = 2;
                    if (nk_option_label(ctx, "Pitch Black", app->wallpaper_theme == 3)) app->wallpaper_theme = 3;

                    nk_layout_row_dynamic(ctx, 25, 1);
                    nk_label(ctx, "Active Desktop Widgets:", NK_TEXT_LEFT);

                    nk_layout_row_dynamic(ctx, 25, 2);
                    nk_checkbox_label(ctx, "Show Analog Clock Widget", (nk_bool*)&app->show_analog_clock);
                    nk_checkbox_label(ctx, "Show March 2025 Calendar", (nk_bool*)&app->show_calendar);

                } else if (active_tab == 1) {
                    // USER MANAGEMENT (GATED BY UAC ELEVATION)
                    nk_layout_row_dynamic(ctx, 25, 1);
                    nk_label(ctx, "👥 WORKSTATION USER ACCOUNTS", NK_TEXT_LEFT);

                    // List existing users database
                    nk_layout_row_dynamic(ctx, 120, 1);
                    char passwd_content[1024] = {0};
                    vfs_read("/System/Config/passwd", passwd_content, sizeof(passwd_content) - 1);
                    nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX | NK_EDIT_READ_ONLY, passwd_content, sizeof(passwd_content), nk_filter_default);

                    nk_layout_row_dynamic(ctx, 20, 1);
                    nk_label(ctx, "➕ Create New Account:", NK_TEXT_LEFT);

                    nk_layout_row_dynamic(ctx, 25, 2);
                    nk_label(ctx, "  Username:", NK_TEXT_LEFT);
                    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->add_username, sizeof(app->add_username), nk_filter_default);

                    nk_layout_row_dynamic(ctx, 25, 2);
                    nk_label(ctx, "  Password:", NK_TEXT_LEFT);
                    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->add_password, sizeof(app->add_password), nk_filter_default);

                    nk_layout_row_dynamic(ctx, 25, 2);
                    nk_checkbox_label(ctx, "Grant Admin Privileges", (nk_bool*)&app->add_is_admin);

                    if (nk_button_label(ctx, "Register User Account")) {
                        if (!app->is_admin) {
                            // Standard users trigger UAC popup
                            app->show_uac = 1;
                            strcpy(app->uac_action_desc, "Register custom user account");
                            app->uac_admin_pass_typed[0] = '\0';
                            app->uac_authorized = 0;
                        } else {
                            // Admin runs immediately
                            db_register_user(app->add_username, app->add_password, app->add_is_admin);
                            app->add_username[0] = '\0';
                            app->add_password[0] = '\0';
                        }
                    }

                } else {
                    nk_layout_row_dynamic(ctx, 25, 1);
                    nk_label(ctx, "⚙️ HARDWARE COMPONENT DIAGNOSTICS", NK_TEXT_LEFT);

                    nk_layout_row_dynamic(ctx, 20, 1);
                    nk_label(ctx, "Attached Block Storage Devices:", NK_TEXT_LEFT);
                    for (int i = 0; i < hal_storage_get_device_count(); i++) {
                        storage_device_t *dev = hal_storage_get_device(i);
                        char desc[128];
                        snprintf(desc, sizeof(desc), "  • %s (%s, %d blocks)",
                                 dev->name,
                                 (dev->type == STORAGE_TYPE_SATA) ? "SATA Hard Disk" :
                                 (dev->type == STORAGE_TYPE_NVME) ? "NVMe SSD" : "USB MSC ThumbDrive",
                                 (int)dev->total_blocks);
                        nk_label(ctx, desc, NK_TEXT_LEFT);
                    }
                }
            }
            if (nk_window_is_closed(ctx, "System Control Settings")) app->show_settings = 0;
            nk_end(ctx);
        }

        // 9. Real UAC popup elevation modal dialog box
        if (app->show_uac) {
            // Draw modal window blocking other interfaces
            if (nk_begin(ctx, "🛡️ User Account Control (Elevation Guard)",
                         nk_rect(window_width/2 - 200, window_height/2 - 120, 400, 240),
                         NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_NO_SCROLLBAR)) {

                nk_layout_row_dynamic(ctx, 25, 1);
                if (app->is_admin) {
                    nk_label_colored(ctx, "⚠️ SYSTEM CONFIRMATION REQUIRED", NK_TEXT_CENTERED, nk_rgb(255, 180, 40));

                    nk_layout_row_dynamic(ctx, 20, 1);
                    char prompt_text[128];
                    snprintf(prompt_text, sizeof(prompt_text), "Allow this action: %s?", app->uac_action_desc);
                    nk_label(ctx, prompt_text, NK_TEXT_LEFT);

                    nk_layout_row_dynamic(ctx, 40, 1);
                    nk_spacer(ctx);

                    nk_layout_row_dynamic(ctx, 30, 2);
                    if (nk_button_label(ctx, "Yes, Allow ✅")) {
                        // Authorized! Add the pending action if any
                        if (strcmp(app->uac_action_desc, "Register custom user account") == 0) {
                            db_register_user(app->add_username, app->add_password, app->add_is_admin);
                            app->add_username[0] = '\0';
                            app->add_password[0] = '\0';
                        }
                        app->show_uac = 0;
                    }
                    if (nk_button_label(ctx, "No, Cancel ❌")) {
                        app->show_uac = 0;
                    }
                } else {
                    nk_label_colored(ctx, "🔒 ADMINISTRATIVE ELEVATION REQUIRED", NK_TEXT_CENTERED, nk_rgb(255, 60, 60));

                    nk_layout_row_dynamic(ctx, 18, 1);
                    char prompt_text[128];
                    snprintf(prompt_text, sizeof(prompt_text), "Action: %s", app->uac_action_desc);
                    nk_label(ctx, prompt_text, NK_TEXT_LEFT);

                    nk_layout_row_dynamic(ctx, 25, 1);
                    nk_label(ctx, "Enter Administrator Password:", NK_TEXT_LEFT);
                    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->uac_admin_pass_typed, sizeof(app->uac_admin_pass_typed), nk_filter_default);

                    nk_layout_row_dynamic(ctx, 25, 1);
                    nk_spacer(ctx);

                    nk_layout_row_dynamic(ctx, 30, 2);
                    if (nk_button_label(ctx, "Authorize 🔓")) {
                        int is_adm = 0;
                        if (db_verify_user("Administrator", app->uac_admin_pass_typed, &is_adm) == 0 && is_adm == 1) {
                            // Authorized! Execute pending action
                            if (strcmp(app->uac_action_desc, "Register custom user account") == 0) {
                                db_register_user(app->add_username, app->add_password, app->add_is_admin);
                                app->add_username[0] = '\0';
                                app->add_password[0] = '\0';
                            } else {
                                term_print(app, "UAC Security Authorization Granted.\n");
                            }
                            app->show_uac = 0;
                        } else {
                            strcpy(app->uac_admin_pass_typed, "[INCORRECT PASSWORD]");
                        }
                    }
                    if (nk_button_label(ctx, "Cancel ❌")) {
                        app->show_uac = 0;
                    }
                }
            }
            nk_end(ctx);
        }

        // 10. Draw Tray Utility Bar
        ui_render_taskbar(ctx, app, window_width, window_height);
    }
}
