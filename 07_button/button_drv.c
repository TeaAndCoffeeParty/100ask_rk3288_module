#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <asm/uaccess.h>

#include "button_drv.h"

int button_major = 0;
struct button_operations *btn_ops;
static struct class *button_class;

static int button_open(struct inode *inode, struct file *file)
{
	int minor = iminor(inode);
	btn_ops->init(minor);
	return 0;
}

static ssize_t button_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	unsigned int minor = iminor(file_inode(file));
	char level;
	int err;
	
	level = btn_ops->read(minor);
	err = copy_to_user(buf, &level, 1);

	return 1;
}

static const struct file_operations button_ops = {
    .owner      = THIS_MODULE,
    .open       = button_open,
    .read       = button_read,
};

void register_button_operations(struct button_operations *ops)
{
	int i;

	btn_ops = ops;
	for(i=0;i<btn_ops->count;i++) {
		device_create(button_class, NULL, MKDEV(button_major, i), NULL, "mybutton%d", i);
	}
}


void unregister_button_operations(void)
{
	int i;
	for(i=0;i<btn_ops->count;i++) {
		device_destroy(button_class, MKDEV(button_major, i));
	}
}

EXPORT_SYMBOL(register_button_operations);
EXPORT_SYMBOL(unregister_button_operations);

static int button_init(void)
{
	int res;
	button_major = register_chrdev(0, "mybutton", &button_ops);
    if (button_major < 0) {
        printk(KERN_ERR "button : couldn't get a major number.\n");
        return res;
    }

	button_class = class_create(THIS_MODULE, "button_class");
	if(IS_ERR(button_class)) {
        printk(KERN_ERR "button class: create failed\n");
		return -1;
	}

	return 0;
}

static void button_exit(void)
{
	class_destroy(button_class);
	unregister_chrdev(button_major, "mybutton");
}

module_init(button_init);
module_exit(button_exit);
MODULE_LICENSE("GPL");
