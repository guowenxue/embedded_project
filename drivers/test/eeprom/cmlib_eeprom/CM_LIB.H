// CryptoMemory Header File
//
// Detailed descriptions of all functions and structures in the header file can be found in
// the "CryptoMemory Library User Manual".
//
// Revision Date      Changes
// -------- --------- -----------------------------------------------------------------
//    0.1   26 Oct 04 First Release
//    0.2   19 Nov 04 Major additions and corrections

// Protect Library against multiple inclusion
#ifndef CM_LIB_H
#define CM_LIB_H

// -------------------------------------------------------------------------------------------------
// Definations
// -------------------------------------------------------------------------------------------------

// Basic Definations (if not available elsewhere)
#ifndef FALSE
#define FALSE (0)
#define TRUE  (!FALSE)
#endif
#ifndef NULL
#define NULL ((void *)0)
#endif

/*AT88SC0104C Security EEPROM*/

#define ATR_ADDR      (0x00)    /*Answer to Reset address, 8 bytes, can not be used */

#define FB_ADDR       (0x08)    /*Fab Code address, 2 bytes, can not be used */
#define MTZ_ADDR      (0x0A)    /*Memory Test Zone address, 2 bytes */
#define CMC_ADDR      (0x0C)    /*Card Manuafacture Code address, 4bytes, */

#define LHC_ADDR      (0x10)    /*Lot History Code address, 8 bytes, readonly can not be used */
#define INN_ADDR      (0x19)    /*Identification Number, 7 bytes, for user custom */

#define AR0_ADDR      (0x20)    /*Access Register for User zone 0, 1 bytes */
#define PR0_ADDR      (0x21)    /*Password Register for user zone 0, 1 bytes */
#define AR1_ADDR      (0x22)    /*Access Register for User zone 1, 1 bytes */
#define PR1_ADDR      (0x23)    /*Password Register for user zone 1, 1 bytes */
#define AR2_ADDR      (0x24)    /*Access Register for User zone 2, 1 bytes */
#define PR2_ADDR      (0x25)    /*Password Register for user zone 2, 1 bytes */

#define IC_ADDR       (0x40)    /*Issuer Code address, 16 bytes */

// Device Configuration Register
#define DCR_ADDR      (0x18)
#define DCR_SME       (0x80)
#define DCR_UCR       (0x40)
#define DCR_UAT       (0x20)
#define DCR_ETA       (0x10)
#define DCR_CS        (0x0F)

// Cryptograms
#define CM_Ci         (0x50)
#define CM_Sk         (0x58)
#define CM_G          (0x90)

// Fuses
#define CM_FAB        (0x06)
#define CM_CMA        (0x04)
#define CM_PER        (0x00)

// Password
#define CM_PSW        (0xB0)
#define CM_PWREAD     (1)
#define CM_PWWRITE    (0)

// Return Code Defination
#define SUCCESS                     0x00    //Everything okay
#define FAILED                      0x01    //General failure
#define FAIL_CMDSTART               0x02    //Command start failed
#define FAIL_CMDSEND                0x03    //Failed to send a command
#define FAIL_WRDATA                 0x04    //Failed to write data
#define FAIL_RDDATA                 0x05    //Failed to read data
#define UNKNOWN_DEVICE              0x06    //Invalid device number
#define PASSWORD_NOT_ACCEPTED       0x07    //write password not accepted
#define CRYPTOGRAM_NOT_ACCEPTED     0x08    //new cryptogram not accepted
#define INVALID_USER_ZONE           0x09    //wrong user zone
#define INVALID_CRYPTO_ADDR         0x0A    //wrong Crypto Address
#define INVALID_NUM_BYTES_2WRITE    0x0B    //number of bytes to write too large
#define INVALID_NUM_BYTES_2READ     0x0C    //number of bytes to read too large
// note: additional specific error codes may be added in the future

// Basic Datatypes
typedef unsigned char uchar;
typedef unsigned char *puchar;
typedef signed char schar;
typedef signed char *pschar;
/*W90P710 corss compiler has defined uint in /usr/local/arm_tools_3.3/arm-elf/include/sys/types.h*/
typedef unsigned int uint;
typedef unsigned int *puint;
typedef signed int sint;
typedef signed int *psint;

#include "ll_port.h"            /*ll_port.h depens on upside basic datatypes and return code define */

// -------------------------------------------------------------------------------------------------

// High Level Function Prototypes

// Select Chip
uchar cm_SelectChip(uchar ucChipId);

// Activate Security
uchar cm_ActiveSecurity(uchar ucChipAddr, uchar ucKeySet, puchar pucKey,
                        puchar pucRandom, uchar ucEncrypt);

// Deactivate Security
uchar cm_DeactiveSecurity(uchar ucChipAddr);

// Verify Password
uchar cm_VerifyPassword(uchar ucChipAddr, puchar pucPassword, uchar ucSet, uchar ucRW);

// Reset Password
uchar cm_ResetPassword(uchar ucChipAddr);

// Verify Secure Code
#define cm_VerifySecureCode(ucChipAddr, CM_PW) cm_VerifyPassword(ucChipAddr, CM_PW, 7, CM_PWWRITE)

// Read Configuration Zone
uchar cm_ReadConfigZone(uchar ucChipAddr, uchar ucCryptoAddr, puchar pucBuffer, uchar ucCount);

// Write Configuration Zone
uchar cm_WriteConfigZone(uchar ucChipAddr, uchar ucCryptoAddr, puchar pucBuffer,
                         uchar ucCount, uchar ucAntiTearing);

// Set User Zone
uchar cm_SetUserZone(uchar ucChipAddr, uchar ucZoneNumber, uchar ucAntiTearing);

// Read User Zone
uchar cm_ReadLargeZone(uchar ucChipAddr, uint uiCryptoAddr, puchar pucBuffer, uchar ucCount);

// Read Small User Zone
uchar cm_ReadSmallZone(uchar ucChipAddr, uchar ucCryptoAddr, puchar pucBuffer, uchar ucCount);

// Write User Zone
char cm_WriteLargeZone(uchar ucChipAddr, uint uiCryptoAddr, puchar pucBuffer, uchar ucCount);

// Write Small User Zone
uchar cm_WriteSmallZone(uchar ucChipAddr, uchar ucCryptoAddr, puchar pucBuffer, uchar ucCount);

// Send Checksum
uchar cm_SendChecksum(uchar ucChipAddr, puchar pucChkSum);

// Read Checksum
uchar cm_ReadChecksum(uchar ucChipAddr, puchar pucChkSum);

// Read Fuse Byte
uchar cm_ReadFuse(uchar ucChipAddr, puchar pucFuze);

// Burn Fuse
uchar cm_BurnFuse(uchar ucChipAddr, uchar ucFuze);

// -------------------------------------------------------------------------------------------------
// Configuration Structures
// -------------------------------------------------------------------------------------------------

// CryptoMemory Low Level Linkage
// 
typedef struct
{
    uchar(*Carddetect) (void);
    void (*PowerOff) (void);
    void (*PowerOn) (void);
     uchar(*SendCommand) (puchar pucCommandBuffer);
     uchar(*ReceiveRet) (puchar pucReceiveData, uchar ucLength);
     uchar(*SendData) (puchar pucSendData, uchar ucLength);
    void (*RandomGen) (puchar pucRandomData);
    void (*WaitClock) (uchar ucLoop);
     uchar(*SendCmdByte) (uchar ucCommand);
} cm_low_level;

// CryptoMemory Low Level Configuration
// 
// If any of the supplied CryptoMemory low level library functions are used, this structure must be
// present in the user code. For a detailed description of the elements in the structure, please
// see the "CryptoMemory Library User Manual".
// 
typedef struct
{
    uchar ucChipSelect;
    uchar ucClockPort;
    uchar ucClockPin;
    uchar ucDataPort;
    uchar ucDataPin;
    uchar ucCardSensePort;
    uchar ucCardSensePin;
    uchar ucCardSensePolarity;
    uchar ucPowerPort;
    uchar ucPowerPin;
    uchar ucPowerPolarity;
    uchar ucDelayCount;
    uchar ucStartTries;
} cm_port_cfg;

// -------------------------------------------------------------------------------------------------
// Externals for Configuration Structures
// -------------------------------------------------------------------------------------------------

extern cm_low_level CM_LOW_LEVEL;
extern cm_port_cfg CM_PORT_CFG;

// -------------------------------------------------------------------------------------------------
// Other Externals
// -------------------------------------------------------------------------------------------------

extern uchar ucCM_Encrypt;
extern uchar ucCM_Authenticate;
extern uchar ucCM_UserZone;
extern uchar ucCM_AntiTearing;
extern uchar ucCM_InsBuff[4];

// end of multiple inclusion protection
#endif
