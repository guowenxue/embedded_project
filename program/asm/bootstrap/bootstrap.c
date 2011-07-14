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

static int dbg_mode(void);



int bootstrap_main(void)
{
    struct boot_nand_t nand;
    int i;
    char    buf[0x20000];

    serial_init();
    init_led_beep();

    printf("\bBootstrap Version 0.0.1\n");

    turn_led_on(LED0);


    nand_init(&nand);

    memset(buf, 0, sizeof(buf));
    nand_read_ll(&nand, buf, 0, 0x20000);

    printf("Nandflash Datas from 0x0000 0000~0x0000 0100:\n");
    for(i=0; i<256; i++)
    {
        printf("%02x ", buf[i]); 
    }
    printf("\n");

#if 0
    if (0 == copy_launcher_to_ram())
    {
        run_launcher();
    }
#endif

#if 0
    dbg_mode();
#endif

    while (1)
        ;

    return 0;
}

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
