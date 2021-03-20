#include <linux/init.h>
#include <linux/module.h>
#include "button_drv.h"


static void board_rk3288_button_init_gpio(int which)
{
	printk("%s %s %d, init gpio for button %d\n", __FILE__, __FUNCTION__, __LINE__, which);
}

static int board_rk3288_button_read_gpio(int which)
{
	printk("%s %s %d, read gpio for button %d\n", __FILE__, __FUNCTION__, __LINE__, which);
	return 1;
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
