#include "pro_os.h"
#include "app_ui.h"
#include <string.h>
#include "serial.h"

void chell_init(void* s) { (void)s; }

void chell_update(struct nk_context* ctx, void* s) {
    (void)s;
    if (nk_begin(ctx, "Chell", nk_rect(50, 50, 600, 450), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE|NK_WINDOW_CLOSABLE)) {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "Sovereign Shell v1.6 [R-TECH Core]", NK_TEXT_LEFT);

        static char output[4096];
        nk_layout_row_dynamic(ctx, 250, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_MULTILINE | NK_EDIT_READ_ONLY, output, sizeof(output), nk_filter_default);

        static char cmd[128]; static int cmd_len;
        nk_layout_row_template_begin(ctx, 30);
        nk_layout_row_template_push_dynamic(ctx);
        nk_layout_row_template_push_static(ctx, 80);
        nk_layout_row_template_end(ctx);

        nk_edit_string(ctx, NK_EDIT_FIELD, cmd, &cmd_len, 128, nk_filter_default);
        if (nk_button_label(ctx, "Run") || (cmd_len > 0 && cmd[cmd_len-1] == '\n')) {
            if (cmd_len > 0 && cmd[cmd_len-1] == '\n') cmd[--cmd_len] = '\0';
            else cmd[cmd_len] = '\0';

            serial_printf("[Chell] Exec: %s\n", cmd);

            if (strncmp(cmd, "ls ", 3) == 0) {
                if (vfs_ls(cmd + 3, output, sizeof(output)) != 0) strcpy(output, "Error: Path not found or inaccessible.");
            } else if (strncmp(cmd, "cat ", 4) == 0) {
                if (vfs_cat(cmd + 4, output, sizeof(output)) != 0) strcpy(output, "Error: File not found.");
            } else if (strncmp(cmd, "mkdir ", 6) == 0) {
                if (vfs_mkdir(cmd + 6) == 0) strcpy(output, "Success: Directory created.");
                else strcpy(output, "Error: Failed to create directory.");
            } else if (strncmp(cmd, "write ", 6) == 0) {
                char* path = cmd + 6; char* space = strchr(path, ' ');
                if (space) {
                    *space = '\0'; char* content = space + 1;
                    if (vfs_write(path, content) == 0) strcpy(output, "Success: File written.");
                    else strcpy(output, "Error: Failed to write file.");
                } else strcpy(output, "Usage: write <path> <content>");
            } else if (strcmp(cmd, "mount") == 0) {
                vfs_get_mounts(output, sizeof(output));
            } else if (strcmp(cmd, "help") == 0) {
                strcpy(output, "Sovereign OS Shell Commands:\n"
                               "mount          - List mounted volumes\n"
                               "ls <path>      - List directory contents\n"
                               "cat <path>     - Display file content\n"
                               "mkdir <path>   - Create directory\n"
                               "write <path> <txt> - Create file with text\n"
                               "help           - Show this help");
            } else if (strcmp(cmd, "clear") == 0) {
                memset(output, 0, sizeof(output));
            } else {
                snprintf(output, sizeof(output), "Unknown command: %s. Type 'help' for options.", cmd);
            }
            cmd_len = 0; memset(cmd, 0, sizeof(cmd));
        }
    }
    nk_end(ctx);
}
