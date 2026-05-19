#include <pro_os.h>

void quartermaster_panic(const char* msg) {
    // paint terminal Emerald Green (10, 0)
    // Emerald green is approx #50C878. RGB (80, 200, 120)
    // 10,0 usually refers to terminal color codes in some contexts, but here we paint the screen.
    // I'll use a solid emerald color.

    extern struct limine_framebuffer *fb;
    if (fb && fb->address) {
        uint32_t emerald = 0x50C878;
        uint32_t* screen = (uint32_t*)fb->address;
        for (uint64_t i = 0; i < fb->width * fb->height; i++) {
            screen[i] = emerald;
        }
    }

    printf("\n!!! KERNEL PANIC: %s !!!\n", msg);

    // Dump x86_64 CPU registers
    // In a real implementation we would capture state from an interrupt frame.
    // Here we just print some placeholders or use inline asm if possible.
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp, r8, r9, r10, r11, r12, r13, r14, r15;
    __asm__ volatile ("mov %%rax, %0" : "=r"(rax));
    __asm__ volatile ("mov %%rbx, %0" : "=r"(rbx));
    __asm__ volatile ("mov %%rcx, %0" : "=r"(rcx));
    __asm__ volatile ("mov %%rdx, %0" : "=r"(rdx));
    __asm__ volatile ("mov %%rsi, %0" : "=r"(rsi));
    __asm__ volatile ("mov %%rdi, %0" : "=r"(rdi));
    __asm__ volatile ("mov %%rbp, %0" : "=r"(rbp));
    __asm__ volatile ("mov %%r8, %0" : "=r"(r8));
    __asm__ volatile ("mov %%r9, %0" : "=r"(r9));
    __asm__ volatile ("mov %%r10, %0" : "=r"(r10));
    __asm__ volatile ("mov %%r11, %0" : "=r"(r11));
    __asm__ volatile ("mov %%r12, %0" : "=r"(r12));
    __asm__ volatile ("mov %%r13, %0" : "=r"(r13));
    __asm__ volatile ("mov %%r14, %0" : "=r"(r14));
    __asm__ volatile ("mov %%r15, %0" : "=r"(r15));

    printf("RAX=%016lx RBX=%016lx RCX=%016lx RDX=%016lx\n", rax, rbx, rcx, rdx);
    printf("RSI=%016lx RDI=%016lx RBP=%016lx\n", rsi, rdi, rbp);
    printf("R8 =%016lx R9 =%016lx R10=%016lx R11=%016lx\n", r8, r9, r10, r11);
    printf("R12=%016lx R13=%016lx R14=%016lx R15=%016lx\n", r12, r13, r14, r15);

    // XHCI state scan
    // These require MMIO base. This is a skeletal representation.
    printf("XHCI State Lines Scan: [OFFLINE]\n");

    for (;;) __asm__("hlt");
}
