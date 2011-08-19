// Read Small User Zone
//
// The Read Small User Zone function is used to read data from CryptoMemory devices that
// have 256 bytes or less in each user zone (AT88SC3216C, and smaller)

// CryptoMemory Library Include Files
#include "CM_LIB.H"
#include "CM_I2C.H"
#include "CM_I2C_L.H"
#include "CM_GPA.H"

// Read Small User Zone
uchar cm_ReadSmallZone(uchar ucChipAddr, uchar ucCryptoAddr, puchar pucBuffer, uchar ucCount)
{
    uchar ucReturn;

    //ucCM_InsBuff[0] = 0xb2;
    ucCM_InsBuff[0] = (0x2 | ((ucChipAddr & 0xf) << 4));
    ucCM_InsBuff[1] = 0;
    ucCM_InsBuff[2] = ucCryptoAddr;
    ucCM_InsBuff[3] = ucCount;

    // Two bytes of the command must be included in the polynominals
    cm_GPAcmd2(ucCM_InsBuff);

    // Read the data
    if ((ucReturn = cm_ReadCommand(ucCM_InsBuff, pucBuffer, ucCount)) != SUCCESS)
        return ucReturn;

    // Include the data in the polynominals and decrypt it required
    cm_GPAdecrypt(ucCM_Encrypt, pucBuffer, ucCount);

    // Done
    return SUCCESS;
}
