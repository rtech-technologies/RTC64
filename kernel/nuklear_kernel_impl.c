/* Modified by Sovereign: Robust libc-style implementations for freestanding environment */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

void* memset(void* s, int c, size_t n) {
    unsigned char* p = s;
    while(n--) *p++ = (unsigned char)c;
    return s;
}

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    while(n--) *d++ = *s++;
    return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char *p1 = s1, *p2 = s2;
    while(n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++; p2++;
    }
    return 0;
}

size_t strlen(const char* s) {
    size_t len = 0;
    while(*s++) len++;
    return len;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while((*d++ = *src++));
    return dest;
}

int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while(n--) {
        if(*s1 != *s2) return *(unsigned char*)s1 - *(unsigned char*)s2;
        if(*s1 == 0) break;
        s1++; s2++;
    }
    return 0;
}

char* strchr(const char* s, int c) {
    while(*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    if (c == 0) return (char*)s;
    return NULL;
}

long strtol(const char* nptr, char** endptr, int base) {
    const char *s = nptr;
    unsigned long acc;
    int c;
    unsigned long cutoff;
    int neg = 0, any, cutlim;

    do { c = *s++; } while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v');
    if (c == '-') { neg = 1; c = *s++; } else if (c == '+') c = *s++;
    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1]; s += 2; base = 16;
    }
    if (base == 0) base = c == '0' ? 8 : 10;

    cutoff = neg ? -(unsigned long)0x8000000000000000 : 0x7FFFFFFFFFFFFFFF;
    cutlim = cutoff % (unsigned long)base;
    cutoff /= (unsigned long)base;
    for (acc = 0, any = 0;; c = *s++) {
        if (c >= '0' && c <= '9') c -= '0';
        else if (c >= 'A' && c <= 'Z') c -= 'A' - 10;
        else if (c >= 'a' && c <= 'z') c -= 'a' - 10;
        else break;
        if (c >= base) break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) any = -1;
        else { any = 1; acc *= base; acc += c; }
    }
    if (any < 0) acc = neg ? 0x8000000000000000 : 0x7FFFFFFFFFFFFFFF;
    else if (neg) acc = -acc;
    if (endptr != 0) *endptr = (char *)(any ? s - 1 : nptr);
    return acc;
}

static void reverse(char* s) {
    int i, j;
    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        char c = s[i]; s[i] = s[j]; s[j] = c;
    }
}

static void itoa_meaty(long n, char* s, int base) {
    unsigned long num;
    int i = 0, sign = 0;
    if (base == 10 && n < 0) { sign = 1; num = (unsigned long)-n; } else { num = (unsigned long)n; }
    const char *digits = "0123456789abcdef";
    do { s[i++] = digits[num % base]; } while ((num /= base) > 0);
    if (sign) s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

int vsnprintf(char* str, size_t size, const char* format, va_list ap) {
    size_t i = 0;
    while (*format && i < size - 1) {
        if (*format == '%') {
            format++;
            bool long_mode = false;
            if (*format == 'l') { long_mode = true; format++; }

            if (*format == 's') {
                const char* s = va_arg(ap, const char*);
                if (!s) s = "(null)";
                while (*s && i < size - 1) str[i++] = *s++;
            } else if (*format == 'd' || *format == 'i') {
                long d = long_mode ? va_arg(ap, long) : va_arg(ap, int);
                char buf[64]; itoa_meaty(d, buf, 10);
                const char* s = buf;
                while (*s && i < size - 1) str[i++] = *s++;
            } else if (*format == 'u') {
                unsigned long u = long_mode ? va_arg(ap, unsigned long) : va_arg(ap, unsigned int);
                char buf[64]; itoa_meaty((long)u, buf, 10);
                const char* s = buf;
                while (*s && i < size - 1) str[i++] = *s++;
            } else if (*format == 'x' || *format == 'p') {
                unsigned long x = long_mode ? va_arg(ap, unsigned long) : (unsigned long)va_arg(ap, unsigned int);
                if (*format == 'p') x = (unsigned long)va_arg(ap, void*);
                char buf[64]; itoa_meaty((long)x, buf, 16);
                const char* s = buf;
                while (*s && i < size - 1) str[i++] = *s++;
            } else if (*format == '%') {
                str[i++] = '%';
            } else { str[i++] = *format; }
        } else {
            str[i++] = *format;
        }
        format++;
    }
    str[i] = '\0';
    return (int)i;
}

int snprintf(char* str, size_t size, const char* format, ...) {
    va_list ap; va_start(ap, format);
    int ret = vsnprintf(str, size, format, ap);
    va_end(ap);
    return ret;
}

#define NK_IMPLEMENTATION
#include "pro_os.h"

void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function) {
    (void)assertion; (void)file; (void)line; (void)function;
    while(1) { __asm__("hlt"); }
}
