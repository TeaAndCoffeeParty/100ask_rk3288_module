#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/major.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/platform_device.h>

//#include "led_ops.h"
//#include "led_drv.h"

static int major = 0;
static struct class *led_class;
//struct led_operations *p_led_ops;
struct gpio_desc *led_gpio;

static int led_drv_open(struct inode *node, struct file *file)
{
//	int minor = iminor(node);
	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

//	p_led_ops->init(minor);	
	gpiod_direction_output(led_gpio, 0);
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
//	struct inode *inode = file_inode(file);
//	int minor = iminor(inode);

	printk("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);
	err = copy_from_user(&status, buf, 1);
	printk("write status:%x \n", status);

//	p_led_ops->ctl(minor, status);
	gpiod_set_value(led_gpio, status);
	
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


static const struct of_device_id myleds[] = {
	{ .compatible = "myled,led_drv"},
	{ },
};


static int chip_demoo_gpio_probe(struct platform_device *pdev)
{
	led_gpio = gpiod_get(&pdev->dev, "myled", 0);
	if(IS_ERR(led_gpio)) {
		dev_err(&pdev->dev, "Failed to get GPIO for led\n");
		return PTR_ERR(led_gpio);
	}

	major = register_chrdev(0, "myled", &led_drv);
	led_class = class_create(THIS_MODULE, "led_class");
	if(IS_ERR(led_class)) {
		unregister_chrdev(major, "myled");	
		gpiod_put(led_gpio);
		printk(KERN_WARNING "class create failed\n");
		return PTR_ERR(led_class);
	}

	device_create(led_class, NULL, MKDEV(major, 0), NULL, "myled%d", 0);

	return 0;
}

static int chip_demoo_gpio_remove(struct platform_device *pdev)
{
	device_destroy(led_class, MKDEV(major, 0));
	class_destroy(led_class); 
	unregister_chrdev(major, "myled");	
	gpiod_put(led_gpio);
	return 0;
}

static struct platform_driver chip_demoo_gpio_driver = {
	.probe    = chip_demoo_gpio_probe,
	.remove   = chip_demoo_gpio_remove,
	.driver   = {
		.name = "myled",
		.of_match_table = myleds,
	},
};

static int __init led_drv_init(void)
{
	int err;
	err = platform_driver_register(&chip_demoo_gpio_driver);

	printk("%s %sled_drv_init\n", __FILE__, __FUNCTION__);
	return 0;
}

static void __exit led_drv_exit(void)
{
	printk("%s %sled_drv_exit\n", __FILE__, __FUNCTION__);

	platform_driver_unregister(&chip_demoo_gpio_driver);
}

module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("chen");

