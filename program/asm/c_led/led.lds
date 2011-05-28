
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
ENTRY(main)

SECTIONS{
    . = 0x33000000;
    .text : {
        *(.text)
        *(.rodata)
    }

    .data ALIGN(4): {
        *(.data)
    }

    .bss ALIGN(4): {
        *(.bss)
    }
}
