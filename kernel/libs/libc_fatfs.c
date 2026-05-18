#include <stdint.h>
#include <stddef.h>
#include <pro_os.h>

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

int memcmp(const void* s1, const void* s2, size_t n) {
    const uint8_t* p1 = (const uint8_t*)s1;
    const uint8_t* p2 = (const uint8_t*)s2;
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

char* strchr(const char* s, int c) {
    while(*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return NULL;
}

void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function) {
    (void)assertion; (void)file; (void)line; (void)function;
    extern void quartermaster_panic(const char* msg);
    quartermaster_panic("Assertion Failed");
}

void* malloc(size_t size) {
    extern void* bump_alloc(size_t size);
    return bump_alloc(size);
}

void free(void* ptr) {
    (void)ptr;
}

// FatFs stubs
#include <diskio.h>

DSTATUS disk_status (BYTE pdrv) { (void)pdrv; return 0; }
DSTATUS disk_initialize (BYTE pdrv) { (void)pdrv; return 0; }
DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) { (void)pdrv; (void)buff; (void)sector; (void)count; return RES_OK; }
DRESULT disk_write (BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) { (void)pdrv; (void)buff; (void)sector; (void)count; return RES_OK; }
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff) { (void)pdrv; (void)cmd; (void)buff; return RES_OK; }
uint32_t get_fattime (void) { return 0; }
