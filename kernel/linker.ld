OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)

ENTRY(kernel_main)

SECTIONS
{
    /* Limine expects kernels to be loaded at this virtual address prefix */
    . = 0xffffffff80000000;

    . = ALIGN(64);
    .limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    }

    .text : {
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    .data : {
        *(.data .data.*)

        . = ALIGN(8);
        __usbh_class_info_start__ = .;
        KEEP(*(.usbh_class_info))
        __usbh_class_info_end__ = .;
    }

    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    }
}
