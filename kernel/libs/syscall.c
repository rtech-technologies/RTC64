#include <pro_os.h>

#define MSR_STAR   0xC0000081
#define MSR_LSTAR  0xC0000082
#define MSR_SFMASK 0xC0000084
#define MSR_GS_BASE 0xC0000101

static inline void wrmsr(uint32_t msr, uint64_t val) {
    uint32_t low = val & 0xFFFFFFFF;
    uint32_t high = val >> 32;
    __asm__ volatile ( "wrmsr" : : "c" (msr), "a" (low), "d" (high) );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

extern void syscall_entry(void);

typedef struct {
    uint64_t kernel_stack;
    uint64_t user_stack;
} cpu_local_t;

static cpu_local_t bsp_cpu_local;

void syscall_init() {
    uint64_t star = ((uint64_t)0x0008 << 32) | ((uint64_t)0x001B << 48);
    wrmsr(MSR_STAR, star);
    wrmsr(MSR_LSTAR, (uint64_t)syscall_entry);
    wrmsr(MSR_SFMASK, 0x200);

    // Initialize GS_BASE
    void* kstack = malloc(16384);
    bsp_cpu_local.kernel_stack = (uint64_t)kstack + 16384;
    wrmsr(MSR_GS_BASE, (uint64_t)&bsp_cpu_local);
}

// x86_64 Unified Input Polling
static char serial_getc() {
    if ((inb(0x3F8 + 5) & 1)) return inb(0x3F8);
    return 0;
}

void syscall_dispatcher(uint64_t id, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    (void)arg2; (void)arg3;
    switch (id) {
        case 1: { // PRINT
            extern void print(const char* msg);
            print((const char*)arg1);
        } break;
        case 2: { // INPUT (Blocking)
            char* buf = (char*)arg1;
            int i = 0;
            for(;;) {
                char c = serial_getc();
                if (c) {
                    if (c == '\r' || c == '\n') {
                        buf[i] = '\0';
                        extern void vga_putc(char c);
                        vga_putc('\n');
                        break;
                    }
                    buf[i++] = c;
                    extern void vga_putc(char c);
                    vga_putc(c);
                }
            }
        } break;
    }
}
