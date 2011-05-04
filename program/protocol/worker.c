#include "worker.h"

uchar test_led(uchar *pacParm, int iParmLen)
{
   int    i;
   dbg_print("Test LED now.\n");
   for(i=0; i<iParmLen; i++)
   {
       printf("0x%02x ",  *(pacParm+i)); 
   }
   printf("\n");

   return 0x00;
}

uchar test_usb(uchar *pacParm, int iParmLen)
{
   int    i;
   dbg_print("Test USB now.\n");
   for(i=0; i<iParmLen; i++)
   {
       printf("0x%02x ", (uchar) pacParm[i]); 
   }
   printf("\n");

   return 0x00;
}

TEST_BODY * find_body(PROT_WORK *test_list, uchar ucCmd)
{
   PROT_WORK   *worker;

   for(worker=test_list; NULL!=worker->test_body; worker++)
   {
       if(ucCmd == worker->ucCmd)
       {
            return worker->test_body; 
       } 
       else
       {
            dbg_print("Can not find a test function.\n");
            return NULL;
       }
   }
}

