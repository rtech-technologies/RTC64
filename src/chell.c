#include "os_api.h"
#include <string.h>
#include "app_ui.h"

void chell_init(void* s) { (void)s; }

void chell_update(struct nk_context* ctx, void* s) {
    (void)s;
    if (nk_begin(ctx, "Chell", nk_rect(50, 50, 600, 450), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE|NK_WINDOW_CLOSABLE)) {
        static char output[4096];
        static char cmd[128];
        static int cmd_len;

        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "Sovereign Application Shell v2.1", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 250, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_MULTILINE | NK_EDIT_READ_ONLY, output, sizeof(output), nk_filter_default);

        nk_layout_row_template_begin(ctx, 30);
        nk_layout_row_template_push_dynamic(ctx);
        nk_layout_row_template_push_static(ctx, 80);
        nk_layout_row_template_end(ctx);

        nk_edit_string(ctx, NK_EDIT_FIELD, cmd, &cmd_len, 128, nk_filter_default);
        if (nk_button_label(ctx, "Run")) {
            cmd[cmd_len] = '\0';
            if (strncmp(cmd, "ls ", 3) == 0) {
                os_vfs_ls(cmd + 3, output, sizeof(output));
            } else if (strncmp(cmd, "cat ", 4) == 0) {
                os_vfs_cat(cmd + 4, output, sizeof(output));
            } else if (strcmp(cmd, "mount") == 0) {
                os_vfs_get_mounts(output, sizeof(output));
            } else if (strcmp(cmd, "device") == 0) {
                os_devmgr_list(output, sizeof(output));
            } else if (strcmp(cmd, "help") == 0) {
                strcpy(output, "Commands: ls, cat, mount, device, help, clear");
            } else if (strcmp(cmd, "clear") == 0) {
                memset(output, 0, sizeof(output));
            } else {
                strcpy(output, "Unknown command.");
            }
            cmd_len = 0;
        }
    }
    nk_end(ctx);
}
