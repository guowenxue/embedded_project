/********************************************************************************************
 *        File:  bootstrap.c
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  This C code is the first stage bootloader(named bootstrap) 
                 main code, test on FL2440 board.
 *   ChangeLog:  1, Release initial version on "Tue Jul 12 16:43:18 CST 2011"
 *
 *******************************************************************************************/

#include <common.h>
#include <nand.h>

//static int dbg_mode(void);

void print_buf(char *prompt, char *buf, int len)
{
    int i;
    printf("%s %d bytes\n", prompt, len);
    printf("------------------------------------------------------------------------------------\n");
    for(i=0; i<len; i++) 
    {
        printf("0x%02x ", buf[i]); 
        if(! (i+1)%32 )
            printf("\n");
    }

    printf("\n");
    printf("------------------------------------------------------------------------------------\n");
}


int bootstrap_main(void)
{
    struct boot_nand_t nand;
    char    buf[0x20000];

    serial_init();
    init_led_beep();

    printf("\bBootstrap Version 0.0.1\n");

    turn_led_on(LED0);

    if (nand_init(&nand) < 0)
    {
        printf("Nandflash init failure.\n"); 
        return -1;
    }


#if 0
    nand_erase(&nand, 0x00000, 0x20000);

    memset(buf,0x44,sizeof(buf));
    nand_write(&nand, 0x0, nand.page_size, buf);

    memset(buf,0x00, sizeof(buf));
    nand_read(&nand, 0x00000, nand.page_size, buf);
    print_buf("Whole page Write:", buf, nand.page_size);


    nand_erase(&nand, 0x1860000, 0x20000);

    memset(buf,0x44, sizeof(buf));
    nand_write(&nand, 0x1860000, nand.page_size>>1, buf);

    memset(buf,0x00, sizeof(buf));
    nand_read(&nand, 0x1860000, nand.page_size, buf);
    print_buf("Whole page read:", buf, nand.page_size);
#endif


    while (1)
        ;

    return 0;
}

#if 0
int dbg_mode(void)
{
    long size;
    char *buf = (char *)TEXT_BASE;

    beep(1);

    printf("Xmodem Receive now:\n");

    size = xmodem_recv(buf);

    printf("\tBootstrap Receive File Size: %ld bytes\n", size);

    return 0;
}
#endif
