#include "pro_os.h"
#include "app_ui.h"
#include <string.h>
#include "serial.h"
#include "ff.h"

void installer_init(void* s) { (void)s; }
void installer_update(struct nk_context* ctx, void* s) {
    (void)s;
    if (nk_begin(ctx, "Sovereign LITE Installer", nk_rect(100, 100, 400, 400), NK_WINDOW_BORDER|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "Available hardware for installation:", NK_TEXT_LEFT);
        for (int i = 0; i < hal_storage_get_device_count(); i++) {
            storage_device_t* dev = hal_storage_get_device(i);
            if (nk_button_label(ctx, dev->name)) {
                serial_printf("[Installer] Probing drive %d...\n", i);
            }
        }
    }
    nk_end(ctx);
}
