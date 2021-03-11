#include <linux/kernel.h>
#include <linux/io.h>
#include "led_resource.h"
#include "led_ops.h"

static struct led_resource *led_rsc;

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
    if(!led_rsc) {
        led_rsc = get_led_resource();
    }
    
	printk("%s which %d", __FUNCTION__, which);
	if(GROUP(led_rsc->pin) == 8) {
		if(!CRU_CLKGATE14_CON) {
			CRU_CLKGATE14_CON = ioremap(CRU_BASE_PHY_ADDRESS + CRU_CLKGATE14_PHY_CON, 4);
			GRF_GPIO8A_IOMUX  = ioremap(GRF_BASE_PHY_ADDRESS + GRF_GPIO8A_PHY_IOMUX, 4);
			GPIO8_SWPORTA_DDR = ioremap(GPIO8_BASE_PHY_ADDRESS + GPIO_SWPORTA_PHY_DDR, 4);
			GPIO8_SWPORTA_DR  = ioremap(GPIO8_BASE_PHY_ADDRESS + GPIO_SWPORTA_PHY_DR, 4);
		}

		if(PIN(led_rsc->pin) == 1) {
			*CRU_CLKGATE14_CON = (1<<(8+16)) | (0<<8);
			*GRF_GPIO8A_IOMUX  |= (3<<(2+16)) | (0<<2);
			*GPIO8_SWPORTA_DDR |= (1<<1);
		}
	}

	return 0;

}

static int board_demo_led_ctrl(int which, char status)
{
	printk("%s which %d, status %c", __FUNCTION__, which, status);
	if(PIN(led_rsc->pin) == 1) {
		if(status) {		/* on: output 0 */
			*GPIO8_SWPORTA_DR &= ~(1<<1);
		} else {			/* off: output 1 */
			*GPIO8_SWPORTA_DR |= (1<<1);
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

