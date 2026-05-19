#include "pro_os.h"
#include "hal.h"
#include "ff.h"
#include "installer.h"
#include <string.h>
#include <stdio.h>

void installer_init(struct installer_state *state) {
    memset(state, 0, sizeof(struct installer_state));
    state->active = 1;
    state->selected_drive = -1;
    strcpy(state->status, "Select a system drive to begin installation.");
}

static void installer_format(struct installer_state *state) {
    if (state->selected_drive < 0) return;

    storage_device_t *dev = hal_storage_get_device(state->selected_drive);
    if (dev->type == STORAGE_TYPE_USB) {
        strcpy(state->status, "Error: System cannot be installed on USB thumb drives.");
        return;
    }

    char drv[4];
    snprintf(drv, 4, "%d:", state->selected_drive);

    MKFS_PARM opt = {FM_FAT32, 0, 0, 0, 0};
    void *work = malloc(FF_MAX_SS);
    if (!work) return;

    strcpy(state->status, "Formatting partition... please wait.");
    FRESULT res = f_mkfs(drv, &opt, work, FF_MAX_SS);
    free(work);

    if (res == FR_OK) {
        state->step = 2;
        strcpy(state->status, "Installation complete! Drive is now mounted and ready.");
        extern void vfs_refresh_mounts(void);
        vfs_refresh_mounts();
    } else {
        snprintf(state->status, 128, "Error: Format failed (code %d)", res);
    }
}

void installer_ui_render(struct nk_context *ctx, struct installer_state *state) {
    if (!state->active) return;

    if (nk_begin(ctx, "Sovereign Installer", nk_rect(100, 100, 500, 400),
        NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, state->status, NK_TEXT_CENTERED);

        if (state->step == 0) {
            nk_layout_row_dynamic(ctx, 200, 1);
            if (nk_group_begin(ctx, "DriveList", NK_WINDOW_BORDER)) {
                int count = hal_storage_get_device_count();
                for (int i = 0; i < count; i++) {
                    storage_device_t *dev = hal_storage_get_device(i);
                    char buf[64];
                    snprintf(buf, 64, "%s (%s)", dev->name, (dev->type == STORAGE_TYPE_USB) ? "USB - Blocked" : "System");
                    if (nk_button_label(ctx, buf)) {
                        state->selected_drive = i;
                    }
                }
                nk_group_end(ctx);
            }

            if (state->selected_drive >= 0) {
                nk_layout_row_dynamic(ctx, 30, 1);
                if (nk_button_label(ctx, "CONFIRM & FORMAT")) {
                    state->step = 1;
                    installer_format(state);
                }
            }
        } else if (state->step == 2) {
            nk_layout_row_dynamic(ctx, 40, 1);
            if (nk_button_label(ctx, "START SOVEREIGN OS")) {
                state->active = 0;
            }
        }
    }
    nk_end(ctx);
}
