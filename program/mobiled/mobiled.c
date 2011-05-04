
#include "mobiled.h"

unsigned char g_ucDebug = DISABLE;
COM_PORT *g_pstCommPort;        /*The comport which used to comunicate with PC */

static inline void print_version(void);
static inline void print_usage(char *pcPath);

int main(int argc, char **argv)
{
    int iOpt = 0;
    GPRS stGprs;
    unsigned char ucRet = 0x00;
    unsigned char ucSim = SIM1;

    struct option long_options[] = {
        {"debug", no_argument, NULL, 'd'},
        {"version", no_argument, NULL, 'v'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    while ((iOpt = getopt_long(argc, argv, "dvh", long_options, NULL)) != -1)
    {
        switch (iOpt)
        {
          case 'd':
              g_ucDebug = ENABLE;
              break;

          case 'h':
              print_usage(argv[1]);

          case 'v':
              print_version();
              return 0;

          default:
              printf("Unsupport option %c\n", iOpt);
              break;
        }
    }

    dbg_print("Initialize GPRS module.\n");
    if (SUCCESS != gprs_init(&stGprs, GSM_GTM900B, ucSim))
    {
        dbg_print("GPRS module initialize failure.\n");
        return FAILED;
    }

#if 0
    dbg_print("Detect SIM card %d.\n", ucSim);
    if (CLOSED != detect_simdoor_status(stGprs.iCtrlport, ucSim))
    {
        dbg_print("SIM card door not closed.\n");
        return FAILED;
    }
#endif

    dbg_print("Power on GPRS.\n");
    gprs_power_on(&stGprs);

    ucRet = check_simcard_validation(&(stGprs.stComport));
    if (SUCCESS == ucRet)
    {
        dbg_print("GPRS module test success.\n");
    }
    else
    {
        dbg_print("GPRS module test failure.\n");
    }

    gprs_power_off(&stGprs);

    return 0;
}

static inline void print_version(void)
{
    printf(" Version: %d.%d.%d Build %d\n", MAJOR, MINOR, REVER, SVNVER);
    printf(" Copyright (C) 2011 GHL Systems Berhad.\n");
}

static inline void print_usage(char *pcPath)
{
    printf("Usage: %s [-d] [-f] [-h] [-v]\n", pcPath);
    printf("Mandatory arguments to long options are mandatory for short options too.\n\n");
    printf(" -f, --force:\tForce to do QC test again\n");
    printf(" -d, --debug:\tEnable debug output.\n");
    printf(" -h, --help:\tDisplay This help information.\n");
    printf(" -v, --version:\tGet version information.\n");
    return;
}
