/********************************************************************************************
 *        File:  k9f2g08.c: K9F2G08 read/write/erase functions for bootstrap
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  This C code is the first stage bootloader(named bootstrap) 
                 main code, test on FL2440 board.
 *   ChangeLog:  1, Release initial version on "Tue Jul 12 16:43:18 CST 2011"
 *
 *******************************************************************************************/


/*************************************************************
 * Read Status Register Definition of K9F2G08(Refer to k9f2g08u0m datasheet)
 *  
 * |-------------------------------------------------------------------------------------|
 * | I/O No. | Page Program | Block Erase | Cache Program |     Read    | Definition     |
 * |-------------------------------------------------------------------------------------|
 * | I/O 0   |  Pass/Fail   |  Pass/Fail  | Pass/Fail(N)  |   Not use   | Pass:0 Fail:1  |
 * |-------------------------------------------------------------------------------------|
 * | I/O 1   |  Not use     |  Not use    | Pass/Fail(N-1)|   Not use   | Pass:0 Fail:1  |
 * |-------------------------------------------------------------------------------------|
 * | I/O 2   |  Not use     |  Not use    | Not use       |   Not use   | Don't care     |
 * |-------------------------------------------------------------------------------------|
 * | I/O 3   |  Not use     |  Not use    | Not use       |   Not use   | Don't care     |
 * |-------------------------------------------------------------------------------------|
 * | I/O 4   |  Not use     |  Not use    | Not use       |   Not use   | Don't care     |
 * |-------------------------------------------------------------------------------------|
 * | I/O 5   |  Ready/Busy  |  Ready/Busy |True Ready/Busy| Ready/Busy  | Busy:0 Ready:1 |
 * |-------------------------------------------------------------------------------------|
 * | I/O 6   |  Ready/Busy  |  Ready/Busy | Ready/Busy    | Ready/Busy  | Busy:0 Ready:1 |
 * |-------------------------------------------------------------------------------------|
 * | I/O 7   |Write protect |Write protect| Write protect |Write protect| Protect:0 Not:1|
 * |-------------------------------------------------------------------------------------|
 *
 *************************************************************/

#include <nand.h>

/**
 * nand_wait - wait Nandflash goes to Ready to operate
 */
static inline void nand_wait(void)
{
    int i;

    /*NFSTAG register bit[0] is RnB:  0->Nandflash Busy  1->Nandflash Ready to operate */
    while (!(REG_NFSTAT & NFSTAT_BUSY))  /*bit[0]*/
    {
//        printf("Nandflash is busy.\n");
        for (i = 0; i < 10; i++) ;
    }
}


/**
 * nand_wait - Read the Nandflash ID
 */
static unsigned short nand_read_id(void)
{
    unsigned short res = 0;

    nand_select(); /* chip Enable */
    nand_clear_RnB(); /*Clear Ready & Busy signal*/

    REG_NFCMD = NAND_CMD_READID; /*Read ID command*/
    REG_NFADDR = 0x00;  /*Give address 0x00*/
    res = REG_NFDATA;  /*First byte, Maker code: 0xEC->Samsung*/
    res = (res << 8) | REG_NFDATA; /*Second byte, Device ID:0xDA->K9F2G08*/

    /*We can read the followed 3 bytes(0x10,0x95,0x44) here, but it's useless.*/

    nand_deselect();

    return res;
}

#if 0
/**
 * nand_wait - Reset the Nandflash 
 */
static void nand_reset(void)
{
    nand_select(); /* chip Enable */
    nand_clear_RnB(); /*Clear Ready & Busy signal*/

    REG_NFCMD = NAND_CMD_RESET; /*Read ID command*/
    REG_NFADDR = 0x00;  /*Give address 0x00*/

    nand_detect_RnB();
    nand_deselect();
}
#endif

#define TACLS       1//7    // 1-clk(0ns) 
#define TWRPH0      4//7    // 3-clk(25ns)
#define TWRPH1      1//7    // 1-clk(10ns)  //TACLS+TWRPH0+TWRPH1>=50ns

/**
 * nand_init - Initial the Nandflash controller and nand flash information structure "nand"
 */
int nand_init(struct boot_nand_t *nand)
{
    int i, ret = -1;
    unsigned short nand_id;

    /*TACL=1, TWRPH0=4, TWRPH1=1, bit[0]=0 means the Nandflash bit witdth is 8bit(K9F2G08 is 8bit)*/
    REG_NFCONF= (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4)|(0<<0);

    /* 
     * Bit[13:12]  No lock
     * Bit[10:8]   No nandflash interrupt
     * Bit[6:4]    Initialize ECC and lock data area & spare area ECC
     * Bit[1]      Enable nand flash chip select
     * Bit[0]      Enable nand flash controller
     */
    REG_NFCONT= (0<<13)|(0<<12)|(0<<10)|(0<<9)|(0<<8)|(1<<6)|(1<<5)|(1<<4)|(1<<1)|(1<<0);


    for (i=0; i<10; i++) ;  /*Wait tWB(100ns)*/

    nand_id = nand_read_id();

    /*K9F2G08 page/read/write size is 2K bytes, spare size is 64 bytes 
     * and block/erase size is 128K*/
    if ( NAND_K9F2G08 == nand_id )  /* Samsung K9F2G08 */
    {                         
        nand->id = NAND_K9F2G08;
        nand->spare_size = 64;    /* 64 bytes */
        nand->page_size = 0x0800;    /* 1<<11=2K */
        nand->block_size = 0x20000;  /* 1<<17=128K*/
        /*Bad block tag is the first byte in spare area*/
        nand->bad_block_offset = nand->page_size; 
        ret = 0;
    }

    return ret;
}


/**
 * nand_read_page - Read the page main area data, don't include the 64 bytes spare date.
 * @nand:      Nandflash information
 * @page_num:  The page number in the whole nand flash address space, it should can get by:
 *               addr>>11 (For page size is 2K)
 * @data:      The page read data output buffer
 */
int nand_read_page(struct boot_nand_t *nand, ulong page_num, char *data)
{
    unsigned short *ptr16 = (unsigned short *)data;
    unsigned int i;

    if(NAND_K9F2G08 != nand->id )
        return -1;

    nand_select();
    nand_clear_RnB();

    REG_NFCMD = NAND_CMD_READ0;

    /* Write Address */
    REG_NFADDR = 0;
    REG_NFADDR = 0;
    REG_NFADDR = page_num & 0xff;
    REG_NFADDR = (page_num >> 8) & 0xff;
    REG_NFADDR = (page_num >> 16) & 0xff;
    REG_NFCMD = NAND_CMD_READSTART;

    nand_detect_RnB();

    for (i=0; i < (nand->page_size >> 1); i++)
    {
        /* For NFDATA register is 16bit in Nand flash controller in MUC, so we can read by 16 bits
         * although the nandflash's width is 8 bytes, most work done by the Nandflash controller*/
        *ptr16 = REG_NFDATA16; 
        ptr16++;
    }

    nand_deselect();
    return nand->page_size;
}

/**
 * nand_random_read_page - Random read the data from a page 
 * @nand:       The nand flash information
 * @page_num:   The read page number
 * @offset:     The read data start offset address in the page
 * @size:       The read data length
 * @data:       The read data output buffer
 */

int nand_random_read_page(struct boot_nand_t *nand, ulong page_num, ulong offset, char *data, ulong size)
{
    uchar  *ptr8 = (uchar *)data;
    ulong  i, bytes;

    /*Avoid the offset+size larger than a page size(include main data and spare data size) */
    bytes = ((nand->page_size+nand->spare_size-offset)>size ? size : (nand->page_size+nand->spare_size)-offset);

    nand_select();
    nand_clear_RnB();

    REG_NFCMD = NAND_CMD_READ0;
    /* Write Address */
    REG_NFADDR = 0;
    REG_NFADDR = 0;
    REG_NFADDR = page_num & 0xff;
    REG_NFADDR = (page_num >> 8) & 0xff;
    REG_NFADDR = (page_num >> 16) & 0xff;
    REG_NFCMD = NAND_CMD_READSTART;

    nand_detect_RnB();

    REG_NFCMD = NAND_CMD_RNDOUT;
    REG_NFADDR = offset & 0xff;
    REG_NFADDR = (offset >> 8) & 0xff;
    REG_NFCMD = NAND_CMD_RNDOUTSTART;

    for(i=0; i<bytes; i++)
    {
       *ptr8 = REG_NFDATA;
        ptr8++;
    }

    nand_deselect();

    return bytes;
}


/**
 * nand_write_page - Write the data to main area data in the page
 * @nand:      Nandflash information
 * @page_num:  The page number which will be written
 * @data:      The data needs to be written 
 */
int nand_write_page(struct boot_nand_t *nand, ulong page_num, char *data)
{
    uchar  *ptr8 = (uchar *)data;
    uchar  stat;
    uint    i;
    int     ret = nand->page_size;

    if(NAND_K9F2G08 != nand->id )
            return -1;

    nand_select();
    nand_clear_RnB();

    REG_NFCMD = NAND_CMD_SEQIN;

    /* Write Address */ 
    REG_NFADDR = 0; 
    REG_NFADDR = 0; 
    REG_NFADDR = page_num & 0xff; 
    REG_NFADDR = (page_num >> 8) & 0xff; 
    REG_NFADDR = (page_num >> 16) & 0xff; 

    for (i=0; i<nand->page_size; i++)
    {
        /* For NFDATA register is 16bit in Nand flash controller in MCU, so we can read by 16 bits
         * although the nandflash's width is 8 bytes, most work done by the Nandflash controller*/
        REG_NFDATA = *ptr8; 
        ptr8++;
    }

    REG_NFCMD = NAND_CMD_PAGEPROG;

    /* We can detect the RnB or the Status register bit[6] as follow like, 
     * to indicate the program is completed or not*/
    nand_wait();  /*Wait for RnB bit goes to high means Nandflash Ready*/

    /* When program operation is completed, the Write Status register Bit[I/O] 0 maybe checked, 
     * bit[0]=0 means pass, or bit[0]=1 means Fail. Refer the top table definition  */
    REG_NFCMD = NAND_CMD_STATUS;
    stat = REG_NFDATA;
    if(stat & 0x01)   
    {
        printf("Program page number @%lu failure\n", page_num);
        ret = -1; /*Program failure*/
    }

    nand_deselect();
    return ret;
}


/**
 * nand_random_write_page - Random write the data to a page 
 * @nand:       The nand flash information
 * @page_num:   The written page number
 * @offset:     The written data start offset address in the page
 * @size:       The written data length
 * @data:       The written data 
 */

int nand_random_write_page(struct boot_nand_t *nand, ulong page_num, ulong offset, char *data, ulong size)
{
    uchar  *ptr8 = (uchar *)data;
    uchar  stat;
    int    ret;
    ulong  i, bytes;

    /*Avoid the offset+size larger than a page size(include main data and spare data size) */
    bytes = ((nand->page_size+nand->spare_size-offset)>size ? size : (nand->page_size+nand->spare_size)-offset);

    nand_select();
    nand_clear_RnB();

    REG_NFCMD = NAND_CMD_SEQIN;
    /* Write Address */
    REG_NFADDR = 0;
    REG_NFADDR = 0;
    REG_NFADDR = page_num & 0xff;
    REG_NFADDR = (page_num >> 8) & 0xff;
    REG_NFADDR = (page_num >> 16) & 0xff;

    REG_NFCMD = NAND_CMD_RNDIN;

    REG_NFADDR = offset & 0xff;
    REG_NFADDR = (offset >> 8) & 0xff;

    for(i=0; i<bytes; i++)
    {
        REG_NFDATA = *ptr8; 
        ptr8++;
    }

    REG_NFCMD = NAND_CMD_PAGEPROG;

    /* We can detect the RnB or the Status register bit[6] as follow like, 
     * to indicate the program is completed or not*/
    nand_wait();  /*Wait for RnB bit goes to high means Nandflash Ready*/

    /* When program operation is completed, the Write Status register Bit[I/O] 0 maybe checked, 
     * bit[0]=0 means pass, or bit[0]=1 means Fail. Refer the top table definition  */
    REG_NFCMD = NAND_CMD_STATUS;
    stat = REG_NFDATA;
    if(stat & 0x01)   
    {
        printf("Random Program page number @%lu failure\n", page_num);
        ret = -1; /*Program failure*/
    }
    else
    {
        ret = bytes; 
    }

    nand_deselect();
    return ret;
}


/**
 * is_bad_block - check the block which the "addr" in is valid or not
 * @nand:  Nandflash information
 * @addr:  The offset address in whole nandflash, must make sure it's block alignment
 */
int is_bad_block(struct boot_nand_t *nand, unsigned long addr)
{
    char data;
    ulong page_num;
    int  page_shift = generic_ffs(nand->page_size)-1;

    /*Make sure the address is block alignment*/
    if ((NAND_K9F2G08!=nand->id) || (addr & (nand->block_size-1)) ) 
            return -1;

    /* Get the page number from "addr", for addr is block alignment, 
     * so it should be the first page in the block*/
    page_num = addr >> page_shift;     

    nand_random_read_page(nand, page_num, nand->bad_block_offset, &data, 1);

    if (data != 0xff)
    {
        printf("Bad Block @%08lx\n", addr);
        nand_deselect();
        return 1;
    }

    nand_deselect();
    return 0;
}

/**
 * mark_bad_block - Mark the block as bad block
 * @nand:  Nandflash information
 * @addr:  The offset address in whole nandflash, must make sure it's block alignment
 */
int mark_bad_block(struct boot_nand_t *nand, unsigned long addr)
{
    char data = 0xEE;
    ulong page_num;
    int  page_shift = generic_ffs(nand->page_size)-1;
    int  ret;

    /*Make sure the address is block alignment*/
    if ((NAND_K9F2G08!=nand->id) || (addr & (nand->block_size-1)) ) 
            return -1;

    /* Get the page number from "addr", for addr is block alignment, 
     * so it should be the first page in the block*/
    page_num = addr >> page_shift;     

    ret = nand_random_write_page(nand, page_num, nand->bad_block_offset, &data, 1);

    printf("Mark bad Block @%08lx", addr);
    if (ret < 0)
    {
        printf("Fail.\n");
        nand_deselect();
        return 1;
    }

    printf("OK.\n");
    nand_deselect();
    return 0;
}



/**
 * nand_read_block - Read some blocks data 
 * @nand:        Nandflash information
 * @start_addr:  The read start address in nandflash, which should be block alignment
 * @size:        The need read data length, which should be block alignment
 * @buf:         The read data output data
 *
 */
int nand_read_block(struct boot_nand_t *nand, ulong start_addr, int size, char *buf)
{
    ulong addr, len=0;
    ulong page_num;
    int  page_shift = generic_ffs(nand->page_size)-1;

    /*Make sure start_addr and size is block aligment*/
    if ( (start_addr & (nand->block_size-1)) || (size & ((nand->block_size-1))) )
    {
        return -1;  
    }

    addr=start_addr;

    while ( len < size )
    {
        /*If it's a block start address, then check this block validation*/
        if ((addr & (nand->block_size-1) )== 0)  
        {
            if ( is_bad_block(nand, addr) )
            {
                /* Bad block and goto next block */
                addr += nand->block_size;
                printf("Skip bad block @0x%08lx\n", addr);
                continue;
            }
        }

        page_num = addr >> page_shift; 
        //printf("addr=0x%08lx page_num=%lu\n", addr, page_num);
        nand_read_page(nand, page_num, buf);

        addr += nand->page_size;   /*Address go to next page*/
        buf += nand->page_size;    /*buffer point increase a page size*/
        len+= nand->page_size;       /*Already read size increase a page size*/
    }

    return 0;
}

int nand_erase(struct boot_nand_t *nand, ulong start_addr, int size)
{
    uchar   stat;
    int     block_num, ret=0;
    ulong   addr;
    int     block_shift = generic_ffs(nand->block_size)-1;

    if ( (start_addr & (nand->block_size-1)) || (size & ((nand->block_size-1))) )
    {
        return -1;              /* invalid alignment */
    }

    nand_select(); 

    for(addr=start_addr; addr<(start_addr+size); )
    {

        block_num = start_addr>>block_shift; 
        nand_clear_RnB();

        REG_NFCMD = NAND_CMD_ERASE1; 
        REG_NFADDR = block_num & 0xff; 
        REG_NFADDR = (block_num >> 8) & 0xff; 
        REG_NFADDR = (block_num >> 16) & 0xff; 
        REG_NFCMD = NAND_CMD_ERASE2; 
        
        /* We can detect the RnB or the Status register bit[6] as follow like, 
         * to indicate the program is completed or not*/
        nand_wait();  /*Wait for RnB bit goes to high means Nandflash Ready*/

        /* When erase operation is completed, the Write Status register Bit[I/O] 0 maybe checked, 
         * bit[0]=0 means pass, or bit[0]=1 means Fail. Refer the top table definition  */
        REG_NFCMD = NAND_CMD_STATUS;
        stat = REG_NFDATA;
        if( stat&(1<<0) )
        {
             printf("Erase block @0x%08lx failed.\n", addr); 
             ret = -2;
        }

        addr+=nand->block_size;
    }

    nand_deselect();

    return ret;    
}


