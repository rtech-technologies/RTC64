.section .text.prologue
.global _start
_start:
    call main
    movq %rax, %rdi
    /* Exit syscall */
    movq $0x0E, %rax /* SYS_EXIT */
    syscall
    hlt
