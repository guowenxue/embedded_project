//filename: kern-boot-params.c
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>

#define MAX_SIZE 5
static int setup_example_int;
static int setup_example_int_array[MAX_SIZE];
static char setup_example_string[16];

static int __init parse_int(char * s)
{
        int ret;

        ret = get_option(&s, &setup_example_int);
        if (ret == 1) {
                printk("setup_example_int=%d\n", setup_example_int);
        }
        return 1;
}

static int __init parse_int_string(char *s)
{
        char * ret_str;
        int i;

        ret_str = get_options(s, MAX_SIZE, setup_example_int_array);
        if (*ret_str != '\0') {
                printk("incorrect setup_example_int_array paramters: %s\n", ret_str);
        }
        else {
                printk("setup_example_int_array=");
                for (i=1; i<MAX_SIZE; i++) {
                        printk("%d", setup_example_int_array[i]);
                        if (i < (MAX_SIZE -1)) {
                                printk(",");
                        }
                }
                printk("\n");
                printk("setup_example_int_array includes %d intergers\n", setup_example_int_array[0]);
        }
        return 1;
}

static int __init parse_string(char *s)
{
        if (strlen(s) > 15) {
                printk("Too long setup_example_string parameter, \n");
                printk("maximum length is less than or equal to 15\n");
        }
        else {
                memcpy(setup_example_string, s, strlen(s) + 1);
                printk("setup_example_string=%s\n", setup_example_string);
        }
        return 1;
}

__setup("setup_example_int=", parse_int);
__setup("setup_example_int_array=", parse_int_string);
__setup("setup_example_string=", parse_string);
