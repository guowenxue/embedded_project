// Data and Functions used by other low level functions
//
// Note: this module must be after all other low level functions in the library
//       to assure that any reference to functions in this library are satistified.

#include "CM_LIB.H"
#include "CM_I2C.H"
#include "CM_I2C_L.H"

// Zone Data
uchar ucCM_UserZone;
uchar ucCM_AntiTearing;

// Chip state
uchar ucCM_Encrypt;
uchar ucCM_Authenticate;

// Global data
uchar ucCM_InsBuff[4];

// Delay
void cm_Delay(uchar ucDelay)
{
    uchar ucTimer;

    while (ucDelay)
    {
        ucTimer = CM_TIMER;
        while (ucTimer)
            ucTimer--;
        ucDelay--;
    }
}
