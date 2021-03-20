#include <linux/init.h>
#include <linux/module.h>
#include <linux/io.h>
#include "button_drv.h"

static volatile unsigned int *CRU_CLKGATE14_CON;
static volatile unsigned int *GRF_GPIO7B_IOMUX;
static volatile unsigned int *GPIO7_SWPORTA_DDR;
//static volatile unsigned int *GPIO7_SWPORTA_DR;
static volatile unsigned int *GPIO7_EXT_PORTA;

#define CRU_BASE_ADDRESS   ((unsigned long)(0xff760000))
#define GRF_BASE_ADDRESS   ((unsigned long)(0xff770000))
#define GPIO7_BASE_ADDRESS ((unsigned long)(0xff7e0000))

#define CRU_CLKGATE14_CON_OFFSET (0x0198)
#define GRF_GPIO7B_IOMUX_OFFSET  (0x0070)
#define GPIO_SWPORTA_DR_OFFSET   (0x0000)
#define GPIO_SWPORTA_DDR_OFFSET  (0x0004)
#define GPIO_EXT_PORTA_OFFSET    (0x0050)

static void board_rk3288_button_init_gpio(int which)
{
	printk("%s %s %d, init gpio for button %d\n", __FILE__, __FUNCTION__, __LINE__, which);
	if(which == 0) {
		if(!CRU_CLKGATE14_CON) {
			CRU_CLKGATE14_CON = ioremap(CRU_BASE_ADDRESS   + CRU_CLKGATE14_CON_OFFSET, 4);
			GRF_GPIO7B_IOMUX  = ioremap(GRF_BASE_ADDRESS   + GRF_GPIO7B_IOMUX_OFFSET,  4);
			GPIO7_SWPORTA_DDR = ioremap(GPIO7_BASE_ADDRESS + GPIO_SWPORTA_DDR_OFFSET,  4);
			GPIO7_EXT_PORTA   = ioremap(GPIO7_BASE_ADDRESS + GPIO_EXT_PORTA_OFFSET,    4);
		}
		//1. enable gpio7
		*CRU_CLKGATE14_CON |= (1<<(16+7)) | (0<<7);
		//2. set gpio7_b1 for gpio
		*GRF_GPIO7B_IOMUX |= (3<<(16+2));
		*GRF_GPIO7B_IOMUX &= ~(3<<2);
		//3. set pio7_b1 for input pin
		*GPIO7_SWPORTA_DDR &= ~(1<<9);
	}
}

static int board_rk3288_button_read_gpio(int which)
{
	printk("%s %s %d, read gpio for button %d\n", __FILE__, __FUNCTION__, __LINE__, which);
	if(which == 0) {
		return ((*GPIO7_EXT_PORTA)&(1<<9))?1:0;
	} else {
		return 0;
	}
}

struct button_operations mybutton_ops = {
	.count = 2,
	.init = board_rk3288_button_init_gpio,
	.read = board_rk3288_button_read_gpio,
};

static int board_rk3288_button_init(void)
{
	register_button_operations(&mybutton_ops);
	return 0;
}

static void board_rk3288_button_exit(void)
{
	unregister_button_operations();
}

module_init(board_rk3288_button_init);
module_exit(board_rk3288_button_exit);
MODULE_LICENSE("GPL");
