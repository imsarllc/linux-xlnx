/* uio_counter.c */

// includes
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <linux/fs.h> //required for fops
#include <linux/uaccess.h> //required for 'copy_from_user' and 'copy_to_user'
#include <linux/signal.h> //required for kernel-to-userspace signals
#include <linux/uio_driver.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");

// defines
#define IRQ_NUM      90  // interrupt line
#define DRIVER_NAME  "uio0"  // device name
#define MYINT_MAJOR  22  // device major number
#define BUF_LEN      80  // max buffer length

// globals
/*
static unsigned long *pMYINT_REG;  // pointer to timer 0 control and status register
*/
static int device_open = 0;		// device status
static char msg[BUF_LEN];  // the msg the device will give when asked
static int enable = 0;
static int intrpt_cnt;


// read
static long myint_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	char *buf = (char*)arg;

	printk("myint_ioctl\n");

	//val = ioread32(pMYINT_REG);

	if (copy_to_user(buf, &intrpt_cnt, sizeof(int)) != 0)
		return -EFAULT;

	return 0;
}


// copied from uio.c
struct uio_device {
	struct module		*owner;
	struct device		*dev;
	int			minor;
	atomic_t		event;
	struct fasync_struct	*async_queue;
	wait_queue_head_t	wait;
	struct uio_info		*info;
	struct kobject		*map_dir;
	struct kobject		*portio_dir;
};


struct uio_listener {
	struct uio_device *dev;
	s32 event_count;
};


// read routine (called when read() is used in user-space)
static ssize_t myint_read(struct file *filep, char *buf, size_t length, loff_t *offset)
{
	struct uio_listener *listener = filep->private_data;
	struct uio_device *idev = listener->dev;
	DECLARE_WAITQUEUE(wait, current);
	ssize_t retval;
	s32 event_count;

	if (!idev->info->irq)
		return -EIO;

	if (length != sizeof(s32))
		return -EINVAL;

	add_wait_queue(&idev->wait, &wait);

	do {
		set_current_state(TASK_INTERRUPTIBLE);

		event_count = atomic_read(&idev->event);
		if (event_count != listener->event_count) {
			if (copy_to_user(buf, &event_count, length))
				retval = -EFAULT;
			else {
				listener->event_count = event_count;
				retval = length;
			}
			break;
		}

		if (filep->f_flags & O_NONBLOCK) {
			retval = -EAGAIN;
			break;
		}

		if (signal_pending(current)) {
			retval = -ERESTARTSYS;
			break;
		}
		schedule();
	} while (1);

	__set_current_state(TASK_RUNNING);
	remove_wait_queue(&idev->wait, &wait);

	return retval;
}


// write routine (called when write() is used in user-space)
static ssize_t myint_write(struct file *filep, const char *buf, size_t length, loff_t *offset)
{
	printk("myint_write\n");

	if (copy_from_user(msg, buf, length) != 0)
		return -EFAULT;
	printk("Received: %s\n", msg);

	if (strcmp(msg, "1") == 0)
	{
		enable = 1;
		intrpt_cnt = 0;
		printk("Enable\n");
	}
	else if (strcmp(msg, "0") == 0)	
	{
		enable = 0;
		printk("Disable\n");
	}
	else 
	{
		printk("Driver received wrong value\n");
		return -EFAULT;
	}

/*
	iowrite32(val, pMYINT_REG);
*/

	return 0;
}


// open routine (called when a device opens /dev/myint)
static int myint_open(struct inode *inode, struct file *file)
{
	if (device_open)
		return -EBUSY;

	device_open++;
	sprintf(msg, "myint_open\n");
	try_module_get(THIS_MODULE);

	return 0;
}


// close routine (called whne a device closes /dev/myint)
static int myint_close(struct inode *inode, struct file *file)
{
	if (device_open > 0)
	{
		device_open--;
		module_put(THIS_MODULE);
	}

	return 0;
}


// device init and file operations
struct file_operations myint_fops = {
	.unlocked_ioctl = myint_ioctl,
	.read = myint_read,
	.write = myint_write,
	.open = myint_open,
	.release = myint_close,
};


// timer interrupt handler
static irqreturn_t irq_handler(int irq,void*dev_id)		
{
	if (enable)
	{
		intrpt_cnt++;
		printk("%s: intrpt_cnt %d\n", DRIVER_NAME, intrpt_cnt);
	}

	return IRQ_HANDLED;
}


// init module      
static int __init myint_init(void)  
{
	intrpt_cnt = 0;

	if (request_irq(IRQ_NUM, irq_handler, IRQF_DISABLED, DRIVER_NAME, NULL))
	{
		printk(KERN_ERR "IRQ#%d not registered\n", IRQ_NUM);
		return -EBUSY;
	}

/*
	pMYINT_REG = ioremap_nocache(MYINT_REG, 0x4); // map register
*/

	// manual node creation
	if (register_chrdev(MYINT_MAJOR, DRIVER_NAME, &myint_fops))
	{
		printk(KERN_ERR "Unable to register major device 22\n");
		//return -EBUSY;
	}

	printk("Type: mknod /dev/%s c %d 0\n", DRIVER_NAME, MYINT_MAJOR);

	printk(KERN_INFO "UIO module loaded for IRQ#%d\n", IRQ_NUM);

	return 0;
}


// exit module
static void __exit myint_exit(void)  		
{
/*
	iounmap(pMYINT_REG);
*/
	free_irq(IRQ_NUM, NULL);
	unregister_chrdev(MYINT_MAJOR, DRIVER_NAME);
	printk(KERN_ERR "Exit myint module\n");
}


module_init(myint_init);
module_exit(myint_exit);

MODULE_AUTHOR ("Joseph Hancock");
MODULE_DESCRIPTION("Zynq PL interrupt handler.");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("custom:myint");
