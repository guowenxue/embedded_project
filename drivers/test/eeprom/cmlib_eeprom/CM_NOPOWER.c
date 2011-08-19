// Functions control the logical power on/off for the chip

#include "CM_LIB.H"
#include "CM_I2C.H"
#include "CM_I2C_L.H"
#include "CM_GPA.H"
#include "ll_port.h"

// Power On Chip  
//
// Returns 0 (SUCCESS) if no error
//
void cm_PowerOn(void)
{
#if 0
    // Reset chip data

    print(("@%04d\n", __LINE__));

    cm_ResetCrypto();

    print(("@%04d\n", __LINE__));

    ucCM_UserZone = ucCM_AntiTearing = 0;

    // Sequence for powering on secure memory according to ATMEL spec
    print(("@%04d\n", __LINE__));
    CM_DATA_OUT;                // SDA and SCL start as outputs
    print(("@%04d\n", __LINE__));
    CM_CLK_OUT;
    print(("@%04d\n", __LINE__));
    CM_CLK_LO;                  // Clock should start LOW
    print(("@%04d\n", __LINE__));
    CM_DATA_HI;                 // Data high during reset
    print(("@%04d\n", __LINE__));
    cm_ClockCycles(CM_PWRON_CLKS);  // Give chip some clocks cycles to get started
    print(("@%04d\n", __LINE__));

    // Chip should now be in sync mode and ready to operate
#endif
    ll_PowerOn();

}

// Shut down secure memory
//
void cm_PowerOff(void)
{
#if 0
    cm_Delay(1);
    CM_CLK_LO;
    cm_Delay(6);
#endif
    ll_PowerOff();
}
