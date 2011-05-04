#include  <stdio.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <string.h>
#include  <sys/types.h>
#include  <fcntl.h>
#include  <sys/stat.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include  <errno.h>
#include  <sys/select.h>
#include  <sys/wait.h>
#include  <getopt.h>

#include  <common/common.h>
#include  <common/logs.h>
#include  "version.h"

#define   DBG_DEFAULT LOG_DISB
//g_ucDebugLev = DBG_DEFAULT;   /* defined in common/logs.{c/h} */
//g_ucStop = 0x00;  /* defined in common/common.{c/h} */

int main(int argc, char **argv)
{
    int                   opt = 0;  /*used by getopt_long*/
    unsigned char         setToForeground = 0x00;
    unsigned char         ucDebug= 0x00;
    struct sigaction      sigact,sigign;

    char                  prg_name[PRGNAME_LEN]={0};
    char                  pid_file[PIDFILE_LEN]={0};

    struct option long_options[] =
    {
        {"foreground", no_argument,       NULL, 'f'},
        {"debug",      required_argument, NULL, 'd'},
        {"version",    no_argument,       NULL, 'v'},
        {"help",       no_argument,       NULL, 'h'},
        {NULL,         0,                 NULL,  0 }
    };

    while ((opt = getopt_long(argc, argv, "fd:vh", long_options, NULL)) != -1)
    {
        switch(opt)
        {
           case 'f':
                setToForeground = 0x01;
                break;
           case 'd':       /*debug*/
                if (NULL != optarg)
                {
                    int iLevel = atoi(optarg);
                    /*Lowest level is LOG_CRIG*/
                    g_ucDebugLev = iLevel > LOG_MAX ? LOG_MAX : iLevel; 
                    ucDebug = 0x01;
                }
                break;
           case 'v':        /*version*/
                print_version(argv[0]);
                return   EXIT_SUCCESS;
           case 'h':         /*help*/
                usage(argv[0]);
                return 0;
           default:
                break;
        }/*end of "switch(opt)"*/
    }

    /*Get the process pid to PID file */
    {
       char    *ptr = NULL;
       ptr = strdup(argv[0]);
       strncpy(prg_name, basename(ptr), PRGNAME_LEN);
       snprintf(pid_file, PIDFILE_LEN, "/tmp/%s/%s.pid", prg_name, prg_name );
       free(ptr);
    }


    if(RUNNING == is_myself_running(pid_file) )
    {
        logs(LOG_CRIT, "Programe running and exit now.\n");
        exit(0); 
    }

    /* Put program running in background*/
    if(!setToForeground )
    {
#ifdef LINUX
        daemon(0, ucDebug);

#elif defined UCLINUX
        daemonize(ucDebug, argc, argv);
#endif
    }

    record_runpid_file(pid_file);      

    /*Install signal.*/
    sigemptyset (&sigact.sa_mask); 
    sigact.sa_flags = 0; 
    sigact.sa_handler = signal_handler; 
    
    sigemptyset (&sigign.sa_mask); 
    sigign.sa_flags = 0; 
    sigign.sa_handler = SIG_IGN;

    sigaction (SIGTERM, &sigact, 0);    /* catch terminate signal "kill" command*/
    sigaction (SIGINT,  &sigact, 0);    /* catch interrupt signal CTRL+C */
    sigaction (SIGSEGV, &sigact, 0);    /* catch segmentation faults  */
    sigaction (SIGCHLD, &sigact, 0);    /* catch child process return */
    sigaction (SIGPIPE, &sigact, 0);    /* catch broken pipe */

    while(!g_ucStop)
    {
         sleep(1);
    }

    logs(LOG_INFO, "Exit now.\n");
    remove(pid_file);

    return 0;
}


