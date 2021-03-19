#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#define  BUTTON_MAJOR 0

static int button_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t button_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	return 0;
}

static int button_release(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations button_ops = {
    .owner      = THIS_MODULE,
    .open       = button_open,
    .read       = button_read,
    .release    = button_release,
};

static int button_init(void)
{
	int res;
	res = register_chrdev(BUTTON_MAJOR, "mybutton", &button_ops);
    if (res < 0) {
        printk(KERN_ERR "button : couldn't get a major number.\n");
        return res;
    }

	return 0;
}

static void button_exit(void)
{
	unregister_chrdev(BUTTON_MAJOR, "mybutton");
}

module_init(button_init);
module_exit(button_exit);
MODULE_LICENSE("GPL");
