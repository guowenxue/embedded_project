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
        dbg_print("Nandflash is busy.\n");
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

#define TACLS       3//1    // 1-clk(0ns) 
#define TWRPH0      7//4    // 3-clk(25ns)
#define TWRPH1      7//1    // 1-clk(10ns)  //TACLS+TWRPH0+TWRPH1>=50ns

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
 * nand_read_page - Read the data from a page main data area 
 * @nand:       The nand flash information
 * @page_num:   The read page number
 * @offset:     The read data start offset address in the page
 * @size:       The request read data length
 * @data:       The read data  output buffer
 */
int nand_read_page(struct boot_nand_t *nand, ulong page_num, ulong offset, char *data, ulong size)
{
    uchar  *ptr = (uchar *)data;
    ulong  i, bytes;

    /*Avoid the offset+size larger than a page size(include main data and spare data size) */
    bytes = ((nand->page_size+nand->spare_size-offset)>=size ? size : (nand->page_size+nand->spare_size)-offset);

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

    /* 
     * After give NAND_CMD_READSTART command, when the RnB get a rising edge signal, it 
     * will set(Read S3C2440 datasheet about NFCONT register) the NFSTAT register bit[2] 
     * in nandflash controller. After get this signal, then we can read the data or send  
     * followed command
     */
    nand_detect_RnB();

    /* If the offset address in the page is 0 and need read a whole page, 
     * then we read the whole page, or use the random page read */
    if(0==offset && nand->page_size==size)
            goto START_READ;

    /*Page Random Read will come here*/
    REG_NFCMD = NAND_CMD_RNDOUT;
    REG_NFADDR = offset & 0xff;
    REG_NFADDR = (offset >> 8) & 0xff;
    REG_NFCMD = NAND_CMD_RNDOUTSTART;

START_READ:
    for(i=0; i<bytes; i++)
    {
       *ptr = REG_NFDATA;
        ptr++;
    }

    nand_deselect();
    return bytes;
}


/**
 * nand_write_page - Write the data to a page main data area 
 * @nand:       The nand flash information
 * @page_num:   The written page number
 * @offset:     The written data start offset address in the page
 * @size:       The written data length
 * @data:       The written data 
 */
int nand_write_page(struct boot_nand_t *nand, ulong page_num, ulong offset, char *data, ulong size)
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

    /* If the offset address in the page is 0 and need write a whole page data, 
     * then we read the whole page, or use the random page program */
    if(0==offset && nand->page_size==size)
            goto START_PROG;

    /*Random page program will come here*/
    REG_NFCMD = NAND_CMD_RNDIN;
    REG_NFADDR = offset & 0xff;
    REG_NFADDR = (offset >> 8) & 0xff;

START_PROG:
    for(i=0; i<bytes; i++)
    {
        REG_NFDATA = *ptr8; 
        ptr8++;
    }
    REG_NFCMD = NAND_CMD_PAGEPROG;

    /* 
     * After give NAND_CMD_PAGEPROG command, when the RnB get a rising edge signal, it 
     * will set(Read S3C2440 datasheet about NFCONT register) the NFSTAT register bit[2] 
     * in nandflash controller. After get this signal, then we can send followed command.
     */
    nand_detect_RnB();

    /* When program operation is completed, the Write Status register Bit[I/O] 0 maybe checked, 
     * bit[0]=0 means pass, or bit[0]=1 means Fail. Refer the top table definition  */
    REG_NFCMD = NAND_CMD_STATUS;
    stat = REG_NFDATA;
    if(stat & 0x01)   
    {
        dbg_print("Random Program page number @%lu failure\n", page_num);
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
 * @addr:  The offset address in whole nandflash, no need block alignment
 */
int is_bad_block(struct boot_nand_t *nand, ulong addr)
{
    char data;
    ulong page_num;
    ulong block_start;
    int  page_shift = generic_ffs(nand->page_size)-1;

    /*Get start address of the block wherre the "addr" in*/
    block_start = addr & (~(nand->block_size-1)); 

    /* Get the first page number in the block, the Bad Block Tag is set in  
     * the first bit in the first page spare area in the block */
    page_num = block_start >> page_shift;     

    nand_read_page(nand, page_num, nand->bad_block_offset, &data, 1);

    if (data != 0xff)
    {
        printf("Bad Block @%08lx by address @%08lx\n",block_start, addr);
        nand_deselect();
        return 1;
    }

    nand_deselect();
    return 0;
}

/**
 * mark_bad_block - Mark the block as bad block
 * @nand:  Nandflash information
 * @addr:  The offset address in whole nandflash, no need block alignment
 */
int mark_bad_block(struct boot_nand_t *nand, ulong addr)
{
    char data = 0xEE;
    int  page_shift = generic_ffs(nand->page_size)-1;
    ulong page_num;
    ulong block_start;
    int  ret;

    /*Get start address of the block wherre the "addr" in*/
    block_start = addr & (~(nand->block_size-1)); 

    /* Get the first page number in the block, the Bad Block Tag is set in  
     * the first bit in the first page spare area in the block */
    page_num = block_start >> page_shift;     

    ret = nand_write_page(nand, page_num, nand->bad_block_offset, &data, 1);

    printf("Mark bad Block @%08lx for address @%08lx ", block_start, addr);
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
 * mark_erase - Erase some blocks from start_addr
 * @nand:        Nandflash information
 * @start_addr:  The erase start address, it must be aligment 
 * @size:        The erase block size, it must be  n*block_size
 */
int nand_erase(struct boot_nand_t *nand, ulong start_addr, ulong size)
{
    uchar   stat;
    int     block_num, ret=0;
    ulong   addr;
    int     block_shift = generic_ffs(nand->block_size)-1;

    if ( (start_addr & (nand->block_size-1)) || (size & (nand->block_size-1)) )
    {
        printf("nand erase address @0x%08lx or size $%lu not block aligment.\n", start_addr, size);
        return -1;       /* invalid alignment */
    }

    addr=start_addr; 
    while( addr<(start_addr+size) )
    {
        /*Skip the bad block*/
        if(is_bad_block(nand, addr))
        {
             dbg_print("Erase skip bad block @0x%08lx.\n",addr);
             addr+=nand->block_size; 
             size+=nand->block_size;  
             continue;
        }

        block_num = addr>>block_shift; 

        nand_select(); 
        nand_clear_RnB();

        REG_NFCMD = NAND_CMD_ERASE1; 
        REG_NFADDR = (block_num<<6) & 0xff;      /*Row address A18~A19*/
        REG_NFADDR = (block_num >> 2) & 0xff;    /*Row address A20~A27*/
        REG_NFADDR = (block_num >> 10) & 0xff;   /*Row address A28*/
        REG_NFCMD = NAND_CMD_ERASE2; 

        delay(1000);

       /* 
        * After give NAND_CMD_ERASE2 command, when the RnB get a rising edge signal, it 
        * will set(Read S3C2440 datasheet about NFCONT register) the NFSTAT register bit[2] 
        * in nandflash controller. After get this signal, then we can send next command.
        */
        nand_detect_RnB();

        /* When erase operation is completed, the Write Status register Bit[I/O] 0 maybe checked, 
         * bit[0]=0 means pass, or bit[0]=1 means Fail. Refer the top table definition  */
        REG_NFCMD = NAND_CMD_STATUS;

        stat = REG_NFDATA;

        if( stat&(1<<0) )
        {
             printf("Erase block @0x%08lx [$%lu] failed.\n",addr,block_num);
             mark_bad_block(nand, addr);
             ret = -2;
             goto RET;
        }
        dbg_print("Erase block @0x%08lx [$%lu] success.\n",addr,block_num);
        nand_deselect();

        addr+=nand->block_size;
    }

RET:

    return ret;    
}

/**
 * nand_read -   Read some datas in the main data area in the page
 * @nand:        Nandflash information
 * @start_addr:  The read start address, it must be page alignment 
 * @size:        The request read data size, no need alignment
 * @buf:         The read data output buffer
 */
int nand_read(struct boot_nand_t *nand, ulong start_addr, ulong size, char *buf)
{
    ulong addr;
    ulong page_num;
    ulong page_offset;
    ulong page_mask = nand->page_size-1;
    ulong bytes;
    ulong left;
    int  page_shift = generic_ffs(nand->page_size)-1;

    left = size;
    addr = start_addr;

    /* The address must be page alignment*/
    if( (addr & page_mask) )
    {
          printf("nand read address @0x%08lx not alignment.\n", addr);
          return -1;  
    }

    while(left > 0)
    {
         /*If the addr is block alignment, then we check this block is valid or not*/
         if ( ((addr&(nand->block_size-1))==0) && is_bad_block(nand, addr) )   
         {
                /*Skip Bad block and goto next block */
                dbg_print("Skip bad block @0x%08lx\n", addr);
                addr += nand->block_size;
                continue;
         }

         page_num = addr >> page_shift;   /*The page number in the whole Nandflash*/
         page_offset = addr & page_mask;     /*The offset address in the page*/

         if( left >= nand->page_size)
         {
             bytes = nand_read_page(nand, page_num, page_offset, buf, nand->page_size);
             dbg_print("Read whole page: addr=%08lx page_num=%d page_offset=%08lx, bytes=%lu \n", 
                             addr, page_num, page_offset, bytes);
         }
         else
         {
             bytes = nand_read_page(nand, page_num, page_offset, buf, size%nand->page_size);
             printf("Read part page: addr=%08lx page_num=%d page_offset=%d, bytes=%lu \n", 
                             addr, page_num, page_offset, size%nand->page_size);
         }

         addr+=bytes;
         buf += bytes;
         left-=bytes;
    }

    return 0;
}

/* nand_read_spare - Read some bytes from the spare first byte
 * @page_addr:  The page address in the whole Nand flash, no need page alignment
 * @size:       The spare area request read size, which start from the spare first byte
 * @buf:        The output data buffer
 */
int nand_read_spare(struct boot_nand_t *nand, ulong page_addr, int size, char *buf)
{
    ulong page_num;
    ulong bytes;
    int  page_shift = generic_ffs(nand->page_size)-1;

    size = size > nand->spare_size ? nand->spare_size : size;

    page_num = page_addr >> page_shift;   /*The page number in the whole Nandflash*/

    bytes = nand_read_page(nand, page_num, nand->bad_block_offset, buf, size);

    return bytes;
}

/* nand_write_spare - Write some bytes to the spare first byte
 * @page_addr:  The page address in the whole Nand flash, no need page alignment
 * @size:       The write data size
 * @buf:        The write data
 */
int nand_write_spare(struct boot_nand_t *nand, ulong page_addr, int size, char *buf)
{
    ulong page_num;
    ulong bytes;
    int  page_shift = generic_ffs(nand->page_size)-1;

    size = size > nand->spare_size ? nand->spare_size : size;

    page_num = page_addr >> page_shift;   /*The page number in the whole Nandflash*/

    bytes = nand_write_page(nand, page_num, nand->bad_block_offset, buf, size);

    return bytes;
}

/**
 * nand_write -  Write some datas to the main data area in the page
 * @nand:        Nandflash information
 * @start_addr:  The write start address, it must be page alignment 
 * @size:        The write data size, no need alignment
 * @buf:         The write data 
 */
int nand_write(struct boot_nand_t *nand, ulong start_addr, ulong size, char *buf)
{
    ulong addr;
    ulong page_num;
    ulong page_offset;
    ulong page_mask = nand->page_size-1;
    ulong bytes;
    ulong left;
    int  page_shift = generic_ffs(nand->page_size)-1;

    left = size;
    addr = start_addr;

    /* The address must be page alignment*/
    if( (addr & page_mask) )
    {
          printf("nand read address @0x%08lx not alignment.\n", addr);
          return -1;  
    }

    while(left > 0)
    {
         /*If the addr is block alignment, then we check this block is valid or not*/
         if ( ((addr&(nand->block_size-1))==0) && is_bad_block(nand, addr) )   
         {
                /*Skip Bad block and goto next block */
                dbg_print("Skip bad block @0x%08lx\n", addr);
                addr += nand->block_size;
                continue;
         }

         page_num = addr >> page_shift;   /*The page number in the whole Nandflash*/
         page_offset = addr & page_mask;     /*The offset address in the page*/

         if( left >= nand->page_size)
         {
             bytes = nand_write_page(nand, page_num, page_offset, buf, nand->page_size);
             dbg_print("Write whole page: addr=%08lx page_num=%d page_offset=%08lx, bytes=%lu \n", 
                             addr, page_num, page_offset, bytes);
         }
         else
         {
             bytes = nand_write_page(nand, page_num, page_offset, buf, size%nand->page_size);
             dbg_print("Write part page: addr=%08lx page_num=%d page_offset=%d, bytes=%lu \n", 
                             addr, page_num, page_offset, size%nand->page_size);
         }

         addr+=bytes;
         buf += bytes;
         left-=bytes;
    }

    return 0;
}

