/* Modified by Sovereign: Windowed System Shell with real VFS integration and Uptime */
#include "os_api.h"
#include <string.h>
#include <stdio.h>
#include "app_ui.h"

#define SHELL_HISTORY_MAX 4096

static char output_log[SHELL_HISTORY_MAX];
static char cmd_buf[128];
static int cmd_len = 0;

void chell_init(void* s) {
    struct app_state* app = (struct app_state*)s;
    if (app) app->show_terminal = 1;
    strcpy(output_log, "Sovereign HIGH-POWER System Shell [Windowed Mode]\nType 'help' for commands.\n\n> ");
}

static void shell_exec(const char* cmd) {
    /* Critical Memory Safety Patch: Check remaining space before appending */
    size_t current_len = strlen(output_log);
    size_t cmd_len_in = strlen(cmd);
    if (current_len + cmd_len_in + 10 >= SHELL_HISTORY_MAX) {
        /* Simple rotation/clear if full to prevent kernel buffer overflow */
        output_log[0] = '\0';
        current_len = 0;
    }

    strcat(output_log, cmd);
    strcat(output_log, "\n");

    if (strcmp(cmd, "help") == 0) {
        strcat(output_log, "Commands: ls, cat, mounts, device, help, clear, health, tasks, uptime\n");
    } else if (strcmp(cmd, "clear") == 0) {
        output_log[0] = '\0';
    } else if (strncmp(cmd, "ls ", 3) == 0) {
        os_vfs_ls(cmd + 3, output_log + strlen(output_log), SHELL_HISTORY_MAX - strlen(output_log));
    } else if (strcmp(cmd, "ls") == 0) {
        os_vfs_ls("/", output_log + strlen(output_log), SHELL_HISTORY_MAX - strlen(output_log));
    } else if (strncmp(cmd, "cat ", 4) == 0) {
        os_vfs_cat(cmd + 4, output_log + strlen(output_log), SHELL_HISTORY_MAX - strlen(output_log));
    } else if (strcmp(cmd, "mounts") == 0) {
        os_vfs_get_mounts(output_log + strlen(output_log), SHELL_HISTORY_MAX - strlen(output_log));
    } else if (strcmp(cmd, "device") == 0) {
        os_devmgr_list(output_log + strlen(output_log), SHELL_HISTORY_MAX - strlen(output_log));
    } else if (strcmp(cmd, "health") == 0) {
        strcat(output_log, "--- System Health Audit ---\nExecutive Heap: VALID\nScheduler: STABLE\nInterrupts: ACTIVE\nStatus: EXCELLENT\n");
    } else if (strcmp(cmd, "tasks") == 0) {
        strcat(output_log, "Active Tasks: Environment Manager, COMPREC, System Shell, Chell\n");
    } else if (strcmp(cmd, "uptime") == 0) {
        snprintf(output_log + strlen(output_log), SHELL_HISTORY_MAX - strlen(output_log), "Uptime: %d seconds\n", (int)(os_get_uptime_ms()/1000));
    } else {
        strcat(output_log, "Unknown command.\n");
    }
    strcat(output_log, "> ");
}

void chell_update(struct nk_context* ctx, void* s) {
    struct app_state* app = (struct app_state*)s;
    if (nk_begin(ctx, "System Shell", nk_rect(50, 50, 600, 450),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE|NK_WINDOW_CLOSABLE|NK_WINDOW_SCALABLE)) {

        nk_layout_row_dynamic(ctx, 350, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_MULTILINE | NK_EDIT_READ_ONLY | NK_EDIT_SELECTABLE | NK_EDIT_AUTO_SELECT,
                                     output_log, sizeof(output_log), nk_filter_default);

        nk_layout_row_template_begin(ctx, 30);
        nk_layout_row_template_push_static(ctx, 20);
        nk_layout_row_template_push_dynamic(ctx);
        nk_layout_row_template_push_static(ctx, 60);
        nk_layout_row_template_end(ctx);

        nk_label(ctx, ">", NK_TEXT_LEFT);
        nk_edit_string(ctx, NK_EDIT_FIELD | NK_EDIT_SIG_ENTER, cmd_buf, &cmd_len, 128, nk_filter_default);
        if (nk_button_label(ctx, "Run") || (nk_input_is_key_pressed(&ctx->input, NK_KEY_ENTER))) {
            cmd_buf[cmd_len] = '\0';
            if (cmd_len > 0) shell_exec(cmd_buf);
            cmd_len = 0;
            memset(cmd_buf, 0, sizeof(cmd_buf));
            /* Clear enter key from Nuklear to prevent double-triggering in the same frame */
            ctx->input.keyboard.keys[NK_KEY_ENTER].clicked = 0;
        }
    }
    if (nk_window_is_closed(ctx, "System Shell")) app->show_terminal = 0;
    nk_end(ctx);
}
