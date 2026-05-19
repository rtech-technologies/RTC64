#include "pro_os.h"
#include "hal.h"
#include "ff.h"
#include "lab.h"
#include <string.h>
#include <stdio.h>

void lab_init(struct lab_state *state) {
    memset(state, 0, sizeof(struct lab_state));
    strcpy(state->test_status, "Ready for Sovereign testing.");
    strcpy(state->file_path, "0:test.txt");
    strcpy(state->file_content, "Hello from RTC64 Sovereign Lab!");
}

static void lab_test_storage(struct lab_state *state) {
    FIL fp;
    FRESULT res;
    UINT bw, br;
    char verify[64];

    res = f_open(&fp, state->file_path, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    if (res != FR_OK) {
        snprintf(state->test_status, 128, "Err: f_open failed (%d)", res);
        return;
    }

    f_write(&fp, state->file_content, strlen(state->file_content), &bw);
    f_lseek(&fp, 0);
    f_read(&fp, verify, bw, &br);
    verify[br] = 0;

    if (strcmp(verify, state->file_content) == 0) {
        snprintf(state->test_status, 128, "Success: Wrote %u bytes, Verified OK!", bw);
    } else {
        snprintf(state->test_status, 128, "Err: Verification mismatch!");
    }

    f_close(&fp);
}

void lab_ui_render(struct nk_context *ctx, struct lab_state *state) {
    if (!state->active) return;

    if (nk_begin(ctx, "Sovereign Lab", nk_rect(50, 50, 600, 450),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE))
    {
        nk_layout_row_static(ctx, 30, 120, 4);
        if (nk_button_label(ctx, "STORAGE")) state->current_tab = 0;
        if (nk_button_label(ctx, "INPUT")) state->current_tab = 1;
        if (nk_button_label(ctx, "PCI")) state->current_tab = 2;
        if (nk_button_label(ctx, "INFO")) state->current_tab = 3;

        nk_layout_row_dynamic(ctx, 320, 1);
        if (nk_group_begin(ctx, "Content", NK_WINDOW_NO_SCROLLBAR)) {
            if (state->current_tab == 0) {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "FatFs & Storage Integration Test", NK_TEXT_LEFT);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, state->file_path, sizeof(state->file_path), nk_filter_default);
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, state->file_content, sizeof(state->file_content), nk_filter_default);

                if (nk_button_label(ctx, "RUN R/W TEST")) {
                    lab_test_storage(state);
                }
                nk_label(ctx, state->test_status, NK_TEXT_LEFT);
            } else if (state->current_tab == 1) {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "Real-time Input Monitoring", NK_TEXT_LEFT);
                char buf[64];
                snprintf(buf, 64, "Mouse X: %d, Y: %d", state->last_x, state->last_y);
                nk_label(ctx, buf, NK_TEXT_LEFT);
                snprintf(buf, 64, "Last HID Key: 0x%x", state->last_key);
                nk_label(ctx, buf, NK_TEXT_LEFT);
            } else if (state->current_tab == 2) {
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Sovereign PCI Map", NK_TEXT_LEFT);
                char buf[64];
                snprintf(buf, 64, "xHCI MMIO: 0x%lx", (unsigned long)xhci_mmio_base); nk_label(ctx, buf, NK_TEXT_LEFT);
                snprintf(buf, 64, "EHCI MMIO: 0x%lx", (unsigned long)ehci_mmio_base); nk_label(ctx, buf, NK_TEXT_LEFT);
                snprintf(buf, 64, "NVMe MMIO: 0x%lx", (unsigned long)nvme_mmio_base); nk_label(ctx, buf, NK_TEXT_LEFT);
                snprintf(buf, 64, "AHCI MMIO: 0x%lx", (unsigned long)ahci_mmio_base); nk_label(ctx, buf, NK_TEXT_LEFT);
            } else {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, "RTC64 Sovereign Kernel v1.0", NK_TEXT_CENTERED);
                nk_label(ctx, "Build: " __DATE__ " " __TIME__, NK_TEXT_CENTERED);
                nk_label(ctx, "Modules: CherryUSB, FatFs, TLSF, TGX", NK_TEXT_CENTERED);
            }
            nk_group_end(ctx);
        }
    }
    if (nk_window_is_closed(ctx, "Sovereign Lab")) state->active = 0;
    nk_end(ctx);
}
