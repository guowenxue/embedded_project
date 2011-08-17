/*********************************************************************************
 *  Copyright(c)  2011, GHL Systems Berhad.
 *  All ringhts reserved.
 *
 *     Filename:  dev_gpio.c
 *  Description:  GHL netAccess common char device GPIO driver
 *
 *     ChangLog:
 *      1,   Version: 2.0.0
 *              Date: 2011-04-08
 *            Author: guoqingdong <guoqingdong@ghlsystems.com>
 *       Descrtipion: Initial first version
 *
 ********************************************************************************/

#include "include/plat_driver.h"

#define DRV_AUTHOR                "GuoQingDong<guoqingdong@ghlsystems.com>"
#define DRV_DESC                  "GHL netAccess GPIO module driver"

/*Driver version*/
#define DRV_MAJOR_VER             2
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  DEV_GPIO_NAME

#ifndef DEV_MAJOR
#define DEV_MAJOR                 0 /*dynamic major by default */
#endif

static struct cdev dev_cdev;
static struct class *dev_class = NULL;

static int debug = DISABLE;
static int dev_major = DEV_MAJOR;
static int dev_minor = 0;

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);
module_param(dev_minor, int, S_IRUGO);

#define dbg_print(format,args...) if(DISABLE!=debug) \
        {printk("[kernel] ");printk(format, ##args);}    


static int GPIO_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int GPIO_release(struct inode *inode, struct file *file)
{
    return 0;
}

#define GPRS_ON_PIN_CMD         4
#define GPRS_38V_PIN_CMD        5
#define GPRS_VBUS_PIN_CMD       6
#define DETECT_RESTORE_KEY      100
#define DETECT_FUNC_SWITCH      101

//compatible with kernel version 2.6.38 
static long GPIO_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd)
    {
        case SET_DRV_DEBUG_OLD:
		case SET_DRV_DEBUG:
			dbg_print("%s driver debug now.\n", DISABLE == arg ? "Disable" : "Enable");
			if (0 == arg)
				debug = DISABLE;
			else
				debug = ENABLE;
			break;
        
		case GPRS_ON_PIN_CMD:
            printk("Turn GPRS_ON_PIN level=%d\n", (int)arg);
            if(arg == 1)
               at91_set_gpio_output(GPRS_ON_PIN, HIGHLEVEL);
            else
               at91_set_gpio_output(GPRS_ON_PIN, LOWLEVEL);
            break;

		case GPRS_38V_PIN_CMD:
            printk("Turn GPRS_38V_PIN level=%d\n", (int)arg);
            if(arg == 1)
               at91_set_gpio_output(GPRS_38V_ON_PIN, HIGHLEVEL);
            else
               at91_set_gpio_output(GPRS_38V_ON_PIN, LOWLEVEL);
            break;

		case GPRS_VBUS_PIN_CMD:
            printk("Turn GPRS_VBUS_PIN level=%d\n", (int)arg);
            if(arg == 1)
               at91_set_gpio_output(GPRS_VBUS_CTRL_PIN, HIGHLEVEL);
            else
               at91_set_gpio_output(GPRS_VBUS_CTRL_PIN, LOWLEVEL);
            break;

        case DETECT_RESTORE_KEY:
            at91_set_gpio_input (RESTR_KEY, DISPULLUP);
            printk("Restore Key GPIO level: %d\n", at91_get_gpio_value(RESTR_KEY));
            break;

        case DETECT_FUNC_SWITCH:
            at91_set_gpio_input (FUNC_SWITCH_PIN0, DISPULLUP);
            at91_set_gpio_input (FUNC_SWITCH_PIN1, DISPULLUP);
            at91_set_gpio_input (FUNC_SWITCH_PIN2, DISPULLUP);
            at91_set_gpio_input (FUNC_SWITCH_PIN3, DISPULLUP);
            printk("FUNC_SWITCH_PIN0 Key GPIO level: %d\n", at91_get_gpio_value(FUNC_SWITCH_PIN0));
            printk("FUNC_SWITCH_PIN1 Key GPIO level: %d\n", at91_get_gpio_value(FUNC_SWITCH_PIN1));
            printk("FUNC_SWITCH_PIN2 Key GPIO level: %d\n", at91_get_gpio_value(FUNC_SWITCH_PIN2));
            printk("FUNC_SWITCH_PIN3 Key GPIO level: %d\n", at91_get_gpio_value(FUNC_SWITCH_PIN3));
            break;

		default:
			dbg_print("%s driver don't support ioctl command=%d\n", DEV_NAME, cmd);
			return -1;
    }

    return 0;
}

static struct file_operations GPIO_fops = {
    .owner = THIS_MODULE,
    .open = GPIO_open,
    .release = GPIO_release,
    .unlocked_ioctl = GPIO_ioctl, //compatible with kernel version 2.6.38 
};


static void GPIO_cleanup(void)
{
    dev_t devno = MKDEV(dev_major, dev_minor);

    device_destroy(dev_class, devno);   
    class_destroy(dev_class);

    cdev_del(&dev_cdev);

    unregister_chrdev_region(devno, 1);

    printk("%s driver removed\n", DEV_NAME);
}

static int __init GPIO_init(void)
{
    int result;
    dev_t devno;

    /* Alloc for the device for driver */
    if (0 != dev_major)
    {
        devno = MKDEV(dev_major, dev_minor);
        result = register_chrdev_region(devno, 1, DEV_NAME);
    }
    else
    {
        result = alloc_chrdev_region(&devno, dev_minor, 1, DEV_NAME);
        dev_major = MAJOR(devno);
    }

    /*Alloc for device major failure */
    if (result < 0)
    {
        printk("%s driver can't get major %d\n", DEV_NAME, dev_major);
        return result;
    }
	
    /*Initialize cdev structure and register it*/
	cdev_init (&dev_cdev, &GPIO_fops);
	dev_cdev.owner 	= THIS_MODULE;
	
	result = cdev_add (&dev_cdev, devno , 1);
	if (result)
	{
	    printk (KERN_NOTICE "error %d add GPIO device", result);
		goto ERROR;
	}

    dev_class = class_create(THIS_MODULE, DEV_NAME);
    if(IS_ERR(dev_class)) 
    {           
        printk("%s driver create class failture\n",DEV_NAME);           
        result =  -ENOMEM;  
        goto ERROR;   
    }       

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)     
    device_create(dev_class, NULL, devno, NULL, DEV_NAME);
#else
    device_create (dev_class, NULL, devno, DEV_NAME);
#endif  
    
    printk("%s driver version %d.%d.%d initiliazed\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER,
               DRV_REVER_VER);
	return 0;

ERROR:
    cdev_del(&dev_cdev);
    unregister_chrdev_region(devno, 1);
    return result;
}

module_init(GPIO_init);
module_exit(GPIO_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);


