/* Modified by Sovereign: Meaty RTC Driver for CMOS clock access */
#include "hal.h"

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

static uint8_t get_rtc_register(int reg) {
    outb(CMOS_ADDR, reg);
    return inb(CMOS_DATA);
}

static int bcd_to_bin(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

void rtc_get_time(int *h, int *m, int *s) {
    /* Wait for RTC update to finish */
    while (get_rtc_register(0x0A) & 0x80);

    uint8_t sec = get_rtc_register(0x00);
    uint8_t min = get_rtc_register(0x02);
    uint8_t hour = get_rtc_register(0x04);
    uint8_t status_b = get_rtc_register(0x0B);

    /* Convert BCD if necessary */
    if (!(status_b & 0x04)) {
        sec = bcd_to_bin(sec);
        min = bcd_to_bin(min);
        hour = ((hour & 0x0F) + (((hour & 0x70) / 16) * 10)) | (hour & 0x80);
    }

    /* Convert to 24h if necessary */
    if (!(status_b & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    if (h) *h = hour;
    if (m) *m = min;
    if (s) *s = sec;
}
