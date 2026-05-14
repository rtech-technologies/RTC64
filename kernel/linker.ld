/* Tell the linker that we want an x86_64 ELF64 output file */
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

/* Define the symbol _start as the entry point */
ENTRY(_start)

/* Define the program headers we want so the bootloader gives us the right */
/* MMU permissions */
PHDRS
{
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    rodata  PT_LOAD    FLAGS((1 << 2)) ;            /* Read only */
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
}

SECTIONS
{
    /* We wanna be placed in the higher half, like most kernels */
    . = 0xffffffff80000000;

    .text : {
        *(.text .text.*)
    } :text

    /* Move to the next memory page for rodata */
    . = ALIGN(0x1000);

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    /* Move to the next memory page for data */
    . = ALIGN(0x1000);

    .data : {
        *(.data .data.*)
    } :data

    /* Dynamic section for relocations, only needed for KASLR */
    .dynamic : {
        *(.dynamic)
    } :data

    /* NOTE: .bss needs to be the last thing in the last PT_LOAD, otherwise */
    /* the bootloader won't load it correctly */
    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /* Discard some unused sections */
    /DISCARD/ : {
        *(.eh_frame)
        *(.note .note.*)
    }
}
