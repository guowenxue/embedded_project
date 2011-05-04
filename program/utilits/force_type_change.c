#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

/*MISC test itmes bit map, bit[0] used for MISC part test result*/
#define TAG         0   /*This bit set means all the MISC item test pass*/
#define LED         1   /*LED test pass will set uiMisc bit[1]*/
#define BEEP        2   /*Beep test pass will set uiMisc bit[2]*/
#define EEPCARD     3   /*EEPROM on card test pass will set uiMisc bit[3]*/
#define EEPBACK     4   /*EEPROM on backplane test pass will set uiMisc bit[4], only N300 will use this bit*/
#define USB         5   /*USB slot test passw will set bit[5]*/
#define SDC         6   /*SD card test pass will set uiMisc bit[6]*/
#define RTC         7   /*RTC test pass will set uiMisc bit[7]*/

/*PSTN test itmes bit map*/
#define TAG         0   /*This bit set means all the PSTN item test pass*/
#define PSTN1       1   /*PSTN1 test pass will set uiPstn bit[1]*/
#define PSTN2       2   /*PSTN2 test pass will set uiPstn bit[2]*/
#define PSTN3       3   /*PSTN3 test pass will set uiPstn bit[3]*/
#define PSTN4       4   /*PSTN4 test pass will set uiPstn bit[4]*/
#define PSTN5       5   /*PSTN5 test pass will set uiPstn bit[5]*/
#define PSTN6       6   /*PSTN6 test pass will set uiPstn bit[6]*/
#define PSTN7       7   /*PSTN7 test pass will set uiPstn bit[7]*/
#define PSTN8       8   /*PSTN8 test pass will set uiPstn bit[8]*/

/*Communication way test itmes bit map*/
#define TAG         0   /*This bit set means all the COMM item test pass*/
#define GPRS_SIM1   1   /*GPRS module with SIM1 test pass will set uiComm bit[1]*/
#define GPRS_SIM2   2   /*GPRS module with SIM2 test pass will set uiComm bit[2]*/
#define ETH0        3   /*Eth0 test pass will set uiComm bit[3]*/
#define ETH1        4   /*Eth1 test pass will set uiComm bit[4]*/
#define UART        5   /*UART(RS232/RS232, RS485/RS485 or RS232/RS485) test pass will set uiComm bit[5]*/

/*Optimus test itmes bit map*/
#define TAG         0   /*This bit set means all the PSTN item test pass*/
#define TELEPHONE   1   /*Telephone test pass will set uiOptim bit[1]*/
#define FAX         2   /*FAX test pass will set uiOptim bit[2]*/
#define POS1        3   /*POS1 port test pass will set uiOptim bit[3]*/
#define POS2        4   /*POS2 port test pass will set uiOptim bit[4]*/
#define POS3        5   /*POS3 port test pass will set uiOptim bit[5]*/

/****** Bit Operate Define *****/
#define SET_BIT(data, i)   ((data) |=  (1 << (i)))    /* Set the bit "i" in "data" to 1  */
#define CLR_BIT(data, i)   ((data) &= ~(1 << (i)))    /* Clear the bit "i" in "data" to 0 */
#define NOT_BIT(data, i)   ((data) ^=  (1 << (i)))    /* Inverse the bit "i" in "data"  */
#define GET_BIT(data, i)   ((data) >> (i) & 1)        /* Get the value of bit "i"  in "data" */
#define L_SHIFT(data, i)¡¡ ((data) << (i))            /* Shift "data" left for "i" bit  */
#define R_SHIFT(data, i)¡¡ ((data) >> (i))            /* Shift "data" Right for "i" bit  */

#define QCBITMAP_DEBUG
#ifdef QCBITMAP_DEBUG
#define dbg_print(format,args...) printf(format, ##args)
#else
#define dbg_print(format,args...) do{} while(0);
#endif


#define QCBITMAP_FILE  "/info/.qcbitmap"

#define QC_MISC            0
#define QC_PSTN            1
#define QC_COMM            2
#define QC_OPTM            3

typedef struct __QCBITMAP
{
        /*This Item is for LED,Beep,Card EEPROM,Backplane EEPROM,USB,SD Card,RTC test item*/
        unsigned int    uiMisc; 
        
        /*This is for PSTN test item*/ 
        unsigned int    uiPstn; 
        
        /*This is for GPRS,ETHERNET,UART communication test item*/ 
        unsigned int    uiComm; 
        
       /*This is for Optimus test item*/ 
        unsigned int    uiOptm;
} QCBITMAP;


static int            s_qcbitmap_fd = -1;

inline void set_qcbitmap(int part, int item)
{
   char           acBuf[64]; 
   unsigned int   *iPtr = (unsigned int *)acBuf;

   memset(acBuf, 0, sizeof(acBuf)); 
   lseek(s_qcbitmap_fd, 0, SEEK_SET); /*Read from file start address*/
   read(s_qcbitmap_fd, acBuf, sizeof(acBuf));

   *(iPtr+part) |= (1<<item);

   lseek(s_qcbitmap_fd, 0, SEEK_SET); /*Write from file start address*/
   write(s_qcbitmap_fd, acBuf, sizeof(QCBITMAP));
}

inline void clr_qcbitmap(int part, int item)
{
   char           acBuf[64]; 
   unsigned int   *iPtr = (unsigned int *)acBuf;

   memset(acBuf, 0, sizeof(acBuf)); 
   lseek(s_qcbitmap_fd, 0, SEEK_SET); /*Read from file start address*/
   read(s_qcbitmap_fd, acBuf, sizeof(acBuf));

   *(iPtr+part) &= ~(1<<item);

   lseek(s_qcbitmap_fd, 0, SEEK_SET); /*Write from file start address*/
   write(s_qcbitmap_fd, acBuf, sizeof(QCBITMAP));
}


inline int get_qcbitmap(int part, int item)
{
   char           acBuf[64]; 
   unsigned int   *iPtr = (unsigned int *)acBuf;

   memset(acBuf, 0, sizeof(acBuf)); 
   lseek(s_qcbitmap_fd, 0, SEEK_SET); /*Read from file start address every time*/
   read(s_qcbitmap_fd, acBuf, sizeof(acBuf));

   dbg_print("part[%d]: %d\n", part, (*(iPtr+part)>>item ) & 1);
   return (*(iPtr+part)>>item ) & 1;
}

void print_qcbitmap(void)
{
   char           acBuf[64]; 
   QCBITMAP       *ptr = (QCBITMAP *)acBuf; 

   memset(acBuf, 0, sizeof(acBuf)); 
   lseek(s_qcbitmap_fd, 0, SEEK_SET); /*Read from file start address*/
   read(s_qcbitmap_fd, acBuf, sizeof(acBuf));

   dbg_print("MISC:0x%0x\n", ptr->uiMisc);
   dbg_print("PSTN:0x%0x\n", ptr->uiPstn);
   dbg_print("COMM:0x%0x\n", ptr->uiComm);
   dbg_print("OPTM:0x%0x\n", ptr->uiOptm);
   return;
}

int init_qcbitmap_file(char *path);
void close_qcbitmap_file(void);

int main(void)
{
   if(init_qcbitmap_file(QCBITMAP_FILE) < 0)
   {
      perror("open test failure"); 
      return 0;
   }

   set_qcbitmap(QC_MISC, LED);
   set_qcbitmap(QC_MISC, RTC);
   set_qcbitmap(QC_PSTN, PSTN1);
   set_qcbitmap(QC_PSTN, PSTN8);
   set_qcbitmap(QC_COMM, ETH0);
   set_qcbitmap(QC_COMM, ETH1);
   set_qcbitmap(QC_OPTM, POS1);
   set_qcbitmap(QC_OPTM, POS2);

   close_qcbitmap_file();
   return 0;
}

int init_qcbitmap_file(char *path)
{
   struct stat        file_stat;

   system("unlock info");
   s_qcbitmap_fd = open(path, O_RDWR|O_CREAT, 0666);

   if(s_qcbitmap_fd < 0)
   {
       dbg_print("Open %s file failure.\n", path);
       return -1;
   }

   /*If file size is zero(new create jsut now), then set the file size to QCBITMAP size. */
   fstat(s_qcbitmap_fd, &file_stat);
   if(0==file_stat.st_size)
   {
       lseek(s_qcbitmap_fd, sizeof(QCBITMAP), SEEK_SET);
       write(s_qcbitmap_fd, "", 1);
   }
   else
   {
       dbg_print("Original QC TAG Bitmap:\n");
       print_qcbitmap();
   }


   return 0;
}

void close_qcbitmap_file(void)
{
   close(s_qcbitmap_fd);
   system("lock info");
}
