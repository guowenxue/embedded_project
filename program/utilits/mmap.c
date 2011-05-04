#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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


#define QCBITMAP_FILE  "/info/.qcbitmap"

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

QCBITMAP        *pstQcBitmap = NULL;

#define set_qcbitmap(part, item) ({ pstQcBitmap->part |= 1<<(item);    })
#define clr_qcbitmap(part, item) ({ pstQcBitmap->part &= ~(1<<(item)); }) 
#define get_qcbitmap(part, item) ({ pstQcBitmap->part>>(item) & 1;     }) 

int main(void)
{
   int                fd = -1;
   struct stat        file_stat;

   fd = open(QCBITMAP_FILE, O_RDWR|O_CREAT, 0666);
   if(fd < 0)
   {
      perror("open test failure"); 
      return 0;
   }

   /* Create the file, let its size be QCBITMAP size. Must do it, or after mmap() and 
    * write data to file will throw "Bus Error"*/
   fstat(fd, &file_stat);
   printf("file size: %d\n", (int)file_stat.st_size);
   if(0==file_stat.st_size)
   {
     lseek(fd, sizeof(QCBITMAP), SEEK_SET);
     write(fd, "", 1);
   }

   /*MAP_PRIVATE will not write data back to file, but MAP_SHARED will write the data back to file*/
//   pstQcBitmap = (QCBITMAP *)mmap(NULL, sizeof(QCBITMAP), PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
   pstQcBitmap = (QCBITMAP *)mmap(NULL, sizeof(QCBITMAP), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
   if(MAP_FAILED == pstQcBitmap)
   {
      return 0;
   }
   printf("Before setting:\n");
   printf("uiMisc: 0x%0x\n", pstQcBitmap->uiMisc);
   printf("uiPstn: 0x%0x\n", pstQcBitmap->uiPstn);
   printf("uiComm: 0x%0x\n", pstQcBitmap->uiComm);
   printf("uiOptm: 0x%0x\n", pstQcBitmap->uiOptm);

   set_qcbitmap(uiMisc, LED);
   set_qcbitmap(uiPstn, PSTN1);
   set_qcbitmap(uiPstn, PSTN2);

   printf("After setting:\n");
   printf("uiMisc: 0x%0x\n", pstQcBitmap->uiMisc);
   printf("uiPstn: 0x%0x\n", pstQcBitmap->uiPstn);
   printf("uiComm: 0x%0x\n", pstQcBitmap->uiComm);
   printf("uiOptm: 0x%0x\n", pstQcBitmap->uiOptm);

   munmap(pstQcBitmap, sizeof(QCBITMAP));
   close(fd);

   return 0;
}


