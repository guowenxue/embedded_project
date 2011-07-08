#include <common.h>

void disp_banner(void)
{
   printf("Bootstrap Version 0.0.1\n");
}

int bootstrap_main(void)
{
   serial_init();
   init_led_beep();
   disp_banner();

   turn_led_on(LED0);
   beep(1);

   while(1)
      ; /*Dead loop here*/

   return 0;
}

