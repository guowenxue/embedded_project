#include <stdio.h>
#include <unistd.h>
#include "protocol.h"
#include "version.h"
#include "worker.h"

uchar g_ucdebug;

PROT_WORK test_list[] = 
{
   {CMD_LED, test_led},
   {CMD_USB, test_usb},
   {0, NULL}
};

void print_ver(void)
{
     printf("Version: %d.%d.%d Build %d\n", MAJOR, MINOR, REVER, SVNVER);
     printf("Copyright (C) 2011 by guowenxue(guowenxue@gmail.com)\n");
     return;
}

void print_usage(char *progname)
{
     printf("Usage: %s [-d] [-f] [-h] [-v]\n", progname);
     printf("   -d: Enable debug output.\n");
     printf("   -l: Test LED.\n");
     printf("   -u: Test USB.\n");
     printf("   -s: Test SD.\n");
     printf("   -r: Test RTC.\n");
     printf("   -h: Display This help information.\n");
     printf("   -v: Get version information.\n");

     print_ver();
     return;
}

int main(int argc, char **argv)
{
    uchar             aucParm[3] = {0x33,0x55,0xFF};
    uchar             ucCmd = CMD_NONE; 
    PROT_WORK         *worker;
    int               iopt;
    TEST_BODY         *test_body; 

    while( (iopt=getopt(argc, argv, "dlusrhv")) != -1)
    {
        switch(iopt)
        {
             case 'd':
                g_ucdebug = 0x01;
                break;
             case 'l':
                ucCmd = CMD_LED;
                break;
             case 'u':
                ucCmd = CMD_USB;
                break;
             case 's':
                ucCmd = CMD_SD;
                break;
             case 'r':
                ucCmd = CMD_RTC;
                break;
             case 'h':
                print_usage(argv[0]);
                return;
             case 'v':
                print_ver();
                return 0;
             default:
                break;
        }
   }

   if(CMD_NONE == ucCmd)
   {
       print_usage(argv[0]); 
       return 0;
   }

   test_body = find_body((PROT_WORK *)&test_list, ucCmd);
   if(NULL != test_body)
        test_body(aucParm, 3);

   return 0;
}

