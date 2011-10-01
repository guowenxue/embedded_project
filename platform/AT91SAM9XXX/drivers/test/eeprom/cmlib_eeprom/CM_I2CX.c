// Functions that directly control the hardware that are not needed in all cases

#include "CM_LIB.H"
#include "CM_I2C.H"
#include "CM_I2C_L.H"

// Send a command byte
//
uchar cm_SendCmdByte(uchar ucCommand)
{
    uchar i, ucCmd;

    i = CM_START_TRIES;

    ucCmd = (ucCommand & 0x0F) | CM_PORT_CFG.ucChipSelect;
    while (i)
    {
        // cm_Start();
        ll_Start();
        //  if (cm_Write(ucCmd) == 0) break;
        if (ll_Write(ucCmd) == 0)
            break;
        if (--i == 0)
            return FAIL_CMDSTART;
    }

    return SUCCESS;
}
