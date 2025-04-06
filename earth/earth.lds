OUTPUT_ARCH("riscv")

ENTRY(_enter)

MEMORY
{
    code (rx) : ORIGIN = 0x80000000, LENGTH = 0x20000
    data (rw) : ORIGIN = 0x80020000, LENGTH = 0x20000
    disk(irx!wa) : ORIGIN = 0x20800000, LENGTH = 0x400000
}

PHDRS
{
    code PT_LOAD;
    data PT_LOAD;
    disk PT_LOAD;
}

SECTIONS
{
    .init : ALIGN(8) {
        *(.text.enter)
    } >code :code

    .text : ALIGN(8) {
        *(.text .text.*)
    } >code :code

    .rodata : ALIGN(8) {
        *(.rdata)
        *(.rodata .rodata.*)
        . = ALIGN(8);
        *(.srodata .srodata.*)
    } >data :data

    .data : ALIGN(8) {
        *(.data .data.*)
        . = ALIGN(8);
        *(.sdata .sdata.* .sdata2.*)
    } >data :data

    .bss (NOLOAD): ALIGN(8) {
        *(.sbss*)
        *(.bss .bss.*)
        *(COMMON)
    } >data :data

    .heap (NOLOAD) : ALIGN(8) {
        PROVIDE( __heap_start = . );
    } >data :data

    .image : {
        *(.image.placeholder);
        . += 0x400000;
    } >disk :disk

    PROVIDE( __heap_end = 0x80100000 );
}
