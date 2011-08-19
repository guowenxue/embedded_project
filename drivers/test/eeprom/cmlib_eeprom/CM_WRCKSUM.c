// Write Checksum
//

// CryptoMemory Library Include Files
#include "CM_LIB.H"
#include "CM_I2C.H"
#include "CM_I2C_L.H"
#include "CM_GPA.H"

uchar ucCmdWrChk[4] = { 0xb4, 0x02, 0x00, 0x02 };

// Send Checksum
uchar cm_SendChecksum(uchar ucChipAddr, puchar pucChkSum)
{
    uchar ucReturn;
    uchar ucChkSum[2];

    ucCmdWrChk[0] = (0x4 | ((ucChipAddr & 0xf) << 4));
    // Get Checksum if required
    if (pucChkSum == NULL)
        cm_CalChecksum(ucChkSum);
    else
    {
        ucChkSum[0] = *pucChkSum++;
        ucChkSum[1] = *pucChkSum;
    }

    // Send the command
    ucReturn = cm_WriteCommand(ucCmdWrChk, ucChkSum, 2);

    // Give the CyrptoMemory some processing time
    CM_LOW_LEVEL.WaitClock(5);

    // Done
    return ucReturn;
}
