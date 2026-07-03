.section .text.prologue
.global _start
_start:
    call main
    movq %rax, %rdi
    /* Exit syscall */
    movq $12, %rax /* SYS_EXIT */
    int $0x80
    hlt
