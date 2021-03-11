#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include "led_resource.h"

struct resource led_resources[] = {
	{
		.start = GROUP_PIN(8,1),
		.flags = IORESOURCE_IRQ,
		.name = "myled0",
	},
	{
		.start = GROUP_PIN(8,2),
		.flags = IORESOURCE_IRQ,
		.name = "myled1",
	},
};

struct resource *get_led_resource(void)
{
    return led_resources;
}

static struct platform_device led_device  = {
	.name = "myled_device",
	.resource = led_resources,
	.num_resources = ARRAY_SIZE(led_resources),
};

static int led_device_init(void)
{
	int err;
	err = platform_device_register(&led_device);
	return err;
}

static void led_device_exit(void)
{
	platform_device_unregister(&led_device);
}

module_init(led_device_init);
module_exit(led_device_exit);
MODULE_LICENSE("GPL");
