#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Math functions are implemented in math.c */
extern double pow(double x, double y);
extern double sqrt(double x);
extern double sin(double x);
extern double cos(double x);
extern double fabs(double x);

/* Core memory implementations for Nuklear and CherryUSB */
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

void* memchr(const void* s, int c, size_t n) {
    const unsigned char* p = s;
    while (n--) {
        if (*p == (unsigned char)c) return (void*)p;
        p++;
    }
    return NULL;
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
    (void)nptr; (void)endptr; (void)base;
    return 0;
}

#include <stdarg.h>

static void reverse(char* s) {
    int i, j;
    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        char c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

static void itoa(int n, char* s) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0) s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

static void utoa(uint64_t n, char* s) {
    int i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    s[i] = '\0';
    reverse(s);
}

static void xtoa(uint64_t n, char* s, int caps) {
    int i = 0;
    const char* digits = caps ? "0123456789ABCDEF" : "0123456789abcdef";
    do {
        s[i++] = digits[n % 16];
    } while ((n /= 16) > 0);
    s[i] = '\0';
    reverse(s);
}

int vsnprintf(char* str, size_t size, const char* format, va_list ap) {
    size_t i = 0;
    while (*format && i < size - 1) {
        if (*format == '%') {
            format++;
            int width = 0;
            int zero_pad = 0;
            if (*format == '0') { zero_pad = 1; format++; }
            while (*format >= '0' && *format <= '9') {
                width = width * 10 + (*format - '0');
                format++;
            }

            if (*format == '%') {
                str[i++] = '%';
            } else if (*format == 's') {
                const char* s = va_arg(ap, const char*);
                while (*s && i < size - 1) str[i++] = *s++;
            } else if (*format == 'c') {
                char c = (char)va_arg(ap, int);
                str[i++] = c;
            } else if (*format == 'd' || *format == 'u' || *format == 'x' || *format == 'p' || *format == 'l') {
                uint64_t val;
                char buf[64];
                int is_x = (*format == 'x' || *format == 'p');

                if (*format == 'l') {
                    format++;
                    if (*format == 'l') format++;
                    val = va_arg(ap, uint64_t);
                    if (*format == 'x') is_x = 1;
                } else if (*format == 'p') {
                    val = va_arg(ap, uintptr_t);
                    is_x = 1;
                } else if (*format == 'd') {
                    val = (uint64_t)va_arg(ap, int);
                } else {
                    val = (uint64_t)va_arg(ap, unsigned int);
                }

                if (is_x) xtoa(val, buf, 0);
                else utoa(val, buf);

                int len = strlen(buf);
                while (width > len && i < size - 1) {
                    str[i++] = zero_pad ? '0' : ' ';
                    width--;
                }
                const char* s = buf;
                while (*s && i < size - 1) str[i++] = *s++;
            } else {
                str[i++] = *format;
            }
        } else {
            str[i++] = *format;
        }
        if (*format) format++;
    }
    str[i] = '\0';
    return (int)i;
}

int snprintf(char* str, size_t size, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vsnprintf(str, size, format, ap);
    va_end(ap);
    return ret;
}

#define NK_IMPLEMENTATION
#include "pro_os.h"

void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function) { (void)assertion; (void)file; (void)line; (void)function; }
