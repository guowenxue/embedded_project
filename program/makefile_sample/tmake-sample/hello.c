#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>

#include "version.h"

#define LONG_OPTION_SUPPORT 

unsigned char            g_ucDebug = 0x00;
#define dbg_print(format,args...)  if(0x00!=g_ucDebug)  { printf(format, ##args); }

int main(int argc, char **argv)
{
    int                 iOpt = 0;   

#ifdef LONG_OPTION_SUPPORT
    struct option long_options[] =
    {
        {"debug",      required_argument, NULL, 'd'},
        {"version",    no_argument,       NULL, 'v'},
        {"help",       no_argument,       NULL, 'h'},
        {NULL,         0,                 NULL,  0 }
    };

    while ((iOpt = getopt_long(argc, argv, "d:vh", long_options, NULL)) != -1)
#else
    while( (iOpt=getopt(argc, argv, "dfvh")) != -1)
#endif
    {
        switch(iOpt)
        {
             case 'd':
                g_ucDebug = 0x01;
                break;

             case 'h':
                printf("Usage: %s [-d] [-f] [-h] [-v]\n", argv[0]);
                printf("   -d: Enable debug output.\n");
                printf("   -h: Display This help information.\n");
                printf("   -v: Get version information.\n");

             case 'v':
                printf("Version: %d.%d.%d Build %d\n", MAJOR, MINOR, REVER, SVNVER);
                printf("Copyright (C) Guo Wenxue(Email:guowenxue@gmail.com QQ:281143292).\n");
                return 0;

             default:
                printf("Unsupport option %c\n", iOpt);
                break;
        }
    }

    printf("Hello World!\n");
    dbg_print("Goodby cruel World!\n");

    return 0;
}

