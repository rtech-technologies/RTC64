ENTRY(_start)

SECTIONS
{
    . = 0x400000;

    .text : {
        *(.text.prologue)
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        *(.bss*)
    }
}
