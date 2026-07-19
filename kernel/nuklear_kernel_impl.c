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

char* strcat(char* dest, const char* src) {
    char* rd = dest;
    while (*rd) rd++;
    while ((*rd++ = *src++));
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

char* strrchr(const char* s, int c) {
    const char* last = NULL;
    while (*s) {
        if (*s == (char)c) last = s;
        s++;
    }
    if (c == 0) return (char*)s;
    return (char*)last;
}

char* strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    for (; *haystack; haystack++) {
        if (*haystack == *needle) {
            const char *h = haystack;
            const char *n = needle;
            while (*h && *n && *h == *n) {
                h++;
                n++;
            }
            if (!*n) return (char*)haystack;
        }
    }
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

int vsnprintf(char* str, size_t size, const char* format, va_list ap) {
    size_t i = 0;
    while (*format && i < size - 1) {
        if (*format == '%') {
            format++;
            if (*format == '%') {
                str[i++] = '%';
            } else if (*format == 's') {
                const char* s = va_arg(ap, const char*);
                while (*s && i < size - 1) str[i++] = *s++;
            } else if (*format == 'd') {
                int d = va_arg(ap, int);
                char buf[16];
                itoa(d, buf);
                const char* s = buf;
                while (*s && i < size - 1) str[i++] = *s++;
            } else {
                str[i++] = *format;
            }
        } else {
            str[i++] = *format;
        }
        format++;
    }
    str[i] = '\0';
    return i;
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
