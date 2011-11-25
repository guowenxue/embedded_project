/*********************************************************************************
 *      Copyright:  (C) 2011 GuoWenxue<guowenxue@gmail.com> 
 *                  All rights reserved.
 *
 *       Filename:  usb_gsm.c
 *    Description:  This is the common 3G USB GSM module driver
 *                 
 *        Version:  1.0.0(11/25/2011~)
 *         Author:  Guo Wenxue <guowenxue@gmail.com>
 *      ChangeLog:  1, Release initial version on "11/25/2011 04:19:31 PM"
 *                 
 ********************************************************************************/
#include "at91_driver.h"

#define DRV_AUTHOR                "Guo Wenxue <guowenxue@gmail.com>"
#define DRV_DESC                  "AT91SAM9XXX USB(3G) GSM Module driver"

/*  Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define GSM_TTY_NAME              "ttyGSM"
#define GSM_TTY_MAJOR             0  /*  Use 0 to Dynamic alloc */
#define GSM_TTY_MINORSTART        0 
#define GSM_TTY_MINORS            3

static int debug = DISABLE;
static int dev_major = GSM_TTY_MAJOR;
static int dev_minor = GSM_TTY_MINORSTART;

/*  Define the 3G USB GPRS module USB ID here */
#define SIMCOM_VENDOR_ID                0x05C6
#define SIMCOM_PRODUCT_SIM5215          0x9000
#define SIMCOM_PRODUCT_SIM5216          0x9000

static struct tty_driver *gsm_tty_driver;

/*  Table of devices that work with this driver */
static struct usb_device_id gsm_serial_table [] = 
{
    { USB_DEVICE(SIMCOM_VENDOR_ID, SIMCOM_PRODUCT_SIM5215) },
    { }  
};
MODULE_DEVICE_TABLE (usb, gsm_serial_table);


struct usb_gsm
{
    struct usb_device * udev;               /*  the usb device for this device  */
    struct usb_interface *  interface;      /*  the interface for this device */
    unsigned char *     bulk_in_buffer;     /*  the buffer to receive data */
    size_t          bulk_in_size;           /*  the size of the receive buffer */
    __u8            bulk_in_endpointAddr;   /*  the address of the bulk in endpoint */
    __u8            bulk_out_endpointAddr;  /*  the address of the bulk out endpoint */
    struct kref     kref;
};

#define to_gsm_dev(d) container_of(d, struct usb_gsm, kref)

static struct usb_driver gsm_usb_driver;

static int gsm_tty_open(struct tty_struct *tty, struct file *filp)
{ 
    return 0;
}

static void gsm_tty_close(struct tty_struct *tty, struct file *filp)
{ 
    return;
}

static int gsm_tty_write(struct tty_struct *tty, const unsigned char *buffer, int count)
{ 
    return 0;
}

static int gsm_tty_put_char(struct tty_struct *tty, unsigned char ch)
{ 
    return 0;
}

static int gsm_tty_write_room(struct tty_struct *tty)
{ 
    return 0;
}

static void gsm_tty_set_termios(struct tty_struct *tty, struct ktermios *old)
{ 
    return ;
}

static struct tty_operations gsm_tty_ops =
{ 
    .open = gsm_tty_open, 
    .close = gsm_tty_close, 
    .write = gsm_tty_write, 
    .put_char = gsm_tty_put_char, 
    .write_room = gsm_tty_write_room, 
    .set_termios = gsm_tty_set_termios,
};

static int gsm_serial_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    return 0;
}

static void gsm_serial_disconnect(struct usb_interface *interface)
{
    return;
}

static struct usb_driver gsm_usb_driver = 
{
    .name = "gsmserial",
    .id_table = gsm_serial_table,
    .probe = gsm_serial_probe,
    .disconnect = gsm_serial_disconnect,
};

static int __init usb_gsm_init(void)
{
    int result;

    /* Alloc for the TTY driver */
    gsm_tty_driver = alloc_tty_driver(GSM_TTY_MINORS);
    if(!gsm_tty_driver)
        return -ENOMEM;

    gsm_tty_driver->owner = THIS_MODULE;
    gsm_tty_driver->driver_name = GSM_TTY_NAME; /*  This is the driver name in /proc/tty or sysfs */
    gsm_tty_driver->name = GSM_TTY_NAME;   /*  This is device node name in /dev */
    gsm_tty_driver->major = dev_major;
    gsm_tty_driver->minor_start = dev_minor;

    /*  type and subtype used to describe the TTY driver type, subtype value depens on type.
     *
     * TTY_DRIVER_TYPE_SYSTEM: Internal used by tty subsystem, shouldn't be used by normal TTY driver.
     *      subtype: SYSTEM_TYPE_TTY, SYSTEM_TYPE_CONSOLE, SYSTEM_TYPE_SYSCONS, SYSTEM_TYPE_SYSPTMX
     * TTY_DRIVER_TYPE_CONSOLE: Only used by console driver
     *      subtype: SERIAL_TYPE_NORMAL, SERIAL_TYPE_CALLOUT
     * TTY_DRIVER_TYPE_SERIAL: Used by any serial port type driver
     *      subtype: SERIAL_TYPE_NORMAL, SERIAL_TYPE_CALLOUT
     * TTY_DRIVER_TYPE_PTY: Used by pty driver
     *      subtype: PTY_TYPE_MASTER, PTY_TYPE_SLAVE
     */
    gsm_tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
    gsm_tty_driver->subtype = SERIAL_TYPE_NORMAL;

    gsm_tty_driver->flags = TTY_DRIVER_REAL_RAW;
    gsm_tty_driver->init_termios = tty_std_termios;
    gsm_tty_driver->init_termios.c_cflag = B115200 | CS8 | CREAD | HUPCL | CLOCAL;

    tty_set_operations (gsm_tty_driver, &gsm_tty_ops);

    /* Register the TTY driver  */
    result = tty_register_driver(gsm_tty_driver);
    if(result)
    {
        printk(KERN_ERR "failed to register %s driver\n", GSM_TTY_NAME);
        put_tty_driver(gsm_tty_driver);
        return result;
    }

    /*  register this USB driver with the USB subsystem  */
    result = usb_register(&gsm_usb_driver);
    if (result)
    {
        goto ERROR;
        printk("usb_register failed. Error number %d", result);
    }

    printk(KERN_INFO DRV_DESC " %d.%d.%d\n", DRV_MAJOR_VER, DRV_MINOR_VER, DRV_REVER_VER);
    return result;
ERROR:
    tty_unregister_driver(gsm_tty_driver);
    put_tty_driver(gsm_tty_driver);
    return result;
}

static void __exit usb_gsm_exit(void)
{
    /*  deregister the TTY driver */
    tty_unregister_driver(gsm_tty_driver);
    put_tty_driver(gsm_tty_driver);

    /*  deregister the USB driver */
    usb_deregister(&gsm_usb_driver);
}

module_init (usb_gsm_init);
module_exit (usb_gsm_exit);

module_param(debug, int, S_IRUGO);
module_param(dev_major, int, S_IRUGO);
module_param(dev_minor, int, S_IRUGO);

MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:AT91SAM9260_GSM_TTY");

