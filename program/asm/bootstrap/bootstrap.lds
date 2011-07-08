
/***********************************************************************
 *        File:  led.lds
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  Cross tool link text, refer to u-boot.lds
 *   ChangeLog:  1, Release initial version on "Mon Mar 21 21:09:52 CST 2011"
 *
 ***********************************************************************/

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

SECTIONS{
    /*We will download this program to SDRAM address 0x33f80000 to running*/
    . = 0x33f80000;
    . = ALIGN(4);
    .text   :
    {
      start.o   (.text)
      *(.text)
    }
    . = ALIGN(4);
    .rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }
    . = ALIGN(4);
    .rodata : { *(.rodata) }
    . = ALIGN(4);
    .data : { *(.data) }
    . = ALIGN(4);
    __bss_start = .;
    .bss : { *(.bss) }
    _end = .;
}

