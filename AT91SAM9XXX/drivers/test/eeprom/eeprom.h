#ifndef __QC_EEPROM_H
#define __QC_EEPROM_H

#include "CM_LIB.H"

#define DEF_CHIP_ADDR         0xB   /*Default EEPROM chip address on I2C bus */
#define EEPROM_CARD           0x00
#define EEPROM_BACK           0x01

#define USERZONE0             0
#define USERZONE1             1
#define USERZONE2             2
#define USERZONE3             3

/* Configure Zone area usage */
#define SN_ADDR               0x40  /*Series number saved in "Secure Issuer Code" area */
#define SN_LEN                16    /*Series number takes up 16 bytes */
#define MAC0_ADDR             0x19  /*MAC0 saved in "Identification Number" area */
#ifndef MAC_LEN
#define MAC_LEN               6 /*MAC0 takes up 6 bytes */
#endif
#define CHST_TYPE_ADDR        0x0C  /*N300 chest nubmer or Card type saved in "Card Manufacture Code" area */
#define CHST_TYPE_LEN         1 /*It just takes up 1 byte */

/* User Zone 0 area usage */
#define MNFCT_ADDR            0x00  /*Manufacture string saved start from 0x00 in User Zone 0 */
#define MNFCT_LEN             20    /*Manufacture string takes up 20 bytes, "GHL SYSTEMS BERHAD" */
#define MAC1_ADDR             0x18  /*MAC1(only on L3/N3) address */

/* User Zone 1 area usage */
#define PRD_TYPE_ADDR        0x00   /*Production type saved start from 0x00 in user zone 1 */
#define PRD_TYPE_LEN         15 /*Production type string take up 15 bytes, "L-Series" */
#define PRD_MODEL_ADDR       0x10   /*Production model saved start from 0x10 in user zone 1 */
#define PRD_MODEL_LEN        10 /*Production model string take up 10 bytes, "L-200" */

/* User Zone 2 area usage */
#define CERT1_ADDR           0x00   /*Certification total has 48 bytes, part 1 will take up whole user zone 2 */
#define CERT1_LEN            32 /*Certification part 1 takes whole user zone 2 */

/* User Zone 3 area usage */
#define CERT2_ADDR           0x00   /*Certification total has 48 bytes, part 2 saved from 0x00 in user zone 3 */
#define CERT2_LEN            16 /*Certification part 2 take 16 bytes in user zone 3 */
#define QCFLAG_ADDR          0x10   /*QC flag just take 1 byte and saved in 0x10 address in user zone 3 */
#define QCFLAG_LEN           1

#define QCPASS_FLAG          0xFE
#define QCFAIL_FLAG          0x00

/*Shouldn't call it directly*/
unsigned char eeprom_init(unsigned char ucWhich);
unsigned char test_MTZ(unsigned char ucWhich);
void eeprom_term(void);

/*Export out*/
unsigned char write_eeprom_qcflag(unsigned char ucWhich, unsigned char ucFlag);
unsigned char read_eeprom_qcflag(unsigned char ucWhich);
//unsigned char test_eeprom(void);

#endif
