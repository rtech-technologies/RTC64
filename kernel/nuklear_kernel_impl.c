#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

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

static void reverse(char* s) { int i, j; for (i = 0, j = strlen(s)-1; i<j; i++, j--) { char c = s[i]; s[i] = s[j]; s[j] = c; } }
static void utoa(uint64_t n, char* s) { int i = 0; do { s[i++] = n % 10 + '0'; } while ((n /= 10) > 0); s[i] = '\0'; reverse(s); }
static void itoa(int64_t n, char* s) {
    int i = 0; uint64_t u;
    if (n < 0) { s[i++] = '-'; u = (uint64_t)-(n + 1) + 1; }
    else u = (uint64_t)n;
    int start = i;
    do { s[i++] = u % 10 + '0'; } while ((u /= 10) > 0);
    s[i] = '\0';
    // Reverse only the digits
    for (int j = start, k = i - 1; j < k; j++, k--) { char c = s[j]; s[j] = s[k]; s[k] = c; }
}
static void xtoa(uint64_t n, char* s, int c) { int i = 0; const char* d = c ? "0123456789ABCDEF" : "0123456789abcdef"; do { s[i++] = d[n % 16]; } while ((n /= 16) > 0); s[i] = '\0'; reverse(s); }

int vsnprintf(char* str, size_t size, const char* format, va_list ap) {
    size_t i = 0;
    while (*format && i < size - 1) {
        if (*format == '%') {
            format++;
            int width = 0, zero = 0;
            if (*format == '0') { zero = 1; format++; }
            while (*format >= '0' && *format <= '9') { width = width * 10 + (*format - '0'); format++; }
            if (*format == 's') {
                const char* s = va_arg(ap, const char*); if (!s) s = "(null)";
                while (*s && i < size - 1) str[i++] = *s++;
            } else if (*format == 'd' || *format == 'u' || *format == 'x' || *format == 'p' || *format == 'l') {
                uint64_t val; char buf[64]; int is_x = (*format == 'x' || *format == 'p');
                int is_d = (*format == 'd');
                if (*format == 'l') { format++; if (*format == 'l') format++; val = va_arg(ap, uint64_t); if (*format == 'x') is_x = 1; }
                else if (*format == 'p') { val = (uintptr_t)va_arg(ap, void*); is_x = 1; }
                else if (*format == 'd') val = (uint64_t)va_arg(ap, int);
                else val = (uint64_t)va_arg(ap, unsigned int);
                if (is_x) xtoa(val, buf, 0); else if (is_d) itoa((int64_t)val, buf); else utoa(val, buf);
                int len = strlen(buf);
                while (width > len && i < size - 1) { str[i++] = zero ? '0' : ' '; width--; }
                const char* b = buf; while (*b && i < size - 1) str[i++] = *b++;
            } else str[i++] = *format;
        } else str[i++] = *format;
        if (*format) format++;
    }
    str[i] = '\0'; return (int)i;
}
int snprintf(char* str, size_t size, const char* format, ...) { va_list ap; va_start(ap, format); int ret = vsnprintf(str, size, format, ap); va_end(ap); return ret; }

extern double sqrt(double x); extern double pow(double x, double y); extern double sin(double x); extern double cos(double x); extern double fabs(double x);
#define NK_IMPLEMENTATION
#include "pro_os.h"
void __assert_fail(const char* a, const char* f, unsigned int l, const char* fn) { (void)a; (void)f; (void)l; (void)fn; }
