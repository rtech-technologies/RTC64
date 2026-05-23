#include "pro_os.h"
#include "app_ui.h"
#include <string.h>
#include "serial.h"

void lab_init(void* state) { (void)state; }
void lab_update(struct nk_context* ctx, void* state) {
    (void)state;
    if (nk_begin(ctx, "Sovereign Lab", nk_rect(150, 150, 400, 300), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE|NK_WINDOW_CLOSABLE)) {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "Hardware Diagnostics", NK_TEXT_LEFT);
        if (nk_button_label(ctx, "Scan PCI Bus")) {
            pci_scan();
        }
        nk_label(ctx, "Storage Devices:", NK_TEXT_LEFT);
        for (int i = 0; i < hal_storage_get_device_count(); i++) {
            storage_device_t* dev = hal_storage_get_device(i);
            nk_label(ctx, dev->name, NK_TEXT_LEFT);
        }
    }
    nk_end(ctx);
}
