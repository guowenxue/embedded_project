#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    char           cmd[64];
    char           buf[512];
    FILE           *fp;
    char           *ptr = NULL;
    unsigned long  ulRxPackets;
    unsigned long  ulTxPackets;
    int            iLostPercent = 0;

    if(argc != 2)
    {
        printf("%s Ipaddr\n", argv[0]); 
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "ping -W1 -c5 -s4 %s", argv[1]);
    printf("popen() excute: %s\n", cmd);

    if(NULL == (fp=popen(cmd, "r")) )
    {
          perror("poen"); 
          return 0;
    }

    printf("popen success.\n");
    while( NULL != fgets(buf, sizeof(buf), fp) )
    {
            if(strstr(buf, "transmitted"))
            {
                 if( NULL == (ptr=strtok(buf, ",")) )
                 {
                     break;
                 }
                 ulTxPackets = atol(ptr);
                 printf("%lu packets transmitted\n", ulTxPackets); 

                 if( NULL == (ptr=strtok(NULL, ",")) )
                 {
                     break;
                 }
                 ulRxPackets = atol(ptr);
                 printf("%lu packets received\n", ulRxPackets); 

                 if( NULL == (ptr=strtok(NULL, ",")) )
                 {
                     break;
                 }
                 iLostPercent = atoi(ptr);
                 printf("%d%% packet loss\n", iLostPercent); 
            }
    }

    pclose(fp);
    return 0;
}

