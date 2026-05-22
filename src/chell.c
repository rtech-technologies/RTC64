#include "pro_os.h"
#include "app_ui.h"
#include <string.h>
#include "serial.h"

void chell_init(void* s) { (void)s; serial_write("[Chell] Shell ready.\n"); }
void chell_update(struct nk_context* ctx, void* s) {
    (void)s;
    if (nk_begin(ctx, "Chell", nk_rect(50, 50, 400, 300), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "Sovereign Shell v1.1 (Stage 4)", NK_TEXT_LEFT);
        static char cmd[64]; static int cmd_len;
        nk_edit_string(ctx, NK_EDIT_FIELD, cmd, &cmd_len, 64, nk_filter_default);
        if (nk_button_label(ctx, "Execute")) {
            serial_printf("[Chell] Executing: %s\n", cmd);
        }
    }
    nk_end(ctx);
}
