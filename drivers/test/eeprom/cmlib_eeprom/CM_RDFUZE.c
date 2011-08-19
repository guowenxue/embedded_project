// Read Fuze Byte
//

// CryptoMemory Library Include Files
#include "CM_LIB.H"
#include "CM_I2C.H"
#include "CM_I2C_L.H"
#include "CM_GPA.H"

uchar ucCmdRdFuze[4] = { 0xb6, 0x01, 0x00, 0x01 };

// Read Fuse Byte
uchar cm_ReadFuse(uchar ucChipAddr, puchar pucFuze)
{
    uchar ucReturn;

    ucCmdRdFuze[0] = (0x6 | ((ucChipAddr & 0xf) << 4));
    // 5 0x00, A2 (0x00), 5 0x00, N (0x01)    
    cm_GPAGenNF(11, 0x01);

    if ((ucReturn = cm_ReadCommand(ucCmdRdFuze, pucFuze, 1)) != SUCCESS)
        return ucReturn;

    cm_GPAGen(*pucFuze);        // fuze byte
    cm_GPAGenN(5);              // 5 0x00s

    return SUCCESS;

}
