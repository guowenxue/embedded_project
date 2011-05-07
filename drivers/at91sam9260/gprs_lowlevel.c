/*********************************************************************************
 *  Copyright(c)  2011, GuoWenxue<guowenxue@gmail.com>
 *  All ringhts reserved.
 *
 *     Filename:  gprs.c
 *  Description:  GPRS driver low level functions, which used control the GPIO port
 *
 *     ChangLog:
 *      1,   Version: 1.0.0
 *              Date: 2011-05-02
 *            Author: guowenxue <guowenxue@gmail.com>
 *       Descrtipion: Initial first version
 *
 ********************************************************************************/

#include "dev_gprs.h"

GSM_DEVICE support_gprs[] = {
    [GSM_GTM900B] = {
                     .name = "GTM900B",
                     .id = GSM_GTM900B,
                     /*10~100 ms specified in user manul. */
                     .poweron_period_time = 100,
                     /*Experience value on L200, datasheet is wrong */
                     .atcmd_active_time = 6000,
                     /*About 2~3 seconds specified in user manul, 
                      * or can send AT command "AT%MSO" to shutdown, but without logout network*/
                     .poweroff_period_time = 3000,
                     /*Experience value on L200, datasheet is wrong */
                     .atcmd_inactive_time = 10000,
                     .ring_call_time = 10,
                     .ring_sms_time = 10,
                     }
    ,
    [GSM_GTM900C] = {
                     .name = "GTM900C",
                     .id = GSM_GTM900C,
                     /*At least 50 ms specified in user manul. */
                     .poweron_period_time = 70,
                     /*Experience value on L200, datasheet is wrong */
                     .atcmd_active_time = 6000,
                     /*At least 50 ms specified in user manul, 
                      * or can send AT command "AT%MSO" to shutdown, but without logout network*/
                     .poweroff_period_time = 50,
                     .atcmd_inactive_time = 5000,
                     .ring_call_time = 10,
                     .ring_sms_time = 10,
                     }
    ,
    [GSM_UC864E] = {            /*Refer to UC864E/G Software User Guide Rev.2 - 22/10/08 */
                    .name = "UC864E",
                    .id = GSM_UC864E,
                    /*Turn GPRS_PWR pin to low level for at least 1 second, Page 14 */
                    .poweron_period_time = 1500,
                    /* Doesn't specified in the datasheet, experience value */
                    .atcmd_active_time = 4000,
                    /* Turn GPRS_PWR pin to low level for at least 2 seconds, Page 16, 
                     * or can send AT command "AT#SHDN" to shutdown */
                    .poweroff_period_time = 3000,
                    .atcmd_inactive_time = 5000,
                    .ring_call_time = 10,
                    .ring_sms_time = 10,
                    }
    ,
    [GSM_SIM5215] = {           /*Refer to "Hardware Design SIM5215_HD_V1.01" 15.01.2010 */
                     .name = "SIM5215",
                     .id = GSM_SIM5215,
                     /*Turn GPRS_PWR pin to low level >=64 ms, Page 22 */
                     .poweron_period_time = 100,
                     /*Reference Page 22, AT command can be set 2~3 after SIM5215 is power on */
                     .atcmd_active_time = 3000,
                     /* Turn GPRS_PWR pin to low level about 2 second, 64 ms in datasheet Page 22 
                      * is wrong. or can send AT command "AT+CPOF" to shutdown */
                     .poweroff_period_time = 2000,
                     .atcmd_inactive_time = 4000,
                     .ring_call_time = 10,
                     .ring_sms_time = 10,
                     }
    ,
};

int dev_count = ARRAY_SIZE(support_gprs);

void init_gprs_pin(void)
{
    static int  initilized = 0;

    if(initilized)
         return; 

    printk("Initilize GPRS module GPIO pin now.\n");

    at91_set_A_periph    (GPRS_CTS_PIN, DISPULLUP);
    at91_set_A_periph    (GPRS_RTS_PIN, DISPULLUP);
    at91_set_A_periph    (GPRS_RXD_PIN, DISPULLUP);
    at91_set_A_periph    (GPRS_TXD_PIN, ENPULLUP);

    at91_set_gpio_input  (GPRS_DSR_PIN, DISPULLUP);
    at91_set_gpio_input  (GPRS_RI_PIN,  DISPULLUP);
    at91_set_gpio_input  (GPRS_DCD_PIN, DISPULLUP);
    at91_set_gpio_output (GPRS_DTR_PIN, LOWLEVEL);

    at91_set_gpio_output (GPRS_POWER_PIN, HIGHLEVEL);
    at91_set_gpio_input  (GPRS_CHK_SIM_PIN, DISPULLUP);

    /*To compatbile with L350*/
    at91_set_gpio_output (GPRS_SELECT_SIM_PIN, HIGHLEVEL);  // Default set to SIM1
    at91_set_gpio_input  (GPRS_CHK_SIM2_PIN, DISPULLUP);
    at91_set_gpio_output (GPRS_RESET_PIN, HIGHLEVEL);

    initilized = 1;
}



#if 1 /*To compatable with L350*/

static inline void vbus_on(void)
{
    at91_set_gpio_output (GPRS_VBUS_CTRL_PIN, HIGHLEVEL);
    at91_set_gpio_value (GPRS_VBUS_CTRL_PIN, HIGHLEVEL);    
}

static inline void vbus_off(void)
{
    at91_set_gpio_output (GPRS_VBUS_CTRL_PIN, LOWLEVEL);
    at91_set_gpio_value (GPRS_VBUS_CTRL_PIN, LOWLEVEL);    
}

/* Return 1: HIGHLEVEL means UC864E already power on
 * Return 0: LOWLEVEL  means UC864E is power down*/
static inline int uc864e_power_mon (void)
{
    return LOWLEVEL!=at91_get_gpio_value (GPRS_POWER_MON_PIN);
}

int gprs_reset(int which)
{
    if(GSM_UC864E!=which && GSM_SIM5215!=which) 
    {
        dbg_print("GPRS reset doesn't support %s module.\n", support_gprs[which].name);
        return -1;
    }

    dbg_print("Reset %s module.\n", support_gprs[which].name);

    at91_set_gpio_value (GPRS_RESET_PIN, LOWLEVEL);
    msleep(220);
    at91_set_gpio_value (GPRS_RESET_PIN, HIGHLEVEL);

    return 0;
}

int gprs_get_worksim(void)
{
    int       worksim;

    worksim = (LOWLEVEL==at91_get_gpio_value (GPRS_SELECT_SIM_PIN) ? SIM2 : SIM1);
    dbg_print("Current working SIM slot %d\n", worksim);

    return worksim;    
}


int gprs_set_worksim(int sim)
{
    int level;
    if (SIM1 != sim && SIM2 != sim)
    {
        printk("ERROR: Set unknow SIM%d to work.\n", sim);
        return -1;
    }
    dbg_print("Set SIM%d work\n", sim);

    level = (SIM2==sim ? LOWLEVEL : HIGHLEVEL);

    at91_set_gpio_value (GPRS_SELECT_SIM_PIN, level);

    return 0;    
}
#endif

int gprs_powerup(int which)
{
    dbg_print("Power up %s module.\n", support_gprs[which].name);
    /*Must turn VBUS_off before turn UC864E on, after UC864E power on, then turn vbus on.*/
    if(GSM_UC864E==which)
    {
       if( 1 == uc864e_power_mon())  /*UC864E already power up*/
       {
           dbg_print("%s module already power up.\n", support_gprs[which].name);
           return 0;
       }
       vbus_off();  
    }

    at91_set_gpio_value (GPRS_POWER_PIN, LOWLEVEL);    
    dbg_print("Pull power pin lowlevel for %ld seconds\n", support_gprs[which].poweron_period_time);
    msleep(support_gprs[which].poweron_period_time);
    at91_set_gpio_value (GPRS_POWER_PIN, HIGHLEVEL);    

    if(GSM_UC864E==which)
       vbus_on();

    dbg_print("Delay for %ld ms wait for AT command ready\n", support_gprs[which].atcmd_active_time);
    msleep(support_gprs[which].atcmd_active_time);

    return 0;
}

int gprs_powerdown(int which)
{
    dbg_print("Power down %s module.\n", support_gprs[which].name);
    /* To turn OFF UC864E , first of all, you MUST cut off supplying power to 
     * the USB_VBUS, or the module does not turn off*/
    if(GSM_UC864E==which)
    {
       vbus_off();  
    }

    at91_set_gpio_value (GPRS_POWER_PIN, LOWLEVEL);    
    msleep(support_gprs[which].poweroff_period_time);
    at91_set_gpio_value (GPRS_POWER_PIN, HIGHLEVEL);    

    dbg_print("Delay for %ld ms wait for AT shutdown\n", support_gprs[which].atcmd_inactive_time);
    msleep(support_gprs[which].atcmd_inactive_time);

    return 0;
}

int gprs_chk_simdoor(int sim)
{
    unsigned pin;
    int status;

    /*To compatible with L350*/
    pin= (SIM2==sim ? GPRS_CHK_SIM2_PIN : GPRS_CHK_SIM_PIN);
    /*pin = GPRS_CHK_SIM_PIN; */

    status = (LOWLEVEL==at91_get_gpio_value(pin) ? SIM_PRESENT : SIM_NOTPRESENT);
    dbg_print("Check %s %s.\n", SIM2==sim?"SIM2":"SIM1", SIM_PRESENT==status?"present":"not present");

    return status;
}

void gprs_set_dtr(int level)
{
    level = (LOWLEVEL==level ? LOWLEVEL: HIGHLEVEL);
    at91_set_gpio_value (GPRS_DTR_PIN, LOWLEVEL);
}

int gprs_chk_ring(int which)
{
    return 0;
}


