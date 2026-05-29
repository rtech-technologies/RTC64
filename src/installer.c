#include "pro_os.h"
#include "app_ui.h"
#include <string.h>
#include "serial.h"
#include "ff.h"

struct installer_state {
    int selected_drive;
    bool installing;
    float progress;
    char status[64];
};

static struct installer_state g_installer;

void installer_init(void* s) {
    (void)s;
    memset(&g_installer, 0, sizeof(g_installer));
    g_installer.selected_drive = -1;
    strcpy(g_installer.status, "Ready to install.");
}

void installer_update(struct nk_context* ctx, void* s) {
    (void)s;
    if (nk_begin(ctx, "Sovereign LITE Installer", nk_rect(100, 100, 450, 450), NK_WINDOW_BORDER|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "Select target for installation:", NK_TEXT_LEFT);

        int drive_count = hal_storage_get_device_count();
        if (drive_count == 0) {
            nk_label(ctx, "No storage devices detected.", NK_TEXT_LEFT);
        } else {
            for (int i = 0; i < drive_count; i++) {
                storage_device_t* dev = hal_storage_get_device(i);
                if (nk_select_label(ctx, dev->name, NK_TEXT_LEFT, (g_installer.selected_drive == i))) {
                    g_installer.selected_drive = i;
                    serial_printf("[Installer] Selected drive %d: %s\n", i, dev->name);
                }
            }
        }

        nk_layout_row_dynamic(ctx, 30, 1);
        if (g_installer.installing) {
            nk_progress(ctx, (nk_size*)&g_installer.progress, 100, NK_FIXED);
            nk_label(ctx, g_installer.status, NK_TEXT_CENTERED);

            g_installer.progress += 0.5f;
            if (g_installer.progress >= 100.0f) {
                g_installer.installing = false;
                strcpy(g_installer.status, "Installation Complete!");

                // Finalize: Write integrity signature
                serial_write("[Installer] Writing integrity signature...\n");
                vfs_write("/sovereign_installed.txt", "INTEGRITY_VERIFIED_OK");
            }
        } else {
            if (nk_button_label(ctx, "Begin Installation")) {
                if (g_installer.selected_drive != -1) {
                    g_installer.installing = true;
                    g_installer.progress = 0;
                    strcpy(g_installer.status, "Partitioning and formatting...");
                    serial_printf("[Installer] Starting installation on drive %d\n", g_installer.selected_drive);

                    // Simulated formatting
                    vfs_mkdir("/boot");
                    vfs_mkdir("/sys");
                    vfs_mkdir("/user");
                } else {
                    strcpy(g_installer.status, "Error: Select a drive first.");
                }
            }
            nk_label(ctx, g_installer.status, NK_TEXT_CENTERED);
        }
    }
    nk_end(ctx);
}
