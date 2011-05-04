/** 
 * Tcp client program, It is a simple example only. * zhengsh 200520602061 2 * connect to server, and echo a message from server. */ 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h> 
#include <time.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define  CLIENT_PORT ((20001+rand())%65536)  //  define the defualt client port as a random port 
#define  BUFFER_SIZE 255 
#define  REUQEST_MESSAGE "welcome to connect the server.\n" 
void  usage(char* name)
{       
        printf( " usage: %s IpAddr port\n " ,name);
}

int  main(int argc, char** argv)
{       
        int                    clifd = -1;       
        struct  sockaddr_in    servaddr,cliaddr;       
        socklen_t              socklen = sizeof (servaddr);       
        int                    port = -1;

        if (argc != 3 )         
        {              
                usage(argv[ 0 ]);              
                exit( 1 );       
        }        

        if ((clifd  =  socket(AF_INET,SOCK_STREAM, 0 ))  <   0 )         
        {             
                printf( " create socket error!\n " );             
                exit( 1 );       
        }         

        srand(time(NULL)); // initialize random generator         

        memset( &cliaddr, 0, sizeof (cliaddr));       
        cliaddr.sin_family  =  AF_INET;       
        cliaddr.sin_port  =  htons(CLIENT_PORT);       
        cliaddr.sin_addr.s_addr  =  htons(INADDR_ANY);       

        memset(&servaddr, 0, sizeof (servaddr));       
        servaddr.sin_family  =  AF_INET;       
        inet_aton(argv[ 1 ], & servaddr.sin_addr);       
        port = atoi(argv[2]);
        servaddr.sin_port  =  htons(port);      // servaddr.sin_addr.s_addr = htons(INADDR_ANY);        

        if  (bind(clifd, (struct sockaddr* ) &cliaddr, sizeof (cliaddr)) < 0 )       
        {              
                printf( " bind to port %d failure!\n " ,CLIENT_PORT);              
                exit( 1 );       
        }         

        if (connect(clifd,( struct  sockaddr * ) & servaddr, socklen)  <   0 )       
        {              
                printf( " can't connect to %s!\n ", argv[ 1 ]);              
                exit( 1 );       
        }        

        sleep(5); /*Sleep 30 seconds*/

        close(clifd);       
        return 0;
} 
