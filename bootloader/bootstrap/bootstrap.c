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

int dbg_mode(struct boot_nand_t *nand);
unsigned long do_go_exec (ulong (*entry)(int, char * const []), int argc, char * const argv[]);

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
    ulong  test_block = 0x0;  /*0x1860000 is bad block*/
//    char *addr = (char *)TEXT_BASE;
    char *addr = (char *)0x36000000;

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
    nand_scrub(&nand);

    nand_erase(&nand, test_block, 0x20000, SKIP_BAD_BLOCK);

    memset(buf,0x00, sizeof(buf));
    nand_read_spare(&nand, test_block, 64, buf);
    print_buf("Spare area read:", buf, 64);

    nand_read(&nand, test_block, nand.page_size, buf);
    print_buf("Whole page Write:", buf, nand.page_size);
#endif

#if 0
    /*Write spare area*/
    memset(buf,0x44,sizeof(buf));

    nand_write(&nand, test_block, nand.page_size, buf);
    nand_read(&nand, test_block, nand.page_size, buf);
    print_buf("Whole page Write:", buf, nand.page_size);


    nand_write_spare(&nand, 0x0000, 64, buf);
    nand_read_spare(&nand, test_block, 64, buf);
    print_buf("Spare area read:", buf, 64);
#endif

    dbg_mode(&nand);

    nand_read(&nand, LAUNCHER_NAND_ADDR, 1024, addr);
    do_go_exec((void *)addr, 0, NULL);

    while (1)
        ;

    return 0;
}


__attribute__((weak))
unsigned long do_go_exec (ulong (*entry)(int, char * const []), int argc, char * const argv[])
{
     printf ("## Starting application at 0x%08lX ...\n", entry);
     return entry (argc, argv);
}

int dbg_mode(struct boot_nand_t *nand)
{
    long size;
    char *addr = (char *)TEXT_BASE;
    ulong erase_size;

    dbg_print("Comes into bootstrap debug mode and Xmodem wait for receive new launcher:\n");

    beep(1);

    size = xmodem_recv(addr);
    dbg_print("\tBootstrap Receive Launcher file size: %ld bytes\n", size);

    erase_size = (size/nand->block_size + 1)*nand->block_size;

    nand_erase(nand, LAUNCHER_NAND_ADDR, erase_size, SKIP_BAD_BLOCK);
    nand_write(nand, LAUNCHER_NAND_ADDR, size, addr);

    return 0;
}

