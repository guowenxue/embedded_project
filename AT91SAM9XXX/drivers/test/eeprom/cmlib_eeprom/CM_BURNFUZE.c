// Burn Fuze
//

// CryptoMemory Library Include Files
#include "CM_LIB.H"
#include "CM_I2C.H"
#include "CM_I2C_L.H"

uchar ucCmdWrFuze[4] = { 0xb4, 0x01, 0x00, 0x00 };

// Burn Fuse
uchar cm_BurnFuse(uchar ucChipAddr, uchar ucFuze)
{
    uchar ucReturn;

    ucCmdWrFuze[0] = (0x4 | ((ucChipAddr & 0xf) << 4));
    // Burn Fuze
    ucCmdWrFuze[2] = ucFuze;
    if ((ucReturn = CM_LOW_LEVEL.SendCommand(ucCmdWrFuze)) != SUCCESS)
        return ucReturn;

    // done   
    return CM_LOW_LEVEL.ReceiveRet(NULL, 0);
}
