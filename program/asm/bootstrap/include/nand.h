/*
 *  linux/include/linux/mtd/nand.h
 *
 *  Copyright (c) 2000 David Woodhouse <dwmw2@infradead.org>
 *                     Steven J. Hill <sjhill@realitydiluted.com>
 *		       Thomas Gleixner <tglx@linutronix.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Info:
 *	Contains standard defines and IDs for NAND flash devices
 *
 * Changelog:
 *	See git changelog.
 */
#ifndef __LINUX_MTD_NAND_H
#define __LINUX_MTD_NAND_H

#include <common.h>
#include <linux/bitops.h>

#define NAND_K9F2G08      0xecda

/* This constant declares the max. oobsize / page, which
 * is supported now. If you add a chip with bigger oobsize/page
 * adjust this accordingly.
 */
#define NAND_MAX_OOBSIZE	218
#define NAND_MAX_PAGESIZE	4096

/*
 * Constants for hardware specific CLE/ALE/NCE function
 *
 * These are bits which can be or'ed to set/clear multiple
 * bits in one go.
 */
/* Select the chip by setting nCE to low */
#define NAND_NCE		0x01
/* Select the command latch by setting CLE to high */
#define NAND_CLE		0x02
/* Select the address latch by setting ALE to high */
#define NAND_ALE		0x04

#define NAND_CTRL_CLE		(NAND_NCE | NAND_CLE)
#define NAND_CTRL_ALE		(NAND_NCE | NAND_ALE)
#define NAND_CTRL_CHANGE	0x80

/*
 * Standard NAND flash commands
 */
#define NAND_CMD_READ0		0x00        /*Page read period 1*/
#define NAND_CMD_READSTART	0x30        /*Page read period 2, for large page*/
#define NAND_CMD_READID		0x90        /*Read production ID*/
#define NAND_CMD_SEQIN		0x80        /*Page write period 1*/
#define NAND_CMD_PAGEPROG	0x10        /*Page write period 2*/
#define NAND_CMD_ERASE1		0x60        /*Block erase period 1*/
#define NAND_CMD_ERASE2		0xd0        /*Block erase period 2*/
#define NAND_CMD_STATUS		0x70        /*Read status command*/
#define NAND_CMD_RESET		0xff        /*Nandflash reset command*/
#define NAND_CMD_READ1		1
#define NAND_CMD_RNDOUT		0x05        /*Page random read period 1*/
#define NAND_CMD_RNDOUTSTART	0xE0    /*Page random read period 2*/
#define NAND_CMD_RNDIN		0x85        /*Page random write*/

#define NAND_CMD_READOOB	0x50
#define NAND_CMD_STATUS_MULTI	0x71

/* Extended commands for large page devices */
#define NAND_CMD_CACHEDPROG	0x15

/* Extended commands for AG-AND device */
/*
 * Note: the command for NAND_CMD_DEPLETE1 is really 0x00 but
 *       there is no way to distinguish that from NAND_CMD_READ0
 *       until the remaining sequence of commands has been completed
 *       so add a high order bit and mask it off in the command.
 */
#define NAND_CMD_DEPLETE1	0x100
#define NAND_CMD_DEPLETE2	0x38
#define NAND_CMD_STATUS_MULTI	0x71
#define NAND_CMD_STATUS_ERROR	0x72
/* multi-bank error status (banks 0-3) */
#define NAND_CMD_STATUS_ERROR0	0x73
#define NAND_CMD_STATUS_ERROR1	0x74
#define NAND_CMD_STATUS_ERROR2	0x75
#define NAND_CMD_STATUS_ERROR3	0x76
#define NAND_CMD_STATUS_RESET	0x7f
#define NAND_CMD_STATUS_CLEAR	0xff

#define NAND_CMD_NONE		-1

/* Status bits */
#define NAND_STATUS_FAIL	0x01
#define NAND_STATUS_FAIL_N1	0x02
#define NAND_STATUS_TRUE_READY	0x20
#define NAND_STATUS_READY	0x40
#define NAND_STATUS_WP		0x80

/*
 * Constants for ECC_MODES
 */
typedef enum
{
    NAND_ECC_NONE,
    NAND_ECC_SOFT,
    NAND_ECC_HW,
    NAND_ECC_HW_SYNDROME,
    NAND_ECC_HW_OOB_FIRST,
} nand_ecc_modes_t;

/*
 * Constants for Hardware ECC
 */
/* Reset Hardware ECC for read */
#define NAND_ECC_READ		0
/* Reset Hardware ECC for write */
#define NAND_ECC_WRITE		1
/* Enable Hardware ECC before syndrom is read back from flash */
#define NAND_ECC_READSYN	2

/* Bit mask for flags passed to do_nand_read_ecc */
#define NAND_GET_DEVICE		0x80

/* Option constants for bizarre disfunctionality and real
*  features
*/
/* Chip can not auto increment pages */
#define NAND_NO_AUTOINCR	0x00000001
/* Buswitdh is 16 bit */
#define NAND_BUSWIDTH_16	0x00000002
/* Device supports partial programming without padding */
#define NAND_NO_PADDING		0x00000004
/* Chip has cache program function */
#define NAND_CACHEPRG		0x00000008
/* Chip has copy back function */
#define NAND_COPYBACK		0x00000010
/* AND Chip which has 4 banks and a confusing page / block
 * assignment. See Renesas datasheet for further information */
#define NAND_IS_AND		0x00000020
/* Chip has a array of 4 pages which can be read without
 * additional ready /busy waits */
#define NAND_4PAGE_ARRAY	0x00000040
/* Chip requires that BBT is periodically rewritten to prevent
 * bits from adjacent blocks from 'leaking' in altering data.
 * This happens with the Renesas AG-AND chips, possibly others.  */
#define BBT_AUTO_REFRESH	0x00000080
/* Chip does not require ready check on read. True
 * for all large page devices, as they do not support
 * autoincrement.*/
#define NAND_NO_READRDY		0x00000100
/* Chip does not allow subpage writes */
#define NAND_NO_SUBPAGE_WRITE	0x00000200

/* Options valid for Samsung large page devices */
#define NAND_SAMSUNG_LP_OPTIONS \
	(NAND_NO_PADDING | NAND_CACHEPRG | NAND_COPYBACK)

/* Macros to identify the above */
#define NAND_CANAUTOINCR(chip) (!(chip->options & NAND_NO_AUTOINCR))
#define NAND_MUST_PAD(chip) (!(chip->options & NAND_NO_PADDING))
#define NAND_HAS_CACHEPROG(chip) ((chip->options & NAND_CACHEPRG))
#define NAND_HAS_COPYBACK(chip) ((chip->options & NAND_COPYBACK))
/* Large page NAND with SOFT_ECC should support subpage reads */
#define NAND_SUBPAGE_READ(chip) ((chip->ecc.mode == NAND_ECC_SOFT) \
					&& (chip->page_shift > 9))

/* Mask to zero out the chip options, which come from the id table */
#define NAND_CHIPOPTIONS_MSK	(0x0000ffff & ~NAND_NO_AUTOINCR)

/* Non chip related options */
/* Use a flash based bad block table. This option is passed to the
 * default bad block table function. */
#define NAND_USE_FLASH_BBT	0x00010000
/* This option skips the bbt scan during initialization. */
#define NAND_SKIP_BBTSCAN	0x00020000
/* This option is defined if the board driver allocates its own buffers
   (e.g. because it needs them DMA-coherent */
#define NAND_OWN_BUFFERS	0x00040000
/* Options set by nand scan */
/* bbt has already been read */
#define NAND_BBT_SCANNED	0x40000000
/* Nand scan has allocated controller struct */
#define NAND_CONTROLLER_ALLOC	0x80000000

/* Cell info constants */
#define NAND_CI_CHIPNR_MSK	0x03
#define NAND_CI_CELLTYPE_MSK	0x0C

/*
 * NAND Flash Manufacturer ID Codes
 */
#define NAND_MFR_TOSHIBA	0x98
#define NAND_MFR_SAMSUNG	0xec
#define NAND_MFR_FUJITSU	0x04
#define NAND_MFR_NATIONAL	0x8f
#define NAND_MFR_RENESAS	0x07
#define NAND_MFR_STMICRO	0x20
#define NAND_MFR_HYNIX		0xad
#define NAND_MFR_MICRON		0x2c
#define NAND_MFR_AMD		0x01

/*
* Constants for oob configuration
*/
#define NAND_SMALL_BADBLOCK_POS		5
#define NAND_LARGE_BADBLOCK_POS		0


#if defined(CONFIG_S3C2410)
#define REG_NFCONF      __REGi(ELFIN_NAND_BASE + 0x0)
#define REG_NFCMD       __REGb(ELFIN_NAND_BASE + 0x4)
#define REG_NFADDR      __REGb(ELFIN_NAND_BASE + 0x8)
#define REG_NFDATA      __REGb(ELFIN_NAND_BASE + 0xc)
#define REG_NFSTAT      __REGb(ELFIN_NAND_BASE + 0x10)
#define NFSTAT_BUSY 1
#define nand_select()   (REG_NFCONF &= ~0x800)   /*Clear bit 11*/
#define nand_deselect() (REG_NFCONF |= 0x800)
#define nand_clear_RnB()    do {} while (0)  
#define nand_detect_RnB()    do {} while (0)  
#define nand_wait_RnB()    do {} while (0)  

#elif defined(CONFIG_S3C2440) || defined(CONFIG_S3C2442)

#define REG_NFCONF      __REGi(ELFIN_NAND_BASE + 0x0)
#define REG_NFCONT      __REGi(ELFIN_NAND_BASE + 0x4)
#define REG_NFCMD       __REGb(ELFIN_NAND_BASE + 0x8)
#define REG_NFADDR      __REGb(ELFIN_NAND_BASE + 0xc)
#define REG_NFDATA      __REGb(ELFIN_NAND_BASE + 0x10)
#define REG_NFDATA16    __REGw(ELFIN_NAND_BASE + 0x10)
#define REG_NFSTAT      __REGb(ELFIN_NAND_BASE + 0x20)
#define NFSTAT_BUSY 1
#define nand_select()   (REG_NFCONT &= ~(1 << 1))
#define nand_deselect() (REG_NFCONT |= (1 << 1))
#define nand_clear_RnB()    (REG_NFSTAT |= (1 << 2))
#define nand_detect_RnB()   {while(!(REG_NFSTAT&(1<<2)));}   /*Wait Read & Busy signal goto high(not busy)*/ 
#define nand_wait_RnB()   {while(!(REG_NFSTAT&(1<<0)));}   /*Wait nand flash not busy, wait bit[0] goes to 1*/ 

#endif

#define SKIP_BAD_BLOCK       1
#define NOT_SKIP_BAD_BLOCK   0

struct boot_nand_t
{
    int page_size; 
    int spare_size; 
    int block_size; 
    int block_num;
    int bad_block_offset; 
    int id;
    //  unsigned long size;
};

int nand_init(struct boot_nand_t *nand);
int nand_read_page(struct boot_nand_t *nand, ulong page_num, ulong offset, char *data, ulong size);
int nand_write_page(struct boot_nand_t *nand, ulong page_num, ulong offset, char *data, ulong size);
int is_bad_block(struct boot_nand_t *nand, ulong addr);
int mark_bad_block(struct boot_nand_t *nand, ulong addr);
int nand_erase_block(struct boot_nand_t *nand, ulong block_num);
int nand_erase(struct boot_nand_t *nand, ulong start_addr, ulong size, int skip_bad);
void nand_scrub(struct boot_nand_t *nand);
int nand_read_spare(struct boot_nand_t *nand, ulong page_addr, int size, char *buf);
int nand_write_spare(struct boot_nand_t *nand, ulong page_addr, int size, char *buf);
int nand_read(struct boot_nand_t *nand, ulong start_addr, ulong size, char *buf);
int nand_write(struct boot_nand_t *nand, ulong start_addr, ulong size, char *buf);



#endif                          /* __LINUX_MTD_NAND_H */
