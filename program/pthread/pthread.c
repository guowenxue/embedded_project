/*********************************************************************************
 *      Copyright:  (C) 2011 (C) 2011 China Undergraduate Embedded League(CUEL)
 *                  All rights reserved.
 *
 *       Filename:  thread.c
 *    Description:  An introduction about how to program in multiple threads
 *                 
 *        Version:  1.0.0(10/25/2011~)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "10/25/2011 10:20:03 AM"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct __COMMAND
{
     int   time;
     char  desc[18];
} COMMAND;

typedef void *(THREAD_BODY) (void *thread_arg);
unsigned char g_ucStop = 0x00;
void signal_handler(int signo);
void *thread1_workbody(void *arg);
void *thread2_workbody(void *arg);
int thread_start(pthread_t * thread_id, THREAD_BODY * thread_workbody, void *thread_arg);

/********************************************************************************
 *  Description:
 *   Input Args:
 *  Output Args:
 * Return Value:
 ********************************************************************************/
int main(int argc, char **argv)
{
    struct sigaction sigact, sigign;
    pthread_t thread1_id;
    pthread_t thread2_id;

    COMMAND   command[2];

    /*  Install signal. */
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = signal_handler;

    sigemptyset(&sigign.sa_mask);
    sigign.sa_flags = 0;
    sigign.sa_handler = SIG_IGN;

    sigaction(SIGTERM, &sigact, 0); /*   catch terminate signal "kill" command */
    sigaction(SIGINT, &sigact, 0);  /*   catch interrupt signal CTRL+C */
    sigaction(SIGSEGV, &sigact, 0); /*   catch segmentation faults  */
    sigaction(SIGCHLD, &sigact, 0); /*   catch child process return */
    sigaction(SIGPIPE, &sigact, 0); /*   catch broken pipe */
    sigaction(SIGUSR1, &sigact, 0); /*   catch the requestion of SMS_GATEWAY wants to hold GPRS module */
    sigaction(SIGUSR2, &sigact, 0); /*   catch the requestion of Netrouter wants to hold GPRS module */

    memset(&command[0], 0, sizeof(COMMAND));
    command[0].time = 1;
    strncpy(command[0].desc, "hard work now!", sizeof(command[0].desc));
    thread_start(&thread1_id, thread1_workbody, (void *)&command[0]);

    memset(&command[1], 0, sizeof(COMMAND));
    command[1].time = 2;
    strncpy(command[1].desc, "lazy work now!", sizeof(command[1].desc));
    thread_start(&thread2_id, thread2_workbody, (void *)&command[1]);

    while (!g_ucStop)
    {
        sleep(1);
    }

    printf("Main thread wait child thread exit now:\n");
    pthread_join(thread1_id, NULL);
    printf("Main thread wait thread1 exit ok.\n");
    pthread_join(thread2_id, NULL);
    printf("Main thread wait thread2 exit ok.\n");

    return 0;
}                               /* ----- End of main() ----- */

void *thread1_workbody(void *arg)
{
    COMMAND *command = (COMMAND *)arg;
    while (!g_ucStop)
    {
        printf("Thread1 %s\n", command->desc);
        sleep(command->time);
    }
    printf("$Pthread1 exit now.\n");
    pthread_exit(NULL);
}

void *thread2_workbody(void *arg)
{
    COMMAND *command = (COMMAND *)arg; 
    while (!g_ucStop)
    {
        printf("Thread2 %s\n", command->desc);
        sleep(command->time);
    }
    printf("$Pthread2 exit now.\n");
    pthread_exit(NULL);
}

int thread_start(pthread_t * thread_id, THREAD_BODY * thread_workbody, void *thread_arg)
{
    pthread_attr_t thread_attr;

    /* Initialize the thread  attribute */
    pthread_attr_init(&thread_attr);

    /* Set the stack size of the thread */
    pthread_attr_setstacksize(&thread_attr, 120 * 1024);

    /* Set thread to detached state:Don`t need pthread_join */
    //pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    /* Create the thread */
    pthread_create(thread_id, &thread_attr, thread_workbody, thread_arg);

    /* Destroy the  attributes  of  thread */
    pthread_attr_destroy(&thread_attr);

    return 0;
}

void signal_handler(int signo)
{
    if (SIGTERM == signo || SIGINT == signo)
    {
        printf("Receive signal %d, set g_ucStop and exit now\n", signo);
        g_ucStop = 0x01;
    }

    else if (SIGSEGV == signo)
    {
        printf("Segment fault error occured!\n");
        _exit(1);
    }
}
