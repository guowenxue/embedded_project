/*
 * nand_read.c: Simple NAND read functions for booting from NAND
 *
 * This is used by cpu/arm920/start.S assembler code,
 * and the board-specific linker script must make sure this
 * file is linked within the first 4kB of NAND flash.
 *
 * Taken from GPLv2 licensed vivi bootloader,
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 * Author: Hwang, Chideok <hwang@mizi.com>
 * Date  : $Date: 2004/02/04 10:37:37 $
 *
 * u-boot integration and bad-block skipping (C) 2006 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 */

#include <nand.h>

static inline void nand_wait(void)
{
    int i;

    while (!(REG_NFSTAT & NFSTAT_BUSY))
        for (i = 0; i < 10; i++) ;
}


static int is_bad_block(struct boot_nand_t *nand, unsigned long i)
{
    unsigned char data;
    unsigned long page_num;

    nand_clear_RnB();
    if (nand->page_size == 512)
    {
        REG_NFCMD = NAND_CMD_READOOB;   /* 0x50 */
        REG_NFADDR = nand->bad_block_offset & 0xf;
        REG_NFADDR = (i >> 9) & 0xff;
        REG_NFADDR = (i >> 17) & 0xff;
        REG_NFADDR = (i >> 25) & 0xff;
    }
    else if (nand->page_size == 2048)
    {
        page_num = i >> 11;     /* addr / 2048 */
        REG_NFCMD = NAND_CMD_READ0;
        REG_NFADDR = nand->bad_block_offset & 0xff;
        REG_NFADDR = (nand->bad_block_offset >> 8) & 0xff;
        REG_NFADDR = page_num & 0xff;
        REG_NFADDR = (page_num >> 8) & 0xff;
        REG_NFADDR = (page_num >> 16) & 0xff;
        REG_NFCMD = NAND_CMD_READSTART;
    }
    else
    {
        return -1;
    }
    nand_wait();
    data = (REG_NFDATA & 0xff);
    if (data != 0xff)
        return 1;

    return 0;
}

static int nand_read_page_ll(struct boot_nand_t *nand, unsigned char *buf, unsigned long addr)
{
    unsigned short *ptr16 = (unsigned short *)buf;
    unsigned int i, page_num;

    nand_clear_RnB();

    REG_NFCMD = NAND_CMD_READ0;

    if (nand->page_size == 512)
    {
        /* Write Address */
        REG_NFADDR = addr & 0xff;
        REG_NFADDR = (addr >> 9) & 0xff;
        REG_NFADDR = (addr >> 17) & 0xff;
        REG_NFADDR = (addr >> 25) & 0xff;
    }
    else if (nand->page_size == 2048)
    {
        page_num = addr >> 11;  /* addr / 2048 */
        /* Write Address */
        REG_NFADDR = 0;
        REG_NFADDR = 0;
        REG_NFADDR = page_num & 0xff;
        REG_NFADDR = (page_num >> 8) & 0xff;
        REG_NFADDR = (page_num >> 16) & 0xff;
        REG_NFCMD = NAND_CMD_READSTART;
    }
    else
    {
        return -1;
    }
    nand_wait();

#if defined(CONFIG_S3C2410)
    for (i = 0; i < nand->page_size; i++)
    {
        *buf = (REG_NFDATA & 0xff);
        buf++;
    }
#elif defined(CONFIG_S3C2440) || defined(CONFIG_S3C2442)
    for (i = 0; i < (nand->page_size >> 1); i++)
    {
        *ptr16 = REG_NFDATA16;
        ptr16++;
    }
#endif

    return nand->page_size;
}

static unsigned short nand_read_id(void)
{
    unsigned short res = 0;
    REG_NFCMD = NAND_CMD_READID;
    REG_NFADDR = 0;
    res = REG_NFDATA;
    res = (res << 8) | REG_NFDATA;
    return res;
}

#define TACLS       1//7    // 1-clk(0ns) 
#define TWRPH0      4//7    // 3-clk(25ns)
#define TWRPH1      1//7    // 1-clk(10ns)  //TACLS+TWRPH0+TWRPH1>=50ns

int nand_init(struct boot_nand_t *nand)
{
    int i, ret = -1;
    unsigned short nand_id;

    REG_NFCONF= (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4)|(0<<0);
    REG_NFCONT= (0<<13)|(0<<12)|(0<<10)|(0<<9)|(0<<8)|(1<<6)|(1<<5)|(1<<4)|(1<<1)|(1<<0);

    /* chip Enable */
    nand_select();
    nand_clear_RnB();

    for (i = 0; i < 10; i++)
        ;

    nand_id = nand_read_id();
    if (0)
    {                           /* dirty little hack to detect if nand id is misread */
        unsigned short *nid = (unsigned short *)0x31fffff0;
        *nid = nand_id;
    }
    printf("Nand ID: %04x\n", nand_id);

    if ( NAND_K9F2G08 == nand_id )  /* Samsung K9F2G08 */
    {                         
        nand->page_size = 2048;
        nand->block_size = 128 * 1024;
        nand->bad_block_offset = nand->page_size;
        ret = 0;
    }

    return ret;
}

/* low level nand read function */
int nand_read_ll(struct boot_nand_t *nand, unsigned char *buf, unsigned long start_addr, int size)
{
    int i, j;

    if ((start_addr & (nand->block_size - 1)) || (size & ((nand->block_size - 1))))
    {
        printf("Address not alignment.\n");
        return -1;              /* invalid alignment */
    }

    for (i = start_addr; i < (start_addr + size);)
    {
        if ((i & (nand->block_size - 1) )== 0)
        {
            if (is_bad_block(nand, i) || is_bad_block(nand, i+nand->page_size))
            {
                /* Bad block */
                i += nand->block_size;
                size += nand->block_size;
                continue;
            }
        }

        j = nand_read_page_ll(nand, buf, i);
        i += j;
        buf += j;
    }

    return 0;
}
