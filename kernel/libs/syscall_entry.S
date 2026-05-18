.global syscall_entry
.extern syscall_dispatcher

syscall_entry:
    // x86_64 syscall convention:
    // RCX = return address, R11 = RFLAGS
    // RAX = syscall ID, RDI, RSI, RDX, R10, R8, R9 = args

    push %rcx
    push %r11

    mov %r10, %rcx // R10 is 4th arg in syscall, but 4th arg in SysV ABI is RCX
    call syscall_dispatcher

    pop %r11
    pop %rcx
    sysretq
