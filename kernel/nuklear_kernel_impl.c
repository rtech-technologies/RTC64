/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include "serial.h"
#include "pro_os.h"
#include "fatfs/ff.h"
#include "stdio.h"
#include "string.h"

static FILE _file_pool[8];

void* memset(void* s, int c, size_t n) {
    uint8_t* p = s;
    if (n >= 64 && ((uintptr_t)p & 15) == 0) {
        __asm__ volatile (
            "movd %1, %%xmm0\n\t"
            "punpcklbw %%xmm0, %%xmm0\n\t"
            "punpcklwd %%xmm0, %%xmm0\n\t"
            "pshufd $0, %%xmm0, %%xmm0\n\t"
            "1:\n\t"
            "movdqa %%xmm0, (%0)\n\t"
            "movdqa %%xmm0, 16(%0)\n\t"
            "movdqa %%xmm0, 32(%0)\n\t"
            "movdqa %%xmm0, 48(%0)\n\t"
            "add $64, %0\n\t"
            "sub $64, %2\n\t"
            "cmp $64, %2\n\t"
            "jae 1b"
            : "+r"(p) : "r"((int)c), "r"(n) : "memory", "xmm0"
        );
        n %= 64;
    }
    while(n--) *p++ = (unsigned char)c;
    return s;
}

void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = dest;
    const uint8_t* s = src;
    if (n >= 64 && ((uintptr_t)d & 15) == 0 && ((uintptr_t)s & 15) == 0) {
        __asm__ volatile (
            "1:\n\t"
            "movdqa (%1), %%xmm0\n\t"
            "movdqa 16(%1), %%xmm1\n\t"
            "movdqa 32(%1), %%xmm2\n\t"
            "movdqa 48(%1), %%xmm3\n\t"
            "movdqa %%xmm0, (%0)\n\t"
            "movdqa %%xmm1, 16(%0)\n\t"
            "movdqa %%xmm2, 32(%0)\n\t"
            "movdqa %%xmm3, 48(%0)\n\t"
            "add $64, %0\n\t"
            "add $64, %1\n\t"
            "sub $64, %2\n\t"
            "cmp $64, %2\n\t"
            "jae 1b"
            : "+r"(d), "+r"(s), "+r"(n) :: "memory", "xmm0", "xmm1", "xmm2", "xmm3"
        );
    }
    while(n--) *d++ = *s++;
    return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    if (d < s) { return memcpy(dest, src, n); }
    else { d += n; s += n; while (n--) *--d = *--s; }
    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char *p1 = s1, *p2 = s2;
    while(n--) { if (*p1 != *p2) return *p1 - *p2; p1++; p2++; }
    return 0;
}

size_t strlen(const char* s) {
    size_t len = 0; while(*s++) len++;
    return len;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest; while((*d++ = *src++));
    return dest;
}

char* strcat(char* dest, const char* src) {
    char* d = dest; while (*d) d++;
    while((*d++ = *src++));
    return dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) dest[i] = src[i];
    for (; i < n; i++) dest[i] = '\0';
    return dest;
}

int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while(n--) { if(*s1 != *s2) return *(unsigned char*)s1 - *(unsigned char*)s2; if(*s1 == 0) break; s1++; s2++; }
    return 0;
}

char* strchr(const char* s, int c) {
    while(*s) { if (*s == (char)c) return (char*)s; s++; }
    if (c == 0) return (char*)s;
    return NULL;
}

long strtol(const char* nptr, char** endptr, int base) {
    const char *s = nptr; unsigned long acc; int c; unsigned long cutoff; int neg = 0, any, cutlim;
    do { c = *s++; } while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v');
    if (c == '-') { neg = 1; c = *s++; } else if (c == '+') c = *s++;
    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) { c = s[1]; s += 2; base = 16; }
    if (base == 0) base = c == '0' ? 8 : 10;
    cutoff = neg ? -(unsigned long)0x8000000000000000 : 0x7FFFFFFFFFFFFFFF;
    cutlim = (int)(cutoff % (unsigned long)base); cutoff /= (unsigned long)base;
    for (acc = 0, any = 0;; c = *s++) {
        if (c >= '0' && c <= '9') c -= '0'; else if (c >= 'A' && c <= 'Z') c -= 'A' - 10; else if (c >= 'a' && c <= 'z') c -= 'a' - 10; else break;
        if (c >= base) break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) any = -1;
        else { any = 1; acc *= (unsigned long)base; acc += (unsigned long)c; }
    }
    if (any < 0) acc = neg ? 0x8000000000000000 : 0x7FFFFFFFFFFFFFFF; else if (neg) acc = -acc;
    if (endptr != 0) *endptr = (char *)(any ? s - 1 : nptr);
    return (long)acc;
}

static void reverse(char* s) {
    int i, j; for (i = 0, j = (int)strlen(s)-1; i<j; i++, j--) { char c = s[i]; s[i] = s[j]; s[j] = c; }
}

void itoa_meaty(unsigned long long n, char* s, int base, bool neg, int width, char pad) {
    int i = 0; const char *digits = "0123456789abcdef";
    do { s[i++] = digits[n % (unsigned long long)base]; } while ((n /= (unsigned long long)base) > 0);
    if (neg) s[i++] = '-';
    while (i < width) s[i++] = pad;
    s[i] = '\0'; reverse(s);
}

int vsnprintf(char* str, size_t size, const char* format, va_list ap) {
    size_t i = 0;
    while (*format && i < size - 1) {
        if (*format == '%') {
            format++;
            char pad = ' ';
            int width = 0;
            if (*format == '0') { pad = '0'; format++; }
            while (*format >= '0' && *format <= '9') {
                width = width * 10 + (*format - '0');
                format++;
            }
            int long_level = 0;
            while (*format == 'l') { long_level++; format++; }
            if (*format == 's') {
                const char* s = va_arg(ap, const char*);
                if (!s) s = "(null)";
                while (*s && i < size - 1) str[i++] = *s++;
            } else if (*format == 'd' || *format == 'i') {
                long long d = (long_level >= 2) ? va_arg(ap, long long) : (long_level == 1) ? va_arg(ap, long) : va_arg(ap, int);
                char buf[64]; bool neg = d < 0;
                itoa_meaty(neg ? (unsigned long long)-d : (unsigned long long)d, buf, 10, neg, width, pad);
                const char* s = buf; while (*s && i < size - 1) str[i++] = *s++;
            } else if (*format == 'u') {
                unsigned long long u = (long_level >= 2) ? va_arg(ap, unsigned long long) : (long_level == 1) ? va_arg(ap, unsigned long) : va_arg(ap, unsigned int);
                char buf[64]; itoa_meaty(u, buf, 10, false, width, pad);
                const char* s = buf; while (*s && i < size - 1) str[i++] = *s++;
            } else if (*format == 'x' || *format == 'p' || *format == 'X') {
                unsigned long long x;
                char spec = *format;
                if (spec == 'p') { x = (uintptr_t)va_arg(ap, void*); if (width == 0) width = 16; if (pad == ' ') pad = '0'; }
                else { x = (long_level >= 2) ? va_arg(ap, unsigned long long) : (long_level == 1) ? va_arg(ap, unsigned long) : (unsigned long long)va_arg(ap, unsigned int); }
                char buf[64]; itoa_meaty(x, buf, 16, false, width, pad);
                const char* s = buf; while (*s && i < size - 1) {
                    char c = *s++; if (spec == 'X' && c >= 'a' && c <= 'z') c -= 32; str[i++] = c;
                }
            } else if (*format == '%') { str[i++] = '%'; }
            else { /* Skip unknown */ }
        } else { str[i++] = *format; }
        format++;
    }
    str[i] = '\0';
    return (int)i;
}

int snprintf(char* str, size_t size, const char* format, ...) {
    va_list ap; va_start(ap, format); int ret = vsnprintf(str, size, format, ap); va_end(ap);
    return ret;
}

int printf(const char* format, ...) {
    va_list ap; va_start(ap, format);
    char buf[512];
    int ret = vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    serial_printf("%s", buf);
    return ret;
}

int abs(int n) { return n < 0 ? -n : n; }

void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function) {
    (void)assertion; (void)file; (void)line; (void)function;
    serial_printf("ASSERTION FAILED: %s at %s:%d\n", assertion, file, line);
    kpanic("ASSERTION FAILURE");
}

void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)) {
    if (nmemb < 2) return;
    char *pivot = (char *)base + (nmemb / 2) * size;
    char *i = (char *)base;
    char *j = (char *)base + (nmemb - 1) * size;
    while (i <= j) {
        while (compar(i, pivot) < 0) i += size;
        while (compar(j, pivot) > 0) j -= size;
        if (i <= j) {
            char tmp[size];
            memcpy(tmp, i, size);
            memcpy(i, j, size);
            memcpy(j, tmp, size);
            if (pivot == i) pivot = j;
            else if (pivot == j) pivot = i;
            i += size;
            j -= size;
        }
    }
    if ((uintptr_t)j > (uintptr_t)base) qsort(base, ((uintptr_t)j - (uintptr_t)base) / size + 1, size, compar);
    if ((uintptr_t)i < (uintptr_t)base + nmemb * size) qsort(i, nmemb - ((uintptr_t)i - (uintptr_t)base) / size, size, compar);
}

extern const char* vfs_resolve(const char* path);

FILE* fopen(const char* filename, const char* mode) {
    const char* resolved = vfs_resolve(filename);
    BYTE flags = 0;
    if (strchr(mode, 'r')) flags |= FA_READ;
    if (strchr(mode, 'w')) flags |= (FA_WRITE | FA_CREATE_ALWAYS);
    if (strchr(mode, 'a')) flags |= (FA_WRITE | FA_OPEN_APPEND);

    for (int i = 0; i < 8; i++) {
        if (!_file_pool[i].is_open) {
            if (f_open(&_file_pool[i].fil, resolved, flags) == FR_OK) {
                _file_pool[i].is_open = 1;
                return &_file_pool[i];
            }
            return NULL;
        }
    }
    return NULL;
}

int fclose(FILE* stream) {
    if (stream) {
        f_close(&stream->fil);
        stream->is_open = 0;
    }
    return 0;
}

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    if (!stream) return 0;
    UINT br;
    if (f_read(&stream->fil, ptr, (UINT)(size * nmemb), &br) == FR_OK) {
        return (size_t)(br / size);
    }
    return 0;
}

int fseek(FILE* stream, long offset, int whence) {
    if (!stream) return -1;
    FSIZE_t target = 0;
    if (whence == SEEK_SET) target = (FSIZE_t)offset;
    else if (whence == SEEK_CUR) target = f_tell(&stream->fil) + (FSIZE_t)offset;
    else if (whence == SEEK_END) target = f_size(&stream->fil) + (FSIZE_t)offset;
    return (f_lseek(&stream->fil, target) == FR_OK) ? 0 : -1;
}

long ftell(FILE* stream) {
    if (!stream) return -1;
    return (long)f_tell(&stream->fil);
}
