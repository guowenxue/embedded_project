/*********************************************************************************
 *  Copyright(c)  2011, Guo Wenxue <guowenxue@gmail.com>
 *  All ringhts reserved.
 *
 *     Filename:  dev_faketty.c
 *  Description:  Common Fake TTY skeleton driver
 *
 *     ChangLog:
 *      1,   Version: 1.0.0
 *              Date: 2011-08-10
 *            Author: Guo Wenxue <guowenxue@gmail.com>
 *       Descrtipion: Initial first version
 *
 ********************************************************************************/

#include "include/plat_driver.h"

#define DRV_AUTHOR                "Guo Wenxue <guowenxue@gmail.com>"
#define DRV_DESC                  "Common fake TTY driver"

/*Driver version*/
#define DRV_MAJOR_VER             1
#define DRV_MINOR_VER             0
#define DRV_REVER_VER             0

#define DEV_NAME                  FAKE_TTY_NAME

static struct tty_driver *fake_tty_driver;


static int fake_tty_open (struct tty_struct *tty, struct file *file)
{
	return 0;
}

static void fake_tty_close(struct tty_struct *tty, struct file * filp)
{
}

static int fake_tty_write (struct tty_struct * tty, const unsigned char *user_buf, int count)
{
	return count;
}

static int fake_tty_write_room (struct tty_struct * tty)
{
	return 0;
}

static int fake_tty_chars_in_buffer (struct tty_struct *tty)
{
	return 0;
}

static struct tty_operations fake_tty_ops = {
	.open				= fake_tty_open,
	.close				= fake_tty_close,
	.write				= fake_tty_write,
	.write_room			= fake_tty_write_room,
	.chars_in_buffer	= fake_tty_chars_in_buffer,
};

void faketty_cleanup(void)
{
	if (fake_tty_driver)
	{
		tty_unregister_driver (fake_tty_driver);
		put_tty_driver (fake_tty_driver);
	}

	printk ("%s driver removed\n",DEV_NAME);
}

int faketty_init(void)
{
	int result;
	
	// allocate the tty driver
	fake_tty_driver = alloc_tty_driver (FAKE_TTY_MINORS);
	
	if (!fake_tty_driver)
		return -ENOMEM;

	// initialize the tty driver
	fake_tty_driver->owner = THIS_MODULE;
	fake_tty_driver->driver_name = FAKE_TTY_DRV_NAME;
	fake_tty_driver->name = FAKE_TTY_NAME;
	fake_tty_driver->major = FAKE_TTY_MAJOR;
	fake_tty_driver->minor_start = FAKE_TTY_MINOR_BASE;
	fake_tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
	fake_tty_driver->subtype = SERIAL_TYPE_NORMAL;
	fake_tty_driver->flags = TTY_DRIVER_REAL_RAW;
	fake_tty_driver->init_termios = tty_std_termios;
	fake_tty_driver->init_termios.c_cflag = B115200 | CS8 | CREAD | HUPCL | CLOCAL;
	
	tty_set_operations (fake_tty_driver, &fake_tty_ops);

	//register tty driver
	result = tty_register_driver (fake_tty_driver);
	
	if (result)
	{
		printk ("failed to register faketty driver\n");
		put_tty_driver (fake_tty_driver);
		return result;
	}
    
    printk("%s driver version %d.%d.%d initiliazed\n", DEV_NAME, DRV_MAJOR_VER, DRV_MINOR_VER,
               DRV_REVER_VER);
	return 0;
}

module_init(faketty_init);
module_exit(faketty_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);

