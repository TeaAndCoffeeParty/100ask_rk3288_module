#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <asm/uaccess.h>

static int ap3216_major = 0;
static struct class *ap3216_class;
static struct i2c_client *ap3216_client;

static const struct of_device_id of_match_ids_ap3216[] = {
	{ .compatible = "com_name,chip_name",	.data = NULL },
	{ /* END OF LIST */ }
};

static const struct i2c_device_id ap3216_ids[] = {
	{ "chip_name", (kernel_ulong_t)NULL },
	{ /* END OF LIST */ }
};

int ap3216_open(struct inode *inode, struct file *file)
{
	i2c_smbus_write_byte_data(ap3216_client, 0, 0x4);
	mdelay(20);
	i2c_smbus_write_byte_data(ap3216_client, 0, 0x3);
	return 0;
}

ssize_t ap3216_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	int err;
	char kernel_buf[6];
	int val;

	if(size != 6)
		return -EINVAL;

	val = i2c_smbus_read_word_data(ap3216_client, 0xA);
	kernel_buf[0] = val & 0xff;
	kernel_buf[1] = (val >> 8) & 0xff;

	val = i2c_smbus_read_word_data(ap3216_client, 0xC);
	kernel_buf[2] = val & 0xff;
	kernel_buf[3] = (val >> 8) & 0xff;

	val = i2c_smbus_read_word_data(ap3216_client, 0xE);
	kernel_buf[4] = val & 0xff;
	kernel_buf[5] = (val >> 8) & 0xff;

	err = copy_to_user(buf, kernel_buf, size);
	return size;
}
//ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
//int (*release) (struct inode *, struct file *);


static struct file_operations ap3216_ops = {
	.owner = THIS_MODULE,
	.open  = ap3216_open,
//	.write = ap3216_write,
	.read  = ap3216_read,
//	.close = ap3216_close,
};

int ap3216_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	ap3216_client = client;

	ap3216_major = register_chrdev(0, "ap3216", &ap3216_ops);
	if(ap3216_major < 0) {
		printk(KERN_ERR "ap3216: couldn't get a major number.\n");
		return -1;
	}

	ap3216_class = class_create(THIS_MODULE, "ap3216_class");
	if(IS_ERR(ap3216_class)) {
		printk(KERN_ERR "ap3216 class: create failed.\n");
		unregister_chrdev(ap3216_major, "ap3216");
		return -1;
	}

	device_create(ap3216_class, NULL, MKDEV(ap3216_major, 0), NULL, "ap3216");
	return 0;
}

int ap3216_remove(struct i2c_client *client)
{
	device_destroy(ap3216_class, MKDEV(ap3216_major, 0));
	class_destroy(ap3216_class);
	unregister_chrdev(ap3216_major, "ap3216");

	return 0;
}

static struct i2c_driver i2c_driver_ap3216 = {
	.driver = {
		.name = "ap3216",
		.of_match_table = of_match_ids_ap3216,
	},
	.probe    = ap3216_probe,
	.remove   = ap3216_remove,
	.id_table = ap3216_ids,
};

static int __init i2c_driver_ap3216_init(void)
{
	int ret = i2c_add_driver(&i2c_driver_ap3216);
	return ret;
}

static void __exit i2c_driver_ap3216_exit(void)
{
	i2c_del_driver(&i2c_driver_ap3216);
}

module_init(i2c_driver_ap3216_init);
module_exit(i2c_driver_ap3216_exit);

MODULE_AUTHOR("chentuo");
MODULE_LICENSE("GPL");

