
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "eeprom.h"
#define dbg_print(format,args...)  printf(format, ##args)

static unsigned char s_ucChipAddr = DEF_CHIP_ADDR;

/*
 * N300 have 2 EEPROM, an EEPROM on card and an EEPROM on backplane
 * The ucWhich argument should be EEPROM_CARD or EEPROM_BACKPLANE
 */
unsigned char eeprom_init(unsigned char ucWhich)
{
    int iRet;

    if (EEPROM_CARD == ucWhich)
    {
#if (defined CPU_W90P710)
        iRet = device_eep_init(NULL);
#elif (defined CPU_AT91SAM9260)
        iRet = device_eep_init("/dev/eeprom");
#endif
    }
#if (defined PLAT_N300)         /*Only N300 support backplane EEPROM */
    else if (EEPROM_BACK == ucWhich)
    {
        iRet = device_eep_init("/dev/bp-eeprom");
    }
#endif
    else
    {
        dbg_print("Not support this EEPROM device.\n");
        return FAILED;
    }

    if (iRet < 0)
    {
        dbg_print("EEPROM device open failure.\n");
        return FAILED;
    }

    ll_PowerOn();
    return SUCCESS;
}

void eeprom_term(void)
{
    ll_PowerOff();
    device_eep_term();
}

unsigned char test_MTZ(unsigned char ucWhich)
{
    unsigned char ucData[2] = { 0x5a, 0xa5 };
    unsigned char ucRet = SUCCESS;

    if (SUCCESS != eeprom_init(ucWhich))    /*Initi EEPROM device */
    {
        dbg_print("Init EEPROM MTZ failure");
        return FAILED;
    }

    ucRet = cm_WriteConfigZone(s_ucChipAddr, MTZ_ADDR, ucData, 2, FALSE);
    if (SUCCESS != ucRet)
    {
        dbg_print("EEPROM MTZ write failure.\n");
        goto CleanUp;
    }

    memset(ucData, 0, sizeof(ucData));
    ucRet = cm_ReadConfigZone(s_ucChipAddr, MTZ_ADDR, ucData, sizeof(ucData));
    if (SUCCESS != ucRet)
    {
        dbg_print("EEPROM MTZ read failure.\n");
        goto CleanUp;
    }
    dbg_print("Test MTZ[0x%02x 0x%02x]         [OK]\n", ucData[0], ucData[1]);

  CleanUp:
    eeprom_term();              /*Close and power off EEPROM device */
    return ucRet;
}

unsigned char write_eeprom_qcflag(unsigned char ucWhich, unsigned char ucFlag)
{
    unsigned char ucRet = FAILED;
    unsigned char ucTmpFlag = ucFlag;

    if (SUCCESS != eeprom_init(ucWhich))    /*Initi EEPROM device */
    {
        dbg_print("Write QC flag to EEPROM failure.\n");
        return FAILED;
    }

    ucRet = cm_SetUserZone(s_ucChipAddr, USERZONE3, FALSE);
    if (SUCCESS != ucRet)
    {
        dbg_print("Set userzone to write QC flag failure.\n");
        ucRet = 0x01;
        goto CleanUp;
    }

    ucRet = cm_WriteSmallZone(s_ucChipAddr, QCFLAG_ADDR, &ucTmpFlag, 1);
    if (SUCCESS != ucRet)
    {
        dbg_print("Write QC flag failure.\n");
        ucRet = 0x02;
        goto CleanUp;
    }
    dbg_print("Write QC Flag[0x%02x]         [OK]\n", ucTmpFlag);

  CleanUp:
    eeprom_term();              /*Close and power off EEPROM device */
    return ucRet;
}

unsigned char read_eeprom_qcflag(unsigned char ucWhich)
{
    unsigned char ucRet;
    unsigned char ucFlag = 0xFF;

    if (SUCCESS != eeprom_init(ucWhich))    /*Initi EEPROM device */
    {
        dbg_print("Read QC flag from EEPROM failure.\n");
        return FAILED;
    }

    ucRet = cm_SetUserZone(s_ucChipAddr, USERZONE3, FALSE);
    if (SUCCESS != ucRet)
    {
        dbg_print("Set userzone to read QC flag failure.\n");
        ucRet = 0x01;
        goto CleanUp;
    }

    ucRet = cm_ReadSmallZone(s_ucChipAddr, QCFLAG_ADDR, &ucFlag, 1);
    if (SUCCESS != ucRet)
    {
        dbg_print("Read QC flag failure.\n");
        ucRet = 0x02;
        goto CleanUp;
    }

    dbg_print("Read QC Flag[0x%02x]          [OK]\n", ucFlag);

  CleanUp:
    eeprom_term();              /*Close and power off EEPROM device */
    return ucFlag;
}

int main(int argc, char **argv)
{
     unsigned char ucQcFlag = 0x00;
     test_MTZ(EEPROM_CARD);


     ucQcFlag = read_eeprom_qcflag(EEPROM_CARD);
     printf("QC Flag=0x%02x\n", ucQcFlag);

     return 0;
}

#if 0
unsigned char test_eeprom(void)
{
    return test_MTZ();

    unsigned char ucQcFlag;
    ucQcFlag = 0xFE;
    write_eeprom_qcflag(&ucQcFlag);

    ucQcFlag = 0x00;
    read_eeprom_qcflag(&ucQcFlag);
    printf("QC Flag=0x%02x\n", ucQcFlag);
}
#endif
