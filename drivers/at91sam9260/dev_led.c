/*********************************************************************************
 *  Copyright(c)  2011, GuoWenxue<guowenxue@gmail.com>.
 *  All ringhts reserved.
 *
 *     Filename:  dev_led.c
 *  Description:  AT91SAM9260 platform LED driver.
 *
 *     ChangLog:
 *      1,   Version: 1.0.0
 *              Date: 2011-05-02
 *            Author: guowenxue <guowenxue@gmail.com>
 *       Descrtipion: Initial first version
 *
 ********************************************************************************/

#include "include/plat_driver.h"

#define DRV_AUTHOR                "GuoWenxue<guowenxue@gmail.com>"
#define DRV_DESC                  "AT91SAM9260 platform LED driver"

/*Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  DEV_LED_NAME

#ifdef LED_DRIVER_DEBUG
int debug = ENABLE;
#else
int debug = DISABLE;
#endif

static int dev_major = DEV_LED_MAJOR;
static int dev_minor = 0;

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);
module_param(dev_minor, int, S_IRUGO);

static const int led_gpio [LED_COUNT] = {LED_RUN_PIN, LED1_PIN, LED2_PIN};
struct timer_list blink_timer;

DECLARE_MUTEX(led_sem);

enum 
{
   RUN_LED = 0,
   LDE1,
   LED2,
};

#define ON            1
#define OFF           0
#define BIT_CMD       0  /* led_status bit[0]: 1->LED On 0->LED Off[0] */
#define BIT_BLINK     1  /* led_status bit[1]: 1->Blink mode, 0->common mode */
static unsigned char led_status[LED_COUNT];

#define dbg_print(format,args...) if(DISABLE!=debug) {printk("[%s] ", DEV_NAME);printk(format, ##args);}

struct cdev *dev_cdev = NULL;
static struct class * dev_class;

static int dev_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int dev_release(struct inode *inode, struct file *file)
{
    return 0;
}

static int dev_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    int  index = NUM(inode->i_rdev); /* Which LED */
    dbg_print("Come into %s() with cmd=%u arg=%ld\n", __FUNCTION__, cmd, arg);
    switch (cmd)
    {
      case LED_ON:
          down_interruptible(&led_sem);
          dbg_print("Turn LED%d on\n", index);
          CLR_BIT(led_status[index], BIT_BLINK);  /* Don't blink */
          SET_BIT(led_status[index], BIT_CMD);  /* Turn LED on */
          up(&led_sem);
          break;

      case LED_OFF: 
          down_interruptible(&led_sem);
          dbg_print("Turn LED%d off\n", index);
          CLR_BIT(led_status[index], BIT_BLINK);  /* Don't blink */
          CLR_BIT(led_status[index], BIT_CMD);  /* Turn LED off */
          up(&led_sem);
          break;

      case LED_BLINK: 
          down_interruptible(&led_sem);
          dbg_print("Turn LED%d blink\n", index);
          SET_BIT(led_status[index], BIT_BLINK);  /* Turn LED blink */
          up(&led_sem);
          break;

      case SET_DRV_DEBUG:
          printk("%s %s driver debug now.\n", DISABLE == arg ? "Disable" : "Enable", DEV_NAME);

          if (0 == arg)
              debug = DISABLE;
          else
              debug = ENABLE;
          return debug;

      case GET_DRV_VER:
          print_version(DRV_VERSION);
          return DRV_VERSION;

      default:
          printk("%s driver don't support ioctl command=%d\n", DEV_NAME, cmd);
          return -ENOTTY;
    }

    return 0;
}

static struct file_operations dev_fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .ioctl = dev_ioctl,
};

static void blink_timer_handle(unsigned long arg)
{
    int  i,  level;

    for(i=0; i<LED_COUNT; i++) 
    {
       level = (ON==GET_BIT(led_status[i], BIT_CMD) ? LOWLEVEL : HIGHLEVEL);

       /* Set GPIO low level will turn LED on, high level will turn led off */
       at91_set_gpio_value(led_gpio[i], level);  

       if(ON==GET_BIT(led_status[i], BIT_BLINK))  /* LED is in blink mode */
       {
           down_interruptible(&led_sem);
           /* Switch the LED status */
           if(ON==GET_BIT(led_status[i], BIT_CMD)) 
           {
               CLR_BIT(led_status[i], BIT_CMD); /* Turn LED off */ 
           }
           else
           {
               SET_BIT(led_status[i], BIT_CMD); /* Turn LED on */
           }
           up(&led_sem);
       }
    }
    mod_timer(&blink_timer, jiffies + HZ/2);
}


static void led_cleanup(void)
{
    int i;

    del_timer(&blink_timer);

    for (i=0; i<LED_COUNT; i++)
    { 
        at91_set_gpio_value(led_gpio[i], HIGHLEVEL);  /* Turn all LED off */
        device_destroy(dev_class, MKDEV(dev_major, i)); 
    }
    class_destroy (dev_class);

    cdev_del(dev_cdev);
    unregister_chrdev_region(MKDEV(dev_major, 0), LED_COUNT);

    printk("%s driver removed\n", DEV_NAME);
}

static int __init led_init(void)
{
    int result;
    int i;
    dev_t devno;

    /*Alloc for the device for driver */
    if (0 != dev_major)
    {
        devno = MKDEV(dev_major, dev_minor);
        result = register_chrdev_region(devno, LED_COUNT, DEV_NAME);
    }
    else
    {
        result = alloc_chrdev_region(&devno, dev_minor, LED_COUNT, DEV_NAME);
        dev_major = MAJOR(devno);
    }

    /*Alloc for device major failure */
    if (result < 0)
    {
        printk("%s driver can't get major %d\n", DEV_NAME, dev_major);
        return result;
    }

    /*Alloc cdev structure */
    dev_cdev = cdev_alloc();;
    if (NULL == dev_cdev)
    {
        printk("%s driver can't alloc for dev_cdev\n", DEV_NAME);
        goto ERROR;
    }

    /*Initialize cdev structure and register it */
    dev_cdev->owner = THIS_MODULE;
    dev_cdev->ops = &dev_fops;
    result = cdev_add(dev_cdev, devno, LED_COUNT);
    if (0 != result)
    {
        printk("%s driver can't regist dev_cdev\n", DEV_NAME);
        goto ERROR;
    }

    /*Create device /dev/$DEV_NAME */
    dev_class = class_create (THIS_MODULE, DEV_NAME);
    if (IS_ERR(dev_class)) 
    { 
        printk("ERROR: %s driver Can't create dev_class.\n", DEV_NAME); 
        result = -1; 
        goto ERROR; 
    }

    for(i=0; i<LED_COUNT; i++)
    {
        devno = MKDEV(dev_major, i);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,24)
        device_create (dev_class, NULL, devno, NULL, "%s%d", DEV_NAME, i);
#else
        device_create (dev_class, NULL, devno, "%s%d", DEV_NAME, i);
#endif
    }

    /*  Initial the LED GPIO Port */
    for(i=0; i<LED_COUNT; i++)
    {
        gpio_direction_output( led_gpio[i], HIGHLEVEL);
    }

    /* Initial the LED blink timer */ 
    init_timer(&blink_timer);
    blink_timer.function = &blink_timer_handle;
    blink_timer.expires = jiffies + HZ;
    add_timer(&blink_timer);

    printk("%s driver version %d.%d.%d initiliazed\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER, DRV_REVER_VER);

    return 0;

  ERROR:
    led_cleanup();
    return -ENODEV;

}

module_init(led_init);
module_exit(led_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);
