#include <stdint.h>
#include <stddef.h>
#include "serial.h"
struct panic_framebuffer { uint64_t address, width, height, pitch; };
struct cpu_state { uint64_t gs,fs,es,ds,cr4,cr3,cr2,r15,r14,r13,r12,r11,r10,r9,r8,rbp,rdi,rsi,rdx,rcx,rbx,rax,int_no,err,rip,cs,rfl,rsp,ss; };
extern struct panic_framebuffer* get_kernel_framebuffer(void);
static int cx=40,cy=40;
void core_panic_handler(void* rsp) {
    struct cpu_state* s = (struct cpu_state*)rsp;
    serial_printf("PANIC: INT %x RIP %lx\n", s->int_no, s->rip);
    while(1) __asm__("cli; hlt");
}
#define G(n) ".global isr"#n"_stub\nisr"#n"_stub: cli\npushq $0\npushq $"#n"\njmp exc\n"
#define E(n) ".global isr"#n"_stub\nisr"#n"_stub: cli\npushq $"#n"\njmp exc\n"
__asm__(G(0)G(1)G(2)G(3)G(4)G(5)G(6)G(7)E(8)G(9)E(10)E(11)E(12)E(13)E(14)G(15)G(16)E(17)G(18)G(19)G(20)E(21)G(22)G(23)G(24)G(25)G(26)G(27)G(28)G(29)E(30)G(31)
"exc: pushq %rax;pushq %rbx;pushq %rcx;pushq %rdx;pushq %rsi;pushq %rdi;pushq %rbp;pushq %r8;pushq %r9;pushq %r10;pushq %r11;pushq %r12;pushq %r13;pushq %r14;pushq %r15;movq %cr2,%rax;pushq %rax;movq %cr3,%rax;pushq %rax;movq %cr4,%rax;pushq %rax;xorq %rax,%rax;movw %ds,%ax;pushq %rax;movw %es,%ax;pushq %rax;movw %fs,%ax;pushq %rax;movw %gs,%ax;pushq %rax;movq %rsp,%rdi;subq $8,%rsp;call core_panic_handler;hlt");
