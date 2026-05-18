.global syscall_entry
.extern syscall_dispatcher

syscall_entry:
    swapgs          // Switch to kernel GS
    mov %rsp, %gs:8 // Save user stack (Assuming kernel GS points to a structure holding it)
    mov %gs:0, %rsp // Load kernel stack

    push %rcx
    push %r11

    mov %r10, %rcx
    call syscall_dispatcher

    pop %r11
    pop %rcx

    mov %gs:8, %rsp // Restore user stack
    swapgs
    sysretq
