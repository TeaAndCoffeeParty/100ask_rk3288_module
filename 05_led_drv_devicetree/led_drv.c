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
#include "led_drv.h"

static int major = 0;
static struct class *led_class;
struct led_operations *p_led_ops;

void led_class_create_device(int index)
{
	device_create(led_class, NULL, MKDEV(major, index), NULL, "myled%d", index);
}

void led_class_destroy_device(int index)
{
	device_destroy(led_class, MKDEV(major, index));
}

void register_led_operations(struct led_operations *led_opr)
{
	p_led_ops = led_opr;
}

EXPORT_SYMBOL(led_class_create_device);
EXPORT_SYMBOL(led_class_destroy_device);
EXPORT_SYMBOL(register_led_operations);

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
	major = register_chrdev(0, "myled", &led_drv);

	led_class = class_create(THIS_MODULE, "led_class");
	err = PTR_ERR(led_class);
	if(IS_ERR(led_class)) {
		unregister_chrdev(major, "myled");	
		printk(KERN_WARNING "class creatge failed %d\n", err);
		return -1;
	}
	
	printk("%s %sled_drv_init\n", __FILE__, __FUNCTION__);
	return 0;
}

static void __exit led_drv_exit(void)
{
	printk("%s %sled_drv_exit\n", __FILE__, __FUNCTION__);

	class_destroy(led_class); 
	unregister_chrdev(major, "myled");	
}

module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("chen");

