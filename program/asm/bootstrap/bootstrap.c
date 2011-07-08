#include <common.h>


int bootstrap_main(void)
{
   serial_init();
   init_led_beep();

#ifdef CONFIG_PRINTF_DBG
   printf("GHL Bootstrap Version 0.0.1\n");
#endif

   turn_led_on(LED0);
   beep(1);

   while(1)
      ; /*Dead loop here*/

   return 0;
}

