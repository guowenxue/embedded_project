// Reset Password
//

// CryptoMemory Library Include Files
#include "CM_LIB.H"
#include "CM_I2C.H"
#include "CM_I2C_L.H"

// Reset Password
//uchar cm_ResetPassword(void)
uchar cm_ResetPassword(uchar ucChipAddr)
{
    uchar cmdByte = (0xa | ((ucChipAddr & 0xf) << 4));

    //return CM_LOW_LEVEL.SendCmdByte(0xba);
    return CM_LOW_LEVEL.SendCmdByte(cmdByte);
}
