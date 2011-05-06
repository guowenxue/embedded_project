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
                     .poweron_period_time = 60,
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
                    .atcmd_inactive_time = 3000,
                    .ring_call_time = 10,
                    .ring_sms_time = 10,
                    }
    ,
    [GSM_SIM5215] = {           /*Refer to "Hardware Design SIM5215_HD_V1.01" 15.01.2010 */
                     .name = "SIM5215",
                     .id = GSM_SIM5215,
                     /*Turn GPRS_PWR pin to low level >=64 ms, Page 22 */
                     .poweron_period_time = 80,
                     /*Reference Page 22, AT command can be set 2~3 after SIM5215 is power on */
                     .atcmd_active_time = 3000,
                     /* Turn GPRS_PWR pin to low level about 2 second, 64 ms in datasheet Page 22 
                      * is wrong. or can send AT command "AT+CPOF" to shutdown */
                     .poweroff_period_time = 2000,
                     .atcmd_inactive_time = 3000,
                     .ring_call_time = 10,
                     .ring_sms_time = 10,
                     }
    ,
};

int dev_count = ARRAY_SIZE(support_gprs);

void init_gprs_pin(void)
{
    at91_set_gpio_output (GPRS_POWER_PIN, HIGHLEVEL);
    at91_set_gpio_output (GPRS_RESET_PIN, LOWLEVEL);
    at91_set_gpio_output (GPRS_DTR_PIN, LOWLEVEL);

    at91_set_gpio_input  (GPRS_CHK_SIM_PIN, DISPULLUP);
}

int gprs_powerup(int which)
{
    at91_set_gpio_value (GPRS_POWER_PIN, LOWLEVEL);    
    msleep(support_gprs[which].poweron_period_time);
    at91_set_gpio_value (GPRS_POWER_PIN, HIGHLEVEL);    

    dbg_print("Delay for %ld ms wait for AT command ready\n", support_gprs[which].atcmd_active_time);
    msleep(support_gprs[which].atcmd_active_time);

    return 0;
}

int gprs_powerdown(int which)
{
    at91_set_gpio_value (GPRS_POWER_PIN, LOWLEVEL);    
    msleep(support_gprs[which].poweron_period_time);
    at91_set_gpio_value (GPRS_POWER_PIN, HIGHLEVEL);    

    dbg_print("Delay for %ld ms wait for AT command ready\n", support_gprs[which].atcmd_active_time);
    msleep(support_gprs[which].atcmd_active_time);

    return 0;
}

int gprs_chk_simdoor(void)
{
    int status = SIM_NOTPRESENT;

    if(LOWLEVEL == at91_get_gpio_value(GPRS_CHK_SIM_PIN))
    {
          status = SIM_PRESENT; 
    }

    return status;
}

int gprs_chk_ring(int which)
{
    return 0;
}

