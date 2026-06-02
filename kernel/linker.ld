/* 1. ELF header / program start / phdrs declaration */
PHDRS
{
    text PT_LOAD FLAGS(5); /* R+E */
    data PT_LOAD FLAGS(6); /* R+W */
}

ENTRY(kernel_main)

SECTIONS
{
    /* 2. .text (code) — :text */
    . = 0xffffffff80000000;

    .text : {
        *(.text .text.*)
    } :text

    /* 3. .rodata (read-only data, constants) — :text */
    .rodata : {
        *(.rodata .rodata.*)
    } :text

    /* 5. ALIGN to page boundary to separate RX and RW segments */
    . = ALIGN(0x1000);

    /* 6. .data (writable initialized data) — :data */
    .data : {
        *(.data .data.*)
        /* 7. KEEP sections that must remain in .data */
        . = ALIGN(8);
        __usbh_class_info_start__ = .;
        KEEP(*(.usbh_class_info))
        __usbh_class_info_end__ = .;
    } :data

    /* 8. .bss (zero-initialized data / COMMON) — :data */
    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /DISCARD/ : {
        *(.note .note.*)
        *(.comment)
        *(.eh_frame)
    }
}
