/* Modified by Sovereign: License Compliance Update */
#include "os_api.h"
#include <string.h>
#include <stdbool.h>
#include "app_ui.h"

struct installer_state {
    int selected_drive;
    bool installing;
    float progress;
};

static struct installer_state state;

void installer_init(void* s) {
    struct app_state* app = (struct app_state*)s;
    if (app) app->current_state = STATE_INSTALLER;
    memset(&state, 0, sizeof(state));
    state.selected_drive = -1;
}

void installer_update(struct nk_context* ctx, void* s) {
    struct app_state* app = (struct app_state*)s;
    (void)app;
    if (nk_begin(ctx, "Sovereign Installer", nk_rect(200, 100, 400, 500), NK_WINDOW_BORDER|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "Select target for OS deployment:", NK_TEXT_LEFT);

        static char dev_info[512];
        os_devmgr_list(dev_info, sizeof(dev_info));
        nk_layout_row_dynamic(ctx, 100, 1);
        nk_label_wrap(ctx, dev_info);

        nk_layout_row_dynamic(ctx, 30, 1);
        if (nk_button_label(ctx, "Probe and Refresh")) { /* Re-scan logic */ }

        if (state.installing) {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_progress(ctx, (nk_size*)&state.progress, 100, NK_FIXED);
            state.progress += 0.1f;
            if (state.progress >= 100.0f) {
                state.installing = false;
                os_vfs_write("/boot/installed.sig", "VERIFIED");
            }
        } else {
            if (nk_button_label(ctx, "Start Installation")) {
                state.installing = true;
                state.progress = 0;
            }
        }
    }
    nk_end(ctx);
}
