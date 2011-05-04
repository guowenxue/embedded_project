#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DEV_LEN     16

#if 0
/* Given a ifreq structure this function returns its IP address, not used */
void getip(struct ifreq *ifr,char *addr)
{ 
        struct sockaddr *sa; 
        sa=(struct sockaddr *)&(ifr->ifr_addr); 
        switch(sa->sa_family)
        { 
           case AF_INET6: 
             inet_ntop(AF_INET6,(struct in6_addr *)&(((struct sockaddr_in6 *)sa)->sin6_addr), addr,INET6_ADDRSTRLEN); 
             break; 

           default : 
             strcpy(addr, (const char *)inet_ntoa(((struct sockaddr_in *)sa)->sin_addr)); 
        } 
        return;
}
#endif

int main(int argc, char **argv)
{ 
        char                   netaddr[INET_ADDRSTRLEN]; 
        char                   maskaddr[INET_ADDRSTRLEN]; 
        char                   ptpaddr[INET_ADDRSTRLEN]; 
        int                    sd; 
        struct ifreq           ifr;
        struct sockaddr_in     *sin;
        char                   devname[DEV_LEN]={0};

        if(argc != 2)
        {
             printf("Usage: %s eth0/eth1/ppp10 \n", argv[0]); 
             return 0;
        }

        strncpy(devname, argv[1], DEV_LEN);

        sd=socket(AF_INET,SOCK_DGRAM,0); 
        strcpy(ifr.ifr_name, devname); 

        memset(netaddr, 0, INET_ADDRSTRLEN);
        memset(maskaddr, 0, INET_ADDRSTRLEN);
        memset(ptpaddr, 0, INET_ADDRSTRLEN);
        
        /*Get IP address*/
        if((ioctl(sd, SIOCGIFADDR, (caddr_t)&ifr, sizeof(struct ifreq)))<0) 
        {
                perror("Error"); 
                return 0;
        }
        sin =(struct sockaddr_in *)&ifr.ifr_addr;
        strcpy(netaddr, (const char *)inet_ntoa(sin->sin_addr) ); 
        printf("inet addr: %s\n",netaddr);

        /*Get netmask address*/
        if((ioctl(sd, SIOCGIFNETMASK, (caddr_t)&ifr, sizeof(struct ifreq)))<0) 
        {
                perror("Error"); 
                return 0;
        }
        sin =(struct sockaddr_in *)&ifr.ifr_netmask;
        strcpy(maskaddr, (const char *)inet_ntoa(sin->sin_addr) ); 
        printf("Mask: %s\n",maskaddr);


        /*Get P-T-P address*/
        if(strstr(devname, "ppp"))
        {
           if((ioctl(sd, SIOCGIFDSTADDR, (caddr_t)&ifr, sizeof(struct ifreq)))<0) 
           {
                perror("Error"); 
                return 0;
           }
           sin = (struct sockaddr_in *)&ifr.ifr_dstaddr;
           strcpy(ptpaddr, (const char *)inet_ntoa(sin->sin_addr) ); 
           printf("P-t-P: %s\n",ptpaddr);
        }
        return 0;
}
