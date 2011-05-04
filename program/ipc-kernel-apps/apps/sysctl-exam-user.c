//application: sysctl-exam-user.c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/sysctl.h>
#include "sysctl-exam.h"


_syscall1(int, _sysctl, struct __sysctl_args *, args);

int sysctl(int *name, int nlen, void *oldval, size_t *oldlenp,
                void *newval, size_t newlen)
{
        struct __sysctl_args args={name,nlen,oldval,oldlenp,newval,newlen};
        return _sysctl(&args);
}

#define SIZE(x) sizeof(x)/sizeof(x[0])
#define OSNAMESZ 100

int oldmyint;
int oldmyintlen;

int newmyint;
int newmyintlen;

char oldmystring[MY_MAX_SIZE];
int oldmystringlen;

char newmystring[MY_MAX_SIZE];
int newmystringlen;

int myintctl[] = {MY_ROOT, MY_INT_EXAM};
int mystringctl[] = {MY_ROOT, MY_STRING_EXAM};

int main(int argc, char ** argv)
{
        if (argc < 2) {
                oldmyintlen = sizeof(int);
                if (sysctl(myintctl, SIZE(myintctl), &oldmyint, &oldmyintlen, 0, 0)) {
                        perror("sysctl");
                        exit(-1);
                }
                else {
                        printf("mysysctl.myint = %d\n", oldmyint);
                }

                oldmystringlen = MY_MAX_SIZE;
                if (sysctl(mystringctl, SIZE(mystringctl), oldmystring, &oldmystringlen, 0, 0)) {
                        perror("sysctl");
                        exit(-1);
                }
                else {
                        printf("mysysctl.mystring = \"%s\"\n", oldmystring);
                }
        }
        else if (argc != 3) {
                printf("Usage:\n");
                printf("\tsysctl-exam-user\n");
                printf("Or\n");
                printf("\tsysctl-exam-user aint astring\n");
        }
        else {
                newmyint = atoi(argv[1]);
                newmyintlen = sizeof(int);
                oldmyintlen = sizeof(int);

                strcpy(newmystring, argv[2]);
                newmystringlen = strlen(newmystring);
                oldmystringlen = MY_MAX_SIZE;

                if (sysctl(myintctl, SIZE(myintctl), &oldmyint, &oldmyintlen, &newmyint, newmyintlen)) {
                        perror("sysctl");
                        exit(-1);
                }
                else {
                        printf("old value: mysysctl.myint = %d\n", oldmyint);
                        printf("new value: mysysctl.myint = %d\n", newmyint);
                }


                if (sysctl(mystringctl, SIZE(mystringctl), oldmystring, &oldmystringlen, newmystring, newmystringlen)) {
                        perror("sysctl");
                        exit(-1);
                }
                else {
                        printf("old vale: mysysctl.mystring = \"%s\"\n", oldmystring);
                        printf("new value: mysysctl.mystring = \"%s\"\n", newmystring);
                }
        }

        exit(0);
}
