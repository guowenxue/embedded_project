//#include <string.h>
//#include <rt_heap.h>
//#include "platform.h"
//#include "flash.h"
//#include "bib.h"
//#include "net.h"
//#include "serial.h"
#include "uprintf.h"
#include "timer.h"
//#include "tftp.h"
//#include "sh.h"
#include "wbled.h"

void all_led_on()
{
        unsigned int    val;

        // set GPIO_CFG6.{0,1,2,3,11} as GPIO mode
        val = inpi(GPIO_CFG6);
        val &= 0x003FFF00;
        outpi(GPIO_CFG6, val);


        val = inpi(GPIO_DATAOUT6);
        val &= ~(LED0|LED1|LED2|LED3|LED4);     // all on 
        outpi(GPIO_DATAOUT6, val);

        // set GPIO_DIR6.{0,1,2,3,11} as output mode: 0x80f -> 1000 0000 1111
        //  & enable internal pull-up resister
        outpi(GPIO_DIR6,0x080f080f);
}

void all_led_off()
{
        unsigned int    val;

        // set GPIO_CFG6.{0,1,2,3,11} as GPIO mode
        val = inpi(GPIO_CFG6);
        val &= 0x003FFF00;
        outpi(GPIO_CFG6, val);


        val = inpi(GPIO_DATAOUT6);
        val |= (LED0|LED1|LED2|LED3|LED4);      // all off 
        outpi(GPIO_DATAOUT6, val);

        // set GPIO_DIR6.{0,1,2,3,11} as output mode: 0x80f -> 1000 0000 1111
        //  & enable internal pull-up resister
        outpi(GPIO_DIR6,0x080f080f);
}

void runled_blink()
{
   unsigned int val;

// init GPIO 2, set it as PWM1 mode
   val = inpi(GPIO_CFG0);
   val &=0xffffffcf;    	// set bit[5:4] = 00
   val |=0x20;                  // set bit[5:4] = 10 , now GPIO 2 work on PWM1 mode
   outpi(GPIO_CFG0,val);

   val = inpl(PWM_CSR); // set PWM_CSR: bit[7:4]=11 
   val &=0x0f;
   val |=0x30;
   outpl(PWM_CSR,val);

   outpl(PWM_PPR,0xff);

   val = inpl(PWM_PCR); // set PWM_PCR: bit[7:0]=0000 1000
   val &=0x0f;
   val |=0x800;
   outpl(PWM_PCR,val);

   outpl(PWM_CMR1,0x2625);
   outpl(PWM_CNR1,0x4caa);

   val = inpl(PWM_PCR); // set PWM_PCR: bit[7:0]=0000 xxx1
   val &=0xf0f;
   val |=0x100;
   outpl(PWM_PCR,val);
}

void runled_on()
{
        unsigned int val;

        val = inpi(GPIO_CFG0);  // set GPIO 1-> PORT0.2 as GPIO mode
        val &= 0x000003c0;
        outpi(GPIO_CFG0, val);


        val = inpi(GPIO_DATAOUT0);
        val &= ~RUN_LED;
        outpi(GPIO_DATAOUT0, val);

        val = inpi(GPIO_DIR0);		// set GPIO 1 as
        val |= 0x00040004;
        outpi(GPIO_DIR0, val);
}

void start_buzzer()
{
   unsigned int val;

// init GPIO 1, set it as PWM0 mode
   val = inpi(GPIO_CFG0);
   val &=0xfffffff3;
   val |=0x08;
   outpi(GPIO_CFG0,val);
// init end

   val = inpl(PWM_CSR); // set PWM_CSR: bit[3:0]=0011 
   val &=0xf0;
   val |=0x03;
   outpl(PWM_CSR,val);

   outpl(PWM_PPR,0xff);

   val = inpl(PWM_PCR); // set PWM_PCR: bit[7:0]=0000 1000
   val &=0xf00;
   val |=0x08;
   outpl(PWM_PCR,val);

   outpl(PWM_CMR0,0x6);
   outpl(PWM_CNR0,0xc);

   val = inpl(PWM_PCR); // set PWM_PCR: bit[7:0]=0000 xxx1
   val &=0xf0f;
   val |=0x01;
   outpl(PWM_PCR,val);
}

void stop_buzzer()
{
   unsigned int val;

// init GPIO 1, set it as PWM0 mode
   val = inpi(GPIO_CFG0);
   val &=0xfffffff3;
   val |=0x08;
   outpi(GPIO_CFG0,val);

   outpl(PWM_CNR0,0);
   val = inpl(PWM_PCR);
   val &=0xf00;
   outpl(PWM_PCR,val);
}

void checkpass()
{
	all_led_on();	
	runled_on();
}

void checkfail()
{
	int i = 0;
	runled_blink();

	for(i=0; i<3; i++)
	{
		start_buzzer();
		all_led_on();
		sleep(500);
		 
		stop_buzzer();
		all_led_off();
		sleep(500); 
	}
}


void wbled()
{
	checkfail();
}
	