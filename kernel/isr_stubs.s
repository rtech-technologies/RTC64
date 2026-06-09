/* Modified by Sovereign: High-performance ISR stubs with Preemptive Return and SSE/FPU State support */
.extern exception_handler
.extern scheduler_switch

.macro isr_no_err num
.global isr_stub_\num
isr_stub_\num:
    pushq $0
    pushq $\num
    jmp isr_common
.endm

.macro isr_err num
.global isr_stub_\num
isr_stub_\num:
    pushq $\num
    jmp isr_common
.endm

isr_no_err 0
isr_no_err 1
isr_no_err 2
isr_no_err 3
isr_no_err 4
isr_no_err 5
isr_no_err 6
isr_no_err 7
isr_err    8
isr_no_err 9
isr_err    10
isr_err    11
isr_err    12
isr_err    13
isr_err    14
isr_no_err 15
isr_no_err 16
isr_err    17
isr_no_err 18
isr_no_err 19
isr_no_err 20
isr_no_err 21
isr_no_err 22
isr_no_err 23
isr_no_err 24
isr_no_err 25
isr_no_err 26
isr_no_err 27
isr_no_err 28
isr_no_err 29
isr_err    30
isr_no_err 31

.macro irq num
.global isr_stub_\num
isr_stub_\num:
    pushq $0
    pushq $\num
    jmp irq_common
.endm

irq 32
irq 33
irq 34
irq 35
irq 36
irq 37
irq 38
irq 39
irq 40
irq 41
irq 42
irq 43
irq 44
irq 45
irq 46
irq 47

isr_common:
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movq %cr2, %rax
    pushq %rax
    movq %cr3, %rax
    pushq %rax
    movq %cr4, %rax
    pushq %rax

    xorq %rax, %rax
    movw %gs, %ax
    pushq %rax
    movw %fs, %ax
    pushq %rax
    movw %es, %ax
    pushq %rax
    movw %ds, %ax
    pushq %rax

    /* ALIGN STACK FOR FXSAVE (16-byte boundary) */
    pushq %rbp
    movq %rsp, %rbp
    andq $-16, %rsp

    subq $512, %rsp
    fxsave (%rsp)

    movq %rsp, %rdi
    call exception_handler

    fxrstor (%rsp)

    movq %rbp, %rsp
    popq %rbp

    popq %rax
    movw %ax, %ds
    popq %rax
    movw %ax, %es
    popq %rax
    movw %ax, %fs
    popq %rax
    movw %ax, %gs

    popq %rax
    movq %rax, %cr4
    popq %rax
    movq %rax, %cr3
    popq %rax
    movq %rax, %cr2

    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    addq $16, %rsp
    iretq

irq_common:
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movq %cr2, %rax
    pushq %rax
    movq %cr3, %rax
    pushq %rax
    movq %cr4, %rax
    pushq %rax

    xorq %rax, %rax
    movw %gs, %ax
    pushq %rax
    movw %fs, %ax
    pushq %rax
    movw %es, %ax
    pushq %rax
    movw %ds, %ax
    pushq %rax

    /* ALIGN STACK FOR FXSAVE (16-byte boundary) */
    pushq %rbp
    movq %rsp, %rbp
    andq $-16, %rsp

    subq $512, %rsp
    fxsave (%rsp)

    movq %rsp, %rdi
    call exception_handler

    /* POWER: Call scheduler to get new stack pointer */
    movq %rsp, %rdi
    call scheduler_switch
    movq %rax, %rsp

    fxrstor (%rsp)

    movq %rbp, %rsp
    popq %rbp

    popq %rax
    movw %ax, %ds
    popq %rax
    movw %ax, %es
    popq %rax
    movw %ax, %fs
    popq %rax
    movw %ax, %gs

    popq %rax
    movq %rax, %cr4
    popq %rax
    movq %rax, %cr3
    popq %rax
    movq %rax, %cr2

    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    addq $16, %rsp
    iretq

.section .data
.global isr_stub_table
isr_stub_table:
    .quad isr_stub_0
    .quad isr_stub_1
    .quad isr_stub_2
    .quad isr_stub_3
    .quad isr_stub_4
    .quad isr_stub_5
    .quad isr_stub_6
    .quad isr_stub_7
    .quad isr_stub_8
    .quad isr_stub_9
    .quad isr_stub_10
    .quad isr_stub_11
    .quad isr_stub_12
    .quad isr_stub_13
    .quad isr_stub_14
    .quad isr_stub_15
    .quad isr_stub_16
    .quad isr_stub_17
    .quad isr_stub_18
    .quad isr_stub_19
    .quad isr_stub_20
    .quad isr_stub_21
    .quad isr_stub_22
    .quad isr_stub_23
    .quad isr_stub_24
    .quad isr_stub_25
    .quad isr_stub_26
    .quad isr_stub_27
    .quad isr_stub_28
    .quad isr_stub_29
    .quad isr_stub_30
    .quad isr_stub_31
    .quad isr_stub_32
    .quad isr_stub_33
    .quad isr_stub_34
    .quad isr_stub_35
    .quad isr_stub_36
    .quad isr_stub_37
    .quad isr_stub_38
    .quad isr_stub_39
    .quad isr_stub_40
    .quad isr_stub_41
    .quad isr_stub_42
    .quad isr_stub_43
    .quad isr_stub_44
    .quad isr_stub_45
    .quad isr_stub_46
    .quad isr_stub_47

.section .note.GNU-stack,"",@progbits
