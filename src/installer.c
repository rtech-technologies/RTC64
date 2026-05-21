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
    state->selected_partition = 1; // Default to partition 1
    strcpy(state->status, "Select a system drive for RTC64 installation.");
}

static void installer_format(struct installer_state *state) {
    if (state->selected_drive < 0) return;

    storage_device_t *dev = hal_storage_get_device(state->selected_drive);
    if (!dev) return;

    if (dev->type == STORAGE_TYPE_USB) {
        strcpy(state->status, "USB formatting BLOCKED for system stability.");
        return;
    }

    char drv[4];
    // Map selection to FatFs volume index (simplified)
    snprintf(drv, 4, "%d:", state->selected_drive);

    MKFS_PARM opt = {FM_ANY, 0, 0, 0, 0};
    void *work = malloc(FF_MAX_SS);
    if (!work) return;

    snprintf(state->status, 128, "Formatting %s... please wait.", dev->name);
    FRESULT res = f_mkfs(drv, &opt, work, dev->block_size);
    free(work);

    if (res == FR_OK) {
        state->step = 2;
        strcpy(state->status, "Success! System partition is active and ready.");
        extern void vfs_refresh_mounts(void);
        vfs_refresh_mounts();
    } else {
        snprintf(state->status, 128, "Error: Format failed (code %d)", res);
    }
}

void installer_ui_render(struct nk_context *ctx, struct installer_state *state) {
    if (!state->active) return;

    if (nk_begin(ctx, "Sovereign LITE Installer", nk_rect(100, 100, 500, 400),
        NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, state->status, NK_TEXT_CENTERED);

        if (state->step == 0) {
            nk_layout_row_dynamic(ctx, 150, 1);
            if (nk_group_begin(ctx, "Drives", NK_WINDOW_BORDER)) {
                int count = hal_storage_get_device_count();
                for (int i = 0; i < count; i++) {
                    storage_device_t *dev = hal_storage_get_device(i);
                    char buf[64];
                    snprintf(buf, 64, "%s [%s]", dev->name, (dev->type == STORAGE_TYPE_USB) ? "EXTERNAL" : "INTERNAL");
                    if (nk_button_label(ctx, buf)) {
                        state->selected_drive = i;
                    }
                }
                nk_group_end(ctx);
            }

            if (state->selected_drive >= 0) {
                nk_layout_row_dynamic(ctx, 30, 2);
                nk_label(ctx, "Partition:", NK_TEXT_LEFT);
                nk_property_int(ctx, "#ID:", 1, &state->selected_partition, 4, 1, 1);

                nk_layout_row_dynamic(ctx, 40, 1);
                if (nk_button_label(ctx, "FORMAT AND MOUNT")) {
                    installer_format(state);
                }
            }
        } else if (state->step == 2) {
            nk_layout_row_dynamic(ctx, 40, 1);
            if (nk_button_label(ctx, "ENTER SOVEREIGN DESKTOP")) {
                state->active = 0;
            }
        }
    }
    nk_end(ctx);
}
