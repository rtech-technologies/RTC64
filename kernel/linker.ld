/* Security Hardened Linker Script - RX/RW Separation */
PHDRS
{
    text PT_LOAD FLAGS(5); /* R+E */
    data PT_LOAD FLAGS(6); /* R+W */
}

ENTRY(_start)

SECTIONS
{
    /* Code and Constants grouped into RX segment */
    . = 0xffffffff80000000;

    .text : {
        *(.text .text.*)
    } :text

    .rodata : {
        *(.rodata .rodata.*)
    } :text

    /* Align to page boundary to separate RX from RW */
    . = ALIGN(0x1000);

    /* Writable data grouped into RW segment */
    .data : {
        *(.data .data.*)

        . = ALIGN(8);
        __usbh_class_info_start__ = .;
        KEEP(*(.usbh_class_info))
        __usbh_class_info_end__ = .;
    } :data

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
