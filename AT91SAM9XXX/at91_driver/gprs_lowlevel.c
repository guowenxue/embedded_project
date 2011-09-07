/*********************************************************************************
 *  Copyright(c)  2011, Guo Wenxue <guowenxue@gmail.com>
 *  All ringhts reserved.
 *
 *     Filename:  gprs.c
 *  Description:  GPRS driver low level functions, which used control the GPIO port
 *
 *     ChangLog:
 *      1,   Version: 1.0.0
 *           Date: 2011-08-10
 *           Author: guowenxue <guowenxue@gmail.com>
 *       	 Descrtipion: Initial first version
 *
 ********************************************************************************/
#include "dev_gprs.h"

GSM_DEVICE support_gprs[] = {
    [GSM_GTM900B] = {
                     .name = "GTM900B",
                     .id = GSM_GTM900B,
                     /*10~100 ms specified in user manul. */
                     .poweron_period_time = 100,
                     /*Experience value on L2, datasheet is wrong */
                     .atcmd_active_time = 6000,
                     /*About 2~3 seconds specified in user manul, 
                      * or can send AT command "AT%MSO" to shutdown, but without logout network*/
                     .poweroff_period_time = 3000,
                     /*Experience value on L2, datasheet is wrong */
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
                     /*Experience value on L2, datasheet is wrong */
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
                    .atcmd_active_time = 8000,
                    /* Turn GPRS_PWR pin to low level for at least 2 seconds, Page 16, 
                     * or can send AT command "AT#SHDN" to shutdown */
                    .poweroff_period_time = 3000,
                    .atcmd_inactive_time = 3000,
                    .ring_call_time = 10,
                    .ring_sms_time = 10,
                    }
    ,
    [GSM_SIM521X] = {           /*Refer to "Hardware Design SIM521X_HD_V1.01" 15.01.2010 */
                     .name = "SIM521X",
                     .id = GSM_SIM521X,
                     /*Turn GPRS_PWR pin to low level >=64 ms, Page 22 */
                     //.poweron_period_time = 80,
                     .poweron_period_time = 100,
                     /*Reference Page 22, AT command can be set 2~3 after SIM521X is power on */
                     .atcmd_active_time = 8000,
                     /* Turn GPRS_PWR pin to low level about 2 second, 64 ms in datasheet Page 22 
                      * is wrong. or can send AT command "AT+CPOF" to shutdown */
                     .poweroff_period_time = 2000,
                     .atcmd_inactive_time = 3000,
                     .ring_call_time = 10,
                     .ring_sms_time = 10,
                     }
    ,
    [GSM_SIM900X] = {
                     .name = "SIM900X",
                     .id = GSM_SIM900X,
                     /*At least 1s specified in user manul. */
                     .poweron_period_time = 1200,
                     /*At least 2.2s specified in user manul. */
                     .atcmd_active_time = 2500,
                     .poweroff_period_time = 50,
                     .atcmd_inactive_time = 2500,
                     .ring_call_time = 10,
                     .ring_sms_time = 10,
                     }
    ,

};

int  dev_count = ARRAY_SIZE(support_gprs);     /*Support GPRS device count, ARRAY_SIZE(support_gprs)*/

void gprs_hw_init(int which)
{	
#ifndef PLAT_L2
    /*sim5215 not support*/
    if (GSM_SIM521X != which) 
    {
        at91_set_A_periph (GPRS_RTS_PIN, DISPULLUP);
        at91_set_gpio_output (GPRS_DTR_PIN, LOWLEVEL);
    }

    at91_set_A_periph (GPRS_CTS_PIN, DISPULLUP);
    at91_set_gpio_input (GPRS_DSR_PIN, DISPULLUP);
    at91_set_gpio_input (GPRS_DCD_PIN, DISPULLUP);
    at91_set_gpio_input (GPRS_RI_PIN, DISPULLUP);
    at91_set_gpio_input (GPRS_CHK_SIM2_PIN, DISPULLUP);
    at91_set_gpio_output (GPRS_38V_ON_PIN, LOWLEVEL);    //gprs module power on init 

    at91_set_gpio_output (GPRS_SELECT_SIM_PIN, HIGHLEVEL); //default set to SIM1
#else
    at91_set_gpio_output (GPRS_DTR_PIN, HIGHLEVEL);
#endif

    at91_set_gpio_output (GPRS_ON_PIN, HIGHLEVEL);         //gprs module power on init 
    at91_set_gpio_input (GPRS_CHK_SIM1_PIN, DISPULLUP);

    at91_set_A_periph (GPRS_RXD_PIN, DISPULLUP);
    at91_set_A_periph (GPRS_TXD_PIN, ENPULLUP);
	    
    at91_set_gpio_output (GPRS_RESET_PIN, HIGHLEVEL);  //GTM900 need init this pin??
    at91_set_deglitch(GPRS_RI_PIN, ENABLE);
}

int detect_gprs_model(void)
{
    int    iModel = GSM_UNKNOW;

    at91_set_gpio_input (GPRS_MODEL_CHK_PIN0, DISPULLUP);
    at91_set_gpio_input (GPRS_MODEL_CHK_PIN1, DISPULLUP);
    at91_set_gpio_input (GPRS_MODEL_CHK_PIN2, DISPULLUP);

    iModel =    at91_get_gpio_value(GPRS_MODEL_CHK_PIN0)  
               | (at91_get_gpio_value(GPRS_MODEL_CHK_PIN1)<<1)
               | (at91_get_gpio_value(GPRS_MODEL_CHK_PIN2)<<2) ;

    dbg_print("AT91_PIN_PB31: %d\n", at91_get_gpio_value(GPRS_MODEL_CHK_PIN0));
    dbg_print("AT91_PIN_PB12: %d\n", at91_get_gpio_value(GPRS_MODEL_CHK_PIN1));
    dbg_print("AT91_PIN_PB13: %d\n", at91_get_gpio_value(GPRS_MODEL_CHK_PIN2));

    if(iModel == 0x00)
    {
         iModel = GSM_SIM900X;
    }
    else if(iModel == 0x01)
    {
         iModel = GSM_SIM521X;
    }
    else 
    {
         iModel = GSM_UNKNOW;
    }

    if(GSM_UNKNOW != iModel)
    {
       dbg_print("Detect GPRS module \"%s\" ID: 0x%02x\n", support_gprs[iModel].name, iModel);
    }
    else
    {
       dbg_print("Detect Unknow GPRS module ID: 0x%02x\n", iModel);
    }

    return iModel;
}


void gprs_powerup(int which)
{
    dbg_print("Power up %s module.\n", support_gprs[which].name);
    /*USB 3G GPRS module power up procedure will goes here, till now, only UC864E support it*/
    if (POWERON == gprs_powermon(which))
    {
        gprs_reset(which);           /*If GPRS module already power up, then we reset it */
        return;
    }
	//Currently it is power off

#ifdef PLAT_L2
    dbg_print("Enable GPRS_38V_ON_PIN.\n");
	if (GSM_SIM900X == which)
    {
        at91_set_gpio_output (GPRS_RESET_PIN, LOWLEVEL);         //gprs module power on init 
    }
    at91_set_gpio_output (GPRS_38V_ON_PIN, HIGHLEVEL);         //gprs module power on init 
#endif

    /*step1: turn off vbus for 3G module*/
	if (GSM_UC864E == which || GSM_SIM521X == which)
	{		
	    at91_set_gpio_output (GPRS_VBUS_CTRL_PIN, LOWLEVEL); 
		SLEEP (100);
    }
    /*step2: power on gprs module*/

#ifdef PLAT_L2
	at91_set_gpio_value (GPRS_ON_PIN, HIGHLEVEL);		
#else
	at91_set_gpio_value (GPRS_ON_PIN, LOWLEVEL);		
#endif

	SLEEP(support_gprs[which].poweron_period_time);

#ifdef PLAT_L2
	at91_set_gpio_value (GPRS_ON_PIN, LOWLEVEL);		
#else
	at91_set_gpio_value (GPRS_ON_PIN, HIGHLEVEL);		
#endif

#ifdef PLAT_L2
	if (GSM_SIM900X == which)
    {
        at91_set_gpio_output (GPRS_DTR_PIN, LOWLEVEL);         //gprs module power on init 
        msleep(50);
    }
#endif
    
    /*step3: Turn on vbus for 3G module*/
    if (GSM_UC864E == which || GSM_SIM521X == which)
    {
	    at91_set_gpio_output (GPRS_VBUS_CTRL_PIN, HIGHLEVEL); 
    }    

    dbg_print("Delay for %ld ms wait for AT command ready\n", support_gprs[which].atcmd_active_time);
    SLEEP(support_gprs[which].atcmd_active_time);
}

void gprs_powerdown(int which)
{
    if (POWEROFF == gprs_powermon(which))   /*Alread power off */
        return;

    dbg_print("Power down %s module.\n", support_gprs[which].name);

	// Currently it is power on
    
    /*step1: turn off vbus*/
    if (GSM_UC864E == which || GSM_SIM521X == which)
    {
	    at91_set_gpio_output (GPRS_VBUS_CTRL_PIN, LOWLEVEL); 
        SLEEP (100);
    }

    /*step 2: turn off gprs module*/
#ifdef PLAT_L2
	at91_set_gpio_value (GPRS_ON_PIN, HIGHLEVEL);		
#else
	at91_set_gpio_value (GPRS_ON_PIN, LOWLEVEL);		
#endif

	SLEEP(support_gprs[which].poweroff_period_time);		

#ifdef PLAT_L2
	at91_set_gpio_value (GPRS_ON_PIN, LOWLEVEL);		
    at91_set_gpio_output (GPRS_38V_ON_PIN, LOWLEVEL);         //gprs module power on init 
#else
	at91_set_gpio_value (GPRS_ON_PIN, HIGHLEVEL);		
#endif

    dbg_print("Delay for %ld ms wait for AT command shutdown\n", support_gprs[which].atcmd_inactive_time);    
    SLEEP(support_gprs[which].atcmd_inactive_time);
}

void gprs_reset(int which)
{
    /*GSM_GTM900B/C doesn't support GPRS reset */
    if (GSM_GTM900B == which || GSM_GTM900C == which)
       return;

    dbg_print("Reset %s module.\n", support_gprs[which].name);
	/*step1: turn off vbus*/	
    if(GSM_UC864E == which || GSM_SIM521X == which)
    {
	    at91_set_gpio_output (GPRS_VBUS_CTRL_PIN, LOWLEVEL); 
        SLEEP (100);
    }

    /*step2: reset pin place high*/
    at91_set_gpio_output (GPRS_RESET_PIN, LOWLEVEL);      
    SLEEP (10);
    at91_set_gpio_output (GPRS_RESET_PIN, HIGHLEVEL);      
	SLEEP (200);

    if(GSM_UC864E == which || GSM_SIM521X == which)   
    {
	    at91_set_gpio_output (GPRS_VBUS_CTRL_PIN, HIGHLEVEL); 
    }

    dbg_print("Delay for %ld ms wait for AT command ready\n", support_gprs[which].atcmd_active_time);    
    SLEEP(support_gprs[which].atcmd_active_time);
}

/*Till now only UC864E support it*/
int gprs_powermon(int which)
{
    /*Only UC864E module support Power monitor ping */
    if (GSM_UC864E != which)
        return -1;

    /*Lowlevel means power off and highlevel means power on */
    at91_set_gpio_input (GPRS_POWER_MON_PIN, DISPULLUP);  
	if (LOWLEVEL == at91_get_gpio_value (GPRS_POWER_MON_PIN))
	{
		return POWEROFF;
	}
	return POWERON;
}

int gprs_set_worksim(int sim)  
{
    if (SIM1 != sim && SIM2 != sim)
    {
        printk("ERROR: Set unknow SIM%d to work.\n", sim);
        return -1;
    }

    dbg_print("Set SIM%d work\n", sim);
    if (SIM2 == sim)
        at91_set_gpio_value (GPRS_SELECT_SIM_PIN, LOWLEVEL);
    else
        at91_set_gpio_value (GPRS_SELECT_SIM_PIN, HIGHLEVEL);

    return 0;
}

int gprs_get_worksim(void)  
{
	if (LOWLEVEL == at91_get_gpio_value (GPRS_SELECT_SIM_PIN))
		return SIM2;
	else
		return SIM1;
}

int gprs_chk_simdoor(int sim) 
{
    if (SIM1 != sim && SIM2 != sim)
    {
        printk("ERROR: Check unknow SIM%d to work.\n", sim);
        return -1;
    }

    dbg_print("Check SIM%d work\n", sim);
	if (SIM1 == sim)
		return at91_get_gpio_value (GPRS_CHK_SIM1_PIN);
	else
		return at91_get_gpio_value (GPRS_CHK_SIM2_PIN);

    return 0;
}

void gprs_set_dtr(int which, int level)
{
    /*SIM521X doesn't support it */
    if (GSM_SIM521X == which)
        return;

	if (LOWLEVEL == level)
		at91_set_gpio_value (GPRS_DTR_PIN, LOWLEVEL);
	else
		at91_set_gpio_value (GPRS_DTR_PIN, HIGHLEVEL);
}

void gprs_set_rts(int which, int level)
{
    /*SIM521X doesn't support it */
    if (GSM_SIM521X == which)
        return;

    if (LOWLEVEL == level)
        at91_set_gpio_value (GPRS_RTS_PIN, LOWLEVEL);
    else
        at91_set_gpio_value (GPRS_RTS_PIN, HIGHLEVEL);
}

