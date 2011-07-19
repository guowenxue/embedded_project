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

#if 1
    printf("Nandflash Datas from 0x0000 0000~0x0000 0100:\n");
    memset(buf, 0, sizeof(buf));
    nand_read_block(&nand, 0, 0x20000, buf);
    print_buf("Block 0 data:", buf, 64);
#endif

    memset(buf, 0, sizeof(buf));
    nand_read_page(&nand, 0, buf);
    print_buf("Page 0 data:", buf, 64);

    printf("Erase 0x0~0x20000\n");
    nand_erase(&nand, 0, 0x20000);

#if 0
    nand_read_page(&nand, 0, buf);
    print_buf("Page 0 data:", buf, 256);

    printf("Write 0x33 to first page\n");
    memset(buf, 0x33, 0x800);
    nand_write_page(&nand, 0, buf);


    memset(buf, 0x88, sizeof(buf));
    nand_random_write_page(&nand, 0, 0x800, buf, 64);
#endif

    memset(buf, 0, sizeof(buf));
    nand_random_read_page(&nand, 0, 0x800, buf, 64);
    print_buf("Ramdom Read Page:", buf, 64);


#if 0
    memset(buf, 0, sizeof(buf));
    printf("Nandflash Datas from 0x0186 0000~0x0186 0100:\n");
    nand_read_block(&nand, 0x01860000, 0x20000, buf);
    print_buf("Read block 0x01860000 data:", buf, 64);
#endif


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
