// Read Checksum
//

// CryptoMemory Library Include Files
#include "CM_LIB.H"
#include "CM_I2C.H"
#include "CM_I2C_L.H"
#include "CM_GPA.H"

uchar ucCmdRdChk[4] = { 0xb6, 0x02, 0x00, 0x02 };

// Read Checksum
uchar cm_ReadChecksum(uchar ucChipAddr, puchar pucChkSum)
{
    uchar ucDCR[1];
    uchar ucReturn;

    ucCmdRdChk[0] = (0x6 | ((ucChipAddr & 0xf) << 4));
    // 20 0x00s (10 0x00s, ignore first byte, 5 0x00s, ignore second byte, 5 0x00s  
    cm_GPAGenN(20);

    // Read the checksum                  
    if ((ucReturn = cm_ReadCommand(ucCmdRdChk, pucChkSum, 2)) != SUCCESS)
        return ucReturn;

    // Check if unlimited reads allowed
    if ((ucReturn = cm_ReadConfigZone(ucChipAddr, DCR_ADDR, ucDCR, 1)) != SUCCESS)
        return ucReturn;
    if ((ucDCR[0] & DCR_UCR))
        cm_ResetCrypto();

    return SUCCESS;
}
