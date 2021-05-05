#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <asm/uaccess.h>

static int pca9685_major = 0;
static struct class *pca9685_class;
static struct i2c_client *pca9685_client;

static const struct of_device_id of_match_ids_pca9685[] = {
	{ .compatible = "nxp,pca9685",	.data = NULL },
	{ /* END OF LIST */ }
};

static const struct i2c_device_id pca9685_ids[] = {
	{ "pca9685", (kernel_ulong_t)NULL },
	{ /* END OF LIST */ }
};

int pca9685_open(struct inode *inode, struct file *file)
{
	struct i2c_adapter *adapter = pca9685_client->adapter;
	struct i2c_msg msg[2];
	int err, i=0;
	msg[0].addr  = 0x40;
	msg[0].flags = 0;
	msg[0].buf   = 0x0;
	msg[0].len   = 1;	

	msg[1].addr  = 0x40;
	msg[1].flags = I2C_M_RD;
	msg[1].buf   = 0x0;
	msg[1].len   = 1;

	if(!adapter) {
		printk(KERN_ERR "pca9685: adapter nullptr.\n");
		return -1;
	}

	err = i2c_transfer(adapter, msg, 2);
	if(err < 0) {
		printk(KERN_ERR "%s i2c_transfer error:%d\n", __FUNCTION__, err);
		return -1;
	} else {
		printk(KERN_ERR "%s i2c_transfer ok:%d\n", __FUNCTION__, err);
		for(i=0;i<msg[1].len;i++) {
			printk(KERN_ERR "msg[1].buf %d:0x%x \n", i, msg[1].buf[i]);
		}
	}

	return 0;
}

ssize_t pca9685_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	char kernel_buf[6];
	int err = 0;

	err = copy_to_user(buf, kernel_buf, size);
	return size;
}
//ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
//int (*release) (struct inode *, struct file *);


static struct file_operations pca9685_ops = {
	.owner = THIS_MODULE,
	.open  = pca9685_open,
	.read  = pca9685_read,
};

int pca9685_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	printk(KERN_ERR "pca9685: probe.\n");
	pca9685_client = client;

	pca9685_major = register_chrdev(0, "pca9685", &pca9685_ops);
	if(pca9685_major < 0) {
		printk(KERN_ERR "pca9685: couldn't get a major number.\n");
		return -1;
	}

	pca9685_class = class_create(THIS_MODULE, "pca9685_class");
	if(IS_ERR(pca9685_class)) {
		printk(KERN_ERR "pca9685 class: create failed.\n");
		unregister_chrdev(pca9685_major, "pca9685");
		return -1;
	}

	device_create(pca9685_class, NULL, MKDEV(pca9685_major, 0), NULL, "pca9685");
	return 0;
}

int pca9685_remove(struct i2c_client *client)
{
	printk(KERN_ERR "pca9685: remove.\n");
	device_destroy(pca9685_class, MKDEV(pca9685_major, 0));
	class_destroy(pca9685_class);
	unregister_chrdev(pca9685_major, "pca9685");

	return 0;
}

static struct i2c_driver i2c_driver_pca9685 = {
	.driver = {
		.name = "pca9685",
		.of_match_table = of_match_ids_pca9685,
	},
	.probe    = pca9685_probe,
	.remove   = pca9685_remove,
	.id_table = pca9685_ids,
};

static int __init pca9685_init(void)
{
	int ret = i2c_add_driver(&i2c_driver_pca9685);
	printk(KERN_ERR "pca9685: init.\n");
	return ret;
}

static void __exit pca9685_exit(void)
{
	i2c_del_driver(&i2c_driver_pca9685);
	printk(KERN_ERR "pca9685: exit.\n");
}

module_init(pca9685_init);
module_exit(pca9685_exit);

MODULE_AUTHOR("chentuo");
MODULE_LICENSE("GPL");

