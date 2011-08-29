// Write Small User Zone
//
// The Write Small User Zone function is used to write data to CryptoMemory devices that have
// 256 bytes or less in each user zone (AT88SC3216C, and smaller)

// CryptoMemory Library Include Files
#include "CM_LIB.H"
#include "CM_I2C.H"
#include "CM_I2C_L.H"
#include "CM_GPA.H"

// Write Small User Zone
uchar cm_WriteSmallZone(uchar ucChipAddr, uchar ucCryptoAddr, puchar pucBuffer, uchar ucCount)
{
    uchar ucReturn;

    //ucCM_InsBuff[0] = 0xb0;
    ucCM_InsBuff[0] = ((ucChipAddr & 0xf) << 4);
    ucCM_InsBuff[1] = 0x00;
    ucCM_InsBuff[2] = ucCryptoAddr;
    ucCM_InsBuff[3] = ucCount;

    // Two bytes of the command must be included in the polynominals
    cm_GPAcmd2(ucCM_InsBuff);

    // Include the data in the polynominals and encrypt it required
    cm_GPAencrypt(ucCM_Encrypt, pucBuffer, ucCount);

    // Write the data
    ucReturn = cm_WriteCommand(ucCM_InsBuff, pucBuffer, ucCount);

    // when anti-tearing, the host should send ACK should >= 20ms after write command
    if (ucCM_AntiTearing)
        CM_LOW_LEVEL.WaitClock(10);

    return ucReturn;
}
