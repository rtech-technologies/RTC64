/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
.section .bss
.align 16
kernel_stack:
    .skip 65536
kernel_stack_top:

.section .text
.global _start
.extern kernel_main
_start:
    /* Secure High-Power Stack Genesis */
    cli
    movq $kernel_stack_top, %rsp
    movq %rsp, %rbp

    /* Cross into the Executive Domain */
    call kernel_main

    /* Fallback Safety */
1:  hlt
    jmp 1b

.section .note.GNU-stack,"",@progbits
