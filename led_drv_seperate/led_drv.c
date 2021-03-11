#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/major.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#include "led_ops.h"

static int major = 0;
static struct class *led_class;
struct led_operations *p_led_ops;

static int led_drv_open(struct inode *node, struct file *file)
{
	int minor = iminor(node);
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

	p_led_ops->init(minor);	
	return 0;
}

static ssize_t led_drv_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

static ssize_t led_drv_write(struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	int err;
	char status;
	struct inode *inode = file_inode(file);
	int minor = iminor(inode);

	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	err = copy_from_user(&status, buf, 1);
	printk("write status:%x \n", status);

	p_led_ops->ctl(minor, status);
	
	return 1;
}

static int led_drv_close(struct inode *node, struct file *file)
{
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}


static struct file_operations led_drv = {
	.owner = THIS_MODULE,
	.open = led_drv_open,
	.read = led_drv_read,
	.write = led_drv_write,
	.release = led_drv_close,
};

static int __init led_drv_init(void)
{
	int err;
	// int i = 0;
	major = register_chrdev(0, "myled", &led_drv);

	led_class = class_create(THIS_MODULE, "led_class");
	err = PTR_ERR(led_class);
	if(IS_ERR(led_class)) {
		unregister_chrdev(major, "myled");	
		printk(KERN_WARNING "class creatge failed %d\n", err);
		return -1;
	}


	p_led_ops = get_board_led_ops();
	// for(i=0;i<p_led_ops->num;i++)
		device_create(led_class, NULL, MKDEV(major, 0), NULL, "myled%d", 0);
	
	printk("%s %sled_drv_init\n", __FILE__, __FUNCTION__);
	return 0;
}

static void __exit led_drv_exit(void)
{
	// int i;
	printk("%s %sled_drv_exit\n", __FILE__, __FUNCTION__);
	// for(i=0;i<p_led_ops->num;i++)
		device_destroy(led_class, MKDEV(major, 0));

	class_destroy(led_class); 
	unregister_chrdev(major, "myled");	
}

module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("chen");

