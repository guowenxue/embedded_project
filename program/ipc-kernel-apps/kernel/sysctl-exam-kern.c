//kernel module: sysctl-exam-kern.c
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sysctl.h>
#include "sysctl-exam.h"

static char mystring[256];
static int myint;

static struct ctl_table my_sysctl_exam[] = {
        {
                .ctl_name       = MY_INT_EXAM,
                .procname       = "myint",
                .data           = &myint,
                .maxlen         = sizeof(int),
                .mode           = 0666,
                .proc_handler   = &proc_dointvec,
        },
        {
                .ctl_name       = MY_STRING_EXAM,
                .procname       = "mystring",
                .data           = mystring,
                .maxlen         = MY_MAX_SIZE,
                .mode           = 0666,
                .proc_handler   = &proc_dostring,
                .strategy       = &sysctl_string,
        }£¬
        {
                .ctl_name = 0
        }
};

static struct ctl_table my_root = {
        .ctl_name       = MY_ROOT,
        .procname       = "mysysctl",
        .mode           = 0555,
        .child          = my_sysctl_exam,
};

static struct ctl_table_header * my_ctl_header;

static int __init sysctl_exam_init(void)
{
        my_ctl_header = register_sysctl_table(&my_root, 0);

        return 0;
}

static void __exit sysctl_exam_exit(void)
{
        unregister_sysctl_table(my_ctl_header);
}

module_init(sysctl_exam_init);
module_exit(sysctl_exam_exit);
MODULE_LICENSE("GPL");
