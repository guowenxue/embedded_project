/*
 * =====================================================================================
 *
 *       Filename:  ioctl.c
 *
 *    Description:  An ioctl tools
 *
 *        Version:  1.0
 *        Created:  05/04/2011 10:20:32 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Guo Wenxue (kernel), guowenxue@ghlsystems.com
 *        Company:  GHL System Berhad.
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <libgen.h>
#include <fcntl.h>
#include <errno.h>

void usage(char *program)
{
    printf("---------------------------------------------------------------\n");
    printf("This program used to excute ioctl() system call for debug.\n");
    printf("Usage: %s [dev_name] [cmd] [arg]\n", basename(program));
    printf("Example: %s /dev/led  24609 0\n", basename(program));
    printf("---------------------------------------------------------------\n");
}

int main(int argc, char **argv)
{
    int            fd = -1;
    int            ret = -1;
    char           *dev_name = NULL;
    unsigned int   cmd = 0;
    unsigned long  arg = 0;

    if(4 != argc)
    {
       usage(argv[0]);
       return -1;
    }

    dev_name = argv[1];
    cmd = strtoul(argv[2], NULL, 10);
    arg = strtoul(argv[3], NULL, 10);



    if((fd=open(dev_name, O_RDWR, 0555)) < 0)
    {
        printf("Open \"%s\" failure: %s\n", dev_name, strerror(errno));
        return 0;
    }

    if( (ret=ioctl(fd, cmd, arg)) < 0 )
    {
        printf("ioctl \"%s\" failure: %s\n", dev_name, strerror(errno));
        return 0;
    }

    printf("ioctl \"%s\" return: %d\n", dev_name, ret);

    close(fd);

    return 0;
}
       
