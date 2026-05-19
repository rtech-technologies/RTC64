#include <pro_os.h>
#define STB_SPRINTF_IMPLEMENTATION
#include <external/stb_sprintf.h>
#include <external/TLSF/tlsf.h>

#define KERNEL_HEAP_SIZE (64 * 1024 * 1024)
static uint8_t kernel_heap[KERNEL_HEAP_SIZE] __attribute__((aligned(8)));
static tlsf_t kernel_pool = NULL;

void libc_init(void) {
    if (!kernel_pool) {
        kernel_pool = tlsf_create_with_pool(kernel_heap, KERNEL_HEAP_SIZE);
    }
}

void* memset(void* s, int c, size_t n) {
    uint8_t* p = (uint8_t*)s;
    while(n--) *p++ = (uint8_t)c;
    return s;
}

void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    while(n--) *d++ = *s++;
    return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
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
    const uint8_t* p1 = (const uint8_t*)s1;
    const uint8_t* p2 = (const uint8_t*)s2;
    while(n--) {
        if (*p1 != *p2) return (int)*p1 - (int)*p2;
        p1++; p2++;
    }
    return 0;
}

size_t strlen(const char* s) {
    size_t len = 0;
    while(s[len]) len++;
    return len;
}

int tolower(int c) {
    if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
    return c;
}

int atoi(const char* s) {
    int res = 0;
    while (*s >= '0' && *s <= '9') {
        res = res * 10 + (*s - '0');
        s++;
    }
    return res;
}

char* strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    for (; *haystack; haystack++) {
        if (*haystack == *needle) {
            const char *h = haystack, *n = needle;
            while (*h && *n && *h == *n) {
                h++; n++;
            }
            if (!*n) return (char*)haystack;
        }
    }
    return NULL;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++; s2++; n--;
    }
    if (n == 0) return 0;
    return (unsigned char)*s1 - (unsigned char)*s2;
}

char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) dest[i] = src[i];
    for ( ; i < n; i++) dest[i] = '\0';
    return dest;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

char* strchr(const char* s, int c) {
    while(*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return NULL;
}

void* malloc(size_t size) {
    if (!kernel_pool) libc_init();
    return tlsf_malloc(kernel_pool, size);
}

void* aligned_alloc(size_t alignment, size_t size) {
    if (!kernel_pool) libc_init();
    return tlsf_memalign(kernel_pool, alignment, size);
}

void free(void* ptr) {
    if (kernel_pool && ptr) tlsf_free(kernel_pool, ptr);
}

void* realloc(void* ptr, size_t size) {
    if (!kernel_pool) libc_init();
    return tlsf_realloc(kernel_pool, ptr, size);
}

void* calloc(size_t nmemb, size_t size) {
    void* ptr = malloc(nmemb * size);
    if (ptr) memset(ptr, 0, nmemb * size);
    return ptr;
}

void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function) {
    (void)assertion; (void)file; (void)line; (void)function;
    extern void quartermaster_panic(const char* msg);
    quartermaster_panic("Assertion Failed");
}

static char* printf_cb(const char* buf, void* user, int len) {
    (void)user;
    for (int i = 0; i < len; ++i) {
        vga_putc(buf[i]);
    }
    return (char*)buf;
}

int printf(const char* fmt, ...) {
    char buf[STB_SPRINTF_MIN];
    va_list va;
    va_start(va, fmt);
    int ret = stbsp_vsprintfcb(printf_cb, buf, buf, fmt, va);
    va_end(va);
    return ret;
}

int snprintf(char* buf, size_t n, const char* fmt, ...) {
    va_list va;
    va_start(va, fmt);
    int ret = stbsp_vsnprintf(buf, (int)n, fmt, va);
    va_end(va);
    return ret;
}

// POSIX Stubs for wolfSSL (Internal OS implementations)
int fcntl(int fd, int cmd, ...) { (void)fd; (void)cmd; return -1; }
int open(const char* path, int flags, ...) { (void)path; (void)flags; return -1; }
int socket(int domain, int type, int protocol) { (void)domain; (void)type; (void)protocol; return -1; }
int accept4(int sockfd, void* addr, void* addrlen, int flags) { (void)sockfd; (void)addr; (void)addrlen; (void)flags; return -1; }
int accept(int sockfd, void* addr, void* addrlen) { (void)sockfd; (void)addr; (void)addrlen; return -1; }
int* __errno_location(void) { static int e; return &e; }
