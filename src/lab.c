#include "pro_os.h"
#include "app_ui.h"
#include <string.h>
#include "serial.h"

void lab_init(void* s) { (void)s; }
void lab_update(struct nk_context* ctx, void* s) {
    (void)s;
    if (nk_begin(ctx, "Hardware Lab", nk_rect(150, 150, 400, 300), NK_WINDOW_BORDER|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "Stage 3/4 Diagnostics", NK_TEXT_LEFT);
        if (nk_button_label(ctx, "Log PCI Tree")) {
            pci_scan();
        }
    }
    nk_end(ctx);
}
