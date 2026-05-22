#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

extern double sqrt(double x); extern double pow(double x, double y); extern double sin(double x); extern double cos(double x); extern double fabs(double x);
void* memset(void* s, int c, size_t n) { unsigned char* p = s; while(n--) *p++ = (unsigned char)c; return s; }
void* memcpy(void* d, const void* s, size_t n) { unsigned char* dest = d; const unsigned char* src = s; while(n--) *dest++ = *src++; return d; }
void* memmove(void* d, const void* s, size_t n) { unsigned char* dest = d; const unsigned char* src = s; if (dest < src) while (n--) *dest++ = *src++; else { dest += n; src += n; while (n--) *--dest = *--src; } return d; }
void* memchr(const void* s, int c, size_t n) { const unsigned char* p = s; while (n--) { if (*p == (unsigned char)c) return (void*)p; p++; } return NULL; }
int memcmp(const void* s1, const void* s2, size_t n) { const unsigned char *p1 = s1, *p2 = s2; while(n--) { if (*p1 != *p2) return *p1 - *p2; p1++; p2++; } return 0; }
size_t strlen(const char* s) { size_t l = 0; while(s && *s++) l++; return l; }
char* strcpy(char* d, const char* s) { char* dest = d; while((*dest++ = *s++)); return d; }
int strcmp(const char* s1, const char* s2) { while(*s1 && (*s1 == *s2)) { s1++; s2++; } return *(unsigned char*)s1 - *(unsigned char*)s2; }
int strncmp(const char* s1, const char* s2, size_t n) { while(n--) { if(*s1 != *s2) return *(unsigned char*)s1 - *(unsigned char*)s2; if(*s1 == 0) break; s1++; s2++; } return 0; }
char* strchr(const char* s, int c) { while(*s) { if (*s == (char)c) return (char*)s; s++; } return (c == 0) ? (char*)s : NULL; }
long strtol(const char* n, char** e, int b) { (void)n; (void)e; (void)b; return 0; }

static void rev(char* s) { int i, j; for (i = 0, j = strlen(s)-1; i<j; i++, j--) { char c = s[i]; s[i] = s[j]; s[j] = c; } }
static void utoa(uint64_t n, char* s) { int i = 0; do { s[i++] = n % 10 + '0'; } while ((n /= 10) > 0); s[i] = '\0'; rev(s); }
static void xtoa(uint64_t n, char* s) { int i = 0; do { s[i++] = "0123456789abcdef"[n % 16]; } while ((n /= 16) > 0); s[i] = '\0'; rev(s); }

int vsnprintf(char* str, size_t size, const char* format, va_list ap) {
    size_t i = 0;
    while (*format && i < size - 1) {
        if (*format == '%') {
            format++; int w = 0; bool zero = false;
            if (*format == '0') { zero = true; format++; }
            while (*format >= '0' && *format <= '9') { w = w * 10 + (*format - '0'); format++; }
            if (*format == 's') {
                const char* s = va_arg(ap, const char*); if (!s) s = "(null)";
                while (*s && i < size - 1) str[i++] = *s++;
            } else if (*format == 'd' || *format == 'u' || *format == 'x' || *format == 'p' || *format == 'l') {
                uint64_t v; char buf[64]; bool is_x = (*format == 'x' || *format == 'p');
                if (*format == 'l') { format++; if (*format == 'l') format++; v = va_arg(ap, uint64_t); if (*format == 'x') is_x = true; }
                else if (*format == 'p') { v = (uintptr_t)va_arg(ap, void*); is_x = true; }
                else if (*format == 'd') v = (uint64_t)va_arg(ap, int);
                else v = (uint64_t)va_arg(ap, unsigned int);
                if (is_x) xtoa(v, buf); else utoa(v, buf);
                int l = strlen(buf); while (w > l && i < size - 1) { str[i++] = zero ? '0' : ' '; w--; }
                const char* b = buf; while (*b && i < size - 1) str[i++] = *b++;
            } else str[i++] = *format;
        } else str[i++] = *format;
        if (*format) format++;
    }
    str[i] = '\0'; return (int)i;
}
int snprintf(char* str, size_t size, const char* format, ...) { va_list ap; va_start(ap, format); int ret = vsnprintf(str, size, format, ap); va_end(ap); return ret; }
#define NK_IMPLEMENTATION
#include "pro_os.h"
void __assert_fail(const char* a, const char* f, unsigned int l, const char* fn) { (void)a; (void)f; (void)l; (void)fn; }
