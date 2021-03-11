#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/io.h>

#include "led_ops.h"

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

static int board_demoo_led_init(int which)
{
	printk("%s %s line %d, led %d\n", __FILE__, __FUNCTION__, __LINE__, which);

	if(!CRU_CLKGATE14_CON) {
		CRU_CLKGATE14_CON = ioremap(CRU_BASE_PHY_ADDRESS + CRU_CLKGATE14_PHY_CON, 4);
		GRF_GPIO8A_IOMUX  = ioremap(GRF_BASE_PHY_ADDRESS + GRF_GPIO8A_PHY_IOMUX, 4);
		GPIO8_SWPORTA_DDR = ioremap(GPIO8_BASE_PHY_ADDRESS + GPIO_SWPORTA_PHY_DDR, 4);
		GPIO8_SWPORTA_DR  = ioremap(GPIO8_BASE_PHY_ADDRESS + GPIO_SWPORTA_PHY_DR, 4);
	}

	if(which == 0) {
		*CRU_CLKGATE14_CON = (1<<(8+16)) | (0<<8);
		*GRF_GPIO8A_IOMUX  |= (3<<(2+16)) | (0<<2);
		*GPIO8_SWPORTA_DDR |= (1<<1);
	} else if(which == 1) {
		*CRU_CLKGATE14_CON = (1<<(8+16)) | (0<<8);
		*GRF_GPIO8A_IOMUX  |= (3<<(4+16)) | (0<<4);
		*GPIO8_SWPORTA_DDR |= (1<<2);
	}
	return 0;
}

static int board_demoo_led_ctl(int which, char status)
{
	printk("%s %s line %d, led %d, %s\n", __FILE__, __FUNCTION__, __LINE__, which, status?"on":"off");
	if(which == 0) {
		if(status) {		/* on: output 0 */
			*GPIO8_SWPORTA_DR &= ~(1<<1);
		} else {			/* off: output 1 */
			*GPIO8_SWPORTA_DR |= (1<<1);
		}
	} else if(which == 1) {
		if(status) {
			*GPIO8_SWPORTA_DR &= ~(1<<2);
		} else {
			*GPIO8_SWPORTA_DR |= (1<<2);
		}
	}
	return 0;
}

static struct led_operations board_demoo_led_ops = {
	.num = 2,
	.init = board_demoo_led_init,
	.ctl  = board_demoo_led_ctl,
};

struct led_operations *get_board_led_ops(void)
{
	return &board_demoo_led_ops;
}

