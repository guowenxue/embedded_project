//header: sysctl-exam.h
#ifndef _SYSCTL_EXAM_H
#define _SYSCTL_EXAM_H
#include <linux/sysctl.h>

#define MY_ROOT (CTL_CPU + 10)
#define MY_MAX_SIZE 256

enum {
        MY_INT_EXAM = 1,
        MY_STRING_EXAM = 2,
};
#endif
