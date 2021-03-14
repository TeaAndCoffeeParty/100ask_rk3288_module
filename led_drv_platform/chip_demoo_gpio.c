#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include "led_resource.h"
#include "led_ops.h"
#include "led_drv.h"

static unsigned int led_pins[100];
static int resPinCount;

#define CRU_BASE_PHY_ADDRESS   ((unsigned long)(0xff760000))
#define GRF_BASE_PHY_ADDRESS   ((unsigned long)(0xff770000))
#define GPIO8_BASE_PHY_ADDRESS ((unsigned long)(0xff7f0000))


#define CRU_CLKGATE14_PHY_CON (0x0198)
#define GRF_GPIO8A_PHY_IOMUX  (0x0080)
#define GPIO_SWPORTA_PHY_DR   (0x0000)
#define GPIO_SWPORTA_PHY_DDR  (0x0004)

static volatile unsigned int *CRU_CLKGATE14_CON;
static volatile unsigned int *GRF_GPIO8A_IOMUX;
static volatile unsigned int *GPIO8_SWPORTA_DDR;
static volatile unsigned int *GPIO8_SWPORTA_DR;

static int board_demo_led_init(int which)
{
    
	printk("%s which %d", __FUNCTION__, which);
	if(GROUP(led_pins[which]) == 8) {
		if(!CRU_CLKGATE14_CON) {
			CRU_CLKGATE14_CON = ioremap(CRU_BASE_PHY_ADDRESS + CRU_CLKGATE14_PHY_CON, 4);
			GRF_GPIO8A_IOMUX  = ioremap(GRF_BASE_PHY_ADDRESS + GRF_GPIO8A_PHY_IOMUX, 4);
			GPIO8_SWPORTA_DDR = ioremap(GPIO8_BASE_PHY_ADDRESS + GPIO_SWPORTA_PHY_DDR, 4);
			GPIO8_SWPORTA_DR  = ioremap(GPIO8_BASE_PHY_ADDRESS + GPIO_SWPORTA_PHY_DR, 4);
		}

		if(PIN(led_pins[which]) == 1) {
			*CRU_CLKGATE14_CON  = (1<<(8+16)) | (0<<8);
			*GRF_GPIO8A_IOMUX  |= (3<<(2+16)) | (0<<2);
			*GPIO8_SWPORTA_DDR |= (1<<1);
		} else if(PIN(led_pins[which]) == 2) {
			*CRU_CLKGATE14_CON  = (1<<(8+16)) | (0<<8);
			*GRF_GPIO8A_IOMUX  |= (3<<(4+16)) | (0<<4);
			*GPIO8_SWPORTA_DDR |= (1<<2);
		}
	}

	return 0;

}

static int board_demo_led_ctrl(int which, char status)
{
	printk("%s which %d, status %c", __FUNCTION__, which, status);
	if(PIN(led_pins[which]) == 1) {
		if(status) {		/* on: output 0 */
			*GPIO8_SWPORTA_DR &= ~(1<<1);
		} else {			/* off: output 1 */
			*GPIO8_SWPORTA_DR |= (1<<1);
		}
	} else if(PIN(led_pins[which]) == 2) {
		if(status) {
			*GPIO8_SWPORTA_DR &= ~(1<<2);
		} else {
			*GPIO8_SWPORTA_DR |= (1<<2);
		}
	}
	return 0;
}

static struct led_operations led_opr = {
	.init = board_demo_led_init,
	.ctl = board_demo_led_ctrl,
};

struct led_operations *get_board_led_ops(void)
{
	return &led_opr; 
}

static int led_platform_driver_probe(struct platform_device *pdev)
{
	int i = 0;
	struct resource *res;
	while(1) {
		//1. get board A led resource
		res = platform_get_resource(pdev, IORESOURCE_IRQ, i);
		if(res == NULL) {
			break;
		}
		led_pins[i] = res->start;
		resPinCount++;

		//2. create device
		led_class_create_device(i);
		i++;
	}
	return 0;
}

static int led_platform_driver_remove(struct platform_device *pdev)
{
	struct resource *res;
	while(1) {
		//1. get board a led resource
		res = platform_get_resource(pdev, IORESOURCE_IRQ, resPinCount);
		if(res == NULL) {
			break;
		}
		//2. destory device
		led_class_destroy_device(resPinCount);
		resPinCount--;
	}
	return 0;
}

static struct platform_driver led_driver = {
	.driver = {
		.name = "myled",
	},
	.probe = led_platform_driver_probe,
	.remove = led_platform_driver_remove,
};

static int led_platform_driver_init(void)
{
	int err;
	err = platform_driver_register(&led_driver);
	register_led_operations(&led_opr); 
	return 0;
}

static void led_platform_driver_exit(void)
{
	platform_driver_unregister(&led_driver);
}

module_init(led_platform_driver_init);
module_exit(led_platform_driver_exit);
MODULE_LICENSE("GPL");
