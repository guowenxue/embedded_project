#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    char           cmd[64];
    char           buf[512];
    FILE           *fp;

    if(argc != 2)
    {
        printf("%s Ipaddr\n", argv[0]); 
        return -1;
    }

    snprintf(cmd, sizeof(cmd), "/bin/netstat -ant | grep %s", argv[1]);
    printf("popen() excute: %s\n", cmd);

    if(NULL == (fp=popen(cmd, "r")) )
    {
          perror("poen"); 
          return 0;
    }

    if(fgets(buf, 512, fp) == NULL)
         printf("Connection idle\n");
    else
         printf("Connection Busy\n");

    pclose(fp);
    return 0;
}

