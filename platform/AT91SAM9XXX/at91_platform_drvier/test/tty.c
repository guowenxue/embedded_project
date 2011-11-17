/*********************************************************************************
 *      Copyright:  (C) 2011 R&D of San Fran Electronics Co., LTD  
 *                  All rights reserved.
 *
 *       Filename:  test.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(11/02/2011~)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "11/02/2011 10:55:45 AM"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>

#define MODE_POLL     0x01
#define MODE_NORMAL   0x02
#define BUF_SIZE      64


unsigned char g_ucStop = 0x00;

void signal_handler (int signo)
{
    g_ucStop = 0x01;
}


/********************************************************************************
 *  Description:
 *   Input Args:
 *  Output Args:
 * Return Value:
 ********************************************************************************/
int main(int argc, char **argv)
{
    int fd = -1;
    int ret;
    char  buf[BUF_SIZE];
    unsigned mode = MODE_NORMAL;
    int debug=0x00;
    struct sigaction      sigact;

    printf("Pass two argument will use POLL mode to test the gstty driver\n");

    if(argc==2)
    {
       debug=0x01; 
    }

    if (argc == 3)
    {
        mode = MODE_POLL;
    }

    fd = open("/dev/gstty0", O_RDWR);
    if (fd < 0)
    {
        perror("Open gstty failure\n");
        return -1;
    } 
    
    /* Install signal.*/ 
    sigemptyset (&sigact.sa_mask); 
    sigact.sa_flags = 0; 
    sigact.sa_handler = signal_handler;

    sigaction (SIGTERM, &sigact, 0);    /*  catch terminate signal "kill" command*/ 
    sigaction (SIGINT,  &sigact, 0);    /*  catch interrupt signal CTRL+C */ 
    sigaction (SIGSEGV, &sigact, 0);    /*  catch segmentation faults  */ 
    sigaction (SIGCHLD, &sigact, 0);    /*  catch child process return */ 
    sigaction (SIGPIPE, &sigact, 0);    /*  catch broken pipe */


    while ( !g_ucStop )
    {
        if (MODE_POLL == mode)
        {
            fd_set rds;

            FD_ZERO(&rds);
            FD_SET(fd, &rds);

            ret = select(fd + 1, &rds, NULL, NULL, NULL);
            if (ret < 0)
            {
                printf("Select gsttys Device Faild!\n");
                return -1;
            }

            if (ret == 0)
            {
                printf("Read gsttys Device Timeout!\n");
            }
            else if (FD_ISSET(fd, &rds))
            {
                ret = read(fd, buf, sizeof(BUF_SIZE));    //读设备
                if (ret <= 0)
                {
                    if (errno != EAGAIN)
                    {
                        printf("Read gstty Device Faild!\n");
                    }
                    continue;
                }
                else
                {
                    int i;
                    for (i=0; i<ret; i++)
                    printf("buf[%d] 0x%x\n", i, buf[i]);
                }
            }
        }
        else
        {
            ret = read(fd, buf, BUF_SIZE);    //读设备
            if (ret < 0)
            {
                perror("read failure");
            }

            if(debug && ret>0)
            {
                int i;
                for (i=0; i<ret; i++)
                  printf("buf[%d] %x\n", i, buf[i]);
            }

            if(ret>0)
            {
               ret = write(fd, buf, ret);
               if(ret < 0)
                   perror("Write falure");
            }

            memset(buf, 0x00, BUF_SIZE);

        }

    }

    ioctl(fd, 0, 0);

    close(fd);
    return 0;
}                               /* ----- End of main() ----- */
