#include "button_drv.h"


void board_rk3288_button_init_gpio(int which)
{

}

void board_rk3288_button_exit_gpio(int which)
{

{

}

struct button_operations mybutton_ops = {
	.count = 2,
	.init = board_rk3288_button_init_gpio,
	.read = board_rk3288_button_exit_gpio,
}

void register_button_operations(struct button_operations *ops)
{
	ops = &mybutton_ops;
}
EXPORT_SYMBOL(register_button_operations);

void unregister_button_operations(void)
{

}
EXPORT_SYMBOL(unregister_button_operations);
