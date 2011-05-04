//filename: module-para-exam.c
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/stat.h>

static int my_invisible_int = 0;
static int my_visible_int = 0;
static char * mystring = "Hello, World";

static int __init exam_module_init(void)
{
        printk("my_invisible_int = %d\n", my_invisible_int);
        printk("my_visible_int = %d\n", my_visible_int);
        printk("mystring = '%s'\n", mystring);
        return 0;
}

static void __exit exam_module_exit(void)
{
        printk("my_invisible_int = %d\n", my_invisible_int);
        printk("my_visible_int = %d\n", my_visible_int);
        printk("mystring = '%s'\n", mystring);
}

module_init(exam_module_init);
module_exit(exam_module_exit);
module_param(my_invisible_int, int, 0);
MODULE_PARM_DESC(my_invisible_int, "An invisible int under sysfs");
module_param(my_visible_int, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(my_visible_int, "An visible int under sysfs");
module_param(mystring, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(mystring, "An visible string under sysfs");
MODULE_AUTHOR("Yang Yi");
MODULE_DESCRIPTION("A module_param example module");
MODULE_LICENSE("GPL");

