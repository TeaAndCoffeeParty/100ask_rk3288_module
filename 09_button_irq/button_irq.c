#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/uaccess.h>
#include <linux/irqreturn.h>
#include <linux/gpio/consumer.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/interrupt.h>

struct gpio_key {
	int gpio;
	struct gpio_desc *gpiod;
	int flag;
	int irq;
};
static struct gpio_key *myBtn_key;

static irqreturn_t myBtn_irq_request(int irq, void *dev_id)
{
	struct gpio_key *gpio_key = dev_id;
	int val;
	val = gpiod_get_value(gpio_key->gpiod);

	printk(KERN_WARNING"key %d %d\n", gpio_key->gpio, val);

	return IRQ_HANDLED;
}

static int my_button_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	int count;
	enum of_gpio_flags flag;
	int i, err;

	count = of_gpio_count(node);
	if(!count) {
		printk("%s,there isn't any gpio availiable\n", __FUNCTION__);
		return -1;
	}
	
	myBtn_key = (struct gpio_key*)kzalloc(sizeof(struct gpio_key)*count, GFP_KERNEL);
	if(!myBtn_key) {
		printk("%s,kzalloc malloc failed\n", __FUNCTION__);
		return -1;
	}


	for(i=0;i<count;i++) {
		myBtn_key[i].gpio = of_get_gpio_flags(node, i, &flag);
		if(myBtn_key[i].gpio < 0) {
			printk("%s, of_get_gpio_flags failed\n", __FUNCTION__);
			return -1;
		}
		myBtn_key[i].gpiod = gpio_to_desc(myBtn_key[i].gpio);
		myBtn_key[i].flag  = flag & OF_GPIO_ACTIVE_LOW;
		myBtn_key[i].irq   = gpio_to_irq(myBtn_key[i].gpio);
		err = request_irq(myBtn_key[i].irq, myBtn_irq_request, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
				"myBtn_key", &myBtn_key[i]);
	}

	return 0;
}

static int my_button_remove(struct platform_device *pdev)
{
	struct device_node *node= pdev->dev.of_node;
	int count;
	int i;

	count = of_gpio_count(node);
	for(i=0;i<count;i++) {
		free_irq(myBtn_key[i].irq, &myBtn_key[i]);
	}

	kfree(myBtn_key);
	return 0;
}

static struct of_device_id mybuttons[] = {
	{ .compatible = "mybtn,btn_drv" },
	{ },
};

static struct platform_driver my_button_driver = {
	.probe  = my_button_probe,
	.remove = my_button_remove,
	.driver = {
		.name = "button_dirver",
		.of_match_table = mybuttons,
	},
};

static int gpio_button_init(void)
{
	int err;
	err = platform_driver_register(&my_button_driver);
	printk(KERN_WARNING"my button dirver init\n");
	return 0;
}

static void gpio_button_exit(void)
{
	platform_driver_unregister(&my_button_driver);
	printk(KERN_WARNING"my button dirver exit\n");
}

module_init(gpio_button_init);
module_exit(gpio_button_exit);
MODULE_LICENSE("GPL");

