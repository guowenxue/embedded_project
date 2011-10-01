// Write Configuration Zone
//

// CryptoMemory Library Include Files
#include "CM_LIB.H"
#include "CM_I2C.H"
#include "CM_I2C_L.H"
#include "CM_GPA.H"

// Write Configuration Zone
uchar cm_WriteConfigZone(uchar ucChipAddr, uchar ucCryptoAddr, puchar pucBuffer,
                         uchar ucCount, uchar ucAntiTearing)
{
    uchar ucReturn, ucEncrypt;

    // ucCM_InsBuff[0] = 0xb4;
    ucCM_InsBuff[0] = (0x4 | ((ucChipAddr & 0xf) << 4));
    if (ucAntiTearing)
        ucCM_InsBuff[1] = 0x08;
    else
        ucCM_InsBuff[1] = 0x00;
    ucCM_InsBuff[2] = ucCryptoAddr;
    ucCM_InsBuff[3] = ucCount;

    // Three bytes of the command must be included in the polynominals
    cm_GPAcmd2(ucCM_InsBuff);

    // Only password zone is ever encrypted
    ucEncrypt = ((ucCryptoAddr >= CM_PSW) && ucCM_Encrypt);

    // Include the data in the polynominals and encrypt if required
    cm_GPAencrypt(ucEncrypt, pucBuffer, ucCount);

    // Do the write
    ucReturn = cm_WriteCommand(ucCM_InsBuff, pucBuffer, ucCount);

    // when anti-tearing, the host should send ACK should >= 20ms after write command
    if (ucAntiTearing)
        CM_LOW_LEVEL.WaitClock(10);

    return ucReturn;
}
