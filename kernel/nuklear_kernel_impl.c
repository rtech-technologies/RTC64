#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#include "nuklear.h"

/* Stub missing math/string functions for the kernel environment */
double pow(double x, double y) { (void)x; (void)y; return 0; }
double fmod(double x, double y) { (void)x; (void)y; return 0; }
double sqrt(double x) {
    if (x <= 0) return 0;
    double res = x;
    for(int i=0; i<10; i++) res = 0.5 * (res + x/res);
    return res;
}
double floor(double x) {
    int i = (int)x;
    return (double)(x < i ? i - 1 : i);
}
double ceil(double x) {
    int i = (int)x;
    return (double)(x > i ? i + 1 : i);
}
double cos(double x) { (void)x; return 0; }
double acos(double x) { (void)x; return 0; }
double fabs(double x) { return x < 0 ? -x : x; }

void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)) {
    (void)base; (void)nmemb; (void)size; (void)compar;
}
size_t strlen(const char* s) {
    size_t i = 0;
    while(s[i]) i++;
    return i;
}
void* memcpy(void* d, const void* s, size_t n) {
    for(size_t i=0; i<n; i++) ((char*)d)[i] = ((const char*)s)[i];
    return d;
}
void* memset(void* s, int c, size_t n) {
    for(size_t i=0; i<n; i++) ((char*)s)[i] = (char)c;
    return s;
}
void __assert_fail(const char *a, const char *b, unsigned int c, const char *d) {
    (void)a; (void)b; (void)c; (void)d;
    for(;;);
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    // Very basic skeleton vsnprintf
    (void)ap;
    size_t i = 0;
    while (format[i] && i < size - 1) {
        str[i] = format[i];
        i++;
    }
    str[i] = '\0';
    return (int)i;
}

// Simple allocator for kernel Nuklear
static uint8_t kernel_heap[128 * 1024];
static size_t heap_ptr = 0;

void* malloc(size_t n) {
    if (heap_ptr + n > sizeof(kernel_heap)) return NULL;
    void* p = &kernel_heap[heap_ptr];
    heap_ptr += n;
    return p;
}

void free(void* p) { (void)p; }

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++; s2++; n--;
    }
    if (n == 0) return 0;
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

char *strchr(const char *s, int c) {
    while (*s != (char)c) {
        if (!*s) return NULL;
        s++;
    }
    return (char *)s;
}

long strtol(const char *nptr, char **endptr, int base) {
    (void)nptr; (void)endptr; (void)base;
    return 0; // Stub
}
