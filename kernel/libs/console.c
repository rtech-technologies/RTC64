#include <rsl.h>
#include <nuklear.h>

extern struct nk_context ctx;
extern void nk_input_begin_wrap(struct nk_context *ctx);
extern void nk_input_char_wrap(struct nk_context *ctx, char c);
extern void nk_input_end_wrap(struct nk_context *ctx);

static inline uint64_t do_syscall(uint64_t id, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    uint64_t ret;
    __asm__ volatile (
        "mov %1, %%rax\n"
        "mov %2, %%rdi\n"
        "mov %3, %%rsi\n"
        "mov %4, %%rdx\n"
        "syscall\n"
        "mov %%rax, %0\n"
        : "=r"(ret)
        : "g"(id), "g"(arg1), "g"(arg2), "g"(arg3)
        : "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory"
    );
    return ret;
}

void print(const char* msg) {
    do_syscall(1, (uint64_t)msg, 0, 0);
}

char* input(const char* prompt) {
    print(prompt);
    static char buf[128];
    do_syscall(2, (uint64_t)buf, 0, 0);

    // Route keystrokes into Nuklear execution loop
    char* p = buf;
    while (*p) {
        nk_input_begin_wrap(&ctx);
        nk_input_char_wrap(&ctx, *p);
        nk_input_end_wrap(&ctx);
        p++;
    }

    return buf;
}
