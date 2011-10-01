// Read User Zone
//
// The Read Large User Zone function is used to read data from CryptoMemory devices
// that have greater than 256 bytes in each user zone (AT88SC6416C, and larger)

// CryptoMemory Library Include Files
#include "CM_LIB.H"
#include "CM_I2C.H"
#include "CM_I2C_L.H"
#include "CM_GPA.H"

// Read User Zone
uchar cm_ReadLargeZone(uchar ucChipAddr, uint uiCryptoAddr, puchar pucBuffer, uchar ucCount)
{
    uchar ucReturn;

    // ucCM_InsBuff[0] = 0xb2;
    ucCM_InsBuff[0] = (0x2 | ((ucChipAddr & 0xf) << 4));
    ucCM_InsBuff[1] = (uchar) (uiCryptoAddr >> 8);
    ucCM_InsBuff[2] = (uchar) uiCryptoAddr;
    ucCM_InsBuff[3] = ucCount;

    // Three bytes of the command must be included in the polynominals
    cm_GPAcmd3(ucCM_InsBuff);

    // Read the data
    if ((ucReturn = cm_ReadCommand(ucCM_InsBuff, pucBuffer, ucCount)) != SUCCESS)
        return ucReturn;

    // Include the data in the polynominals and decrypt if required
    cm_GPAdecrypt(ucCM_Encrypt, pucBuffer, ucCount);

    return SUCCESS;
}
